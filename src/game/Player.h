// Player.h - プレイヤ
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Define.h"


// プレイヤ
//
struct Player {

    // フィールド
    struct Vector fieldPosition;

    // バトル
    struct Vector battlePosition;

};

// 矩形
//
enum {
    kPlayerRectLeft = -12, 
    kPlayerRectTop = -23, 
    kPlayerRectRight = 11, 
    kPlayerRectBottom = 0, 
};

// プレイヤアクタ
//
struct PlayerActor {

    // アクタ
    struct Actor actor;

    // 位置
    struct Vector position;

    // 目的地
    struct Vector destination;

    // 向き
    int direction;

    // ジャンプ
    int jump;
    int step;

    // 矩形
    struct Rect rect;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// カメラ
//
enum {
    kPlayerCameraX = -200, 
    kPlayerCameraY = -131, 
};

// 移動
//
enum {
    kPlayerSpeedField = 4, 
    kPlayerSpeedBattle = 2, 
};

// 外部参照関数
//
extern void PlayerInitialize(void);
extern void PlayerRelease(void);
extern void PlayerFieldActorLoad(void);
extern void PlayerBattleActorLoad(void);
extern void PlayerGetFieldPosition(struct Vector *position);

// 外部参照変数
//
extern struct Player *player;
