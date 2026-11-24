// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MinHook.h"
#include <iostream>

//BlackAnt's WLID Changer for GFWL
//This was a pain but I got it to redirect to C:\wlid
//Thanks to Diman, zzVertigo, TheTank20, and WitherOrNot

static DWORD(WINAPI* RegQueryValueExA_orig)(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
static DWORD(WINAPI* RegQueryValueExW_orig)(HKEY hKey, LPWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);


//DWORD WINAPI RegQueryValueExA_new(
//    HKEY hKey,
//    LPCSTR lpValueName,
//    LPDWORD lpReserved,
//    LPDWORD lpType,
//    LPBYTE lpData,
//    LPDWORD lpcbData
//) {
//    DWORD status = RegQueryValueExA_orig(
//        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData
//    );
//
//    if (lpType && *lpType == REG_SZ && lpData && lpcbData && *lpcbData > 1) {
//        std::string value((char*)lpData, *lpcbData - 1);
//
//        if (lpValueName && strcmp(lpValueName, "TargetDir") == 0)
//        {
//            std::cout << "TargetDir Found: " << value << std::endl;
//
//            const char* newValue = "C:\\wlid";
//            size_t newLen = strlen(newValue) + 1; // include null terminator
//
//            if (*lpcbData >= newLen) {
//                memcpy(lpData, newValue, newLen);
//                *lpcbData = (DWORD)newLen;
//                std::cout << "TargetDir replaced with: " << newValue << std::endl;
//            }
//            else {
//                std::cerr << "Buffer too small to replace TargetDir!" << std::endl;
//            }
//        }
//    }
//
//    return status;
//}
//
//DWORD WINAPI RegQueryValueExW_new(
//    HKEY hKey,
//    LPWSTR lpValueName,
//    LPDWORD lpReserved,
//    LPDWORD lpType,
//    LPBYTE lpData,
//    LPDWORD lpcbData
//) {
//    DWORD status = RegQueryValueExW_orig(
//        hKey, lpValueName, lpReserved, lpType, lpData, lpcbData
//    );
//
//    if (lpType && *lpType == REG_SZ && lpData && lpcbData) {
//        if (lpValueName && wcscmp(lpValueName, L"TargetDir") == 0) {
//            const wchar_t* fakeValue = L"C:\\wlid";
//            size_t lenBytes = (wcslen(fakeValue) + 1) * sizeof(wchar_t);
//
//            if (*lpcbData >= lenBytes) {
//                memcpy(lpData, fakeValue, lenBytes);
//                *lpcbData = (DWORD)lenBytes;
//
//                std::wcout << L"TargetDir overridden: " << fakeValue << std::endl;
//            }
//            else {
//                status = ERROR_MORE_DATA;
//            }
//        }
//    }
//
//    return status;
//}



std::string GetGamePathA() {
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path, len);
    size_t pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos) {
        exePath = exePath.substr(0, pos);
    }
    return exePath;
}

std::wstring GetGamePathW() {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(NULL, path, MAX_PATH);
    std::wstring exePath(path, len);
    size_t pos = exePath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        exePath = exePath.substr(0, pos);
    }
    return exePath;
}

DWORD WINAPI RegQueryValueExA_new(
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
) {
    DWORD status = RegQueryValueExA_orig(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (lpValueName && strcmp(lpValueName, "TargetDir") == 0) {
        std::string gamePath = GetGamePathA();
        DWORD requiredLen = (DWORD)(gamePath.size() + 1); // include null terminator

        if (lpcbData) *lpcbData = requiredLen; // always tell caller how big buffer is

        if (lpData && *lpcbData >= requiredLen) {
            memcpy(lpData, gamePath.c_str(), requiredLen);
            std::cout << "TargetDir overridden with game location: " << gamePath << std::endl;
            return ERROR_SUCCESS;
        }
        else {
            std::cerr << "Buffer too small to replace TargetDir!" << std::endl;
            return ERROR_MORE_DATA;
        }
    }

    return status;
}

DWORD WINAPI RegQueryValueExW_new(
    HKEY hKey,
    LPWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
) {
    DWORD status = RegQueryValueExW_orig(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);

    if (lpValueName && wcscmp(lpValueName, L"TargetDir") == 0) {
        std::wstring gamePath = GetGamePathW();
        DWORD requiredLen = (DWORD)((gamePath.size() + 1) * sizeof(wchar_t)); // include null terminator

        if (lpcbData) *lpcbData = requiredLen; // always tell caller how big buffer is

        if (lpData && *lpcbData >= requiredLen) {
            memcpy(lpData, gamePath.c_str(), requiredLen);
            std::wcout << L"TargetDir overridden with game location: " << gamePath << std::endl;
            return ERROR_SUCCESS;
        }
        else {
            std::wcerr << L"Buffer too small to replace TargetDir!" << std::endl;
            return ERROR_MORE_DATA;
        }
    }

    return status;
}

HRESULT WINAPI WinVerifyTrustEx_new(
    HWND          hwnd,
    GUID* pgActionID,
    enum WINTRUST_DATA* pWinTrustData
) {
    /*
       xlive checks signature msidcrl40.dll with WINTRUST
       By returning S_OK here it makes it think that the signature is valid.
    */
    return S_OK;
}

DWORD WINAPI Initialize(LPVOID lpThreadParameter)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONIN$", "r", stdin);
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);

    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0) {
        std::cout << "A Executable path: " << path << std::endl;
    }
    else {
        std::cerr << "Error getting path" << std::endl;
    }

    wchar_t path2[MAX_PATH];
    if (GetModuleFileNameW(NULL, path2, MAX_PATH) != 0) {
        std::wcout << "W Executable path: " << path2 << std::endl;
    }
    else {
        std::cerr << "Error getting path" << std::endl;
    }

    const wchar_t* dlls[] = { L"kernelbase.dll", L"kernel32.dll", L"advapi32.dll" };

    if (MH_Initialize() == MH_OK)
    {
        bool hookedA = false;
        bool hookedW = false;

        for (const wchar_t* dll : dlls)
        {
            if (!hookedA && MH_CreateHookApi(dll, "RegQueryValueExA", (LPVOID)RegQueryValueExA_new, (LPVOID*)&RegQueryValueExA_orig) == MH_OK)
            {
                hookedA = true;
            }

            if (!hookedW && MH_CreateHookApi(dll, "RegQueryValueExW", (LPVOID)RegQueryValueExW_new, (LPVOID*)&RegQueryValueExW_orig) == MH_OK)
            {
                hookedW = true;
            }

            if (hookedA && hookedW)
            {
                break;
            }
        }
        MH_CreateHookApi(L"wintrust.dll", "WinVerifyTrustEx", (LPVOID)WinVerifyTrustEx_new, NULL);
        if ((hookedA || hookedW) && MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
        {
            std::cout << "Successful Startup\n";
        }
    }
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, Initialize, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

