// Field.c - フィールド
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
#include "Field.h"

// 内部関数
//
static void FieldBuildLocation(void);
static void FieldUnbuildLocation(void);
static void FieldBuildMap(void);
static void FieldUnbuildMap(void);
static void FieldLockLocation(int location);
static void FieldDigLocation(int location);
static void FieldActorUnload(struct FieldActor *actor);
static void FieldActorDraw(struct FieldActor *actor);
static void FieldActorLoop(struct FieldActor *actor);
static int FieldGetMapX(int x);
static int FieldGetMapY(int x);
static bool FieldIsMapBack(unsigned char map);
static bool FieldIsMapBlock(unsigned char map);
static bool FieldIsMapLadder(unsigned char map);
static bool FieldIsMapLock(unsigned char map);

// 内部変数
//
static struct Field *field = NULL;
static const char *fieldAnimationNames[kFieldAnimationSize] = {
    "Back", 
    "Back", 
    "Back", 
    "Block", 
    "Solid", 
    "Checker", 
    "Ladder", 
    "Icicle", 
    "Pole", 
    "Cave00", 
    "Cave01", 
    "Cave02", 
    "Cave10", 
    "Cave11", 
    "Cave12", 
    "Castle00", 
    "Castle01", 
    "Castle02", 
    "Castle03", 
    "Castle04", 
    "Castle05", 
    "Castle06", 
    "Castle10", 
    "Castle11", 
    "Castle12", 
    "Castle13", 
    "Castle14", 
    "Castle15", 
    "Castle16", 
    "Castle20", 
    "Castle21", 
    "Castle22", 
    "Castle23", 
    "Castle24", 
    "Castle25", 
    "Castle26", 
    "Castle30", 
    "Castle31", 
    "Castle32", 
    "Castle33", 
    "Castle34", 
    "Castle35", 
    "Castle36", 
    "Castle40", 
    "Castle41", 
    "Castle42", 
    "Castle43", 
    "Castle44", 
    "Castle45", 
    "Castle46", 
    "Shop00", 
    "Shop01", 
    "Shop02", 
    "Shop10", 
    "Shop11", 
    "Shop12", 
    "Shop20", 
    "Shop21", 
    "Shop22", 
};


// フィールドを初期化する
//
void FieldInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct FieldActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: field actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct FieldActor));
    }

    // フィールドの作成
    field = (struct Field *)playdate->system->realloc(NULL, sizeof (struct Field));
    if (field == NULL) {
        playdate->system->error("%s: %d: field instance is not created.", __FILE__, __LINE__);
    }

    // フィールドの初期化
    {
        // 乱数の設定
        IocsSetRandomSeed(&field->xorshift, 123456789);

        // 配置の作成
        FieldBuildLocation();

        // マップの作成
        FieldBuildMap();
    }
}

// フィールドを解放する
//
void FieldRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // マップの解放
    FieldUnbuildMap();

    // 配置の解放
    FieldUnbuildLocation();

    // フィールドの解放
    if (field != NULL) {        
        playdate->system->realloc(field, 0);
        field = NULL;
    }
}

