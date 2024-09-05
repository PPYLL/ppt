#define _CRT_SECURE_NO_WARNINGS 1 //要放在第一行
# pragma warning(disable:4996)
#include <windows.h>
#include <stdio.h>
#include "curl/curl.h"
#include "cjson/cJSON.h"
#include "md5/md5.h"
#pragma comment (lib,"libcurl.lib")

char *memstr;//文件分片内容存放

#define MAXRESPNSESIZE 1024*1024
#define MEMSIZE 32*1024*1024

struct FILEDATA {
    char *filename;
    char *filepath;
    long long filesize;
};

struct UPLOADDATA {
    cJSON *data;
    char *Key;
    char *Bucket;
    char *UploadId;
    char *StorageNode;
    int SliceSize;
    int FileId;
    int isMultipart;
    struct FILEDATA FileData;
    //char *md5;//临时使用，为分片文件md5
};


struct RESPONSE {
    char *str;
    size_t maxsize;
    size_t nowsize;
};


char * md5_hash(char * md5_string,int size)
{
    int status = 0;

    md5_state_t state;
    md5_byte_t digest[16];
    static char* hex_output;
    if(!hex_output)hex_output= (char *)malloc(16*2 + 1);
    int di;

    md5_init(&state);
    md5_append(&state, (const md5_byte_t *)md5_string, size);
    md5_finish(&state, digest);

    for (di = 0; di < 16; ++di)
        sprintf(hex_output + di * 2, "%02x", digest[di]);

    return hex_output;
}

void SetNormalHeaders(CURL *hnd) {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Linux; Android 12; JLH-AN00 Build/HONORJLH-AN00) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.105 Mobile Safari/537.36");
    headers = curl_slist_append(headers, "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjYxMTc4NTcsImlhdCI6MTcyNTUxMzA1NywiaWQiOjE4MzczNzg4NTcsIm1haWwiOiIiLCJuaWNrbmFtZSI6IjE4OTcyOTA4NjE3Iiwic3VwcGVyIjpmYWxzZSwidXNlcm5hbWUiOjE4OTcyOTA4NjE3LCJ2IjowfQ.d4QgDvfA6ITBZ8hkBTMgj29yl2n9ktsUSiN-Bpl0v9s");
    headers = curl_slist_append(headers, "App-Version: 3");
    headers = curl_slist_append(headers, "platform: web");
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER,headers );
    return;
}

size_t writeCallback(char *b, size_t size, size_t nitems, void *p)
{
    if(!p) ExitProcess(9);
    struct RESPONSE *response=(RESPONSE *)p;
    if(0==response->nowsize) ZeroMemory(response->str,response->maxsize);

    CopyMemory(response->str+response->nowsize,b,nitems);
    response->nowsize=response->nowsize+nitems+1;
    if(response->nowsize>response->maxsize)
    {
        return 0;
    }
    *(response->str+response->nowsize)='\0';
    return nitems;
}



