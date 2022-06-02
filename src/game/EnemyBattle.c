// EnemyBattle.c - バトルエネミー
//

// 外部参照
//
#include <string.h>
#include "pd_api.h"
#include "Iocs.h"
#include "Actor.h"
#include "Game.h"
#include "Battle.h"
#include "Enemy.h"

// 内部関数
//
static void EnemyBattleActorUnload(struct EnemyActor *actor);
static void EnemyBattleActorDrawCharacter(struct EnemyActor *actor);
static void EnemyBattleActorDrawDeath(struct EnemyActor *actor);
static void EnemyBattleActorIdle(struct EnemyActor *actor);
static void EnemyBattleActorWalkRandom(struct EnemyActor *actor);
static void EnemyBattleActorDeath(struct EnemyActor *actor);
static int EnemyBattleGetWalkableDirection(struct EnemyActor *actor);
static int EnemyBattleGetWalkableRandomDirection(struct EnemyActor *actor);
static int EnemyBattleMove(struct EnemyActor *actor, int direction, int distance);
static int EnemyBattleForward(struct EnemyActor *actor);
static void EnemyBattleSetDamage(struct EnemyActor *actor, int direction, int point);
static bool EnemyBattleDamage(struct EnemyActor *actor);
static void EnemyBattleBlink(struct EnemyActor *actor);
static void EnemyBattleCalcRect(struct EnemyActor *actor);

// 内部変数
//
static ActorFunction enemyBattleActorFunctions[kEnemyBattleActionSize] = {
    (ActorFunction)EnemyActorNull, 
    (ActorFunction)EnemyBattleActorIdle, 
    (ActorFunction)EnemyBattleActorWalkRandom, 
};
static const char *enemyBattleAnimationNames_Walk[kEnemyFaceSize] = {
    "WalkLeft", 
    "WalkRight", 
};


// エネミーアクタを読み込む
//
void EnemyBattleActorLoad(int type, int rest, int direction)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // エネミーの読み込み
    for (int i = 0; i < rest; i++) {

        // エネミーの取得
        const struct EnemyData *data = &enemyDatas[type];

        // アクタの登録
        struct EnemyActor *actor = (struct EnemyActor *)ActorLoad(enemyBattleActorFunctions[data->battleAction], kGamePriorityEnemy);
        if (actor == NULL) {
            playdate->system->error("%s: %d: enemy actor is not loaded.", __FILE__, __LINE__);
        }

        // エネミーの初期化
        {
            // 解放処理の設定
            ActorSetUnload(&actor->actor, (ActorFunction)EnemyBattleActorUnload);

            // タグの設定
            ActorSetTag(&actor->actor, kGameTagEnemy);

            // データの設定
            actor->data = data;

            // 体力の設定
            actor->life = data->life;

            // 位置の設定
            BattleGetEnemyPosition(i, direction, &actor->position);

            // ダメージの設定
            actor->damagePoint = 0;

            // 点滅の設定
            actor->blink = 0;

            // 矩形の計算
            EnemyBattleCalcRect(actor);
        }
    }
}

// エネミーアクタを解放する
//
static void EnemyBattleActorUnload(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }
}

// エネミーアクタを描画する
//
static void EnemyBattleActorDrawCharacter(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クリップの設定
    BattleSetClip();

    // スプライトの描画
    if ((actor->blink & kEnemyBlinkInterval) == 0) {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        int drawmode = actor->damagePoint > 0 ? kDrawModeInverted : kDrawModeCopy;
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, actor->data->centerX, actor->data->centerY, 1.0f, 1.0f, drawmode);
    }

    // クリップの解除
    BattleClearClip();
}
static void EnemyBattleActorDrawDeath(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // クリップの設定
    BattleSetClip();

    // スプライトの描画
    {
        struct Vector view;
        GameGetBattleCameraPosition(actor->position.x, actor->position.y, &view);
        int drawmode = actor->damagePoint > 0 ? kDrawModeInverted : kDrawModeCopy;
        AsepriteDrawRotatedSpriteAnimation(&actor->animation, view.x, view.y, 0.0f, 0.5f, 0.8f, 1.0f, 1.0f, kDrawModeCopy);
    }

    // クリップの解除
    BattleClearClip();
}

// エネミーアクタが待機する
//
void EnemyBattleActorIdle(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 向きの設定
        actor->direction = IocsGetRandomNumber(NULL) % 4;

        // 体の向きの設定
        actor->face = IocsGetRandomBool(NULL) ? kEnemyFaceLeft : kEnemyFaceRight;

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // ダメージ
        if (EnemyBattleDamage(actor)) {
            ;

        // 待機
        } else {

            // アニメーションの更新
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }

        // 点滅
        EnemyBattleBlink(actor);
    }

    // 矩形の計算
    EnemyBattleCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDrawCharacter, kGameOrderCharacter + actor->position.y);
}

