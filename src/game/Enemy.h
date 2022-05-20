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

// フィールドでの行動
//
enum {
    kEnemyFieldActionNull = 0, 
    kEnemyFieldActionStay, 
    kEnemyFieldActionWalk, 
    kEnemyFieldActionFly, 
    kEnemyFieldActionSlime, 
    kEnemyFieldActionSize, 
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

    // フィールドでの行動
    int fieldAction;

};

// プール
//
struct EnemyPool {

    // 種類
    int type;

    // 数
    int entry;

};

// フィールド
//
enum {
    kEnemyFieldSize = 8, 
};
struct EnemyField {

    // データ
    const struct EnemyData *data;

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
    struct EnemyField *fields[kEnemyFieldSize];

};

// エネミーアクタ
//
struct EnemyActor {

    // アクタ
    struct Actor actor;

    // 位置
    struct Vector position;

    // 向き
    int direction;

    // 移動
    struct Vector move;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 移動
//
enum {
    kEnemyMoveSpeedSlow = 1, 
    kEnemyMoveSpeedNormal = 2, 
    kEnemyMoveSpeedFast = 3, 
};

// 外部参照関数
//
extern void EnemyInitialize(void);
extern void EnemyRelease(void);
extern void EnemyActorLoadOnField(void);

// 外部参照変数
//
extern const struct EnemyData enemyDatas[kEnemyTypeSize];
extern const struct EnemyPool enemyPoolOnFields[kEnemyFieldSize];

