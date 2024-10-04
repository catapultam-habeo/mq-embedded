/**
* Copyright (C) 2020 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/
#define FMT_HEADER_ONLY

#include "dinput8.h"

AddressLookupTable<void> ProxyAddressLookupTable = AddressLookupTable<void>();

DirectInput8CreateProc m_pDirectInput8Create;
DllCanUnloadNowProc m_pDllCanUnloadNow;
DllGetClassObjectProc m_pDllGetClassObject;
DllRegisterServerProc m_pDllRegisterServer;
DllUnregisterServerProc m_pDllUnregisterServer;
GetdfDIJoystickProc m_pGetdfDIJoystick;

#include <Windows.h>
#include <shlwapi.h>
#include <stdexcept>

#pragma comment(lib, "Shlwapi.lib")

typedef int(__stdcall* CheckVersionProc)();

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	static HMODULE dinput8dll = nullptr;
	static HMODULE mqmaindll = nullptr;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		// Load dinput8.dll from system directory
		char sysPath[MAX_PATH];
		GetSystemDirectoryA(sysPath, MAX_PATH);
		strcat_s(sysPath, "\\dinput8.dll");
		dinput8dll = LoadLibraryA(sysPath);

		// Construct the full path to MQ2Main.dll in the current directory
		char currentDir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, currentDir);
		strcat_s(currentDir, "\\MQ2Main.dll");
		mqmaindll = LoadLibrary(currentDir);

		/*
		// Ensure the MQ2Main.dll exists in the current directory
		if (PathFileExistsA(currentDir))
		{
			mqmaindll = LoadLibraryA(currentDir);
		}
		else
		{
			ExitProcess(1);
		}

		// Get the CheckVersion method from MQ2Main.dll
		typedef int(__stdcall* CheckVersionProc)();
		CheckVersionProc checkVersion = (CheckVersionProc)GetProcAddress(mqmaindll, "CheckVersion");
		if (checkVersion == nullptr || checkVersion() != 1)
		{
			ExitProcess(1);
		}
		*/

		// DInput8 Redirecting Stuff
		m_pDirectInput8Create = (DirectInput8CreateProc)GetProcAddress(dinput8dll, "DirectInput8Create");
		m_pDllCanUnloadNow = (DllCanUnloadNowProc)GetProcAddress(dinput8dll, "DllCanUnloadNow");
		m_pDllGetClassObject = (DllGetClassObjectProc)GetProcAddress(dinput8dll, "DllGetClassObject");
		m_pDllRegisterServer = (DllRegisterServerProc)GetProcAddress(dinput8dll, "DllRegisterServer");
		m_pDllUnregisterServer = (DllUnregisterServerProc)GetProcAddress(dinput8dll, "DllUnregisterServer");
		m_pGetdfDIJoystick = (GetdfDIJoystickProc)GetProcAddress(dinput8dll, "GetdfDIJoystick");
	}
	break;

	case DLL_PROCESS_DETACH:
		FreeLibrary(dinput8dll);
		FreeLibrary(mqmaindll);
		break;

	default:
		// Optional: Handle other cases (DLL_THREAD_ATTACH, DLL_THREAD_DETACH) if necessary
		break;
	}

	return true;
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter)
{
	if (!m_pDirectInput8Create)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riidltf, ppvOut);
	}

	return hr;
}

HRESULT WINAPI DllCanUnloadNow()
{
	if (!m_pDllCanUnloadNow)
	{
		return E_FAIL;
	}

	return m_pDllCanUnloadNow();
}

HRESULT WINAPI DllGetClassObject(IN REFCLSID rclsid, IN REFIID riid, OUT LPVOID FAR* ppv)
{
	if (!m_pDllGetClassObject)
	{
		return E_FAIL;
	}

	HRESULT hr = m_pDllGetClassObject(rclsid, riid, ppv);

	if (SUCCEEDED(hr))
	{
		genericQueryInterface(riid, ppv);
	}

	return hr;
}

HRESULT WINAPI DllRegisterServer()
{
	if (!m_pDllRegisterServer)
	{
		return E_FAIL;
	}

	return m_pDllRegisterServer();
}

HRESULT WINAPI DllUnregisterServer()
{
	if (!m_pDllUnregisterServer)
	{
		return E_FAIL;
	}

	return m_pDllUnregisterServer();
}

LPCDIDATAFORMAT WINAPI GetdfDIJoystick()
{
	if (!m_pGetdfDIJoystick)
	{
		return nullptr;
	}

	return m_pGetdfDIJoystick();
}
