## 项目描述
本项目是由C++实现的web服务器，采用高效的Reactor事件分发模式，支持高并发的http请求 

## 主要技术
• 使用Reactor模型+线程池进行客户端连接任务管理

• 使用RAII方法管理内存资源

• 使用eventfd事件进行线程唤醒

• 使用 std::mutex 和 std::condition_variable 来实现线程同步和互斥

• 使用 std::forward 和完美转发来传递任务和参数，提高效率
