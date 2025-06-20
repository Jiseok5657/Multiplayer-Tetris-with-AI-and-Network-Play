/**
 * @file game_defs.h
 * @brief ゲーム全体の定義と定数
 * 
 * このファイルはテトリスゲーム全体で使用される定数、列挙型、構造体を定義します。
 * 主な内容:
 *   - ゲーム状態とモードの列挙型
 *   - テトリミノタイプと回転方向
 *   - キーバインディング
 *   - ゲームボードの定数
 *   - スコアリングシステム
 *   - ゲーム構造体の定義
 * 
 * 設計思想:
 *   - 一箇所での定義による一貫性の確保
 *   - モジュール間の依存関係最小化
 *   - 設定変更の容易化
 */

#ifndef GAME_DEFS_H
#define GAME_DEFS_H

#include <stdint.h>

/* ゲーム状態の列挙型 */
typedef enum {
    GAME_STATE_MENU,        /**< メニュー画面 */
    GAME_STATE_PLAYING,     /**< ゲームプレイ中 */
    GAME_STATE_PAUSED,      /**< 一時停止中 */
    GAME_STATE_GAME_OVER,   /**< ゲームオーバー */
    GAME_STATE_NETWORKING,  /**< ネットワーク接続中 */
    GAME_STATE_EXIT         /**< ゲーム終了 */
} GameState;

/* ゲームモードの列挙型 */
typedef enum {
    GAME_MODE_SINGLE,       /**< シングルプレイヤー */
    GAME_MODE_AI_VS_PLAYER, /**< AI対戦 */
    GAME_MODE_MULTIPLAYER   /**< マルチプレイヤー */
} GameMode;

/* テトリミノタイプの列挙型 */
typedef enum {
    TETROMINO_I,            /**< I型テトリミノ */
    TETROMINO_O,            /**< O型テトリミノ */
    TETROMINO_S,            /**< S型テトリミノ */
    TETROMINO_Z,            /**< Z型テトリミノ */
    TETROMINO_J,            /**< J型テトリミノ */
    TETROMINO_L,            /**< L型テトリミノ */
    TETROMINO_T,            /**< T型テトリミノ */
    TETROMINO_COUNT         /**< テトリミノの種類数 */
} TetrominoType;

/* 回転方向の列挙型 */
typedef enum {
    ROTATE_CW,              /**< 時計回り */
    ROTATE_CCW              /**< 反時計回り */
} RotateDirection;

/* キーバインディング */
#define KEY_MOVE_LEFT    'A' /**< 左移動キー */
#define KEY_MOVE_RIGHT   'D' /**< 右移動キー */
#define KEY_ROTATE_CW    'W' /**< 時計回り回転キー */
#define KEY_ROTATE_CCW   'Q' /**< 反時計回り回転キー */
#define KEY_SOFT_DROP    'S' /**< ソフトドロップキー */
#define KEY_HARD_DROP    ' ' /**< ハードドロップキー (スペース) */
#define KEY_HOLD         'C' /**< ホールドキー */
#define KEY_PAUSE        'P' /**< 一時停止キー */
#define KEY_QUIT         'X' /**< 終了キー */
#define KEY_COUNT        9   /**< キーの総数 */

/* ゲームボードの定数 */
#define BOARD_WIDTH      10  /**< ボードの幅 (ブロック数) */
#define BOARD_HEIGHT     20  /**< ボードの高さ (ブロック数) */
#define BOARD_SIZE       (BOARD_WIDTH * BOARD_HEIGHT) /**< ボードの総セル数 */

/* ゲームタイミング定数 */
#define INITIAL_FALL_DELAY   1000 /**< 初期落下遅延 (ms) */
#define MIN_FALL_DELAY       100  /**< 最小落下遅延 (ms) */
#define LEVEL_SPEED_REDUCTION 50 /**< レベルごとの速度減少 (ms) */
#define LINES_PER_LEVEL      10  /**< レベルアップに必要なライン数 */

/* スコアリング定数 */
#define INITIAL_LEVEL     1  /**< 初期レベル */
#define SCORE_SINGLE      100 /**< 1ライン消去のスコア */
#define SCORE_DOUBLE      300 /**< 2ライン消去のスコア */
#define SCORE_TRIPLE      500 /**< 3ライン消去のスコア */
#define SCORE_TETRIS      800 /**< 4ライン消去のスコア */
#define SCORE_SOFT_DROP   1   /**< ソフトドロップ1ブロックごとのスコア */
#define SCORE_HARD_DROP   2   /**< ハードドロップ1ブロックごとのスコア */
#define SCORE_COMBO_BONUS 50  /**< コンボボーナス (連続ライン消去ごと) */

/* テトリミノ形状定義 */
#define TETROMINO_SIZE 4 /**< テトリミノのマトリックスサイズ (4x4) */

/**
 * @brief テトリミノ構造体
 * 
 * テトリミノの状態を保持します。
 */
typedef struct {
    TetrominoType type;          /**< テトリミノのタイプ */
    int matrix[TETROMINO_SIZE][TETROMINO_SIZE]; /**< 4x4回転マトリックス */
    int x;                       /**< ボード上のX位置 */
    int y;                       /**< ボード上のY位置 */
    int rotation;                /**< 現在の回転状態 (0-3) */
} Piece;

/**
 * @brief ゲームボード構造体
 * 
 * ゲームボードの状態を保持します。
 */
typedef struct {
    int width;                   /**< ボードの幅 */
    int height;                  /**< ボードの高さ */
    uint8_t* grid;               /**< ボードグリッドデータへのポインタ */
} Board;

/**
 * @brief スコア管理構造体
 * 
 * スコアとレベル情報を保持します。
 */
typedef struct {
    int score;                   /**< 現在のスコア */
    int level;                   /**< 現在のレベル */
    int lines_cleared;           /**< 消去したライン数 */
    int lines_since_last_level;  /**< 前回レベルアップからのライン数 */
    int combo_count;             /**< 現在のコンボ数 */
    int last_clear_type;         /**< 最後に消去したライン数 */
} ScoreCtx;

/**
 * @brief プレイヤー入力構造体
 * 
 * プレイヤーの入力状態を保持します。
 */
typedef struct {
    uint8_t keys[KEY_COUNT];     /**< 現在のキー状態 */
    uint8_t prev_keys[KEY_COUNT];/**< 前フレームのキー状態 */
} PlayerInput;

/**
 * @brief ゲームコンテキストのサブコンポーネント
 */
typedef struct {
    Board* board;               /**< ゲームボードインスタンス */
    Piece current_piece;        /**< 現在操作中のテトリミノ */
    Piece next_piece;           /**< 次のテトリミノ */
    ScoreCtx score;             /**< スコア管理コンテキスト */
    Timer timer;                /**< ゲームタイマー */
} GamePlayContext;

typedef struct {
    PlayerInput input;          /**< プレイヤー入力状態 */
} InputContext;

typedef struct {
    ServerContext server;       /**< サーバーコンテキスト */
    ClientContext client;       /**< クライアントコンテキスト */
    int player_id;              /**< プレイヤーID */
} NetworkContext;

/**
 * @brief ゲーム全体のコンテキスト構造体
 * 
 * ゲームの実行に必要な全ての状態を保持します。
 */
typedef struct {
    GameMode mode;              /**< 現在のゲームモード */
    GameState state;            /**< ゲームの実行状態 */
    GamePlayContext gameplay;   /**< ゲームプレイ関連コンテキスト */
    InputContext input;         /**< 入力関連コンテキスト */
    NetworkContext network;     /**< ネットワーク関連コンテキスト */
} GameContext;

#endif /* GAME_DEFS_H */