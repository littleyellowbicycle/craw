#include "Precompile.h"
#include "WebCrawler.h"

MultiIo::MultiIo(void)
{
    m_epoll = epoll_create1(0);
    if (m_epoll == -1)
    {
        g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,
                            "epoll create false %s", strerror(errno));
    }
}
MultiIo::~MultiIo(void){
    close(m_epoll);
}
bool MultiIo::add(
    int fd,            ///< [in] 发生输入输出事件的文件描述符
    epoll_event &event ///< [in] 事件描述结构
    ) const
{
    if (epoll_ctl(m_epoll, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                            "epoll add fasle %s", strerror(errno));
        return false;
    }
    return true;
}

bool MultiIo::del(
    int fd,            ///< [in] 发生输入输出事件的文件描述符
    epoll_event &event ///< [in] 事件描述结构
    ) const
{
    if (epoll_ctl(m_epoll, EPOLL_CTL_DEL, fd, &event) == -1)
    {
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                            "epoll add fasle %s", strerror(errno));
        return false;
    }
    return true;
}

int MultiIo::wait(
    epoll_event events[], ///< [out] 事件描述结构数组
    int max,              ///< [in]  事件描述结构数组容量
    int timeout           ///< [in]  超时毫秒数，0立即超时，-1无限超时
    ) const
{
    int fds = epoll_wait(m_epoll, events, max, timeout);
    // 若发生除被信号中断以外的错误
    if (fds == -1 && errno != EINTR)
        // 记录警告日志
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                            "epoll_wait: %s", strerror(errno));

    // 返回处于就绪状态的文件描述符数，超时返回0，失败返回-1
    return fds;
}