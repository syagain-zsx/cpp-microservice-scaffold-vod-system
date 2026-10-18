#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

namespace svc_transcode {
    //实现一个简单的线程池，进行视频转码处理
    const size_t MAX_THREADS = 1;
    using Task = std::function<void()>;
    class SvcWorker {
        public:
            using ptr = std::shared_ptr<SvcWorker>;
            SvcWorker(size_t thread_count = MAX_THREADS);
            ~SvcWorker();
            void addTask(Task &&task);
            void stop();
        private:
            void threadEntry();
        private:
            std::mutex _mtx;
            std::condition_variable _cv;
            bool _is_stop;
            size_t _thread_count;
            std::queue<Task> _tasks;
            std::vector<std::thread> _threads;
    };

}