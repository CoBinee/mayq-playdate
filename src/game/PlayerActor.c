// PlayerActor.c - プレイヤアクタ
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
static void PlayerActorUnload(struct PlayerActor *actor);
static void PlayerActorDraw(struct PlayerActor *actor);
static void PlayerActorWalk(struct PlayerActor *actor);
static void PlayerActorJump(struct PlayerActor *actor);
static void PlayerActorClimb(struct PlayerActor *actor);
static void PlayerActorAttack(struct PlayerActor *actor);
static void PlayerActorClearCrank(struct PlayerActor *actor);
static void PlayerActorInputCrank(struct PlayerActor *actor);
static void PlayerActorMoveVtoH(struct PlayerActor *actor, int movex, int movey);
static void PlayerActorMoveHtoV(struct PlayerActor *actor, int movex, int movey);
static void PlayerActorBlink(struct PlayerActor *actor);
static bool PlayerActorIsGrabLadder(struct PlayerActor *actor);
static bool PlayerActorIsDownLadder(struct PlayerActor *actor);

// 内部変数
//
static const struct Rect playerActorMoveRect = {
    .left = -7, 
    .top = -23, 
    .right = 6, 
    .bottom = 0, 
};
static const char *playerActorSpriteName = "player";
static const char *playerActorAnimationNames_Idle[kFaceSize] = {
    "IdleLeft", 
    "IdleRight", 
};
static const char *playerActorAnimationNames_Walk[kFaceSize] = {
    "WalkLeft", 
    "WalkRight", 
};
static const char *playerActorAnimationNames_Jump[kFaceSize] = {
    "JumpLeft", 
    "JumpRight", 
};
static const char *playerActorAnimationNames_Fall[kFaceSize] = {
    "FallLeft", 
    "FallRight", 
};
static const char *playerActorAnimationNames_Climb[kFaceSize] = {
    "Climb", 
    "Climb", 
};
static const char *playerActorAnimationNames_Attack[kFaceSize][kPlayerAttackCount] = {
    {
        "AttackLeft1", 
        "AttackLeft2", 
        "AttackLeft3", 
    }, 
    {
        "AttackRight1", 
        "AttackRight2", 
        "AttackRight3", 
    }, 
};


// プレイヤアクタを読み込む
//
void PlayerActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct PlayerActor *actor = (struct PlayerActor *)ActorLoad((ActorFunction)PlayerActorWalk, kGamePriorityPlayer);
    if (actor == NULL) {
        playdate->system->error("%s: %d: player actor is not loaded.", __FILE__, __LINE__);
    }

    // プレイヤの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)PlayerActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagPlayer);

        // 位置の設定
        actor->position = player->position;

        // 向きの設定
        actor->face = kFaceRight;

        // 行動の設定
        actor->action = kPlayerActionIdle;

        // 移動の設定
        actor->moveVector.x = 0;
        actor->moveVector.y = 0;
        actor->moveRect.left = actor->position.x + playerActorMoveRect.left;
        actor->moveRect.top = actor->position.y + playerActorMoveRect.top;
        actor->moveRect.right = actor->position.x + playerActorMoveRect.right;
        actor->moveRect.bottom = actor->position.y + playerActorMoveRect.bottom;

        // 点滅の設定
        actor->blink = 0;
    }
}

// プレイヤアクタを解放する
//
static void PlayerActorUnload(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 位置の保存
    player->position = actor->position;
}

// プレイヤアクタを描画する
//
static void PlayerActorDraw(struct PlayerActor *actor)
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

    // DEBUG
    {
        GameDrawFieldRect(&actor->moveRect, kDrawModeCopy, kColorWhite);
    }

    // クリップの解除
    FieldClearClip();
}

