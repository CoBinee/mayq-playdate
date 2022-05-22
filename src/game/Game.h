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

    // 状態
    int state;

    // カメラ
    struct Vector camera;

};

// スプライト
//
enum {
    kGameSpriteField = 0, 
    kGameSpritePlayer, 
    kGameSpriteMob, 
    kGameSpriteNameSize, 
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
};


// 外部参照関数
//
extern void GameUpdate(struct Game *game);
extern struct Vector *GameGetCamera(void);
extern void GameSetCamera(int x, int y);
extern void GameGetCameraPosition(int x, int y, struct Vector *position);

