#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <memory>

constexpr uint8_t MIN_WAIT_TASK_NUM = 30; //调整线程池中线程个数的阈值
constexpr uint8_t DEFAULT_THREAD_VARY = 10;//线程池在调整线程数量时每次增加或减少的线程数量

class ThreadPool{
private:
    size_t min_thr_num, max_thr_num;// 线程池中最小、最大线程个数
    size_t live_thr_num;//当前存活线程个数
    size_t busy_thr_num;//忙状态线程个数
    size_t wait_exit_thr_num;//要销毁的线程个数
    size_t queue_max_size;//任务队列的最大值

    bool shutdown;//线程池的使用状态

    std::vector<std::shared_ptr<std::thread>> threads;//存储线程指针
    std::shared_ptr<std::thread> adjust_thread_ptr;//管理线程: 负责监视线程池中的任务情况，并根据需要调整线程数量

    std::mutex mutex_; //互斥量
    std::condition_variable not_empty_;//队不空的时候 通知等待任务的线程
    std::condition_variable not_full_;//当队满时 添加任务的线程阻塞

    void executeWithArgs(std::function<void()>& taskWithArgs) {
        taskWithArgs();
    }

public:

    std::queue<std::function<void()>> task_queue;
    ThreadPool(size_t min_thr_num, size_t max_thr_num, size_t queue_max_size)
        : min_thr_num(min_thr_num), max_thr_num(max_thr_num), queue_max_size(queue_max_size),
          shutdown(false), live_thr_num(min_thr_num), busy_thr_num(0), wait_exit_thr_num(0) {
        threads.reserve(max_thr_num);

        for (size_t i = 0; i < min_thr_num; ++i) {
            threads.emplace_back(std::make_shared<std::thread>(&ThreadPool::threadpool_thread, this));
        }

        adjust_thread_ptr = std::make_shared<std::thread>(&ThreadPool::adjust_thread, this);
    }

    ~ThreadPool() {
        shutdown = true;
        not_empty_.notify_all();//唤醒所有等待 not_empty_ 条件变量的线程
        adjust_thread_ptr->join();//等待管理线程完成工作
        for (auto& thread : threads) 
            thread->join();//等待所有工作线程结束
        std::cout << "线程池析构函数调用完成" << std::endl;
    }

    template<typename F, typename... Args>
    void add_task(F&& f, Args&&... args) {//向线程池添加任务
        {//创建一个局部作用域以确保在作用域结束时 lock 对象被销毁
            std::unique_lock<std::mutex> lock(mutex_);
            while (task_queue.size() >= queue_max_size && !shutdown) {//防止假唤醒
                not_full_.wait(lock);//阻塞等待并释放 lock 所持有的锁 直到条件变量not_full满足再次抢到锁
            }
            if (shutdown) return;
            task_queue.emplace(std::function<void()>([f=std::forward<F>(f),args=std::make_tuple(std::forward<Args>(args)...)] {
                std::apply(f, args);
            }));
        }
        not_empty_.notify_one();//唤醒一个等待not_empty的线程
    }

    void threadpool_thread(){//线程池中工作线程执行的函数
        while (true) {//不断地从任务队列中取出任务并执行
            std::function<void()> task;
            {//创建一个局部作用域以确保在作用域结束时 lock 对象被销毁
                std::unique_lock<std::mutex> lock(mutex_);
                while (task_queue.empty() && !shutdown) {
                    not_empty_.wait(lock);
                }
                if (shutdown) return;
                //取走一个任务
                task = std::move(task_queue.front());
                task_queue.pop();
            }
            not_full_.notify_one(); //通知一个线程任务队列不满
            ++busy_thr_num;
            executeWithArgs(task);//执行任务
            --busy_thr_num;
        }
    }

    void adjust_thread(){
         while (!shutdown) {
            //每隔10秒执行一次调整操作
            std::this_thread::sleep_for(std::chrono::seconds(10));

            std::unique_lock<std::mutex> lock(mutex_);
            size_t queue_size = task_queue.size();
            size_t busy_thr = busy_thr_num;
            size_t live_thr = live_thr_num;
            lock.unlock();

            //任务数大于阈值 存活的线程数少于最大线程个数时 创建新线程
            if (queue_size >= MIN_WAIT_TASK_NUM && live_thr < max_thr_num) {
                std::cout << "增加新的线程" << std::endl;
                lock.lock();
                for (size_t i = 0; i < max_thr_num && live_thr_num < max_thr_num; ++i) {
                    ++live_thr_num;
                    ++busy_thr;
                    threads.emplace_back(std::make_shared<std::thread>(&ThreadPool::threadpool_thread, this));
                }
                lock.unlock();
            }
            //当忙线程*2 小于存活的线程数 且存活的线程数 大于 最小线程数时 销毁多余的空闲线程
            if (2 * busy_thr < live_thr && live_thr > min_thr_num) {
                std::cout << "销毁空闲线程" << std::endl;
                lock.lock();
                wait_exit_thr_num = DEFAULT_THREAD_VARY;
                lock.unlock();
                not_empty_.notify_all();
            }
        }
    }
};
