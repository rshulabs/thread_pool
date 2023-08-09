#include "thread_pool.hpp"

const int TASK_MAX_THRESHOLD = 1024;
const int THREAD_MAX_THRESHOLD = 100;
const int THREAD_MAX_IDLE_TIME = 60; // s

/**
 * Task对象
 */
Task::Task() : result_(nullptr) {}

void Task::setResult(Result *res)
{
    result_ = res;
}

void Task::exec()
{
    if (result_ != nullptr)
    {
        result_->setVal(run());
    }
}

/**
 * Result对象
 */
Result::Result(std::shared_ptr<Task> task, bool isValid)
    : task_(task),
      isValid_(isValid)
{
    task_->setResult(this);
}

Any Result::get()
{
    if (!isValid_)
        return "";
    sem_.wait();            // 若task没有执行完，会阻塞用户进程
    return std::move(any_); // 禁止左值赋值
}

void Result::setVal(Any any)
{
    this->any_ = std::move(any);
    sem_.post(); // 已经获取任务返回值，sem+1
}

/**
 * 线程池对象
 */

ThreadPool::ThreadPool() : initThreadSize_(0),
                           taskSize_(0),
                           taskQueThreshold_(TASK_MAX_THRESHOLD),
                           poolMode_(PoolMode::MODE_FIXED),
                           isPoolRunning_(false),
                           idleThreadSize_(0),
                           threadMaxSizeThreshold_(THREAD_MAX_THRESHOLD),
                           curThreadSize_(0)
{
}

ThreadPool::~ThreadPool()
{
    isPoolRunning_ = false;
    notEmpty_.notify_all();
    // 等待线程池里所有线程返回 阻塞和运行线程
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    exitCond_.wait(lock, [&]() -> bool
                   { return threads_.size() == 0; });
}

// 设置工作模式
void ThreadPool::setMode(PoolMode mode)
{
    // 不允许启动之后设置mode
    if (checkRunningState())
        return;
    poolMode_ = mode;
}

// 设置任务队列阈值
void ThreadPool::setTaskQueThreshold(int threshold)
{
    // 不允许启动之后设置mode
    if (checkRunningState())
        return;
    taskQueThreshold_ = threshold;
}

// 设置cached模式线程上限阈值
void ThreadPool::setThreadSizeThreshold(int threshold)
{
    // 不允许启动之后设置mode
    if (checkRunningState())
        return;
    // 只有cached有上限
    if (poolMode_ == PoolMode::MODE_CACHED)
        threadMaxSizeThreshold_ = threshold;
}

// 提交任务
Result ThreadPool::submitTask(std::shared_ptr<Task> sp)
{
    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueMtx_);
    // 线程的通信 等待队列有空余
    /**
     * 面试点：队列满了，需要设置超时时间，超过返回提交任务失败响应
     */
    // 改变为等待状态 等待一秒
    if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
                           { return taskQue_.size() < taskQueThreshold_; }))
    {
        // 等待1s后，条件依然没有满足-队列还是慢的 输出到标准输出
        std::cerr << "task queue is full,submit task fail." << std::endl;
        return Result(sp, false);
    }
    // 有空余，加入等待队列
    taskQue_.emplace(sp);
    taskSize_++;
    // 此时队列不空，在notEmpty上通知
    notEmpty_.notify_all();
    // cached模式 任务处理比较紧急 场景：小而快的任务 需要根据任务数量和空闲线程的数量，判断是否需要扩容
    if (poolMode_ == PoolMode::MODE_CACHED && taskSize_ > idleThreadSize_ && curThreadSize_ < threadMaxSizeThreshold_)
    {
        std::cout << "cached mode triggled,create new thread." << std::endl;
        // 创建新线程
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        // unique_ptr不允许右值拷贝 move移动语义
        threads_.emplace(threadId, std::move(ptr));
        // 启动线程
        threads_[threadId]->start();
        // 修改线程个数变量
        curThreadSize_++;
        idleThreadSize_++;
    }
    // 返回任务result对象
    return Result(sp);
}

