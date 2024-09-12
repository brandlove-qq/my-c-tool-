#pragma once

namespace zhb {
//线程池返回的相关错误代码
typedef enum threadpool_errorcode
{
    Threadpool_Success = 0,     // 成功
    Threadpool_KeyExist,    // key值已经存在[一般出现在添加中]
    Threadpool_KeyInvalid,  // 无效的key值
    Threadpool_KeyNotExist, // key值不存在
    Threadpool_KeySlotIsFull, //key值槽已满
    Threadpool_PostTaskFailed,  //添加任务失败
    Threadpool_Not_Initial //未初始化
}TP_CODE;

static const char *threadpool_errorcode_string[] =
{
    "Success, Code = Threadpool_Success.",     // 成功
    "Key has been existed,Error Code = Threadpool_KeyExist.",    // key值已经存在[一般出现在添加中]
    "Key is invalid, Error Code = Threadpool_KeyInvalid.",  // 无效的key值
    "Key is not Exist, Error Code = Threadpool_KeyNotExist.", // key值不存在
    "key slots is full. Error Code = Threadpool_KeySlotIsFull",  //key值槽已满
    "Threadpool post task failed. Error Code = Threadpool_PostTaskFailed.",  //添加任务失败
    "Threadpool_Not_Initial."
};
}