// 配置を作成する
//
static void FieldBuildLocation(void)
{
    // 配置の初期化
    for (int i = 0; i < kFieldLocationSize; i++) {
        field->locations[i].left = i % kFieldLocationSizeX;
        field->locations[i].top = i / kFieldLocationSizeX;
        field->locations[i].right = field->locations[i].left;
        field->locations[i].bottom = field->locations[i].top;
    }
    /*
    for (int i = 0; i < kFieldLocationSize; i++) {
        int j = IocsGetRandomNumber(&field->xorshift) % kFieldLocationSize;
        struct Rect r = field->locations[j];
        field->locations[j] = field->locations[i];
        field->locations[i] = r;
    }
    */

    // 大きさの設定
    {
        // 開始位置の大きさの設定
        ++field->locations[kFieldLocationStart].right;
        ++field->locations[kFieldLocationStart].bottom;

        // 洞窟の大きさの設定

        // 城の大きさの設定
        ++field->locations[kFieldLocationCastle].right;
        ++field->locations[kFieldLocationCastle].bottom;

        // 店の大きさの設定
        for (int i = 0; i < kFieldLocationShopSize; i++) {
            ++field->locations[kFieldLocationShop + i].right;
            ++field->locations[kFieldLocationShop + i].bottom;
        }

        // エネミーの大きさの設定
        for (int i = kFieldLocationEnemy; i < kFieldLocationSize; i++) {
            field->locations[i].right = field->locations[i].left + kFieldLocationAreaSizeX - 1;
            field->locations[i].bottom = field->locations[i].top + kFieldLocationAreaSizeY - 1;
        }
    }

    // 配置の範囲をランダムに設定
    for (int i = 0; i < kFieldLocationSize; i++) {
        int locationx = field->locations[i].left;
        int locationy = field->locations[i].top;
        int areax = locationx * kFieldLocationAreaSizeX;
        int areay = locationy * kFieldLocationAreaSizeY;
        int sizex = field->locations[i].right - locationx + 1;
        int sizey = field->locations[i].bottom - locationy + 1;
        if (sizex < kFieldLocationAreaSizeX - 1) {
            areax += IocsGetRandomNumber(&field->xorshift) % ((kFieldLocationAreaSizeX - 1 - sizex));
        }
        if (sizey < kFieldLocationAreaSizeY - 1) {
            areay += IocsGetRandomNumber(&field->xorshift) % ((kFieldLocationAreaSizeY - 1 - sizey));
        }
        field->locations[i].left = areax * kFieldSectionSizeX;
        field->locations[i].top = areay * kFieldSectionSizeY;
        field->locations[i].right = field->locations[i].left + sizex * kFieldSectionSizeX- 1;
        field->locations[i].bottom = field->locations[i].top + sizey * kFieldSectionSizeY- 1;
    }

    // エネミーの設定
    field->locationEnemy = kFieldLocationEnemy;
}

// 配置を解放する
//
static void FieldUnbuildLocation(void)
{
    ;
}

