#pragma once
#include "concurrentqueue.h"

namespace zhb
{
    template<typename T>
    class ConcurrentQueueSafeCount
    {
    public:
        ConcurrentQueueSafeCount() = default;
        ~ConcurrentQueueSafeCount() = default;

        ConcurrentQueueSafeCount(size_t capacity)
            : m_size_(0)
            , m_queque_(capacity)
        {

        }

    public:
        // Enqueues a single item (by moving it, if possible).
        // Allocates memory if required. Only fails if memory allocation fails (or implicit
        // production is disabled because Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE is 0,
        // or Traits::MAX_SUBQUEUE_SIZE has been defined and would be surpassed).
        // Thread-safe.
        bool enqueue(T &&data)
        {
            bool b = m_queque_.enqueue(std::move(data));
            if (b)
            {
                m_size_.fetch_add(1, std::memory_order_acq_rel);
                //m_size_++;
            }

            return b;
        }

        // Enqueues several items.
        // Allocates memory if required. Only fails if memory allocation fails (or
        // implicit production is disabled because Traits::INITIAL_IMPLICIT_PRODUCER_HASH_SIZE
        // is 0, or Traits::MAX_SUBQUEUE_SIZE has been defined and would be surpassed).
        // Note: Use std::make_move_iterator if the elements should be moved instead of copied.
        // Thread-safe.
        template<typename It>
        bool enqueue_bulk(It itemFirst, size_t count)
        {
            bool b = m_queque_.enqueue_bulk(itemFirst, count);
            if (b)
            {
                m_size_.fetch_add(count, std::memory_order_acq_rel);
                //m_size_ += count;
            }

            return b;
        }

        // Attempts to dequeue from the queue.
        // Returns false if all producer streams appeared empty at the time they
        // were checked (so, the queue is likely but not guaranteed to be empty).
        // Never allocates. Thread-safe.
        bool try_dequeue(T& item)
        {
            if (m_size_.load(std::memory_order_relaxed) == 0)
            {
                return false;
            }

            bool b = m_queque_.try_dequeue(item);
            if (b)
            {
                m_size_.fetch_sub(1, std::memory_order_acq_rel);
                //m_size_--;
            }

            return b;
        }

        // Attempts to dequeue several elements from the queue.
        // Returns the number of items actually dequeued.
        // Returns 0 if all producer streams appeared empty at the time they
        // were checked (so, the queue is likely but not guaranteed to be empty).
        // Never allocates. Thread-safe.
        template<typename It>
        size_t try_dequeue_bulk(It itemFirst, size_t max)
        {
            size_t size = m_size_.load(std::memory_order_relaxed);
            size_t count = max > size ? size : max;
            if (count > 0)
            {
                count = m_queque_.try_dequeue_bulk(itemFirst, count);
                m_size_.fetch_sub(count, std::memory_order_acq_rel);
                //m_size_ -= count;
            }

            return count;
        }

        size_t size_approx() const
        {
            return m_size_.load(std::memory_order_relaxed);
        }

    private:
        ConcurrentQueueSafeCount(const ConcurrentQueueSafeCount&) = delete;
        ConcurrentQueueSafeCount& operator=(const ConcurrentQueueSafeCount&) = delete;
    private:
        std::atomic_uint32_t	m_size_{ 0 };
        moodycamel::ConcurrentQueue<T>	m_queque_{1024};
    };
}
