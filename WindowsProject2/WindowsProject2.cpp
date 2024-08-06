#include <windows.h>
#include <stdio.h>
#include "curl/curl.h"
#pragma comment (lib,"libcurl.lib")
int main() {
    printf("started\n");
    CURL *hnd = curl_easy_init();
    int i=0;
    printf("0\n");++i;
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    printf("%d\n",i);++i;
    curl_easy_setopt(hnd, CURLOPT_URL, "https://www.123pan.com/b/api/file/upload_complete");
    printf("%d\n",i);++i;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) alist-client");
    printf("%d\n",i);++i;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    printf("%d\n",i);++i;
    headers = curl_slist_append(headers, "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjMzNzU5NjcsImlhdCI6MTcyMjc3MTE2NywiaWQiOjE4MzczNzg4NTcsIm1haWwiOiIiLCJuaWNrbmFtZSI6IjE4OTcyOTA4NjE3Iiwic3VwcGVyIjpmYWxzZSwidXNlcm5hbWUiOjE4OTcyOTA4NjE3LCJ2IjowfQ.taBpf9iV0FQikSPk6594pWT444HMAMQn4nXjPkZcg6M");
    printf("%d\n",i);++i;
    headers = curl_slist_append(headers, "Platform: open_platform");
    headers = curl_slist_append(headers, "app-version: 3");
    printf("%d\n",i);++i;
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
    printf("%d\n",i);++i;
    curl_easy_setopt(hnd, CURLOPT_COOKIE, "aliyungf_tc=5c841a49275419542a71df1e90017e1c6accd3aef405365eeafd5fa12c9eefc5");
    printf("%d\n",i);++i;
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "DriveId=0&duplicate=2&etag=d41d8cd98f00b204e9800998ecf8427e&fileName=txtt1&parentFileId=0&size=0&type=0");
    printf("%d\n",i);++i;
    CURLcode ret = curl_easy_perform(hnd);
    printf("ended\n",i);
}