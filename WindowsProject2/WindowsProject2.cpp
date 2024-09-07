#include<windows.h>
#include<stdio.h>


int main(int argc, char* argv[]) {
	PROCESS_INFORMATION lpProcessInformation;
	STARTUPINFOA lpStartupInfo;
	ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
	lpStartupInfo.cb = sizeof(lpStartupInfo);
	CreateProcessA(NULL,argv[1],NULL,NULL,FALSE,CREATE_DEFAULT_ERROR_MODE|CREATE_BREAKAWAY_FROM_JOB,NULL,NULL,&lpStartupInfo,&lpProcessInformation);
	//printf("%d\n", lpProcessInformation.dwProcessId);
	//inject(lpProcessInformation.dwProcessId);
	//ResumeThread(lpProcessInformation.hThread);
    waitForSingleObject(lpProcessInformation.hThread,INFINITE);
	DWORD exitCode = 0;
    
    GetExitCodeProcess(
        OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_QUERY_LIMITED_INFORMATION,0,lpProcessInformation.dwProcessId)
, &exitCode);
    printf("call Process exit code:d\n",exitCode);
    return exitCode;
}