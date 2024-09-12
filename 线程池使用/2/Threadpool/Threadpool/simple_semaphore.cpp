#include "simple_semaphore.h"

bool zhb::simple_semaphore::wait(int ms)
{
    bool ret = true;
    std::unique_lock<std::mutex>    lck(m_mx_);
    auto count = m_count_.fetch_sub(1, std::memory_order_acq_rel);
    if(count <= 0)
    {
        if(ms <= 0)
        {
            m_con_.wait(lck, [this](){ return m_wakeup_.load(std::memory_order_relaxed) > 0;});
            ret = true;
        }
        else
        {
            ret = m_con_.wait_for(lck, std::chrono::milliseconds(ms), [this](){ return m_wakeup_.load(std::memory_order_relaxed) > 0;});
        }
    }

    m_wakeup_.fetch_sub(1, std::memory_order_acq_rel);

    return ret;
}

void zhb::simple_semaphore::singal()
{
    std::unique_lock<std::mutex>    lck(m_mx_);
    int count = m_count_.fetch_add(1, std::memory_order_acq_rel);
    if(count < 0)
    {
        m_wakeup_.fetch_add(1, std::memory_order_acq_rel);
        m_con_.notify_one();
    }
}
