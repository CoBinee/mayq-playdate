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

    // バトル
    int battleEncount;
    int battleType;
    int battleRest;
    int battleRoute;
    int battleDirection;
    struct Vector battlePosition;

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
    kGamePriorityDungeon = kGamePriorityField, 
    kGamePriorityBattle, 
    kGamePriorityPlayer, 
    kGamePriorityEnemy, 
};

// タグ
//
enum {
    kGameTagNull = 0, 
    kGameTagField, 
    kGameTagDungeon = kGameTagField, 
    kGameTagBattle, 
    kGameTagPlayer, 
    kGameTagEnemy, 
};

// 描画順
//
enum {
    kGameOrderNull = 0, 
    kGameOrderField, 
    kGameOrderDungeon = kGameOrderField, 
    kGameOrderBattle, 
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

