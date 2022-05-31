// Game.c - ゲーム
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Scene.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Application.h"
#include "Game.h"
#include "Field.h"
#include "Battle.h"
#include "Player.h"
#include "Enemy.h"

// 内部関数
//
static void GameUnload(struct Game *game);
static void GameTransition(struct Game *game, GameFunction function);
static void GameLoadField(struct Game *game);
static void GameStartField(struct Game *game);
static void GamePlayField(struct Game *game);
static void GameUnloadField(struct Game *game);
static void GameLoadBattle(struct Game *game);
static void GameStartBattle(struct Game *game);
static void GamePlayBattle(struct Game *game);
static void GameUnloadBattle(struct Game *game);
static void GameDone(struct Game *game);

// 内部変数
//
static const char *gameSpriteNames[] = {
    "tileset", 
    "player", 
    "weapon", 
    "skeleton",     
};
static const char *gameAudioSamplePaths[] = {
    "", 
};
static const char *gameAudioMusicPath = "";


// ゲームを更新する
//
void GameUpdate(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game == NULL) {

        // ゲームの作成
        game = playdate->system->realloc(NULL, sizeof (struct Game));
        if (game == NULL) {
            playdate->system->error("%s: %d: game instance is not created.", __FILE__, __LINE__);
            return;
        }
        memset(game, 0, sizeof (struct Game));

        // ゲームの初期化
        {
            // ユーザデータの設定
            SceneSetUserdata(game);

            // 解放の設定
            SceneSetUnload((SceneFunction)GameUnload);
        }

        // スプライトの読み込み
        AsepriteLoadSpriteList(gameSpriteNames, sizeof (gameSpriteNames) / sizeof (char *));

        // オーディオの読み込み
        // IocsLoadAudioEffects(gameAudioSamplePaths, kGameAudioSampleSize);

        // フィールドの初期化
        FieldInitialize();

        // バトルの初期化
        BattleInitialize();

        // プレイヤの初期化
        PlayerInitialize();

        // エネミーの初期化
        EnemyInitialize();

        // 処理の設定
        GameTransition(game, (GameFunction)GameLoadField);
    }

    // 処理の更新
    if (game->function != NULL) {
        (*game->function)(game);
    }
}

// ゲームを解放する
//
static void GameUnload(struct Game *game)
{
    // アクタの解放
    ActorUnloadAll();

    // エネミーの解放
    EnemyRelease();

    // プレイヤの解放
    PlayerRelease();

    // バトルの解放
    BattleRelease();

    // フィールドの解放
    FieldRelease();

    // スプライトの解放
    AsepriteUnloadAllSprites();

    // オーディオの解放
    IocsUnloadAllAudioEffects();

}

// 処理を遷移する
//
static void GameTransition(struct Game *game, GameFunction function)
{
    game->function = function;
    game->state = 0;
}

// フィールドを読み込む
//
static void GameLoadField(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // フィールドアクタの読み込み
        FieldActorLoad();

        // プレイヤアクタの読み込み
        PlayerFieldActorLoad();

        // エネミーアクタの読み込み
        EnemyFieldActorLoad();

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // 処理の遷移
    GameTransition(game, (GameFunction)GameStartField);
}

// フィールドを開始する
//
static void GameStartField(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // 処理の遷移
    GameTransition(game, (GameFunction)GamePlayField);
}

// フィールドをプレイする
//
static void GamePlayField(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = true;

        // 初期化の完了
        ++game->state;
    }

    // DEBUG
    if (IocsIsButtonEdge(kButtonA)) {
        game->play = !game->play;
    }
    if (IocsIsButtonEdge(kButtonB)) {
        GameTransition(game, (GameFunction)GameUnloadField);
    }

}

// フィールドを解放する
//
static void GameUnloadField(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // アクタの解放
    ActorUnloadAll();

    // 処理の遷移
    GameTransition(game, (GameFunction)GameLoadBattle);
}

