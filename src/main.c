/**
 * @file main.c
 * @brief ゲームメインループとエントリーポイント
 * 
 * 主な機能:
 *   - コマンドライン引数の解析
 *   - ゲームモードの選択 (シングルプレイヤー, AI対戦, マルチプレイヤー)
 *   - ゲームコンテキストの初期化
 *   - メインゲームループの実行
 *   - ネットワーク通信の統合
 *   - リソース管理とクリーンアップ
 * 
 * 設計思想:
 *   - ステートマシンベースのゲームループ
 *   - モジュール間の疎結合を維持
 *   - フレームレート制御による安定したゲームプレイ
 *   - エラー処理とリソースリーク防止
 */

#include "game_defs.h"
#include "board.h"
#include "piece.h"
#include "score.h"
#include "input.h"
#include "render.h"
#include "timer.h"
#include "server.h"
#include "client.h"
#include "protocol.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 関数プロトタイプ宣言 */
void parse_arguments(int argc, char* argv[], GameMode* mode, char** server_ip);
int init_game(GameContext* ctx, GameMode mode, const char* server_ip);
void run_game_loop(GameContext* ctx);
void update_game_state(GameContext* ctx);
void handle_network_communication(GameContext* ctx);
void cleanup_game(GameContext* ctx);
void handle_single_player(GameContext* ctx);
void handle_ai_vs_player(GameContext* ctx);
void handle_multiplayer(GameContext* ctx);

/**
 * @brief プログラムのメインエントリーポイント
 * 
 * @param argc コマンドライン引数の数
 * @param argv コマンドライン引数の配列
 * @return int 終了ステータス (0: 正常終了)
 * 
 * 処理フロー:
 * 1. コマンドライン引数の解析
 * 2. ゲームコンテキストの初期化
 * 3. メインゲームループの実行
 * 4. リソースのクリーンアップ
 */
int main(int argc, char* argv[]) {
    GameContext ctx;
    GameMode mode;
    char* server_ip = NULL;
    
    /* コマンドライン引数の解析 */
    parse_arguments(argc, argv, &mode, &server_ip);
    
    /* ゲーム初期化 */
    if (init_game(&ctx, mode, server_ip) != 0) {
        fprintf(stderr, "ゲームの初期化に失敗しました\n");
        return EXIT_FAILURE;
    }
    
    /* メインゲームループ実行 */
    run_game_loop(&ctx);
    
    /* ゲームリソースの解放 */
    cleanup_game(&ctx);
    
    return EXIT_SUCCESS;
}

/**
 * @brief コマンドライン引数を解析
 * 
 * @param argc 引数の数
 * @param argv 引数配列
 * @param mode [出力] 解析されたゲームモード
 * @param server_ip [出力] サーバーIPアドレス (マルチプレイヤーモード時)
 * 
 * サポートされる引数:
 *   --multiplayer [IP] : マルチプレイヤーモード (IP指定時はクライアント)
 *   --ai               : AI対戦モード
 *   --help             : ヘルプ表示
 * 
 * デフォルト: シングルプレイヤーモード
 */
void parse_arguments(int argc, char* argv[], GameMode* mode, char** server_ip) {
    /* デフォルト値設定 */
    *mode = GAME_MODE_SINGLE;
    *server_ip = NULL;
    
    /* 引数解析 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--multiplayer") == 0) {
            *mode = GAME_MODE_MULTIPLAYER;
            
            /* サーバーIPアドレスの指定があるか確認 */
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                *server_ip = argv[i + 1];
                i++;
            }
        }
        else if (strcmp(argv[i], "--ai") == 0) {
            *mode = GAME_MODE_AI_VS_PLAYER;
        }
        else if (strcmp(argv[i], "--help") == 0) {
            printf("使用法: %s [オプション]\n", argv[0]);
            printf("オプション:\n");
            printf("  --multiplayer [サーバーIP]  マルチプレイヤーモード (IP未指定でサーバー)\n");
            printf("  --ai                        AI対戦モード\n");
            printf("  --help                      ヘルプを表示\n");
            exit(EXIT_SUCCESS);
        }
    }
}

