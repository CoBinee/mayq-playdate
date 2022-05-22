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
static void EnemyActorUnloadOnField(struct EnemyActor *actor);
static void EnemyActorNull(struct EnemyActor *actor);

// 内部変数
//
static struct Enemy *enemy = NULL;
static ActorFunction enemyActorFunctionFields[kEnemyFieldActionSize] = {
    (ActorFunction)EnemyActorNull, 
    (ActorFunction)EnemyFieldActorIdle, 
    (ActorFunction)EnemyFieldActorWalk, 
    (ActorFunction)EnemyFieldActorFree, 
    (ActorFunction)EnemyFieldActorStep, 
};


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
                const struct EnemyPool *pool = &enemyPoolFields[i];
                const struct EnemyData *data = &enemyDatas[pool->type];
                enemy->fields[i].type = pool->type;
                enemy->fields[i].entry = pool->entry;
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

// エネミーアクタを読み込む
//
void EnemyActorLoadOnField(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの読み込み
    for (int i = 0; i < kEnemyPoolFieldSize; i++) {
        if (enemy->fields[i].entry > 0) {

            // エネミーの取得
            const struct EnemyData *data = &enemyDatas[enemy->fields[i].type];

            // アクタの登録
            struct EnemyActor *actor = (struct EnemyActor *)ActorLoad(enemyActorFunctionFields[data->fieldAction], kGamePriorityEnemy);
            if (actor == NULL) {
                playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
            }

            // エネミーの初期化
            {
                // 解放処理の設定
                ActorSetUnload(&actor->actor, (ActorFunction)EnemyActorUnloadOnField);

                // タグの設定
                ActorSetTag(&actor->actor, kGameTagEnemy);

                // インデックスの設定
                actor->index = i;

                // データの設定
                actor->data = data;

                // 位置の設定
                actor->position = enemy->fields[i].position;
            }
        }
    }

}

// エネミーアクタを解放する
//
static void EnemyActorUnloadOnField(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// エネミーアクタが何もしない
//
static void EnemyActorNull(struct EnemyActor *actor)
{
    ;
}

// エネミーアクタを描画する
//
void EnemyActorDraw(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // カメラの取得
    struct Vector *camera = GameGetCamera();
    
    // スプライトの描画
    {
        int x = actor->position.x - camera->x;
        int y = actor->position.y - camera->y;
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, x, y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }
}
