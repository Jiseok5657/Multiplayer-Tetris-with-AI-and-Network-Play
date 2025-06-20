/**
 * @file render.c
 * @brief ゲーム画面レンダリング実装
 * 
 * このファイルはテトリスゲームの画面表示機能を実装します。
 * 主な機能:
 *   - ゲームボードの描画
 *   - テトリミノの表示
 *   - スコアとゲーム情報の表示
 *   - コンソールグラフィックスの管理
 *   - カラー表示の制御
 * 
 * 設計思想:
 *   - コンソールベースの軽量レンダリング
 *   - ダブルバッファリングによるちらつき防止
 *   - カラーを使った直感的な表示
 *   - 最小限の画面更新による効率化
 */

#include "render.h"
#include "game_defs.h"
#include <windows.h>
#include <stdio.h>

/* コンソールハンドル */
static HANDLE hConsole = NULL; /**< コンソール出力ハンドル */
static CONSOLE_SCREEN_BUFFER_INFO csbi; /**< コンソール情報 */
static CHAR_INFO back_buffer[BOARD_HEIGHT + 10][BOARD_WIDTH + 20]; /**< ダブルバッファ用バックバッファ */

/* カラーコード定義 */
typedef enum {
    COLOR_BLACK = 0,    /**< 黒色 */
    COLOR_BLUE = 1,     /**< 青色 */
    COLOR_GREEN = 2,    /**< 緑色 */
    COLOR_CYAN = 3,     /**< シアン */
    COLOR_RED = 4,      /**< 赤色 */
    COLOR_MAGENTA = 5,  /**< マゼンタ */
    COLOR_YELLOW = 6,   /**< 黄色 */
    COLOR_WHITE = 7     /**< 白色 */
} ConsoleColor;

/**
 * @brief レンダリングシステムを初期化する
 * @return 0:成功, -1:失敗
 * 
 * 初期化処理:
 * 1. コンソールハンドルの取得
 * 2. コンソール情報の取得
 * 3. カーソルの非表示化
 * 4. コンソールタイトルの設定
 * 5. バックバッファの初期化
 */
int render_init() {
    /* コンソールハンドル取得 */
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    /* コンソール情報取得 */
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return -1;
    }
    
    /* カーソルを非表示 */
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    
    /* コンソールタイトル設定 */
    SetConsoleTitle("TCPベース テトリスゲーム");
    
    /* バックバッファをクリア */
    clear_back_buffer();
    
    return 0;
}

/**
 * @brief ゲーム画面全体をレンダリングする
 * 
 * @param board ゲームボード
 * @param current_piece 現在のテトリミノ
 * @param next_piece 次のテトリミノ
 * @param score スコアコンテキスト
 * @param state ゲーム状態
 * @param player_id プレイヤーID
 * 
 * レンダリング手順:
 * 1. バックバッファをクリア
 * 2. ゲームボードをレンダリング
 * 3. 次のテトリミノを表示
 * 4. スコアと情報を表示
 * 5. バックバッファを画面に出力
 */
void render_game(Board* board, Piece* current_piece, Piece* next_piece, 
                ScoreCtx* score, GameState state, int player_id) {
    /* バックバッファをクリア */
    clear_back_buffer();
    
    /* ゲームボードをレンダリング */
    render_board(board, current_piece);
    
    /* 次のテトリミノを表示 */
    render_next_piece(next_piece);
    
    /* スコアとゲーム情報を表示 */
    render_score(score, player_id, state);
    
    /* バックバッファを画面に出力 */
    flush_back_buffer();
}

/**
 * @brief ゲームボードをレンダリングする
 * 
 * @param board ゲームボード
 * @param piece 現在のテトリミノ
 * 
 * 処理手順:
 * 1. テンポラリボード作成 (現在のピースを含む)
 * 2. ボードの枠線を描画
 * 3. 各ブロックをカラー付きで描画
 * 4. 現在のテトリミノを描画
 */