/**
 * @brief ゲームコンテキストを初期化
 * 
 * @param ctx 初期化するゲームコンテキスト
 * @param mode ゲームモード
 * @param server_ip サーバーIPアドレス (クライアントモード時)
 * @return int 0:成功, -1:失敗
 * 
 * 初期化処理:
 * 1. メモリのゼロ初期化
 * 2. ゲームボードの作成
 * 3. スコアシステムの初期化
 * 4. ゲームタイマーの初期化
 * 5. テトリミノの初期生成
 * 6. ネットワークモジュールの初期化 (マルチプレイヤーモード時)
 * 7. レンダリングシステムの初期化
 * 8. 入力システムの初期化
 */
int init_game(GameContext* ctx, GameMode mode, const char* server_ip) {
    /* コンテキストをゼロ初期化 */
    
    /* ゲームボードの初期化 */
    
    /* スコアシステムの初期化 */

    /* タイマーの初期化 */

    /* 最初のテトリミノを生成 */
    
    /* ネットワークの初期化 (マルチプレイヤーモード時) */
    switch (mode) {
        case GAME_MODE_MULTIPLAYER:
            if (server_ip == NULL) {
                /* サーバーモード */
                LOG_INFO("サーバーモードで起動 (ポート: %d)", DEFAULT_PORT);
                if (server_init(&ctx->network.server, DEFAULT_PORT) != NET_OK) {
                    LOG_ERROR("サーバーの初期化に失敗しました", ERR_NET_INIT_FAILED);
                    return -1;
                }
                if (server_start(&ctx->network.server) != NET_OK) {
                    LOG_ERROR("サーバーの起動に失敗しました", ERR_NET_INIT_FAILED);
                    return -1;
                }
                ctx->network.player_id = 0;  /* サーバーはプレイヤー0 */
            } else {
                /* クライアントモード */
                LOG_INFO("クライアントモードで起動 (サーバー: %s:%d)", server_ip, DEFAULT_PORT);
                if (client_init(&ctx->network.client) != NET_OK) {
                    LOG_ERROR("クライアントの初期化に失敗しました", ERR_NET_INIT_FAILED);
                    return -1;
                }
                if (client_connect(&ctx->network.client, server_ip, DEFAULT_PORT) != NET_OK) {
                    LOG_ERROR("サーバーへの接続に失敗しました", ERR_NET_CONN_FAILED);
                    return -1;
                }
                ctx->network.player_id = 1;  /* クライアントはプレイヤー1 */
            }
            break;
            
        case GAME_MODE_AI_VS_PLAYER:
            LOG_INFO("AI対戦モードで起動");
            /* TODO: AIの初期化 */
            break;
            
        case GAME_MODE_SINGLE:
        default:
            LOG_INFO("シングルプレイヤーモードで起動");
            /* ネットワークを使用しない */
            break;
    }
    
    /* レンダリングシステムの初期化 */
    if (render_init() != 0) {
        LOG_ERROR("レンダリングシステムの初期化に失敗しました", ERROR_RENDERING);
        return -1;
    }
    
    /* 入力システムの初期化 */
    input_init();
    
    LOG_INFO("ゲーム初期化完了");
    return 0;
}

/**
 * @brief メインゲームループを実行
 * 
 * @param ctx ゲームコンテキスト
 * 
 * ゲームループのステップ:
 * 1. 入力処理
 * 2. 一時停止状態のチェック
 * 3. ゲームモード別の更新処理
 * 4. ゲーム状態の更新
 * 5. レンダリング
 * 6. フレームレート制御
 * 
 * フレームレート: 約60FPS (16ms遅延)
 */