// バトルを読み込む
//
static void GameLoadBattle(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // バトルアクタの読み込み
        {
            struct Vector position;
            PlayerGetFieldPosition(&position);
            int route = FieldGetBattleRoute(position.x, position.y);
            BattleActorLoad(kBattleTypeDungeon, route);
        }

        // プレイヤアクタの読み込み
        PlayerBattleActorLoad();

        // エネミーアクタの読み込み
        EnemyBattleActorLoad(kEnemyTypeSkeleton, 1, kDirectionLeft);

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // 処理の遷移
    GameTransition(game, (GameFunction)GameStartBattle);
}

// バトルを開始する
//
static void GameStartBattle(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // 処理の遷移
    GameTransition(game, (GameFunction)GamePlayBattle);
}

// バトルをプレイする
//
static void GamePlayBattle(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = true;

        // 初期化の完了
        ++game->state;
    }

    // DEBUG
    if (IocsIsButtonEdge(kButtonA)) {
        // game->play = !game->play;
    }
    if (IocsIsButtonEdge(kButtonB)) {
        GameTransition(game, (GameFunction)GameUnloadBattle);
    }

}

// バトルを解放する
//
static void GameUnloadBattle(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // ゲームの停止
        game->play = false;

        // 初期化の完了
        ++game->state;
    }

    // アクタの解放
    ActorUnloadAll();

    // 処理の遷移
    GameTransition(game, (GameFunction)GameLoadField);
}

// ゲームを完了する
//
static void GameDone(struct Game *game)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (game->state == 0) {

        // 初期化の完了
        ++game->state;
    }
    // シーンの遷移
    ApplicationTransition(kApplicationSceneTitle);
}

// プレイ中かどうかを判定する
//
bool GameIsPlay(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL ? game->play : false;
}

// カメラを取得する
//
struct Vector *GameGetCamera(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    return game != NULL ? &game->camera : NULL;
}

// カメラを設定する
//
void GameSetFieldCamera(int x, int y)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        while (x < 0) {
            x += kFieldSizeX * kFieldSizePixel;
        }
        while (x >= kFieldSizeX * kFieldSizePixel) {
            x -= kFieldSizeX * kFieldSizePixel;
        }
        // while (y < 0) {
        //     y += kFieldSizeY * kFieldSizePixel;
        // }
        // while (y >= kFieldSizeY * kFieldSizePixel) {
        //     y -= kFieldSizeY * kFieldSizePixel;
        // }
        game->camera.x = x;
        game->camera.y = y;
    }
}
void GameSetBattleCamera(int x, int y)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        game->camera.x = x;
        game->camera.y = y;
    }
}

// カメラからの位置を取得する
//
void GameGetFieldCameraPosition(int x, int y, struct Vector *position)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        if (position != NULL) {
            while (x < 0) {
                x += kFieldSizeX * kFieldSizePixel;
            }
            while (y < 0) {
                y += kFieldSizeY * kFieldSizePixel;
            }
            position->x = x - game->camera.x;
            if (position->x < -kFieldSizeX * kFieldSizePixel / 2) {
                position->x += kFieldSizeX * kFieldSizePixel;
            } else if (position->x > kFieldSizeX * kFieldSizePixel / 2) {
                position->x -= kFieldSizeX * kFieldSizePixel;
            }
            position->y = y - game->camera.y;
            // if (position->y < -kFieldSizeY * kFieldSizePixel / 2) {
            //     position->y += kFieldSizeY * kFieldSizePixel;
            // } else if (position->y > kFieldSizeY * kFieldSizePixel / 2) {
            //     position->y -= kFieldSizeY * kFieldSizePixel;
            // }
        }
    }
}
void GameGetBattleCameraPosition(int x, int y, struct Vector *position)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        if (position != NULL) {
            position->x = x - game->camera.x;
            position->y = y - game->camera.y;
        }
    }
}