// マップを作成する
//
static void FieldBuildMap(void)
{
    // 迷路の作成
    {
        // 初期化
        field->maze = MazeLoad(kFieldMazeSizeX, kFieldMazeSizeY, &field->xorshift);

        // ロック
        {
            // 開始位置をロック
            FieldLockLocation(kFieldLocationStart);

            // 洞窟をロック
            for (int i = 0; i < kFieldLocationCaveSize; i++) {
                FieldLockLocation(kFieldLocationCave + i);
            }

            // 城をロック
            FieldLockLocation(kFieldLocationCastle);

            // 店をロック
            for (int i = 0; i < kFieldLocationShopSize; i++) {
                FieldLockLocation(kFieldLocationShop + i);
            }
        }

        // 穴を掘る
        {
            int x = (field->locations[kFieldLocationDig].left / kFieldSectionSizeX) * 2 + 1;
            int y = (field->locations[kFieldLocationDig].top / kFieldSectionSizeY) * 2 + 1;
            MazeDig(field->maze, x, y);
        }

        // 経路の設定
        MazeSetRoute(field->maze);

        // 中心の設定
        for (int routey = 0; routey < field->maze->routeSize.y; routey++) {
            for (int routex = 0; routex < field->maze->routeSize.x; routex++) {
                field->os[routey][routex].x = IocsGetRandomNumber(&field->xorshift) % (kFieldSectionSizeX - 1) + 1;
                field->os[routey][routex].y = IocsGetRandomNumber(&field->xorshift) % (kFieldSectionSizeY - 1) + 1;
            }
        }
    }

    // マップの初期化
    {
        for (int mapy = 0; mapy < kFieldSizeY; mapy++) {
            for (int mapx = 0; mapx < kFieldSizeX; mapx++) {
                field->maps[mapy][mapx] = kFieldMapBack;
            }
        }
    }

    // マップの作成
    {
        for (int routey = 0; routey < field->maze->routeSize.y; routey++) {
            int mapy = routey * kFieldSectionSizeY;
            for (int routex = 0; routex < field->maze->routeSize.x; routex++) {
                int mapx = routex * kFieldSectionSizeX;
                unsigned char route = field->maze->routes[routey * field->maze->routeSize.x + routex];
                if (route == 0) {
                    for (int y = 0; y < kFieldSectionSizeY; y++) {
                        for (int x = 0; x < kFieldSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                    }
                } else if (route == (kMazeRouteUp | kMazeRouteDown)) {
                    int x = field->os[routey - 1][routex].x;
                    int y = 0;
                    while (y < field->os[routey][routex].y) {
                        field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        ++y;
                    }
                    do {
                        field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        x = x + (x < field->os[routey][routex].x ? 1 : -1);
                    } while (x != field->os[routey][routex].x);
                    while (y < kFieldSectionSizeY) {
                        field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        ++y;
                    }
                } else if (route == kMazeRouteUp) {
                    {
                        int x = field->os[routey - 1][routex].x;
                        for (int y = 0; y < field->os[routey][routex].y; y++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        }
                    }
                    {
                        int y = field->os[routey][routex].y;
                        for (int x = 0; x < kFieldSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                    }
                } else if (route == kMazeRouteDown) {
                    {
                        int y = field->os[routey][routex].y;
                        for (int x = 0; x < kFieldSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                    }
                    {
                        int x = field->os[routey][routex].x;
                        for (int y = field->os[routey][routex].y; y < kFieldSectionSizeY; y++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        }
                    }
                } else {
                    unsigned char lr = route & (kMazeRouteLeft | kMazeRouteRight);
                    if (lr == kMazeRouteLeft) {
                        int x = 0;
                        int y = field->os[routey][routex - 1].y;
                        if (y < field->os[routey][routex].y) {
                            while (y < field->os[routey][routex].y) {
                                field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                                ++y;
                            }
                        } else if (y > field->os[routey][routex].y) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                            do {
                                --y;
                                field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                            } while (y > field->os[routey][routex].y);
                            ++x;
                        }
                        int t = routey > 0 && field->os[routey - 1][routex].x > field->os[routey][routex].x ? field->os[routey - 1][routex].x : field->os[routey][routex].x;
                        while (x <= t) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                            ++x;
                        }
                        if (routey >= field->maze->routeSize.y - 1) {
                            while (y < kFieldSectionSizeY) {
                                field->maps[mapy + y][mapx + t] = kFieldMapLadder;
                                ++y;
                            }
                        }
                    } else if (lr == kMazeRouteRight) {
                        int y = field->os[routey][routex].y;
                        int t = routey > 0 && field->os[routey - 1][routex].x < field->os[routey][routex].x ? field->os[routey - 1][routex].x : field->os[routey][routex].x;
                        for (int x = t; x < kFieldSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                        if (routey >= field->maze->routeSize.y - 1) {
                            while (y < kFieldSectionSizeY) {
                                field->maps[mapy + y][mapx + t] = kFieldMapLadder;
                                ++y;
                            }
                        }
                    } else {
                        int y = field->os[routey][routex - 1].y;
                        int h = y - field->os[routey][routex].y;
                        if (h < 0) {
                            h = -h;
                            if (h > field->os[routey][routex].x) {
                                int x = 0;
                                while (x < field->os[routey][routex].x) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                                while (y < field->os[routey][routex].y) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                                    ++y;
                                }
                                while (x < kFieldSectionSizeX) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                            } else {
                                int x = 0;
                                do {
                                    field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                                    ++y;
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                } while (y < field->os[routey][routex].y);
                                while (x < kFieldSectionSizeX) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                            }
                        } else if (y > field->os[routey][routex].y) {
                            if (h > field->os[routey][routex].x) {
                                int x = 0;
                                do {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                } while (x < field->os[routey][routex].x);
                                --x;
                                while (y > field->os[routey][routex].y) {
                                    --y;
                                    field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                                }
                                ++x;
                                while (x < kFieldSectionSizeX) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                            } else {
                                int x = 0;
                                do {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    --y;
                                    field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                                    ++x;
                                } while (y > field->os[routey][routex].y);
                                while (x < kFieldSectionSizeX) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                            }
                        } else {
                            for (int x = 0; x < kFieldSectionSizeX; x++) {
                                field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                            }
                        }
                    }
                    if ((route & kMazeRouteUp) != 0) {
                        int x = field->os[routey - 1][routex].x;
                        for (int y = 0; y < field->os[routey][routex].y; y++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        }
                    }
                    if ((route & kMazeRouteDown) != 0) {
                        int x = field->os[routey][routex].x;
                        for (int y = field->os[routey][routex].y; y < kFieldSectionSizeY; y++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        }
                    }
                }
            }
        }
    }

    // ロックした場所を開ける
    {
        // 開始位置を開ける
        FieldDigLocation(kFieldLocationStart);

        // 洞窟を開ける
        for (int i = 0; i < kFieldLocationCaveSize; i++) {
            FieldDigLocation(kFieldLocationCave + i);
        }

        // 城を開ける
        FieldDigLocation(kFieldLocationCastle);

        // 店を開ける
        for (int i = 0; i < kFieldLocationShopSize; i++) {
            FieldDigLocation(kFieldLocationShop + i);
        }
    }

    // 建物などを置く
    {
        // 開始位置を置く

        // 洞窟の入り口を置く
        for (int i = 0; i < kFieldLocationCaveSize; i++) {
            int location = kFieldLocationCave + i;
            int x = ((field->locations[location].right - field->locations[location].left + 1) - kFieldCaveSizeX) / 2 + field->locations[location].left;
            int y = field->locations[location].bottom - kFieldCaveSizeY;
            for (int h = 0; h < kFieldCaveSizeY; h++) {
                for (int w = 0; w < kFieldCaveSizeX; w++) {
                    field->maps[y + h][x + w] = kFieldMapCave00 + h * kFieldCaveSizeX + w;
                }
            }
        }

        // 城を置く
        {
            int x = ((field->locations[kFieldLocationCastle].right - field->locations[kFieldLocationCastle].left + 1) - kFieldCastleSizeX) / 2 + field->locations[kFieldLocationCastle].left;
            int y = field->locations[kFieldLocationCastle].bottom - kFieldCastleSizeY;
            for (int h = 0; h < kFieldCastleSizeY; h++) {
                for (int w = 0; w < kFieldCastleSizeX; w++) {
                    field->maps[y + h][x + w] = kFieldMapCastle00 + h * kFieldCastleSizeX + w;
                }
            }
        }

        // 店を置く
        for (int i = 0; i < kFieldLocationShopSize; i++) {
            int location = kFieldLocationShop + i;
            int x = ((field->locations[location].right - field->locations[location].left + 1) - kFieldShopSizeX) / 2 + field->locations[location].left;
            int y = field->locations[location].bottom - kFieldShopSizeY;
            for (int h = 0; h < kFieldShopSizeY; h++) {
                for (int w = 0; w < kFieldShopSizeX; w++) {
                    field->maps[y + h][x + w] = kFieldMapShop00 + h * kFieldShopSizeX + w;
                }
            }
        }
    }

    // 柱を立てる
    for (int y = 1; y < kFieldSizeY - 1; y++) {
        for (int x = 1; x < kFieldSizeX - 1; x++) {
            if (
                FieldIsMapBlock(field->maps[y - 1][x]) && 
                FieldIsMapBack(field->maps[y + 0][x]) && 
                FieldIsMapBack(field->maps[y + 1][x]) 
            ) {
                if (
                    (FieldIsMapBack(field->maps[y - 1][x - 1]) && FieldIsMapBlock(field->maps[y - 1][x + 1])) || 
                    (FieldIsMapBack(field->maps[y - 1][x + 1]) && FieldIsMapBlock(field->maps[y - 1][x - 1]))
                ) {
                    int y_0 = y + 1;
                    while (y_0 < kFieldSizeY && FieldIsMapBack(field->maps[y_0][x])) {
                        ++y_0;
                    }
                    if (y_0 >= kFieldSizeY || FieldIsMapBlock(field->maps[y_0][x])) {
                        for (int y_1 = y; y_1 < y_0; y_1++) {
                            field->maps[y_1][x] = kFieldMapPole;
                        }
                    }
                }
            }
        }
    }

    // つららを立てる
    for (int y = 0; y < kFieldSizeY - 1; y++) {
        for (int x = 0; x < kFieldSizeX; x++) {
            if (
                (y == 0 || FieldIsMapBlock(field->maps[y - 1][x])) && 
                (FieldIsMapLock(field->maps[y + 0][x + 0]) || FieldIsMapBack(field->maps[y + 0][x + 0])) && 
                (FieldIsMapLock(field->maps[y + 1][x + 0]) || FieldIsMapBack(field->maps[y + 1][x + 0])) && 
                !FieldIsMapBlock(field->maps[y + 1][x - 1]) && 
                !FieldIsMapBlock(field->maps[y + 1][x + 1])
            ) {
                field->maps[y][x] = kFieldMapIcicle;
            }
        }
    }
}

// マップを解放する
//
static void FieldUnbuildMap(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 迷路の解放
    if (field != NULL) {
        MazeUnload(field->maze);
    }
}

// 指定した配置をロックする
//
static void FieldLockLocation(int location)
{
    int left = field->locations[location].left / kFieldSectionSizeX;
    int top = field->locations[location].top / kFieldSectionSizeY;
    int right = field->locations[location].right / kFieldSectionSizeX;
    int bottom = field->locations[location].bottom / kFieldSectionSizeY;
    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            MazeLock(field->maze, x, y);
        }
    }
}

