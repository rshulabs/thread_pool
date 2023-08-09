#pragma

#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <future>

/**
 * package-task future版
 */

const int THREAD_MAX_THRESHOLD = 2; // INT32_MAX;
const int TASK_MAX_THRESHOLD = 1024;
const int THREAD_MAX_IDLE_TIME = 60; // 单位：秒

// 线程池支持模式
enum PoolMode
{
    MODE_FIXED,  // 固定数量线程
    MODE_CACHED, // 线程数量可动态增长
};

// 线程类型
class Thread
{
public:
    // 通用函数类型，接受一个无参数且无返回值的函数
    using ThreadFunc = std::function<void(int)>;
    Thread(ThreadFunc func) : func_(func), threadId_(genId_++)
    {
    }
    ~Thread() = default;

    // 启动线程
    void start()
    {
        // 创建一个线程来执行一个线程函数
        std::thread t(func_, threadId_); // c++11来说 线程对象t 和线程函数func
        t.detach();                      // 设置分离线程
    }

    // 获取线程id
    int getId() const
    {
        return threadId_;
    }

private:
    ThreadFunc func_;
    static int genId_;
    int threadId_; // 线程id
};
int Thread::genId_ = 0;

// 线程池类型
class ThreadPool
{
public:
    ThreadPool() : initThreadSize_(0),
                   taskSize_(0),
                   taskQueThreshold_(TASK_MAX_THRESHOLD),
                   poolMode_(PoolMode::MODE_FIXED),
                   isPoolRunning_(false),
                   idleThreadSize_(0),
                   threadMaxSizeThreshold_(THREAD_MAX_THRESHOLD),
                   curThreadSize_(0)
    {
    }
    ~ThreadPool()
    {
        isPoolRunning_ = false;
        notEmpty_.notify_all();
        // 等待线程池里所有线程返回 阻塞和运行线程
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        exitCond_.wait(lock, [&]() -> bool
                       { return threads_.size() == 0; });
    }

    // 设置线程池工作模式
    void setMode(PoolMode mode)
    {
        if (checkRunningState())
            return;
        poolMode_ = mode;
    }

    // 设置task任务上限阈值
    void setTaskQueThreshold(int threshold)
    {
        if (checkRunningState())
            return;
        taskQueThreshold_ = threshold;
    }

    // 设置cached模式线程上限阈值
    void setThreadSizeThreshold(int threshold)
    {
        if (poolMode_ == MODE_CACHED)
        {
            if (checkRunningState())
                return;
            threadMaxSizeThreshold_ = threshold;
        }
    }

    // 提交任务 模板参数
    template <typename Func, typename... Args>
    auto submitTask(Func &&func, Args &&...args) -> std::future<decltype(func(args...))>
    {
        using RType = decltype(func(args...));
        auto task = std::make_shared<std::packaged_task<RType()>>(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<RType> res = task->get_future();
        // 获取锁
        std::unique_lock<std::mutex> lock(taskQueMtx_);
        if (!notFull_.wait_for(lock, std::chrono::seconds(1), [&]() -> bool
                               { return taskQue_.size() < taskQueThreshold_; }))
        {
            // 等待1s后，条件依然没有满足-队列还是慢的 输出到标准输出
            std::cerr << "task queue is full,submit task fail." << std::endl;
            auto task = std::make_shared<std::packaged_task<RType()>>([]() -> RType
                                                                      { return RType(); });
            (*task)();
            return task->get_future();
        }
        // 有空余，加入等待队列
        taskQue_.emplace([task]()
                         { (*task)(); });
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
        return res;
    }

    // 开启线程池
    void start(int initThreadSize = std::thread::hardware_concurrency())
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

    // 禁止对象构造
    ThreadPool(const ThreadPool &) = delete;
    // 禁止对象拷贝
    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    // 定义线程函数
    void threadFunc(int threadId)
    {
        auto lastTime = std::chrono::high_resolution_clock().now();
        for (;;)
        {
            Task task;
            {
                // 获取锁
                std::unique_lock<std::mutex> lock(taskQueMtx_);

                std::cout << std::this_thread::get_id() << "尝试获取任务" << std::endl;
                // 双重判断，对应pool先拿到锁，形成死锁
                while (taskQue_.size() == 0)
                {
                    // 没有任务且已经析构，销毁线程池对象
                    if (!isPoolRunning_)
                    {
                        // 把线程对象从线程容器里删除
                        threads_.erase(threadId);
                        std::cout << "threadid:" << std::this_thread::get_id() << " exit!" << std::endl;
                        exitCond_.notify_all();
                        return;
                    }
                    if (poolMode_ == PoolMode::MODE_CACHED)
                    {
                        // 每一秒返回一次
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
                    else
                    {
                        // 等待notEmpty条件 这里一直等待
                        notEmpty_.wait(lock);
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
                task();
            }
            // 处理完了，空闲线程++
            idleThreadSize_++;
            lastTime = std::chrono::high_resolution_clock().now();
        }
    }

    // 检查pool运行状态
    bool checkRunningState() const
    {
        return isPoolRunning_;
    }

private:
    // std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    std::unordered_map<int, std::unique_ptr<Thread>> threads_; // 线程列表
    int initThreadSize_;                                       // 初始线程数量
    int threadMaxSizeThreshold_;                               // 线程数量上限
    std::atomic_int curThreadSize_;                            // 当前线程总数量

    // std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列，用智能指针保证用户任务的管理
    using Task = std::function<void()>;
    std::queue<Task> taskQue_;
    std::atomic_int taskSize_; // 任务数量
    int taskQueThreshold_;     // 任务队列上限阈值

    std::mutex taskQueMtx_;            // 保证任务队列线程安全
    std::condition_variable notFull_;  // 任务队列不满
    std::condition_variable notEmpty_; // 任务队列不空
    std::condition_variable exitCond_; // 等待线程资源回收

    PoolMode poolMode_;              // 线程池模式
    std::atomic_bool isPoolRunning_; // 线程池启动状态
    std::atomic_int idleThreadSize_; // 空闲线程数量
};
