#include "Precompile.h"
#include "DnsThread.h"
#include "WebCrawler.h"

// @brief 线程处理函数
/// @note 根据域名解析线程的任务实现基类中的纯虚函数
void *DnsThread::run(void)
{
	g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__, "dns线程开始");
	while (1)
	{
		// 从统一资源定位符队列，弹出原始统一资源
		// 定位符，并显式转换为解析统一资源定位符
		DnsUrl dnsUrl = static_cast<DnsUrl>(g_app->m_urlQueues.popRawUrl());
		// 在主机域名————IP地址映射表中，
		// 查找该统一资源定位符的主机域名
		map<string, string>::const_iterator it =
			s_hosts.find(dnsUrl.m_domain);
		// 若找到了 将与该主机域名对应的IP地址,存入解析统一资源定位符
		if (it != s_hosts.end())
		{
			dnsUrl.m_ip = it->second;
			g_app->m_urlQueues.pushDnsUrl(dnsUrl);
			// 记录调试日志
			g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
								"域名\"%s\"曾经被解析为\"%s\"", dnsUrl.m_domain.c_str(),
								dnsUrl.m_ip.c_str());
			continue;
		} //若没找到，查询
		hostent *host = gethostbyname(dnsUrl.m_domain.c_str());
		//     hostent
		// +-------------+
		// | h_name      -> xxx\0          - 正式主机名
		// | h_aliases   -> * * * ... NULL - 别名表
		// | h_addrtype  |  AF_INET        - 地址类型
		// | h_length    |  4              - 地址字节数
		// | h_addr_list -> * * * ... NULL - 地址表
		// +-------------+  +-> in_addr    - IPv4地址结构
		if (!host)
		{
			g_app->m_log.printf(Log::LEVEL_ERR, __FILE__, __LINE__,
								"gethostbyname: %s", hstrerror(h_errno));
			continue;
		}
		//若地址类型不是ipv4
		if (host->h_addrtype != AF_INET)
		{
			g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
								"无效地址类型");
			continue;
		}
		// 将IPv4地址结构转换为点分十进制字符串，存入解析统
		// 一资源定位符，同时加入主机域名————IP地址映射表
		s_hosts[dnsUrl.m_domain] = inet_ntoa(**(in_addr **)host->h_addr_list);
		// 将解析统一资源定位符，压入统一资源定位符队列
		g_app->m_urlQueues.pushDnsUrl(dnsUrl);
		g_app->m_log.printf(Log::LEVEL_DBG, __FILE__,__LINE__,
							"域名\"%s\"被成功解析为\"%s\"", dnsUrl.m_domain.c_str(),
							dnsUrl.m_ip.c_str());
	}
	g_app->m_log.printf (Log::LEVEL_DBG, __FILE__, __LINE__,
                "DNS线程终止");
        // 终止线程
    return NULL;
}
/*
	/// @brief 域名解析回调函数
	static void callback (
		int   res,   ///< [in] 结果码
		char  type,  ///< [in] 地址类型
		int   cnt,   ///< [in] 地址数
		int   ttl,   ///< [in] 解析记录缓存时间(秒)
		void* addrs, ///< [in] 地址列表
		void* arg    ///< [in] 回调参数
	)
*/

// 主机域名————IP地址映射表
//map<string, string> DnsThread::s_hosts;