// 指定した配置に空間を作る
//
static void FieldDigLocation(int location)
{
    // 空間の作成
    for (int y = field->locations[location].top; y < field->locations[location].bottom; y++) {
        for (int x = field->locations[location].left; x <= field->locations[location].right; x++) {
            field->maps[y][x] = kFieldMapLock;
        }
    }
    for (int x = field->locations[location].left; x <= field->locations[location].right; x++) {
        field->maps[field->locations[location].bottom][x] = kFieldMapBlock;
    }

    // 左側をつなげる
    if (field->locations[location].left > 0) {
        int x = field->locations[location].left - 1;
        int y = field->locations[location].bottom;
        while (!FieldIsMapBlock(field->maps[y][x]) && !FieldIsMapLadder(field->maps[y][x])) {
            --y;
            if (y < field->locations[location].top) {
                y = field->locations[location].bottom;
                --x;
                if (x < 0) {
                    break;
                }
            }
        }
        ++x;
        while (x < field->locations[location].left) {
            field->maps[y][x] = kFieldMapBlock;
            ++x;
        }
        while (y < field->locations[location].bottom) {
            field->maps[y][x] = kFieldMapLadder;
            ++y;
        }
    }

    // 右側をつなげる
    if (field->locations[location].right < kFieldSizeX - 1) {
        int x = field->locations[location].right + 1;
        int y = field->locations[location].bottom;
        while (!FieldIsMapBlock(field->maps[y][x]) && !FieldIsMapLadder(field->maps[y][x])) {
            --y;
            if (y < field->locations[location].top) {
                y = field->locations[location].bottom;
                ++x;
                if (x >= kFieldSizeX) {
                    break;
                }
            }
        }
        --x;
        while (x > field->locations[location].right) {
            field->maps[y][x] = kFieldMapBlock;
            --x;
        }
        while (y < field->locations[location].bottom) {
            field->maps[y][x] = kFieldMapLadder;
            ++y;
        }
    }
}

