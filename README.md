## 项目描述
本项目是基于Reactor+线程池的高性能 C++ 服务器，用于处理多个客户端的并发连接和请求。实现了客户端连接的管理，包括超时断开连接和防止过多客户端连接等功能

## 主要技术
• 使用Reactor模型+线程池进行客户端连接任务管理

• 使用RAII方法管理内存资源

• 应用了事件驱动的编程模型，通过事件回调机制处理客户端连接、数据接收和数据发送等事件

• 使用 std::mutex 和 std::condition_variable 来实现线程同步和互斥

• 线程池方面使用 std::forward 和完美转发来传递任务和参数，提高效率
