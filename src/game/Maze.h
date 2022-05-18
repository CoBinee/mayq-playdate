// Maze.h - 迷路
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Define.h"


// マップ
//
enum {
    kMazeMapNull = 0x00, 
    kMazeMapBlock = 0x80, 
    kMazeMapLock = 0x40, 
};

// 経路
//
enum {
    kMazeRouteNull = 0x00, 
    kMazeRouteUp = 0x01, 
    kMazeRouteDown = 0x02, 
    kMazeRouteLeft = 0x04, 
    kMazeRouteRight = 0x08, 
};

// 迷路
//
struct Maze {

    // マップ
    unsigned char *maps;
    struct Vector mapSize;

    // 経路
    unsigned char *routes;
    struct Vector routeSize;

    // 乱数
    struct XorShift xorshift;

};

// 外部参照関数
//
extern struct Maze *MazeLoad(int sizex, int sizey, int seed);
extern void MazeUnload(struct Maze *maze);
extern void MazeDig(struct Maze *maze, int x, int y);
extern void MazeSetRoute(struct Maze *maze);
