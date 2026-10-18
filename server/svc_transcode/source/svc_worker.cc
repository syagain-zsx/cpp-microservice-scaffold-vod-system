#include "svc_worker.h"


namespace svc_transcode {
    SvcWorker::SvcWorker(size_t thread_count): _thread_count(thread_count), _is_stop(false) {
        for (size_t i = 0; i < _thread_count; ++i) {
            _threads.emplace_back(std::thread(&SvcWorker::threadEntry, this));
        }
    }
    SvcWorker::~SvcWorker() {
        for (auto &t : _threads) {
            t.join();
        }
    }
    void SvcWorker::addTask(Task &&task) {
        std::unique_lock<std::mutex> lock(_mtx);
        _tasks.push(std::move(task));
        _cv.notify_one();
    }
    void SvcWorker::stop() {
        std::unique_lock<std::mutex> lock(_mtx);
        _is_stop = true;
        _cv.notify_all();
    }

    void SvcWorker::threadEntry() {
        while(_is_stop == false) {
            // 加锁
            Task task;
            {
                std::unique_lock<std::mutex> lock(_mtx);
                // 判断队列中是否有任务，没有任务， 且_is_stop为false, 则等待
                _cv.wait(lock, [this]() { return !_tasks.empty() && _is_stop == false; });
                // 有任务就取出任务，执行任务
                task = std::move(_tasks.front());
                _tasks.pop();
            }
            if (task) task();
        }
    }
}