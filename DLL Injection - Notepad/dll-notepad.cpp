#define DLL_EXPORT
#include "dll-notepad.h"

extern "C"
{
	DECLDIR void Share()
	{
		std::cout << "Running the dll\n";
	}
	void Keep()
	{
		MessageBox(
			NULL,
			(LPCWSTR)L"Dll injection worked!",
			(LPCWSTR)L"Notepad message",
			MB_CANCELTRYCONTINUE
		);
	}
}

bool APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)

{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// A process is loading the DLL.
		Share();
		Keep();
		break;
	case DLL_THREAD_ATTACH:
		Share();
		Keep();
		// A process is creating a new thread.
		break;
	case DLL_THREAD_DETACH:
		// A thread exits normally.
		break;
	case DLL_PROCESS_DETACH:
		// A process unloads the DLL.
		break;
	}
	return TRUE;
}