// Dungeon.c - ダンジョン
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
#include "Dungeon.h"

// 内部関数
//
static void DungeonBuildLocation(void);
static void DungeonUnbuildLocation(void);
static void DungeonBuildMap(void);
static void DungeonUnbuildMap(void);
static void DungeonLockLocation(int location);
static void DungeonDigLocation(int location);
static void DungeonActorUnload(struct DungeonActor *actor);
static void DungeonActorDraw(struct DungeonActor *actor);
static void DungeonActorLoop(struct DungeonActor *actor);

// 内部変数
//
static struct Dungeon *dungeon = NULL;


// ダンジョンを初期化する
//
void DungeonInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct DungeonActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: dungeon actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct DungeonActor));
    }

    // ダンジョンの作成
    dungeon = (struct Dungeon *)playdate->system->realloc(NULL, sizeof (struct Dungeon));
    if (dungeon == NULL) {
        playdate->system->error("%s: %d: dungeon instance is not created.", __FILE__, __LINE__);
    }

    // ダンジョンの初期化
    {
        // 乱数の設定
        IocsSetRandomSeed(&dungeon->xorshift, 987654321);

        // 配置の作成
        DungeonBuildLocation();

        // マップの作成
        DungeonBuildMap();
    }
}

// ダンジョンを解放する
//
void DungeonRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // マップの解放
    DungeonUnbuildMap();

    // 配置の解放
    DungeonUnbuildLocation();

    // ダンジョンの解放
    if (dungeon != NULL) {        
        playdate->system->realloc(dungeon, 0);
        dungeon = NULL;
    }
}

// 配置を作成する
//
static void DungeonBuildLocation(void)
{
    // 配置の初期化
    for (int i = 0; i < kDungeonLocationSize; i++) {
        dungeon->locations[i].left = i % kDungeonLocationSizeX;
        dungeon->locations[i].top = i / kDungeonLocationSizeX;
        dungeon->locations[i].right = dungeon->locations[i].left;
        dungeon->locations[i].bottom = dungeon->locations[i].top;
    }
    /*
    for (int i = 0; i < kDungeonLocationSize; i++) {
        int j = IocsGetRandomNumber(&dungeon->xorshift) % kDungeonLocationSize;
        struct Rect r = dungeon->locations[j];
        dungeon->locations[j] = dungeon->locations[i];
        dungeon->locations[i] = r;
    }
    */

    // 大きさの設定
    {
        ;
    }

    // 配置の範囲でランダムに設定
    for (int i = 0; i < kDungeonLocationSize; i++) {
        int locationx = dungeon->locations[i].left;
        int locationy = dungeon->locations[i].top;
        int sizex = dungeon->locations[i].right - locationx + 1;
        int sizey = dungeon->locations[i].bottom - locationy + 1;
        int mazex = (IocsGetRandomNumber(&dungeon->xorshift) % ((kDungeonLocationMazeSizeX - 1) - sizex)) + locationx * kDungeonLocationMazeSizeX;
        int mazey = (IocsGetRandomNumber(&dungeon->xorshift) % ((kDungeonLocationMazeSizeY - 1) - sizey)) + locationy * kDungeonLocationMazeSizeY;
        dungeon->locations[i].left = mazex * kDungeonSectionSizeX;
        dungeon->locations[i].top = mazey * kDungeonSectionSizeY;
        dungeon->locations[i].right = dungeon->locations[i].left + sizex * kDungeonSectionSizeX- 1;
        dungeon->locations[i].bottom = dungeon->locations[i].top + sizey * kDungeonSectionSizeY- 1;
    }
}

// 配置を解放する
//
static void DungeonUnbuildLocation(void)
{
    ;
}

