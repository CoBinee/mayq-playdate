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

// バトル
//
enum {
    kEnemyBattleActionNull = 0, 
    kEnemyBattleActionIdle, 
    kEnemyBattleActionWalkRandom, 
    kEnemyBattleActionSize, 
};
enum {
    kEnemyBattleSpeedSlow = 1, 
    kEnemyBattleSpeedNormal = 2, 
    kEnemyBattleSpeedFast = 3, 
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

    // 矩形
    struct Rect rect;

    // スプライト
    const char *sprite;

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

    // 体の向き
    int face;

    // 移動
    int moveStep;
    struct Rect moveRect;

    // アニメーション
    struct AsepriteSpriteAnimation animation;

};

// 外部参照関数
//
extern void EnemyInitialize(void);
extern void EnemyRelease(void);
extern void EnemyActorNull(struct EnemyActor *actor);
extern void EnemyFieldActorLoad(void);
extern void EnemyBattleActorLoad(int type, int rest, int direction);

// 外部参照変数
//
extern struct Enemy *enemy;
extern const struct EnemyData enemyDatas[kEnemyTypeSize];
extern const struct EnemyPool enemyPoolFields[kEnemyPoolFieldSize];

