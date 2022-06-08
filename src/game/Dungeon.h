// Dungeon.h - ダンジョン
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Define.h"
#include "Maze.h"


// 迷路
//
enum {
    kDungeonMazeSizeX = 16, 
    kDungeonMazeSizeY = 16, 
};

// ダンジョン
//
enum {
    kDungeonSizeX = kDungeonMazeSizeX, 
    kDungeonSizeY = kDungeonMazeSizeY, 
};

// 配置
//
enum {
    kDungeonLocationSizeX = 4, 
    kDungeonLocationSizeY = 4, 
    kDungeonLocationSize = kDungeonLocationSizeX * kDungeonLocationSizeY, 
    kDungeonLocationAreaSizeX = kDungeonMazeSizeX / kDungeonLocationSizeX, 
    kDungeonLocationAreaSizeY = kDungeonMazeSizeY / kDungeonLocationSizeY, 
    kDungeonLocationDig = 0, 
    kDungeonLocationEntrance, 
    kDungeonLocationEntranceSize = 8, 
    kDungeonLocationBoss = kDungeonLocationEntrance + kDungeonLocationEntranceSize, 
    kDungeonLocationBossSize = 4, 
    kDungeonLocationEnemy = kDungeonLocationBoss + kDungeonLocationBossSize, 
};

// ダンジョン
//
struct Dungeon {

    // 乱数
    struct XorShift xorshift;

    // 迷路
    struct Maze *maze;

    // 配置
    struct Vector locations[kDungeonLocationSize];

};

// アクタ
//
struct DungeonActor {

    // アクタ
    struct Actor actor;

};

// 外部参照関数
//
extern void DungeonInitialize(void);
extern void DungeonRelease(void);
extern void DungeonActorLoad(void);
extern unsigned char DungeonGetRoute(int x, int y);
extern void DungeonGetDirectionalPosition(int x, int y, int direction, struct Vector *position);
extern void DungeonGetEntrancePosition(int entrance, struct Vector *position);

