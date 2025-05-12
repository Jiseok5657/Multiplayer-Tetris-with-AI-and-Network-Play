/* GAME_H */
#ifndef GAME_H
#define GAME_H

#include <ncurses.h>  /* PDCursesライブラリのインクルード（コンソールグラフィック用） */

// テトリスボードのサイズ
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

/* ブロックの種類（7種類のテトリスブロック） */
#define I_BLOCK 1
#define O_BLOCK 2
#define T_BLOCK 3
#define S_BLOCK 4
#define Z_BLOCK 5
#define J_BLOCK 6
#define L_BLOCK 7

/* 色の定義（4種類の色） check if it's c89 or c99 and make changes accordingly (replacing enum)*/
#define COLOR_RED 1
#define COLOR_BLUE 2
#define COLOR_GREEN 3
#define COLOR_YELLOW 4

/* ブロック構造体の定義 */
typedef struct {
    int type;          /* ブロックの種類（I, O, T, S, Z, J, L） */
    int color;         /* ブロックの色 */
    int shape[4][4];   /* ブロックの形状（回転のため4x4配列を使用） */
    int x, y;          /* 現在の位置（x, y座標） */
    int rotation;      /* 回転状態（0, 90, 180, 270） */
} Block;

/* ゲームボード */
extern int board[BOARD_HEIGHT][BOARD_WIDTH];

/* 基本機能の関数宣言 */
void init_board();                            /* ゲームボードの初期化 */
void print_board();                           /* ゲームボードの表示 */
int check_collision(int x, int y, int rotation); /* 衝突判定（位置と回転状態に基づく） */
void move_block(int direction);              /* ブロックの左右移動 */
void drop_block();                           /* ブロックの落下 */
void rotate_block();                         /* ブロックの回転 */
void fix_block();                            /* ブロックの固定 */
void delete_full_lines();                    /* 完成した行の削除 */
int check_game_over();                       /* ゲームオーバー判定 */

/* 色の初期化関数（PDCurses用） */
void init_colors();                          /* 色の初期化 */

#endif /* GAME_H */
