#include "Precompile.h"
#include "Socket.h"
#include "WebCrawler.h"
#include "StrKit.h"

Socket::Socket(DnsUrl const &dnsUrl) : m_dnsUrl(dnsUrl), m_sockfd(-1) {}

Socket::~Socket()
{
    if (m_sockfd == 0)
    {
        close(m_sockfd);
        m_sockfd = -1;
    }
}

bool Socket::sendRequest()
{
    //创建套接字
    if (socket(AF_INET, SOCK_STREAM, 0) == -1)
    {
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__, "socket:%s", strerror(errno));
        return false;
    }
    g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__, "创建套接字%d成功", m_sockfd);
    //确定服务器结构体
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = m_dnsUrl.m_port;
    //获取ip网络字节序
    if (inet_aton(m_dnsUrl.m_ip.c_str(), &addr.sin_addr))
    {
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__, "inet:%s", strerror(errno));
        return false;
    }
    //连接服务器
    if(connect(m_sockfd,(sockaddr*)&addr,sizeof(addr))==-1){
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__, "connect: %s", strerror(errno));
        return false;
    }
    g_app->m_log.printf(Log::LEVEL_DBG,__FILE__,__LINE__,"连接服务器\":%s\"成功",m_dnsUrl.m_ip.c_str());
    //获取套接字状态标志
    if(int flags=fcntl(m_sockfd,F_GETFD)==-1){
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__, "fcntl: %s", strerror(errno));
        return false;
    }
    int flags = fcntl (m_sockfd, F_GETFL);
    // 为套接字增加非阻塞状态标志，若失败
    if (fcntl (m_sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        // 记录警告日志
        g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                            "fcntl: %s", strerror(errno));
        // 返回失败
        return false;
    }
    // 输出字符串流
    ostringstream oss;
    // 在输出字符串流中格式化超文本传输协议请求
    oss <<
                "GET /" << m_dnsUrl.m_path << " HTTP/1.0\r\n"
                "Host: " << m_dnsUrl.m_domain << "\r\n"
                "Accept: */*\r\n"
                "Connection: Close\r\n"
                "User-Agent: Mozilla/5.0\r\n"
                "Referer: " << m_dnsUrl.m_domain << "\r\n\r\n";
    string request=oss.str();
    // 从超文本传输协议请求字符串中获取其内存缓冲区指针
    char const* buf = request.c_str ();
    // 成功发送的字节数
    ssize_t slen;
    //发送请求头
    for(size_t len=request.size();len;len-=slen,buf+=slen){
        if ((slen = send(m_sockfd, buf, len, 0)) == -1)
        {
            //如果因为内核缓冲区满延迟重发
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                usleep(1000);
                slen = 0;
                continue;
            }
            //否则报错
            g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                               "send: %s", strerror(errno));
            return false;
        }
    }
    // 记录调试日志
    g_app->m_log.printf (Log::LEVEL_DBG, __FILE__, __LINE__,
                "发送超文本传输协议请求包%u字节\n\n%s", request.size (),request.c_str ());
    // 关注边沿触发的输入事件，并将套接字对象指针存入事件结
    // 构中。一旦发自服务器的超文本传输协议响应到达，即套接
    // 字接收缓冲区由空变为非空，MultiIo::wait便会立即发现，
    // 并在独立的接收线程中通过此套接字对象接收响应数据
    epoll_event event = {EPOLLIN | EPOLLET, this};
    // 增加需要被关注的输入输出事件，若失败
    if (! g_app->m_multiIo.add (m_sockfd, event))
        // 返回失败
        return false;
    // 记录调试日志
    g_app->m_log.printf (Log::LEVEL_DBG, __FILE__, __LINE__,
                "关注套接字%d上的I/O事件", m_sockfd);
    return true;
}

