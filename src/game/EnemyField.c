// EnemyField.c - フィールドエネミー
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
static int EnemyFieldGetWalkableDirection(struct EnemyActor *actor);
static int EnemyFieldGetWalkableRandomDirection(struct EnemyActor *actor);
static bool EnemyFieldMoveToDestination(struct EnemyActor *actor);

// 内部変数
//


// エネミーアクタがフィールドを待機する
//
void EnemyFieldActorIdle(struct EnemyActor *actor)
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
        actor->direction = kDirectionDown;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "mob", actor->data->animation, true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // スプライトの更新
    AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}

// エネミーアクタがフィールドを歩く
//
void EnemyFieldActorWalk(struct EnemyActor *actor)
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
        actor->direction = EnemyFieldGetWalkableRandomDirection(actor);

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "mob", actor->data->animation, true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // 移動の設定
    if (actor->position.x == actor->destination.x && actor->position.y == actor->destination.y) {
        static const int reverses[] = {
            kDirectionDown, 
            kDirectionUp, 
            kDirectionRight, 
            kDirectionLeft, 
        };
        int way = -1;
        int d = EnemyFieldGetWalkableDirection(actor);
        if (d == (1 << kDirectionUp)) {
            way = kDirectionUp;
        } else if (d == (1 << kDirectionDown)) {
            way = kDirectionDown;
        } else if (d == (1 << kDirectionLeft)) {
            way = kDirectionLeft;
        } else if (d == (1 << kDirectionRight)) {
            way = kDirectionRight;
        } else {
            int c = d & ~(1 << reverses[actor->direction]);
            if (c == (1 << kDirectionUp)) {
                way = kDirectionUp;
            } else if (c == (1 << kDirectionDown)) {
                way = kDirectionDown;
            } else if (c == (1 << kDirectionLeft)) {
                way = kDirectionLeft;
            } else if (c == (1 << kDirectionRight)) {
                way = kDirectionRight;
            } else {
                int t = IocsGetRandomNumber(NULL) % 4;
                while ((c & (1 << t)) == 0) {
                    ++t;
                    if (t >= 4) {
                        t = 0;
                    }
                }
                way = t;
            }
        }
        actor->direction = way;
        if (FieldWalk(actor->position.x, actor->position.y, actor->direction, false, false, &actor->destination)) {
            if (actor->position.x != actor->destination.x) {
                FieldAdjustMovePosition(&actor->position, &actor->destination);
            }
        }
    }

    // 移動
    EnemyFieldMoveToDestination(actor);

    // スプライトの更新
    AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}

// エネミーアクタがフィールドを自由に移動する
//
void EnemyFieldActorFree(struct EnemyActor *actor)
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
        actor->direction = kDirectionDown;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "mob", actor->data->animation, true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // スプライトの更新
    AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}

// エネミーアクタがフィールドをステップ移動する
//
void EnemyFieldActorStep(struct EnemyActor *actor)
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
        actor->direction = kDirectionDown;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "mob", actor->data->animation, true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // スプライトの更新
    AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}

// 移動できる方向を取得する
//
static int EnemyFieldGetWalkableDirection(struct EnemyActor *actor)
{
    int direction = 0;
    if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionUp, false, false)) {
        direction |= (1 << kDirectionUp);
    }
    if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, false, false)) {
        direction |= (1 << kDirectionDown);
    }
    if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionLeft, false, false)) {
        direction |= (1 << kDirectionLeft);
    }
    if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionRight, false, false)) {
        direction |= (1 << kDirectionRight);
    }
    return direction;
}
static int EnemyFieldGetWalkableRandomDirection(struct EnemyActor *actor)
{
    int direction = -1;
    int d = EnemyFieldGetWalkableDirection(actor);
    if (IocsGetRandomBool(NULL)) {
        if ((d & (1 << kDirectionLeft)) != 0) {
            direction = kDirectionLeft;
        } else if ((d & (1 << kDirectionRight)) != 0) {
            direction = kDirectionRight;
        }
    } else {
        if ((d & (1 << kDirectionRight)) != 0) {
            direction = kDirectionRight;
        } else if ((d & (1 << kDirectionLeft)) != 0) {
            direction = kDirectionLeft;
        }
    }
    if (direction < 0) {
        if (IocsGetRandomBool(NULL)) {
            if ((d & (1 << kDirectionUp)) != 0) {
                direction = kDirectionUp;
            } else if ((d & (1 << kDirectionDown)) != 0) {
                direction = kDirectionDown;
            }
        } else {
            if ((d & (1 << kDirectionDown)) != 0) {
                direction = kDirectionDown;
            } else if ((d & (1 << kDirectionUp)) != 0) {
                direction = kDirectionUp;
            }
        }
    }
    return direction;
}

// 目的地に向かって移動する
//
static bool EnemyFieldMoveToDestination(struct EnemyActor *actor)
{
    bool move;
    if (actor->position.x < actor->destination.x) {
        actor->position.x += actor->data->fieldSpeed;
        if (actor->position.x > actor->destination.x) {
            actor->position.x = actor->destination.x;
        }
        move = true;
    } else if (actor->position.x > actor->destination.x) {
        actor->position.x -= actor->data->fieldSpeed;
        if (actor->position.x < actor->destination.x) {
            actor->position.x = actor->destination.x;
        }
        move = true;
    }
    if (actor->position.y < actor->destination.y) {
        actor->position.y += actor->data->fieldSpeed;
        if (actor->position.y > actor->destination.y) {
            actor->position.y = actor->destination.y;
        }
        move = true;
    } else if (actor->position.y > actor->destination.y) {
        actor->position.y -= actor->data->fieldSpeed;
        if (actor->position.y < actor->destination.y) {
            actor->position.y = actor->destination.y;
        }
        move = true;
    }
    return move;
}
