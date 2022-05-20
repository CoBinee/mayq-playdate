// Enemy.c - エネミー
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Field.h"
#include "Enemy.h"

// 内部関数
//
static void EnemyLocateOnField(void);

// 内部変数
//
static struct Enemy *enemy = NULL;


// エネミーを初期化する
//
void EnemyInitialize(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの確認
    if (sizeof (struct EnemyActor) > kActorBlockSize) {
        playdate->system->error("%s: %d: enemy actor size is over: %d bytes.", __FILE__, __LINE__, sizeof (struct EnemyActor));
    }

    // エネミーの作成
    enemy = (struct Enemy *)playdate->system->realloc(NULL, sizeof (struct Enemy));
    if (enemy == NULL) {
        playdate->system->error("%s: %d: enemy instance is not created.", __FILE__, __LINE__);
    }

    // エネミーの初期化
    {
        // フィールドへの配置
        EnemyLocateOnField();
    }
}

// エネミーを解放する
//
void EnemyRelease(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの解放
    if (enemy != NULL) {
        playdate->system->realloc(enemy, 0);
        enemy = NULL;
    }
}

// エネミーをフィールドに配置する
//
static void EnemyLocateOnField(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // プールされたエネミーの配置
    {
    }
}


#if 0
// エネミーアクタを読み込む
//
void EnemyActorLoadOnField(void)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // アクタの登録
    struct EnemyActor *actor = (struct EnemyActor *)ActorLoad((ActorFunction)EnemyActorPlayOnField, kGamePriorityEnemy);
    if (actor == NULL) {
        playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
    }

    // エネミーの初期化
    {
        // 解放処理の設定
        ActorSetUnload(&actor->actor, (ActorFunction)EnemyActorUnload);

        // タグの設定
        ActorSetTag(&actor->actor, kGameTagEnemy);

        // 位置の設定
        actor->position.x = 2 * kFieldSizePixel + (kFieldSizePixel / 2);
        actor->position.y = 3 * kFieldSizePixel + (kFieldSizePixel - 1);
    }
}

// エネミーアクタを解放する
//
static void EnemyActorUnload(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// エネミーアクタを描画する
//
static void EnemyActorDraw(struct EnemyActor *actor)
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
        int x = actor->position.x - camera->x;
        int y = actor->position.y - camera->y;
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, x, y, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // 
    {
        int x = ((actor->position.x / (kFieldMazeSectionSizeX * kFieldSizePixel)) * 2 + 1);
        int y = ((actor->position.y / (kFieldMazeSectionSizeY * kFieldSizePixel)) * 2 + 1);
        playdate->graphics->fillRect(x * 4 + 248 + 1, y * 4 + 8 + 1, 2, 2, kColorWhite);
    }
}

// エネミーアクタがフィールドをプレイする
//
static void EnemyActorPlayOnField(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 向きの設定
        actor->direction = kDirectionDown;

        // 移動の設定
        actor->move = actor->position;

        // ジャンプの設定
        actor->jump = 0;
        actor->step = 1;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "enemy", enemyAnimationNames_Walk[actor->direction], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // 移動の操作
    if (actor->move.x == actor->position.x && actor->move.y == actor->position.y) {
        int direction = actor->direction;
        if (IocsIsButtonPush(kButtonUp)) {
            bool jump = FieldIsLand(actor->position.x, actor->position.y);
            if (jump) {
                actor->jump = actor->step;
                if (actor->step < 2) {
                    ++actor->step;
                }
            } else {
                --actor->jump;
                if (actor->jump > 0) {
                    jump = true;
                }
            }
            if (IocsIsButtonPush(kButtonLeft)) {
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUpLeft, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionLeft, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionLeft;
            } else if (IocsIsButtonPush(kButtonRight)) {
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUpRight, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                } else if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionRight, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionRight;
            } else {
                if (FieldIsWalkAndJump(actor->position.x, actor->position.y, kDirectionUp, jump, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionUp;
            }
        } else if (IocsIsButtonPush(kButtonDown)) {
            if (IocsIsButtonPush(kButtonLeft)) {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDownLeft, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionLeft, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionLeft;
            } else if (IocsIsButtonPush(kButtonRight)) {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDownRight, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                } else if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionRight, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionRight;
            } else {
                if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionDown, &actor->move)) {
                    ;
                }
                actor->direction = kDirectionDown;
            }
        } else if (IocsIsButtonPush(kButtonLeft)) {
            if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionLeft, &actor->move)) {
                actor->direction = kDirectionLeft;
            }
        } else if (IocsIsButtonPush(kButtonRight)) {
            if (FieldIsWalk(actor->position.x, actor->position.y, kDirectionRight, &actor->move)) {
                actor->direction = kDirectionRight;
            }
        }
        if (actor->position.y == actor->move.y) {
            if (FieldIsFall(actor->move.x, actor->move.y)) {
                if (FieldIsWalk(actor->move.x, actor->move.y, kDirectionDown, &actor->move)) {
                    ;
                }
            }
        }
        if (actor->position.x != actor->move.x) {
            if (actor->move.x < 0) {
                actor->move.x += kFieldSizeX * kFieldSizePixel;
                actor->position.x += kFieldSizeX * kFieldSizePixel;
            } else if (actor->move.x >= kFieldSizeX * kFieldSizePixel) {
                actor->move.x -= kFieldSizeX * kFieldSizePixel;
                actor->position.x -= kFieldSizeX * kFieldSizePixel;
            }
        }
        if (actor->direction != direction) {
            AsepriteStartSpriteAnimation(&actor->animation, "enemy", enemyAnimationNames_Walk[actor->direction], true);
        }
        if (actor->position.y == actor->move.y) {
            actor->jump = 0;
            actor->step = 1;
        }
    }

    // 移動
    {
        bool move = false;
        if (actor->position.x < actor->move.x) {
            actor->position.x += kEnemyMoveSpeed;
            if (actor->position.x > actor->move.x) {
                actor->position.x = actor->move.x;
            }
            move = true;
        } else if (actor->position.x > actor->move.x) {
            actor->position.x -= kEnemyMoveSpeed;
            if (actor->position.x < actor->move.x) {
                actor->position.x = actor->move.x;
            }
            move = true;
        }
        if (actor->position.y < actor->move.y) {
            actor->position.y += kEnemyMoveSpeed;
            if (actor->position.y > actor->move.y) {
                actor->position.y = actor->move.y;
            }
            move = true;
        } else if (actor->position.y > actor->move.y) {
            actor->position.y -= kEnemyMoveSpeed;
            if (actor->position.y < actor->move.y) {
                actor->position.y = actor->move.y;
            }
            move = true;
        }
        if (move) {
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }
    }

    // カメラの設定
    GameSetCamera(actor->position.x + kEnemyCameraX, actor->position.y + kEnemyCameraY);

    // スプライトの更新
    // AsepriteUpdateSpriteAnimation(&actor->animation);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyActorDraw, kGameOrderEnemy);
}
#endif
