


// WJpgMain.c

#include	<windows.h>

HANDLE		ghModule = 0;

BOOL APIENTRY DllMain( HANDLE hModule, 
                        DWORD ul_reason_for_call, 
                        LPVOID lpReserved )
{
	switch( ul_reason_for_call )
	{
	case DLL_PROCESS_ATTACH:
		ghModule = hModule;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

// eof - WJpgMain.c
