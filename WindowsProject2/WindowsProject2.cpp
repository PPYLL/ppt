#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "curl/curl.h"
#pragma comment (lib,"libcurl.lib")
 
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    ((std::string*)userdata)->append(ptr, nmemb);
    return nmemb;
}
 
 
 
 
int main()
{
    printf("started\n");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (curl) 
    {        
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
 
        // no certificate, not verify server certificate
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
 
        std::string response_data;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        }
        else
        {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("response code %ld \n", response_code);
            printf("response data : \n ");
            printf(response_data.c_str());
        }
 
        curl_easy_cleanup(curl);
    }
 
    curl_global_cleanup();
    printf("ended\n");
    return 0;
}
