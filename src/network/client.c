/**
 * @file client.c
 * @brief TCPクライアント実装
 * 
 * このファイルはマルチプレイヤーテトリスゲームのクライアント機能を実装します。
 * 主な機能:
 *   - サーバーへの接続確立
 *   - プレイヤー入力の送信
 *   - ゲーム状態の受信と更新
 *   - 接続維持管理 (ハートビート)
 *   - リソース管理とクリーンアップ
 * 
 * 設計思想:
 *   - ノンブロッキングI/Oによる応答性の確保
 *   - シンプルな状態管理
 *   - 効率的なメッセージ処理
 *   - 堅牢なエラー処理
 */

#include "client.h"
#include "network_defs.h"
#include "protocol.h"
#include "error.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief クライアントを初期化する
 * 
 * @param ctx クライアントコンテキスト
 * @return int エラーコード (NET_OK: 成功)
 * 
 * 初期化処理:
 * 1. Winsockライブラリの初期化
 * 2. コンテキストのゼロ初期化
 * 3. 状態の設定
 */
int client_init(ClientContext* ctx) {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        LOG_ERROR("WSAStartupが失敗しました", WSAGetLastError());
        return ERR_NET_INIT_FAILED;
    }
    
    /* コンテキストをゼロ初期化 */
    memset(ctx, 0, sizeof(ClientContext));
    ctx->state = CLIENT_STATE_DISCONNECTED;
    ctx->socket = INVALID_SOCKET;
    
    LOG_DEBUG("クライアント初期化完了");
    return NET_OK;
}

/**
 * @brief サーバーに接続する
 * 
 * @param ctx クライアントコンテキスト
 * @param ip サーバーIPアドレス
 * @param port サーバーポート番号
 * @return int エラーコード (NET_OK: 成功)
 * 
 * 処理内容:
 * 1. ソケットの作成
 * 2. サーバーアドレスの設定
 * 3. 接続の試行
 * 4. ノンブロッキングモードの設定
 */
int client_connect(ClientContext* ctx, const char* ip, int port) {
    /* ソケットの作成 */
    ctx->socket = socket(ADDRESS_FAMILY, SOCKET_TYPE, PROTOCOL);
    if (ctx->socket == INVALID_SOCKET) {
        LOG_ERROR("ソケットの作成に失敗しました", WSAGetLastError());
        return ERR_NET_CONN_FAILED;
    }
    
    /* ノンブロッキングモードに設定 */
    u_long mode = 1;
    if (ioctlsocket(ctx->socket, FIONBIO, &mode) == SOCKET_ERROR) {
        LOG_WARNING("ノンブロッキングモードの設定に失敗しました", WSAGetLastError());
    }
    
    /* サーバーアドレス設定 */
    memset(&ctx->server_addr, 0, sizeof(ctx->server_addr));
    ctx->server_addr.sin_family = ADDRESS_FAMILY;
    ctx->server_addr.sin_port = htons(port);
    
    /* IPアドレスの変換 */
    if (inet_pton(ADDRESS_FAMILY, ip, &ctx->server_addr.sin_addr) <= 0) {
        LOG_ERROR("無効なIPアドレス: %s", ip);
        closesocket(ctx->socket);
        ctx->socket = INVALID_SOCKET;
        return ERR_NET_CONN_FAILED;
    }
    
    /* 接続試行 */
    ctx->state = CLIENT_STATE_CONNECTING;
    if (connect(ctx->socket, (struct sockaddr*)&ctx->server_addr, sizeof(ctx->server_addr)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            LOG_ERROR("サーバーへの接続に失敗しました", error);
            closesocket(ctx->socket);
            ctx->socket = INVALID_SOCKET;
            ctx->state = CLIENT_STATE_DISCONNECTED;
            return ERR_NET_CONN_FAILED;
        }
    }
    
    /* 接続状態を更新 */
    ctx->state = CLIENT_STATE_CONNECTED;
    ctx->last_heartbeat = time(NULL);
    ctx->player_id = -1;  /* サーバーから割り当てられるまで不明 */
    
    LOG_INFO("サーバーに接続中: %s:%d", ip, port);
    return NET_OK;
}

/**
 * @brief メッセージを送信する
 * 
 * @param ctx クライアントコンテキスト
 * @param msg 送信するメッセージ
 * @return int エラーコード (NET_OK: 成功)
 * 
 * 処理内容:
 * 1. メッセージのシリアライズ
 * 2. ソケットへの送信
 * 3. エラー処理
 */
int client_send_message(ClientContext* ctx, const NetworkMessage* msg) {
    char buffer[BUFFER_SIZE];
    int size = serialize_message(msg, buffer, BUFFER_SIZE);
    
    if (size <= 0) {
        LOG_ERROR("メッセージのシリアライズに失敗", 0);
        return ERR_NET_SEND_FAILED;
    }
    
    if (send(ctx->socket, buffer, size, 0) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            LOG_WARNING("送信がブロックされました");
            return NET_OK;  // 非致命的エラー
        }
        
        LOG_ERROR("サーバーへの送信に失敗", error);
        return ERR_NET_SEND_FAILED;
    }
    
    return NET_OK;
}

