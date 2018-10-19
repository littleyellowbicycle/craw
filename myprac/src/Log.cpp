#include "WebCrawler.h"
#include "Precompile.h"
#include "time.h"

void Log::printf(int level,          ///< [in] 日志等级
                char const *file,   ///< [in] 源码文件
                int line,           ///< [in] 源码行号
                char const *format, ///< [in] 格式化串
                ...) const
{
    if (level >= g_app->m_cfg.m_logLevel)
    {
        char printTime[32];
        time_t tm=time(NULL);
        
        strftime(printTime,sizeof(printTime),"%y %m %d  %h %m %s",localtime(&tm));
        // 打印日志头：
        // [日期时间][日志等级][pid=进程标识 tid=线程标识][文件:行号]
        fprintf(stdout, "[%s] [%s] [pid=%d] [tid=%lu] [%s] [%d] "
            ,printTime, s_levels[level],getpid(),pthread_self(), file, line);
        va_list ap;
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
        fprintf(stdout, "\n\n");
    }
    if(level >= LEVEL_ERR)
        exit(EXIT_FAILURE);
}

char const* Log::s_levels[] = {"dbg", "inf", "war", "err", "crt"};

