/**
 * @file board.c
 * @brief ゲームボード管理実装
 * 
 * 主な機能:
 *   - ボードメモリ管理
 *   - テトリミノの配置と衝突検出
 *   - ライン消去アルゴリズム
 *   - ボード状態のリセット
 * 
 * 設計思想:
 *   - 単一アロケーションによる効率的なメモリ管理
 *   - 境界チェックを伴う安全な操作
 *   - 最適化されたライン消去処理
 */

#include "board.h"
#include <stdlib.h>
#include <string.h>
#include "error.h"

Board* board_create(int width, int height) {
    // 構造体とグリッドデータのメモリを一括割当
    size_t total_size = sizeof(Board) + (width * height * sizeof(uint8_t));
    Board* board = (Board*)malloc(total_size);
    
    if (!board) {
        LOG_ERROR("ボードメモリ割当失敗", ERROR_MEMORY_ALLOC);
        return NULL;
    }
    
    board->width = width;
    board->height = height;
    board->grid = (uint8_t*)(board + 1);  // 構造体直後にグリッド
    
    // グリッドを空状態で初期化
    board_reset(board);
    
    LOG_DEBUG("ボード作成: %dx%d", width, height);
    return board;
}

void board_destroy(Board* board) {
    if (board) {
        free(board);
        LOG_DEBUG("ボードメモリ解放");
    }
}

void board_reset(Board* board) {
    // グリッド全体をクリア (空状態=0)
    memset(board->grid, 0, board->width * board->height * sizeof(uint8_t));
    LOG_DEBUG("ボードリセット完了");
}

void board_place_piece(Board* board, const Piece* piece) {
    // テトリミノの4x4マトリックスを走査
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            // テトリミノのブロックが存在する場合
            if (piece->matrix[y][x]) {
                int board_x = piece->x + x;
                int board_y = piece->y + y;
                
                // ボード範囲内チェック
                if (board_x >= 0 && board_x < board->width && 
                    board_y >= 0 && board_y < board->height) {
                    // ボードにブロックタイプを設定 (タイプ+1)
                    board->grid[board_y * board->width + board_x] = piece->type + 1;
                }
            }
        }
    }
    LOG_DEBUG("テトリミノ配置: タイプ=%d @(%d,%d)", piece->type, piece->x, piece->y);
}

int board_check_collision(const Piece* piece, const Board* board) {
    // テトリミノの4x4マトリックスを走査
    for (int y = 0; y < TETROMINO_SIZE; y++) {
        for (int x = 0; x < TETROMINO_SIZE; x++) {
            // テトリミノのブロックが存在する場合
            if (piece->matrix[y][x]) {
                int board_x = piece->x + x;
                int board_y = piece->y + y;
                
                // 境界チェック (左右と下部)
                if (board_x < 0 || board_x >= board->width || board_y >= board->height) {
                    return 1;  // 衝突あり
                }
                
                // 下部境界より上にある場合のみブロックチェック
                if (board_y >= 0) {
                    // 既存ブロックとの衝突チェック
                    if (board->grid[board_y * board->width + board_x] != 0) {
                        return 1;  // 衝突あり
                    }
                }
            }
        }
    }
    return 0;  // 衝突なし
}

int board_clear_lines(Board* board) {
    int lines_cleared = 0;
    int dest_row = board->height - 1;
    
    // 下から上に走査
    for (int src_row = board->height - 1; src_row >= 0; src_row--) {
        int is_full = 1;
        
        // ラインが完全に埋まっているかチェック
        for (int x = 0; x < board->width; x++) {
            if (board->grid[src_row * board->width + x] == 0) {
                is_full = 0;
                break;
            }
        }
        
        if (!is_full) {
            // 非フルラインをコピー
            if (dest_row != src_row) {
                memcpy(&board->grid[dest_row * board->width],
                       &board->grid[src_row * board->width],
                       board->width * sizeof(uint8_t));
            }
            dest_row--;
        } else {
            lines_cleared++;
        }
    }
    
    // 上部の空き領域をクリア
    for (int y = 0; y < lines_cleared; y++) {
        memset(&board->grid[y * board->width], 0, board->width * sizeof(uint8_t));
    }
    
    if (lines_cleared > 0) {
        LOG_DEBUG("%d ライン消去完了", lines_cleared);
    }
    return lines_cleared;
}