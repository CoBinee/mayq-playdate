// EnemyData.c - エネミーデータ
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Enemy.h"

// 内部関数
//

// 内部変数
//

// エネミーデータ
//
const struct EnemyData enemyDatas[kEnemyTypeSize] = {

    // null
    {
    }, 
    // スケルトン
    {
        .name = "SKELETON", 
        .type = kEnemyTypeSkeleton, 
        .life = 100, 
        .fieldAction = kEnemyFieldActionWalk, 
    }, 
};

// フィールドプール
//
const struct EnemyPool enemyPoolOnFields[kEnemyFieldSize] = {
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
    {.type = kEnemyTypeSkeleton, 1, }, 
};


