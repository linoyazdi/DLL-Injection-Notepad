#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>

#define BUFSIZE 4096
#define EXE_NAME_SIZE 261

int getProcessIdByName(PCSTR name);

int main()
{
	// Get full path of DLL to inject
	char buffer[BUFSIZE] = { 0 };
	TCHAR** lppPart = { NULL };
	LPCSTR fileName = "DLL Injection - Notepad";

	DWORD pathLen = GetFullPathNameA(fileName,
		BUFSIZE,
		buffer,
		(LPSTR*)lppPart);

	// Get LoadLibrary function address –
	// the address doesn't change at remote process
	PVOID addrLoadLibrary = (PVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	
	// Open remote process
	std::cout << "id: "<< getProcessIdByName("notepad.exe") << "\n";
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, getProcessIdByName("notepad.exe"));
	
	if (NULL == proc) {
		std::cout << "make sure u opened notepad!\n";
		std::cout << GetLastError() << std::endl;
		return -1;
	}

	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	PVOID memAddr = (PVOID)VirtualAllocEx(proc, NULL, pathLen, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	
	if (NULL == memAddr) {
		std::cout << GetLastError() << std::endl;
		return -1;
	}

	// Write DLL name to remote process memory
	int check = WriteProcessMemory(proc, memAddr, buffer, pathLen, NULL);
	
	if (0 == check) {
		std::cout << GetLastError() << std::endl;
		return -1;
	}

	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE hRemote = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)addrLoadLibrary, memAddr, 0, NULL);
	
	if (NULL == hRemote) {
		std::cout << GetLastError() << std::endl;
		return -1;
	}

	WaitForSingleObject(hRemote, INFINITE);
	check = CloseHandle(hRemote);

	if (!check)
	{
		std::cout << GetLastError() << std::endl;
		return -1;
	}

	return 0;
}


int getProcessIdByName(PCSTR name)
{
	DWORD pid = 0;
	std::string exeName;
	char DefChar = ' ';

	// Create toolhelp snapshot.
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);
	
	// Walkthrough all processes.
	if (Process32First(snapshot, &process))
	{
		do
		{
			// Comparing the processes name to the traget process name 
			char exeNameBuff[EXE_NAME_SIZE] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, process.szExeFile, -1, exeNameBuff, EXE_NAME_SIZE - 1, &DefChar, NULL);
			exeName = std::string(exeNameBuff);

			if (exeName == std::string(name))
			{
				pid = process.th32ProcessID;
				break;
			}

		} while (Process32Next(snapshot, &process));
	}

	CloseHandle(snapshot);

	return pid;
}