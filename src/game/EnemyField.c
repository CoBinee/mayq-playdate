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
static void EnemyFieldActorUnload(struct EnemyActor *actor);
static void EnemyFieldActorDraw(struct EnemyActor *actor);
static void EnemyFieldActorIdle(struct EnemyActor *actor);
static void EnemyFieldActorWalk(struct EnemyActor *actor);
static void EnemyFieldActorFree(struct EnemyActor *actor);
static void EnemyFieldActorStep(struct EnemyActor *actor);
static int EnemyFieldGetWalkableDirection(struct EnemyActor *actor);
static int EnemyFieldGetWalkableRandomDirection(struct EnemyActor *actor);
static bool EnemyFieldMoveToDestination(struct EnemyActor *actor);
static void EnemyFieldBlink(struct EnemyActor *actor);
static void EnemyFieldCalcRect(struct EnemyActor *actor);

// 内部変数
//
static ActorFunction enemyFieldActorFunctions[kEnemyFieldActionSize] = {
    (ActorFunction)EnemyActorNull, 
    (ActorFunction)EnemyFieldActorIdle, 
    (ActorFunction)EnemyFieldActorWalk, 
    (ActorFunction)EnemyFieldActorFree, 
    (ActorFunction)EnemyFieldActorStep, 
};
static const char *enemyFieldAnimationNames_Walk[kEnemyFaceSize] = {
    "WalkLeft", 
    "WalkRight", 
};


// エネミーアクタを読み込む
//
void EnemyFieldActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの読み込み
    for (int i = 0; i < kEnemyPoolFieldSize; i++) {
        if (enemy->fields[i].rest > 0) {

            // エネミーの取得
            const struct EnemyData *data = &enemyDatas[enemy->fields[i].type];

            // アクタの登録
            struct EnemyActor *actor = (struct EnemyActor *)ActorLoad(enemyFieldActorFunctions[data->fieldAction], kGamePriorityEnemy);
            if (actor == NULL) {
                playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
            }

            // エネミーの初期化
            {
                // 解放処理の設定
                ActorSetUnload(&actor->actor, (ActorFunction)EnemyFieldActorUnload);

                // タグの設定
                ActorSetTag(&actor->actor, kGameTagEnemy);

                // インデックスの設定
                actor->index = i;

                // データの設定
                actor->data = data;

                // 位置の設定
                actor->position = enemy->fields[i].position;
                actor->origin = actor->position;
                actor->destination = actor->position;

                // 点滅の設定
                actor->blink = 0;

                // 矩形の計算
                EnemyFieldCalcRect(actor);
            }
        }
    }
}

// エネミーアクタを解放する
//
static void EnemyFieldActorUnload(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 位置の保存
    enemy->fields[actor->index].position = actor->origin;
}

// エネミーアクタを描画する
//
static void EnemyFieldActorDraw(struct EnemyActor *actor)
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

// エネミーアクタが待機する
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

        // 位置の設定
        actor->origin = actor->position;
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = kDirectionDown;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 点滅
        EnemyFieldBlink(actor);

        // アニメーションの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyFieldCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyFieldActorDraw, kGameOrderEnemy);
}

// エネミーアクタが歩く
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

        // 位置の設定
        actor->origin = actor->position;
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = EnemyFieldGetWalkableRandomDirection(actor);

        // 体の向きの設定
        if (actor->direction == kDirectionLeft || actor->direction == kDirectionUp) {
            actor->face = kEnemyFaceLeft;
        } else {
            actor->face = kEnemyFaceRight;
        }

        // 移動の設定
        actor->moveSpeed = 0;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動の完了
        if (actor->position.x == actor->destination.x && actor->position.y == actor->destination.y) {

            // 位置の保存
            actor->origin = actor->position;

            // 移動の設定
            int reverse = actor->direction ^ 0x01;
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
                int c = d & ~(1 << reverse);
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
                {
                    bool face = false;
                    if (actor->direction == kDirectionLeft && actor->face != kEnemyFaceLeft) {
                        actor->face = kEnemyFaceLeft;
                        face = true;
                    } else if (actor->direction == kDirectionRight && actor->face != kEnemyFaceRight) {
                        actor->face = kEnemyFaceRight;
                        face = true;
                    }
                    if (face) {
                        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);
                    }
                }
            }
        }

        // 移動
        EnemyFieldMoveToDestination(actor);

        // 点滅
        EnemyFieldBlink(actor);

        // アニメーションの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyFieldCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyFieldActorDraw, kGameOrderEnemy);
}

