// Define.h - 定義
//
#pragma once

// 外部参照
//
#include <stdbool.h>
#include "pd_api.h"


// ベクタ
//
struct Vector {
    int x;
    int y;
};

// 矩形
//
struct Rect {
    int left;
    int top;
    int right;
    int bottom;
};

// 向き
//
enum {
    kDirectionUp = 0, 
    kDirectionDown, 
    kDirectionLeft, 
    kDirectionRight, 
    kDirectionUpLeft, 
    kDirectionUpRight, 
    kDirectionDownLeft, 
    kDirectionDownRight, 
};


