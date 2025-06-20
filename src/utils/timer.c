/**
 * @file timer.c
 * @brief ゲームタイマー実装
 * 
 * このファイルはゲームの時間管理機能を実装します。
 * 主な機能:
 *   - 高精度タイマーの管理
 *   - フレームレート制御
 *   - デルタタイム計算
 *   - 時間ベースのゲームイベント処理
 * 
 * 設計思想:
 *   - QueryPerformanceCounterによるマイクロ秒精度のタイミング
 *   - フレームレート独立のゲームロジック実現
 *   - 最小限のオーバーヘッド
 */

#include "timer.h"
#include <windows.h>

/**
 * @brief タイマーを初期化する
 * 
 * @param timer 初期化するタイマー構造体
 * @param target_fps 目標フレームレート
 * 
 * 処理内容:
 * 1. パフォーマンスカウンタの周波数を取得
 * 2. 開始時刻を記録
 * 3. 目標フレーム時間を計算
 */
void timer_init(Timer* timer, int target_fps) {
    QueryPerformanceFrequency(&timer->frequency);
    QueryPerformanceCounter(&timer->start_time);
    
    timer->current_time = timer->start_time;
    timer->previous_time = timer->start_time;
    timer->delta_time = 0.0;
    timer->elapsed_time = 0.0;
    
    // 目標フレーム時間の計算 (秒単位)
    timer->frame_delay = (target_fps > 0) ? 1.0 / target_fps : 0.0;
    timer->last_frame_time = 0.0;
}

/**
 * @brief タイマーを更新する
 * 
 * @param timer 更新するタイマー構造体
 * 
 * 処理内容:
 * 1. 現在時刻を取得
 * 2. 前フレームからの経過時間(デルタタイム)を計算
 * 3. 経過時間を更新
 */
void timer_update(Timer* timer) {
    timer->previous_time = timer->current_time;
    QueryPerformanceCounter(&timer->current_time);
    
    // デルタタイム計算 (秒単位)
    timer->delta_time = (double)(timer->current_time.QuadPart - timer->previous_time.QuadPart) 
                      / timer->frequency.QuadPart;
    
    // 経過時間更新
    timer->elapsed_time = (double)(timer->current_time.QuadPart - timer->start_time.QuadPart) 
                         / timer->frequency.QuadPart;
}

/**
 * @brief フレームレート制御を行う
 * 
 * @param timer タイマー構造体
 * 
 * 処理内容:
 * 1. 現在フレームの処理時間を計算
 * 2. 目標フレーム時間まで不足している時間を計算
 * 3. 必要に応じてスリープを実行
 */
void timer_regulate_framerate(Timer* timer) {
    if (timer->frame_delay <= 0.0) return;
    
    // 現在のフレーム処理時間を計算
    LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    double frame_time = (double)(end_time.QuadPart - timer->current_time.QuadPart) 
                      / timer->frequency.QuadPart;
    
    // 目標フレーム時間までにスリープすべき時間を計算
    double sleep_time = timer->frame_delay - frame_time;
    
    // スリープが必要な場合
    if (sleep_time > 0.0) {
        // ミリ秒に変換してスリープ
        DWORD sleep_ms = (DWORD)(sleep_time * 1000.0);
        if (sleep_ms > 0) {
            Sleep(sleep_ms);
        }
    }
    
    timer->last_frame_time = timer->elapsed_time;
}

/**
 * @brief 経過時間を取得する
 * 
 * @param timer タイマー構造体
 * @return タイマー開始からの経過時間(秒)
 */
double timer_get_elapsed(const Timer* timer) {
    return timer->elapsed_time;
}

/**
 * @brief デルタタイムを取得する
 * 
 * @param timer タイマー構造体
 * @return 前フレームからの経過時間(秒)
 */
double timer_get_delta(const Timer* timer) {
    return timer->delta_time;
}

/**
 * @brief 指定時間だけスリープする
 * 
 * @param ms スリープする時間(ミリ秒)
 */
void timer_delay(int ms) {
    if (ms > 0) {
        Sleep(ms);
    }
}