size_t read_cb(char *ptr, size_t size, size_t nitems, void *userdata)
{

    struct RESPONSE *response=(RESPONSE*)userdata;


    ptr=memstr+(response->nowsize)+1;
    if((response->nowsize+nitems)>(response->maxsize))
    {
        size_t i=(response->maxsize)-(response->nowsize);
        response->nowsize=response->nowsize+nitems;
        return i;
    }



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
    return NULL;
}
/*
此函数专用于123盘post
@param url:网址(写全)
@param data:格式:application/x-www-form-urlencoded
@param 状态值，正常:0;失败:函数本身<0,其他>0
返回值:成功，返回rootJson;失败，返回NULL
*/
char *Https_Post(char *url,char *data,int *httpcode) {
    static struct RESPONSE response;
    if(!(response.str))
    {
        response.str=(char *)malloc(MAXRESPNSESIZE);
        if(!(response.str))
        {
            printf("mem alloc err");
            *httpcode=-1;
            return NULL;
        }
    }
    ZeroMemory(response.str,MAXRESPNSESIZE);
    response.maxsize=MAXRESPNSESIZE;
    response.nowsize=0;
    cJSON *str_json;
    CURL *hnd= curl_easy_init();
    if(!hnd)
    {
        printf("curl_init err\n");
        *httpcode=-2;
        return NULL;
    }
    //重要!禁用ssl证书检查
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, 0);

    SetNormalHeaders(hnd);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS,data);




    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&response);
    CURLcode ret = curl_easy_perform(hnd);
    if(0!=ret) {
        printf("code %d\n",ret);
        printf("err:%s\n",curl_easy_strerror(ret));
        curl_easy_cleanup(hnd);
        //free(datastr);
        //free(response.str);

    }
    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &httpcode);
    printf("HTTP Response Code: %ld\n", *httpcode);
    if(200!=(*httpcode))
    {
        printf("uploadRequest err,code=%ld\n",*httpcode);
        curl_easy_cleanup(hnd);
        return NULL;
    }

    str_json= cJSON_Parse(response.str);
    if (!str_json)
    {
        printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
        cJSON_Delete(str_json);//释放内存
        curl_easy_cleanup(hnd);
        *httpcode=-3;
        return NULL;
    }
    if(!(cJSON_GetObjectItem(str_json, "code")))
    {
        printf("Get code err\n");
        curl_easy_cleanup(hnd);
        cJSON_Delete(str_json);
        *httpcode=-4;
        return NULL;
    }
    if(0!=cJSON_GetObjectItem(str_json, "code")->valueint) {
        printf("respose:\n");
        printf(response.str);
        cJSON_Delete(str_json);
        curl_easy_cleanup(hnd);
        *httpcode=cJSON_GetObjectItem(str_json, "code")->valueint;
        return NULL;
    }
    cJSON_Delete(str_json);
    curl_easy_cleanup(hnd);
    return response.str;
}



int UploadFileChuck(int start,int end,struct UPLOADDATA UploadData) {
    char *datastr=(char *)malloc(1024*2);
    int flag=0;
    cJSON *data=NULL,*str_json=NULL,*presignedUrls=NULL;
    CURL *curl=NULL;
    CURLcode res;
    CURLHcode ret;
    struct curl_header *http_header=NULL;
    struct RESPONSE response;
    DWORD ReadLen = 0;
    ZeroMemory(&response,sizeof(struct RESPONSE));
    HANDLE hFile=CreateFileA(UploadData.FileData.filepath,GENERIC_READ,
                             0,//可共享读
                             NULL, OPEN_ALWAYS,//打开已经存在的文件
                             FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE) {
        printf("打开文件失败:%d\n",GetLastError());
        printf("filepath:%s\n",UploadData.FileData.filepath);
        return start;
    }
	LARGE_INTEGER Sizeinfo;
    ZeroMemory(&Sizeinfo,sizeof(LARGE_INTEGER));
    Sizeinfo.QuadPart=(UploadData.SliceSize)*(start-1);
    if(!(SetFilePointerEx(hFile,Sizeinfo,NULL,FILE_BEGIN)))
    {
        printf("setFilePointerEx err\n");
        CloseHandle(hFile);
        return start;
    }

    for(int i = start+1; i <= end; i++) {
        sprintf_s(datastr,1024*2,"bucket=%s&key=%s&partNumberStart=%d&partNumberEnd=%d&uploadId=%s&StorageNode=%s", UploadData.Bucket,UploadData.Key,start,i,UploadData.UploadId,UploadData.StorageNode);
        printf("\ndata:%s\n\n",datastr);
        str_json= cJSON_Parse(Https_Post((char *)"https://www.123pan.com/b/api/file/s3_repare_upload_parts_batch",datastr,&flag));
        if(!flag)
        {
            free(datastr);
            printf("post err");
            return start;
        }
        data=cJSON_GetObjectItem(str_json, "data");
        if (!data)
        {
            printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
            free(datastr);
            return start;
        }

        ZeroMemory(datastr,1024*2);
        sprintf_s(datastr,1024*2,"%d",start);
        presignedUrls=cJSON_GetObjectItem(data,datastr);
        if (!presignedUrls)
        {
            printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
            free(datastr);
            return start;
        }
        curl = curl_easy_init();

        if(curl) {
            // 设置要上传的URL
            curl_easy_setopt(curl, CURLOPT_URL, presignedUrls->valuestring);

            // 允许上传文件
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);


            ReadLen = 0;
            if(((UploadData.FileData.filesize)-(UploadData.SliceSize)*start)>(UploadData.SliceSize))
            {
                ReadFile(hFile,memstr,UploadData.SliceSize,&ReadLen,NULL);
                if(ReadLen!=(UploadData.SliceSize))
                {
                    printf("readfile err\n");
                    CloseHandle(hFile);
                    return start;
                }
            }
            else
            {
                ReadFile(hFile,memstr,UploadData.SliceSize,&ReadLen,NULL);
                if(ReadLen!=((UploadData.FileData.filesize)-(UploadData.SliceSize)*start))
                {
                    printf("readfile err\n");
                    CloseHandle(hFile);
                    return start;
                }
            }


            ZeroMemory(&response,sizeof(struct RESPONSE));
            response.str=memstr;
            response.maxsize=ReadLen;

            curl_easy_setopt(curl, CURLOPT_READDATA,&response);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_cb);

            // 设置文件大小
            curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, UploadData.SliceSize);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE, UploadData.SliceSize);

            // 执行上传
            res = curl_easy_perform(curl);

            // 检查错误
            if(res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                CloseHandle(hFile);
                return start;
            }

            ret=curl_easy_header(curl,"ETag",0,CURLH_HEADER,-1,&http_header);
            if(ret != CURLE_OK) {
                fprintf(stderr, "curl_easy_header() failed\n");
                CloseHandle(hFile);
                return start;
            }
            if(!memcmp(http_header->value,md5_hash(memstr,ReadLen),32))
            {
                printf("check md5 err\n");
                return start;
            }
            // 清理
            curl_easy_cleanup(curl);
            curl=NULL;
        }


        ++start;
        presignedUrls=NULL;
        str_json=NULL;
        data=NULL;
    }

    CloseHandle(hFile);
    free(datastr);
    return 0;
}


