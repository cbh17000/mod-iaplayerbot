/*
 * mod-iaplayerbot — Shared type definitions
 * Include this instead of duplicating enums across headers.
 */

#pragma once

#include "Define.h"

// Bot update priority levels for the Scheduler
enum class BotPriority : uint8
{
    CRITICAL = 0,   // Combat with real player involved
    HIGH     = 1,   // Dungeon / group active
    MEDIUM   = 2,   // Quest / farming / movement
    LOW      = 3,   // Social / economy
    LOWEST   = 4    // Idle / far away
};

// Level of Detail for AI processing
enum class AILOD : uint8
{
    FULL,       // Full BT + perception + bridge allowed
    REDUCED,    // Simplified BT + reduced perception
    MINIMAL     // C++ routines only (follow, regen)
};