// エネミーアクタがランダムに歩く
//
void EnemyBattleActorWalkRandom(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // 向きの設定
        actor->direction = EnemyBattleGetWalkableRandomDirection(actor);

        // 体の向きの設定
        actor->face = (actor->direction == kDirectionLeft || (actor->direction != kDirectionRight && IocsGetRandomBool(NULL))) ? kEnemyFaceLeft : kEnemyFaceRight;

        // 移動の設定
        actor->moveSpeed = 0;
        actor->moveParams[0] = actor->data->battleMoveBase + (IocsGetRandomNumber(NULL) % actor->data->battleMoveRange);

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // ダメージ
        if (EnemyBattleDamage(actor)) {
            ;

        // 移動
        } else {

            // 前に進む
            int distance = EnemyBattleForward(actor);
            if (distance > 0) {
                actor->moveParams[0] -= distance;
            }

            // 移動の完了
            if (actor->moveParams[0] <= 0 || distance == 0) {

                // 向きの変更
                actor->direction = EnemyBattleGetWalkableRandomDirection(actor);
                int face = actor->face;
                if (actor->face == kEnemyFaceLeft && actor->direction == kDirectionRight) {
                    actor->face = kEnemyFaceRight;
                } else if (actor->face == kEnemyFaceRight && actor->direction == kDirectionLeft) {
                    actor->face = kEnemyFaceLeft;
                }

                // 移動の設定
                actor->moveParams[0] = actor->data->battleMoveBase + (IocsGetRandomNumber(NULL) % actor->data->battleMoveRange);

                // アニメーションの開始
                if (face != actor->face) {
                    AsepriteStartSpriteAnimation(&actor->animation, actor->data->sprite, enemyBattleAnimationNames_Walk[actor->face], true);
                }
            }

            // アニメーションの更新
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }

        // 点滅
        EnemyBattleBlink(actor);
    }

    // 矩形の計算
    EnemyBattleCalcRect(actor);

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDrawCharacter, kGameOrderCharacter + actor->position.y);
}

// エネミーアクタが死亡する
//
void EnemyBattleActorDeath(struct EnemyActor *actor)
{
    // Playdate の取得
    PlaydateAPI *playdate = IocsGetPlaydate();
    if (playdate == NULL) {
        return;
    }

    // 初期化
    if (actor->actor.state == 0) {

        // アニメーションの開始
        AsepriteStartSpriteAnimation(&actor->animation, "death", "Idle", false);

        // 初期化の完了
        ++actor->actor.state;
    }

    // プレイ中
    if (GameIsPlay()) {

        // アニメーションの完了
        if (AsepriteIsSpriteAnimationDone(&actor->animation)) {
            actor->actor.state = -1;

        // アニメーションの更新
        } else {
            AsepriteUpdateSpriteAnimation(&actor->animation);
        }
    }

    // 描画処理の設定
    ActorSetDraw(&actor->actor, (ActorFunction)EnemyBattleActorDrawDeath, kGameOrderCharacter + actor->position.y);

    // 死亡の完了
    if (actor->actor.state < 0) {
        ActorUnload(&actor->actor);
    }
}

// 移動できる方向を取得する
//
static int EnemyBattleGetWalkableDirection(struct EnemyActor *actor)
{
    int d = 0;
    if (BattleIsSpace(actor->moveRect.left, actor->moveRect.top - 1) && BattleIsSpace(actor->moveRect.right, actor->moveRect.top - 1)) {
        d |= (1 << kDirectionUp);
    }
    if (BattleIsSpace(actor->moveRect.left, actor->moveRect.bottom + 1) && BattleIsSpace(actor->moveRect.right, actor->moveRect.bottom + 1)) {
        d |= (1 << kDirectionDown);
    }
    if (BattleIsSpace(actor->moveRect.left - 1, actor->moveRect.top) && BattleIsSpace(actor->moveRect.left - 1, actor->moveRect.bottom)) {
        d |= (1 << kDirectionLeft);
    }
    if (BattleIsSpace(actor->moveRect.left + 1, actor->moveRect.top) && BattleIsSpace(actor->moveRect.left + 1, actor->moveRect.bottom)) {
        d |= (1 << kDirectionRight);
    }
    return d;
}
static int EnemyBattleGetWalkableRandomDirection(struct EnemyActor *actor)
{
    int direction = -1;
    int d = EnemyBattleGetWalkableDirection(actor);
    if (d == (1 << actor->direction)) {
        direction = actor->direction;
    } else {
        d &= ~(1 << actor->direction);
        direction = IocsGetRandomNumber(NULL) % 4;
        while ((d & (1 << direction)) == 0) {
            ++direction;
            if (direction >= 4) {
                direction = 0;
            }
        }
    }
    return direction;
}

