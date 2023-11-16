#include <shlwapi.h>
#include "Memory.h"
#include "MinHook.h"
#include "ini.h"
#include "types.h"

HMODULE GameModule = GetModuleHandleA("METAL GEAR SOLID3.exe");
uintptr_t GameBase = (uintptr_t)GameModule;
uintptr_t* CamoIndexData = NULL;
uintptr_t ActSquatStillOffset = 0;
MovementWork* plWorkGlobal = NULL;
MotionControl* mCtrlGlobal = NULL;
mINI::INIStructure Config;

float CamoIndexModifier = 1.0f;
int CamoIndexValue = 0;
bool CrouchWalkEnabled = false;
bool CrouchMoving = false;
bool CrouchMovingSlow = false;
bool IgnoreButtonHold = false;

InitializeCamoIndexDelegate* InitializeCamoIndex;
CalculateCamoIndexDelegate* CalculateCamoIndex;
ActionSquatStillDelegate* ActionSquatStill;
PlayerSetMotionDelegate* PlayerSetMotionInternal;
SetMotionDataDelegate* SetMotionData;
PlayerStatusCheckDelegate* PlayerStatusCheck;
ActMovementDelegate* ActMovement;
GetButtonHoldingStateDelegate* GetButtonHoldingState;

uintptr_t PlayerSetMotion(int64_t work, PlayerMotion motion)
{
    int motionIndex = (int)motion;

    if (motionIndex > 0)
        motionIndex--;

    return PlayerSetMotionInternal(work, (PlayerMotion)motionIndex);
}

int64_t __fastcall ActMovementHook(MovementWork* plWork, int64_t work, int flag)
{
    if (plWorkGlobal != NULL && plWorkGlobal->action != ActSquatStillOffset)
    {
        CrouchWalkEnabled = false;
        CrouchMoving = false;
    }

    return ActMovement(plWork, work, flag);
}

void __fastcall SetMotionDataHook(MotionControl* motionControl, int layer, PlayerMotion motion, int time, int64_t mask)
{
    if (motionControl->mtcmControl->mtarName == 0x6891CC)
        mCtrlGlobal = motionControl;

    if (motion == PlayerMotion::StandMoveStalk && CrouchWalkEnabled)
    {
        float* currentTime = (float*)((uintptr_t)motionControl + 0x128);

        time = (int)*currentTime;
        motion = PlayerMotion::SquatMove;
    }

    SetMotionData(motionControl, layer, motion, time, mask);
}

int64_t __fastcall GetButtonHoldingStateHook(int64_t work, MovementWork* plWork)
{
    if (IgnoreButtonHold)
        return 0;

    return GetButtonHoldingState(work, plWork);
}

int* __fastcall CalculateCamoIndexHook(int* a1, int a2)
{
    int* result = CalculateCamoIndex(a1, a2);

    if (CamoIndexData == NULL || CrouchWalkEnabled || !CrouchMoving)
        return result;

    int index = a2 << 7;
    auto camoIndex = (int*)((char*)&CamoIndexData[4] + index + 4);

    if (*camoIndex >= 1000) // ignore if stealth is equipped (todo: properly check item for ezgun and spider camo)
        return result;

    *camoIndex = *camoIndex < 0 ? *camoIndex / CamoIndexModifier : *camoIndex * CamoIndexModifier;
    *camoIndex += CamoIndexValue;

    if (*camoIndex > 950) *camoIndex = 950;
    if (*camoIndex < -1000) *camoIndex = -1000;

    return result;
}

