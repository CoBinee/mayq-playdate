// Battle.h - バトル
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


// バトル
//
enum {
    kBattleSizeX = 16, 
    kBattleSizeY = 10, 
    kBattleSizePixel = 24, 
};

// マップ
//
enum {
    kBattleMapNull = 0, 
    kBattleMapFloor, 
    kBattleMapBlock, 
};

// 種類
//
enum {
    kBattleTypeField = 0, 
    kBattleTypeDungeon, 
    kBattleTypeEntrance, 
    kBattleTypeSize, 
};

// エネミー
//
enum {
    kBattleEnemySize = 9, 
};

// バトル
//
struct Battle {

    // マップ
    unsigned char maps[kBattleSizeY][kBattleSizeX];

};

// 視界
//
enum {
    kBattleViewSizeX = 384, 
    kBattleViewSizeY = 240, 
    kBattleViewLeft = 8, 
    kBattleViewTop = 0, 
    kBattleViewRight = 391, 
    kBattleViewBottom = 239, 
};

// アニメーション
//
enum {
    kBattleAnimationNull = 0, 
    kBattleAnimationFloor, 
    kBattleAnimationBlock, 
    kBattleAnimationSize, 
};

// アクタ
//
struct BattleActor {

    // アクタ
    struct Actor actor;

    // アニメーション
    struct AsepriteSpriteAnimation animations[kBattleAnimationSize];

};

// 外部参照関数
//
extern void BattleInitialize(void);
extern void BattleRelease(void);
extern void BattleActorLoad(int type, int route);
extern unsigned char BattleGetMap(int x, int y);
extern bool BattleIsSpace(int x, int y);
extern int BattleGetMoveDistance(int x, int y, int direction, int speed, bool outside);
extern void BattleGetStartPosition(int direction, struct Vector *position);
extern void BattleGetEnemyPosition(int index, int direction, struct Vector *position);
extern bool BattleIsInside(int x, int y);
extern bool BattleIsInsideX(int x);
extern bool BattleIsInsideY(int y);
extern void BattleClearClip(void);
extern void BattleSetClip(void);
