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
        .life = 3, 
        .action = kEnemyActionIdle, 
        .speed = kEnemySpeed_0_5, 
        .rect.left = -12, 
        .rect.top = -23, 
        .rect.right = 11, 
        .rect.bottom = 0, 
        .sprite = "skeleton", 
        .centerX = 0.5f, 
        .centerY = 1.0f, 
    }, 
};

// フィールドプール
//
const struct EnemyPool enemyPools[kEnemyPoolSize] = {
    {.type = kEnemyTypeSkeleton, }, 
};


