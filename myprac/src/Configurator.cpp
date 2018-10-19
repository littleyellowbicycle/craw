#include "Precompile.h"
#include "WebCrawler.h"
#include "StrKit.h"

Configurator::Configurator(void) : m_logLevel(Log::LEVEL_DBG), // 最低日志等级缺省为调试
                                   m_maxJobs(-1),              // 最大抓取任务数缺省为无限
                                   m_maxDepth(-1),             // 最大递归深度缺省为无限
                                   m_maxRawUrls(-1),           // 原始统一资源定位符队
                                                               // 列最大容量缺省为无限
                                   m_maxDnsUrls(-1),           // 解析统一资源定位符队
                                                               // 列最大容量缺省为无限
                                   m_statInterval(0)
{
} // 状态间隔缺省为不设定时器

void Configurator::load(string const &cfgFile) //< [in] 配置文件路径
{
    ifstream ifs(cfgFile.c_str());
    if (!ifs)
    {
        g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,
                            "load cfgfile %s error", cfgFile.c_str());
    }
    string line;
    for (int linenum = 0; getline(ifs, line); ++linenum)
    {
        if (line[0] == '#' || line[0] == '\0')
            continue;
        StrKit::trim(line);
        vector<string> strv = StrKit::split(line, "=", 1);
        if (strv.size() == 2)
        {
            if (!strcasecmp(strv[0].c_str(), "LOG_LEVEL"))
                m_logLevel = atoi(strv[1].c_str());
            else if (!strcasecmp(strv[0].c_str(), "LOG_FILE"))
                m_logFile = strv[1];
            else if (!strcasecmp(strv[0].c_str(), "MAX_JOBS"))
                // 则值为"最大抓取任务数"
                m_maxJobs = atoi(strv[1].c_str());
            else if (!strcasecmp(strv[0].c_str(), "MAX_DEPTH"))
                // 则值为"最大递归深度"
                m_maxDepth = atoi(strv[1].c_str());
            // 否则
            else
                // 若键为"MAX_RAW_URLS"
                if (!strcasecmp(strv[0].c_str(), "MAX_RAW_URLS"))
                // 则值为"原始统一资源定位符队列最大容量"
                m_maxRawUrls = atoi(strv[1].c_str());
            // 否则
            else
                // 若键为"MAX_DNS_URLS"
                if (!strcasecmp(strv[0].c_str(), "MAX_DNS_URLS"))
                // 则值为"解析统一资源定位符队列最大容量"
                m_maxDnsUrls = atoi(strv[1].c_str());
            // 否则
            else
                // 若键为"STAT_INTERVAL"
                if (!strcasecmp(strv[0].c_str(), "STAT_INTERVAL"))
                // 则值为"状态间隔"
                m_statInterval = atoi(strv[1].c_str());
            // 否则
            else
                // 若键为"SEEDS"
                if (!strcasecmp(strv[0].c_str(), "SEEDS"))
                // 则值为"种子链接"
                m_seeds = strv[1];
            // 否则
            else
                // 若键为"INCLUDE_PREFIXES"
                if (!strcasecmp(strv[0].c_str(), "INCLUDE_PREFIXES")) // 则值为"包含前缀"
                m_includePrefixes = strv[1];
            // 否则
            else
                // 若键为"EXCLUDE_PREFIXES"
                if (!strcasecmp(strv[0].c_str(), "EXCLUDE_PREFIXES"))
                // 则值为"排除前缀"
                m_excludePrefixes = strv[1];
            // 否则
            else
                // 若键为"PLUGINS_PATH"
                if (!strcasecmp(strv[0].c_str(), "PLUGINS_PATH"))
                // 则值为"插件路径"
                m_pluginsPath = strv[1];
            // 否则
            else
                // 若键为"LOAD_PLUGIN"
                if (!strcasecmp(strv[0].c_str(), "LOAD_PLUGIN"))
                // 则值为"插件名"
                m_loadPlugins.push_back(strv[1]);
            // 否则
            else
                // 若键为"ACCEPT_TYPE"
                if (!strcasecmp(strv[0].c_str(), "ACCEPT_TYPE"))
                // 则值为"接受类型"
                m_acceptTypes.push_back(strv[1]);
            // 否则

            else
                // 记录一般错误日志
                g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,
                                    "无效配置项: %s, %d, %s", cfgFile.c_str(),
                                    linenum, strv[0].c_str());
        }
        else
                        // 记录一般错误日志
                        g_app->m_log.printf (Log::LEVEL_ERR,  __FILE__, __LINE__,
                                "无效配置行: %s, %d, %s", cfgFile.c_str (),
                                linenum, line.c_str ());
                        
    }
    ifs.close();
}