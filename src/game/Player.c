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
        // 攻撃範囲の読み込み
        AsepriteLoadSpriteJson(&player->battleSprite, "images/attack.json");

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

        // 攻撃範囲の解放
        AsepriteUnloadSpriteJson(&player->battleSprite);

        // プレイヤの解放
        playdate->system->realloc(player, 0);
        player = NULL;
    }
}

// プレイヤのフィールド上の位置を取得する
//
void PlayerGetFieldPosition(struct Vector *position)
{
    if (player != NULL) {
        *position = player->fieldPosition;
    }
}
