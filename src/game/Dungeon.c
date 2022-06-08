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
        IocsSetRandomSeed(&dungeon->xorshift, 123456789);

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
        dungeon->locations[i].x = (i % kDungeonLocationSizeX) * kDungeonLocationAreaSizeX + IocsGetRandomNumber(&dungeon->xorshift) % (kDungeonLocationAreaSizeX - 1);
        dungeon->locations[i].y = (i / kDungeonLocationSizeX) * kDungeonLocationAreaSizeY + IocsGetRandomNumber(&dungeon->xorshift) % (kDungeonLocationAreaSizeY - 1);
    }
    /*
    for (int i = 0; i < kDungeonLocationSize; i++) {
        int j = IocsGetRandomNumber(&dungeon->xorshift) % kDungeonLocationSize;
        struct Vector r = dungeon->locations[j];
        dungeon->locations[j] = dungeon->locations[i];
        dungeon->locations[i] = r;
    }
    */
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
            // 入り口をロック
            for (int i = 0; i < kDungeonLocationEntranceSize; i++) {
                DungeonLockLocation(kDungeonLocationEntrance + i);
            }

            // ボスをロック
            for (int i = 0; i < kDungeonLocationBossSize; i++) {
                DungeonLockLocation(kDungeonLocationBoss + i);
            }
        }

        // 穴を掘る
        {
            int x = dungeon->locations[kDungeonLocationDig].x * 2 + 1;
            int y = dungeon->locations[kDungeonLocationDig].y * 2 + 1;
            MazeDig(dungeon->maze, x, y);
        }

        // 経路の設定
        MazeSetRoute(dungeon->maze);

        // 行き止まりの解消
        MazeSolveDeadend(dungeon->maze);

        // 入り口を開ける
        for (int i = 0; i < kDungeonLocationEntranceSize; i++) {
            dungeon->maze->routes[dungeon->locations[kDungeonLocationEntrance + i].y * dungeon->maze->routeSize.x + dungeon->locations[kDungeonLocationEntrance + i].x] |= (kMazeRouteUp | kMazeRouteDown);
            {
                struct Vector position;
                DungeonGetDirectionalPosition(dungeon->locations[kDungeonLocationEntrance + i].x, dungeon->locations[kDungeonLocationEntrance + i].y, kDirectionUp, &position);
                dungeon->maze->routes[position.y * dungeon->maze->routeSize.x + position.x] |= kMazeRouteDown;
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
    MazeLock(dungeon->maze, dungeon->locations[location].x, dungeon->locations[location].y);
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

    // DEBUG
    {
        playdate->graphics->setDrawMode(kDrawModeCopy);
        for (int ry = 0; ry < dungeon->maze->routeSize.y; ry++) {
            for (int rx = 0; rx < dungeon->maze->routeSize.x; rx++) {
                unsigned char r = dungeon->maze->routes[ry * dungeon->maze->routeSize.x + rx];
                int dx = rx * 14;
                int dy = ry * 14;
                if (r == 0) {
                    playdate->graphics->fillRect(dx +  0, dy +  0, 14, 14, kColorWhite);
                } else {
                    playdate->graphics->fillRect(dx +  0, dy +  0, 2, 2, kColorWhite);
                    playdate->graphics->fillRect(dx + 12, dy +  0, 2, 2, kColorWhite);
                    playdate->graphics->fillRect(dx +  0, dy + 12, 2, 2, kColorWhite);
                    playdate->graphics->fillRect(dx + 12, dy + 12, 2, 2, kColorWhite);
                    if ((r & kMazeRouteUp) == 0) {
                        playdate->graphics->fillRect(dx +  2, dy +  0, 10,  2, kColorWhite);
                    }
                    if ((r & kMazeRouteDown) == 0) {
                        playdate->graphics->fillRect(dx +  2, dy + 12, 10,  2, kColorWhite);
                    }
                    if ((r & kMazeRouteLeft) == 0) {
                        playdate->graphics->fillRect(dx +  0, dy +  2,  2, 10, kColorWhite);
                    }
                    if ((r & kMazeRouteRight) == 0) {
                        playdate->graphics->fillRect(dx + 12, dy +  2,  2, 10, kColorWhite);
                    }
                }
            }
        }
    }
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

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {
        ;
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)DungeonActorDraw, kGameOrderDungeon);
}

// ダンジョンの経路を取得する
//
unsigned char DungeonGetRoute(int x, int y)
{
    return dungeon->maze->routes[y * dungeon->maze->routeSize.x + x];
}

// 指定された方向の位置を取得する
//
void DungeonGetDirectionalPosition(int x, int y, int direction, struct Vector *position)
{
    if (direction == kDirectionUp) {
        position->x = x;
        position->y = y > 0 ? y - 1 : kDungeonSizeY - 1;
    } else if (direction == kDirectionDown) {
        position->x = x;
        position->y = y < kDungeonSizeY - 1 ? y + 1 : 0;
    } else if (direction == kDirectionLeft) {
        position->x = x > 0 ? x - 1 : kDungeonSizeX - 1;
        position->y = y;
    } else if (direction == kDirectionRight) {
        position->x = x < kDungeonSizeX - 1 ? x + 1 : 0;
        position->y = y;
    }
}

// 入り口の位置を取得する
//
void DungeonGetEntrancePosition(int entrance, struct Vector *position)
{
    *position = dungeon->locations[kDungeonLocationEntrance + entrance];
}
