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
        actor->position.x = 2 * kFieldSizePixel + (kFieldSizePixel / 2);
        actor->position.y = 3 * kFieldSizePixel + (kFieldSizePixel - 1);
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

    // カメラの取得
    struct Vector *camera = GameGetCamera();
    
    // スプライトの描画
    {
        int x = actor->position.x - camera->x;
        int y = actor->position.y - camera->y;
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, x, y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // 
    {
        int x = ((actor->position.x / (kFieldMazeSectionSizeX * kFieldSizePixel)) * 2 + 1);
        int y = ((actor->position.y / (kFieldMazeSectionSizeY * kFieldSizePixel)) * 2 + 1);
        playdate->graphics->fillRect(x * 4 + 248 + 1, y * 4 + 8 + 1, 2, 2, kColorWhite);
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

        // 向きの設定
        actor->direction = kDirectionDown;

        // 移動の設定
        actor->move = actor->position;

        // ジャンプの設定
        actor->jump = 0;
        actor->step = 1;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerAnimationNames_Walk[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // 移動の操作
    if (actor->move.x == actor->position.x && actor->move.y == actor->position.y) {
        int direction = actor->direction;
        if (IocsIsButtonPush(kButtonUp)) {
            bool jump = FieldIsLand(actor->position.x, actor->position.y);
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
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUpLeft, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionLeft, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionLeft;
            } else if (IocsIsButtonPush(kButtonRight)) {
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUpRight, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionRight, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionRight;
            } else {
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionUp;
            }
        } else if (IocsIsButtonPush(kButtonDown)) {
            if (IocsIsButtonPush(kButtonLeft)) {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDownLeft, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionLeft, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionLeft;
            } else if (IocsIsButtonPush(kButtonRight)) {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDownRight, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionRight, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionRight;
            } else {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionDown;
            }
        } else if (IocsIsButtonPush(kButtonLeft)) {
            if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionLeft, &actor->move)) {
                actor->direction = kDirectionLeft;
            }
        } else if (IocsIsButtonPush(kButtonRight)) {
            if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionRight, &actor->move)) {
                actor->direction = kDirectionRight;
            }
        }
        if (actor->position.y == actor->move.y) {
            if (FieldIsFall(actor->move.x, actor->move.y)) {
                if (FieldIsWalk(actor->move.x, actor->move.y, kDirectionDown, &actor->move)) {
                    ;
                }
            }
        }
        if (actor->position.x != actor->move.x) {
            if (actor->move.x < 0) {
                actor->move.x += kFieldSizeX * kFieldSizePixel;
                actor->position.x += kFieldSizeX * kFieldSizePixel;
            } else if (actor->move.x >= kFieldSizeX * kFieldSizePixel) {
                actor->move.x -= kFieldSizeX * kFieldSizePixel;
                actor->position.x -= kFieldSizeX * kFieldSizePixel;
            }
        }
        if (actor->direction != direction) {
            AsepriteStartSpriteAnimation(&actor->animation, "player", playerAnimationNames_Walk[actor->direction], true);
        }
        if (actor->position.y == actor->move.y) {
            actor->jump = 0;
            actor->step = 1;
        }
    }

    // 移動
    {
        bool move = false;
        if (actor->position.x < actor->move.x) {
            actor->position.x += kPlayerMoveSpeed;
            if (actor->position.x > actor->move.x) {
                actor->position.x = actor->move.x;
            }
            move = true;
        } else if (actor->position.x > actor->move.x) {
            actor->position.x -= kPlayerMoveSpeed;
            if (actor->position.x < actor->move.x) {
                actor->position.x = actor->move.x;
            }
            move = true;
        }
        if (actor->position.y < actor->move.y) {
            actor->position.y += kPlayerMoveSpeed;
            if (actor->position.y > actor->move.y) {
                actor->position.y = actor->move.y;
            }
            move = true;
        } else if (actor->position.y > actor->move.y) {
            actor->position.y -= kPlayerMoveSpeed;
            if (actor->position.y < actor->move.y) {
                actor->position.y = actor->move.y;
            }
            move = true;
        }
        if (move) {
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }
    }

    // カメラの設定
    GameSetCamera(actor->position.x + kPlayerCameraX, actor->position.y + kPlayerCameraY);

    // スプライトの更新
    // AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerActorDraw, kGameOrderPlayer);
}

