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
    "Back", 
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
void BattleActorLoad(int type, int route)
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
            if (type == kBattleTypeField) {
                if ((route & (1 << kDirectionUp)) == 0) {
                    for (int x = 0; x < kBattleSizeX; x++) {
                        battle->maps[0][x] = kBattleMapBlock;
                    }
                }
                if ((route & (1 << kDirectionDown)) == 0) {
                    for (int x = 0; x < kBattleSizeX; x++) {
                        battle->maps[kBattleSizeY - 1][x] = kBattleMapBlock;
                    }
                }
                if ((route & (1 << kDirectionLeft)) == 0) {
                    for (int y = 0; y < kBattleSizeY; y++) {
                        battle->maps[y][0] = kBattleMapBlock;
                    }
                }
                if ((route & (1 << kDirectionRight)) == 0) {
                    for (int y = 0; y < kBattleSizeY; y++) {
                        battle->maps[y][kBattleSizeX - 1] = kBattleMapBlock;
                    }
                }
            } else {
                for (int x = 0; x < kBattleSizeX; x++) {
                    battle->maps[0][x] = kBattleMapBlock;
                }
                for (int x = 0; x < kBattleSizeX; x++) {
                    battle->maps[kBattleSizeY - 1][x] = kBattleMapBlock;
                }
                for (int y = 0; y < kBattleSizeY; y++) {
                    battle->maps[y][0] = kBattleMapBlock;
                }
                for (int y = 0; y < kBattleSizeY; y++) {
                    battle->maps[y][kBattleSizeX - 1] = kBattleMapBlock;
                }
                if ((route & (1 << kDirectionUp)) != 0) {
                    battle->maps[0][kBattleSizeX / 2 - 1] = kBattleMapFloor;
                    battle->maps[0][kBattleSizeX / 2 + 0] = kBattleMapFloor;
                }
                if ((route & (1 << kDirectionDown)) != 0) {
                    battle->maps[kBattleSizeY - 1][kBattleSizeX / 2 - 1] = kBattleMapFloor;
                    battle->maps[kBattleSizeY - 1][kBattleSizeX / 2 + 0] = kBattleMapFloor;
                }
                if ((route & (1 << kDirectionLeft)) != 0) {
                    battle->maps[kBattleSizeY / 2 - 1][0] = kBattleMapFloor;
                    battle->maps[kBattleSizeY / 2 + 0][0] = kBattleMapFloor;
                }
                if ((route & (1 << kDirectionRight)) != 0) {
                    battle->maps[kBattleSizeY / 2 - 1][kBattleSizeX - 1] = kBattleMapFloor;
                    battle->maps[kBattleSizeY / 2 + 0][kBattleSizeX - 1] = kBattleMapFloor;
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

    // クリップの設定
    BattleSetClip();

    // スプライトの描画
    {
        int viewx = camera->x >= 0 ? -(camera->x % kBattleSizePixel) : -kBattleSizePixel - (camera->x % kBattleSizePixel);
        int viewy = camera->y >= 0 ? -(camera->y % kBattleSizePixel) : -kBattleSizePixel - (camera->y % kBattleSizePixel);
        int mapx = camera->x >= 0 ? camera->x / kBattleSizePixel : camera->x / kBattleSizePixel - 1;
        int mapy = camera->y >= 0 ? camera->y / kBattleSizePixel : camera->y / kBattleSizePixel - 1;
        int my = mapy;
        for (int vy = viewy; vy < kBattleViewSizeY; vy += kBattleSizePixel) {
            int mx = mapx;
            for (int vx = viewx; vx < kBattleViewSizeX; vx += kBattleSizePixel) {
                int animation = kBattleAnimationNull;
                if (mx >= 0 && mx < kBattleSizeX && my >= 0 && my < kBattleSizeY) {
                    animation = battle->maps[my][mx];
                }
                AsepriteDrawSpriteAnimation(&actor->animations[animation], vx, vy, kDrawModeCopy, kBitmapUnflipped);
                ++mx;
            }
            ++my;
        }
    }

    // クリップの解除
    BattleClearClip();

    // DEBUG
    playdate->system->drawFPS(0, 0);
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
        GameSetBattleCamera(((kBattleSizeX * kBattleSizePixel) - kBattleViewSizeX) / 2 - kBattleViewLeft, ((kBattleSizeY * kBattleSizePixel) - kBattleViewSizeY) / 2 - kBattleViewTop);

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
        if (x >= 0 && x < kBattleSizeX * kBattleSizePixel && y >= 0 && y < kBattleSizeY * kBattleSizePixel) {
            x = x / kBattleSizePixel;
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

// バトル内で移動できる距離を取得する
//
int BattleGetMoveDistance(int x, int y, int direction, int speed, bool outside)
{
    int distance = 0;
    if (direction == kDirectionUp) {
        int y_0 = y;
        while (speed > 0) {
            int d = speed;
            if (d > kBattleSizePixel) {
                d = kBattleSizePixel;
            }
            speed -= d;
            y_0 -= d;
            if (outside && (y_0 < 0 || y_0 >= kBattleSizeY * kBattleSizePixel)) {
                ;
            } else if (!BattleIsSpace(x, y_0)) {
                if (y_0 > 0) {
                    y_0 = y_0 + (kBattleSizePixel - (y_0 % kBattleSizePixel));
                } else {
                    y_0 = 0;
                }
                break;
            }
        }
        distance = y - y_0;
    } else if (direction == kDirectionDown) {
        int y_0 = y;
        while (speed > 0) {
            int d = speed;
            if (d > kBattleSizePixel) {
                d = kBattleSizePixel;
            }
            speed -= d;
            y_0 += d;
            if (outside && (y_0 < 0 || y_0 >= kBattleSizeY * kBattleSizePixel)) {
                ;
            } else if (!BattleIsSpace(x, y_0)) {
                y_0 = y_0 - (y_0 % kBattleSizePixel) - 1;
                break;
            }
        }
        distance = y_0 - y;
    } else if (direction == kDirectionLeft) {
        int x_0 = x;
        while (speed > 0) {
            int d = speed;
            if (d > kBattleSizePixel) {
                d = kBattleSizePixel;
            }
            speed -= d;
            x_0 -= d;
            if (outside && (x_0 < 0 || x_0 >= kBattleSizeX * kBattleSizePixel)) {
                ;
            } else if (!BattleIsSpace(x_0, y)) {
                if (x_0 > 0) {
                    x_0 = x_0 + (kBattleSizePixel - (x_0 % kBattleSizePixel));
                } else {
                    x_0 = 0;
                }
                break;
            }
        }
        distance = x - x_0;
    } else if (direction == kDirectionRight) {
        int x_0 = x;
        while (speed > 0) {
            int d = speed;
            if (d > kBattleSizePixel) {
                d = kBattleSizePixel;
            }
            speed -= d;
            x_0 += d;
            if (outside && (x_0 < 0 || x_0 >= kBattleSizeX * kBattleSizePixel)) {
                ;
            } else if (!BattleIsSpace(x_0, y)) {
                x_0 = x_0 - (x_0 % kBattleSizePixel) - 1;
                break;
            }
        }
        distance = x_0 - x;
    }
    return distance;
}

// 開始位置を取得する
//
void BattleGetStartPosition(int direction, struct Vector *position)
{
    static const struct Vector positions[] = {
        {.x = (kBattleSizeX * kBattleSizePixel) / 2, .y = kBattleSizePixel - 1, }, 
        {.x = (kBattleSizeX * kBattleSizePixel) / 2, .y = kBattleSizeY * kBattleSizePixel - 1, }, 
        {.x = kBattleSizePixel / 2, .y = (kBattleSizeY * kBattleSizePixel) / 2 + (kBattleSizePixel / 2 - 1), }, 
        {.x = kBattleSizeX * kBattleSizePixel - kBattleSizePixel / 2, .y = (kBattleSizeY * kBattleSizePixel) / 2 + (kBattleSizePixel / 2 - 1), }, 
    };
    static const struct Vector offsets[] = {
        {.x = 0, .y = kBattleSizePixel, }, 
        {.x = 0, .y = -kBattleSizePixel, }, 
        {.x = kBattleSizePixel, .y = 0, }, 
        {.x = -kBattleSizePixel, .y = 0, }, 
    };
    *position = positions[direction];
    if (!BattleIsSpace(position->x, position->y)) {
        position->x += offsets[direction].x;
        position->y += offsets[direction].y;
    }
}

// エネミーの配置位置を取得する
//
void BattleGetEnemyPosition(int index, int direction, struct Vector *position)
{
    static const struct Vector o = {
        .x = (kBattleSizeX * kBattleSizePixel) / 2, .y = (kBattleSizeY * kBattleSizePixel) / 2 + (kBattleSizePixel / 2 - 1), 
    };
    static const struct Vector offsets[] = {
        { 0 * kBattleSizePixel,  0 * kBattleSizePixel, }, 
        { 1 * kBattleSizePixel,  1 * kBattleSizePixel, }, 
        {-1 * kBattleSizePixel,  1 * kBattleSizePixel, }, 
        {-1 * kBattleSizePixel,  0 * kBattleSizePixel, }, 
        { 1 * kBattleSizePixel, -1 * kBattleSizePixel, }, 
        { 0 * kBattleSizePixel, -1 * kBattleSizePixel, }, 
        { 0 * kBattleSizePixel,  1 * kBattleSizePixel, }, 
        { 1 * kBattleSizePixel,  0 * kBattleSizePixel, }, 
        {-1 * kBattleSizePixel, -1 * kBattleSizePixel, }, 
    };
    if (direction == kDirectionUp) {
        position->x = o.x + offsets[index].x;
        position->y = o.y + offsets[index].y;
    } else if (direction == kDirectionDown) {
        position->x = o.x - offsets[index].x;
        position->y = o.y - offsets[index].y;
    } else if (direction == kDirectionLeft) {
        position->x = o.x + offsets[index].y;
        position->y = o.y - offsets[index].x;
    } else {
        position->x = o.x - offsets[index].y;
        position->y = o.y + offsets[index].x;
    }
}

// クリップを設定する
//
void BattleClearClip(void)
{
    IocsGetPlaydate()->graphics->clearClipRect();
}
void BattleSetClip(void)
{
    IocsGetPlaydate()->graphics->setClipRect(kBattleViewLeft, kBattleViewTop, kBattleViewSizeX, kBattleViewSizeY);
}
