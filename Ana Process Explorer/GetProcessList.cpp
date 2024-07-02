/*	Project Name:	Ana Process Explorer
*	Written By	:	Ahmad			Siavashi 	->	Email: a.siavosh@yahoo.com,
*					Ali				Kianinejad	->	Email: af.kianinejad@gmail.com,
*				 	Farid			Amiri,
*				 	Mohammad Javad	Moein.
*	Course Title:	Principles of Programming.
*	Instructor	: 	Dr. Ali Hamze.
*	T.A			: 	Mr. Hojat Doulabi.
*	Shiraz University, Shiraz, Iran.
*	Spring 1391, 2012.
*/
// Core's library.
#include "AnaMainHeader.h"
#include "ProcessTools.h"
#include <strsafe.h>
#include <psapi.h>
#include <stdlib.h>
#include "AnaCoreMainHeader.h"
#include <string>
#include <locale>
#include <sstream>
#include <iostream>
#include <vector>
#include <fstream>


//	Holds number of counted processes.
INT CoreProcessCount;


std::string WCHARToString(const WCHAR* wstr) {
    // 获取需要的缓冲区大小
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (bufferSize == 0) {
        // 处理错误
        return "";
    }

    // 创建一个缓冲区来保存转换后的字符串
    std::string str(bufferSize, 0);

    // 进行转换
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], bufferSize, NULL, NULL);

    return str;
}

// 比较函数，用于 qsort 排序
int compareProcessNames(const void* a, const void* b) {
	struct tagProcessInfo* processA = (struct tagProcessInfo*)a;
	struct tagProcessInfo* processB = (struct tagProcessInfo*)b;
    return wcscmp(processA->ProcessName, processB->ProcessName);
}

PROCESS_MEMORY_COUNTERS_EX2 GetMemoryUsageInfo(DWORD Process_PID) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);

    PROCESS_MEMORY_COUNTERS_EX2 PMC;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, Process_PID);
    GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&PMC), sizeof(PMC));


    return PMC;
}

void InnerWriteCSV(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
	std::ofstream file(filename, std::ios::app);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return;
    }

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

// 用于修剪浮点数字符串的小数位数
std::string toFixed(float value, int decimals) {
	std::string str = std::to_string(value);
	size_t pos = str.find('.');
	if (pos != std::string::npos && pos + decimals + 1 < str.size()) {
		str = str.substr(0, pos + decimals + 1);
	}
	return str;
}


