// Player.c - プレイヤ
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
static void PlayerActorPlayOnField(struct PlayerActor *actor);

// 内部変数
//
static struct Player *player = NULL;
static const char *playerAnimationNames_Walk[] = {
    "WalkUp", 
    "WalkDown", 
    "WalkLeft", 
    "WalkRight", 
};


// プレイヤを初期化する
//
void PlayerInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct PlayerActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: player actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct PlayerActor));
    }

    // プレイヤの作成
    player = (struct Player *)playdate->system->realloc(NULL, sizeof (struct Player));
    if (player == NULL) {
        playdate->system->error("%s: %d: player instance is not created.", __FILE__, __LINE__);
    }

    // プレイヤの初期化
    {
        // フィールドの設定
        FieldGetStartPosition(&player->fieldPosition);

    }

}

// プレイヤを解放する
//
void PlayerRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // プレイヤの解放
    if (player != NULL) {
        playdate->system->realloc(player, 0);
        player = NULL;
    }
}

// プレイヤアクタを読み込む
//
void PlayerActorLoadOnField(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct PlayerActor *actor = (struct PlayerActor *)ActorLoad((ActorFunction)PlayerActorPlayOnField, kGamePriorityPlayer);
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
        actor->position = player->fieldPosition;
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
    player->fieldPosition = actor->destination;
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

    // スプライトの描画
    {
        struct Vector view;
        GameGetCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }
}

// プレイヤアクタがフィールドをプレイする
//
static void PlayerActorPlayOnField(struct PlayerActor *actor)
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

        // ジャンプの設定
        actor->jump = 0;
        actor->step = 1;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerAnimationNames_Walk[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動の操作
        if (actor->destination.x == actor->position.x && actor->destination.y == actor->position.y) {
            bool land = FieldIsLand(actor->position.x, actor->position.y);
            int direction = actor->direction;
            if (IocsIsButtonPush(kButtonUp)) {
                bool jump = land;
                if (jump) {
                    actor->jump = actor->step;
                    if (actor->step < 2) {
                        ++actor->step;
                    }
                } else {
                    --actor->jump;
                    if (actor->jump > 0) {
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
                    if (FieldWalk(actor->position.x, actor->position.y, kDirectionUp, jump, true, &actor->destination)) {
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
                    if (FieldWalk(actor->destination.x, actor->destination.y, kDirectionDown, land, true, &actor->destination)) {
                        ;
                    }
                }
            }
            if (actor->position.x != actor->destination.x) {
                FieldAdjustMovePosition(&actor->position, &actor->destination);
            }
            if (actor->direction != direction) {
                AsepriteStartSpriteAnimation(&actor->animation, "player", playerAnimationNames_Walk[actor->direction], true);
            }
            if (actor->position.y == actor->destination.y) {
                actor->jump = 0;
                actor->step = 1;
            }
        }

        // 移動
        {
            bool move = false;
            if (actor->position.x < actor->destination.x) {
                actor->position.x += kPlayerMoveSpeed;
                if (actor->position.x > actor->destination.x) {
                    actor->position.x = actor->destination.x;
                }
                move = true;
            } else if (actor->position.x > actor->destination.x) {
                actor->position.x -= kPlayerMoveSpeed;
                if (actor->position.x < actor->destination.x) {
                    actor->position.x = actor->destination.x;
                }
                move = true;
            }
            if (actor->position.y < actor->destination.y) {
                actor->position.y += kPlayerMoveSpeed;
                if (actor->position.y > actor->destination.y) {
                    actor->position.y = actor->destination.y;
                }
                move = true;
            } else if (actor->position.y > actor->destination.y) {
                actor->position.y -= kPlayerMoveSpeed;
                if (actor->position.y < actor->destination.y) {
                    actor->position.y = actor->destination.y;
                }
                move = true;
            }
            if (move) {
                AsepriteUpdateSpriteAnimation(&actor->animation);
            }
        }
    }

    // カメラの設定
    GameSetCamera(actor->position.x + kPlayerCameraX, actor->position.y + kPlayerCameraY);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);
}

