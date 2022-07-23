// EnemyActor.c - エネミーアクタ
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
static void EnemyActorUnload(struct EnemyActor *actor);
static void EnemyActorDraw(struct EnemyActor *actor);
static void EnemyActorNull(struct EnemyActor *actor);
static void EnemyActorIdle(struct EnemyActor *actor);
static int EnemyActorGetWalkableDirection(struct EnemyActor *actor);
static int EnemyActorGetWalkableRandomDirection(struct EnemyActor *actor);
static bool EnemyActorMoveToDestination(struct EnemyActor *actor);
static bool EnemyActorBlink(struct EnemyActor *actor);
static void EnemyActorCalc(struct EnemyActor *actor);

// 内部変数
//
static ActorFunction enemyFieldActorFunctions[kEnemyActionSize] = {
    (ActorFunction)EnemyActorNull, 
    (ActorFunction)EnemyActorIdle, 
};
static const char *enemyFieldAnimationNames_Walk[kEnemyFaceSize] = {
    "WalkLeft", 
    "WalkRight", 
};


// エネミーアクタを読み込む
//
void EnemyActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの読み込み
    for (int i = 0; i < kEnemyPoolSize; i++) {

        // エネミーの取得
        const struct EnemyData *data = &enemyDatas[enemy->pools[i].type];

        // アクタの登録
        struct EnemyActor *actor = (struct EnemyActor *)ActorLoad(enemyFieldActorFunctions[data->action], kGamePriorityEnemy);
        if (actor == NULL) {
            playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
        }

        // エネミーの初期化
        {
            // 解放処理の設定
            ActorSetUnload(&actor->actor, (ActorFunction)EnemyActorUnload);

            // タグの設定
            ActorSetTag(&actor->actor, kGameTagEnemy);

            // インデックスの設定
            actor->index = i;

            // データの設定
            actor->data = data;

            // 位置の設定
            actor->position = enemy->pools[i].position;

            // 点滅の設定
            actor->blink = 0;
            
            // 計算
            EnemyActorCalc(actor);
        }
    }
}

// エネミーアクタを解放する
//
static void EnemyActorUnload(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 位置の保存
    enemy->pools[actor->index].position = actor->position;
}

// エネミーアクタを描画する
//
static void EnemyActorDraw(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クリップの設定
    FieldSetClip();

    // スプライトの描画
    if ((actor->blink & kEnemyBlinkInterval) == 0) {
        struct Vector view;
        GameGetFieldCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, actor->data->centerX, actor->data->centerY, 1.0f, 1.0f, kDrawModeCopy);
    }

    // クリップの解除
    FieldClearClip();
}

// エネミーアクタが何もしない
//
void EnemyActorNull(struct EnemyActor *actor)
{
}

// エネミーアクタが待機する
//
void EnemyActorIdle(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 向きの設定
        actor->direction = kDirectionDown;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // 計算
        EnemyActorCalc(actor);

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 点滅
        if (!EnemyActorBlink(actor)) {

            // アニメーションの更新
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }

        // 計算
        EnemyActorCalc(actor);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}

// 点滅する
//
static bool EnemyActorBlink(struct EnemyActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
    return actor->blink > 0 ? true : false;
}

// エネミーを計算する
//
static void EnemyActorCalc(struct EnemyActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + actor->data->rect.left;
        actor->moveRect.top = actor->position.y + actor->data->rect.top;
        actor->moveRect.right = actor->position.x + actor->data->rect.right;
        actor->moveRect.bottom = actor->position.y + actor->data->rect.bottom;
    }
}
