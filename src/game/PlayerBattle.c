// PlayerBattle.c - バトルプレイヤ
//

// 外部参照
//
#include <string.h>
#include <math.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Battle.h"
#include "Player.h"

// 内部関数
//
static void PlayerBattleActorUnload(struct PlayerActor *actor);
static void PlayerBattleActorDraw(struct PlayerActor *actor);
static void PlayerBattleActorWalk(struct PlayerActor *actor);
static void PlayerBattleBlink(struct PlayerActor *actor);
static void PlayerBattleCalc(struct PlayerActor *actor);

// 内部変数
//
static const struct Rect playerBattleMoveRect = {
    .left = -12, 
    .top = -23, 
    .right = 11, 
    .bottom = 0, 
};
static const char *playerBattleAnimationNames_Walk[] = {
    "WalkUp", 
    "WalkDown", 
    "WalkLeft", 
    "WalkRight", 
};


// プレイヤアクタを読み込む
//
void PlayerBattleActorLoad(int x, int y, int direction)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct PlayerActor *actor = (struct PlayerActor *)ActorLoad((ActorFunction)PlayerBattleActorWalk, kGamePriorityPlayer);
    if (actor == NULL) {
        playdate->system->error("%s: %d: player actor is not loaded.", __FILE__, __LINE__);
    }

    // プレイヤの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)PlayerBattleActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagPlayer);

        // 位置の設定
        actor->position.x = x;
        actor->position.y = y;
        actor->origin = actor->position;

        // 向きの設定
        actor->direction = direction ^ 0x01;

        // 点滅の設定
        actor->blink = 0;

        // 計算
        PlayerBattleCalc(actor);
    }
}

// プレイヤアクタを解放する
//
static void PlayerBattleActorUnload(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// プレイヤアクタを描画する
//
static void PlayerBattleActorDraw(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クリップの設定
    BattleSetClip();

    // スプライトの描画
    if ((actor->blink & kPlayerBlinkInterval) == 0) {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // DEBUG
    if (actor->attackDirection >= 0) {
        struct Vector view;
        GameGetBattleCameraPosition(actor->attackRect.left, actor->attackRect.top, &view);
        playdate->graphics->setDrawMode(kDrawModeCopy);
        playdate->graphics->drawRect(view.x, view.y, actor->attackRect.right - actor->attackRect.left + 1, actor->attackRect.bottom - actor->attackRect.top + 1, kColorWhite);
    }

    // クリップの解除
    BattleClearClip();
}

// プレイヤアクタが歩く
//
static void PlayerBattleActorWalk(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames_Walk[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 位置の保存
        actor->origin = actor->position;

        // 移動
        {
            bool move = false;
            int direction = actor->direction;
            if (IocsIsButtonPush(kButtonUp)) {
                int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionUp, kPlayerSpeedBattle, true);
                int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionUp, kPlayerSpeedBattle, true);
                if (dl == 0 && dr > 0) {
                    int dt = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionRight, 1, true);
                    int db = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionRight, 1, true);
                    actor->position.x += dt < db ? dt : db;
                } else if (dr == 0 && dl > 0) {
                    int dt = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionLeft, 1, true);
                    int db = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionLeft, 1, true);
                    actor->position.x -= dt < db ? dt : db;
                } else {
                    actor->position.y -= dl < dr ? dl : dr;
                }
                actor->direction = kDirectionUp;
                move = true;
            } else if (IocsIsButtonPush(kButtonDown)) {
                int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionDown, kPlayerSpeedBattle, true);
                int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionDown, kPlayerSpeedBattle, true);
                if (dl == 0 && dr > 0) {
                    int dt = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionRight, 1, true);
                    int db = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionRight, 1, true);
                    actor->position.x += dt < db ? dt : db;
                } else if (dr == 0 && dl > 0) {
                    int dt = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionLeft, 1, true);
                    int db = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionLeft, 1, true);
                    actor->position.x -= dt < db ? dt : db;
                } else {
                    actor->position.y += dl < dr ? dl : dr;
                }
                actor->direction = kDirectionDown;
                move = true;
            } else if (IocsIsButtonPush(kButtonLeft)) {
                int dt = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionLeft, kPlayerSpeedBattle, true);
                int db = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionLeft, kPlayerSpeedBattle, true);
                if (dt == 0 && db > 0) {
                    int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionDown, 1, true);
                    int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionDown, 1, true);
                    actor->position.y += dl < dr ? dl : dr;
                } else if (db == 0 && dt > 0) {
                    int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionUp, 1, true);
                    int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionUp, 1, true);
                    actor->position.y -= dl < dr ? dl : dr;
                } else {
                    actor->position.x -= dt < db ? dt : db;
                }
                actor->direction = kDirectionLeft;
                move = true;
            } else if (IocsIsButtonPush(kButtonRight)) {
                int dt = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionRight, kPlayerSpeedBattle, true);
                int db = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionRight, kPlayerSpeedBattle, true);
                if (dt == 0 && db > 0) {
                    int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionDown, 1, true);
                    int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionDown, 1, true);
                    actor->position.y += dl < dr ? dl : dr;
                } else if (db == 0 && dt > 0) {
                    int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionUp, 1, true);
                    int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionUp, 1, true);
                    actor->position.y -= dl < dr ? dl : dr;
                } else {
                    actor->position.x += dt < db ? dt : db;
                }
                actor->direction = kDirectionRight;
                move = true;
            }
            if (actor->direction != direction) {
                AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames_Walk[actor->direction], true);
            }
            if (move) {
                AsepriteUpdateSpriteAnimation(&actor->animation);
            }
        }

        // 点滅
        PlayerBattleBlink(actor);

        // 計算
        PlayerBattleCalc(actor);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerBattleActorDraw, kGameOrderCharacter + actor->position.y);
}