void render_board(Board* board, Piece* piece) {
    /* 一時ボード作成 (現在のピースを含む) */
    uint8_t temp_board[BOARD_HEIGHT][BOARD_WIDTH];
    memcpy(temp_board, board->grid, sizeof(temp_board));
    
    /* 現在のピースを一時ボードに追加 */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece->matrix[i][j]) {
                int x = piece->x + j;
                int y = piece->y + i;
                if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
                    temp_board[y][x] = piece->type + 1;  /* タイプ+1でブロックを表示 */
                }
            }
        }
    }
    
    /* ボードの開始位置 */
    int board_start_x = 2;
    int board_start_y = 2;
    
    /* ボード上部の枠線 */
    draw_string(board_start_x - 1, board_start_y - 1, "┌──────────┐");
    
    /* ボードのレンダリング */
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        /* 左端の枠線 */
        draw_char(board_start_x - 1, board_start_y + y, '│');
        
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (temp_board[y][x]) {
                /* ブロックをカラー付きで描画 */
                ConsoleColor color = get_tetromino_color(temp_board[y][x] - 1);
                draw_block(board_start_x + x, board_start_y + y, '■', COLOR_WHITE, color);
            } else {
                /* 空ブロック */
                draw_char(board_start_x + x, board_start_y + y, ' ');
            }
        }
        
        /* 右端の枠線 */
        draw_char(board_start_x + BOARD_WIDTH, board_start_y + y, '│');
    }
    
    /* ボード下部の枠線 */
    draw_string(board_start_x - 1, board_start_y + BOARD_HEIGHT, "└──────────┘");
}

/**
 * @brief 次のテトリミノを表示する
 * 
 * @param piece 次のテトリミノ
 */
void render_next_piece(Piece* piece) {
    /* 表示位置 */
    int start_x = BOARD_WIDTH + 6;
    int start_y = 4;
    
    /* タイトル */
    draw_string(start_x, start_y - 2, "次のテトリミノ:");
    
    /* ピースの描画 */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece->matrix[i][j]) {
                ConsoleColor color = get_tetromino_color(piece->type);
                draw_block(start_x + j, start_y + i, '■', COLOR_WHITE, color);
            } else {
                draw_char(start_x + j, start_y + i, ' ');
            }
        }
    }
}

/**
 * @brief スコアとゲーム情報を表示する
 * 
 * @param score スコアコンテキスト
 * @param player_id プレイヤーID
 * @param state ゲーム状態
 */
void render_score(ScoreCtx* score, int player_id, GameState state) {
    /* プレイヤー情報 */
    char player_info[20];
    snprintf(player_info, sizeof(player_info), "プレイヤー: %d", player_id);
    draw_string(2, 0, player_info);
    
    /* スコア情報 */
    char score_info[30];
    snprintf(score_info, sizeof(score_info), "スコア: %d", score->score);
    draw_string(2, 1, score_info);
    
    /* レベル情報 */
    char level_info[30];
    snprintf(level_info, sizeof(level_info), "レベル: %d", score->level);
    draw_string(BOARD_WIDTH + 6, 0, level_info);
    
    /* ライン情報 */
    char line_info[40];
    snprintf(line_info, sizeof(line_info), "ライン: %d/%d", 
             score->lines_cleared, LINES_PER_LEVEL);
    draw_string(BOARD_WIDTH + 6, 1, line_info);
    
    /* ゲーム状態メッセージ */
    if (state == GAME_STATE_PAUSED) {
        draw_string_centered(BOARD_HEIGHT + 3, "=== 一時停止中 ===");
    } else if (state == GAME_STATE_GAME_OVER) {
        draw_string_centered(BOARD_HEIGHT + 3, "=== ゲームオーバー! ===");
    }
    
    /* 操作ガイド */
    draw_string(2, BOARD_HEIGHT + 5, "操作: ←→移動, W:回転, S:ソフトドロップ, スペース:ハードドロップ, P:一時停止");
}

