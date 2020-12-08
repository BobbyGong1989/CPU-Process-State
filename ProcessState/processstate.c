#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>	//进程快照函数头文件
#include <psapi.h>
#include <assert.h>
#include "processstate.h"
#include "RBTree.h"
#include  <time.h>
RBRoot *_GLOBAL_PROCESSES;

PROCESSENTRY32 getProcessByName(const char *filename) {
	PROCESSENTRY32 currentProcess;
	currentProcess.dwSize = sizeof(currentProcess);
	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	BOOL bMore = Process32First(hProcess, &currentProcess);	//获取第一个进程信息
	while (bMore) {
		if (strcmp(currentProcess.szExeFile, filename) == 0) {
			break;
		} else {
			bMore = Process32Next(hProcess, &currentProcess);	//遍历下一个
		}
	}
	return currentProcess;
}

void killProcessByName(const char *filename) {
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes) {
		if (strcmp(pEntry.szExeFile, filename) == 0) {
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, (DWORD) pEntry.th32ProcessID);
			if (hProcess != NULL) {
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}

/// 时间转换
uint64_t file_time_2_utc(const FILETIME *ftime) {
	LARGE_INTEGER li;
	assert(ftime);
	li.LowPart = ftime->dwLowDateTime;
	li.HighPart = ftime->dwHighDateTime;
	return li.QuadPart;
}

/// 获得CPU的核数
int get_processor_number() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return (int) info.dwNumberOfProcessors;
}

void logProcessTime(RBNode *_THIS_PROCESS, char *key, long long last_time_, long long last_system_time_) {
	if (_THIS_PROCESS == NULL) {
		_THIS_PROCESS = malloc(sizeof(RBNode));
		_THIS_PROCESS->last_time_ = last_time_;
		_THIS_PROCESS->last_system_time_ = last_system_time_;
		_THIS_PROCESS->key = key;
		_THIS_PROCESS->left = NULL;    //left;
		_THIS_PROCESS->right = NULL;   //right;
		_THIS_PROCESS->parent = NULL;  //parent;
		_THIS_PROCESS->color = BLACK;
		rbtree_insert(_GLOBAL_PROCESSES, _THIS_PROCESS);
	} else {
		_THIS_PROCESS->last_time_ = last_time_;
		_THIS_PROCESS->last_system_time_ = last_system_time_;
	}
}

float get_cpu_usage(HANDLE hProcess, DWORD process_id) {

	int64_t last_time_ = 0;
	int64_t last_system_time_ = 0;

	//cpu数量
	int processor_count_ = -1;

	char key[8];
	sprintf(key, "%lu", process_id);
	//上一次的时间
	RBNode *_THIS_PROCESS = rbtree_search(_GLOBAL_PROCESSES, key);
	if (_THIS_PROCESS != NULL) {
		last_time_ = _THIS_PROCESS->last_time_;
		last_system_time_ = _THIS_PROCESS->last_system_time_;
	}

	FILETIME now;
	FILETIME creation_time;
	FILETIME exit_time;
	FILETIME kernel_time;
	FILETIME user_time;
	int64_t system_time;
	int64_t time;
	int64_t system_time_delta;
	int64_t time_delta;

	float cpu = -1;

	if (processor_count_ == -1) {
		processor_count_ = get_processor_number();
	}

	GetSystemTimeAsFileTime(&now);

	//if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time)) {
	if (!GetProcessTimes(hProcess, &creation_time, &exit_time, &kernel_time, &user_time)) {
		return -1;
	}

	system_time = (file_time_2_utc(&kernel_time) + file_time_2_utc(&user_time)) / processor_count_;
	time = file_time_2_utc(&now);

	if ((last_system_time_ == 0) || (last_time_ == 0)) {
		char *key = malloc(sizeof(char));
		sprintf(key, "%lu", process_id);
		last_system_time_ = system_time;
		last_time_ = time;
		logProcessTime(NULL, key, last_time_, last_system_time_);
		//free(key);
		return -1;
	}

	system_time_delta = system_time - last_system_time_;
	time_delta = time - last_time_;

	assert(time_delta != 0);

	if (time_delta == 0) {
		return -1;
	}

	cpu = ((float) (system_time_delta * 100)) / time_delta;
	last_system_time_ = system_time;
	last_time_ = time;
	logProcessTime(_THIS_PROCESS, NULL, last_time_, last_system_time_);
	return cpu;
}

int main() {

	time_t tt;
	char tmpbuf[80];
	float cpu = -1;
	int run = 2;
	char text[5000] = { '\0' };

	_GLOBAL_PROCESSES = create_rbtree();
	PROCESSENTRY32 currentProcess;
	currentProcess.dwSize = sizeof(currentProcess);

	printf("This program is to monitor the Master Server CPU Usage for adexa process\n\nPlease don't close it!!!\n Running... \n");

	FILE *file = fopen("D:/Adexa/CPU_Debug.txt", "a+");
	//FILE *file = fopen("C:/Users/515333/CPU_Debug.txt", "a+");
	if (file == NULL) {
		printf("open error!\n");
		return 0;
	}

	while (run > 0) {

		tt = time(NULL);
		strftime(tmpbuf, 80, "%Y-%m-%d %H:%M:%S", localtime(&tt));
		HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcess == INVALID_HANDLE_VALUE) {
			printf("Call CreateToolhelp32Snapshot Failed!\n");
			return -1;
		}

		BOOL bMore = Process32First(hProcess, &currentProcess);	//获取第一个进程信息
		while (bMore) {
			HANDLE h_thisprocess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, (DWORD) currentProcess.th32ProcessID);
			if (h_thisprocess != NULL) {
				cpu = get_cpu_usage(h_thisprocess, currentProcess.th32ProcessID);
				if (cpu > 0) {
					char line[100];
					sprintf(line, "%s PID=%lu PName=%s CPU=%.2f \n", tmpbuf, currentProcess.th32ProcessID, currentProcess.szExeFile, cpu);

					strcat(text, line);
					memset(line, 0, sizeof line);
				}
			}
			CloseHandle(h_thisprocess);
			bMore = Process32Next(hProcess, &currentProcess);	//遍历下一个
		}

		fputs(text, file);
		fflush(file);
		memset(text, 0, sizeof text);
		CloseHandle(hProcess);
		Sleep(1000);
		run--;

		if (run == 0) {
			Sleep(1000 * 60);
			run = 2;
			destroy_rbtree(_GLOBAL_PROCESSES);
			_GLOBAL_PROCESSES = create_rbtree();

		}
	}
	fclose(file);
	system("pause");
	return 0;
}