void run_game_loop(GameContext* ctx) {
    LOG_DEBUG("メインゲームループ開始");
    
    while (ctx->state != GAME_STATE_EXIT) {
        /* 入力処理 */
        input_poll(&ctx->input.input);
        
        /* 終了処理 */
        if (ctx->input.input.keys[KEY_QUIT] && !ctx->input.input.prev_keys[KEY_QUIT]) {
            ctx->state = GAME_STATE_EXIT;
            LOG_INFO("ゲーム終了");
            break;
        }
        
        /* 一時停止処理 */
        if (ctx->input.input.keys[KEY_PAUSE] && !ctx->input.input.prev_keys[KEY_PAUSE]) {
            ctx->state = (ctx->state == GAME_STATE_PLAYING) ? 
                         GAME_STATE_PAUSED : GAME_STATE_PLAYING;
            LOG_DEBUG("ゲーム状態変更: %s", 
                     (ctx->state == GAME_STATE_PAUSED) ? "一時停止" : "再開");
            timer_delay(200);  /* トグル防止 */
        }
        
        /* 一時停止中は更新処理をスキップ */
        if (ctx->state == GAME_STATE_PAUSED) {
            timer_delay(100);
            continue;
        }
        
        /* ゲームオーバー状態のチェック */
        if (ctx->state == GAME_STATE_GAME_OVER) {
            /* TODO: リスタート処理 */
            timer_delay(100);
            continue;
        }
        
        /* モード別ゲーム処理 */
        switch (ctx->mode) {
            case GAME_MODE_SINGLE:
                handle_single_player(ctx);
                break;
                
            case GAME_MODE_AI_VS_PLAYER:
                handle_ai_vs_player(ctx);
                break;
                
            case GAME_MODE_MULTIPLAYER:
                handle_multiplayer(ctx);
                break;
        }
        
        /* レンダリング */
        render_game(ctx->gameplay.board, 
                   &ctx->gameplay.current_piece, 
                   &ctx->gameplay.next_piece, 
                   &ctx->gameplay.score, 
                   ctx->state, 
                   ctx->network.player_id);
        
        /* フレームレート制御 (約60FPS) */
        timer_delay(16);
    }
    
    LOG_DEBUG("メインゲームループ終了");
}

/**
 * @brief シングルプレイヤーモードの処理
 * 
 * @param ctx ゲームコンテキスト
 * 
 * 処理内容:
 * 1. プレイヤー入力の処理
 * 2. ゲーム状態の更新
 */
void handle_single_player(GameContext* ctx) {
    /* 入力に基づいたテトリミノ操作 */
    handle_player_input(ctx, ctx->network.player_id);
    
    /* ゲーム状態の更新 */
    update_game_state(ctx);
}

/**
 * @brief AI対戦モードの処理
 * 
 * @param ctx ゲームコンテキスト
 * 
 * 処理内容:
 * 1. プレイヤー入力の処理
 * 2. AIの入力処理 (TODO)
 * 3. ゲーム状態の更新
 */
void handle_ai_vs_player(GameContext* ctx) {
    /* プレイヤー入力処理 */
    handle_player_input(ctx, ctx->network.player_id);
    
    /* AI LOGIC NOT IMPLEMENTED (Version 1.0) */
    LOG_WARNING("AI対戦モードは今後のバージョンで実装予定");
    
    /* ゲーム状態の更新 */
    update_game_state(ctx);
}

/**
 * @brief マルチプレイヤーモードの処理
 * 
 * @param ctx ゲームコンテキスト
 * 
 * 処理内容:
 * 1. ネットワーク通信の処理
 * 2. ローカルプレイヤーの入力処理
 * 3. ゲーム状態の更新
 */
void handle_multiplayer(GameContext* ctx) {
    /* ネットワーク通信の処理 */
    handle_network_communication(ctx);
    
    /* ローカルプレイヤーの入力処理 */
    handle_player_input(ctx, ctx->network.player_id);
    
    /* ゲーム状態の更新 */
    update_game_state(ctx);
}

