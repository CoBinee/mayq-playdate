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
    kFieldLocationCaveSize = 7, 
    kFieldLocationCastle = kFieldLocationCave + kFieldLocationCaveSize, 
    kFieldLocationEnemy
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
extern bool FieldIsBlock(int x, int y);
extern bool FieldIsLadder(int x, int y);
extern bool FieldIsCave(int x, int y);
extern bool FieldIsCastle(int x, int y);
extern bool FieldIsLand(int x, int y);
extern bool FieldIsFall(int x, int y);
extern bool FieldIsWalk(int x, int y, int direction, bool jump, bool fall);
extern bool FieldWalk(int x, int y, int direction, bool jump, bool fall, struct Vector *to);
extern void FieldAdjustMovePosition(struct Vector *from, struct Vector *to);
extern void FieldGetStartPosition(struct Vector *position);
extern void FieldGetEnemyPosition(struct Vector *position, bool land);
extern void FieldGetDirectinalPosition(int x, int y, int direction, struct Vector *position);
extern int FieldGetBattleRoute(int x, int y);
extern void FieldClearClip(void);
extern void FieldSetClip(void);

