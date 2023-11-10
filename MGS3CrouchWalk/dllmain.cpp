#include <shlwapi.h>
#include "Memory.h"
#include "MinHook.h"
#include "ini.h"

HMODULE GameModule = GetModuleHandleA("METAL GEAR SOLID3.exe");
uintptr_t GameBase = (uintptr_t)GameModule;
uintptr_t* CamoIndexData = NULL;
float CamoIndexModifier = 1.0f;
int CamoIndexValue = 0;
mINI::INIStructure Config;

typedef uintptr_t* __fastcall InitializeCamoIndexDelegate(uintptr_t a1, int a2);
InitializeCamoIndexDelegate* InitializeCamoIndex;

typedef uintptr_t __fastcall CalculateCamoIndexDelegate(uintptr_t a1, int a2);
CalculateCamoIndexDelegate* CalculateCamoIndex;
uintptr_t __fastcall CalculateCamoIndexHook(uintptr_t a1, int a2)
{
    uintptr_t result = CalculateCamoIndex(a1, a2);

    if (CamoIndexData != NULL)
    {
        int index = a2 << 7;
        auto camoIndex = (int*)((char*)&CamoIndexData[4] + index + 4);
        auto movementState = (int*)((char*)&CamoIndexData[4] + index + 8);

        if (*camoIndex >= 1000) // ignore if stealth is equipped
        {
            return result;
        }

        if (*movementState == 0x6002)
        {
             *camoIndex = *camoIndex < 0 ? *camoIndex / CamoIndexModifier : *camoIndex * CamoIndexModifier;
             *camoIndex += CamoIndexValue;

            if (*camoIndex > 950) *camoIndex = 950;
            if (*camoIndex < -1000) *camoIndex = -1000;
        }
    }

    return result;
}

void InstallHooks() 
{
    int status = MH_Initialize();

    uintptr_t calcuateCamoIndexOffset = (uintptr_t)Memory::PatternScan(GameModule, "48 83 EC 30 0F 29 74 24 20 48 8B F9 48 63 F2 E8") - 0x10;
    uintptr_t initializeCamoIndexOffset = (uintptr_t)Memory::PatternScan(GameModule, "85 D2 75 33 0F 57 C0 48 63 C2 48 C1 E0 07 48 8D");

    Memory::DetourFunction(calcuateCamoIndexOffset, (LPVOID)CalculateCamoIndexHook, (LPVOID*)&CalculateCamoIndex);

    InitializeCamoIndex = (InitializeCamoIndexDelegate*)initializeCamoIndexOffset;
    CamoIndexData = InitializeCamoIndex(0, 0);
}

void ReadConfig()
{
    mINI::INIFile file("MGS3CrouchWalk.ini");
    file.read(Config);
    CamoIndexModifier = std::stof(Config["Settings"]["CamoIndexModifier"]);
    CamoIndexValue = std::stoi(Config["Settings"]["CamoIndexValue"]) * 10;
}

DWORD WINAPI MainThread(LPVOID lpParam)
{
    WCHAR exePath[_MAX_PATH] = { 0 };
    GetModuleFileName(GameModule, exePath, MAX_PATH);
    WCHAR* filename = PathFindFileName(exePath);

    if (wcsncmp(filename, L"launcher.exe", 13) == 0)
    {
        return true;
    }

    Sleep(3000); // delay, just in case
    ReadConfig();
    InstallHooks();
    return true;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, NULL, MainThread, NULL, NULL, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

