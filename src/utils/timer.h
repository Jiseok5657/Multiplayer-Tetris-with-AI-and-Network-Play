/**
 * @file timer.h
 * @brief ゲームタイマー管理
 * 
 * このファイルはゲームの時間管理機能を宣言します。
 * 主な機能:
 *   - 高精度タイマーの初期化
 *   - フレームレート制御
 *   - ゲーム状態の時間管理
 *   - デルタタイム計算
 * 
 * 設計思想:
 *   - QueryPerformanceCounterを使用した高精度タイミング
 *   - フレームレート独立のゲームロジックを実現
 *   - 最小限のオーバーヘッド
 */

#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

/**
 * @brief ゲームタイマー構造体
 * 
 * ゲームの時間管理に必要な全ての状態を保持します。
 */
typedef struct {
    LARGE_INTEGER start_time;       /**< タイマー開始時刻 */
    LARGE_INTEGER current_time;     /**< 現在時刻 */
    LARGE_INTEGER previous_time;    /**< 前フレーム時刻 */
    LARGE_INTEGER frequency;        /**< タイマー周波数 */
    double delta_time;              /**< 前フレームからの経過時間(秒) */
    double elapsed_time;            /**< タイマー開始からの経過時間(秒) */
    double frame_delay;             /**< 目標フレーム間隔(秒) */
    double last_frame_time;         /**< 前フレームの終了時刻 */
} Timer;

/* 関数プロトタイプ宣言 */

/**
 * @brief タイマーを初期化する
 * @param timer 初期化するタイマー構造体
 * @param target_fps 目標フレームレート
 */
void timer_init(Timer* timer, int target_fps);

/**
 * @brief タイマーを更新する
 * @param timer 更新するタイマー構造体
 * 
 * デルタタイムと経過時間を計算します。
 * 各ゲームフレームの開始時に呼び出す必要があります。
 */
void timer_update(Timer* timer);

/**
 * @brief フレームレート制御を行う
 * @param timer タイマー構造体
 * 
 * 目標フレームレートを維持するために適切な時間だけスリープします。
 * ゲームフレームの終了時に呼び出す必要があります。
 */
void timer_regulate_framerate(Timer* timer);

/**
 * @brief 経過時間を取得する
 * @param timer タイマー構造体
 * @return タイマー開始からの経過時間(秒)
 */
double timer_get_elapsed(const Timer* timer);

/**
 * @brief デルタタイムを取得する
 * @param timer タイマー構造体
 * @return 前フレームからの経過時間(秒)
 */
double timer_get_delta(const Timer* timer);

/**
 * @brief 指定時間だけスリープする
 * @param ms スリープする時間(ミリ秒)
 */
void timer_delay(int ms);

#endif /* TIMER_H */