#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "curl/curl.h"
#include "cjson/cJSON.h"
#include "md5/md5.h"
#pragma comment (lib,"libcurl.lib")
//禁用安全警告
#define _CRT_SECURE_NO_WARNINGS 1
# pragma warning(disable:4996)
# pragma warning(disable:2664)
size_t item=0;


char * md5_hash(char * md5_string,int size)
{
  int status = 0;

	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output [16*2 + 1];
	int di;

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)md5_string, size);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
	    sprintf_s(hex_output + di * 2,sizeof(hex_output), "%02x", digest[di]);
    
    return hex_output;
}


struct curl_slist * SetNormalHeaders() {
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
    *(str+item)='\0';
    return nitems;
}

char *GetFileName(char *filepath) {
    int len=strlen(filepath);
    for(int i = len; i > 0; i--) {
        if('\\'==*(filepath+i))
        {
            return filepath+i+1;
        }
    }
}


int PreUpload(CURL *hnd,char *FilePath) {
    HANDLE hFile=CreateFileA(FilePath,GENERIC_READ,
                             0,//可共享读
                             NULL, OPEN_ALWAYS,//打开已经存在的文件
                             FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE) {
        printf("打开文件失败:%d\n",GetLastError());
        printf("filepath:%s\n",FilePath);
        ExitProcess(2);
    }
    LARGE_INTEGER lpFileSize;
    if(0==GetFileSizeEx(hFile, &lpFileSize)) {
        printf("获取文件大小失败： %d\n",GetLastError());
        printf("filepath:%s\n",FilePath);
        ExitProcess(3);
    }
    char *filestr=(char *)malloc(lpFileSize.QuadPart);
    ReadFile(hFile,filestr,lpFileSize.QuadPart,NULL,NULL);
    char *datastr=(char *)malloc(1024*2);
    sprintf_s(datastr,1024*2 ,"driveId=0&etag=%s&fileName=%s&parentFileId=0&size=%lld&type=0", md5_hash(filestr,lpFileSize.QuadPart),GetFileName(FilePath),lpFileSize.QuadPart);
    printf("\ndata:%s\n\n",datastr);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://www.123pan.com/b/api/file/upload_request");
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS,datastr);
    char *respondstr=(char *)malloc(1024*1024);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)respondstr);
    CURLcode ret = curl_easy_perform(hnd);
    if(0!=ret) {
        printf("code %d\n",ret);
        printf("err:%s\n",curl_easy_strerror(ret));
        curl_easy_cleanup(hnd);
        ExitProcess(5);
    }
    printf("respose:\n");
    printf(respondstr);

    cJSON *str_json= cJSON_Parse(respondstr);
    if (!str_json)
    {
        printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
        cJSON_Delete(str_json);//释放内存
        ExitProcess(4);
    }
    cJSON *data=cJSON_GetObjectItem(str_json, "data");
    if (!data)
    {
        printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
        cJSON_Delete(str_json);
        cJSON_Delete(data);
        ExitProcess(4);
    }
    printf("Reuse:%d\n",cJSON_GetObjectItem(data, "Reuse")->valueint);
    printf("Key:%s\n",cJSON_GetObjectItem(data, "Key")->valuestring);
    printf("Bucket:%s\n",cJSON_GetObjectItem(data, "Bucket")->valuestring);
    printf("UploadId:%s\n",cJSON_GetObjectItem(data, "UploadId")->valuestring);
    printf("SliceSize:%s\n",cJSON_GetObjectItem(data, "SliceSize")->valuestring);
    printf("StorageNode:%s\n",cJSON_GetObjectItem(data, "StorageNode")->valuestring);
    cJSON_Delete(str_json);
    cJSON_Delete(data);
    // free(str);
    free(respondstr);
    free(filestr);
    free(datastr);
}



int main() {
    printf("start\n\n");
    CURL *hnd= curl_easy_init();
    //重要!禁用ssl证书检查
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, SetNormalHeaders());

    //curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "driveId=0&etag=d41d8cd98f00b204e9800998ecf8427e&fileName=txt112&parentFileId=0&size=0&type=0");
    PreUpload(hnd,(char *)".\\WindowsProject2\\curl\\1");

    return 0;
}