#include "Precompile.h"
#include "RecvThread.h"
#include "WebCrawler.h"
#include "Socket.h"
// @brief 构造器
RecvThread::RecvThread(
    Socket *socket ///< [in] 套接字
    ) : m_socket(socket)
{
}
//析构器
RecvThread::~RecvThread(){
    delete m_socket;
}
/// @brief 线程处理函数
/// @note 根据接收线程的任务实现基类中的纯虚函数
void *RecvThread::run(void)
{
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "接收线程开始");

    // 通过套接字接收超文本传输协议响
    // 应，根据其执行情况停止抓取任务
    g_app->stopJob(m_socket->recvResponse());
    //对象自毁
    delete this;
    // 记录调试日志
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "接收线程终止");
    // 终止线程
    return NULL;
}