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
#include "Player.h"
#include "Enemy.h"

// 内部関数
//
static void GameUnload(struct Game *game);
static void GameTransition(GameFunction function);
static void GameLoadField(struct Game *game);
static void GameStartField(struct Game *game);
static void GamePlayField(struct Game *game);
static void GameUnloadField(struct Game *game);
static void GameDone(struct Game *game);
static void GameSetFieldCamera(void);

// 内部変数
//
static const char *gameSpriteNames[] = {
    "tileset", 
    "player", 
    "weapon", 
    "skeleton", 
    "death", 
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

            // フィールドの設定

            // 城の設定

            // 店の設定

            // プレイの設定
            game->play = false;
        }

        // スプライトの読み込み
        AsepriteLoadSpriteList(gameSpriteNames, sizeof (gameSpriteNames) / sizeof (char *));

        // オーディオの読み込み
        // IocsLoadAudioEffects(gameAudioSamplePaths, kGameAudioSampleSize);

        // フィールドの初期化
        FieldInitialize();

        // プレイヤの初期化
        PlayerInitialize();

        // エネミーの初期化
        EnemyInitialize();

        // 処理の設定
        GameTransition((GameFunction)GameLoadField);
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

    // フィールドの解放
    FieldRelease();

    // スプライトの解放
    AsepriteUnloadAllSprites();

    // オーディオの解放
    IocsUnloadAllAudioEffects();

}

// 処理を遷移する
//
static void GameTransition(GameFunction function)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        game->function = function;
        game->state = 0;
    }
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

        // ゲームの停止
        game->play = false;

        // フィールドアクタの読み込み
        FieldActorLoad();

        // プレイヤアクタの読み込み
        PlayerActorLoad();

        // エネミーアクタの読み込み
        EnemyActorLoad();

        // 初期化の完了
        ++game->state;
    }

    // カメラの設定
    GameSetFieldCamera();

    // 処理の遷移
    GameTransition((GameFunction)GameStartField);
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

    // カメラの設定
    GameSetFieldCamera();

    // 処理の遷移
    GameTransition((GameFunction)GamePlayField);
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

        // ゲームの再開
        game->play = true;

        // 遷移の設定
        game->transition = NULL;

        // 初期化の完了
        ++game->state;
    }

    // プレイの監視
    if (game->play) {

        // 洞窟に入る
        if (game->transition == NULL) {
            /*
            int cave = PlayerFieldGetEnterCaveIndex();
            if (cave >= 0) {
                ;
            }
            */
        }

        // 城に入る
        if (game->transition == NULL) {
            /*
            if (PlayerFieldIsEnterCastle()) {
                ;
            }
            */
        }

        // 店に入る
        if (game->transition == NULL) {
            /*
            int shop = PlayerFieldGetEnterShopIndex();
            if (shop >= 0) {
                ;
            }
            */
        }

        // 処理の遷移
        if (game->transition != NULL) {
            GameTransition((GameFunction)GameUnloadField);
        }
    }

    // カメラの設定
    GameSetFieldCamera();

    // DEBUG
    /*
    if (IocsIsButtonEdge(kButtonA)) {
        game->play = !game->play;
    }
    */
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
    GameTransition(game->transition);
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
static void GameSetFieldCamera(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        struct Vector position;
        PlayerActorGetPosition(&position);
        while (position.x < 0) {
            position.x += kFieldSizeX * kFieldSizePixel;
        }
        while (position.x >= kFieldSizeX * kFieldSizePixel) {
            position.x -= kFieldSizeX * kFieldSizePixel;
        }
        // while (position.y < 0) {
        //     position.y += kFieldSizeY * kFieldSizePixel;
        // }
        // while (position.y >= kFieldSizeY * kFieldSizePixel) {
        //     position.y -= kFieldSizeY * kFieldSizePixel;
        // }
        game->camera.x = position.x + kGameCameraFieldX;
        game->camera.y = position.y + kGameCameraFieldY;
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
            position->x = x - game->camera.x + kGameViewFieldLeft;
            if (position->x < -kFieldSizeX * kFieldSizePixel / 2) {
                position->x += kFieldSizeX * kFieldSizePixel;
            } else if (position->x > kFieldSizeX * kFieldSizePixel / 2) {
                position->x -= kFieldSizeX * kFieldSizePixel;
            }
            position->y = y - game->camera.y + kGameViewFieldTop;
            // if (position->y < -kFieldSizeY * kFieldSizePixel / 2) {
            //     position->y += kFieldSizeY * kFieldSizePixel;
            // } else if (position->y > kFieldSizeY * kFieldSizePixel / 2) {
            //     position->y -= kFieldSizeY * kFieldSizePixel;
            // }
        }
    }
}

// フィールド位置ので矩形を描画する
//
void GameDrawFieldRect(struct Rect *rect, LCDBitmapDrawMode drawmode, LCDColor color)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 矩形の描画
    {
        struct Vector view;
        GameGetFieldCameraPosition(rect->left, rect->top, &view);
        playdate->graphics->setDrawMode(drawmode);
        playdate->graphics->drawRect(view.x, view.y, rect->right - rect->left + 1, rect->bottom - rect->top + 1, color);
    }
}
