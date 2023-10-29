// Thread safe queue
// Defines a thread safe queue for use in the server and client.
// ---------------------------------------------------------------------------
#pragma once

#include <mutex>
#include <deque>

namespace meow::net {

    template <typename T>
    class TSQueue {
    protected:
        std::deque<T> queue;
        std::mutex muxQueue;
        std::mutex muxBlocking;
        std::condition_variable cvBlocking;
    public:
        TSQueue() = default;
        TSQueue(const TSQueue<T> &) = delete;
        virtual ~TSQueue() { clear(); }

        const T &front() {
            std::scoped_lock lock(muxQueue);
            return queue.front();
        }

        const T &back() {
            std::scoped_lock lock(muxQueue);
            return queue.back();
        }

        T pop_front() {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(queue.front());
            queue.pop_front();
            return t;
        }

        T pop_back() {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(queue.back());
            queue.pop_back();
            return t;
        }

        void emplace_front(const T &t) {
            std::scoped_lock lock(muxQueue);
            queue.emplace_front(std::move(t));

            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }
        
        void emplace_back(const T &t) {
            std::scoped_lock lock(muxQueue);
            queue.emplace_back(std::move(t));

            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }

        bool empty() {
            std::scoped_lock lock(muxQueue);
            return queue.empty();
        }

        size_t size() {
            std::scoped_lock lock(muxQueue);
            return queue.size();
        }

        void clear() {
            std::scoped_lock lock(muxQueue);
            queue.clear();
        }

        void wait() {
            while (empty()) {
                std::unique_lock<std::mutex> lock(muxBlocking);
                cvBlocking.wait(lock);
            }
        }
    };

} // namespace meow::net