// マップを作成する
//
static void DungeonBuildMap(void)
{
    // 迷路の作成
    {
        // 初期化
        dungeon->maze = MazeLoad(kDungeonMazeSizeX, kDungeonMazeSizeY, &dungeon->xorshift);

        // ロック
        {
            // 開始位置をロック
            DungeonLockLocation(kDungeonLocationStart);

            // 洞窟をロック
            for (int i = 0; i < kDungeonLocationCaveSize; i++) {
                DungeonLockLocation(kDungeonLocationCave + i);
            }

            // 城をロック
            DungeonLockLocation(kDungeonLocationCastle);
        }

        // 穴を掘る
        {
            int x = dungeon->locations[kDungeonLocationDig].left * kDungeonLocationMazeSizeX * 2 + 1;
            int y = dungeon->locations[kDungeonLocationDig].top * kDungeonLocationMazeSizeY * 2 + 1;
            MazeDig(dungeon->maze, x, y);
        }

        // 経路の設定
        MazeSetRoute(dungeon->maze);

        // 中心の設定
        for (int routey = 0; routey < dungeon->maze->routeSize.y; routey++) {
            for (int routex = 0; routex < dungeon->maze->routeSize.x; routex++) {
                dungeon->os[routey][routex].x = IocsGetRandomNumber(&dungeon->xorshift) % (kDungeonSectionSizeX - 1) + 1;
                dungeon->os[routey][routex].y = IocsGetRandomNumber(&dungeon->xorshift) % (kDungeonSectionSizeY - 1) + 1;
            }
        }
    }

    // マップの初期化
    {
        for (int mapy = 0; mapy < kDungeonSizeY; mapy++) {
            for (int mapx = 0; mapx < kDungeonSizeX; mapx++) {
                dungeon->maps[mapy][mapx] = kDungeonMapBack;
            }
        }
    }

    // マップの作成
    {
        for (int routey = 0; routey < dungeon->maze->routeSize.y; routey++) {
            int mapy = routey * kDungeonSectionSizeY;
            for (int routex = 0; routex < dungeon->maze->routeSize.x; routex++) {
                int mapx = routex * kDungeonSectionSizeX;
                unsigned char route = dungeon->maze->routes[routey * dungeon->maze->routeSize.x + routex];
                if (route == 0) {
                    for (int y = 0; y < kDungeonSectionSizeY; y++) {
                        for (int x = 0; x < kDungeonSectionSizeX; x++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                        }
                    }
                } else if (route == (kMazeRouteUp | kMazeRouteDown)) {
                    int x = dungeon->os[routey - 1][routex].x;
                    int y = 0;
                    while (y < dungeon->os[routey][routex].y) {
                        dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        ++y;
                    }
                    do {
                        dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                        x = x + (x < dungeon->os[routey][routex].x ? 1 : -1);
                    } while (x != dungeon->os[routey][routex].x);
                    while (y < kDungeonSectionSizeY) {
                        dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        ++y;
                    }
                } else if (route == kMazeRouteUp) {
                    {
                        int x = dungeon->os[routey - 1][routex].x;
                        for (int y = 0; y < dungeon->os[routey][routex].y; y++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        }
                    }
                    {
                        int y = dungeon->os[routey][routex].y;
                        for (int x = 0; x < kDungeonSectionSizeX; x++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                        }
                    }
                } else if (route == kMazeRouteDown) {
                    {
                        int y = dungeon->os[routey][routex].y;
                        for (int x = 0; x < kDungeonSectionSizeX; x++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                        }
                    }
                    {
                        int x = dungeon->os[routey][routex].x;
                        for (int y = dungeon->os[routey][routex].y; y < kDungeonSectionSizeY; y++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        }
                    }
                } else {
                    unsigned char lr = route & (kMazeRouteLeft | kMazeRouteRight);
                    if (lr == kMazeRouteLeft) {
                        int x = 0;
                        int y = dungeon->os[routey][routex - 1].y;
                        if (y < dungeon->os[routey][routex].y) {
                            while (y < dungeon->os[routey][routex].y) {
                                dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                                ++y;
                            }
                        } else if (y > dungeon->os[routey][routex].y) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                            do {
                                --y;
                                dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                            } while (y > dungeon->os[routey][routex].y);
                            ++x;
                        }
                        int t = routey > 0 && dungeon->os[routey - 1][routex].x > dungeon->os[routey][routex].x ? dungeon->os[routey - 1][routex].x : dungeon->os[routey][routex].x;
                        while (x <= t) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                            ++x;
                        }
                        if (routey >= dungeon->maze->routeSize.y - 1) {
                            while (y < kDungeonSectionSizeY) {
                                dungeon->maps[mapy + y][mapx + t] = kDungeonMapLadder;
                                ++y;
                            }
                        }
                    } else if (lr == kMazeRouteRight) {
                        int y = dungeon->os[routey][routex].y;
                        int t = routey > 0 && dungeon->os[routey - 1][routex].x < dungeon->os[routey][routex].x ? dungeon->os[routey - 1][routex].x : dungeon->os[routey][routex].x;
                        for (int x = t; x < kDungeonSectionSizeX; x++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                        }
                        if (routey >= dungeon->maze->routeSize.y - 1) {
                            while (y < kDungeonSectionSizeY) {
                                dungeon->maps[mapy + y][mapx + t] = kDungeonMapLadder;
                                ++y;
                            }
                        }
                    } else {
                        int y = dungeon->os[routey][routex - 1].y;
                        int h = y - dungeon->os[routey][routex].y;
                        if (h < 0) {
                            h = -h;
                            if (h > dungeon->os[routey][routex].x) {
                                int x = 0;
                                while (x < dungeon->os[routey][routex].x) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                }
                                while (y < dungeon->os[routey][routex].y) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                                    ++y;
                                }
                                while (x < kDungeonSectionSizeX) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                }
                            } else {
                                int x = 0;
                                do {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                                    ++y;
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                } while (y < dungeon->os[routey][routex].y);
                                while (x < kDungeonSectionSizeX) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                }
                            }
                        } else if (y > dungeon->os[routey][routex].y) {
                            if (h > dungeon->os[routey][routex].x) {
                                int x = 0;
                                do {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                } while (x < dungeon->os[routey][routex].x);
                                --x;
                                while (y > dungeon->os[routey][routex].y) {
                                    --y;
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                                }
                                ++x;
                                while (x < kDungeonSectionSizeX) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                }
                            } else {
                                int x = 0;
                                do {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    --y;
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                                    ++x;
                                } while (y > dungeon->os[routey][routex].y);
                                while (x < kDungeonSectionSizeX) {
                                    dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                                    ++x;
                                }
                            }
                        } else {
                            for (int x = 0; x < kDungeonSectionSizeX; x++) {
                                dungeon->maps[mapy + y][mapx + x] = kDungeonMapBlock;
                            }
                        }
                    }
                    if ((route & kMazeRouteUp) != 0) {
                        int x = dungeon->os[routey - 1][routex].x;
                        for (int y = 0; y < dungeon->os[routey][routex].y; y++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        }
                    }
                    if ((route & kMazeRouteDown) != 0) {
                        int x = dungeon->os[routey][routex].x;
                        for (int y = dungeon->os[routey][routex].y; y < kDungeonSectionSizeY; y++) {
                            dungeon->maps[mapy + y][mapx + x] = kDungeonMapLadder;
                        }
                    }
                }
            }
        }
    }

    // ロックした場所を開ける
    {
        // 開始位置を開ける
        DungeonDigLocation(kDungeonLocationStart);

        // 洞窟を開ける
        for (int i = 0; i < kDungeonLocationCaveSize; i++) {
            DungeonDigLocation(kDungeonLocationCave + i);
        }

        // 城を開ける
        DungeonDigLocation(kDungeonLocationCastle);
    }

    // 建物などを置く
    {
        // 開始位置を置く

        // 洞窟の入り口を置く
        for (int i = 0; i < kDungeonLocationCaveSize; i++) {
            int location = kDungeonLocationCave + i;
            if (location >= kDungeonLocationCave && location < kDungeonLocationCave + kDungeonLocationCaveSize) {
                int x = ((dungeon->locations[location].right - dungeon->locations[location].left + 1) - kDungeonCaveSizeX) / 2 + dungeon->locations[location].left;
                int y = dungeon->locations[location].bottom - kDungeonCaveSizeY;
                for (int h = 0; h < kDungeonCaveSizeY; h++) {
                    for (int w = 0; w < kDungeonCaveSizeX; w++) {
                        dungeon->maps[y + h][x + w] = kDungeonMapCave00 + h * kDungeonCaveSizeX + w;
                    }
                }
            }
        }

        // 城を置く
        {
            int x = ((dungeon->locations[kDungeonLocationCastle].right - dungeon->locations[kDungeonLocationCastle].left + 1) - kDungeonCastleSizeX) / 2 + dungeon->locations[kDungeonLocationCastle].left;
            int y = dungeon->locations[kDungeonLocationCastle].bottom - kDungeonCastleSizeY;
            for (int h = 0; h < kDungeonCastleSizeY; h++) {
                for (int w = 0; w < kDungeonCastleSizeX; w++) {
                    dungeon->maps[y + h][x + w] = kDungeonMapCastle00 + h * kDungeonCastleSizeX + w;
                }
            }
        }
    }

    // 柱を立てる
    for (int y = 1; y < kDungeonSizeY - 1; y++) {
        for (int x = 1; x < kDungeonSizeX - 1; x++) {
            if (
                DungeonIsMapBlock(dungeon->maps[y - 1][x]) && 
                DungeonIsMapBack(dungeon->maps[y + 0][x]) && 
                DungeonIsMapBack(dungeon->maps[y + 1][x]) 
            ) {
                if (
                    (DungeonIsMapBack(dungeon->maps[y - 1][x - 1]) && DungeonIsMapBlock(dungeon->maps[y - 1][x + 1])) || 
                    (DungeonIsMapBack(dungeon->maps[y - 1][x + 1]) && DungeonIsMapBlock(dungeon->maps[y - 1][x - 1]))
                ) {
                    int y_0 = y + 1;
                    while (y_0 < kDungeonSizeY && DungeonIsMapBack(dungeon->maps[y_0][x])) {
                        ++y_0;
                    }
                    if (y_0 >= kDungeonSizeY || DungeonIsMapBlock(dungeon->maps[y_0][x])) {
                        for (int y_1 = y; y_1 < y_0; y_1++) {
                            dungeon->maps[y_1][x] = kDungeonMapPole;
                        }
                    }
                }
            }
        }
    }

    // つららを立てる
    for (int y = 0; y < kDungeonSizeY - 1; y++) {
        for (int x = 0; x < kDungeonSizeX; x++) {
            if (
                (y == 0 || DungeonIsMapBlock(dungeon->maps[y - 1][x])) && 
                (DungeonIsMapLock(dungeon->maps[y + 0][x + 0]) || DungeonIsMapBack(dungeon->maps[y + 0][x + 0])) && 
                (DungeonIsMapLock(dungeon->maps[y + 1][x + 0]) || DungeonIsMapBack(dungeon->maps[y + 1][x + 0])) && 
                !DungeonIsMapBlock(dungeon->maps[y + 1][x - 1]) && 
                !DungeonIsMapBlock(dungeon->maps[y + 1][x + 1])
            ) {
                dungeon->maps[y][x] = kDungeonMapIcicle;
            }
        }
    }
}

