#include <windows.h>
#include <stdio.h>
#include "curl/curl.h"
#pragma comment (lib,"libcurl.lib")

CURL *hnd=NULL;
size_t item=0;

struct curl_slist * SetNormalHeaders(){
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; Android 12; JLH-AN00 Build/HONORJLH-AN00) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.105 Mobile Safari/537.36");
    headers = curl_slist_append(headers, "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjMzNzU5NjcsImlhdCI6MTcyMjc3MTE2NywiaWQiOjE4MzczNzg4NTcsIm1haWwiOiIiLCJuaWNrbmFtZSI6IjE4OTcyOTA4NjE3Iiwic3VwcGVyIjpmYWxzZSwidXNlcm5hbWUiOjE4OTcyOTA4NjE3LCJ2IjowfQ.taBpf9iV0FQikSPk6594pWT444HMAMQn4nXjPkZcg6M");
    headers = curl_slist_append(headers, "App-Version: 3");
    headers = curl_slist_append(headers, "platform: web");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    return headers;
}


size_t writeCallback(char *b, size_t size, size_t nitems, void *p)
{
    char *str=(char *)p;
    CopyMemory(str+item,b,nitems);
    item=1+item+nitems;
    return nitems;
}

int main() {
    char *str=(char *)malloc(1024*1024);
    hnd = curl_easy_init();
    //重要!禁用ssl证书检查
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)str);


    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://www.123pan.com/b/api/file/upload_request");

    
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, SetNormalHeaders());

    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "driveId=0&etag=d41d8cd98f00b204e9800998ecf8427e&fileName=txt112&parentFileId=0&size=0&type=0");

    CURLcode ret = curl_easy_perform(hnd);
    printf("code %d\n",ret);
    if(0!=ret){
        printf("err:%s\n",curl_easy_strerror(ret));
        curl_easy_cleanup(hnd);
        ExitProcess(1);
   }
    printf("respose:\n");
    *(str+item)='\0';
    printf(str);
    return 0;
}