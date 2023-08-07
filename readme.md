# ThreadPool 线程池

- 基于 c++17 实现的线程池项目

## 版本

- camke 3.22.1

- c++17

## Usage

- cd ./src && touch main.cpp

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

- cd .. && mkdir build && cd ./build && cmake --build . --config Debug --target all -j 4 --
- cd ../bin && ./ThreadPool

