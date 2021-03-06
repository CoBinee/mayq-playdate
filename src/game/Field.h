// Field.h - フィールド
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


// フィールド関数
//
typedef bool (*FieldIsFunction)(int x, int y);

// 迷路
//
enum {
    kFieldMazeSizeX = 32, 
    kFieldMazeSizeY = 32, 
};

// 区画
//
enum {
    kFieldSectionSizeX = 6, 
    kFieldSectionSizeY = 4, 
};

// フィールド
//
enum {
    kFieldSizeX = kFieldMazeSizeX * kFieldSectionSizeX, 
    kFieldSizeY = kFieldMazeSizeY * kFieldSectionSizeY, 
    kFieldSizePixel = 24, 
};

// マップ
//
enum {
    kFieldMapNull = 0, 
    kFieldMapLock, 
    kFieldMapBack, 
    kFieldMapBlock, 
    kFieldMapSolid, 
    kFieldMapChecker, 
    kFieldMapLadder, 
    kFieldMapLadderGround, 
    kFieldMapIcicle, 
    kFieldMapPole, 
    kFieldMapCave00, 
    kFieldMapCave01, 
    kFieldMapCave02, 
    kFieldMapCave10, 
    kFieldMapCave11, 
    kFieldMapCaveEntrance = kFieldMapCave11, 
    kFieldMapCave12, 
    kFieldMapCastle00, 
    kFieldMapCastle01, 
    kFieldMapCastle02, 
    kFieldMapCastle03, 
    kFieldMapCastle04, 
    kFieldMapCastle05, 
    kFieldMapCastle06, 
    kFieldMapCastle10, 
    kFieldMapCastle11, 
    kFieldMapCastle12, 
    kFieldMapCastle13, 
    kFieldMapCastle14, 
    kFieldMapCastle15, 
    kFieldMapCastle16, 
    kFieldMapCastle20, 
    kFieldMapCastle21, 
    kFieldMapCastle22, 
    kFieldMapCastle23, 
    kFieldMapCastle24, 
    kFieldMapCastle25, 
    kFieldMapCastle26, 
    kFieldMapCastle30, 
    kFieldMapCastle31, 
    kFieldMapCastle32, 
    kFieldMapCastle33, 
    kFieldMapCastle34, 
    kFieldMapCastle35, 
    kFieldMapCastle36, 
    kFieldMapCastle40, 
    kFieldMapCastle41, 
    kFieldMapCastle42, 
    kFieldMapCastle43, 
    kFieldMapCastleEntrance = kFieldMapCastle43, 
    kFieldMapCastle44, 
    kFieldMapCastle45, 
    kFieldMapCastle46, 
    kFieldMapShop00, 
    kFieldMapShop01, 
    kFieldMapShop02, 
    kFieldMapShop10, 
    kFieldMapShop11, 
    kFieldMapShop12, 
    kFieldMapShop20, 
    kFieldMapShop21, 
    kFieldMapShopEntrance = kFieldMapShop21, 
    kFieldMapShop22, 
};

// ダンジョン
//
enum {
    kFieldDungeonSize = 8, 
};

// エネミー
//
enum {
    kFieldEnemySize = 8, 
};

// 配置
//
enum {
    kFieldLocationSizeX = 8, 
    kFieldLocationSizeY = 8, 
    kFieldLocationSize = kFieldLocationSizeX * kFieldLocationSizeY, 
    kFieldLocationAreaSizeX = kFieldMazeSizeX / kFieldLocationSizeX, 
    kFieldLocationAreaSizeY = kFieldMazeSizeY / kFieldLocationSizeY, 
    kFieldLocationDig = 0, 
    kFieldLocationStart, 
    kFieldLocationCave, 
    kFieldLocationCaveSize = 8, 
    kFieldLocationCastle = kFieldLocationCave + kFieldLocationCaveSize, 
    kFieldLocationShop, 
    kFieldLocationShopSize = 4, 
    kFieldLocationEnemy = kFieldLocationShop + kFieldLocationShopSize, 
};

// 洞窟
//
enum {
    kFieldCaveSizeX = 3, 
    kFieldCaveSizeY = 2, 
};

// 城
//
enum {
    kFieldCastleSizeX = 7, 
    kFieldCastleSizeY = 5, 
};

