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
#include "Battle.h"
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
        // フィールドへの配置
        {
            for (int i = 0; i < kEnemyPoolFieldSize; i++) {
                const struct EnemyPool *pool = &enemyPoolFields[0];
                const struct EnemyData *data = &enemyDatas[pool->type];
                enemy->fields[i].type = pool->type;
                enemy->fields[i].rest = pool->rest;
                FieldGetEnemyPosition(&enemy->fields[i].position, data->fieldAction != kEnemyFieldActionFree ? true : false);
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

// エネミーアクタが何もしない
//
void EnemyActorNull(struct EnemyActor *actor)
{
    ;
}

// フィールド上にいるエネミーの種類を取得する
//
int EnemyGetFieldType(int index)
{
    return enemy != NULL ? enemy->fields[index].type : kEnemyTypeNull;
}

// フィールド上にいるエネミーの数を取得する
//
int EnemyGetFieldRest(int index)
{
    return enemy != NULL ? enemy->fields[index].rest : 0;
}

// フィールド上にいるエネミーの数を設定する
//
void EnemySetFieldRest(int index, int rest)
{
    if (enemy != NULL) {
        enemy->fields[index].rest = rest;
    }
}
// フィールド上にいるエネミーの位置を取得する
//
void EnemyGetFieldPosition(int index, struct Vector *position)
{
    if (enemy != NULL) {
        *position = enemy->fields[index].position;
    }
}


