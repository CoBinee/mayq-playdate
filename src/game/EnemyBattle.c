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

        // 矩形の計算
        EnemyBattleCalcRect(actor);

        // スプライトの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

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
        actor->move = 0;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動
        {
            bool move = false;
            if (actor->move > 0) {
                if (actor->direction == kDirectionUp) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionUp, actor->data->battleSpeed);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionUp, actor->data->battleSpeed);
                    if (dl > 0 && dr > 0) {
                        actor->position.y -= dl < dr ? dl : dr;
                        move = true;
                    } else {
                        actor->move = 0;
                    }
                } else if (actor->direction == kDirectionDown) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionDown, actor->data->battleSpeed);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionDown, actor->data->battleSpeed);
                    if (dl > 0 && dr > 0) {
                        actor->position.y += dl < dr ? dl : dr;
                        move = true;
                    } else {
                        actor->move = 0;
                    }
                } else if (actor->direction == kDirectionLeft) {
                    int dt = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionLeft, actor->data->battleSpeed);
                    int db = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionLeft, actor->data->battleSpeed);
                    if (dt > 0 && db > 0) {
                        actor->position.x -= dt < db ? dt : db;
                        move = true;
                    } else {
                        actor->move = 0;
                    }
                } else if (actor->direction == kDirectionRight) {
                    int dt = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionRight, actor->data->battleSpeed);
                    int db = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionRight, actor->data->battleSpeed);
                    if (dt > 0 && db > 0) {
                        actor->position.x += dt < db ? dt : db;
                        move = true;
                    } else {
                        actor->move = 0;
                    }
                }
                --actor->move;
            }
            if (actor->move <= 0) {

                // 向きの変更
                actor->direction = EnemyBattleGetWalkableRandomDirection(actor);
                int face = actor->face;
                if (actor->face == kEnemyFaceLeft && actor->direction == kDirectionRight) {
                    actor->face = kEnemyFaceRight;
                } else if (actor->face == kEnemyFaceRight && actor->direction == kDirectionLeft) {
                    actor->face = kEnemyFaceLeft;
                }

                // 移動の設定
                actor->move = 16 + (IocsGetRandomNumber(NULL) % 16);

                // アニメーションの開始
                if (face != actor->face) {
                    AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);
                }
            }

            if (move) {
                AsepriteUpdateSpriteAnimation(&actor->animation);
            }
        }


        // 矩形の計算
        EnemyBattleCalcRect(actor);

        // スプライトの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDraw, kGameOrderCharacter + actor->position.y);
}

// 移動できる方向を取得する
//
static int EnemyBattleGetWalkableDirection(struct EnemyActor *actor)
{
    int d = 0;
    if (BattleIsSpace(actor->rect.left, actor->rect.top - 1) && BattleIsSpace(actor->rect.right, actor->rect.top - 1)) {
        d |= (1 << kDirectionUp);
    }
    if (BattleIsSpace(actor->rect.left, actor->rect.bottom + 1) && BattleIsSpace(actor->rect.right, actor->rect.bottom + 1)) {
        d |= (1 << kDirectionDown);
    }
    if (BattleIsSpace(actor->rect.left - 1, actor->rect.top) && BattleIsSpace(actor->rect.left - 1, actor->rect.bottom)) {
        d |= (1 << kDirectionLeft);
    }
    if (BattleIsSpace(actor->rect.left + 1, actor->rect.top) && BattleIsSpace(actor->rect.left + 1, actor->rect.bottom)) {
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

// 矩形を計算する
//
static void EnemyBattleCalcRect(struct EnemyActor *actor)
{
    actor->rect.left = actor->position.x + kEnemyRectLeft;
    actor->rect.top = actor->position.y + kEnemyRectTop;
    actor->rect.right = actor->position.x + kEnemyRectRight;
    actor->rect.bottom = actor->position.y + kEnemyRectBottom;
}


