// PlayerField.c - フィールドプレイヤ
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Field.h"
#include "Player.h"

// 内部関数
//
static void PlayerFieldActorUnload(struct PlayerActor *actor);
static void PlayerFieldActorDraw(struct PlayerActor *actor);
static void PlayerFieldActorPlay(struct PlayerActor *actor);
static void PlayerFieldBlink(struct PlayerActor *actor);;
static void PlayerFieldCalcRect(struct PlayerActor *actor);

// 内部変数
//
static const struct Rect playerFieldMoveRect = {
    .left = -12, 
    .top = -23, 
    .right = 11, 
    .bottom = 0, 
};
static const char *playerFieldAnimationNames[] = {
    "WalkUp", 
    "WalkDown", 
    "WalkLeft", 
    "WalkRight", 
};


// プレイヤアクタを読み込む
//
void PlayerFieldActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct PlayerActor *actor = (struct PlayerActor *)ActorLoad((ActorFunction)PlayerFieldActorPlay, kGamePriorityPlayer);
    if (actor == NULL) {
        playdate->system->error("%s: %d: player actor is not loaded.", __FILE__, __LINE__);
    }

    // プレイヤの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)PlayerFieldActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagPlayer);

        // 位置の設定
        actor->position = player->fieldPosition;

        // 入るの設定
        actor->enterLocation = kPlayerEnterNull;

        // 点滅の設定
        actor->blink = 0;

        // 矩形の計算
        PlayerFieldCalcRect(actor);
    }
}

// プレイヤアクタを解放する
//
static void PlayerFieldActorUnload(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 位置の保存
    player->fieldPosition = actor->origin;
}

