#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

/**
 * @brief A thread-safe queue implementation using std::mutex and std::condition_variable.
 * 
 * Strategy:
 * 1. Mutual Exclusion: A std::mutex protects the underlying std::queue. Every access (push/pop) 
 *    is wrapped in a std::lock_guard or std::unique_lock.
 * 2. Signaling: A std::condition_variable is used to notify the consumer thread when new 
 *    data is available (push). This prevents the consumer from busy-waiting.
 * 3. Real-time Considerations: While this uses mutexes (which can block), the lock duration 
 *    is minimized to simple pointer/data copies to reduce jitter in the audio pipeline.
 */
template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() = default;

    // Delete copy/assignment for safety
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * @brief Pushes an item to the queue and notifies one waiting thread.
     */
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(value));
        }
        m_condVar.notify_one();
    }

    /**
     * @brief Waits until data is available and pops it.
     */
    void waitAndPop(T& value) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condVar.wait(lock, [this] { return !m_queue.empty(); });
        value = std::move(m_queue.front());
        m_queue.pop();
    }

    /**
     * @brief Tries to pop data immediately without waiting.
     */
    bool tryPop(T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    mutable std::mutex m_mutex;
    std::queue<T> m_queue;
    std::condition_variable m_condVar;
};

#endif // THREAD_SAFE_QUEUE_H
