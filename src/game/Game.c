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
#include "Dungeon.h"
#include "Battle.h"
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
static void GameLoadDungeon(struct Game *game);
static void GameStartDungeon(struct Game *game);
static void GamePlayDungeon(struct Game *game);
static void GameUnloadDungeon(struct Game *game);
static void GameLoadBattle(struct Game *game);
static void GameStartBattle(struct Game *game);
static void GamePlayBattle(struct Game *game);
static void GameUnloadBattle(struct Game *game);
static void GameDone(struct Game *game);
static void GameHit(void);
static void GameSetFieldCamera(void);
static void GameSetBattleCamera(void);

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

            // ダンジョンの設定

            // バトルの設定
            game->battleEncount = -1;

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

        // ダンジョンの初期化
        DungeonInitialize();

        // バトルの初期化
        BattleInitialize();

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

    // バトルの解放
    BattleRelease();

    // ダンジョンの解放
    DungeonRelease();

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
        PlayerFieldActorLoad();

        // エネミーアクタの読み込み
        EnemyFieldActorLoad();

        // バトルの設定
        {
            // 直前のバトルで逃げた場合、プレイヤとエネミーを一定時間エンカウントさせない
            if (game->battleEncount >= 0) {
                PlayerFieldSetEscapeBlink();
                EnemyFieldSetEscapeBlink(game->battleEncount);
            }
            game->battleEncount = -1;
        }

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
    {
        // エネミーとの接触
        if (game->transition == NULL) {
            if (!PlayerFieldIsBlink()) {
                struct Rect rect;
                PlayerFieldGetMoveRect(&rect);
                game->battleEncount = EnemyFieldGetHitIndex(&rect);
                if (game->battleEncount >= 0) {
                    game->transition = (GameFunction)GameLoadBattle;
                }
            }
        }

        // 洞窟に入る
        if (game->transition == NULL) {
            int cave = PlayerFieldGetEnterCaveIndex();
            if (cave >= 0) {
                game->dungeonIndex = cave;
                DungeonGetEntrancePosition(game->dungeonIndex, &game->dungeonPosition);
                game->dungeonDirection = kDirectionDown;
                BattleGetStartPosition(game->dungeonDirection, &game->dungeonLast);
                game->transition = (GameFunction)GameLoadDungeon;
            }
        }

        // 城に入る
        if (game->transition == NULL) {
            if (PlayerFieldIsEnterCastle()) {
                game->transition = (GameFunction)GameLoadBattle;
            }
        }

        // 店に入る
        if (game->transition == NULL) {
            int shop = PlayerFieldGetEnterShopIndex();
            if (shop >= 0) {
                game->shopIndex = shop;
                game->transition = (GameFunction)GameLoadBattle;
            }
        }

        // 処理の遷移
        if (game->transition != NULL) {
            GameTransition((GameFunction)GameUnloadField);
        }
    }

    // カメラの設定
    GameSetFieldCamera();

    // DEBUG
    if (IocsIsButtonEdge(kButtonA)) {
        game->play = !game->play;
    }
    if (IocsIsButtonEdge(kButtonB)) {
        game->transition = (GameFunction)GameLoadBattle;
        GameTransition((GameFunction)GameUnloadField);
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
    GameTransition(game->transition);
}

// ダンジョンを読み込む
//
static void GameLoadDungeon(struct Game *game)
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

        // ダンジョンの設定
        {
            game->dungeonType = kEnemyTypeSkeleton;
            game->dungeonRest = 4;
        }

        // ダンジョンアクタの読み込み
        DungeonActorLoad();

        // バトルアクタの読み込み
        {
            int entrance = DungeonGetEntranceIndex(game->dungeonPosition.x, game->dungeonPosition.y);
            unsigned char route = DungeonGetRoute(game->dungeonPosition.x, game->dungeonPosition.y);
            BattleActorLoad(entrance >= 0 ? kBattleTypeEntrance : kBattleTypeDungeon, route);
        }

        // プレイヤアクタの読み込み
        {
            struct Vector position;
            BattleGetStartPosition(game->dungeonDirection, &position);
            if (game->dungeonDirection == kDirectionUp || game->dungeonDirection == kDirectionDown) {
                position.x = game->dungeonLast.x;
            } else {
                position.y = game->dungeonLast.y;
            }
            PlayerBattleActorLoad(position.x, position.y, game->dungeonDirection);
        }

        // エネミーアクタの読み込み
        EnemyBattleActorLoad(game->dungeonType, game->dungeonRest, game->dungeonDirection);

        // DEBUG
        playdate->system->logToConsole("dungeon = %d, %d", game->dungeonPosition.x, game->dungeonPosition.y);

        // 初期化の完了
        ++game->state;
    }

    // カメラの設定
    GameSetBattleCamera();

    // 処理の遷移
    GameTransition((GameFunction)GameStartDungeon);
}

