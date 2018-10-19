#include "Precompile.h"
//#include "UrlQueues.h"
#include "UrlFilter.h"
#include "WebCrawler.h"
//构造器
UrlQueues::UrlQueues(UrlFilter &filter) : m_filter(filter)
{
    //初始化原始统一资源定位符队列互斥锁
    pthread_mutex_init(&m_rawUrlMutex, NULL);
    //初始化原始统一资源定位符队列非空变量
    pthread_cond_init(&m_rawUrlNoEmpty, NULL);
    //初始化原始统一资源定位符队列非满变量
    pthread_cond_init(&m_rawUrlNoFull, NULL);
    //初始化解析统一资源定位符队列互斥锁
    pthread_mutex_init(&m_dnsUrlMutex, NULL);
    //初始化解析统一资源定位符队列非空变量
    pthread_cond_init(&m_dnsUrlNoEmpty, NULL);
    //初始化解析统一资源定位符队列非满变量
    pthread_cond_init(&m_dnsUrlNoEmpty, NULL);
}
//析构器
UrlQueues::~UrlQueues()
{
    //销毁原始统一资源定位符队列互斥锁
    pthread_mutex_destroy(&m_rawUrlMutex);
    //销毁原始统一资源定位符队列非空变量
    pthread_cond_destroy(&m_rawUrlNoEmpty);
    //销毁原始统一资源定位符队列非满变量
    pthread_cond_destroy(&m_rawUrlNoFull);
    //销毁解析统一资源定位符队列互斥锁
    pthread_mutex_destroy(&m_dnsUrlMutex);
    //销毁解析统一资源定位符队列非空变量
    pthread_cond_destroy(&m_dnsUrlNoEmpty);
    //销毁解析统一资源定位符队列非满变量
    pthread_cond_destroy(&m_dnsUrlNoEmpty);
}
//压入原始统一资源定位符
void UrlQueues::pushRawUrl(RawUrl const &rawUrl)
{
    pthread_mutex_lock(&m_rawUrlMutex);
    //调用过滤器判断此定位符是否以处理
    if (m_filter.exist(rawUrl.m_strUrl))
        g_app->m_log.printf(Log::LEVEL_DBG, __FILE__,
                            __LINE__, "不再处理已处理过的统一资源定位符\"%s\"",
                            rawUrl.m_strUrl.c_str());
    //未处理过调用统一资源定位符处理插件
    else if (g_app->m_pluginMngr.invokeUrlPlugins(const_cast<RawUrl *>(&rawUrl)))
    {
        // 若配置器中的原始统一资源定位符队列最大容量有效且到限
        while (0 <= g_app->m_cfg.m_maxRawUrls &&
               (size_t)g_app->m_cfg.m_maxRawUrls <= m_rawUrlQueue.size())
            // 等待原始统一资源定位符队列非满条件变量
            pthread_cond_wait(&m_rawUrlNoFull, &m_rawUrlMutex);
        // 向原始统一资源定位符队列压入原始统一资源定位符
        m_rawUrlQueue.push_back(rawUrl);
            g_app->m_log.printf (Log::LEVEL_DBG, __FILE__, __LINE__,
                        "原始统一资源定位符\"%s\"入队", rawUrl.m_strUrl.c_str ());

    // 若原始统一资源定位符队列由空变为非空,唤醒等待原始统一资源定位符队列非空条件变量的线程
    if(m_rawUrlQueue.size () == 1)
       pthread_cond_signal (&m_rawUrlNoEmpty);
    }
    // 解锁原始统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_rawUrlMutex);
}
//弹出统一资源定位符
RawUrl UrlQueues::popRawUrl(void)
{
    pthread_mutex_lock(&m_rawUrlMutex);
    //若原始统一资源定位符队列非空，弹出队列首元素
    while (m_rawUrlQueue.empty())
        pthread_cond_wait(&m_rawUrlNoEmpty, &m_rawUrlMutex);
    RawUrl rawUrl = m_rawUrlQueue.front();
    m_rawUrlQueue.pop_front();
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "原始统一资源定位符\"%s\"出队", rawUrl.m_strUrl.c_str());
    // 若原始统一资源定位符队列由满变为非满
    if (m_rawUrlQueue.size() < g_app->m_cfg.m_maxRawUrls)
        pthread_cond_signal(&m_rawUrlNoFull);
    //解锁线程，返回原始统一资源定位符
    pthread_mutex_unlock(&m_rawUrlMutex);
    return rawUrl;
}
// @brief 获取原始统一资源定位符数
size_t UrlQueues::sizeRawUrl(void) const
{
    // 加锁原始统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_rawUrlMutex);
    size_t size = m_rawUrlQueue.size();
    // 解锁原始统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_rawUrlMutex);
    return size;
}
// @brief 原始统一资源定位符队列空否
bool UrlQueues::emptyRawUrl(void) const
{
    // 加锁原始统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_rawUrlMutex);
    bool empty = m_rawUrlQueue.empty();
    // 解锁原始统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_rawUrlMutex);
    return empty;
}
/// @brief 原始统一资源定位符队列满否
/// @retval true  满
/// @retval false 不满
bool UrlQueues::fullRawUrl(void) const
{
    // 加锁原始统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_rawUrlMutex);
    bool full = 0 < g_app->m_cfg.m_maxRawUrls &&
                (size_t)g_app->m_cfg.m_maxRawUrls <= m_rawUrlQueue.size();
    // 解锁原始统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_rawUrlMutex);
    return full;
}
/// @brief 清空原始统一资源定位符队列
void UrlQueues::clearRawUrl(void)
{
    // 加锁原始统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_rawUrlMutex);
    m_rawUrlQueue.clear();
    pthread_cond_signal(&m_rawUrlNoFull);
    // 解锁原始统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_rawUrlMutex);
}
//-----------------------------------解析统一资源定位符------------------------------
/// @brief 压入解析统一资源定位符
void UrlQueues::pushDnsUrl(
    DnsUrl const &dnsUrl ///< [in] 解析统一资源定位符
)
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    //解析统一资源定位符队列深度配置参数有效且队列未满时，线程挂起
    while (0 < g_app->m_cfg.m_maxDnsUrls &&
           (size_t)g_app->m_cfg.m_maxDnsUrls <= m_dnsUrlQueue.size())
    {
        pthread_cond_wait(&m_dnsUrlNoFull, &m_dnsUrlMutex);
    }
    //插入解析统一资源定位符
    m_dnsUrlQueue.push_back(dnsUrl);
    // 记录调试日志
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "解析统一资源定位符\"ip=%s, port=%d, path=%s\"入队",
                        dnsUrl.m_ip.c_str(), dnsUrl.m_port, dnsUrl.m_path.c_str());
    //如果解析统一资源定位符队列非空，唤醒解析线程
    if (m_dnsUrlQueue.size() == 1)
        pthread_cond_signal(&m_dnsUrlNoEmpty);
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
}
/// @brief 弹出解析统一资源定位符
/// @return 解析统一资源定位符
DnsUrl UrlQueues::popDnsUrl(void)
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    if (m_dnsUrlQueue.empty())
        pthread_cond_wait(&m_dnsUrlNoEmpty, &m_dnsUrlMutex);
    // 弹出解析统一资源定位符
    DnsUrl dnsUrl = m_dnsUrlQueue.front();
    m_dnsUrlQueue.pop_front();
    // 记录调试日志
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                        "解析统一资源定位符\"ip=%s, port=%d, path=%s\"出队",
                        dnsUrl.m_ip.c_str(), dnsUrl.m_port, dnsUrl.m_path.c_str());
    // 若解析统一资源定位符队列未满，唤醒线程
    if (m_dnsUrlQueue.size() == g_app->m_cfg.m_maxDnsUrls - 1)
        pthread_cond_signal(&m_dnsUrlNoFull);
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
    return dnsUrl;
}
/// @brief 获取解析统一资源定位符数
/// @return 解析统一资源定位符数
size_t UrlQueues::sizeDnsUrl(void) const
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    size_t size = m_dnsUrlQueue.size();
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
    return size;
}
/// @brief 解析统一资源定位符队列空否
/// @retval true  空
/// @retval false 不空
bool UrlQueues::emptyDnsUrl(void) const
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    bool empty = m_dnsUrlQueue.empty();
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
    return empty;
}
/// @brief 解析统一资源定位符队列满否
/// @retval true  满
/// @retval false 不满
bool UrlQueues::fullDnsUrl(void) const
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    bool full = 0 < g_app->m_cfg.m_maxDnsUrls &&
                (size_t)g_app->m_cfg.m_maxDnsUrls <= m_dnsUrlQueue.size();
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
    return full;
}
/// @brief 清空解析统一资源定位符队列
void UrlQueues::clearDnsUrl(void)
{
    // 加锁解析统一资源定位符队列互斥锁
    pthread_mutex_lock(&m_dnsUrlMutex);
    m_dnsUrlQueue.clear();
    // 唤醒等待解析统一资源定位符队列非满条件变量的线程
    pthread_cond_signal(&m_dnsUrlNoFull);
    // 解锁解析统一资源定位符队列互斥锁
    pthread_mutex_unlock(&m_dnsUrlMutex);
}