// フィールドアクタを読み込む
//
void FieldActorLoad(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct FieldActor *actor = (struct FieldActor *)ActorLoad((ActorFunction)FieldActorLoop, kGamePriorityField);
    if (actor == NULL) {
        playdate->system->error("%s: %d: field actor is not loaded.", __FILE__, __LINE__);
    }

    // フィールドの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)FieldActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagField);

        // スプライトの作成
        actor->animations = (struct AsepriteSpriteAnimation *)playdate->system->realloc(NULL, kFieldAnimationSize * sizeof (struct AsepriteSpriteAnimation));
        if (actor->animations == NULL) {
            playdate->system->error("%s: %d: field actor animation is not created.", __FILE__, __LINE__);
        }

    }
}

// フィールドアクタを解放する
//
static void FieldActorUnload(struct FieldActor *actor)
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

// フィールドアクタを描画する
//
static void FieldActorDraw(struct FieldActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // カメラの取得
    struct Vector *camera = GameGetCamera();

    // クリップの設定
    FieldSetClip();

    // スプライトの描画
    {
        int viewx = camera->x >= 0 ? -(camera->x % kFieldSizePixel) : -kFieldSizePixel - (camera->x % kFieldSizePixel);
        int viewy = camera->y >= 0 ? -(camera->y % kFieldSizePixel) : -kFieldSizePixel - (camera->y % kFieldSizePixel);
        int mapx = camera->x >= 0 ? camera->x / kFieldSizePixel : camera->x / kFieldSizePixel - 1;
        int mapy = camera->y >= 0 ? camera->y / kFieldSizePixel : camera->y / kFieldSizePixel - 1;
        int my = mapy;
        for (int vy = viewy; vy < kGameViewFieldSizeY; vy += kFieldSizePixel) {
            int mx = mapx;
            for (int vx = viewx; vx < kGameViewFieldSizeX; vx += kFieldSizePixel) {
                int animation = kFieldAnimationBlock;
                if (my >= 0 && my < kFieldSizeY) {
                    int ax = mx < 0 ? mx + kFieldSizeX : (mx >= kFieldSizeX ? mx - kFieldSizeX : mx);
                    animation = field->maps[my][ax];
                }
                AsepriteDrawSpriteAnimation(&actor->animations[animation], vx + kGameViewFieldLeft, vy + kGameViewFieldTop, kDrawModeCopy, kBitmapUnflipped);
                ++mx;
            }
            ++my;
        }
    }

    // クリップの解除
    FieldClearClip();

    // DEBUG
    playdate->system->drawFPS(0, 0);
}

