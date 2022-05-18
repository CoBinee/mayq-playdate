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
    kFieldMazeSectionSizeX = 6, 
    kFieldMazeSectionSizeY = 4, 
};

// フィールド
//
enum {
    kFieldSizeX = kFieldMazeSizeX * kFieldMazeSectionSizeX, 
    kFieldSizeY = kFieldMazeSizeY * kFieldMazeSectionSizeY, 
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

// ロック
//
enum {
    kFieldLockSize = 8, 
};

// フィールド
//
struct Field {

    // 迷路
    struct Maze *maze;

    // 中心
    struct Vector os[kFieldMazeSizeY][kFieldMazeSizeX];

    // マップ
    unsigned char maps[kFieldSizeY][kFieldSizeX];

};

// 視界
//
enum {
    kFieldViewSizeX = 240, 
    kFieldViewSizeY = 240, 
    kFieldViewLeft = 0, 
    kFieldViewTop = 0, 
    kFieldViewRight = 239, 
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