// ダンジョンを開始する
//
static void GameStartDungeon(struct Game *game)
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
    GameSetBattleCamera();

    // 処理の遷移
    GameTransition((GameFunction)GamePlayDungeon);
}

// ダンジョンをプレイする
//
static void GamePlayDungeon(struct Game *game)
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

        // 初期化の完了
        ++game->state;
    }

    // ヒット判定
    GameHit();

    // ダンジョンの監視
    {
        // エネミーの数の取得
        game->dungeonRest = EnemyBattleGetRest(game->dungeonType);

        // 逃げる方向の取得
        game->dungeonDirection = PlayerBattleGetEscapeDirection();

        // 逃げた
        if (game->dungeonDirection >= 0) {
            GameTransition((GameFunction)GameUnloadDungeon);
        }
    }

    // カメラの設定
    GameSetBattleCamera();
}

// ダンジョンを解放する
//
static void GameUnloadDungeon(struct Game *game)
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

        // 遷移の設定
        game->transition = NULL;

        // ダンジョンから出た
        if (game->transition == NULL) {
            int entrance = DungeonGetEntranceIndex(game->dungeonPosition.x, game->dungeonPosition.y);
            if (entrance >= 0 && game->dungeonDirection == kDirectionDown) {
                struct Vector position;
                FieldGetCavePosition(entrance, &position);
                PlayerSetFieldPosition(position.x, position.y);
                game->transition = (GameFunction)GameLoadField;
            }
        }

        // ダンジョン内の移動
        if (game->transition == NULL) {
            struct Vector position;
            DungeonGetDirectionalPosition(game->dungeonPosition.x, game->dungeonPosition.y, game->dungeonDirection, &position);
            game->dungeonPosition = position;
            game->dungeonDirection = game->dungeonDirection ^ 0x01;
            PlayerFieldGetPosition(&game->dungeonLast);
            game->transition = (GameFunction)GameLoadDungeon;
        }

        // 初期化の完了
        ++game->state;
    }

    // アクタの解放
    ActorUnloadAll();

    // 処理の遷移
    GameTransition(game->transition);
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

        // ゲームの停止
        game->play = false;

        // バトルの設定
        {
            struct Vector pp;
            PlayerGetFieldPosition(&pp);
            struct Vector ep;
            if (game->battleEncount >= 0) {
                EnemyGetFieldPosition(game->battleEncount, &ep);
            } else {
                ep = pp;
            }
            if (game->battleEncount >= 0) {
                game->battlePosition = ep;
                game->battleType = EnemyGetFieldType(game->battleEncount);
                game->battleRest = EnemyGetFieldRest(game->battleEncount);
            } else {
                game->battlePosition = pp;
                game->battleType = kEnemyTypeSkeleton;
                game->battleRest = 9;
            }
            game->battleRoute = FieldGetBattleRoute(game->battlePosition.x, game->battlePosition.y);
            game->battleDirection = -1;
            {
                int dx = pp.x - ep.x;
                int dy = pp.y - ep.y;
                if (abs(dy) > abs(dx)) {
                    if (dy < 0 && (game->battleRoute & (1 << kDirectionUp)) != 0) {
                        game->battleDirection = kDirectionUp;
                    } else if (dy > 0 && (game->battleRoute & (1 << kDirectionDown)) != 0) {
                        game->battleDirection = kDirectionDown;
                    }
                } else {
                    if (dx < 0 && (game->battleRoute & (1 << kDirectionLeft)) != 0) {
                        game->battleDirection = kDirectionLeft;
                    } else if (dx > 0 && (game->battleRoute & (1 << kDirectionRight)) != 0) {
                        game->battleDirection = kDirectionRight;
                    }
                }
                if (game->battleDirection < 0) {
                    if ((game->battleRoute & (1 << kDirectionUp)) != 0) {
                        game->battleDirection = kDirectionUp;
                    } else if ((game->battleRoute & (1 << kDirectionDown)) != 0) {
                        game->battleDirection = kDirectionDown;
                    } else if ((game->battleRoute & (1 << kDirectionLeft)) != 0) {
                        game->battleDirection = kDirectionLeft;
                    } else if ((game->battleRoute & (1 << kDirectionRight)) != 0) {
                        game->battleDirection = kDirectionRight;
                    } else {
                        playdate->system->error("%s: %d: encount error.", __FILE__, __LINE__);
                    }
                }
            }
        }

        // バトルアクタの読み込み
        BattleActorLoad(kBattleTypeField, game->battleRoute);

        // プレイヤアクタの読み込み
        {
            struct Vector position;
            BattleGetStartPosition(game->battleDirection, &position);
            PlayerBattleActorLoad(position.x, position.y, game->battleDirection);
        }

        // エネミーアクタの読み込み
        EnemyBattleActorLoad(game->battleType, game->battleRest, game->battleDirection);

        // 初期化の完了
        ++game->state;
    }

    // カメラの設定
    GameSetBattleCamera();

    // 処理の遷移
    GameTransition((GameFunction)GameStartBattle);
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

    // カメラの設定
    GameSetBattleCamera();

    // 処理の遷移
    GameTransition((GameFunction)GamePlayBattle);
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

        // ゲームの再開
        game->play = true;

        // 初期化の完了
        ++game->state;
    }

    // ヒット判定
    GameHit();

    // バトルの監視
    {
        // エネミーの数の取得
        game->battleRest = EnemyBattleGetRest(game->battleType);

        // 逃げる方向の取得
        game->battleDirection = PlayerBattleGetEscapeDirection();

        // エネミーを倒した
        if (game->battleRest == 0) {
            GameTransition((GameFunction)GameUnloadBattle);

        // 逃げた
        } else if (game->battleDirection >= 0) {
            GameTransition((GameFunction)GameUnloadBattle);
        }
    }

    // カメラの設定
    GameSetBattleCamera();

    // DEBUG
    if (IocsIsButtonEdge(kButtonB)) {
        if ((game->battleRoute & (1 << kDirectionUp)) != 0) {
            game->battleDirection = kDirectionUp;
        } else if ((game->battleRoute & (1 << kDirectionDown)) != 0) {
            game->battleDirection = kDirectionDown;
        } else if ((game->battleRoute & (1 << kDirectionLeft)) != 0) {
            game->battleDirection = kDirectionLeft;
        } else {
            game->battleDirection = kDirectionRight;
        }
        GameTransition((GameFunction)GameUnloadBattle);
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

        // エネミーの数の反映
        EnemySetFieldRest(game->battleEncount, game->battleRest);

        // エネミーを全て倒した
        if (game->battleRest == 0) {
            game->battleEncount = -1;

        // バトルから逃げた
        } else {
            struct Vector position;
            FieldGetDirectinalPosition(game->battlePosition.x, game->battlePosition.y, game->battleDirection, &position);
            PlayerSetFieldPosition(position.x, position.y);
        }

        // 初期化の完了
        ++game->state;
    }

    // アクタの解放
    ActorUnloadAll();

    // 処理の遷移
    GameTransition((GameFunction)GameLoadField);
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

// ヒット判定を行う
//
static void GameHit(void)
{
    // ゲームの取得
    struct Game *game = (struct Game *)SceneGetUserdata();

    // プレイヤの攻撃の判定
    {
        struct Rect rect;
        PlayerBattleGetAttackRect(&rect);
        if (rect.left < rect.right) {
            struct Vector position;
            PlayerBattleGetPosition(&position);
            EnemyBattleIsHitThenDamage(&rect, position.x, position.y, 1);
        }
    }
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
        PlayerFieldGetPosition(&position);
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
static void GameSetBattleCamera(void)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        game->camera.x = kGameCameraBattleX;
        game->camera.y = kGameCameraBattleY;
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
void GameGetBattleCameraPosition(int x, int y, struct Vector *position)
{
    struct Game *game = (struct Game *)SceneGetUserdata();
    if (game != NULL) {
        if (position != NULL) {
            position->x = x - game->camera.x + kGameViewBattleLeft;
            position->y = y - game->camera.y + kGameViewBattleTop;
        }
    }
}