// 点滅する
//
static void PlayerBattleBlink(struct PlayerActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
}

// プレイヤを計算する
//
static void PlayerBattleCalc(struct PlayerActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + playerBattleMoveRect.left;
        actor->moveRect.top = actor->position.y + playerBattleMoveRect.top;
        actor->moveRect.right = actor->position.x + playerBattleMoveRect.right;
        actor->moveRect.bottom = actor->position.y + playerBattleMoveRect.bottom;
    }

    // 攻撃の計算
    {
        struct Vector v = {
            actor->position.x - actor->origin.x, 
            actor->position.y - actor->origin.y, 
        };
        if (actor->direction == kDirectionUp && v.y < 0) {
            actor->attackRect.left = actor->moveRect.left;
            actor->attackRect.top = actor->moveRect.top;
            actor->attackRect.right = actor->moveRect.right;
            actor->attackRect.bottom = actor->moveRect.top - v.y;
            actor->attackDirection = kDirectionUp;
        } else if (actor->direction == kDirectionDown && v.y > 0) {
            actor->attackRect.left = actor->moveRect.left;
            actor->attackRect.top = actor->moveRect.bottom - v.y;
            actor->attackRect.right = actor->moveRect.right;
            actor->attackRect.bottom = actor->moveRect.bottom;
            actor->attackDirection = kDirectionDown;
        } else if (actor->direction == kDirectionLeft && v.x < 0) {
            actor->attackRect.left = actor->moveRect.left;
            actor->attackRect.top = actor->moveRect.top;
            actor->attackRect.right = actor->moveRect.left - v.x;
            actor->attackRect.bottom = actor->moveRect.bottom;
            actor->attackDirection = kDirectionLeft;
        } else if (actor->direction == kDirectionRight && v.x > 0) {
            actor->attackRect.left = actor->moveRect.right - v.x;
            actor->attackRect.top = actor->moveRect.top;
            actor->attackRect.right = actor->moveRect.right;
            actor->attackRect.bottom = actor->moveRect.bottom;
            actor->attackDirection = kDirectionRight;
        } else {
            actor->attackRect.left = 0;
            actor->attackRect.top = 0;
            actor->attackRect.right = 0;
            actor->attackRect.bottom = 0;
            actor->attackDirection = -1;
        }
    }
}

// 位置を取得する
//
void PlayerBattleGetPosition(struct Vector *position)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *position = actor->position;
    }
}

// 向きを取得する
//
int PlayerBattleGetDirection(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL ? actor->direction : kDirectionDown;
}

// バトルから抜けた方向を取得する
//
int PlayerBattleGetEscapeDirection(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    int direction = -1;
    if (actor != NULL) {
        if (!BattleIsInsideY(actor->moveRect.top)) {
            direction = kDirectionUp;
        } else if (!BattleIsInsideY(actor->moveRect.bottom)) {
            direction = kDirectionDown;
        } else if (!BattleIsInsideX(actor->moveRect.left)) {
            direction = kDirectionLeft;
        } else if (!BattleIsInsideX(actor->moveRect.right)) {
            direction = kDirectionRight;
        }
    }
    return direction;
}

// 移動を取得する
//
void PlayerBattleGetMoveRect(struct Rect *rect)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *rect = actor->moveRect;
    }
}

// 攻撃を取得する
//
void PlayerBattleGetAttackRect(struct Rect *rect)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    if (actor != NULL) {
        *rect = actor->attackRect;
    }
}
int PlayerBattleGetAttackDirection(void)
{
    struct PlayerActor *actor = (struct PlayerActor *)ActorFindWithTag(kGameTagPlayer);
    return actor != NULL ? actor->attackDirection : -1;
}

