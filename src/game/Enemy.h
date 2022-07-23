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

// アクション
//
enum {
    kEnemyActionNull = 0, 
    kEnemyActionIdle, 
    kEnemyActionSize, 
};

// 速度
//
enum {
    kEnemySpeed_0_5 = 0x0080, 
    kEnemySpeed_1_0 = 0x0100, 
    kEnemySpeed_2_0 = 0x0200, 
    kEnemySpeedOne = 0x0100, 
    kEnemySpeedShift = 8, 
    kEnemySpeedMask = 0xff, 
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

    // 行動
    int action;

    // 速度
    int speed;

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
    kEnemyPoolSize = 96, 
};
struct EnemyPool {

    // 種類
    int type;

    // 位置
    struct Vector position;

};

// エネミー
//
struct Enemy {

    // プール
    struct EnemyPool pools[kEnemyPoolSize];

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

// 点滅
//
enum {
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

    // 向き
    int direction;

    // 体の向き
    int face;

    // 移動
    int moveSpeed;
    int moveParams[kEnemyMoveParamSize];
    struct Rect moveRect;

    // 点滅
    int blink;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 外部参照関数
//
extern void EnemyInitialize(void);
extern void EnemyRelease(void);
extern void EnemyActorLoad(void);

// 外部参照変数
//
extern struct Enemy *enemy;
extern const struct EnemyData enemyDatas[kEnemyTypeSize];
extern const struct EnemyPool enemyPools[kEnemyPoolSize];

