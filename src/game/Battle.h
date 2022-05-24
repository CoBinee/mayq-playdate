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
    kBattleSizeX = 17, 
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
    kBattleViewSizeX = 400, 
    kBattleViewSizeY = 240, 
    kBattleViewLeft = 0, 
    kBattleViewTop = 0, 
    kBattleViewRight = 399, 
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
extern void BattleActorLoad(int block);
extern unsigned char BattleGetMap(int x, int y);
extern bool BattleIsSpace(int x, int y);