// マップを解放する
//
static void DungeonUnbuildMap(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 迷路の解放
    if (dungeon != NULL) {
        MazeUnload(dungeon->maze);
    }
}

// 指定した配置をロックする
//
static void DungeonLockLocation(int location)
{
    int left = (dungeon->locations[location].left / kDungeonSectionSizeX) * 2 + 1;
    int top = (dungeon->locations[location].top / kDungeonSectionSizeY) * 2 + 1;
    int right = (dungeon->locations[location].right / kDungeonSectionSizeX) * 2 + 1;
    int bottom = (dungeon->locations[location].bottom / kDungeonSectionSizeY) * 2 + 1;
    for (int y = top; y <= bottom; y += 2) {
        for (int x = left; x <= right; x += 2) {
            dungeon->maze->maps[y * dungeon->maze->mapSize.x + x] |= kMazeMapLock;
        }
    }
}

// 指定した配置に空間を作る
//
static void DungeonDigLocation(int location)
{
    // 空間の作成
    for (int y = dungeon->locations[location].top; y < dungeon->locations[location].bottom; y++) {
        for (int x = dungeon->locations[location].left; x <= dungeon->locations[location].right; x++) {
            dungeon->maps[y][x] = kDungeonMapLock;
        }
    }
    for (int x = dungeon->locations[location].left; x <= dungeon->locations[location].right; x++) {
        dungeon->maps[dungeon->locations[location].bottom][x] = kDungeonMapBlock;
    }

    // 左側をつなげる
    if (dungeon->locations[location].left > 0) {
        int x = dungeon->locations[location].left - 1;
        int y = dungeon->locations[location].bottom;
        while (!DungeonIsMapBlock(dungeon->maps[y][x]) && !DungeonIsMapLadder(dungeon->maps[y][x])) {
            --y;
            if (y < dungeon->locations[location].top) {
                y = dungeon->locations[location].bottom;
                --x;
                if (x < 0) {
                    break;
                }
            }
        }
        ++x;
        while (x < dungeon->locations[location].left) {
            dungeon->maps[y][x] = kDungeonMapBlock;
            ++x;
        }
        while (y < dungeon->locations[location].bottom) {
            dungeon->maps[y][x] = kDungeonMapLadder;
            ++y;
        }
    }

    // 右側をつなげる
    if (dungeon->locations[location].right < kDungeonSizeX - 1) {
        int x = dungeon->locations[location].right + 1;
        int y = dungeon->locations[location].bottom;
        while (!DungeonIsMapBlock(dungeon->maps[y][x]) && !DungeonIsMapLadder(dungeon->maps[y][x])) {
            --y;
            if (y < dungeon->locations[location].top) {
                y = dungeon->locations[location].bottom;
                ++x;
                if (x >= kDungeonSizeX) {
                    break;
                }
            }
        }
        --x;
        while (x > dungeon->locations[location].right) {
            dungeon->maps[y][x] = kDungeonMapBlock;
            --x;
        }
        while (y < dungeon->locations[location].bottom) {
            dungeon->maps[y][x] = kDungeonMapLadder;
            ++y;
        }
    }
}