// プレイヤアクタが歩く
//
static void PlayerActorWalk(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 行動の設定
        actor->action = kPlayerActionIdle;

        // クランクの設定
        PlayerActorClearCrank(actor);

        // 移動の設定
        actor->moveVector.y = 0;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Idle[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの初期化
        const char *animation = NULL;

        // クランクの操作
        PlayerActorInputCrank(actor);

        // 基本は左右の移動
        actor->moveVector.y = 0;

        // 攻撃
        if (
            actor->action == kPlayerActionIdle || 
            actor->action == kPlayerActionWalk
        ) {
            if (IocsIsButtonEdge(kButtonA)) {
                actor->action = kPlayerActionAttack;
            }
        }

        // 上下の操作
        if (
            actor->action == kPlayerActionIdle || 
            actor->action == kPlayerActionWalk
        ) {
            if (IocsIsButtonPush(kButtonUp)) {
                if (
                    IocsIsButtonEdge(kButtonUp) && 
                    !PlayerActorIsGrabLadder(actor)
                ) {
                    actor->moveVector.y = -kPlayerMoveJumpStart;
                } else if (PlayerActorIsGrabLadder(actor)) {
                    actor->moveVector.y = -kPlayerMoveClimbStart;
                }
            } else if (IocsIsButtonPush(kButtonDown)) {
                if (PlayerActorIsDownLadder(actor)) {
                    actor->moveVector.y = kPlayerMoveClimbStart;
                }
            }
        }

        // 左右の操作
        if (
            actor->action == kPlayerActionIdle || 
            actor->action == kPlayerActionWalk
        ) {
            if (IocsIsButtonPush(kButtonLeft)) {
                actor->moveVector.x -= kPlayerMoveWalkAccel;
                if (actor->moveVector.x < -kPlayerMoveWalkMaximum) {
                    actor->moveVector.x = -kPlayerMoveWalkMaximum;
                }
                if (actor->face != kFaceLeft || actor->action != kPlayerActionWalk) {
                    actor->face = kFaceLeft;
                    if (actor->action == kPlayerActionIdle) {
                        actor->action = kPlayerActionWalk;
                    }
                    animation = playerActorAnimationNames_Walk[actor->face];
                }
            } else if (IocsIsButtonPush(kButtonRight)) {
                actor->moveVector.x += kPlayerMoveWalkAccel;
                if (actor->moveVector.x > kPlayerMoveWalkMaximum) {
                    actor->moveVector.x = kPlayerMoveWalkMaximum;
                }
                if (actor->face != kFaceRight || actor->action != kPlayerActionWalk) {
                    actor->face = kFaceRight;
                    if (actor->action == kPlayerActionIdle) {
                        actor->action = kPlayerActionWalk;
                    }
                    animation = playerActorAnimationNames_Walk[actor->face];
                }
            } else {
                if (actor->moveVector.x < -kPlayerMoveWalkBrake) {
                    actor->moveVector.x += kPlayerMoveWalkBrake;
                } else if (actor->moveVector.x > kPlayerMoveWalkBrake) {
                    actor->moveVector.x -= kPlayerMoveWalkBrake;
                } else if (actor->moveVector.x != 0) {
                    actor->moveVector.x = 0;
                    if (actor->action == kPlayerActionWalk) {
                        actor->action = kPlayerActionIdle;
                    }
                    animation = playerActorAnimationNames_Idle[actor->face];
                }
            }
        }

        // 移動
        {
            int y = actor->position.y;
            PlayerActorMoveHtoV(actor, actor->moveVector.x >> kPlayerMoveShift, actor->moveVector.y >> kPlayerMoveShift);
            if (actor->moveVector.y < 0) {
                if (PlayerActorIsGrabLadder(actor)) {
                    actor->action = kPlayerActionClimb;
                    animation = playerActorAnimationNames_Climb[actor->face];
                } else if (actor->position.y < y) {
                    actor->action = kPlayerActionJump;
                    animation = playerActorAnimationNames_Jump[actor->face];
                }
            } else if (actor->position.y > y) {
                actor->action = kPlayerActionClimb;
                animation = playerActorAnimationNames_Climb[actor->face];
            }
        }

        // 落下
        if (
            actor->action == kPlayerActionIdle || 
            actor->action == kPlayerActionWalk
        ) {
            if (FieldMoveRect(&actor->moveRect, kDirectionDown, kPlayerMoveFallStart, FieldIsFall, NULL) > 0) {
                actor->action = kPlayerActionFall;
                animation = playerActorAnimationNames_Fall[actor->face];
            }
        }

        // 点滅
        PlayerActorBlink(actor);

        // アニメーションの更新
        if (animation != NULL) {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, animation, true);
        }
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);

    // 処理の更新
    if (
        actor->action == kPlayerActionJump || 
        actor->action == kPlayerActionFall
    ) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorJump);
    } else if (
        actor->action == kPlayerActionClimb 
    ) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorClimb);
    } else if (
        actor->action == kPlayerActionAttack 
    ) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorAttack);
    }
}

