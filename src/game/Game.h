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

    // ダンジョン
    int dungeonIndex;
    struct Vector dungeonPosition;
    int dungeonDirection;
    struct Vector dungeonLast;
    int dungeonType;
    int dungeonRest;

    // バトル
    int battleEncount;
    struct Vector battlePosition;
    int battleType;
    int battleRest;
    int battleRoute;
    int battleDirection;

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

// カメラ
//
enum {
    kGameCameraFieldX = -200, 
    kGameCameraFieldY = -131, 
};
enum {
    kGameCameraBattleX = 0, 
    kGameCameraBattleY = 0, 
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
enum {
    kGameViewBattleSizeX = 240, 
    kGameViewBattleSizeY = 240, 
    kGameViewBattleLeft = 80, 
    kGameViewBattleTop = 0, 
    kGameViewBattleRight = 319, 
    kGameViewBattleBottom = 239, 
};


// 外部参照関数
//
extern void GameUpdate(struct Game *game);
extern bool GameIsPlay(void);
extern struct Vector *GameGetCamera(void);
extern void GameGetFieldCameraPosition(int x, int y, struct Vector *position);
extern void GameGetBattleCameraPosition(int x, int y, struct Vector *position);

