## introduction

Linux平台下的小型HTTP服务器


## programming model

* epoll
* non-blocking I/O
* thread-pool

## compile and run (for now only support Linux2.6+)

please make sure you have [cmake](https://cmake.org/) installed.
```
mkdir build && cd build
cmake .. && make
cd .. && ./build/buffalo -c buffalo.conf