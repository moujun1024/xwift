#ifndef XWIFT_STDLIB_CONCURRENCY_ASYNC_H
#define XWIFT_STDLIB_CONCURRENCY_ASYNC_H

#include "xwift/AST/Type.h"
#include "xwift/Basic/LLVM.h"
#include <memory>
#include <functional>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace xwift {

// Task type for async operations
template<typename T>
class Task {
private:
    std::future<T> future;
    
public:
    Task(std::function<T()> func) {
        std::packaged_task<T> task(func);
        future = task.get_future();
    }
    
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    
    T await() {
        return future.get();
    }
    
    bool isReady() const {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }
};

// Actor class for concurrent operations
template<typename T>
class Actor {
private:
    T state;
    std::queue<std::function<void()>> messageQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::thread actorThread;
    bool running;
    
    void processMessages() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return !messageQueue.empty() || !running; });
            
            if (!running && messageQueue.empty()) {
                break;
            }
            
            auto message = messageQueue.front();
            messageQueue.pop();
            lock.unlock();
            
            message();
        }
    }
    
public:
    Actor(const T& initialState) : state(initialState), running(true) {
        actorThread = std::thread(&Actor::processMessages, this);
    }
    
    ~Actor() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            running = false;
            condition.notify_all();
        }
        
        if (actorThread.joinable()) {
            actorThread.join();
        }
    }
    
    void send(std::function<void()> message) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            messageQueue.push(message);
        }
        condition.notify_one();
    }
    
    template<typename F>
    void send(F&& message) {
        send(std::function<void()>(std::forward<F>(message)));
    }
    
    const T& getState() const {
        return state;
    }
    
    template<typename F>
    void modifyState(F&& modifier) {
        state = modifier(state);
    }
};

}

#endif