/// @brief 接收超文本传输协议响应
/// @retval true  成功
/// @retval false 失败
bool Socket::recvResponse(void)
{
    HttpResponse res(m_dnsUrl);
    // 超文本传输协议响应包头尚未解析
    bool headerParsed = false;
    // 分多次将超文本传输协议响应包收完
    for (;;)
    {
        char buf[1024];
        //接受响应，留一个位置放‘/0’
        int rlen = recv(m_sockfd, buf, sizeof(buf) - sizeof(buf[0]), 0);
        //若接受失败
        if (rlen == -1)
        {
            //若因为内核缓存不足，延迟重收
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            {
                usleep(1000);
                continue;
            }
            //否则打印
            g_app->m_log.printf(Log::LEVEL_WAR, __FILE__, __LINE__,
                                "接受响应头失败:%s", strerror(errno));
            return false;
        }
        // 若服务器已关闭连接
        if (!rlen)
        {
            // 记录调试日志
            g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                                "接收超文本传输协议响应包体%u字节数据", res.m_len);
            // 若超文本传输协议响应的内容类型为超文本标记语言
            if (res.m_header.m_contentType.find("text/html", 0) != string::npos)
            {
                g_app->m_urlQueues.extractUrl(res.m_body, m_dnsUrl);
            }
            // 调用超文本标记语言插件处理函数
            g_app->m_pluginMngr.invokeHtmlPlugins(&res);
            break;
        }
        // 扩展超文本传输协议响应包体缓冲区，以容纳新
        // 接收到的响应数据，注意多分配一个字符放'\0'
        res.m_body = (char *)realloc(res.m_body,
                                     res.m_len + rlen + sizeof(res.m_body[0]));
        // 将新接收到的响应数据，连同终止空字符('\0')一起，
        // 从接收缓冲区复制到超文本传输协议响应包体缓冲区
        memcpy(res.m_body + res.m_len, buf,
               rlen + sizeof(res.m_body[0]));
        res.m_len += rlen;
        //若包头包体未分开
        if (!headerParsed)
        {
            char *p = strstr(res.m_body, "\r\n\r\n");
            if (p)
            {
                // 截取超文本传输协议响应包头
                string header(res.m_body, p - res.m_body);
                // 记录调试日志
                g_app->m_log.printf(Log::LEVEL_DBG, __FILE__, __LINE__,
                                    "接收超文本传输协议响应包头%u字节\n\n%s",
                                    header.size(), header.c_str());
            //解析响应包头
            res.m_header = parseHeader(header);                         
            }
            // 调用超文本传输协议响应包头插件处理函数，若失>败
            if (!g_app->m_pluginMngr.invokeHeaderPlugins(
                    &res.m_header))
                // 返回失败
                return false;
            headerParsed = true;
            p += 4;
            res.m_len -= p - res.m_body;
            // 分配足以容纳已接收超文本传输协议响应
            // 包体(含终止空字符('\0'))的临时缓冲区
            char *tmp = new char[res.m_len + sizeof(res.m_body[0])];
            // 将已接收超文本传输协议响应包体，连同
            // 终止空字符('\0')一起，从超文本传输协
            // 议响应包体缓冲区复制到临时缓冲区
            memcpy(tmp, p, res.m_len + sizeof(res.m_body[0]));
            //将缓冲区的文本复制到包体
            memcpy(res.m_body, tmp, res.m_len + sizeof(res.m_body[0]));
            delete[] tmp;
        }
    }
    return true;
}

int Socket::sockfd(void) const
{
    return m_sockfd;
}

/// @brief 解析超文本传输协议响应包头,确定状态码和内容类型
/// @return 超文本传输协议响应包头
HttpHeader Socket::parseHeader(
    string str ///< [in] 超文本传输协议响应包头字符串
    ) const
{
    HttpHeader header = {};
    // 超文本传输协议响应包头实例：
    //
    // HTTP/1.1 200 OK
    // Server: nginx
    // Date: Wed, 26 Oct 2016 10:52:04 GMT
    // Content-Type: text/html;charset=UTF-8
    // Connection: close
    // Vary: Accept-Encoding
    // Server-Host: classa-study30
    // Set-Cookie: Domain=study.163.com
    // Cache-Control: no-cache
    // Pragma: no-cache
    // Expires: -1
    // Content-Language: en-US
    string::size_type pos = str.find("\r\n", 0);
    //查找状态码
    if (pos != string::npos)
    {
        // 拆分超文本传输协议响应包头的第一
        // 行，以空格为分隔符，最多拆分两次
        // HTTP/1.1 200 OK
        //    0    ^ 1 ^ 2
        vector<string> strv = StrKit::split(str.substr(0, pos), " ", 2);
        // 若成功拆分出三个子串
        if (strv.size() == 3)
        {
            // 其中的第二个子串即为状态码
            header.m_statusCode = atoi(strv[1].c_str());
        }
        else
            // 取状态码为600，即"无法解析的响应包头"
            header.m_statusCode = 600;
        // 截取除第一行(含回车换行)以外的其余内容，继续解析
        str = str.substr(pos + 2);
    }
    //查找文件类型
    while((pos=str.find ("\r\n", 0)) != string::npos){
        vector<string> strv = StrKit::split (
                        str.substr (0, pos), ":", 1);
        if(strv.size()==2&&!strcasecmp(strv[0].c_str(),"Content-Type:")){
            header.m_contentType=strv[1];
            break;
        }
        str = str.substr (pos + 2);
    }
    return header;
}
