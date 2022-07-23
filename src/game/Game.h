// Game.h - ゲーム
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Define.h"
#include "Maze.h"


// ゲーム関数
//
typedef void (*GameFunction)(void *game);

// ゲーム
//
struct Game {

    // 処理関数
    GameFunction function;
    GameFunction transition;

    // 状態
    int state;

    // カメラ
    struct Vector camera;

    // フィールド

    // 城

    // 店
    int shopIndex;

    // プレイ中
    bool play;

};

// オーディオ
//
enum {
    kGameAudioSampleSize, 
};

// プライオリティ
//
enum {
    kGamePriorityNull = 0, 
    kGamePriorityField, 
    kGamePriorityPlayer, 
    kGamePriorityEnemy, 
};

// タグ
//
enum {
    kGameTagNull = 0, 
    kGameTagField, 
    kGameTagPlayer, 
    kGameTagEnemy, 
};

// 描画順
//
enum {
    kGameOrderNull = 0, 
    kGameOrderField, 
    kGameOrderEnemy, 
    kGameOrderPlayer, 
    kGameOrderCharacter, 
};

// カメラ
//
enum {
    kGameCameraFieldX = -200, 
    kGameCameraFieldY = -131, 
};

// 視界
//
enum {
    kGameViewFieldSizeX = 400, 
    kGameViewFieldSizeY = 240, 
    kGameViewFieldLeft = 0, 
    kGameViewFieldTop = 0, 
    kGameViewFieldRight = 399, 
    kGameViewFieldBottom = 239, 
};


// 外部参照関数
//
extern void GameUpdate(struct Game *game);
extern bool GameIsPlay(void);
extern struct Vector *GameGetCamera(void);
extern void GameGetFieldCameraPosition(int x, int y, struct Vector *position);
extern void GameDrawFieldRect(struct Rect *rect, LCDBitmapDrawMode drawmode, LCDColor color);