// フィールドアクタが待機する
//
static void FieldActorLoop(struct FieldActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // アニメーションの開始
        for (int i = 0; i < kFieldAnimationSize; i++) {
            AsepriteStartSpriteAnimation(&actor->animations[i], "tileset", fieldAnimationNames[i], false);
        }

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの更新
        for (int i = 0; i < kFieldAnimationSize; i++) {
            AsepriteUpdateSpriteAnimation(&actor->animations[i]);
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)FieldActorDraw, kGameOrderField);
}

// マップを取得する
//
unsigned char FieldGetMap(int x, int y)
{
    unsigned char result = kFieldMapBlock;
    if (field != NULL) {
        if (y >= 0 && y < kFieldSizeY * kFieldSizePixel) {
            x = FieldGetMapX(x); // (x < 0 ? x + kFieldSizeX * kFieldSizePixel : (x >= kFieldSizeX * kFieldSizePixel ? x - kFieldSizeX * kFieldSizePixel : x)) / kFieldSizePixel;
            y = FieldGetMapY(y); // y / kFieldSizePixel;
            result = field->maps[y][x];
        }
    }
    return result;
}
static int FieldGetMapX(int x)
{
    return (x < 0 ? x + kFieldSizeX * kFieldSizePixel : (x >= kFieldSizeX * kFieldSizePixel ? x - kFieldSizeX * kFieldSizePixel : x)) / kFieldSizePixel;
}
static int FieldGetMapY(int y)
{
    return y / kFieldSizePixel;
}

// フィールドマップを判定する
//
static bool FieldIsMapBack(unsigned char map)
{
    return map == kFieldMapBack ? true : false;
}
static bool FieldIsMapBlock(unsigned char map)
{
    return map == kFieldMapBlock|| map == kFieldMapSolid || map == kFieldMapChecker ? true : false;
}
static bool FieldIsMapLadder(unsigned char map)
{
    return map == kFieldMapLadder ? true : false;
}
static bool FieldIsMapLock(unsigned char map)
{
    return map == kFieldMapLock ? true : false;
}

// フィールドが空いているかどうかを判定する
//
bool FieldIsSpace(int x, int y)
{
    unsigned char m = FieldGetMap(x, y);
    return !FieldIsMapBlock(m) && m != kFieldMapIcicle ? true : false;
}

// フィールドが梯子かどうかを判定する
//
bool FieldIsLadder(int x, int y)
{
    return FieldIsMapLadder(FieldGetMap(x, y));
}

// フィールドが洞窟かどうかを判定する
//
bool FieldIsCave(int x, int y)
{
    return FieldGetMap(x, y) == kFieldMapCaveEntrance ? true : false;
}

// フィールドが城かどうかを判定する
//
bool FieldIsCastle(int x, int y)
{
    return FieldGetMap(x, y) == kFieldMapCastleEntrance ? true : false;
}

// フィールドが店かどうかを判定する
//
bool FieldIsShop(int x, int y)
{
    return FieldGetMap(x, y) == kFieldMapShopEntrance ? true : false;
}

// フィールドで落下するかどうかを判定する
//
bool FieldIsLand(int x, int y)
{
    return !FieldIsSpace(x, y + kFieldSizePixel) || FieldIsLadder(x, y) || FieldIsLadder(x, y + kFieldSizePixel) ? true : false;
}
bool FieldIsFall(int x, int y)
{
    return !FieldIsLand(x, y);
}

