#pragma once
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace zhb {
class simple_semaphore
{
public:
    simple_semaphore() = default;

    /*!
     * \brief wait 等待
     * \param ms 等待毫秒数，当ms=-1时，无限等待直到唤醒
     * \return 超时false，正常true
     */
    bool wait(int ms = -1);

    /*!
     * \brief singal 唤醒一个信号
     */
    void singal();

private:
    std::mutex  m_mx_;
    std::condition_variable m_con_;
    std::atomic_int m_count_{0};
    std::atomic_int m_wakeup_{0};
};
}


