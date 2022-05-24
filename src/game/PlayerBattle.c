// PlayerBattle.c - バトルプレイヤ
//

// 外部参照
//
#include <string.h>
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
static void PlayerBattleActorPlay(struct PlayerActor *actor);

// 内部変数
//
static const char *playerBattleAnimationNames[] = {
    "WalkUp", 
    "WalkDown", 
    "WalkLeft", 
    "WalkRight", 
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
    struct PlayerActor *actor = (struct PlayerActor *)ActorLoad((ActorFunction)PlayerBattleActorPlay, kGamePriorityPlayer);
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
        actor->position.x = 200;
        actor->position.y = 120;
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

    // スプライトの描画
    {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }
}

// プレイヤアクタがプレイする
//
static void PlayerBattleActorPlay(struct PlayerActor *actor)
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
        AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // 移動
        {
            bool move = false;
            int direction = actor->direction;
            if (IocsIsButtonPush(kButtonUp)) {
                --actor->position.y;
                actor->direction = kDirectionUp;
                move = true;
            } else if (IocsIsButtonPush(kButtonDown)) {
                ++actor->position.y;
                actor->direction = kDirectionDown;
                move = true;
            } else if (IocsIsButtonPush(kButtonLeft)) {
                --actor->position.x;
                actor->direction = kDirectionLeft;
                move = true;
            } else if (IocsIsButtonPush(kButtonRight)) {
                ++actor->position.x;
                actor->direction = kDirectionRight;
                move = true;
            }
            if (actor->direction != direction) {
                AsepriteStartSpriteAnimation(&actor->animation, "player", playerBattleAnimationNames[actor->direction], true);
            }
            if (move) {
                AsepriteUpdateSpriteAnimation(&actor->animation);
            }
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)PlayerBattleActorDraw, kGameOrderPlayer);
}

