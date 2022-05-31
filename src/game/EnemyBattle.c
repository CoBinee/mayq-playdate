// EnemyBattle.c - バトルエネミー
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Battle.h"
#include "Enemy.h"

// 内部関数
//
static void EnemyBattleActorUnload(struct EnemyActor *actor);
static void EnemyBattleActorDraw(struct EnemyActor *actor);
static void EnemyBattleActorIdle(struct EnemyActor *actor);
static void EnemyBattleActorWalkRandom(struct EnemyActor *actor);
static int EnemyBattleGetWalkableDirection(struct EnemyActor *actor);
static int EnemyBattleGetWalkableRandomDirection(struct EnemyActor *actor);
static int EnemyBattleMove(struct EnemyActor *actor);
static void EnemyBattleCalcRect(struct EnemyActor *actor);

// 内部変数
//
static ActorFunction enemyBattleActorFunctions[kEnemyBattleActionSize] = {
    (ActorFunction)EnemyActorNull, 
    (ActorFunction)EnemyBattleActorIdle, 
    (ActorFunction)EnemyBattleActorWalkRandom, 
};
static const char *enemyBattleAnimationNames_Walk[kEnemyFaceSize] = {
    "WalkLeft", 
    "WalkRight", 
};


// エネミーアクタを読み込む
//
void EnemyBattleActorLoad(int type, int rest, int direction)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの読み込み
    for (int i = 0; i < rest; i++) {

        // エネミーの取得
        const struct EnemyData *data = &enemyDatas[type];

        // アクタの登録
        struct EnemyActor *actor = (struct EnemyActor *)ActorLoad(enemyBattleActorFunctions[data->battleAction], kGamePriorityEnemy);
        if (actor == NULL) {
            playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
        }

        // エネミーの初期化
        {
            // 解放処理の設定
            ActorSetUnload(&actor->actor, (ActorFunction)EnemyBattleActorUnload);

            // タグの設定
            ActorSetTag(&actor->actor, kGameTagEnemy);

            // データの設定
            actor->data = data;

            // 位置の設定
            BattleGetEnemyPosition(i, direction, &actor->position);

            // 矩形の計算
            EnemyBattleCalcRect(actor);
        }
    }
}

// エネミーアクタを解放する
//
static void EnemyBattleActorUnload(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// エネミーアクタを描画する
//
static void EnemyBattleActorDraw(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クリップの設定
    BattleSetClip();

    // スプライトの描画
    {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // クリップの解除
    BattleClearClip();
}

// エネミーアクタが待機する
//
void EnemyBattleActorIdle(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 目的地の設定
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = IocsGetRandomNumber(NULL) % 4;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // スプライトの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyBattleCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDraw, kGameOrderCharacter + actor->position.y);
}

// エネミーアクタがランダムに歩く
//
void EnemyBattleActorWalkRandom(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 目的地の設定
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = EnemyBattleGetWalkableRandomDirection(actor);

        // 体の向きの設定
        actor->face = (actor->direction == kDirectionLeft || (actor->direction != kDirectionRight && IocsGetRandomBool(NULL))) ? kEnemyFaceLeft : kEnemyFaceRight;

        // 移動の設定
        actor->moveSpeed = 0;
        actor->moveParams[0] = actor->data->battleMoveBase + (IocsGetRandomNumber(NULL) % actor->data->battleMoveRange);

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動
        {
            int distance = EnemyBattleMove(actor);
            if (distance > 0) {
                actor->moveParams[0] -= distance;
            }
            if (actor->moveParams[0] <= 0 || distance == 0) {

                // 向きの変更
                actor->direction = EnemyBattleGetWalkableRandomDirection(actor);
                int face = actor->face;
                if (actor->face == kEnemyFaceLeft && actor->direction == kDirectionRight) {
                    actor->face = kEnemyFaceRight;
                } else if (actor->face == kEnemyFaceRight && actor->direction == kDirectionLeft) {
                    actor->face = kEnemyFaceLeft;
                }

                // 移動の設定
                actor->moveParams[0] = actor->data->battleMoveBase + (IocsGetRandomNumber(NULL) % actor->data->battleMoveRange);

                // アニメーションの開始
                if (face != actor->face) {
                    AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);
                }
            }
        }

        // スプライトの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyBattleCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDraw, kGameOrderCharacter + actor->position.y);
}

// 移動できる方向を取得する
//
static int EnemyBattleGetWalkableDirection(struct EnemyActor *actor)
{
    int d = 0;
    if (BattleIsSpace(actor->moveRect.left, actor->moveRect.top - 1) && BattleIsSpace(actor->moveRect.right, actor->moveRect.top - 1)) {
        d |= (1 << kDirectionUp);
    }
    if (BattleIsSpace(actor->moveRect.left, actor->moveRect.bottom + 1) && BattleIsSpace(actor->moveRect.right, actor->moveRect.bottom + 1)) {
        d |= (1 << kDirectionDown);
    }
    if (BattleIsSpace(actor->moveRect.left - 1, actor->moveRect.top) && BattleIsSpace(actor->moveRect.left - 1, actor->moveRect.bottom)) {
        d |= (1 << kDirectionLeft);
    }
    if (BattleIsSpace(actor->moveRect.left + 1, actor->moveRect.top) && BattleIsSpace(actor->moveRect.left + 1, actor->moveRect.bottom)) {
        d |= (1 << kDirectionRight);
    }
    return d;
}
static int EnemyBattleGetWalkableRandomDirection(struct EnemyActor *actor)
{
    int direction = -1;
    int d = EnemyBattleGetWalkableDirection(actor);
    if (d == (1 << actor->direction)) {
        direction = actor->direction;
    } else {
        d &= ~(1 << actor->direction);
        direction = IocsGetRandomNumber(NULL) % 4;
        while ((d & (1 << direction)) == 0) {
            ++direction;
            if (direction >= 4) {
                direction = 0;
            }
        }
    }
    return direction;
}

// 向いている方向に移動する
//
static int EnemyBattleMove(struct EnemyActor *actor)
{
    int distance = -1;
    actor->moveSpeed += actor->data->battleSpeed;
    if (actor->moveSpeed >= kEnemyBattleSpeedOne) {
        distance = actor->moveSpeed >> kEnemyBattleSpeedShift;
        actor->moveSpeed &= kEnemyBattleSpeedMask;
        if (actor->direction == kDirectionUp) {
            int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionUp, distance, false);
            int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionUp, distance, false);
            distance = dl < dr ? dl : dr;
            actor->position.y -= distance;
        } else if (actor->direction == kDirectionDown) {
            int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionDown, distance, false);
            int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionDown, distance, false);
            distance = dl < dr ? dl : dr;
            actor->position.y += distance;
        } else if (actor->direction == kDirectionLeft) {
            int dt = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionLeft, distance, false);
            int db = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionLeft, distance, false);
            distance = dt < db ? dt : db;
            actor->position.x -= distance;
        } else if (actor->direction == kDirectionRight) {
            int dt = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionRight, distance, false);
            int db = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionRight, distance, false);
            distance = dt < db ? dt : db;
            actor->position.x += distance;
        }
    }
    return distance;
}

// 矩形を計算する
//
static void EnemyBattleCalcRect(struct EnemyActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + actor->data->rect.left;
        actor->moveRect.top = actor->position.y + actor->data->rect.top;
        actor->moveRect.right = actor->position.x + actor->data->rect.right;
        actor->moveRect.bottom = actor->position.y + actor->data->rect.bottom;
    }
}


