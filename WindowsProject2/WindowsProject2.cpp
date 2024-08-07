#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#pragma comment (lib,"libcurl.lib")
#define _CRT_SECURE_NO_WARNINGS 1
# pragma warning(disable:4996)

bool getUrl(const char *filename)//GET请求
{
        CURL *curl;
        CURLcode res;
        FILE *fp;
        if ((fp = fopen(filename, "w")) == NULL)  // 返回结果用文件存储
                return false;
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Accept: Agent-007");
        curl = curl_easy_init();    // 初始化
        if (curl)
        {
                //curl_easy_setopt(curl, CURLOPT_PROXY, "10.99.60.201:8080");// 代理
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
                curl_easy_setopt(curl, CURLOPT_URL,"http://www.baidu.com");
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //将返回的http头输出到fp>指向的文件,
                //即为filename文件，而主函数传入的参数是 /tmp/get.html，即为真正保存在get.html文件中
                
                curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp); //将返回的html主体数据输出到fp指向的文件
                res = curl_easy_perform(curl);   // 执行
                if (res != 0) 
                {
                        curl_slist_free_all(headers);  //释放句柄
                        curl_easy_cleanup(curl);
                }
                fclose(fp);
                return true;
        }
}
/*
bool postUrl(char *filename)//POST请求
{
        CURL *curl;
        CURLcode res;
        FILE *fp;
        if ((fp = fopen(filename, "w")) == NULL)
                return false;
        curl = curl_easy_init();
        if (curl)
        {
                curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/cookie.txt"); // 指定cookie文件
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "&logintype=uid&u=xieyan&psw=xxx86");    // 指定post内容：用户信息 字段之间&连接，尝试登陆新浪邮箱
                //curl_easy_setopt(curl, CURLOPT_PROXY, "10.99.60.201:8080");
                curl_easy_setopt(curl, CURLOPT_URL, " http://mail.sina.com.cn/cgi-bin/login.cgi ");   // 指定url
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                res = curl_easy_perform(curl);//执行
                curl_easy_cleanup(curl);
        }
        fclose(fp);
        return true;
}
*/
int main()
{
        getUrl("c:\\codee\\get.html");
        //postUrl("c:\\codee\\post.html");
}