// プレイヤアクタがジャンプする
//
static void PlayerActorJump(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // クランクの設定
        PlayerActorClearCrank(actor);

        // ジャンプの設定
        actor->jumpCount = 1;

        // アニメーションの開始
        if (actor->action == kPlayerActionJump) {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Jump[actor->face], true);
        } else {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Fall[actor->face], true);
        }

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの初期化
        const char *animation = NULL;

        // クランクの操作
        PlayerActorInputCrank(actor);

        // 上下の操作
        {
            if (
                actor->jumpCount < kPlayerJumpCount && 
                IocsIsButtonEdge(kButtonUp)
            ) {
                actor->moveVector.y = -kPlayerMoveJumpBoost;
                ++actor->jumpCount;
                AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Jump[actor->face], true);
            }
            if (actor->moveVector.y < 0) {
                actor->moveVector.y += kPlayerMoveGravity;
                if (actor->moveVector.y >= 0) {
                    actor->action = kPlayerActionFall;
                    animation = playerActorAnimationNames_Fall[actor->face];
                }
            } else {
                actor->moveVector.y += kPlayerMoveGravity;
                if (actor->moveVector.y > kPlayerMoveFallMaximum) {
                    actor->moveVector.y = kPlayerMoveFallMaximum;
                }
            }
        }

        // 左右の操作
        {
            if (IocsIsButtonPush(kButtonLeft)) {
                actor->moveVector.x -= kPlayerMoveWalkAccel;
                if (actor->moveVector.x < -kPlayerMoveWalkMaximum) {
                    actor->moveVector.x = -kPlayerMoveWalkMaximum;
                }
            } else if (IocsIsButtonPush(kButtonRight)) {
                actor->moveVector.x += kPlayerMoveWalkAccel;
                if (actor->moveVector.x > kPlayerMoveWalkMaximum) {
                    actor->moveVector.x = kPlayerMoveWalkMaximum;
                }
            } else {
                if (actor->moveVector.x < -kPlayerMoveWalkBrake) {
                    actor->moveVector.x += kPlayerMoveWalkBrake;
                } else if (actor->moveVector.x > kPlayerMoveWalkBrake) {
                    actor->moveVector.x -= kPlayerMoveWalkBrake;
                } else if (actor->moveVector.x != 0) {
                    actor->moveVector.x = 0;
                }
            }
        }

        // 移動
        PlayerActorMoveVtoH(actor, actor->moveVector.x >> kPlayerMoveShift, actor->moveVector.y >> kPlayerMoveShift);

        // 点滅
        PlayerActorBlink(actor);

        // アニメーションの更新
        if (animation != NULL) {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, animation, true);
        }
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);

    // 処理の更新
    if (
        IocsIsButtonEdge(kButtonUp) && 
        PlayerActorIsGrabLadder(actor)
    ) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorClimb);
    } else if (FieldMoveRect(&actor->moveRect, kDirectionDown, 1, FieldIsFall, NULL) == 0) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorWalk);
    }
}

// プレイヤアクタが梯子を上り下りする
//
static void PlayerActorClimb(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 行動の設定
        actor->action = kPlayerActionClimb;

        // クランクの設定
        PlayerActorClearCrank(actor);

        // 移動の設定
        actor->moveVector.x = 0;
        actor->moveVector.y = 0;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Climb[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの初期化
        const char *animation = NULL;

        // クランクの操作
        PlayerActorInputCrank(actor);

        // 上下の操作
        {
            if (IocsIsButtonPush(kButtonUp)) {
                actor->moveVector.y -= kPlayerMoveClimbAccel;
                if (actor->moveVector.y < -kPlayerMoveClimbMaximum) {
                    actor->moveVector.y = -kPlayerMoveClimbMaximum;
                }
            } else if (IocsIsButtonPush(kButtonDown)) {
                actor->moveVector.y += kPlayerMoveClimbAccel;
                if (actor->moveVector.y > kPlayerMoveClimbMaximum) {
                    actor->moveVector.y = kPlayerMoveClimbMaximum;
                }
            } else {
                if (actor->moveVector.y < -kPlayerMoveClimbBrake) {
                    actor->moveVector.y += kPlayerMoveClimbBrake;
                } else if (actor->moveVector.y > kPlayerMoveClimbBrake) {
                    actor->moveVector.y -= kPlayerMoveClimbBrake;
                } else if (actor->moveVector.y != 0) {
                    actor->moveVector.y = 0;
                }
            }
        }

        // 左右の操作
        {
            if (IocsIsButtonPush(kButtonLeft)) {
                actor->moveVector.x -= kPlayerMoveClimbAccel;
                if (actor->moveVector.x < -kPlayerMoveClimbMaximum) {
                    actor->moveVector.x = -kPlayerMoveClimbMaximum;
                }
            } else if (IocsIsButtonPush(kButtonRight)) {
                actor->moveVector.x += kPlayerMoveClimbAccel;
                if (actor->moveVector.x > kPlayerMoveClimbMaximum) {
                    actor->moveVector.x = kPlayerMoveClimbMaximum;
                }
            } else {
                if (actor->moveVector.x < -kPlayerMoveClimbBrake) {
                    actor->moveVector.x += kPlayerMoveClimbBrake;
                } else if (actor->moveVector.x > kPlayerMoveClimbBrake) {
                    actor->moveVector.x -= kPlayerMoveClimbBrake;
                } else if (actor->moveVector.x != 0) {
                    actor->moveVector.x = 0;
                }
            }
        }

        // 移動
        PlayerActorMoveHtoV(actor, actor->moveVector.x >> kPlayerMoveShift, actor->moveVector.y >> kPlayerMoveShift);

        // 点滅
        PlayerActorBlink(actor);

        // アニメーションの更新
        if (animation != NULL) {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, animation, true);
        }
        if (actor->moveVector.x != 0 || actor->moveVector.y != 0) {
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);

    // 処理の更新
    if (!PlayerActorIsGrabLadder(actor)) {
        actor->action = kPlayerActionIdle;
        actor->moveVector.y = 0;
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorWalk);
    }
}

