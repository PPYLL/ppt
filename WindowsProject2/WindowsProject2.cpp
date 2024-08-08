#define _CRT_SECURE_NO_WARNINGS 1 //要放在第一行
# pragma warning(disable:4996)
#include <windows.h>
#include <stdio.h>
#include "curl/curl.h"
#include "cjson/cJSON.h"
#include "md5/md5.h"
#pragma comment (lib,"libcurl.lib")

size_t item=0;
char * md5_hash(char * md5_string,int size)
{
  int status = 0;

	md5_state_t state;
	md5_byte_t digest[16];
	static char* hex_output;
    if(!hex_output)hex_output= malloc(16*2 + 1);
	int di;

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)md5_string, size);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
	    sprintf(hex_output + di * 2, "%02x", digest[di]);

    return hex_output;
}

struct curl_slist * SetNormalHeaders() {
    static struct curl_slist *headers = NULL;
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


void PreUpload(CURL *hnd,char *FilePath) {
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
    printf("filesize:%lld\n",lpFileSize.QuadPart);
    char *filestr=(char *)malloc(lpFileSize.QuadPart);
    ReadFile(hFile,filestr,lpFileSize.QuadPart,NULL,NULL);
    printf("md5:%s\n",md5_hash(filestr,lpFileSize.QuadPart));
    char *datastr=(char *)malloc(1024*2);
    sprintf_s(datastr,1024*2 ,"driveId=0&etag=%s&fileName=%s&parentFileId=0&size=%lld&type=0", md5_hash(filestr,lpFileSize.QuadPart),GetFileName(FilePath),lpFileSize.QuadPart);
    printf("\ndata:%s\n\n",datastr);
    
}

int main() {
    printf("started\n");
    CURL *hnd= curl_easy_init();
    //重要!禁用ssl证书检查
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, SetNormalHeaders());
/*
    char FilePath[]=".\\WindowsProject2\\2.json";
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
    printf("filesize:%lld\n",lpFileSize.QuadPart);
    char *filestr=(char *)malloc(lpFileSize.QuadPart);
    ReadFile(hFile,filestr,lpFileSize.QuadPart,NULL,NULL);
    
    printf("md5:%s\n",md5_hash(filestr,lpFileSize.QuadPart));
   // printf(filestr);
    char *datastr=(char *)malloc(1024*2);
    sprintf_s(datastr,1024*2 ,"driveId=0&etag=%s&fileName=%s&parentFileId=0&size=%lld&type=0", md5_hash(filestr,lpFileSize.QuadPart),GetFileName(FilePath),lpFileSize.QuadPart);
    printf("\ndata:%s\n\n",datastr);
    


    cJSON *str_json= cJSON_Parse(filestr);
    if (!str_json)
    {
        printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr()); //输出json格式错误信息

        cJSON_Delete(str_json);//释放内存
        exit(0);
    }
    cJSON *data=cJSON_GetObjectItem(str_json, "data");
    //printf(cJSON_Print(data));
    //printf("%p\n",data);
    printf("Reuse:%d\n",cJSON_GetObjectItem(data, "Reuse")->valueint);
    printf("Key:%s\n",cJSON_GetObjectItem(data, "Key")->valuestring);
    printf("Bucket:%s\n",cJSON_GetObjectItem(data, "Bucket")->valuestring);
    printf("UploadId:%s\n",cJSON_GetObjectItem(data, "UploadId")->valuestring);
    printf("SliceSize:%s\n",cJSON_GetObjectItem(data, "SliceSize")->valuestring);
    printf("StorageNode:%s\n",cJSON_GetObjectItem(data, "StorageNode")->valuestring);
    cJSON_Delete(str_json);//释放内存


*/
    /*
    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, "https://www.123pan.com/b/api/file/upload_request");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; Android 12; JLH-AN00 Build/HONORJLH-AN00) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.105 Mobile Safari/537.36");
    headers = curl_slist_append(headers, "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjMzNzU5NjcsImlhdCI6MTcyMjc3MTE2NywiaWQiOjE4MzczNzg4NTcsIm1haWwiOiIiLCJuaWNrbmFtZSI6IjE4OTcyOTA4NjE3Iiwic3VwcGVyIjpmYWxzZSwidXNlcm5hbWUiOjE4OTcyOTA4NjE3LCJ2IjowfQ.taBpf9iV0FQikSPk6594pWT444HMAMQn4nXjPkZcg6M");
    headers = curl_slist_append(headers, "LoginUuid: edf2d4acef7d7d559e10c12d5b5baaa70fdd6b27fb22af35e48c1aa6b4b778bda1647c17962d60e792e4926a08c9dcf0");
    headers = curl_slist_append(headers, "App-Version: 3");
    headers = curl_slist_append(headers, "platform: web");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    headers = curl_slist_append(headers, "Origin: https://www.123pan.com");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "driveId=0&etag=d41d8cd98f00b204e9800998ecf8427e&fileName=txt111112&parentFileId=0&size=0&type=0");

    CURLcode ret = curl_easy_perform(hnd);
    printf("code %d\n",ret);
    //if(!ret){
        printf(curl_easy_strerror(ret));
   // }
*/
    printf("ended\n");
}