// ダンジョンアクタを読み込む
//
void DungeonActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct DungeonActor *actor = (struct DungeonActor *)ActorLoad((ActorFunction)DungeonActorLoop, kGamePriorityDungeon);
    if (actor == NULL) {
        playdate->system->error("%s: %d: dungeon actor is not loaded.", __FILE__, __LINE__);
    }

    // ダンジョンの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)DungeonActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagDungeon);

        // スプライトの作成
        actor->animations = (struct AsepriteSpriteAnimation *)playdate->system->realloc(NULL, kDungeonAnimationSize * sizeof (struct AsepriteSpriteAnimation));
        if (actor->animations == NULL) {
            playdate->system->error("%s: %d: dungeon actor animation is not created.", __FILE__, __LINE__);
        }

    }
}

// ダンジョンアクタを解放する
//
static void DungeonActorUnload(struct DungeonActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // スプライトの解放
    if (actor->animations != NULL) {
        playdate->system->realloc(actor->animations, 0);
    }
}

// ダンジョンアクタを描画する
//
static void DungeonActorDraw(struct DungeonActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // カメラの取得
    struct Vector *camera = GameGetCamera();

    // クリップの設定
    DungeonSetClip();

    // スプライトの描画
    {
        int viewx = camera->x >= 0 ? -(camera->x % kDungeonSizePixel) : -kDungeonSizePixel - (camera->x % kDungeonSizePixel);
        int viewy = camera->y >= 0 ? -(camera->y % kDungeonSizePixel) : -kDungeonSizePixel - (camera->y % kDungeonSizePixel);
        int mapx = camera->x >= 0 ? camera->x / kDungeonSizePixel : camera->x / kDungeonSizePixel - 1;
        int mapy = camera->y >= 0 ? camera->y / kDungeonSizePixel : camera->y / kDungeonSizePixel - 1;
        int my = mapy;
        for (int vy = viewy; vy < kDungeonViewSizeY; vy += kDungeonSizePixel) {
            int mx = mapx;
            for (int vx = viewx; vx < kDungeonViewSizeX; vx += kDungeonSizePixel) {
                int animation = kDungeonAnimationBlock;
                if (my >= 0 && my < kDungeonSizeY) {
                    int ax = mx < 0 ? mx + kDungeonSizeX : (mx >= kDungeonSizeX ? mx - kDungeonSizeX : mx);
                    animation = dungeon->maps[my][ax];
                }
                AsepriteDrawSpriteAnimation(&actor->animations[animation], vx, vy, kDrawModeCopy, kBitmapUnflipped);
                ++mx;
            }
            ++my;
        }
    }

    // クリップの解除
    DungeonClearClip();

    // DEBUG
    playdate->system->drawFPS(0, 0);
}