// 指定した方向に移動する
//
static int EnemyBattleMove(struct EnemyActor *actor, int direction, int distance)
{
    if (direction == kDirectionUp) {
        int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionUp, distance, false);
        int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionUp, distance, false);
        distance = dl < dr ? dl : dr;
        actor->position.y -= distance;
    } else if (direction == kDirectionDown) {
        int dl = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionDown, distance, false);
        int dr = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionDown, distance, false);
        distance = dl < dr ? dl : dr;
        actor->position.y += distance;
    } else if (direction == kDirectionLeft) {
        int dt = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.top, kDirectionLeft, distance, false);
        int db = BattleGetMoveDistance(actor->moveRect.left, actor->moveRect.bottom, kDirectionLeft, distance, false);
        distance = dt < db ? dt : db;
        actor->position.x -= distance;
    } else if (direction == kDirectionRight) {
        int dt = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.top, kDirectionRight, distance, false);
        int db = BattleGetMoveDistance(actor->moveRect.right, actor->moveRect.bottom, kDirectionRight, distance, false);
        distance = dt < db ? dt : db;
        actor->position.x += distance;
    } else {
        distance = 0;
    }
    return distance;
}

// 向いている方向に移動する
//
static int EnemyBattleForward(struct EnemyActor *actor)
{
    int distance = -1;
    actor->moveSpeed += actor->data->battleSpeed;
    if (actor->moveSpeed >= kEnemyBattleSpeedOne) {
        distance = actor->moveSpeed >> kEnemyBattleSpeedShift;
        actor->moveSpeed &= kEnemyBattleSpeedMask;
        distance = EnemyBattleMove(actor, actor->direction, distance);
    }
    return distance;
}

// ダメージを受ける
//
static void EnemyBattleSetDamage(struct EnemyActor *actor, int direction, int point)
{
    if (actor->damagePoint == 0) {
        actor->damagePoint = point;
        actor->damageDirection = direction;
        actor->damageSpeed = kEnemyDamageSpeed;
    }
}
static bool EnemyBattleDamage(struct EnemyActor *actor)
{
    if (actor->damagePoint > 0) {
        EnemyBattleMove(actor, actor->damageDirection, actor->damageSpeed);
        if (actor->damageSpeed > 0) {
            --actor->damageSpeed;
        } else {
            actor->life -= actor->damagePoint;
            actor->damagePoint = 0;
            if (actor->life > 0) {
                actor->blink = kEnemyBlinkDamage;
            } else {
                ActorTransition(&actor->actor, (ActorFunction)EnemyBattleActorDeath);
            }
        }
    }
    return actor->damagePoint > 0 ? true : false;
}

// 点滅する
//
static void EnemyBattleBlink(struct EnemyActor *actor)
{
    if (actor->blink > 0) {
        --actor->blink;
    }
}

// 矩形を計算する
//
static void EnemyBattleCalcRect(struct EnemyActor *actor)
{
    // 移動の計算
    {
        actor->moveRect.left = actor->position.x + actor->data->rect.left;
        actor->moveRect.top = actor->position.y + actor->data->rect.top;
        actor->moveRect.right = actor->position.x + actor->data->rect.right;
        actor->moveRect.bottom = actor->position.y + actor->data->rect.bottom;
    }
}

// エネミーにヒットするかどうかを判定する
//
void EnemyBattleIsHitThenDamage(struct Rect *rect, int x, int y, int point)
{
    struct EnemyActor *actor = (struct EnemyActor *)ActorFindWithTag(kGameTagEnemy);
    while (actor != NULL) {
        if (actor->moveRect.left > rect->right || actor->moveRect.right < rect->left || actor->moveRect.top > rect->bottom || actor->moveRect.bottom < rect->top) {
            ;
        } else if (actor->damagePoint == 0 && actor->blink == 0) {
            int dx = actor->position.x - x;
            int dy = actor->position.y - y;
            int direction = abs(dy) > abs(dx) ? (dy <= 0 ? kDirectionUp : kDirectionDown) : (dx <= 0 ? kDirectionLeft : kDirectionRight);
            EnemyBattleSetDamage(actor, direction, point);
        }
        actor = (struct EnemyActor *)ActorNextWithTag(&actor->actor);
    }
}

// 指定した種類のエネミーの数を取得する
//
int EnemyBattleGetRest(int type)
{
    int rest = 0;
    struct EnemyActor *actor = (struct EnemyActor *)ActorFindWithTag(kGameTagEnemy);
    while (actor != NULL) {
        if (actor->data->type == type) {
            ++rest;
        }
        actor = (struct EnemyActor *)ActorNextWithTag(&actor->actor);
    }
    return rest;
}

