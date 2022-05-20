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
    kFieldMapWall, 
    kFieldMapBlock, 
    kFieldMapLadder, 
    kFieldMapIcicle, 
    kFieldMapPole, 
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
    kFieldLocationMazeSizeX = 4, 
    kFieldLocationMazeSizeY = 4, 
    kFieldLocationStart = 0, 
    kFieldLocationDungeon, 
    kFieldLocationDungeonSize = 8, 
    kFieldLocationEnemy = kFieldLocationDungeon + kFieldLocationDungeonSize, 
    kFieldLocationEnemtSize = 8, 
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

};

// 視界
//
enum {
    kFieldViewSizeX = 400, 
    kFieldViewSizeY = 240, 
    kFieldViewLeft = 0, 
    kFieldViewTop = 0, 
    kFieldViewRight = 399, 
    kFieldViewBottom = 239, 
};

// アニメーション
//
enum {
    kFieldAnimationNull = 0, 
    kFieldAnimationWall, 
    kFieldAnimationBlock, 
    kFieldAnimationLadder, 
    kFieldAnimationIcicle, 
    kFieldAnimationPole, 
    kFieldAnimationSize, 
};

// アクタ
//
struct FieldActor {

    // アクタ
    struct Actor actor;

    // アニメーション
    struct AsepriteSpriteAnimation animations[kFieldAnimationSize];

};

// 外部参照関数
//
extern void FieldInitialize(void);
extern void FieldRelease(void);
extern void FieldActorLoad(void);
extern unsigned char FieldGetMap(int x, int y);
extern bool FieldIsSpace(int x, int y);
extern bool FieldIsClimb(int x, int y);
extern bool FieldIsLand(int x, int y);
extern bool FieldIsFall(int x, int y);
extern bool FieldIsWalk(int x, int y, int direction, struct Vector *move);
extern bool FieldIsWalkAndJump(int x, int y, int direction, bool jump, struct Vector *move);
extern void FieldGetStartPosition(struct Vector *position);