/**
 * @brief メッセージを受信する
 * 
 * @param ctx クライアントコンテキスト
 * @param msg [出力] 受信したメッセージ
 * @return int >0: 受信メッセージサイズ, 0: 受信データなし, <0: エラー
 * 
 * 処理内容:
 * 1. ソケットの読み取り準備チェック (select)
 * 2. データの受信
 * 3. メッセージのデシリアライズ
 * 4. ハートビートの更新
 */
int client_receive_message(ClientContext* ctx, NetworkMessage* msg) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(ctx->socket, &read_fds);
    
    /* 非ブロッキングチェック (5msタイムアウト) */
    struct timeval timeout = {0, 5000};
    int activity = select(0, &read_fds, NULL, NULL, &timeout);
    
    if (activity == SOCKET_ERROR) {
        LOG_ERROR("ソケット選択エラー", WSAGetLastError());
        return ERR_NET_RECV_FAILED;
    }
    
    /* 受信データなし */
    if (activity == 0 || !FD_ISSET(ctx->socket, &read_fds)) {
        return 0;
    }
    
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(ctx->socket, buffer, BUFFER_SIZE, 0);
    
    if (bytes_received <= 0) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            return 0;  // データなし
        }
        
        LOG_DEBUG("サーバーから切断されました (エラー: %d)", error);
        return ERR_NET_DISCONNECTED;
    }
    
    /* メッセージのデシリアライズ */
    int result = deserialize_message(buffer, bytes_received, msg);
    if (result <= 0) {
        LOG_WARNING("無効なメッセージを受信");
        return ERR_NET_INVALID_MSG;
    }
    
    /* メッセージの検証 */
    if (!validate_message(msg)) {
        LOG_WARNING("不正なメッセージフォーマット");
        return ERR_NET_INVALID_MSG;
    }
    
    /* ハートビートの更新 */
    if (msg->header.msg_type == MSG_HEARTBEAT) {
        ctx->last_heartbeat = time(NULL);
        LOG_DEBUG("ハートビート受信");
    }
    
    return result;
}

/**
 * @brief 接続状態を確認する
 * 
 * @param ctx クライアントコンテキスト
 * @return int エラーコード (NET_OK: 接続良好)
 * 
 * 処理内容:
 * 1. 最終ハートビート時刻のチェック
 * 2. タイムアウトの場合はハートビート送信
 * 3. 接続喪失の検出
 */
int client_check_connection(ClientContext* ctx) {
    time_t current_time = time(NULL);
    double seconds_since = difftime(current_time, ctx->last_heartbeat);
    
    /* ハートビート間隔の3倍を超えたら接続チェック */
    if (seconds_since > (HEARTBEAT_INTERVAL * 3) / 1000.0) {
        LOG_WARNING("サーバー接続が不安定 (最終確認: %.0f秒前)", seconds_since);
        return ERR_NET_TIMEOUT;
    }
    
    /* ハートビート間隔の2倍を超えたらハートビート送信 */
    if (seconds_since > (HEARTBEAT_INTERVAL * 2) / 1000.0) {
        NetworkMessage heartbeat;
        heartbeat.header = create_header(MSG_HEARTBEAT, 0);
        
        if (client_send_message(ctx, &heartbeat) == NET_OK) {
            LOG_DEBUG("ハートビートを送信");
            ctx->last_heartbeat = current_time;
        } else {
            LOG_ERROR("ハートビート送信に失敗", 0);
            return ERR_NET_SEND_FAILED;
        }
    }
    
    return NET_OK;
}

/**
 * @brief クライアントリソースを解放する
 * 
 * @param ctx クライアントコンテキスト
 * 
 * 処理内容:
 * 1. ソケットの切断
 * 2. Winsockライブラリの終了
 * 3. 状態のリセット
 */
void client_cleanup(ClientContext* ctx) {
    LOG_DEBUG("クライアントリソースの解放開始");
    
    if (ctx->socket != INVALID_SOCKET) {
        /* 接続を安全に切断 */
        if (ctx->state == CLIENT_STATE_CONNECTED) {
            LOG_DEBUG("サーバーへの接続を切断");
            
            /* 切断通知を送信 */
            NetworkMessage disconnect_msg;
            disconnect_msg.header = create_header(MSG_DISCONNECT, 0);
            client_send_message(ctx, &disconnect_msg);
            
            /* ソケットシャットダウン */
            shutdown(ctx->socket, SD_BOTH);
        }
        
        /* ソケットを閉じる */
        closesocket(ctx->socket);
        ctx->socket = INVALID_SOCKET;
    }
    
    /* Winsock終了 */
    WSACleanup();
    
    ctx->state = CLIENT_STATE_DISCONNECTED;
    LOG_INFO("クライアントリソースの解放完了");
}