// 开启线程池
void ThreadPool::start(int initThreadSize)
{
    // 设置线程池运行状态
    isPoolRunning_ = true;
    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    curThreadSize_ = initThreadSize;
    // 创建线程对象
    for (int i = 0; i < initThreadSize_; i++)
    {
        // 创建线程对象，把线程函数给到thread对象
        // move移动
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        // unique_ptr不允许右值拷贝 move移动语义
        threads_.emplace(threadId, std::move(ptr));
    }

    // 启动所有线程
    for (int i = 0; i < initThreadSize_; i++)
    {
        threads_[i]->start();
        idleThreadSize_++; // 记录初始空闲线程数量
    }
}

// 检查pool运行状态
bool ThreadPool::checkRunningState() const
{
    return isPoolRunning_;
}

// 定义线程函数 线程池所有线程从任务队列里消费任务
void ThreadPool::threadFunc(int threadId)
{
    auto lastTime = std::chrono::high_resolution_clock().now();
    while (isPoolRunning_)
    {
        std::shared_ptr<Task> task;
        {
            // 获取锁
            std::unique_lock<std::mutex> lock(taskQueMtx_);

            std::cout << std::this_thread::get_id() << "尝试获取任务" << std::endl;
            while (taskQue_.size() == 0)
            {
                if (poolMode_ == PoolMode::MODE_CACHED)
                {
                    // 每一秒返回一次
                    while (taskQue_.size() == 0)
                    {
                        // 超时返回
                        if (std::cv_status::timeout == notEmpty_.wait_for(lock, std::chrono::seconds(1)))
                        {
                            auto now = std::chrono::high_resolution_clock().now();
                            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                            if (dur.count() >= THREAD_MAX_IDLE_TIME && curThreadSize_ > initThreadSize_)
                            {
                                /*闲置了60s，回收当前线程*/
                                // 把线程对象从线程容器里删除
                                threads_.erase(threadId);
                                // 记录线程数量的相关变量值修改
                                curThreadSize_--;
                                idleThreadSize_--;
                                std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                                return;
                            }
                        }
                    }
                }
                else
                {
                    // 等待notEmpty条件 这里一直等待
                    notEmpty_.wait(lock);
                }
                if (!isPoolRunning_)
                {
                    // 把线程对象从线程容器里删除
                    threads_.erase(threadId);
                    std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                    exitCond_.notify_all();
                    return;
                }
            }
            // 消费了，空闲线程--
            idleThreadSize_--;
            // 从任务队列取一个任务
            task = taskQue_.front();
            taskQue_.pop();
            taskSize_--;
            std::cout << std::this_thread::get_id() << "获取任务成功" << std::endl;
            // 如果仍然有其他任务，继续通知其他任务
            if (taskQue_.size() > 0)
            {
                notEmpty_.notify_all();
            }
            // 取出一个任务，通知
            notFull_.notify_all();
        } // 释放锁
        // 当前线程负责执行此任务
        if (task != nullptr)
        {
            // task->run();
            // 执行任务，完后将返回值setVal到Result
            task->exec();
        }
        // 处理完了，空闲线程++
        idleThreadSize_++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
    // 把线程对象从线程容器里删除
    threads_.erase(threadId);
    std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
    exitCond_.notify_all();
}

/**
 * 线程对象
 */
Thread::Thread(ThreadFunc func)
    : func_(func),
      threadId_(genId_++)
{
}

Thread::~Thread() {}

int Thread::genId_ = 0;

// 获取id
int Thread::getId() const
{
    return threadId_;
}

// 启动线程
void Thread::start()
{
    // 创建一个线程来执行一个线程函数
    std::thread t(func_, threadId_); // c++11来说 线程对象t 和线程函数func
    t.detach();                      // 设置分离线程
}