// プレイヤアクタが攻撃する
//
static void PlayerActorAttack(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 行動の設定
        actor->action = kPlayerActionAttack;

        // クランクの設定
        PlayerActorClearCrank(actor);

        // 移動の設定
        actor->moveVector.y = 0;

        // 攻撃の設定
        actor->attackCount = 1;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, playerActorAnimationNames_Attack[actor->face][actor->attackCount - 1], false);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの初期化
        const char *animation = NULL;

        // クランクの操作
        PlayerActorInputCrank(actor);

        // 上下の操作
        {
            ;
        }

        // 左右の操作
        {
            if (actor->moveVector.x < -kPlayerMoveAttackBrake) {
                actor->moveVector.x += kPlayerMoveAttackBrake;
            } else if (actor->moveVector.x > kPlayerMoveAttackBrake) {
                actor->moveVector.x -= kPlayerMoveAttackBrake;
            } else if (actor->moveVector.x != 0) {
                actor->moveVector.x = 0;
            }
        }

        // 移動
        PlayerActorMoveHtoV(actor, actor->moveVector.x >> kPlayerMoveShift, actor->moveVector.y >> kPlayerMoveShift);

        // 点滅
        PlayerActorBlink(actor);

        // アニメーションの更新
        if (animation != NULL) {
            AsepriteStartSpriteAnimation(&actor->animation, playerActorSpriteName, animation, false);
        }
        AsepriteUpdateSpriteAnimation(&actor->animation);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);

    // 処理の更新
    if (AsepriteIsSpriteAnimationDone(&actor->animation)) {
        ActorTransition(&actor->actor, (ActorFunction)PlayerActorWalk);
    }
}

// クランクを操作する
//
static void PlayerActorClearCrank(struct PlayerActor *actor)
{
    actor->crank = 0.0f;
}
static void PlayerActorInputCrank(struct PlayerActor *actor)
{
    float change = IocsGetCrankChange();
    if (change == 0.0f) {
        actor->crank = 0.0f;
    } else {
        actor->crank += change;
    }
}