int* __fastcall ActionSquatStillHook(int64_t work, MovementWork* plWork, int64_t a3, int64_t a4)
{
    // we store this here so we don't have to hardcode another address that
    // needs to be updated with each new game patch
    plWorkGlobal = plWork;

    // process the default squatting logic while ignoring the button hold state check
    IgnoreButtonHold = true;
    int* result = ActionSquatStill(work, plWork, a3, a4);
    IgnoreButtonHold = false;

    // detect holding X while crouched to go into prone
    if (!PlayerStatusCheck(0xE0u))
    {
        auto buttonState = GetButtonHoldingState(work, plWork);

        if (buttonState == 1)
            plWork->flag |= MovementFlag::FlagStand;
        else if (buttonState == 2)
            plWork->flag |= MovementFlag::FlagSquatToGround;
    }

    // check that the pad is being held down
    int16_t padForce = *(int16_t*)(work + 2016);
    CrouchMoving = padForce > plWork->padForceLimit;
    CrouchMovingSlow = padForce < 180;

    if (CrouchMoving && !PlayerStatusCheck(0xDE)) // 0xDE seems to make sure we aren't in first person mode 
    {
        if (!CrouchWalkEnabled)
            plWork->motion = PlayerSetMotion(work, PlayerMotion::RunUpwards); // we must set the motion to something unusable on the first run, otherwise the anim won't reset properly
        else
            plWork->motion = PlayerSetMotion(work, PlayerMotion::StandMoveStalk); // hijack the stalking motion

        if (mCtrlGlobal != NULL)
            mCtrlGlobal->mtcmControl->motionTimeBase = CrouchMovingSlow ? 3.0 : 6.0;

        CrouchWalkEnabled = true;
    }

    if (CrouchWalkEnabled && (plWork->flag & MovementFlag::FlagStand) != 0)
        CrouchWalkEnabled = false;

    return result;
}

void InstallHooks()
{
    int status = MH_Initialize();

    uintptr_t actMovementOffset         = (uintptr_t)Memory::PatternScan(GameModule, "40 53 56 57 48 81 EC F0 00 00 00 48 8B 05 ?? ?? ?? 00 48 33 C4 48 89 84 24 B0 00 00 00 48 8B F9");
    uintptr_t setMotionDataOffset       = (uintptr_t)Memory::PatternScan(GameModule, "48 85 C9 0F 84 42 03 00 00 4C 8B DC 55 53 56 41");
    uintptr_t calcuateCamoIndexOffset   = (uintptr_t)Memory::PatternScan(GameModule, "48 83 EC 30 0F 29 74 24 20 48 8B F9 48 63 F2 E8") - 0x10;
    uintptr_t getBtnHoldStateOffset     = (uintptr_t)Memory::PatternScan(GameModule, "44 0F B7 8A 8E 00 00 00 4C 8B C2 66 45 85 C9 78");
    uint8_t* disableCrouchProneOffset   = Memory::PatternScan(GameModule, "00 00 7E 19 83 4F 68 10");

    ActSquatStillOffset     = (uintptr_t)Memory::PatternScan(GameModule, "4C 8B DC 55 57 41 56 49 8D 6B A1 48 81 EC 00 01");
    InitializeCamoIndex     = (InitializeCamoIndexDelegate*)Memory::PatternScan(GameModule, "85 D2 75 33 0F 57 C0 48 63 C2 48 C1 E0 07 48 8D");
    PlayerSetMotionInternal = (PlayerSetMotionDelegate*)(Memory::PatternScan(GameModule, "B9 0F 01 00 00 E8 ?? 36 FF FF 85 C0 74 2A BA FF") - 0x10);
    PlayerStatusCheck       = (PlayerStatusCheckDelegate*)Memory::PatternScan(GameModule, "8B D1 B8 01 00 00 00 83 E1 1F D3 E0 8B CA 48 C1");

    Memory::DetourFunction(actMovementOffset, (LPVOID)ActMovementHook, (LPVOID*)&ActMovement);
    Memory::DetourFunction(setMotionDataOffset, (LPVOID)SetMotionDataHook, (LPVOID*)&SetMotionData);
    Memory::DetourFunction(calcuateCamoIndexOffset, (LPVOID)CalculateCamoIndexHook, (LPVOID*)&CalculateCamoIndex);
    Memory::DetourFunction(getBtnHoldStateOffset, (LPVOID)GetButtonHoldingStateHook, (LPVOID*)&GetButtonHoldingState);
    Memory::DetourFunction(ActSquatStillOffset, (LPVOID)ActionSquatStillHook, (LPVOID*)&ActionSquatStill);

    CamoIndexData = InitializeCamoIndex(0, 0);

    DWORD oldProtect;
    VirtualProtect((LPVOID)disableCrouchProneOffset, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    disableCrouchProneOffset[7] = 0x00;
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
        return true;

    Sleep(3000); // delay, just in case
    ReadConfig();
    InstallHooks();

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
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