// ダンジョンアクタが待機する
//
static void DungeonActorLoop(struct DungeonActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // アニメーションの開始
        for (int i = 0; i < kDungeonAnimationSize; i++) {
            AsepriteStartSpriteAnimation(&actor->animations[i], "tileset", dungeonAnimationNames[i], false);
        }

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの更新
        for (int i = 0; i < kDungeonAnimationSize; i++) {
            AsepriteUpdateSpriteAnimation(&actor->animations[i]);
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)DungeonActorDraw, kGameOrderDungeon);
}

// マップを取得する
//
unsigned char DungeonGetMap(int x, int y)
{
    unsigned char result = kDungeonMapBlock;
    if (dungeon != NULL) {
        if (y >= 0 && y < kDungeonSizeY * kDungeonSizePixel) {
            x = (x < 0 ? x + kDungeonSizeX * kDungeonSizePixel : (x >= kDungeonSizeX * kDungeonSizePixel ? x - kDungeonSizeX * kDungeonSizePixel : x)) / kDungeonSizePixel;
            y = y / kDungeonSizePixel;
            result = dungeon->maps[y][x];
        }
    }
    return result;
}

// ダンジョンマップを判定する
//
static bool DungeonIsMapBack(unsigned char map)
{
    return map == kDungeonMapBack ? true : false;
}
static bool DungeonIsMapBlock(unsigned char map)
{
    return map == kDungeonMapBlock|| map == kDungeonMapSolid || map == kDungeonMapChecker ? true : false;
}
static bool DungeonIsMapLadder(unsigned char map)
{
    return map == kDungeonMapLadder ? true : false;
}
static bool DungeonIsMapLock(unsigned char map)
{
    return map == kDungeonMapLock ? true : false;
}