// 移動する
//
static void PlayerActorMoveVtoH(struct PlayerActor *actor, int movex, int movey)
{
    if (movey < 0) {
        movey = -movey;
        int d_0 = actor->moveRect.top % kFieldSizePixel;
        int d_1 = 0;
        if (d_0 > movey) {
            d_0 = movey;
        } else {
            d_1 = movey - d_0;
        }
        if (d_0 > 0) {
            if (FieldMoveRect(&actor->moveRect, kDirectionUp, d_0, FieldIsSpace, &actor->moveRect) == 0) {
                d_1 = 0;
            }
        }
        if (movex < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionLeft, -movex, FieldIsSpace, &actor->moveRect);
        } else if (movex > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionRight, movex, FieldIsSpace, &actor->moveRect);
        }
        if (d_1 > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionUp, d_1, FieldIsSpace, &actor->moveRect);
        }
    } else if (movey > 0) {
        int d_0 = (kFieldSizePixel - 1) - (actor->moveRect.bottom % kFieldSizePixel);
        int d_1 = 0;
        if (d_0 > movey) {
            d_0 = movey;
        } else {
            d_1 = movey - d_0;
        }
        if (d_0 > 0) {
            if (FieldMoveRect(&actor->moveRect, kDirectionDown, d_0, FieldIsSpace, &actor->moveRect) == 0) {
                d_1 = 0;
            }
        }
        if (movex < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionLeft, -movex, FieldIsSpace, &actor->moveRect);
        } else if (movex > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionRight, movex, FieldIsSpace, &actor->moveRect);
        }
        if (d_1 > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionDown, d_1, FieldIsFall, &actor->moveRect);
        }
    } else {
        if (movex < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionLeft, -movex, FieldIsSpace, &actor->moveRect);
        } else if (movex > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionRight, movex, FieldIsSpace, &actor->moveRect);
        }
    }
    actor->position.x = actor->moveRect.left - playerActorMoveRect.left;
    actor->position.y = actor->moveRect.top - playerActorMoveRect.top;
}
static void PlayerActorMoveHtoV(struct PlayerActor *actor, int movex, int movey)
{
    if (movex < 0) {
        movex = -movex;
        int d_0 = actor->moveRect.left % kFieldSizePixel;
        int d_1 = 0;
        if (d_0 > movex) {
            d_0 = movex;
        } else {
            d_1 = movex - d_0;
        }
        if (d_0 > 0) {
            if (FieldMoveRect(&actor->moveRect, kDirectionLeft, d_0, FieldIsSpace, &actor->moveRect) == 0) {
                d_1 = 0;
            }
        }
        if (movey < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionUp, -movey, FieldIsSpace, &actor->moveRect);
        } else if (movey > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionDown, movey, FieldIsSpace, &actor->moveRect);
        }
        if (d_1 > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionLeft, d_1, FieldIsSpace, &actor->moveRect);
        }
    } else if (movex > 0) {
        int d_0 = (kFieldSizePixel - 1) - (actor->moveRect.right % kFieldSizePixel);
        int d_1 = 0;
        if (d_0 > movex) {
            d_0 = movex;
        } else {
            d_1 = movex - d_0;
        }
        if (d_0 > 0) {
            if (FieldMoveRect(&actor->moveRect, kDirectionRight, d_0, FieldIsSpace, &actor->moveRect) == 0) {
                d_1 = 0;
            }
        }
        if (movey < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionUp, -movey, FieldIsSpace, &actor->moveRect);
        } else if (movey > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionDown, movey, FieldIsSpace, &actor->moveRect);
        }
        if (d_1 > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionRight, d_1, FieldIsSpace, &actor->moveRect);
        }
    } else {
        if (movey < 0) {
            FieldMoveRect(&actor->moveRect, kDirectionUp, -movey, FieldIsSpace, &actor->moveRect);
        } else if (movey > 0) {
            FieldMoveRect(&actor->moveRect, kDirectionDown, movey, FieldIsSpace, &actor->moveRect);
        }
    }
    actor->position.x = actor->moveRect.left - playerActorMoveRect.left;
    actor->position.y = actor->moveRect.top - playerActorMoveRect.top;
}

// 点滅する
//
static void PlayerActorBlink(struct PlayerActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
}

// 梯子を掴んでいるかどうかを判定する
//
static bool PlayerActorIsGrabLadder(struct PlayerActor *actor)
{
    return 
        FieldIsLadder(actor->position.x, actor->position.y + playerActorMoveRect.top) || 
        FieldIsLadder(actor->position.x, actor->position.y + playerActorMoveRect.bottom)
        ? true 
        : false;
}
static bool PlayerActorIsDownLadder(struct PlayerActor *actor)
{
    return 
        FieldIsLadder(actor->position.x, actor->position.y + playerActorMoveRect.top) || 
        FieldIsLadder(actor->position.x, actor->position.y + playerActorMoveRect.bottom) || 
        FieldIsLadder(actor->position.x, actor->position.y + playerActorMoveRect.bottom + 1)
        ? true 
        : false;
}

// 位置を取得する
//
void PlayerActorGetPosition(struct Vector *position)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *position = actor->position;
    }
}

// 矩形を取得する
//
void PlayerActorGetMoveRect(struct Rect *rect)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *rect = actor->moveRect;
    }
}

// プレイヤが点滅しているかどうかを判定する
//
bool PlayerActorIsBlink(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL && actor->blink > 0 ? true : false;
}

// プレイヤを点滅させる
//
void PlayerActorSetDamageBlink(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        actor->blink = kPlayerBlinkDamage;
    }
}