// プレイヤアクタを描画する
//
static void PlayerFieldActorDraw(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
    
    // クリップの設定
    FieldSetClip();

    // スプライトの描画
    if ((actor->blink & kPlayerBlinkInterval) == 0) {
        struct Vector view;
        GameGetFieldCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // クリップの解除
    FieldClearClip();
}

// プレイヤアクタがプレイする
//
static void PlayerFieldActorPlay(struct PlayerActor *actor)
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

        // ジャンプの設定
        actor->jumpCount = 0;
        actor->jumpStep = 1;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerFieldAnimationNames[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動の完了
        if (actor->destination.x == actor->position.x && actor->destination.y == actor->position.y) {

            // 位置の保存
            actor->origin = actor->position;

            // 移動の操作
            bool land = FieldIsLand(actor->position.x, actor->position.y);
            int direction = actor->direction;
            if (IocsIsButtonPush(kButtonUp)) {
                bool jump = land;
                if (jump) {
                    actor->jumpCount = actor->jumpStep;
                    if (actor->jumpStep < kPlayerJumpMaximum) {
                        ++actor->jumpStep;
                    }
                } else {
                    --actor->jumpCount;
                    if (actor->jumpCount > 0) {
                        jump = true;
                    }
                }
                if (IocsIsButtonPush(kButtonLeft)) {
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionUpLeft, jump, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionUp, jump, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionLeft, jump, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionLeft;
                } else if (IocsIsButtonPush(kButtonRight)) {
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionUpRight, jump, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionUp, jump, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionRight, jump, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionRight;
                } else {
                    if (FieldIsCave(actor->position.x, actor->position.y)) {
                        actor->enterLocation = kPlayerEnterCave;
                        actor->enterIndex = FieldGetCaveIndex(actor->position.x, actor->position.y);
                    } else if (FieldIsCastle(actor->position.x, actor->position.y)) {
                        actor->enterLocation = kPlayerEnterCastle;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionUp, jump, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionUp;
                }
            } else if (IocsIsButtonPush(kButtonDown)) {
                if (IocsIsButtonPush(kButtonLeft)) {
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionDownLeft, land, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionDown, land, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionLeft, land, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionLeft;
                } else if (IocsIsButtonPush(kButtonRight)) {
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionDownRight, land, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionDown, land, true, &actor->destination)) {
                        ;
                    } else if (FieldWalk(actor->position.x, actor->position.y, kDirectionRight, land, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionRight;
                } else {
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionDown, land, true, &actor->destination)) {
                        ;
                    }
                    actor->direction = kDirectionDown;
                }
            } else if (IocsIsButtonPush(kButtonLeft)) {
                if (FieldWalk(actor->position.x, actor->position.y, kDirectionLeft, land, true, &actor->destination)) {
                    actor->direction = kDirectionLeft;
                }
            } else if (IocsIsButtonPush(kButtonRight)) {
                if (FieldWalk(actor->position.x, actor->position.y, kDirectionRight, land, true, &actor->destination)) {
                    actor->direction = kDirectionRight;
                }
            }
            if (actor->position.y == actor->destination.y) {
                if (FieldIsFall(actor->destination.x, actor->destination.y)) {
                    if (
                        actor->position.x == actor->destination.x || 
                        FieldIsFall(actor->position.x, actor->position.y)
                    ) {
                        if (FieldWalk(actor->destination.x, actor->destination.y, kDirectionDown, land, true, &actor->destination)) {
                            ;
                        }
                    }
                }
            }
            if (actor->position.x != actor->destination.x) {
                FieldAdjustMovePosition(&actor->position, &actor->destination);
            }
            if (actor->direction != direction) {
                AsepriteStartSpriteAnimation(&actor->animation, "player", playerFieldAnimationNames[actor->direction], true);
            }
            if (actor->position.y == actor->destination.y) {
                actor->jumpCount = 0;
                actor->jumpStep = 1;
            }
        }

        // 移動
        {
            bool move = false;
            if (actor->position.x < actor->destination.x) {
                actor->position.x += kPlayerSpeedField;
                if (actor->position.x > actor->destination.x) {
                    actor->position.x = actor->destination.x;
                }
                move = true;
            } else if (actor->position.x > actor->destination.x) {
                actor->position.x -= kPlayerSpeedField;
                if (actor->position.x < actor->destination.x) {
                    actor->position.x = actor->destination.x;
                }
                move = true;
            }
            if (actor->position.y < actor->destination.y) {
                actor->position.y += kPlayerSpeedField;
                if (actor->position.y > actor->destination.y) {
                    actor->position.y = actor->destination.y;
                }
                move = true;
            } else if (actor->position.y > actor->destination.y) {
                actor->position.y -= kPlayerSpeedField;
                if (actor->position.y < actor->destination.y) {
                    actor->position.y = actor->destination.y;
                }
                move = true;
            }
            if (move) {
                AsepriteUpdateSpriteAnimation(&actor->animation);
            }
        }

        // 点滅
        PlayerFieldBlink(actor);
    }

    // 矩形の計算
    PlayerFieldCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerFieldActorDraw, kGameOrderPlayer);
}

// 点滅する
//
static void PlayerFieldBlink(struct PlayerActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
}

// 矩形を計算する
//
static void PlayerFieldCalcRect(struct PlayerActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + playerFieldMoveRect.left;
        actor->moveRect.top = actor->position.y + playerFieldMoveRect.top;
        actor->moveRect.right = actor->position.x + playerFieldMoveRect.right;
        actor->moveRect.bottom = actor->position.y + playerFieldMoveRect.bottom;
    }
}

// 位置を取得する
//
void PlayerFieldGetPosition(struct Vector *position)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *position = actor->position;
    }
}

// 矩形を取得する
//
void PlayerFieldGetMoveRect(struct Rect *rect)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *rect = actor->moveRect;
    }
}

// プレイヤが点滅しているかどうかを判定する
//
bool PlayerFieldIsBlink(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL && actor->blink > 0 ? true : false;
}
// プレイヤを点滅させる
//
void PlayerFieldSetEscapeBlink(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        actor->blink = kPlayerBlinkEscape;
    }
}
// プレイヤが洞窟に入ったかどうかを判定する
//
bool PlayerFieldIsEnterCave(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL && actor->enterLocation == kPlayerEnterCave ? true : false;
}
int PlayerFieldGetEnterCaveIndex(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL && actor->enterLocation == kPlayerEnterCave ? actor->enterIndex : -1;
}

// プレイヤが白に入ったかどうかを判定する
//
bool PlayerFieldIsEnterCastle(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL && actor->enterLocation == kPlayerEnterCastle ? true : false;
}

