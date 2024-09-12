#include "threadpool_cpp11.h"
using namespace zhb;

zhb::zhb_threadpool::threadpool_cpp11::threadpool_cpp11(size_t thread_nums)
    : m_workers(std::max<size_t>(thread_nums, 2))
{
    size_t index{0};
    for(auto &worker : m_workers)
    {
        worker.reset(new task_worker());
        worker->run(index);
        m_index2key.emplace(index, invalidKey);
        ++index;
    }

    m_worker_size = m_workers.size();
}

zhb::zhb_threadpool::threadpool_cpp11::~threadpool_cpp11()
{
    stop();
}

void zhb::zhb_threadpool::threadpool_cpp11::stop()
{
    for(auto &worker : m_workers)
    {
        worker->stop();
    }

    std::lock_guard<std::mutex> locker(m_mutex_worker);
    m_index2key.clear();
    m_key2index.clear();
}

TP_CODE zhb::zhb_threadpool::threadpool_cpp11::set_key(size_t key)
{
    if(invalidKey == key)
    {
        return Threadpool_KeyInvalid;
    }

    std::lock_guard<std::mutex> locker(m_mutex_worker);

    //查询key值是否已经存在
    if(m_key2index.end() != m_key2index.find(key))
    {
        return Threadpool_KeyExist;
    }

    //查询没有被占位的线程
    size_t index = m_worker_size;
    for(size_t idx = 0; idx < m_worker_size; ++idx)
    {
        if(invalidKey == m_index2key.at(idx))
        {
            index = idx;
            break;
        }
    }

//    if(m_worker_size == index)
//    {
//        //扩展线程池
//        m_workers.emplace_back(new task_worker());
//        m_workers[index]->run(index);

//        //更新key与index的映射关系
////        m_key2index.emplace(std::make_pair(key, index));
////        m_index2key.emplace(std::make_pair(index, key));
//        m_key2index.emplace(key, index);
//        m_index2key.emplace(index, key);
//    }
//    else
//    {
//        //更新key与index的映射关系
//        m_key2index.emplace(key, index);
//        m_index2key.at(index) = key;
//    }

    if(m_worker_size == index)
    {
        return Threadpool_KeySlotIsFull;
    }

    //更新key与index的映射关系
    m_key2index.emplace(key, index);
    m_index2key.at(index) = key;

    return Threadpool_Success;
}

TP_CODE zhb::zhb_threadpool::threadpool_cpp11::delete_key(size_t key)
{
    if(invalidKey == key)
    {
        return Threadpool_KeyInvalid;
    }

    std::lock_guard<std::mutex> locker(m_mutex_worker);
    auto b = m_key2index.find(key);
    if(m_key2index.end() != b)
    {
        //找到Key值，更新映射关系
        m_index2key.at(b->second) = invalidKey;
        m_key2index.erase(b);
        return Threadpool_Success;
    }

    return Threadpool_KeyNotExist;
}


