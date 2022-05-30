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
static void PlayerBattleActorAttack(struct PlayerActor *actor);
static void PlayerBattleCalcRect(struct PlayerActor *actor);
static void PlayerBattleControlCrank(struct PlayerActor *actor);

// 内部変数
//
static const char *playerBattleAnimationNames_Walk[] = {
    "WalkUp", 
    "WalkDown", 
    "WalkLeft", 
    "WalkRight", 
};
static const char *playerBattleAnimationNames_Attack[][kPlayerAttackMaximum] = {
    {
        "AttackUp1", 
        "AttackUp2", 
        "AttackUp3", 
    }, 
    {
        "AttackDown1", 
        "AttackDown2", 
        "AttackDown3", 
    }, 
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
void PlayerBattleActorLoad(void)
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
        BattleGetStartPosition(kDirectionRight, &actor->position);

        // 向きの設定
        actor->direction = kDirectionDown;

        // 矩形の計算
        PlayerBattleCalcRect(actor);
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
    {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, kDrawModeCopy);
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

        // クランクの設定
        actor->crank = 0.0f;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames_Walk[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // クランクの操作
        PlayerBattleControlCrank(actor);

        // 攻撃
        if (
            actor->crank >= 180.0f || 
            IocsIsButtonEdge(kButtonA)
        ) {
            ActorTransition(&actor->actor, (ActorFunction)PlayerBattleActorAttack);

        // 移動
        } else {
            bool move = false;
            int direction = actor->direction;
            if (IocsIsButtonPush(kButtonUp)) {
                int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionUp, kPlayerSpeedBattle);
                int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionUp, kPlayerSpeedBattle);
                if (dl == 0 && dr > 0) {
                    int dt = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionRight, 1);
                    int db = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionRight, 1);
                    actor->position.x += dt < db ? dt : db;
                } else if (dr == 0 && dl > 0) {
                    int dt = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionLeft, 1);
                    int db = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionLeft, 1);
                    actor->position.x -= dt < db ? dt : db;
                } else {
                    actor->position.y -= dl < dr ? dl : dr;
                }
                actor->direction = kDirectionUp;
                move = true;
            } else if (IocsIsButtonPush(kButtonDown)) {
                int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionDown, kPlayerSpeedBattle);
                int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionDown, kPlayerSpeedBattle);
                if (dl == 0 && dr > 0) {
                    int dt = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionRight, 1);
                    int db = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionRight, 1);
                    actor->position.x += dt < db ? dt : db;
                } else if (dr == 0 && dl > 0) {
                    int dt = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionLeft, 1);
                    int db = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionLeft, 1);
                    actor->position.x -= dt < db ? dt : db;
                } else {
                    actor->position.y += dl < dr ? dl : dr;
                }
                actor->direction = kDirectionDown;
                move = true;
            } else if (IocsIsButtonPush(kButtonLeft)) {
                int dt = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionLeft, kPlayerSpeedBattle);
                int db = BattleGetMoveDistance(actor->rect.left, actor->rect.bottom, kDirectionLeft, kPlayerSpeedBattle);
                if (dt == 0 && db > 0) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionDown, 1);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionDown, 1);
                    actor->position.y += dl < dr ? dl : dr;
                } else if (db == 0 && dt > 0) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionUp, 1);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionUp, 1);
                    actor->position.y -= dl < dr ? dl : dr;
                } else {
                    actor->position.x -= dt < db ? dt : db;
                }
                actor->direction = kDirectionLeft;
                move = true;
            } else if (IocsIsButtonPush(kButtonRight)) {
                int dt = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionRight, kPlayerSpeedBattle);
                int db = BattleGetMoveDistance(actor->rect.right, actor->rect.bottom, kDirectionRight, kPlayerSpeedBattle);
                if (dt == 0 && db > 0) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionDown, 1);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionDown, 1);
                    actor->position.y += dl < dr ? dl : dr;
                } else if (db == 0 && dt > 0) {
                    int dl = BattleGetMoveDistance(actor->rect.left, actor->rect.top, kDirectionUp, 1);
                    int dr = BattleGetMoveDistance(actor->rect.right, actor->rect.top, kDirectionUp, 1);
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

        // 矩形の計算
        PlayerBattleCalcRect(actor);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerBattleActorDraw, kGameOrderPlayer);
}

// プレイヤアクタが攻撃する
//
static void PlayerBattleActorAttack(struct PlayerActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // クランクの設定
        actor->crank = 0.0f;

        // 攻撃の設定
        actor->attackCount = 0;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames_Attack[actor->direction][actor->attackCount], false);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // クランクの操作
        PlayerBattleControlCrank(actor);

        // 攻撃の完了
        if (AsepriteIsSpriteAnimationDone(&actor->animation)) {
            ++actor->attackCount;
            if (
                actor->attackCount < kPlayerAttackMaximum && 
                (
                    (
                        ((actor->attackCount & 1) == 1 && actor->crank <= -180.0f) || 
                        ((actor->attackCount & 1) == 0 && actor->crank >= 180.0f)
                    ) || 
                    IocsIsButtonPush(kButtonA)
                )
            ) {
                AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames_Attack[actor->direction][actor->attackCount], false);
            } else {
                ActorTransition(&actor->actor, (ActorFunction)PlayerBattleActorWalk);
            }
        }

        // アニメーションの更新
        AsepriteUpdateSpriteAnimation(&actor->animation);

        // 矩形の計算
        PlayerBattleCalcRect(actor);
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerBattleActorDraw, kGameOrderPlayer);
}

// 矩形を計算する
//
static void PlayerBattleCalcRect(struct PlayerActor *actor)
{
    actor->rect.left = actor->position.x + kPlayerRectLeft;
    actor->rect.top = actor->position.y + kPlayerRectTop;
    actor->rect.right = actor->position.x + kPlayerRectRight;
    actor->rect.bottom = actor->position.y + kPlayerRectBottom;
}

// クランクを操作する
//
static void PlayerBattleControlCrank(struct PlayerActor *actor)
{
    float change = IocsGetCrankChange();
    if (change == 0.0f || (actor->crank < 0.0f && change > 0.0f) || (actor->crank > 0.0f && change < 0.0f)) {
        actor->crank = change;
    } else {
        actor->crank += change;
    }
}