int CheckAllparts(struct UPLOADDATA UploadData,char *filepath) {
    char *datastr=(char *)malloc(1024*2);
    int flag=0,size=0;
    if(!datastr)
    {
        printf("malloc err\n");
        return -5;
    }
    sprintf_s(datastr,1024*2,"bucket=%s&key=%s&uploadId=%s&StorageNode=%s", UploadData.Bucket,UploadData.Key,UploadData.UploadId,UploadData.StorageNode);
    printf("\ndata:%s\n\n",datastr);
    cJSON *str_json= cJSON_Parse(Https_Post((char *)"https://www.123pan.com/b/api/file/s3_list_upload_parts",datastr,&flag));
    if(!flag)
    {
        free(datastr);
        cJSON_Delete(str_json);
        printf("post err");
        return -10;
    }
    cJSON *jsondata=cJSON_GetObjectItem(str_json, "data");
    if(!jsondata)
    {
        cJSON_Delete(str_json);
        free(datastr);
        printf("json err");
        return -1;
    }
    cJSON * PartsArr = cJSON_GetObjectItem(jsondata,"Parts");
    if (cJSON_IsArray(PartsArr))
    {
        int ArrLen = cJSON_GetArraySize(PartsArr);
        printf("ObjArr Len: %d\n", ArrLen);
        for (int i = 0; i < ArrLen; i++)
        {
            cJSON * SubObj = cJSON_GetArrayItem(PartsArr, i);
            if(NULL == SubObj)
            {
                continue;
            }




            if(!(cJSON_GetObjectItem(SubObj, "Size")))
            {
                cJSON_Delete(str_json);
                free(datastr);
                printf("Get cjosn err\n");
                return -3;
            }
            if(!(cJSON_GetObjectItem(SubObj, "PartNumber")))
            {
                cJSON_Delete(str_json);
                free(datastr);
                printf("Get cjosn err\n");
                return -3;
            }
            size=atoi(cJSON_GetObjectItem(SubObj, "Size")->valuestring);
            if(0==size)
            {
                cJSON_Delete(str_json);
                free(datastr);
                printf("err");
                return -4;
            }
            flag=atoi(cJSON_GetObjectItem(SubObj, "PartNumber")->valuestring);
            if(0==flag)
            {
                cJSON_Delete(str_json);
                free(datastr);
                printf("err");
                return -5;
            }
            //sprintf_s(size,sizeof(int),, UploadData.Bucket,UploadData.Key,start,i,UploadData.UploadId,UploadDa);
            if(!(memcmp(md5_hash(memstr,size),(cJSON_GetObjectItem(SubObj, "ETag")->valuestring)+1,32))) {

                UploadFileChuck(flag,flag+1,UploadData);
            }



        }


    }
    else
    {
        cJSON_Delete(str_json);
        free(datastr);
        return 0;
    }
    cJSON_Delete(str_json);
    free(datastr);
    return flag;
}

