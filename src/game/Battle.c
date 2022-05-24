// Battle.c - バトル
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Aseprite.h"
#include "Game.h"
#include "Maze.h"
#include "Battle.h"

// 内部関数
//
static void BattleActorUnload(struct BattleActor *actor);
static void BattleActorDraw(struct BattleActor *actor);
static void BattleActorLoop(struct BattleActor *actor);

// 内部変数
//
static struct Battle *battle = NULL;
static const char *battleAnimationNames[kBattleAnimationSize] = {
    "Null", 
    "Floor", 
    "Block", 
};


// バトルを初期化する
//
void BattleInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct BattleActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: battle actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct BattleActor));
    }

    // バトルの作成
    battle = (struct Battle *)playdate->system->realloc(NULL, sizeof (struct Battle));
    if (battle == NULL) {
        playdate->system->error("%s: %d: battle instance is not created.", __FILE__, __LINE__);
    }

    // バトルの初期化
    {
        ;
    }
}

// バトルを解放する
//
void BattleRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // バトルの解放
    if (battle != NULL) {        
        playdate->system->realloc(battle, 0);
        battle = NULL;
    }
}

// バトルアクタを読み込む
//
void BattleActorLoad(int block)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct BattleActor *actor = (struct BattleActor *)ActorLoad((ActorFunction)BattleActorLoop, kGamePriorityBattle);
    if (actor == NULL) {
        playdate->system->error("%s: %d: battle actor is not loaded.", __FILE__, __LINE__);
    }

    // バトルの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)BattleActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagBattle);

        // マップの作成
        {
            for (int y = 0; y < kBattleSizeY; y++) {
                for (int x = 0; x < kBattleSizeX; x++) {
                    battle->maps[y][x] = kBattleMapFloor;
                }
            }
            if ((block & (1 << kDirectionUp)) != 0) {
                for (int x = 0; x < kBattleSizeX; x++) {
                    battle->maps[0][x] = kBattleMapBlock;
                }
            }
            if ((block & (1 << kDirectionDown)) != 0) {
                for (int x = 0; x < kBattleSizeX; x++) {
                    battle->maps[kBattleSizeY - 1][x] = kBattleMapBlock;
                }
            }
            if ((block & (1 << kDirectionLeft)) != 0) {
                for (int y = 0; y < kBattleSizeY; y++) {
                    battle->maps[y][0] = kBattleMapBlock;
                }
            }
            if ((block & (1 << kDirectionRight)) != 0) {
                for (int y = 0; y < kBattleSizeY; y++) {
                    battle->maps[y][kBattleSizeX - 1] = kBattleMapBlock;
                }
            }
        }
    }
}

// バトルアクタを解放する
//
static void BattleActorUnload(struct BattleActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// バトルアクタを描画する
//
static void BattleActorDraw(struct BattleActor *actor)
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
        int viewx = camera->x >= 0 ? -(camera->x % kBattleSizePixel) : -kBattleSizePixel - (camera->x % kBattleSizePixel);
        int viewy = camera->y >= 0 ? -(camera->y % kBattleSizePixel) : -kBattleSizePixel - (camera->y % kBattleSizePixel);
        int mapx = camera->x >= 0 ? camera->x / kBattleSizePixel : camera->x / kBattleSizePixel - 1;
        int mapy = camera->y >= 0 ? camera->y / kBattleSizePixel : camera->y / kBattleSizePixel - 1;
        int my = mapy;
        playdate->graphics->setClipRect(kBattleViewLeft, kBattleViewTop, kBattleViewSizeX, kBattleViewSizeY);
        for (int vy = viewy; vy < kBattleViewSizeY; vy += kBattleSizePixel) {
            int mx = mapx;
            for (int vx = viewx; vx < kBattleViewSizeX; vx += kBattleSizePixel) {
                int animation = kBattleAnimationNull;
                if (mx >= 0 && mx < kBattleSizeX && my >= 0 && my < kBattleSizeY) {
                    animation = battle->maps[my][mx];
                }
                AsepriteDrawSpriteAnimation(&actor->animations[animation], vx + kBattleViewLeft, vy + kBattleViewTop, kDrawModeCopy, kBitmapUnflipped);
                ++mx;
            }
            ++my;
        }
        playdate->graphics->clearClipRect();
    }
}

// バトルアクタが待機する
//
static void BattleActorLoop(struct BattleActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // カメラの設定
        GameSetBattleCamera(((kBattleSizeX * kBattleSizePixel) - kBattleViewSizeX) / 2, ((kBattleSizeY * kBattleSizePixel) - kBattleViewSizeY) / 2);

        // アニメーションの開始
        for (int i = 0; i < kBattleAnimationSize; i++) {
            AsepriteStartSpriteAnimation(&actor->animations[i], "tileset", battleAnimationNames[i], false);
        }

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // スプライトの更新
        for (int i = 0; i < kBattleAnimationSize; i++) {
            AsepriteUpdateSpriteAnimation(&actor->animations[i]);
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)BattleActorDraw, kGameOrderBattle);
}

// マップを取得する
//
unsigned char BattleGetMap(int x, int y)
{
    unsigned char result = kBattleMapBlock;
    if (battle != NULL) {
        if (y >= 0 && y < kBattleSizeY * kBattleSizePixel) {
            x = (x < 0 ? x + kBattleSizeX * kBattleSizePixel : (x >= kBattleSizeX * kBattleSizePixel ? x - kBattleSizeX * kBattleSizePixel : x)) / kBattleSizePixel;
            y = y / kBattleSizePixel;
            result = battle->maps[y][x];
        }
    }
    return result;
}

// バトルが空いているかどうかを判定する
//
bool BattleIsSpace(int x, int y)
{
    unsigned char m = BattleGetMap(x, y);
    return m != kBattleMapBlock ? true : false;
}
