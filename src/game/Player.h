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

    // 位置
    struct Vector position;

};

// 行動
//
enum {
    kPlayerActionIdle = 0, 
    kPlayerActionWalk, 
    kPlayerActionJump, 
    kPlayerActionFall, 
    kPlayerActionClimb, 
    kPlayerActionAttack, 
    kPlayerActionSize, 
};

// 移動
//
enum {
    kPlayerMoveOne = 0x0100, 
    kPlayerMoveShift = 8, 
    kPlayerMoveMask = 0xff, 
    kPlayerMoveWalkStart = 0x0100, 
    kPlayerMoveWalkMaximum = 0x0400, 
    kPlayerMoveWalkAccel = 0x0100, 
    kPlayerMoveWalkBrake = 0x0100, 
    kPlayerMoveJumpStart = 0x0800, 
    kPlayerMoveJumpBoost = 0x0a00, 
    kPlayerMoveFallStart = 0x0100, 
    kPlayerMoveFallMaximum = 0x0800, 
    kPlayerMoveGravity = 0x0100, 
    kPlayerMoveClimbStart = 0x0100, 
    kPlayerMoveClimbMaximum = 0x0400, 
    kPlayerMoveClimbAccel = 0x0100, 
    kPlayerMoveClimbBrake = 0x0100, 
    kPlayerMoveAttackBrake = 0x0100, 
};

// ジャンプ
//
enum {
    kPlayerJumpCount = 2, 
};

// 攻撃
//
enum {
    kPlayerAttackCount = 3, 
};

// 点滅
//
enum {
    kPlayerBlinkDamage = 30, 
    kPlayerBlinkInterval = 0x02, 
};

// プレイヤアクタ
//
struct PlayerActor {

    // アクタ
    struct Actor actor;

    // 位置
    struct Vector position;

    // 向き
    int face;

    // 行動
    int action;

    // クランク
    float crank;

    // 移動
    struct Vector moveVector;
    struct Rect moveRect;

    // ジャンプ
    int jumpCount;

    // 攻撃
    int attackCount;

    // 点滅
    int blink;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 外部参照関数
//
extern void PlayerInitialize(void);
extern void PlayerRelease(void);
extern void PlayerGetPosition(struct Vector *position);
extern void PlayerSetPosition(int x, int y);
extern void PlayerActorLoad(void);
extern void PlayerActorGetPosition(struct Vector *position);
extern void PlayerActorGetMoveRect(struct Rect *rect);
extern bool PlayerActorIsBlink(void);
extern void PlayerActorSetDamageBlink(void);

// 外部参照変数
//
extern struct Player *player;
