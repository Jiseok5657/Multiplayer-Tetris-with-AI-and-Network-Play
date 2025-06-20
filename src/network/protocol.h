/**
 * @file protocol.h
 * @brief ネットワーク通信プロトコル定義
 * 
 * このファイルはマルチプレイヤーモードで使用される
 * ネットワーク通信プロトコルを定義します。
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "game_defs.h"

/* ネットワークメッセージタイプ */
typedef enum {
    MSG_GAME_STATE,     /**< ゲーム状態更新 */
    MSG_PLAYER_INPUT,   /**< プレイヤー入力 */
    MSG_HEARTBEAT       /**< 接続維持 */
} MessageType;

/**
 * @brief メッセージヘッダ構造体
 */
typedef struct {
    uint32_t msg_type;  /**< メッセージタイプ */
    uint32_t msg_size;  /**< メッセージデータサイズ */
} MessageHeader;

/**
 * @brief ゲーム状態データ構造体
 */
typedef struct {
    float game_time;            /**< ゲーム経過時間 */
    int player_score;           /**< プレイヤースコア */
    uint8_t board[BOARD_SIZE];  /**< ボード状態 */
    TetrominoType next_piece;   /**< 次のテトリミノ */
} GameStateData;

/**
 * @brief プレイヤー入力データ構造体
 */
typedef struct {
    PlayerInput commands;       /**< 入力コマンド */
    float timestamp;            /**< 入力時刻 */
} PlayerInputData;

/**
 * @brief ネットワークメッセージ構造体
 */
typedef struct {
    MessageHeader header;       /**< メッセージヘッダ */
    union {
        GameStateData game_state;   /**< ゲーム状態データ */
        PlayerInputData player_input; /**< プレイヤー入力データ */
    } data;
} NetworkMessage;

#endif /* PROTOCOL_H */