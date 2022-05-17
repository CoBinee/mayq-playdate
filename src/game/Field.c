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
static void FieldLoadMap(int seed);
static void FieldUnloadMap(void);
static void FieldActorUnload(struct FieldActor *actor);
static void FieldActorDraw(struct FieldActor *actor);
static void FieldActorLoop(struct FieldActor *actor);

// 内部変数
//
static struct Field *field = NULL;
static const char *fieldAnimationNames[] = {
    "Null", 
    "Wall", 
    "Block", 
    "Ladder", 
    "Icicle", 
    "Pole", 
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

    // マップの作成
    FieldLoadMap(123456789);
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
    FieldUnloadMap();

    // フィールドの解放
    if (field != NULL) {        
        playdate->system->realloc(field, 0);
        field = NULL;
    }
}

// マップを作成する
//
static void FieldLoadMap(int seed)
{
    // 迷路の作成
    {
        // 初期化
        field->maze = MazeLoad(kFieldMazeSizeX, kFieldMazeSizeY, seed);

        // 穴を掘る
        MazeDig(field->maze, 1, 1);

        // 経路の設定
        MazeSetRoute(field->maze);

        // 中心の設定
        for (int routey = 0; routey < field->maze->routeSize.y; routey++) {
            for (int routex = 0; routex < field->maze->routeSize.x; routex++) {
                field->os[routey][routex].x = IocsGetRandom(&field->maze->xorshift) % (kFieldMazeSectionSizeX - 1) + 1;
                field->os[routey][routex].y = IocsGetRandom(&field->maze->xorshift) % (kFieldMazeSectionSizeY - 1) + 1;
            }
        }
    }

    // マップの初期化
    {
        for (int mapy = 0; mapy < kFieldSizeY; mapy++) {
            for (int mapx = 0; mapx < kFieldSizeX; mapx++) {
                field->maps[mapy][mapx] = kFieldMapWall;
            }
        }
    }

    // マップの作成
    {
        for (int routey = 0; routey < field->maze->routeSize.y; routey++) {
            int mapy = routey * kFieldMazeSectionSizeY;
            for (int routex = 0; routex < field->maze->routeSize.x; routex++) {
                int mapx = routex * kFieldMazeSectionSizeX;
                unsigned char route = field->maze->routes[routey * field->maze->routeSize.x + routex];
                if (route == (kMazeRouteUp | kMazeRouteDown)) {
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
                    while (y < kFieldMazeSectionSizeY) {
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
                        for (int x = 0; x < kFieldMazeSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                    }
                } else if (route == kMazeRouteDown) {
                    {
                        int y = field->os[routey][routex].y;
                        for (int x = 0; x < kFieldMazeSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                    }
                    {
                        int x = field->os[routey][routex].x;
                        for (int y = field->os[routey][routex].y; y < kFieldMazeSectionSizeY; y++) {
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
                            while (y < kFieldMazeSectionSizeY) {
                                field->maps[mapy + y][mapx + t] = kFieldMapLadder;
                                ++y;
                            }
                        }
                    } else if (lr == kMazeRouteRight) {
                        int y = field->os[routey][routex].y;
                        int t = routey > 0 && field->os[routey - 1][routex].x < field->os[routey][routex].x ? field->os[routey - 1][routex].x : field->os[routey][routex].x;
                        for (int x = t; x < kFieldMazeSectionSizeX; x++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                        }
                        if (routey >= field->maze->routeSize.y - 1) {
                            while (y < kFieldMazeSectionSizeY) {
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
                                while (x < kFieldMazeSectionSizeX) {
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
                                while (x < kFieldMazeSectionSizeX) {
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
                                while (x < kFieldMazeSectionSizeX) {
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
                                while (x < kFieldMazeSectionSizeX) {
                                    field->maps[mapy + y][mapx + x] = kFieldMapBlock;
                                    ++x;
                                }
                            }
                        } else {
                            for (int x = 0; x < kFieldMazeSectionSizeX; x++) {
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
                        for (int y = field->os[routey][routex].y; y < kFieldMazeSectionSizeY; y++) {
                            field->maps[mapy + y][mapx + x] = kFieldMapLadder;
                        }
                    }
                }
            }
        }
    }

    // 柱を立てる
    for (int y = 1; y < kFieldSizeY - 1; y++) {
        for (int x = 1; x < kFieldSizeX - 1; x++) {
            if (
                field->maps[y - 1][x] == kFieldMapBlock && 
                field->maps[y + 0][x] == kFieldMapWall && 
                field->maps[y + 1][x] == kFieldMapWall
            ) {
                if (
                    (field->maps[y - 1][x - 1] == kFieldMapWall && field->maps[y - 1][x + 1] == kFieldMapBlock) || 
                    (field->maps[y - 1][x + 1] == kFieldMapWall && field->maps[y - 1][x - 1] == kFieldMapBlock)
                ) {
                    int y_0 = y + 1;
                    while (y_0 < kFieldSizeY && field->maps[y_0][x] == kFieldMapWall) {
                        ++y_0;
                    }
                    if (y_0 >= kFieldSizeY || field->maps[y_0][x] == kFieldMapBlock) {
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
                (y == 0 || field->maps[y - 1][x] == kFieldMapBlock) && 
                field->maps[y + 0][x + 0] == kFieldMapWall && 
                field->maps[y + 1][x + 0] == kFieldMapWall && 
                field->maps[y + 1][x - 1] != kFieldMapBlock && 
                field->maps[y + 1][x + 1] != kFieldMapBlock
            ) {
                field->maps[y][x] = kFieldMapIcicle;
            }
        }
    }
}

// マップを解放する
//
static void FieldUnloadMap(void)
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

    // スプライトの描画
    {
        int viewx = camera->x >= 0 ? -(camera->x % kFieldSizePixel) : -kFieldSizePixel - (camera->x % kFieldSizePixel);
        int viewy = camera->y >= 0 ? -(camera->y % kFieldSizePixel) : -kFieldSizePixel - (camera->y % kFieldSizePixel);
        int mazex = camera->x >= 0 ? camera->x / kFieldSizePixel : camera->x / kFieldSizePixel - 1;
        int mazey = camera->y >= 0 ? camera->y / kFieldSizePixel : camera->y / kFieldSizePixel - 1;
        int my = mazey;
        playdate->graphics->setClipRect(kFieldViewLeft, kFieldViewTop, kFieldViewSizeX, kFieldViewSizeY);
        for (int vy = viewy; vy < kFieldViewSizeY; vy += kFieldSizePixel) {
            int mx = mazex;
            for (int vx = viewx; vx < kFieldViewSizeX; vx += kFieldSizePixel) {
                int animation = kFieldAnimationBlock;
                if (my >= 0 && my < kFieldSizeY) {
                    int ax = mx < 0 ? mx + kFieldSizeX : (mx >= kFieldSizeX ? mx - kFieldSizeX : mx);
                    animation = field->maps[my][ax];
                }
                AsepriteDrawSpriteAnimation(&actor->animations[animation], vx + kFieldViewLeft, vy + kFieldViewTop, kDrawModeCopy, kBitmapUnflipped);
                ++mx;
            }
            ++my;
        }
        playdate->graphics->clearClipRect();
    }

    // 
    {
        playdate->graphics->setDrawMode(kDrawModeCopy);
        for (int y = 0; y < field->maze->mapSize.y; y++) {
            for (int x = 0; x < field->maze->mapSize.x; x++) {
                if (field->maze->maps[y * field->maze->mapSize.x + x] != 0) {
                    playdate->graphics->fillRect(x * 4 + 248, y * 4 + 8, 4, 4, kColorWhite);
                }
            }
        }
    }
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
            AsepriteStartSpriteAnimation(&actor->animations[i], "tile", fieldAnimationNames[i], false);
        }

        // 初期化の完了
        ++actor->actor.state;
    }

    // スプライトの更新
    for (int i = 0; i < kFieldAnimationSize; i++) {
        AsepriteUpdateSpriteAnimation(&actor->animations[i]);
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
            x = (x < 0 ? x + kFieldSizeX * kFieldSizePixel : (x >= kFieldSizeX * kFieldSizePixel ? x - kFieldSizeX * kFieldSizePixel : x)) / kFieldSizePixel;
            y = y / kFieldSizePixel;
            result = field->maps[y][x];
        }
    }
    return result;
}

// フィールドが空いているかどうかを判定する
//
bool FieldIsSpace(int x, int y)
{
    unsigned char m = FieldGetMap(x, y);
    return m != kFieldMapBlock && m != kFieldMapIcicle ? true : false;
}

// フィールドを登れるかどうかを判定する
//
bool FieldIsClimb(int x, int y)
{
    unsigned char m = FieldGetMap(x, y);
    return m == kFieldMapLadder ? true : false;
}

// フィールドで落下するかどうかを判定する
//
bool FieldIsLand(int x, int y)
{
    return !FieldIsSpace(x, y + kFieldSizePixel) || FieldIsClimb(x, y) || FieldIsClimb(x, y + kFieldSizePixel) ? true : false;
}
bool FieldIsFall(int x, int y)
{
    return !FieldIsLand(x, y);
}

// フィールドを指定した方向に移動できるかどうかを判定する
//
bool FieldIsWalk(int x, int y, int direction, bool land, struct Vector *move)
{
    bool result = false;
    if (direction == kDirectionUp) {
        if (
            // (step || FieldIsLand(x, y)) && 
            land && 
            FieldIsSpace(x, y - kFieldSizePixel)
        ) {
            if (move != NULL) {
                move->x = x;
                move->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpLeft) {
        if (
            // (step || FieldIsLand(x, y)) && 
            land && 
            FieldIsSpace(x - kFieldSizePixel, y - kFieldSizePixel) && 
            FieldIsSpace(x - kFieldSizePixel, y) &&
            FieldIsSpace(x, y - kFieldSizePixel)
        ) {
            if (move != NULL) {
                move->x = x - kFieldSizePixel;
                move->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionUpRight) {
        if (
            // (step || FieldIsLand(x, y)) && 
            land && 
            FieldIsSpace(x + kFieldSizePixel, y - kFieldSizePixel) && 
            FieldIsSpace(x + kFieldSizePixel, y) && 
            FieldIsSpace(x, y - kFieldSizePixel)
        ) {
            if (move != NULL) {
                move->x = x + kFieldSizePixel;
                move->y = y - kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDown) {
        if (FieldIsSpace(x, y + kFieldSizePixel)) {
            if (move != NULL) {
                move->x = x;
                move->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownLeft) {
        if (
            FieldIsSpace(x - kFieldSizePixel, y + kFieldSizePixel) && 
            FieldIsSpace(x - kFieldSizePixel, y) && 
            FieldIsSpace(x, y + kFieldSizePixel)
        ) {
            if (move != NULL) {
                move->x = x - kFieldSizePixel;
                move->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionDownRight) {
        if (
            FieldIsSpace(x + kFieldSizePixel, y + kFieldSizePixel) && 
            FieldIsSpace(x + kFieldSizePixel, y) && 
            FieldIsSpace(x, y + kFieldSizePixel)
        ) {
            if (move != NULL) {
                move->x = x + kFieldSizePixel;
                move->y = y + kFieldSizePixel;
            }
            result = true;
        }
    } else if (direction == kDirectionLeft) {
        if (FieldIsSpace(x - kFieldSizePixel, y)) {
            if (move != NULL) {
                move->x = x - kFieldSizePixel;
                move->y = y;
            }
            result = true;
        }
    } else if (direction == kDirectionRight) {
        if (FieldIsSpace(x + kFieldSizePixel, y)) {
            if (move != NULL) {
                move->x = x + kFieldSizePixel;
                move->y = y;
            }
            result = true;
        }
    }
    return result;
}
