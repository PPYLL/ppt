#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#pragma comment (lib,"libcurl.lib")

struct memory {
    char* response;
    size_t size;
};

static size_t callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = (char*)realloc((void*)mem->response, mem->size + realsize + 1);
    if(ptr == NULL) {
        return 0;  /* out of memory! */
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = '\0';

    return realsize;
}
int main() {
    printf("started\n");
     /* 1. 初始化 */
    CURL* curl = curl_easy_init();
    if (!curl) {
        printf("curl_easy_init failed\n");
        return 1;
    }

    struct memory chunk = {0};
    chunk.response = NULL;
    chunk.size = 0;

    /* 2. 发送请求 */
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_perform(curl);

    /* 3. 查看请求返回结果 */
    printf("%s\n", chunk.response);

    /* 4. 清理 */
    if (chunk.response) {
        free(chunk.response);
        chunk.response = NULL;
        chunk.size = 0;
    }
    curl_easy_cleanup(curl);
    curl = NULL;

    return 0;
    printf("ended\n");
}
