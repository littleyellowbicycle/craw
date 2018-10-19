#include "Precompile.h"
#include "StrKit.h"
#include "Url.h"

// @brief 构造器
RawUrl::RawUrl(
    string const &strUrl, ///< [in] 统一资源定位符字符串
    ETYPE type,           ///< [in] 资源类型
    int depth             ///< [in] 链接深度
)
{
    //将统一资源定位符修剪后赋值给内置变量
    string strUrlTmp = strUrl;
    RawUrl::normalized(strUrlTmp);
    m_strUrl = strUrlTmp;
    m_type = type;
    m_depth = depth;
}

/// @brief 规格化
/// @retval true  成功
/// @retval false 失败
/// @remark 删除协议标签(http://或https://)和结尾分隔符(/)

bool RawUrl::normalized(
    string &strUrl ///< [in,out] 待规格化统一资源定位符字符串
)
{
    string strtmp = strUrl;
    //修剪字符串中的空格
    StrKit::trim(strtmp);
    //空串报错
    if (!strtmp.size())
        return false;
    //除去字符串中开头的协议标志和结尾的分隔符
    //去除开头的“http://”与”https://"
    if (!strtmp.find("http://"))
        strtmp = strtmp.substr(7);
    else if (!strtmp.find("https://"))
        strtmp = strtmp.substr(8);
    //去除结尾的“/”
    if (*(strtmp.end() - 1) == '/')
        strtmp.erase(strtmp.end() - 1);
    //太长报错
    if (strtmp.size() >= 128)
        return false;
    strUrl = strtmp;
    return true;
}
//---------------------------------------------------------------
DnsUrl::DnsUrl(RawUrl const &rawUrl) : RawUrl(rawUrl)
{
    //第一个‘/’之前的即为域名，若没有‘/’则整个字符串都是域名
    string::size_type pos = m_strUrl.find('/');
    if (pos == string::npos)
        m_domain = m_strUrl;
    else
    {
        m_domain = m_strUrl.substr(0, pos);
        //第一个‘/’后到末尾即为路径
        m_path = m_strUrl.substr(pos);
    }
    //“：”后即为端口号，否则使用默认端口80
    if ((pos = m_domain.find_last_of(':')) == string::npos ||
            !(m_port = atoi(m_domain.substr(pos + 1).c_str())))
    {
        m_port = 80;
    }
}

//构建文件名字符串
//域名+‘/’+路径+“.html”=文件名
string DnsUrl::toFilename() const
{
    string fileName = m_domain;
    //如果路径不为空，则文件名补上路径
    if (m_path.size())
        (fileName + '/') += m_path;
    //将‘/’替换为-
    for (string::size_type pos = 0;
         (pos = fileName.find('/', pos)) != string::npos;
         pos++)
    {
        fileName.replace(pos, 1, "_");
    }
    //查找是否有文件名后缀并补齐
    if (string::size_type pos = fileName.find_last_of('.') == string::npos ||
                                (fileName.substr(pos) != ".html" && fileName.substr(pos) != ".htm"))
        fileName += ".html";
    return fileName;
}

//在定位符前添加域名
bool DnsUrl::attachDomain(
    string &strUrl ///< [in,out] 待加域名统一资源定位符字符串
    )

    const
{
    //如果定位符开头为“http”则不需要添加
    if (!strUrl.find("http", 0))
        return true;
    //如果待加域名统一资源定位符字符串为空，或首字符不是'/'，返回失败
    if (strUrl.empty() || *(strUrl.begin()) != '/')
        return false;
    strUrl.insert(0, m_domain);
    return true;
}