#include <StrKit.h>
#include <string.h>
#include <stdarg.h>
#include <vector>

//chars link
string StrKit::strcat(char const *str1, ///< [in] 字符串1
                      char const *str2, ///< [in] 字符串2
                      ...)
{
    string str = str1;
    str += str2;
    va_list ap;
    va_start(ap, str2);
    while (char *temp = va_arg(ap, char *))
    {
        str += temp;
    }
    va_end(ap);
    return str;
}

string &StrKit::trim(
    string &str ///< [in,out] 待修剪字符串
)
{
    //string temp =str;
    string::size_type first = str.find_first_not_of(" \n\t\r");
    string::size_type last = str.find_last_not_of(" \n\t\r");
    if (first != string::npos && last != string::npos)
        str = str.substr(first, last - first + 1);
    else
        str.clear();
    return str;
}

vector<string> StrKit::split(
    string const &str,   ///< [in] 待拆分字符串
    string const &delim, ///< [in] 分隔符字符串
    int limit = 0        ///< [in] 拆分次数限制
)
{
    vector<string> res;
    char temp[str.size() + 1];
    strcpy(temp, str.c_str());
    for (char *token = strtok(temp, delim.c_str()); token;
         token = strtok(NULL, delim.c_str()))
    {
        string part = token;
        res.push_back(trim(part));
        if (!--limit && (token += strlen(token)) - temp <
                            (int)str.size())
        {
            // 将待拆分字符串的其余部分一次
            // 性压入存放拆分结果的子串向量
            strv.push_back(trim(part = ++token));
            // 提前结束拆分循环
            break;
        }
        return res;
    }