/// @brief 从超文本标记语言页面内容中抽取统一资源定位符
void UrlQueues::extractUrl(
    char const *html,    ///< [in] 超文本标记语言页面内容字符串
    DnsUrl const &dnsUrl ///< [in] 被抽取页面解析统一资源定位符
)
{
    //正则表达式
    regex_t ex;
    // 编译正则表达式：href="\s*\([^ >"]*\)\s*"
    //     \s - 匹配任意空白字符(空格、制表、换页等)
    //      * - 重复前一个匹配项任意次
    //     \( - 子表达式左边界
    //     \) - 子表达式右边界
    // [^ >"] - 匹配任意不是空格、大于号和双引号的字符

    //编译正则表达式
    int error = regcomp(&ex, "href=\"\\s*\\([^ >\"]*\\)\\s*\"", 0);
    //编译失败记录
    if (error)
    {
        char errInfo[1024];
        bzero(errInfo, sizeof(errInfo) / sizeof(errInfo[0]));
        regerror(error, &ex, errInfo, sizeof(errInfo) / sizeof(errInfo[0]));
        g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,
                            "regcomp: %s", errInfo);
    }
    //匹配结果集合
    regmatch_t match[2];
    // 在超文本标记语言页面内容字符串中，查找所有与正则表达式匹配的内容
    while (regexec(&ex, html, sizeof(match) / sizeof(match[0]), match, 0) != REG_NOMATCH)
    {
        // regex : href="\s*\([^ >"]*\)\s*"
        // html  : ...href="  /software/download.html  "...
        //            |       |<-----match[1]------>|  |
        //            |     rm_so                 rm_eo|
        //            |<-------------match[0]--------->|
        //          rm_so                            rm_eo

        // 匹配子表达式的内容首地址
        html += match[1].rm_so;
        // 匹配子表达式的内容字符数
        size_t len = match[1].rm_eo - match[1].rm_so;
        // 匹配子表达式的内容字符串，即超链接中的统一资源定位符
        string strUrl(html, len);
        // 移至匹配主表达式的内容后，以备在下一轮循环中继续查找
        html += len + match[0].rm_eo - match[1].rm_eo;

        // 若是二进制资源
        if (isBinary(strUrl))
            // 继续下一轮循环
            continue;
        // 若添加域名失败
        if (!dnsUrl.attachDomain(strUrl))
            // 继续下一轮循环
            continue;

        // 记录调试日志
        g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                            "抽取到一个深度为%d的统一资源定位符\"%s\"",
                            dnsUrl.m_depth + 1, strUrl.c_str());
        // 若规格化失败
        if (!RawUrl::normalized(strUrl))
        {
            // 记录警告日志
            g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                                "规格化统一资源定位符\"%s\"失败", strUrl.c_str());
            // 继续下一轮循环
            continue;
        }

        // 压入原始统一资源定位符队列
        pushRawUrl(RawUrl(strUrl, RawUrl::ETYPE_HTML,
                          dnsUrl.m_depth + 1));
    }
    regfree(&ex);
}

/// @brief 判断某统一资源定位符所表示的资源是否是二进制资源
/// @retval true  是二进制资源
/// @retval false 非二进制资源
bool UrlQueues::isBinary(
    string const &strUrl ///< [in] 统一资源定位符字符串
)
{
    string::size_type pos;
    return pos = strUrl.find_last_of('.') != string::npos &&
                 string(".jpg.jpeg.gif.png.ico.bmp.swf").find(strUrl.substr(pos)) != string::npos;
}
/*
	/// @brief 将以毫秒表示的相对时间换算为以秒和纳秒表示的绝对时间
	/// @return 以秒和纳秒表示的绝对时间
	static timespec getTimespec (
		long msec ///< [in] 以毫秒表示的相对时间
	);
*/