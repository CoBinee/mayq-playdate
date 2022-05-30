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

    // プレイ中
    bool play;

    // カメラ
    struct Vector camera;

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
    kGamePriorityBattle = kGamePriorityField, 
    kGamePriorityPlayer, 
    kGamePriorityEnemy, 
};

// タグ
//
enum {
    kGameTagNull = 0, 
    kGameTagField, 
    kGameTagBattle = kGameTagField, 
    kGameTagPlayer, 
    kGameTagEnemy, 
};

// 描画順
//
enum {
    kGameOrderNull = 0, 
    kGameOrderField, 
    kGameOrderBattle = kGameOrderField, 
    kGameOrderEnemy, 
    kGameOrderPlayer, 
    kGameOrderCharacter, 
};


// 外部参照関数
//
extern void GameUpdate(struct Game *game);
extern bool GameIsPlay(void);
extern struct Vector *GameGetCamera(void);
extern void GameSetFieldCamera(int x, int y);
extern void GameSetBattleCamera(int x, int y);
extern void GameGetFieldCameraPosition(int x, int y, struct Vector *position);
extern void GameGetBattleCameraPosition(int x, int y, struct Vector *position);