int PreUpload(char *FilePath,struct UPLOADDATA *UploadData) {

    int flag=0; //此函数返回值
    HANDLE hFile=CreateFileA(FilePath,GENERIC_READ,
                             0,//可共享读
                             NULL, OPEN_ALWAYS,//打开已经存在的文件
                             FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE) {
        printf("打开文件失败:%d\n",GetLastError());
        printf("filepath:%s\n",FilePath);
        return -18;
    }
    LARGE_INTEGER lpFileSize;
    if(0==GetFileSizeEx(hFile, &lpFileSize)) {
        printf("获取文件大小失败： %d\n",GetLastError());
        printf("filepath:%s\n",FilePath);
        CloseHandle(hFile);
        return -20;
    }
    printf("filesize:%lld\n",lpFileSize.QuadPart);
    UploadData->FileData.filesize=lpFileSize.QuadPart;
    UploadData->FileData.filepath=FilePath;
    UploadData->FileData.filename=GetFileName(FilePath);
    //计算整个文件的md5
    md5_state_t state;
    md5_byte_t digest[16];
    char md5str[16+1];
    int di;
    DWORD ReadLen = 0;

    md5_init(&state);


    int count=0;
    if(lpFileSize.QuadPart%(MEMSIZE)==0)
    {
        count=lpFileSize.QuadPart/(MEMSIZE);
    }
    else
    {
        count=lpFileSize.QuadPart/(MEMSIZE)+1;
    }
    int left=lpFileSize.QuadPart,stdlen=MEMSIZE;
    for(int i = 0; i < count; i++) {
        if(left<stdlen) stdlen=left;
        ReadFile(hFile,memstr,stdlen,&ReadLen,NULL);
        if(ReadLen!=stdlen) {
            printf("file read error\n");
            CloseHandle(hFile);
            return -30;
        }
        md5_append(&state, (const md5_byte_t *)(memstr), stdlen);
        left-=MEMSIZE;
        printf("%d\n",i);
    }
    printf("\n");
    md5_finish(&state, digest);

    for (di = 0; di < 16; ++di) sprintf(md5str + di * 2, "%02x", digest[di]);
	ZeroMemory(memstr,MEMSIZE);
    CloseHandle(hFile);

    char *datastr=(char *)malloc(1024*2);
    sprintf_s(datastr,1024*2,"driveId=0&etag=%s&fileName=%s&parentFileId=0&size=%lld&type=0", md5str,GetFileName(FilePath),lpFileSize.QuadPart);
    printf("\ndata:%s\n\n",datastr);


    cJSON *str_json= cJSON_Parse(Https_Post((char *)"https://www.123pan.com/b/api/file/upload_request",datastr,&flag));
    cJSON *data=cJSON_GetObjectItem(str_json, "data");
    if(!flag)
    {
        free(datastr);
        printf("post err");
        return flag;
    }
    if (!data)
    {
        printf("JSON格式错误:%s\n\n", cJSON_GetErrorPtr());
        free(datastr);
        return -4;
    }

    printf("Reuse:%d\n",cJSON_GetObjectItem(data, "Reuse")->valueint);
    if(cJSON_GetObjectItem(data, "Reuse")->valueint==1)
    {
        printf("%s秒传成功",FilePath);
        //ZeroMemory(lpFileSize,sizeof(lpFileSize));
        free(datastr);
        cJSON_Delete(str_json);
        return 0;
    }

    if(!cJSON_GetObjectItem(data, "Key"))
    {
        printf("GetData err\n");
        flag=1;
        goto goto_exit;
    }
    if(!cJSON_GetObjectItem(data, "Bucket"))
    {
        printf("GetData err\n");
        flag=2;
        goto goto_exit;
    }
    if(!cJSON_GetObjectItem(data, "UploadId"))
    {
        printf("GetData err\n");
        flag=3;
        goto goto_exit;
    }
    if(!cJSON_GetObjectItem(data, "StorageNode"))
    {
        printf("GetData err\n");
        flag=4;
        goto goto_exit;
    }
    if(!cJSON_GetObjectItem(data, "FileId"))
    {
        printf("GetData err\n");
        flag=5;
        goto goto_exit;
    }
    if(!cJSON_GetObjectItem(data, "SliceSize"))
    {
        printf("GetData err\n");
        flag=6;
        goto goto_exit;
    }
    UploadData->data=str_json;
    UploadData->Key=cJSON_GetObjectItem(data, "Key")->valuestring;
    UploadData->Bucket=cJSON_GetObjectItem(data, "Bucket")->valuestring;
    UploadData->UploadId=cJSON_GetObjectItem(data, "UploadId")->valuestring;
    UploadData->StorageNode=cJSON_GetObjectItem(data, "StorageNode")->valuestring;
    UploadData->SliceSize=atoi(cJSON_GetObjectItem(data, "SliceSize")->valuestring);
    UploadData->FileId=cJSON_GetObjectItem(data, "FileId")->valueint;
    if((UploadData->SliceSize)<=0)
    {
        printf("SliceSize err\n");
        flag=7;
        goto goto_exit;
    }
    if((UploadData->FileId)<=0)
    {
        printf("FileId err\n");
        flag=8;
        goto goto_exit;
    }

    if((lpFileSize.QuadPart)>(UploadData->SliceSize))
    {
        UploadData->isMultipart=1;
    }
    else
    {
        UploadData->isMultipart=0;
    }
//printf("%p\n",cJSON_GetObjectItem(data, "Key"));
    printf("Key:%s\n",cJSON_GetObjectItem(data, "Key")->valuestring);
    printf("Bucket:%s\n",cJSON_GetObjectItem(data, "Bucket")->valuestring);
    printf("UploadId:%s\n",cJSON_GetObjectItem(data, "UploadId")->valuestring);
    printf("SliceSize:%s\n",cJSON_GetObjectItem(data, "SliceSize")->valuestring);
    printf("StorageNode:%s\n",cJSON_GetObjectItem(data, "StorageNode")->valuestring);
    return 0;
goto_exit:
    cJSON_Delete(str_json);
    // cJSON_Delete(data);
    // free(str);
    //free(response.str);
    free(datastr);
    //response.maxsize=MAXRESPNSESIZE;
    // response.nowsize=0;

    //curl_easy_cleanup(hnd);
    return flag;
}


