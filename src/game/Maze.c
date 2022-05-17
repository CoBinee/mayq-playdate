// Maze.c - 迷路
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Maze.h"

// 内部関数
//

// 内部変数
//


// 迷路を初期化する
//
struct Maze *MazeLoad(int sizex, int sizey, int seed)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return NULL;
    }

    // 迷路の作成
    struct Maze *maze = (struct Maze *)playdate->system->realloc(NULL, sizeof (struct Maze));
    if (maze == NULL) {
        playdate->system->error("%s: %d: maze is not created.", __FILE__, __LINE__);
    }

    // 乱数の初期化
    IocsSetRandomSeed(&maze->xorshift, seed);

    // マップの作成
    maze->mapSize.x = sizex * 2 + 1;
    maze->mapSize.y = sizey * 2 + 1;
    maze->maps = (unsigned char *)playdate->system->realloc(NULL, maze->mapSize.x * maze->mapSize.y * sizeof (unsigned char));
    if (maze->maps == NULL) {
        playdate->system->error("%s: %d: maze map is not created.", __FILE__, __LINE__);
    }

    // マップの初期化
    for (int y = 0; y < maze->mapSize.y; y++) {
        for (int x = 0; x < maze->mapSize.x; x++) {
            maze->maps[y * maze->mapSize.x + x] = kMazeMapBlock;
        }
    }

    // 経路の作成
    maze->routeSize.x = sizex;
    maze->routeSize.y = sizey;
    maze->routes = (unsigned char *)playdate->system->realloc(NULL, maze->routeSize.x * maze->routeSize.y * sizeof (unsigned char));
    if (maze->routes == NULL) {
        playdate->system->error("%s: %d: maze route is not created.", __FILE__, __LINE__);
    }

    // 経路の初期化
    for (int y = 0; y < maze->routeSize.y; y++) {
        for (int x = 0; x < maze->routeSize.x; x++) {
            maze->routes[y * maze->routeSize.x + x] = kMazeRouteNull;
        }
    }

    // 終了
    return maze;
}

// 迷路を解放する
//
void MazeUnload(struct Maze *maze)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 迷路の解放
    if (maze != NULL) {
        if (maze->routes != NULL) {
            playdate->system->realloc(maze->routes, 0);
        }
        if (maze->maps != NULL) {
            playdate->system->realloc(maze->maps, 0);
        }
        playdate->system->realloc(maze, 0);
    }
}

// 穴を掘る
//
void MazeDig(struct Maze *maze, int x, int y)
{
    // 方向の初期化
    int up = 0;
    int down = 0;
    int left = 0;
    int right = 0;

    // 全方向に掘る
    while (up == 0 || down == 0 || left == 0 || right == 0) {

        // ランダムに方向を選択
        int d = IocsGetRandom(&maze->xorshift) & 0x03;

        // 上に掘る
        if (d == 0) {
            if (y >= 2) {
                int y_2 = y - 2;
                int y_1 = y - 1;
                if (maze->maps[y_2 * maze->mapSize.x + x] == kMazeMapBlock) {
                    maze->maps[y_2 * maze->mapSize.x + x] = kMazeMapNull;
                    maze->maps[y_1 * maze->mapSize.x + x] = kMazeMapNull;
                    MazeDig(maze, x, y_2);
                }
            }
            ++up;

        // 下に掘る
        } else if (d == 1) {
            if (y < maze->mapSize.y - 2) {
                int y_2 = y + 2;
                int y_1 = y + 1;
                if (maze->maps[y_2 * maze->mapSize.x + x] == kMazeMapBlock) {
                    maze->maps[y_2 * maze->mapSize.x + x] = kMazeMapNull;
                    maze->maps[y_1 * maze->mapSize.x + x] = kMazeMapNull;
                    MazeDig(maze, x, y_2);
                }
            }
            ++down;

        // 左に掘る
        } else if (d == 2) {
            if (x >= 2) {
                int x_2 = x - 2;
                int x_1 = x - 1;
                if (maze->maps[y * maze->mapSize.x + x_2] == kMazeMapBlock) {
                    maze->maps[y * maze->mapSize.x + x_2] = kMazeMapNull;
                    maze->maps[y * maze->mapSize.x + x_1] = kMazeMapNull;
                    MazeDig(maze, x_2, y);
                }
            }
            ++left;

        // 右に掘る
        } else {
            if (x < maze->mapSize.x - 2) {
                int x_2 = x + 2;
                int x_1 = x + 1;
                if (maze->maps[y * maze->mapSize.x + x_2] == kMazeMapBlock) {
                    maze->maps[y * maze->mapSize.x + x_2] = kMazeMapNull;
                    maze->maps[y * maze->mapSize.x + x_1] = kMazeMapNull;
                    MazeDig(maze, x_2, y);
                }
            }
            ++right;
        }
    }
}

// 経路を設定する
//
void MazeSetRoute(struct Maze *maze)
{
    for (int routey = 0; routey < maze->routeSize.y; routey++) {
        int mapy = routey * 2 + 1;
        for (int routex = 0; routex < maze->routeSize.x; routex++) {
            int mapx = routex * 2 + 1;
            unsigned char route = kMazeRouteNull;
            if (mapy > 0 && maze->maps[(mapy - 1) * maze->mapSize.x + mapx] == kMazeMapNull) {
                route |= kMazeRouteUp;
            }
            if (mapy < maze->mapSize.y - 1 && maze->maps[(mapy + 1) * maze->mapSize.x + mapx] == kMazeMapNull) {
                route |= kMazeRouteDown;
            }
            if (mapx > 0 && maze->maps[mapy * maze->mapSize.x + (mapx - 1)] == kMazeMapNull) {
                route |= kMazeRouteLeft;
            }
            if (mapx < maze->mapSize.x - 1 && maze->maps[mapy * maze->mapSize.x + (mapx + 1)] == kMazeMapNull) {
                route |= kMazeRouteRight;
            }
            maze->routes[routey * maze->routeSize.x + routex] = route;
        }
    }
}

