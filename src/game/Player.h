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

// ジャンプ
//
enum {
    kPlayerJumpMaximum = 2, 
};

// 攻撃
//
enum {
    kPlayerAttackMaximum = 3, 
};
enum {
    kPlayerAttackFrameSize = 4, 
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

    // 移動
    struct Rect moveRect;

    // ジャンプ
    int jumpCount;
    int jumpStep;

    // 攻撃
    int attackCount;
    struct Rect attackRect;

    // クランク
    float crank;

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
