#include <Windows.h>
#include <stdio.h>

WCHAR SERVICE_NAME[] = L"SysWOW64";

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void ServiceMain(int argc, char* argv[]);
void ControlHandler(DWORD request);

void start();

int main(int argc, char* argv[]) {
	SERVICE_TABLE_ENTRY ServiceTable[2] = { { NULL, NULL }, { NULL, NULL } };

	ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
	ServiceTable[0].lpServiceName = SERVICE_NAME;

	StartServiceCtrlDispatcher(ServiceTable);
	return 0;
}

void ServiceMain(int argc, char* argv[]) {
	ServiceStatus.dwServiceType = SERVICE_WIN32;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP; //| SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	hStatus = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler);

	if (hStatus == (SERVICE_STATUS_HANDLE)NULL)
		return;

	start();
	ExitProcess(0);
}

void ControlHandler(DWORD request) {
	switch (request) {
	//case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwWin32ExitCode = 0;
		GetTickCount();
		SetServiceStatus(hStatus, &ServiceStatus);
		return;

	default:
		break;
	}

	return;
}