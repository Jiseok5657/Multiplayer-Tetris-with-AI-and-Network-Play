/* src/game.c */
#include <stdio.h>
#include "game.h"

/* ゲームボード配列 */
int board[BOARD_HEIGHT][BOARD_WIDTH];

/* ゲームボードの初期化 */
void init_board() {
    memset(board, 0, sizeof(board));
}

void init_board_loops() {
    int i, j;
    
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        for (j = 0; j < BOARD_WIDTH; ++j) {
            board[i][j] = 0;
        }
    }
}

/* ゲームボードの表示 */
void print_board() {
    /* Jiseok */
}

/* 衝突判定（座標および回転に基づく） */
int check_collision(int x, int y, int rotation) {
    /* Skip */
}

/* ブロックの移動 */
void move_block(int direction) {
    /* Jiseok */
}

/* ブロックの落下 */
void drop_block() {
    /* Jiseok */
}

/* ブロックの回転 */
void rotate_block() {
    /* Skip */
}

/* ブロックの固定 */
void fix_block() {
    /* Jiseok */
}

/* 完成行の削除 */
void delete_full_lines() {
    /* Jiseok */
}

/* ゲームオーバー判定 */
int check_game_over() {
    /* Jiseok */
}
