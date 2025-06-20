/**
 * @file input.h
 * @brief 入力処理機能の宣言
 * 
 */

#ifndef INPUT_H
#define INPUT_H

#include "game_defs.h"

/**
 * @brief 入力システムを初期化する
 */
void input_init();

/**
 * @brief 入力状態を更新する
 * @param input 入力状態構造体
 */
void input_poll(PlayerInput* input);

/**
 * @brief 入力システムを終了する
 */
void input_cleanup();

/**
 * @brief プレイヤー入力を処理する
 * @param ctx ゲームコンテキスト
 * @param player_id プレイヤーID
 */
void handle_player_input(GameContext* ctx, int player_id);

#endif /* INPUT_H */