// フィールドを指定した方向に移動する
//
bool FieldIsWalk(int x, int y, int direction, bool jump, bool fall)
{
    return FieldWalk(x, y, direction, jump, fall, NULL);
}
bool FieldWalk(int x, int y, int direction, bool jump, bool fall, struct Vector *to)
{
    bool result = false;
    if (direction == kDirectionUp) {
        if (
            FieldIsSpace(x, y - kFieldSizePixel) && 
            (
                jump || 
                FieldIsLadder(x, y)
            )
        ) {
            if (to != NULL) {
                to->x = x;
                to->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpLeft) {
        if (
            FieldIsSpace(x - kFieldSizePixel, y) &&
            FieldIsSpace(x, y - kFieldSizePixel) && 
            (
                (
                    jump && 
                    FieldIsSpace(x - kFieldSizePixel, y - kFieldSizePixel) 
                ) || 
                (
                    FieldIsLadder(x, y) &&
                    FieldIsLadder(x - kFieldSizePixel, y - kFieldSizePixel) 
                )
            )
        ) {
            if (to != NULL) {
                to->x = x - kFieldSizePixel;
                to->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpRight) {
        if (
            FieldIsSpace(x + kFieldSizePixel, y) && 
            FieldIsSpace(x, y - kFieldSizePixel) && 
            (
                (
                    jump && 
                    FieldIsSpace(x + kFieldSizePixel, y - kFieldSizePixel)
                ) || 
                (
                    FieldIsLadder(x, y) && 
                    FieldIsLadder(x + kFieldSizePixel, y - kFieldSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x + kFieldSizePixel;
                to->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDown) {
        if (
            (
                fall && 
                FieldIsSpace(x, y + kFieldSizePixel)
            ) || 
            FieldIsLadder(x, y + kFieldSizePixel)
        ) {
            if (to != NULL) {
                to->x = x;
                to->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownLeft) {
        if (
            FieldIsSpace(x - kFieldSizePixel, y) && 
            FieldIsSpace(x, y + kFieldSizePixel) && 
            (
                (
                    fall &&
                    FieldIsSpace(x - kFieldSizePixel, y + kFieldSizePixel)
                ) || 
                (
                    FieldIsLadder(x, y) && 
                    FieldIsLadder(x - kFieldSizePixel, y + kFieldSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x - kFieldSizePixel;
                to->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownRight) {
        if (
            FieldIsSpace(x + kFieldSizePixel, y) && 
            FieldIsSpace(x, y + kFieldSizePixel) && 
            (
                (
                    fall && 
                    FieldIsSpace(x + kFieldSizePixel, y + kFieldSizePixel)
                ) || 
                (
                    FieldIsLadder(x, y) && 
                    FieldIsLadder(x + kFieldSizePixel, y + kFieldSizePixel)
                )
            )
        ) {
            if (to != NULL) {
                to->x = x + kFieldSizePixel;
                to->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionLeft) {
        if (
            FieldIsSpace(x - kFieldSizePixel, y) && 
            (
                fall || 
                FieldIsLand(x - kFieldSizePixel, y)
            )
        ) {
            if (to != NULL) {
                to->x = x - kFieldSizePixel;
                to->y = y;
            }
            result = true;
        }
    } else if (direction == kDirectionRight) {
        if (
            FieldIsSpace(x + kFieldSizePixel, y) && 
            (
                fall || 
                FieldIsLand(x + kFieldSizePixel, y)
            )
        ) {
            if (to != NULL) {
                to->x = x + kFieldSizePixel;
                to->y = y;
            }
            result = true;
        }
    }
    return result;
}

// 移動する位置を調整する
//
void FieldAdjustMovePosition(struct Vector *from, struct Vector *to)
{
    if (to->x < 0) {
        to->x += kFieldSizeX * kFieldSizePixel;
        from->x += kFieldSizeX * kFieldSizePixel;
    } else if (to->x >= kFieldSizeX * kFieldSizePixel) {
        to->x -= kFieldSizeX * kFieldSizePixel;
        from->x -= kFieldSizeX * kFieldSizePixel;
    }
}

// 開始位置を取得する
//
void FieldGetStartPosition(struct Vector *position)
{
    int x = (field->locations[kFieldLocationStart].left + field->locations[kFieldLocationStart].right + 1) / 2;
    int y = field->locations[kFieldLocationStart].top;
    while (!FieldIsSpace(x * kFieldSizePixel, y * kFieldSizePixel)) {
        ++y;
    }
    while (FieldIsSpace(x * kFieldSizePixel, y * kFieldSizePixel)) {
        ++y;
    }
    position->x = x * kFieldSizePixel + kFieldSizePixel / 2;
    position->y = y * kFieldSizePixel - 1;
}

// エネミーの配置位置を取得する
//
void FieldGetEnemyPosition(struct Vector *position, bool land)
{
    int x = field->locations[field->locationEnemy].left + (IocsGetRandomNumber(&field->xorshift) % (field->locations[field->locationEnemy].right - field->locations[field->locationEnemy].left + 1));
    int y = field->locations[field->locationEnemy].top + (IocsGetRandomNumber(&field->xorshift) % (field->locations[field->locationEnemy].bottom - field->locations[field->locationEnemy].top + 1));
    while (!FieldIsSpace(x * kFieldSizePixel, y * kFieldSizePixel)) {
        ++y;
        if (y > kFieldSizeY) {
            y = 0;
        }
    }
    int top = y;
    while (FieldIsSpace(x * kFieldSizePixel, y * kFieldSizePixel)) {
        ++y;
    }
    position->x = x * kFieldSizePixel + kFieldSizePixel / 2;
    if (land) {
        position->y = y * kFieldSizePixel - 1;
    } else {
        position->y = ((y + top) / 2) * kFieldSizePixel - 1;
    }
    ++field->locationEnemy;
    if (field->locationEnemy >= kFieldLocationSize) {
        field->locationEnemy = kFieldLocationEnemy;
    }
}

// 指定された方向の位置を取得する
//
void FieldGetDirectinalPosition(int x, int y, int direction, struct Vector *position)
{
    if (direction == kDirectionUp) {
        position->x = x;
        position->y = y - kFieldSizePixel;
    } else if (direction == kDirectionDown) {
        position->x = x;
        position->y = y + kFieldSizePixel;
    } else if (direction == kDirectionLeft) {
        position->x = x - kFieldSizePixel;
        position->y = y;
    } else if (direction == kDirectionRight) {
        position->x = x + kFieldSizePixel;
        position->y = y;
    }
}

// バトルの経路を取得する
//
int FieldGetBattleRoute(int x, int y)
{
    int route = 0;
    if (FieldIsSpace(x, y - kFieldSizePixel)) {
        route |= (1 << kDirectionUp);
    }
    if (FieldIsSpace(x, y + kFieldSizePixel)) {
        route |= (1 << kDirectionDown);
    }
    if (FieldIsSpace(x - kFieldSizePixel, y)) {
        route |= (1 << kDirectionLeft);
    }
    if (FieldIsSpace(x + kFieldSizePixel, y)) {
        route |= (1 << kDirectionRight);
    }
    return route;
}

// 洞窟のインデックスを取得する
//
int FieldGetCaveIndex(int x, int y)
{
    int index = -1;
    if (FieldIsCave(x, y)) {
        x = FieldGetMapX(x);
        y = FieldGetMapY(y);
        for (int i = 0; i < kFieldLocationCaveSize; i++) {
            if (
                x >= field->locations[kFieldLocationCave + i].left && 
                x <= field->locations[kFieldLocationCave + i].right && 
                y >= field->locations[kFieldLocationCave + i].top && 
                y <= field->locations[kFieldLocationCave + i].bottom
            ) {
                index = i;
                break;
            }
        }
    }
    return index;
}

// 店のインデックスを取得する
//
int FieldGetShopIndex(int x, int y)
{
    int index = -1;
    if (FieldIsShop(x, y)) {
        x = FieldGetMapX(x);
        y = FieldGetMapY(y);
        for (int i = 0; i < kFieldLocationShopSize; i++) {
            if (
                x >= field->locations[kFieldLocationShop + i].left && 
                x <= field->locations[kFieldLocationShop + i].right && 
                y >= field->locations[kFieldLocationShop + i].top && 
                y <= field->locations[kFieldLocationShop + i].bottom
            ) {
                index = i;
                break;
            }
        }
    }
    return index;
}

// クリップを設定する
//
void FieldClearClip(void)
{
    IocsGetPlaydate()->graphics->clearClipRect();
}
void FieldSetClip(void)
{
    IocsGetPlaydate()->graphics->setClipRect(kGameViewFieldLeft, kGameViewFieldTop, kGameViewFieldSizeX, kGameViewFieldSizeY);
}