// エネミーアクタが自由に移動する
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

        // 位置の設定
        actor->origin = actor->position;
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = kDirectionDown;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 点滅
        EnemyFieldBlink(actor);

        // アニメーションの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyFieldCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyFieldActorDraw, kGameOrderEnemy);
}

// フィールドエネミーアクタがステップ移動する
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

        // 位置の設定
        actor->origin = actor->position;
        actor->destination = actor->position;

        // 向きの設定
        actor->direction = kDirectionDown;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyFieldAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 点滅
        EnemyFieldBlink(actor);

        // アニメーションの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 矩形の計算
    EnemyFieldCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyFieldActorDraw, kGameOrderEnemy);
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
    actor->moveSpeed += actor->data->fieldSpeed;
    if (actor->moveSpeed >= kEnemyFieldSpeedOne) {
        int distance = actor->moveSpeed >> kEnemyFieldSpeedShift;
        actor->moveSpeed &= kEnemyFieldSpeedMask;
        if (actor->position.x < actor->destination.x) {
            actor->position.x += distance;
            if (actor->position.x > actor->destination.x) {
                actor->position.x = actor->destination.x;
            }
            move = true;
        } else if (actor->position.x > actor->destination.x) {
            actor->position.x -= distance;
            if (actor->position.x < actor->destination.x) {
                actor->position.x = actor->destination.x;
            }
            move = true;
        }
        if (actor->position.y < actor->destination.y) {
            actor->position.y += distance;
            if (actor->position.y > actor->destination.y) {
                actor->position.y = actor->destination.y;
            }
            move = true;
        } else if (actor->position.y > actor->destination.y) {
            actor->position.y -= distance;
            if (actor->position.y < actor->destination.y) {
                actor->position.y = actor->destination.y;
            }
            move = true;
        }
    }
    return move;
}

// 点滅する
//
static void EnemyFieldBlink(struct EnemyActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
}

// 矩形を計算する
//
static void EnemyFieldCalcRect(struct EnemyActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + actor->data->rect.left;
        actor->moveRect.top = actor->position.y + actor->data->rect.top;
        actor->moveRect.right = actor->position.x + actor->data->rect.right;
        actor->moveRect.bottom = actor->position.y + actor->data->rect.bottom;
    }
}

// ヒットしたエネミーのインデックスを取得する
//
int EnemyFieldGetHitIndex(struct Rect *rect)
{
    struct EnemyActor *actor = (struct EnemyActor *)ActorFindWithTag(kGameTagEnemy);
    int index = -1;
    while (actor != NULL) {
        if (actor->moveRect.left > rect->right || actor->moveRect.right < rect->left || actor->moveRect.top > rect->bottom || actor->moveRect.bottom < rect->top) {
            ;
        } else if (actor->blink == 0) {
            index = actor->index;
            break;
        }
        actor = (struct EnemyActor *)ActorNextWithTag(&actor->actor);
    }
    return index;
}

// 指定したインデックスのエネミーを点滅させる
//
void EnemyFieldSetEscapeBlink(int index)
{
    struct EnemyActor *actor = (struct EnemyActor *)ActorFindWithTag(kGameTagEnemy);
    while (actor != NULL) {
        if (actor->index == index) {
            actor->blink = kEnemyBlinkEscape;
            break;
        }
        actor = (struct EnemyActor *)ActorNextWithTag(&actor->actor);
    }
}
