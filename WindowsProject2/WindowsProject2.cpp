#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#pragma comment(lib,"wininet.lib")
#define TRANSFER_SIZE    1024
int main() {
    bool bResult;
DWORD statusCode=0;
DWORD dwBytesAvailable;
DWORD dwBytesRead;

    char headers[] = "Content-Type: application/x-www-form-urlencoded\r\nAuthorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3MjMzNzU5NjcsImlhdCI6MTcyMjc3MTE2NywiaWQiOjE4MzczNzg4NTcsIm1haWwiOiIiLCJuaWNrbmFtZSI6IjE4OTcyOTA4NjE3Iiwic3VwcGVyIjpmYWxzZSwidXNlcm5hbWUiOjE4OTcyOTA4NjE3LCJ2IjowfQ.taBpf9iV0FQikSPk6594pWT444HMAMQn4nXjPkZcg6M\r\nApp-Version: 3\r\nplatform: web\r\n\r\n";



    char data[] = "driveId=0&etag=d41d8cd98f00b204e9800998ecf8427e&fileName=txt1&parentFileId=0&size=0&type=0";

    printf(headers);
    printf("\n\n");
    printf(data);
    printf("\n\n");

    HINTERNET hInternet = InternetOpenA("Mozilla/5.0 (Linux; Android 12; JLH-AN00 Build/HONORJLH-AN00) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.105 Mobile Safari/537.36", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

    HINTERNET hConnect = InternetConnectA(hInternet,
                                          "www.123pan.com", INTERNET_DEFAULT_HTTPS_PORT,NULL, NULL,   INTERNET_SERVICE_HTTP, 0, 0);
    if( ! hConnect ) {
        return 1;
    }

    DWORD dwOpenRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
                               INTERNET_FLAG_KEEP_CONNECTION |
                               INTERNET_FLAG_NO_AUTH |
                               INTERNET_FLAG_NO_COOKIES |
                               INTERNET_FLAG_NO_UI |
                               //设置启用HTTPS
                               INTERNET_FLAG_SECURE |
                               INTERNET_FLAG_IGNORE_CERT_CN_INVALID|
                               INTERNET_FLAG_RELOAD;
    //初始化Request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/b/api/file/upload_request", "HTTP/1.1", NULL, NULL, dwOpenRequestFlags, 0);
    if( ! hRequest ) {
        goto GOTO_EXIT;
    }
    /*
        HttpAddRequestHeadersA(hRequest,headers,-1L,HTTP_ADDREQ_FLAG_ADD_IF_NEW);

    */
    /*
        DWORD dwFlags;
    //dwBuffLen = sizeof(DWORD);

        InternetQueryOptionA (hRequest, INTERNET_OPTION_SECURITY_FLAGS, (LPVOID)&dwFlags, NULL);
        dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
        InternetSetOption (hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof (dwFlags) );
    */
    bResult = HttpSendRequestA(hRequest, headers, -1L, data, -1L);
    if ( !bResult )
    {
        printf("errSend  errcode:%d\n",GetLastError());
        goto GOTO_EXIT;
    }


   
    HttpQueryInfoA(hConnect, HTTP_QUERY_STATUS_CODE , &statusCode, NULL, NULL);
    printf("Status Code: %lu\n", statusCode);
    //获得HTTP Response Header信息
    char szBuff[TRANSFER_SIZE];
    //DWORD dwReadSize = 2048;
    bResult = HttpQueryInfoA(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, szBuff, NULL, NULL);
    if( ! bResult ) {
        printf("errGetheaders code:%d\n",GetLastError());
        goto GOTO_EXIT;
    }
    szBuff[TRANSFER_SIZE] = '\0';
    printf("%s\n", szBuff);
    //HTTP Response 的 Body
   
    bResult = InternetQueryDataAvailable(hRequest, &dwBytesAvailable, 0, 0);
    if( ! bResult ) {
        printf("errQueryDataAvailable code:%d\n",GetLastError());
        goto GOTO_EXIT;
    }

    if( dwBytesAvailable > TRANSFER_SIZE )
    {
        printf("tool long %d \n", GetLastError());
        goto GOTO_EXIT;
    }

    
    bResult = InternetReadFile(hRequest, szBuff, dwBytesAvailable, &dwBytesRead);
    if( ! bResult ) {
        printf("errGetBody code:%d\n",GetLastError());
        goto GOTO_EXIT;
    }
    szBuff[TRANSFER_SIZE] = '\0';
    printf("%s\n", szBuff);


GOTO_EXIT:
    if( hRequest ) {
        InternetCloseHandle(hRequest);
    }
    if( hConnect ) {
        InternetCloseHandle(hConnect);
    }
    if( hInternet ) {
        InternetCloseHandle(hInternet);
    }

    return bResult;
}