/*
此函数为完成上传，
返回值:成功->0，失败!=0
*/
int CompleteUpload(struct UPLOADDATA UploadData) {

    char *datastr=(char *)malloc(1024*2);
    sprintf_s(datastr,1024*2,"StorageNode=%s&bucket=%s&key=%s&uploadId=%s&fileId=%d&fileSize=5857&isMultipart=0",
              UploadData.StorageNode,UploadData.Bucket,UploadData.Key,UploadData.UploadId,UploadData.FileId,UploadData.FileData.filesize,UploadData.isMultipart);
    printf("\ndata:%s\n\n",datastr);
    int flag=0;

    Https_Post((char *)"https://www.123pan.com/b/api/file/upload_complete",datastr,&flag);
    free(datastr);
    return 0;
}

int main() {
    printf("started\n");
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    memstr=NULL;
    memstr=(char *)malloc(MEMSIZE);
    if(!memstr) {
        printf("mem alloc err\n");
        return 1;
    }

    struct UPLOADDATA UploadData;
    ZeroMemory(&UploadData,sizeof(struct UPLOADDATA));
    
    
    if(0==(PreUpload((char *)".\\WindowsProject2\\curl\\1",&UploadData)))
    {
       if(UploadData.data!=NULL){
           UploadFileChuck(1,2,UploadData);
           CompleteUpload(UploadData);
           cJSON_Delete(UploadData.data);
           ZeroMemory(&UploadData,sizeof(struct UPLOADDATA));
       }
    }
    
    if(0==(PreUpload((char *)".\\WindowsProject2\\x64\\Release\\WindowsProject2.exe",&UploadData)))
    {
       if(UploadData.data!=NULL){
           UploadFileChuck(1,2,UploadData);
           CompleteUpload(UploadData);
           cJSON_Delete(UploadData.data);
           ZeroMemory(&UploadData,sizeof(struct UPLOADDATA));
       }
    }
    printf("ended\n");
}