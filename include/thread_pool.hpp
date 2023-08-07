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

// Any类型：接收任意数据类型 这是一个模板类
class Any
{
public:
    Any() = default;
    ~Any() = default;
    // 左值引用
    Any(const Any &) = delete;
    Any &operator=(const Any &) = delete;
    // 右值引用，对右值操作
    Any(Any &&) = default;
    Any &operator=(Any &&) = default;
    template <typename T>
    Any(T data) : base_(std::make_unique<Derive<T>>(data)) {}

    // 把any对象里的data取出来
    template <typename T>
    T cast_()
    {
        // 基类转化为派生类指针
        Derive<T> *pd = dynamic_cast<Derive<T> *>(base_.get());
        // 用户返回类型和接受类型不一样，转化失败，抛异常提示
        if (pd == nullptr)
        {
            throw "type is incompatible";
        }
        return pd->data_;
    }

private:
    // 基类类型
    class Base
    {
    public:
        virtual ~Base() = default; // default->{}
    };
    // 派生类型
    template <typename T>
    class Derive : public Base
    {
    public:
        Derive(T data) : data_(data) {}

        T data_;
    };

private:
    // 定义一个基类指针
    std::unique_ptr<Base> base_;
};

// 信号量类
class Semaphore
{
public:
    Semaphore(int limit = 0) : resLimit_(limit) {}
    ~Semaphore() = default;

    // 获取一个信号量资源
    void wait()
    {
        std::unique_lock<std::mutex> lock(mtx_);
        // 等待信号量有资源，无资源，阻塞当前线程
        cond_.wait(lock, [&]() -> bool
                   { return resLimit_ > 0; });
        resLimit_--;
    }

    // 增加一个信号量资源
    void post()
    {
        std::unique_lock<std::mutex> loc(mtx_);
        resLimit_++;
        // 通知
        cond_.notify_all();
    }

private:
    int resLimit_;
    std::mutex mtx_;
    std::condition_variable cond_;
};

class Task;
// 任务完成后的返回值
class Result
{
public:
    Result(std::shared_ptr<Task> task, bool isValid = true);
    ~Result() = default;

    // 获取任务执行完的返回值
    void setVal(Any any);

    Any get();

private:
    Any any_;                    // 存储任务的返回值
    Semaphore sem_;              // 线程通信信号量
    std::shared_ptr<Task> task_; // 指向对应任务对象，目的是拿到task，避免线程完成任务后，task销毁
    std::atomic_bool isValid_;   // 是否有效
};

// 任务抽象基类
// 用户可以自定义任意任务类型，从Task继承，重写run方法
class Task
{
public:
    Task();
    ~Task() = default;
    void exec();
    void setResult(Result *res);
    virtual Any run() = 0; // 没有返回值的
private:
    Result *result_; // 不能用智能指针，因为Task已经为智能指针
};

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
    using ThreadFunc = std::function<void()>;
    Thread(ThreadFunc func);
    ~Thread();

    // 启动线程
    void start();

private:
    ThreadFunc func_;
};
/**
 * example:
 * TreadPool pool;
 * pool.start(4);
 *
 * class MyTask : public Task {
 *  public:
 *      void run() {// code...}
 * }
 *
 * pool.submitTask(std::make_shared<MyTask>());
 */

// 线程池类型
class ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    // 设置线程池工作模式
    void setMode(PoolMode mode);

    // 设置task任务上限阈值
    void setTaskQueThreshold(int threshold);

    // 设置cached模式线程上限阈值
    void setThreadSizeThreshold(int threshold);

    // 提交任务
    Result submitTask(std::shared_ptr<Task> sp);

    // 开启线程池
    void start(int initThreadSize = 4);

    // 禁止对象构造
    ThreadPool(const ThreadPool &) = delete;
    // 禁止对象拷贝
    ThreadPool &operator=(const ThreadPool &) = delete;

private:
    // 定义线程函数
    void threadFunc();

    // 检查pool运行状态
    bool checkRunningState() const;

private:
    std::vector<std::unique_ptr<Thread>> threads_; // 线程列表
    int initThreadSize_;                           // 初始线程数量
    int threadMaxSizeThreshold_;                   // 线程数量上限
    std::atomic_int curThreadSize_;                // 当前线程总数量

    std::queue<std::shared_ptr<Task>> taskQue_; // 任务队列，用智能指针保证用户任务的管理
    std::atomic_int taskSize_;                  // 任务数量
    int taskQueThreshold_;                      // 任务队列上限阈值

    std::mutex taskQueMtx_;            // 保证任务队列线程安全
    std::condition_variable notFull_;  // 任务队列不满
    std::condition_variable notEmpty_; // 任务队列不空

    PoolMode poolMode_;              // 线程池模式
    std::atomic_bool isPoolRunning_; // 线程池启动状态
    std::atomic_int idleThreadSize_; // 空闲线程数量
};