/**
 * @brief テトリミノタイプから色を取得する
 * 
 * @param type テトリミノタイプ
 * @return ConsoleColor 対応するコンソール色
 */
ConsoleColor get_tetromino_color(int type) {
    switch (type) {
        case TETROMINO_I: return COLOR_CYAN;
        case TETROMINO_O: return COLOR_YELLOW;
        case TETROMINO_S: return COLOR_GREEN;
        case TETROMINO_Z: return COLOR_RED;
        case TETROMINO_J: return COLOR_BLUE;
        case TETROMINO_L: return COLOR_MAGENTA;
        case TETROMINO_T: return COLOR_MAGENTA;
        default: return COLOR_WHITE;
    }
}

/**
 * @brief コンソールカラーを設定する
 * 
 * @param fg 前景色
 * @param bg 背景色
 */
void set_color(ConsoleColor fg, ConsoleColor bg) {
    SetConsoleTextAttribute(hConsole, (bg << 4) | fg);
}

/**
 * @brief バックバッファをクリアする
 */
void clear_back_buffer() {
    for (int y = 0; y < BOARD_HEIGHT + 10; y++) {
        for (int x = 0; x < BOARD_WIDTH + 20; x++) {
            back_buffer[y][x].Char.UnicodeChar = ' ';
            back_buffer[y][x].Attributes = csbi.wAttributes;
        }
    }
}

/**
 * @brief バックバッファを画面に出力する
 */
void flush_back_buffer() {
    COORD buffer_size = {BOARD_WIDTH + 20, BOARD_HEIGHT + 10};
    COORD buffer_coord = {0, 0};
    SMALL_RECT write_region = {0, 0, BOARD_WIDTH + 19, BOARD_HEIGHT + 9};
    
    /* バックバッファを画面に書き出し */
    WriteConsoleOutput(
        hConsole,
        (CHAR_INFO*)back_buffer,
        buffer_size,
        buffer_coord,
        &write_region
    );
}

/**
 * @brief 指定位置に文字を描画
 * 
 * @param x X座標
 * @param y Y座標
 * @param ch 描画する文字
 */
void draw_char(int x, int y, char ch) {
    if (x >= 0 && x < BOARD_WIDTH + 20 && y >= 0 && y < BOARD_HEIGHT + 10) {
        back_buffer[y][x].Char.AsciiChar = ch;
        back_buffer[y][x].Attributes = csbi.wAttributes;
    }
}

/**
 * @brief 指定位置に文字列を描画
 * 
 * @param x X座標
 * @param y Y座標
 * @param str 描画する文字列
 */
void draw_string(int x, int y, const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(x + i, y, str[i]);
    }
}

/**
 * @brief 中央揃えで文字列を描画
 * 
 * @param y Y座標
 * @param str 描画する文字列
 */
void draw_string_centered(int y, const char* str) {
    int len = strlen(str);
    int x = (BOARD_WIDTH + 20 - len) / 2;
    draw_string(x, y, str);
}

/**
 * @brief カラー付きブロックを描画
 * 
 * @param x X座標
 * @param y Y座標
 * @param ch 描画する文字
 * @param fg 前景色
 * @param bg 背景色
 */
void draw_block(int x, int y, char ch, ConsoleColor fg, ConsoleColor bg) {
    if (x >= 0 && x < BOARD_WIDTH + 20 && y >= 0 && y < BOARD_HEIGHT + 10) {
        back_buffer[y][x].Char.AsciiChar = ch;
        back_buffer[y][x].Attributes = (bg << 4) | fg;
    }
}

/**
 * @brief レンダリングシステムを終了する
 */
void render_cleanup() {
    /* カーソルを表示 */
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    
    /* カラーをリセット */
    set_color(COLOR_WHITE, COLOR_BLACK);
}