/**
 * @brief ゲーム状態を更新
 * 
 * @param ctx ゲームコンテキスト
 * 
 * 更新処理:
 * 1. テトリミノの自動落下 (タイマーベース)
 * 2. ブロックの固定とライン消去
 * 3. スコアの更新
 * 4. ゲームオーバーチェック
 * 5. 新しいテトリミノの生成
 * 6. レベルに応じた速度調整
 */
void update_game_state(GameContext* ctx) {
    Board* board = ctx->gameplay.board;
    Piece* current_piece = &ctx->gameplay.current_piece;
    Piece* next_piece = &ctx->gameplay.next_piece;
    ScoreCtx* score = &ctx->gameplay.score;
    Timer* timer = &ctx->gameplay.timer;
    
    /* テトリミノの自動落下チェック */
    if (timer_check(timer)) {
        if (!piece_move_down(current_piece, board)) {
            /* ブロックを固定 */
            board_place_piece(board, current_piece);
            
            /* ライン消去とスコア更新 */
            int lines_cleared = board_clear_lines(board);
            if (lines_cleared > 0) {
                score_update_lines(score, lines_cleared);
                LOG_DEBUG("%d ライン消去! スコア: %d", lines_cleared, score->score);
            }
            
            /* ゲームオーバーチェック (ブロックが上部に到達) */
            if (board->grid[0][4] != 0 || board->grid[0][5] != 0) {
                ctx->state = GAME_STATE_GAME_OVER;
                LOG_INFO("ゲームオーバー! 最終スコア: %d", score->score);
                return;
            }
            
            /* 新しいテトリミノを生成 */
            *current_piece = *next_piece;
            *next_piece = piece_spawn_at_top();
            
            /* 新しいテトリミノが即時に衝突するかチェック (ゲームオーバー) */
            if (board_check_collision(current_piece, board)) {
                ctx->state = GAME_STATE_GAME_OVER;
                LOG_INFO("ゲームオーバー! 新しいブロックが配置不可");
                return;
            }
        }
        timer_reset(timer);
    }
    
    /* レベルに基づく速度調整 */
    int new_delay = INITIAL_FALL_DELAY - (score->level * LEVEL_SPEED_REDUCTION);
    if (new_delay < MIN_FALL_DELAY) new_delay = MIN_FALL_DELAY;
    timer_set_delay(timer, new_delay);
}

/**
 * @brief ネットワーク通信を処理
 * 
 * @param ctx ゲームコンテキスト
 * 
 * サーバーモード時の処理:
 * 1. クライアント接続の受付
 * 2. クライアントメッセージの処理
 * 3. ゲーム状態のブロードキャスト
 * 
 * クライアントモード時の処理:
 * 1. 入力メッセージの送信
 * 2. ゲーム状態の受信と更新
 * 
 * 共通処理:
 * 1. ハートビートチェック
 */
