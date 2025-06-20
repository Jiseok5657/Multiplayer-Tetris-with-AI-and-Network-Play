/**
 * @file piece.c
 * @brief テトリミノ操作実装
 * 
 * 主な機能:
 *   - テトリミノ形状の定義
 *   - 移動と回転操作
 *   - 壁キック機構
 *   - 衝突検出
 * 
 * 設計思想:
 *   - テトリミノの回転状態を事前定義マトリックスで管理
 *   - SRS (Super Rotation System) 準拠の壁キック
 *   - 効率的なマトリックス操作
 */

#include "piece.h"
#include <stdlib.h>
#include <time.h>
#include "error.h"

/* テトリミノ形状定義 (4x4マトリックス) */
const int TETROMINO_SHAPES[TETROMINO_COUNT][4][4][4] = {
    // TETROMINO_I
    {
        {
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0}
        },
        {
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0}
        }
    },
    // TETROMINO_O
    {
        {
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    },
    // TETROMINO_S
    {
        {
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,1,0},
            {0,0,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    // TETROMINO_Z
    {
        {
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {0,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {1,0,0,0},
            {0,0,0,0}
        }
    },
    // TETROMINO_J
    {
        {
            {1,0,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,0,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {1,1,0,0},
            {0,0,0,0}
        }
    },
    // TETROMINO_L
    {
        {
            {0,0,1,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,1,0},
            {1,0,0,0},
            {0,0,0,0}
        },
        {
            {1,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    // TETROMINO_T
    {
        {
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    }
};

/* 壁キックテストパターン (通常テトリミノ) */
const int WALL_KICK_DATA[4][WALL_KICK_TESTS][2] = {
    /* 0>>1 */ {{0,0}, {-1,0}, {-1,1}, {0,-2}, {-1,-2}},
    /* 1>>2 */ {{0,0}, {1,0}, {1,-1}, {0,2}, {1,2}},
    /* 2>>3 */ {{0,0}, {1,0}, {1,1}, {0,-2}, {1,-2}},
    /* 3>>0 */ {{0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2}}
};

/* 壁キックテストパターン (Iテトリミノ) */
const int WALL_KICK_I_DATA[4][WALL_KICK_TESTS][2] = {
    /* 0>>1 */ {{0,0}, {-2,0}, {1,0}, {-2,-1}, {1,2}},
    /* 1>>2 */ {{0,0}, {-1,0}, {2,0}, {-1,2}, {2,-1}},
    /* 2>>3 */ {{0,0}, {2,0}, {-1,0}, {2,1}, {-1,-2}},
    /* 3>>0 */ {{0,0}, {1,0}, {-2,0}, {1,-2}, {-2,1}}
};

/* テトリミノの初期位置 */
const int INITIAL_POSITIONS[TETROMINO_COUNT][2] = {
    {3, -1}, // I (中央上部)
    {4, -1}, // O
    {4, -1}, // S
    {4, -1}, // Z
    {4, -1}, // J
    {4, -1}, // L
    {4, -1}  // T
};

/**
 * @brief テトリミノを生成する
 */
Piece piece_create(TetrominoType type) {
    Piece piece;
    piece.type = type;
    piece.x = INITIAL_POSITIONS[type][0];
    piece.y = INITIAL_POSITIONS[type][1];
    piece.rotation = 0;
    piece_set_shape(&piece, type);
    return piece;
}

/**
 * @brief テトリミノをボード上部に生成する
 */
Piece piece_spawn_at_top() {
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    TetrominoType type = (TetrominoType)(rand() % TETROMINO_COUNT);
    return piece_create(type);
}

/**
 * @brief テトリミノを左に移動する
 */
int piece_move_left(Piece *piece, const Board *board) {
    piece->x--;
    if (board_check_collision(piece, board)) {
        piece->x++; // 元に戻す
        return 0;
    }
    return 1;
}

/**
 * @brief テトリミノを右に移動する
 */
int piece_move_right(Piece *piece, const Board *board) {
    piece->x++;
    if (board_check_collision(piece, board)) {
        piece->x--; // 元に戻す
        return 0;
    }
    return 1;
}

/**
 * @brief テトリミノを下に移動する
 */
int piece_move_down(Piece *piece, const Board *board) {
    piece->y++;
    if (board_check_collision(piece, board)) {
        piece->y--; // 元に戻す
        return 0;
    }
    return 1;
}

/**
 * @brief テトリミノを回転する
 */
int piece_rotate(Piece *piece, const Board *board, RotateDirection direction) {
    // 元の状態を保存
    Piece original = *piece;
    
    // 回転方向に応じて新しい回転状態を計算
    int new_rotation = piece->rotation;
    if (direction == ROTATE_CW) {
        new_rotation = (new_rotation + 1) % 4;
    } else {
        new_rotation = (new_rotation + 3) % 4; // CCWは+3で等価
    }
    
    // 新しい回転状態を適用
    piece->rotation = new_rotation;
    piece_set_shape(piece, piece->type);
    
    // 壁キックテスト用データを選択
    const int (*kick_data)[2] = (piece->type == TETROMINO_I) ? 
        WALL_KICK_I_DATA[original.rotation] : 
        WALL_KICK_DATA[original.rotation];
    
    // 壁キックテストを実行
    for (int i = 0; i < WALL_KICK_TESTS; i++) {
        int x_offset = kick_data[i][0];
        int y_offset = kick_data[i][1];
        
        piece->x = original.x + x_offset;
        piece->y = original.y + y_offset;
        
        if (!board_check_collision(piece, board)) {
            return 1; // 回転成功
        }
    }
    
    // 全てのテストが失敗したら元に戻す
    *piece = original;
    return 0; // 回転失敗
}

/**
 * @brief テトリミノの形状を設定する
 */
void piece_set_shape(Piece *piece, TetrominoType type) {
    // 現在の回転状態に基づいて形状を設定
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            piece->matrix[y][x] = TETROMINO_SHAPES[type][piece->rotation][y][x];
        }
    }
}

/**
 * @brief テトリミノを複製する
 */
Piece piece_clone(const Piece *src) {
    Piece dest;
    dest.type = src->type;
    dest.x = src->x;
    dest.y = src->y;
    dest.rotation = src->rotation;
    
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            dest.matrix[y][x] = src->matrix[y][x];
        }
    }
    
    return dest;
}