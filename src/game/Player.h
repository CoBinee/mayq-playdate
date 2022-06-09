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

    // ダンジョン
    struct Vector dungeonPosition;

    // バトル
    struct Vector battlePosition;
    struct AsepriteSprite battleSprite;

};

// ジャンプ
//
enum {
    kPlayerJumpMaximum = 2, 
};

// 入る
//
enum {
    kPlayerEnterNull = 0, 
    kPlayerEnterCave, 
    kPlayerEnterCastle, 
    kPlayerEnterShop, 
};

// 攻撃
//
enum {
    kPlayerAttackMaximum = 3, 
};
enum {
    kPlayerAttackFrameSize = 4, 
};

// 点滅
//
enum {
    kPlayerBlinkEscape = 30, 
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
    struct Vector origin;
    struct Vector destination;

    // 向き
    int direction;

    // 移動
    struct Rect moveRect;

    // ジャンプ
    int jumpCount;
    int jumpStep;

    // 入る
    int enterLocation;
    int enterIndex;

    // 攻撃
    int attackCount;
    struct Rect attackRect;

    // クランク
    float crank;

    // 点滅
    int blink;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

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
extern void PlayerGetFieldPosition(struct Vector *position);
extern void PlayerSetFieldPosition(int x, int y);
extern void PlayerFieldActorLoad(void);
extern void PlayerFieldGetPosition(struct Vector *position);
extern void PlayerFieldGetMoveRect(struct Rect *rect);
extern bool PlayerFieldIsBlink(void);
extern void PlayerFieldSetEscapeBlink(void);
extern bool PlayerFieldIsEnterCave(void);
extern int PlayerFieldGetEnterCaveIndex(void);
extern bool PlayerFieldIsEnterCastle(void);
extern bool PlayerFieldIsEnterShop(void);
extern int PlayerFieldGetEnterShopIndex(void);
extern void PlayerBattleActorLoad(int direction);
extern int PlayerBattleGetDirection(void);
extern int PlayerBattleGetEscapeDirection(void);
extern void PlayerBattleGetPosition(struct Vector *position);
extern void PlayerBattleGetMoveRect(struct Rect *rect);
extern void PlayerBattleGetAttackRect(struct Rect *rect);

// 外部参照変数
//
extern struct Player *player;
