// Enemy.c - エネミー
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Field.h"
#include "Enemy.h"

// 内部関数
//

// 内部変数
//
struct Enemy *enemy = NULL;


// エネミーを初期化する
//
void EnemyInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct EnemyActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: enemy actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct EnemyActor));
    }

    // エネミーの作成
    enemy = (struct Enemy *)playdate->system->realloc(NULL, sizeof (struct Enemy));
    if (enemy == NULL) {
        playdate->system->error("%s: %d: enemy instance is not created.", __FILE__, __LINE__);
    }

    // エネミーの初期化
    {
        // エネミー配置
        {
            for (int i = 0; i < kEnemyPoolSize; i++) {
                const struct EnemyPool *pool = &enemyPools[0];
                const struct EnemyData *data = &enemyDatas[pool->type];
                enemy->pools[i].type = pool->type;
                FieldGetEnemyPosition(&enemy->pools[i].position, true);
            }
        }
    }
}

// エネミーを解放する
//
void EnemyRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの解放
    if (enemy != NULL) {
        playdate->system->realloc(enemy, 0);
        enemy = NULL;
    }
}
