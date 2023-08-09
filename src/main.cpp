#include <iostream>
#include "thread_pool.hpp"
#include <chrono>
using namespace std;

class MyTask : public Task
{
public:
    MyTask(int begin, int end) : begin_(begin),
                                 end_(end) {}
    Any run()
    {
        cout << "tid:" << this_thread::get_id() << " begin" << endl;
        // this_thread::sleep_for(chrono::seconds(2));
        int sum = 0;
        for (int i = begin_; i < end_; i++)
        {
            sum += i;
        }
        cout << "tid:" << this_thread::get_id() << " end" << endl;
        return sum;
    }

private:
    int begin_;
    int end_;
};

int main()
{
    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(4);
        Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 10000));
        int sum1 = res1.get().cast_<int>();
        Result res2 = pool.submitTask(std::make_shared<MyTask>(10001, 20000));
        int sum2 = res2.get().cast_<int>();
        Result res3 = pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        int sum3 = res3.get().cast_<int>();
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        pool.submitTask(std::make_shared<MyTask>(20001, 100000));
        cout << sum1 + sum2 + sum3 << endl;
    }
    getchar();
    return 0;
}