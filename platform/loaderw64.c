#pragma once

#include "platform.h"


#ifdef PLATFORM_WINDOWS
#include <Windows.h>

#if (NTDDI_VERSION >= NTDDI_WIN8) //Windows 8+ deprecates PathRemoveFileSpec... So check it before using
#include <PathCch.h>
#pragma comment(lib, "Pathcch.lib")
#else
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

#define FILE_STRLEN 1024

typedef BOOL (*pInitializePlatform_t)(Platform_t* platform);

void UnloadPlatform(Platform_t* platform);

struct _internal
{
	HMODULE library;
};

pError_t LoadPlatform(Platform_t* platform, void* appInstanceHandle)
{
	HANDLE hInstance = (HINSTANCE)appInstanceHandle;
	TCHAR filePath[FILE_STRLEN] = { 0 };
	HRESULT result = S_FALSE;
	DWORD bytesWritten;
	const TCHAR *winDll = TEXT("C:\\Source\\id-proxy\\bin\\Debug\\pwindows.dll");

	if (platform == NULL)
		return;

	memset(platform, 0, sizeof(Platform_t));

	platform->_internal = calloc(1, sizeof(struct _internal));
	if (platform->_internal == NULL)
		return P_ERROR(P_ERR_INSUFFICIENT_MEMORY);
		
	GetModuleFileName(hInstance, filePath, FILE_STRLEN);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsole(hConsole, winDll, lstrlen(winDll), &bytesWritten, NULL);

	HMODULE hLib = LoadLibrary(winDll);

	if (hLib != 0)
	{
		FARPROC proc = GetProcAddress(hLib, "InitializePlatform");
		pInitializePlatform_t pInitFunc = (pInitializePlatform_t)proc;
		pInitFunc(platform);		
	}

	platform->Unload = UnloadPlatform;

	return P_ERROR(0);
}

void UnloadPlatform(Platform_t* platform)
{
	if (platform->browserFrame.Unload != NULL)	
		platform->browserFrame.Unload(platform);	

	if (platform->restApi.Unload != NULL)
		platform->restApi.Unload(platform);

	if (platform->strings.Unload != NULL)
		platform->strings.Unload(platform);

	if (platform->_internal != NULL)	
		FreeLibrary((HMODULE)platform->_internal);	
}

#endif //PLATFORM_WINDOWS