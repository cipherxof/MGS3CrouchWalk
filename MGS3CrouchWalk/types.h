#pragma once

#include <cstdint>

const enum PlayerMotion
{
    Stand = 0,
    Squat = 2,
    StandMoveSlow = 3,
    StandMoveStalk = 4,
    Run = 5,
    RunUpwards = 6,
    Ground = 9,
    GroundMove = 10,
    RunToRolling = 21,
    RollingToStand = 22,
    RollingToGround = 23,
    StandCover = 97,
    SquatCover = 98,
    SquatMove = 199
};

const enum MovementFlag
{
    FlagWalk = 0x20u,
    FlagSquatToGround = 0x30u,
    FlagStand = 0x40u,
};

struct MovementWork
{
    uint32_t field00;
    uint32_t field04;
    uint32_t field08;
    uint32_t field0C;
    uint32_t field10;
    uint32_t field14;
    uint32_t field18;
    uint32_t field1C;
    uint32_t field20;
    uint32_t field24;
    uint32_t field28;
    uint32_t field2C;
    uint32_t field30;
    uint32_t field34;
    uint32_t field38;
    uint32_t field3C;
    uint32_t field40;
    uint32_t field44;
    uint32_t field48;
    uint32_t field4C;
    uint32_t field50;
    uint32_t field54;
    uintptr_t action;
    uint32_t motion;
    uint32_t field64;
    uint32_t flag;
    uint16_t padCount;
    uint16_t field6C;
    uint16_t turnDir;
    uint32_t field70;
    uint32_t field74;
    uint32_t field78;
    uint32_t field7C;
    uint32_t field80;
    uint32_t field84;
    uint32_t field88;
    uint16_t padForceLimit;
};

typedef uintptr_t* __fastcall InitializeCamoIndexDelegate(int* a1, int a2);
typedef int* __fastcall CalculateCamoIndexDelegate(int* a1, int a2);
typedef int* __fastcall ActionSquatStillDelegate(__int64 work, MovementWork* plWork, __int64 a3, __int64 a4);
typedef uint32_t  __fastcall PlayerSetMotionDelegate(__int64 work, PlayerMotion motion);
typedef void __fastcall SetMotionDataDelegate(int* m_ctrl, int layer, int motion, int time, __int64 a5);
typedef __int64 __fastcall PlayerStatusCheckDelegate(unsigned int a1);
typedef __int64 __fastcall ActMovementDelegate(MovementWork* plWork, __int64 work, int flag);
typedef __int64 __fastcall GetButtonHoldingStateDelegate(__int64 work, MovementWork* plWork);