// ダンジョンが空いているかどうかを判定する
//
bool DungeonIsSpace(int x, int y)
{
    unsigned char m = DungeonGetMap(x, y);
    return !DungeonIsMapBlock(m) && m != kDungeonMapIcicle ? true : false;
}

// ダンジョンが梯子かどうかを判定する
//
bool DungeonIsLadder(int x, int y)
{
    return DungeonIsMapLadder(DungeonGetMap(x, y));
}

// ダンジョンが洞窟かどうかを判定する
//
bool DungeonIsCave(int x, int y)
{
    return DungeonGetMap(x, y) == kDungeonMapCaveEntrance ? true : false;
}

// ダンジョンが城かどうかを判定する
//
bool DungeonIsCastle(int x, int y)
{
    return DungeonGetMap(x, y) == kDungeonMapCastleEntrance ? true : false;
}

// ダンジョンで落下するかどうかを判定する
//
bool DungeonIsLand(int x, int y)
{
    return !DungeonIsSpace(x, y + kDungeonSizePixel) || DungeonIsLadder(x, y) || DungeonIsLadder(x, y + kDungeonSizePixel) ? true : false;
}
bool DungeonIsFall(int x, int y)
{
    return !DungeonIsLand(x, y);
}

// ダンジョンを指定した方向に移動する
//
bool DungeonIsWalk(int x, int y, int direction, bool jump, bool fall)
{
    return DungeonWalk(x, y, direction, jump, fall, NULL);
}
bool DungeonWalk(int x, int y, int direction, bool jump, bool fall, struct Vector *to)
{
    bool result = false;
    if (direction == kDirectionUp) {
        if (
            DungeonIsSpace(x, y - kDungeonSizePixel) && 
            (
                jump || 
                DungeonIsLadder(x, y)
            )
        ) {
            if (to != NULL) {
                to->x = x;
                to->y = y - kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpLeft) {
        if (
            DungeonIsSpace(x - kDungeonSizePixel, y) &&
            DungeonIsSpace(x, y - kDungeonSizePixel) && 
            (
                (
                    jump && 
                    DungeonIsSpace(x - kDungeonSizePixel, y - kDungeonSizePixel) 
                ) || 
                (
                    DungeonIsLadder(x, y) &&
                    DungeonIsLadder(x - kDungeonSizePixel, y - kDungeonSizePixel) 
                )
            )
        ) {
            if (to != NULL) {
                to->x = x - kDungeonSizePixel;
                to->y = y - kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpRight) {
        if (
            DungeonIsSpace(x + kDungeonSizePixel, y) && 
            DungeonIsSpace(x, y - kDungeonSizePixel) && 
            (
                (
                    jump && 
                    DungeonIsSpace(x + kDungeonSizePixel, y - kDungeonSizePixel)
                ) || 
                (
                    DungeonIsLadder(x, y) && 
                    DungeonIsLadder(x + kDungeonSizePixel, y - kDungeonSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x + kDungeonSizePixel;
                to->y = y - kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDown) {
        if (
            (
                fall && 
                DungeonIsSpace(x, y + kDungeonSizePixel)
            ) || 
            DungeonIsLadder(x, y + kDungeonSizePixel)
        ) {
            if (to != NULL) {
                to->x = x;
                to->y = y + kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownLeft) {
        if (
            DungeonIsSpace(x - kDungeonSizePixel, y) && 
            DungeonIsSpace(x, y + kDungeonSizePixel) && 
            (
                (
                    fall &&
                    DungeonIsSpace(x - kDungeonSizePixel, y + kDungeonSizePixel)
                ) || 
                (
                    DungeonIsLadder(x, y) && 
                    DungeonIsLadder(x - kDungeonSizePixel, y + kDungeonSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x - kDungeonSizePixel;
                to->y = y + kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownRight) {
        if (
            DungeonIsSpace(x + kDungeonSizePixel, y) && 
            DungeonIsSpace(x, y + kDungeonSizePixel) && 
            (
                (
                    fall && 
                    DungeonIsSpace(x + kDungeonSizePixel, y + kDungeonSizePixel)
                ) || 
                (
                    DungeonIsLadder(x, y) && 
                    DungeonIsLadder(x + kDungeonSizePixel, y + kDungeonSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x + kDungeonSizePixel;
                to->y = y + kDungeonSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionLeft) {
        if (
            DungeonIsSpace(x - kDungeonSizePixel, y) && 
            (
                fall || 
                DungeonIsLand(x - kDungeonSizePixel, y)
            )
        ) {
            if (to != NULL) {
                to->x = x - kDungeonSizePixel;
                to->y = y;
            }
            result = true;
        }
    } else if (direction == kDirectionRight) {
        if (
            DungeonIsSpace(x + kDungeonSizePixel, y) && 
            (
                fall || 
                DungeonIsLand(x + kDungeonSizePixel, y)
            )
        ) {
            if (to != NULL) {
                to->x = x + kDungeonSizePixel;
                to->y = y;
            }
            result = true;
        }
    }
    return result;
}

// 移動する位置を調整する
//
void DungeonAdjustMovePosition(struct Vector *from, struct Vector *to)
{
    if (to->x < 0) {
        to->x += kDungeonSizeX * kDungeonSizePixel;
        from->x += kDungeonSizeX * kDungeonSizePixel;
    } else if (to->x >= kDungeonSizeX * kDungeonSizePixel) {
        to->x -= kDungeonSizeX * kDungeonSizePixel;
        from->x -= kDungeonSizeX * kDungeonSizePixel;
    }
}

// 開始位置を取得する
//
void DungeonGetStartPosition(struct Vector *position)
{
    int x = (dungeon->locations[kDungeonLocationStart].left + dungeon->locations[kDungeonLocationStart].right + 1) / 2;
    int y = dungeon->locations[kDungeonLocationStart].top;
    while (!DungeonIsSpace(x * kDungeonSizePixel, y * kDungeonSizePixel)) {
        ++y;
    }
    while (DungeonIsSpace(x * kDungeonSizePixel, y * kDungeonSizePixel)) {
        ++y;
    }
    position->x = x * kDungeonSizePixel + kDungeonSizePixel / 2;
    position->y = y * kDungeonSizePixel - 1;
}

// エネミーの配置位置を取得する
//
void DungeonGetEnemyPosition(struct Vector *position, bool land)
{
    int x = (dungeon->locations[dungeon->locationEnemy].left + dungeon->locations[dungeon->locationEnemy].right + 1) / 2;
    int y = dungeon->locations[dungeon->locationEnemy].top;
    while (!DungeonIsSpace(x * kDungeonSizePixel, y * kDungeonSizePixel)) {
        ++y;
    }
    while (DungeonIsSpace(x * kDungeonSizePixel, y * kDungeonSizePixel)) {
        ++y;
    }
    position->x = x * kDungeonSizePixel + kDungeonSizePixel / 2;
    if (land) {
        position->y = y * kDungeonSizePixel - 1;
    } else {
        position->y = ((y + dungeon->locations[dungeon->locationEnemy].top) / 2) * kDungeonSizePixel - 1;
    }
    ++dungeon->locationEnemy;
    if (dungeon->locationEnemy >= kDungeonLocationSize) {
        dungeon->locationEnemy = kDungeonLocationEnemy;
    }
}

// 指定された方向の位置を取得する
//
void DungeonGetDirectinalPosition(int x, int y, int direction, struct Vector *position)
{
    if (direction == kDirectionUp) {
        position->x = x;
        position->y = y - kDungeonSizePixel;
    } else if (direction == kDirectionDown) {
        position->x = x;
        position->y = y + kDungeonSizePixel;
    } else if (direction == kDirectionLeft) {
        position->x = x - kDungeonSizePixel;
        position->y = y;
    } else if (direction == kDirectionRight) {
        position->x = x + kDungeonSizePixel;
        position->y = y;
    }
}

// バトルの経路を取得する
//
int DungeonGetBattleRoute(int x, int y)
{
    int route = 0;
    if (DungeonIsSpace(x, y - kDungeonSizePixel)) {
        route |= (1 << kDirectionUp);
    }
    if (DungeonIsSpace(x, y + kDungeonSizePixel)) {
        route |= (1 << kDirectionDown);
    }
    if (DungeonIsSpace(x - kDungeonSizePixel, y)) {
        route |= (1 << kDirectionLeft);
    }
    if (DungeonIsSpace(x + kDungeonSizePixel, y)) {
        route |= (1 << kDirectionRight);
    }
    return route;
}

// クリップを設定する
//
void DungeonClearClip(void)
{
    IocsGetPlaydate()->graphics->clearClipRect();
}
void DungeonSetClip(void)
{
    IocsGetPlaydate()->graphics->setClipRect(kDungeonViewLeft, kDungeonViewTop, kDungeonViewSizeX, kDungeonViewSizeY);
}


