/**
 * @file render.h
 * @brief ゲームレンダリング機能の宣言
 * 
 * このファイルはテトリスゲームの画面表示に関する関数とデータ構造を宣言します。
 * 主な機能:
 *   - レンダリングシステムの初期化と終了
 *   - ゲーム画面全体のレンダリング
 *   - ゲームボードの描画
 *   - テトリミノの表示
 *   - スコアとゲーム情報の表示
 * 
 * 設計思想:
 *   - コンソールベースの軽量レンダリング
 *   - ダブルバッファリングによるちらつき防止
 *   - カラーを使った直感的な表示
 *   - モジュール化された描画機能
 */

#ifndef RENDER_H
#define RENDER_H

#include "game_defs.h"
#include "board.h"
#include "piece.h"
#include "score.h"

/* 関数プロトタイプ宣言 */

/**
 * @brief レンダリングシステムを初期化する
 * @return 0:成功, -1:失敗
 */
int render_init();

/**
 * @brief ゲーム画面全体をレンダリングする
 * 
 * @param board ゲームボード
 * @param current_piece 現在操作中のテトリミノ
 * @param next_piece 次のテトリミノ
 * @param score スコアコンテキスト
 * @param state 現在のゲーム状態
 * @param player_id プレイヤーID
 */
void render_game(Board* board, Piece* current_piece, Piece* next_piece, 
                ScoreCtx* score, GameState state, int player_id);

/**
 * @brief ゲームボードをレンダリングする
 * 
 * @param board ゲームボード
 * @param piece 現在のテトリミノ
 */
void render_board(Board* board, Piece* piece);

/**
 * @brief 次のテトリミノを表示する
 * 
 * @param piece 次のテトリミノ
 */
void render_next_piece(Piece* piece);

/**
 * @brief レンダリングシステムを終了する
 */
void render_cleanup();

#endif /* RENDER_H */