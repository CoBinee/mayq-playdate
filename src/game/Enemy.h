// Enemy.h - エネミー
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Define.h"


// 種類
//
enum {
    kEnemyTypeNull = 0, 
    kEnemyTypeSkeleton, 
    kEnemyTypeSize, 
};

// 体力
//
enum {
    kEnemyLifeNull = 0, 
};

// フィールド
//
enum {
    kEnemyFieldActionNull = 0, 
    kEnemyFieldActionIdle, 
    kEnemyFieldActionWalk, 
    kEnemyFieldActionFree, 
    kEnemyFieldActionStep, 
    kEnemyFieldActionSize, 
};
enum {
    kEnemyFieldSpeed_0_5 = 0x0080, 
    kEnemyFieldSpeed_1_0 = 0x0100, 
    kEnemyFieldSpeed_2_0 = 0x0200, 
    kEnemyFieldSpeedOne = 0x0100, 
    kEnemyFieldSpeedShift = 8, 
    kEnemyFieldSpeedMask = 0xff, 
};

// バトル
//
enum {
    kEnemyBattleActionNull = 0, 
    kEnemyBattleActionIdle, 
    kEnemyBattleActionWalkRandom, 
    kEnemyBattleActionSize, 
};
enum {
    kEnemyBattleSpeed_0_5 = 0x0080, 
    kEnemyBattleSpeed_1_0 = 0x0100, 
    kEnemyBattleSpeed_2_0 = 0x0200, 
    kEnemyBattleSpeedOne = 0x0100, 
    kEnemyBattleSpeedShift = 8, 
    kEnemyBattleSpeedMask = 0xff, 
};

// データ
//
struct EnemyData {

    // 名前
    const char *name;

    // 種類
    int type;

    // 体力
    int life;

    // フィールド
    int fieldAction;
    int fieldSpeed;

    // バトル
    int battleAction;
    int battleSpeed;
    int battleMoveBase;
    int battleMoveRange;

    // 矩形
    struct Rect rect;

    // スプライト
    const char *sprite;

    // 中心
    float centerX;
    float centerY;
 
};

// プール
//
enum {
    kEnemyPoolFieldSize = 96, 
};
struct EnemyPool {

    // 種類
    int type;

    // 数
    int rest;

};

// フィールド
//
struct EnemyField {

    // 種類
    int type;

    // 数
    int rest;

    // 位置
    struct Vector position;

};

// ダンジョン
//

// エネミー
//
struct Enemy {

    // フィールド
    struct EnemyField fields[kEnemyPoolFieldSize];

};

// 体の向き
//
enum {
    kEnemyFaceLeft = 0, 
    kEnemyFaceRight, 
    kEnemyFaceSize, 
};

// 移動
//
enum {
    kEnemyMoveParamSize = 8, 
};

// 攻撃
//

// ダメージ
//
enum {
    kEnemyDamageSpeed = 5, 
};

// 点滅
//
enum {
    kEnemyBlinkEscape = 30, 
    kEnemyBlinkDamage = 30, 
    kEnemyBlinkInterval = 0x02, 
};

// エネミーアクタ
//
struct EnemyActor {

    // アクタ
    struct Actor actor;

    // インデックス
    int index;

    // データ
    const struct EnemyData *data;

    // 体力
    int life;

    // 位置
    struct Vector position;
    struct Vector origin;
    struct Vector destination;

    // 向き
    int direction;

    // 体の向き
    int face;

    // 移動
    int moveSpeed;
    int moveParams[kEnemyMoveParamSize];
    struct Rect moveRect;

    // 攻撃
    struct Rect attackRect;
    int attackDirection;

    // ダメージ
    int damagePoint;
    int damageDirection;
    int damageSpeed;

    // 点滅
    int blink;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 外部参照関数
//
extern void EnemyInitialize(void);
extern void EnemyRelease(void);
extern void EnemyActorNull(struct EnemyActor *actor);
extern int EnemyGetFieldType(int index);
extern int EnemyGetFieldRest(int index);
extern void EnemySetFieldRest(int index, int rest);
extern void EnemyGetFieldPosition(int index, struct Vector *position);
extern void EnemyFieldActorLoad(void);
extern int EnemyFieldGetHitIndex(struct Rect *rect);
extern void EnemyFieldSetEscapeBlink(int index);
extern void EnemyBattleActorLoad(int type, int rest, int direction);
extern int EnemyBattleGetRest(int type);
extern void EnemyBattleIsHitThenDamage(struct Rect *rect, int direction, int point);

// 外部参照変数
//
extern struct Enemy *enemy;
extern const struct EnemyData enemyDatas[kEnemyTypeSize];
extern const struct EnemyPool enemyPoolFields[kEnemyPoolFieldSize];

