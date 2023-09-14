# ThreadPool 线程池

- 基于 c++17 实现的线程池项目

## 版本

- camke 3.22.1

- c++11

## Usage

#### _基于 v1 版本_

- cd ./src/v1 && touch main.cpp

- cd .. && mkdir build && cd ./build && cmake --build . --config Debug --target all -j 4 --

  ```
  // main.cpp

  #include "thread_pool.hpp"
  #include <iostream>
  using namespace std;

  class MyTask : public Task {
  public:
      void run() { cout<<"hello world!";}
  }

  int main(){
      TreadPool pool;
      pool.setMode(PoolMode::MODE_CACHED);
      pool.start(4);

      pool.submitTask(std::make_shared<MyTask>());
      pool.submitTask(std::make_shared<MyTask>());
      pool.submitTask(std::make_shared<MyTask>());
      getchar();
  }
  ```

- cd ../bin && ./ThreadPoolv1

#### _基于 v2 版本_

- cd ./src/v2 && touch main.cpp

- cd .. && mkdir build && cd ./build && cmake --build . --config Debug --target all -j 4 --

  ```
  // main.cpp

  #include <iostream>
  #include "threadpool.hpp"
  #include <chrono>
  using namespace std;

  int add(int a, int b)
  {
      std::chrono::seconds(2);
      return a + b;
  }

  int sub(int a, int b, int c)
  {
      return a - b - c;
  }
  int main()
  {
      ThreadPool pool;
      pool.setMode(PoolMode::MODE_CACHED);
      pool.start();
      future<int> r1 = pool.submitTask(add, 77, 96);
      future<int> r2 = pool.submitTask(sub, 77, 96, 86);
      cout << r1.get() << endl;
      cout << r2.get() << endl;
      getchar();
      return 0;
  }
  ```

- cd ../bin && ./ThreadPool
