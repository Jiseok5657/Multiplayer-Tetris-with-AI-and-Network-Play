/* src/game.c */
#include <string.h>   /* memset用 */
#include <stdlib.h>   /* rand用 */
#include "game.h"

/* ゲームボード配列 */
int board[BOARD_HEIGHT][BOARD_WIDTH];

/* 操作中ブロック（グローバル） */
Block current_block;

/* 各ブロック形状データ (4x4固定) */
static int BLOCK_SHAPES[7][4][4] = {
    /* I_BLOCK */
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    /* O_BLOCK */
    {
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    },
    /* T_BLOCK */
    {
        {0,0,0,0},
        {1,1,1,0},
        {0,1,0,0},
        {0,0,0,0}
    },
    /* S_BLOCK */
    {
        {0,0,0,0},
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0}
    },
    /* Z_BLOCK */
    {
        {0,0,0,0},
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0}
    },
    /* J_BLOCK */
    {
        {0,0,0,0},
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0}
    },
    /* L_BLOCK */
    {
        {0,0,0,0},
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0}
    }
};

/* ブロック種類ごとの色割り当て（COLOR_* 4色のみ） */
static int BLOCK_COLORS[7] = {
    COLOR_BLUE,    /* I_BLOCK */
    COLOR_YELLOW,  /* O_BLOCK */
    COLOR_GREEN,   /* T_BLOCK */
    COLOR_GREEN,   /* S_BLOCK */
    COLOR_RED,     /* Z_BLOCK */
    COLOR_BLUE,    /* J_BLOCK */
    COLOR_YELLOW   /* L_BLOCK */
};

/* ゲームボードの初期化 */
void init_board() {
    memset(board, 0, sizeof(board));
}

/* ボード初期化(ループ) */
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
    int i, j;
    for (i = 0; i < BOARD_HEIGHT; ++i) {
        for (j = 0; j < BOARD_WIDTH; ++j) {
            if (board[i][j]) {
                attron(COLOR_PAIR(board[i][j]));
                mvaddch(i, j, '#');
                attroff(COLOR_PAIR(board[i][j]));
            } else {
                mvaddch(i, j, ' ');
            }
        }
    }

    /* 操作中ブロック（current_block）の表示 */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if (current_block.shape[i][j]) {
                int y = current_block.y + i;
                int x = current_block.x + j;
                /* 境界チェック：画面外にはみ出さない */
                if (y >= 0 && y < BOARD_HEIGHT && x >= 0 && x < BOARD_WIDTH) {
                    attron(COLOR_PAIR(current_block.color));
                    mvaddch(y, x, '#');
                    attroff(COLOR_PAIR(current_block.color));
                }
            }
        }
    }
    refresh();
}

/* 衝突判定 */
void spawn_new_block() {
    int type, i, j;
    type = (rand() % 7) + 1;  /* 1～7: I_BLOCK～L_BLOCK */
    current_block.type = type;
    current_block.color = BLOCK_COLORS[type - 1];
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            current_block.shape[i][j] = BLOCK_SHAPES[type - 1][i][j];
        }
    }
    current_block.x = (BOARD_WIDTH - 4) / 2;
    current_block.y = 0;
    current_block.rotation = 0;
}

/* 衝突判定（座標および回転に基づく） */
int check_collision(int x, int y, int rotation) {
    int i, j, r;
    int shape[4][4];
    /* 形状コピー */
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            shape[i][j] = current_block.shape[i][j];
    /* 回転処理 */
    for (r = 0; r < rotation; ++r) {
        int tmp[4][4];
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
                tmp[i][j] = shape[3 - j][i];
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
                shape[i][j] = tmp[i][j];
    }
    /* 衝突チェック */
    for (i = 0; i < 4; ++i) {
        for (j = 0; j < 4; ++j) {
            if (shape[i][j]) {
                int nx = x + j;
                int ny = y + i;
                if (nx < 0 || nx >= BOARD_WIDTH || ny < 0 || ny >= BOARD_HEIGHT)
                    return 1;
                if (board[ny][nx])
                    return 1;
            }
        }
    }
    return 0;
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