void handle_network_communication(GameContext* ctx) {
    /* サーバーモードの処理 */
    if (ctx->network.server.state == SERVER_STATE_LISTENING) {
        /* クライアント接続の受付 */
        server_accept_client(&ctx->network.server);
    }
    
    if (ctx->network.server.state == SERVER_STATE_RUNNING) {
        /* クライアントメッセージの処理 */
        int processed = server_handle_messages(&ctx->network.server);
        if (processed > 0) {
            LOG_DEBUG("処理したメッセージ: %d件", processed);
        }
        
        /* ゲーム状態のブロードキャスト */
        NetworkMessage state_msg;
        state_msg.header = create_header(MSG_GAME_STATE, sizeof(GameStateData));
        
        GameStateData* state_data = &state_msg.data.game_state;
        state_data->game_time = timer_get_elapsed(&ctx->gameplay.timer);
        state_data->player_score = ctx->gameplay.score.score;
        memcpy(state_data->board, ctx->gameplay.board->grid, BOARD_WIDTH * BOARD_HEIGHT);
        state_data->next_piece = ctx->gameplay.next_piece.type;
        
        if (server_broadcast(&ctx->network.server, &state_msg) == NET_OK) {
            LOG_DEBUG("ゲーム状態をブロードキャスト");
        }
    }
    
    /* クライアントモードの処理 */
    if (ctx->network.client.state == CLIENT_STATE_CONNECTED) {
        /* 入力メッセージの送信 */
        NetworkMessage input_msg;
        input_msg.header = create_header(MSG_PLAYER_INPUT, sizeof(PlayerInputData));
        
        PlayerInputData* input_data = &input_msg.data.player_input;
        memcpy(&input_data->commands, &ctx->input.input, sizeof(PlayerInput));
        input_data->timestamp = timer_get_elapsed(&ctx->gameplay.timer);
        
        if (client_send_message(&ctx->network.client, &input_msg) == NET_OK) {
            LOG_DEBUG("入力メッセージを送信");
        }
        
        /* ゲーム状態の受信 */
        NetworkMessage state_msg;
        int result = client_receive_message(&ctx->network.client, &state_msg);
        if (result > 0) {
            if (state_msg.header.msg_type == MSG_GAME_STATE) {
                /* ゲーム状態の更新 */
                GameStateData* state_data = &state_msg.data.game_state;
                ctx->gameplay.score.score = state_data->player_score;
                memcpy(ctx->gameplay.board->grid, state_data->board, BOARD_WIDTH * BOARD_HEIGHT);
                ctx->gameplay.next_piece.type = (TetrominoType)state_data->next_piece;
                LOG_DEBUG("ゲーム状態を更新");
            }
        } else if (result == ERR_NET_DISCONNECTED) {
            LOG_ERROR("サーバーから切断されました", ERR_NET_DISCONNECTED);
            ctx->state = GAME_STATE_GAME_OVER;
        }
    }
    
    /* 接続確認 */
    if (ctx->network.server.state == SERVER_STATE_RUNNING) {
        int disconnected = server_check_heartbeats(&ctx->network.server);
        if (disconnected > 0) {
            LOG_WARNING("%d クライアントが切断されました", disconnected);
        }
    }
    if (ctx->network.client.state == CLIENT_STATE_CONNECTED) {
        if (client_check_connection(&ctx->network.client) != NET_OK) {
            LOG_ERROR("サーバー接続がタイムアウトしました", ERR_NET_TIMEOUT);
            ctx->state = GAME_STATE_GAME_OVER;
        }
    }
}

/**
 * @brief ゲームリソースを解放
 * 
 * @param ctx ゲームコンテキスト
 * 
 * 解放処理:
 * 1. ゲームボードの解放
 * 2. ネットワークリソースの解放
 * 3. レンダリングシステムの終了
 * 4. 入力システムの終了
 * 
 * 注意: 初期化の逆順で解放を行う
 */
void cleanup_game(GameContext* ctx) {
    LOG_DEBUG("ゲームリソースの解放開始");
    
    /* ボードの解放 */
    if (ctx->gameplay.board) {
        board_destroy(ctx->gameplay.board);
        ctx->gameplay.board = NULL;
        LOG_DEBUG("ゲームボードを解放");
    }
    
    /* ネットワークリソースの解放 */
    if (ctx->network.server.state != SERVER_STATE_SHUTDOWN) {
        server_cleanup(&ctx->network.server);
        LOG_DEBUG("サーバーリソースを解放");
    }
    if (ctx->network.client.state != CLIENT_STATE_DISCONNECTED) {
        client_cleanup(&ctx->network.client);
        LOG_DEBUG("クライアントリソースを解放");
    }
    
    /* レンダリングシステムの終了 */
    render_cleanup();
    LOG_DEBUG("レンダリングシステムを終了");
    
    /* 入力システムの終了 */
    input_cleanup();
    LOG_DEBUG("入力システムを終了");
    
    LOG_INFO("ゲームリソースの解放完了");
}