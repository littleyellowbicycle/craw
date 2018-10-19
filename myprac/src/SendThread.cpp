#include "Precompile.h"
#include "SendThread.h"
#include "WebCrawler.h"

/// @brief 线程处理函数
/// @note 根据发送线程的任务实现基类中的纯虚函数
void *SendThread::run(void)
{
    // 记录调试日志
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "发送线程开始");
    while(1){
        // 启动一个抓取任务
        g_app->startJob ();
        // 记录调试日志
        g_app->m_log.printf (Log::LEVEL_DBG, __FILE__, __LINE__,
                "发送线程终止");
        // 终止线程
        return NULL;        
    }
}