/*	Lists running processes plus
*	Some information about them. */
BOOL GetProcessList(std::string ProcessName , std::string outFilePath){
	//	Initializing the number of processes variable.
	INT NumberOfProcess=0;
	//	Creating a structure to sustain each process information.
	PROCESSENTRY32 pe32;
	//	Contains the memory statistics for a process.
	PROCESS_MEMORY_COUNTERS	pmc32;
	PROCESS_MEMORY_COUNTERS_EX2 pmc32EX2;
	// Creating a snapshot of all running processes in the system.
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	HANDLE hProcess;
	// If taking the snapshot was unsuccessful, returns FALSE.
	if(hProcessSnap==INVALID_HANDLE_VALUE)
		return FALSE;
	// Setting the size of the stuctute before using it.
	pe32.dwSize=sizeof(PROCESSENTRY32);
	//	Checking availability of processes in the snapshot.
	if(!Process32First(hProcessSnap,&pe32))
		// failure of taking snapshot or there is no process in the list.
		return FALSE;
	// Obtaining the information of each process.
	do{
		// Opens a handle to the current process.
		hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE,pe32.th32ProcessID);

        // 检查是否成功
        if (hProcess == NULL) {
            DWORD error = GetLastError();
           // std::cerr << "Failed to open process. Error code: " << error << std::endl;

            // 根据错误代码进行处理
            switch (error) {
            case ERROR_ACCESS_DENIED:
                //std::cerr << "Access denied. Try running the program as an administrator." << std::endl;
                break;
            case ERROR_INVALID_PARAMETER:
                //std::cerr << "Invalid process ID." << std::endl;
                break;
                // 处理其他可能的错误代码
            default:
               // std::cerr << "An unknown error occurred." << std::endl;
                break;
            }
        }
        else {
          //  std::cout << "Process opened successfully!" << std::endl;

            // 处理进程句柄
            // ...

            // 关闭进程句柄
            //CloseHandle(hProcess);
        }

		//	Retrieving memory infromation about the process.
		GetProcessMemoryInfo(hProcess,&pmc32,sizeof(pmc32));

        
		// 
		//pmc32EX2 = GetMemoryUsageInfo(pe32.th32ProcessID);
		//	Assigning Obtained information about each process to its specific structure.
			//	Process Name.

			
			wcscpy(PeInfo[NumberOfProcess].ProcessName,pe32.szExeFile);
			//	Process Identifier.
			PeInfo[NumberOfProcess].PID = pe32.th32ProcessID;
			//	Retrieves the path of the process.
			GetModuleFileNameEx(hProcess,NULL,PeInfo[NumberOfProcess].ProcessPath,sizeof(PeInfo[NumberOfProcess].ProcessPath));
			if(!wcscmp(TEXT(""),PeInfo[NumberOfProcess].ProcessPath))
				wcscpy(PeInfo[NumberOfProcess].ProcessPath,TEXT("N/A"));
			//	ParentID of the process.
			PeInfo[NumberOfProcess].ParentPID = pe32.th32ParentProcessID;
				/*	Memory Information of the selected process.	*/
			if(NumberOfProcess > 1){
				PeInfo[NumberOfProcess].PageFaultCount = pmc32.PageFaultCount;
				PeInfo[NumberOfProcess].PagefileUsage = pmc32.PagefileUsage;
				PeInfo[NumberOfProcess].PeakPagefileUsage = pmc32.PeakPagefileUsage;
				PeInfo[NumberOfProcess].PeakWorkingSetSize = pmc32.PeakWorkingSetSize;
				PeInfo[NumberOfProcess].QuotaNonPagedPoolUsage = pmc32.QuotaNonPagedPoolUsage;
				PeInfo[NumberOfProcess].QuotaPagedPoolUsage = pmc32.QuotaPagedPoolUsage;
				PeInfo[NumberOfProcess].QuotaPeakNonPagedPoolUsage = pmc32.QuotaPeakNonPagedPoolUsage;
				PeInfo[NumberOfProcess].QuotaPeakPagedPoolUsage = pmc32.QuotaPeakPagedPoolUsage;
				//	Obtainig Working Set Size.
				PeInfo[NumberOfProcess].WorkingSetSize = pmc32.WorkingSetSize;

                if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc32EX2), sizeof(pmc32EX2))) {
                    //EX2
                    PeInfo[NumberOfProcess].PrivateUsage = pmc32EX2.PrivateUsage;
                    PeInfo[NumberOfProcess].PrivateWorkingSetSize = pmc32EX2.PrivateWorkingSetSize;
                    PeInfo[NumberOfProcess].SharedCommitUsage = pmc32EX2.SharedCommitUsage;
                }
                else {
                    //EX2
                    PeInfo[NumberOfProcess].PrivateUsage = 0;
                    PeInfo[NumberOfProcess].PrivateWorkingSetSize = 0;
                    PeInfo[NumberOfProcess].SharedCommitUsage = 0;

                    // Handle the error
                  //  DWORD error = GetLastError();
                   // printf("Error: %lu\n", error);
                }

				
			}

			if (ProcessName != "")
			{
				std::string NString = WCHARToString(pe32.szExeFile);
				// 使用 find 函数查找子字符串
				if (NString.find(ProcessName) != std::string::npos) {
					std::cout << "The substring '" << NString << "' was found in the main string." << std::endl;
                    std::vector<std::vector<std::string>> data = {
						{toFixed(PeInfo[NumberOfProcess].PrivateUsage / (1024 * 1024.0f),2),toFixed(PeInfo[NumberOfProcess].WorkingSetSize / (1024 * 1024.0f),2), toFixed(PeInfo[NumberOfProcess].PrivateWorkingSetSize / (1024 * 1024.0f),2), toFixed(PeInfo[NumberOfProcess].SharedCommitUsage / (1024 * 1024.0f),2) }
                    };

					InnerWriteCSV(outFilePath, data);
				}
				else {

				}
			}


			//	Priority Base of process's threads.
			PeInfo[NumberOfProcess].Priority = GetPriorityClass(hProcess);
			//	Retrieving Process Cycle.
			QueryProcessCycleTime(hProcess,&PeInfo[NumberOfProcess].CycleTime);
			//	Retrieving Process String information.
			GetFileInfo(NumberOfProcess);
		// Closing a handle which was refering to an especific process in the snapshot.
		CloseHandle(hProcess);
	//	Next Process.
	++NumberOfProcess;
	}while(Process32Next(hProcessSnap,&pe32));

    // 使用 qsort 排序
    //qsort(PeInfo, NumberOfProcess, sizeof(struct tagProcessInfo), compareProcessNames);

	// Keeping number of processes.
	CoreProcessCount = NumberOfProcess;
	// Closing the handle aftrer finishing work with it.
	CloseHandle(hProcessSnap);
	// If everything went well, returns TRUE as output.
	return TRUE;
}