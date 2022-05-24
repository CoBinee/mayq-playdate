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
    kEnemyFieldSpeedSlow = 1, 
    kEnemyFieldSpeedNormal = 2, 
    kEnemyFieldSpeedFast = 3, 
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

    // アニメーション
    const char *animation;

};

// プール
//
enum {
    kEnemyPoolFieldSize = 1, 
};
struct EnemyPool {

    // 種類
    int type;

    // 数
    int entry;

};

// フィールド
//
struct EnemyField {

    // 種類
    int type;

    // 数
    int entry;

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

// エネミーアクタ
//
struct EnemyActor {

    // アクタ
    struct Actor actor;

    // インデックス
    int index;

    // データ
    const struct EnemyData *data;

    // 位置
    struct Vector position;

    // 目的地
    struct Vector destination;

    // 向き
    int direction;

    // 待機
    int idle;

    // 歩数
    int step;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 外部参照関数
//
extern void EnemyInitialize(void);
extern void EnemyRelease(void);
extern void EnemyActorNull(struct EnemyActor *actor);
extern void EnemyFieldActorLoad(void);

// 外部参照変数
//
extern struct Enemy *enemy;
extern const struct EnemyData enemyDatas[kEnemyTypeSize];
extern const struct EnemyPool enemyPoolFields[kEnemyPoolFieldSize];