// 店
//
enum {
    kFieldShopSizeX = 3, 
    kFieldShopSizeY = 3, 
};

// フィールド
//
struct Field {

    // 乱数
    struct XorShift xorshift;

    // 迷路
    struct Maze *maze;

    // 中心
    struct Vector os[kFieldMazeSizeY][kFieldMazeSizeX];

    // マップ
    unsigned char maps[kFieldSizeY][kFieldSizeX];

    // 配置
    struct Rect locations[kFieldLocationSize];
    int locationEnemy;

};

// アニメーション
//
enum {
    kFieldAnimationNull = 0, 
    kFieldAnimationLock, 
    kFieldAnimationBack, 
    kFieldAnimationBlock, 
    kFieldAnimationSolid, 
    kFieldAnimationChecker, 
    kFieldAnimationLadder, 
    kFieldAnimationLadderGround, 
    kFieldAnimationIcicle, 
    kFieldAnimationPole, 
    kFieldAniamtionCave00, 
    kFieldAniamtionCave01, 
    kFieldAniamtionCave02, 
    kFieldAniamtionCave10, 
    kFieldAniamtionCave11, 
    kFieldAniamtionCave12, 
    kFieldAnimationCastle00, 
    kFieldAnimationCastle01, 
    kFieldAnimationCastle02, 
    kFieldAnimationCastle03, 
    kFieldAnimationCastle04, 
    kFieldAnimationCastle05, 
    kFieldAnimationCastle06, 
    kFieldAnimationCastle10, 
    kFieldAnimationCastle11, 
    kFieldAnimationCastle12, 
    kFieldAnimationCastle13, 
    kFieldAnimationCastle14, 
    kFieldAnimationCastle15, 
    kFieldAnimationCastle16, 
    kFieldAnimationCastle20, 
    kFieldAnimationCastle21, 
    kFieldAnimationCastle22, 
    kFieldAnimationCastle23, 
    kFieldAnimationCastle24, 
    kFieldAnimationCastle25, 
    kFieldAnimationCastle26, 
    kFieldAnimationCastle30, 
    kFieldAnimationCastle31, 
    kFieldAnimationCastle32, 
    kFieldAnimationCastle33, 
    kFieldAnimationCastle34, 
    kFieldAnimationCastle35, 
    kFieldAnimationCastle36, 
    kFieldAnimationCastle40, 
    kFieldAnimationCastle41, 
    kFieldAnimationCastle42, 
    kFieldAnimationCastle43, 
    kFieldAnimationCastle44, 
    kFieldAnimationCastle45, 
    kFieldAnimationCastle46, 
    kFieldAnimationShop00, 
    kFieldAnimationShop01, 
    kFieldAnimationShop02, 
    kFieldAnimationShop10, 
    kFieldAnimationShop11, 
    kFieldAnimationShop12, 
    kFieldAnimationShop20, 
    kFieldAnimationShop21, 
    kFieldAnimationShop22, 
    kFieldAnimationSize, 
};

// アクタ
//
struct FieldActor {

    // アクタ
    struct Actor actor;

    // アニメーション
    struct AsepriteSpriteAnimation *animations;

};

// 外部参照関数
//
extern void FieldInitialize(void);
extern void FieldRelease(void);
extern void FieldActorLoad(void);
extern unsigned char FieldGetMap(int x, int y);
extern bool FieldIsSpace(int x, int y);
extern bool FieldIsFall(int x, int y);
extern bool FieldIsLadder(int x, int y);
extern bool FieldIsCave(int x, int y);
extern bool FieldIsCastle(int x, int y);
extern bool FieldIsShop(int x, int y);
extern int FieldMove(int x, int y, int direction, int distance, FieldIsFunction is, struct Vector *to);
extern int FieldMoveRect(struct Rect *from, int direction, int distance, FieldIsFunction is, struct Rect *to);
extern void FieldGetStartPosition(struct Vector *position);
extern void FieldGetEnemyPosition(struct Vector *position, bool land);
extern int FieldGetCaveIndex(int x, int y);
extern void FieldGetCavePosition(int index, struct Vector *position);
extern int FieldGetShopIndex(int x, int y);
extern void FieldClearClip(void);
extern void FieldSetClip(void);

