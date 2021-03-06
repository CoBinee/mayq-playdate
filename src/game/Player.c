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

// 内部変数
//
struct Player *player = NULL;


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
        // 位置の設定
        FieldGetStartPosition(&player->position);
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

        // プレイヤの解放
        playdate->system->realloc(player, 0);
        player = NULL;
    }
}

// プレイヤの位置を取得する
//
void PlayerGetPosition(struct Vector *position)
{
    if (player != NULL) {
        *position = player->position;
    }
}

// プレイヤの位置を設定する
//
void PlayerSetPosition(int x, int y)
{
    if (player != NULL) {
        player->position.x = x;
        player->position.y = y;
    }
}
