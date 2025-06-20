/**
 * @file server.c
 * @brief TCPサーバー実装
 * 
 * このファイルはマルチプレイヤーテトリスゲームのサーバー機能を実装します。
 * 主な機能:
 *   - サーバーの初期化と起動
 *   - クライアント接続の受付
 *   - ネットワークメッセージの処理
 *   - ゲーム状態のブロードキャスト
 *   - 接続維持管理 (ハートビート)
 *   - リソース管理とクリーンアップ
 * 
 * 設計思想:
 *   - ノンブロッキングI/Oによる効率的な多重化
 *   - スレッドを使用しないシングルスレッド設計
 *   - 最小限のメモリ使用
 *   - 堅牢なエラー処理
 */

#include "server.h"
#include "network_defs.h"
#include "protocol.h"
#include "error.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief サーバーを初期化する
 * 
 * @param ctx サーバーコンテキスト
 * @param port 使用するポート番号
 * @return int エラーコード (NET_OK: 成功)
 * 
 * 初期化処理:
 * 1. Winsockライブラリの初期化
 * 2. サーバーソケットの作成
 * 3. サーバーアドレスの設定
 * 4. ソケットのバインド
 */
int server_init(ServerContext* ctx, int port) {
    WSADATA wsa_data;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        LOG_ERROR("WSAStartupが失敗しました", WSAGetLastError());
        return ERR_NET_INIT_FAILED;
    }

    /* サーバーコンテキストをゼロ初期化 */
    memset(ctx, 0, sizeof(ServerContext));
    ctx->state = SERVER_STATE_IDLE;
    
    /* 待機ソケットの作成 */
    ctx->listen_socket = socket(ADDRESS_FAMILY, SOCKET_TYPE, PROTOCOL);
    if (ctx->listen_socket == INVALID_SOCKET) {
        LOG_ERROR("ソケットの作成に失敗しました", WSAGetLastError());
        WSACleanup();
        return ERR_NET_INIT_FAILED;
    }

    /* ソケットをノンブロッキングモードに設定 */
    u_long mode = 1;
    if (ioctlsocket(ctx->listen_socket, FIONBIO, &mode) == SOCKET_ERROR) {
        LOG_ERROR("ノンブロッキングモードの設定に失敗しました", WSAGetLastError());
        closesocket(ctx->listen_socket);
        WSACleanup();
        return ERR_NET_INIT_FAILED;
    }

    /* アドレス設定 */
    INIT_ADDR(ctx->addr, port);
    
    /* ソケットのバインド */
    if (bind(ctx->listen_socket, (struct sockaddr*)&ctx->addr, sizeof(ctx->addr)) == SOCKET_ERROR) {
        LOG_ERROR("ソケットのバインドに失敗しました", WSAGetLastError());
        closesocket(ctx->listen_socket);
        WSACleanup();
        return ERR_NET_INIT_FAILED;
    }
    
    LOG_INFO("サーバー初期化完了 (ポート: %d)", port);
    return NET_OK;
}

/**
 * @brief サーバーを起動する
 * 
 * @param ctx サーバーコンテキスト
 * @return int エラーコード (NET_OK: 成功)
 * 
 * 処理内容:
 * 1. 接続待機状態に移行
 * 2. 最大接続数を設定
 */
int server_start(ServerContext* ctx) {
    if (listen(ctx->listen_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        LOG_ERROR("接続待機状態への移行に失敗しました", WSAGetLastError());
        return ERR_NET_INIT_FAILED;
    }
    
    ctx->state = SERVER_STATE_LISTENING;
    LOG_INFO("サーバー起動完了、クライアント接続待機中");
    return NET_OK;
}

/**
 * @brief クライアント接続を受け付ける
 * 
 * @param ctx サーバーコンテキスト
 * @return int クライアントID (負の値: エラー)
 * 
 * 処理内容:
 * 1. 保留中の接続があるか確認
 * 2. 新しいクライアントソケットを受け付け
 * 3. クライアント情報を登録
 * 4. 接続数を更新
 */
int server_accept_client(ServerContext* ctx) {
    if (ctx->client_count >= MAX_CLIENTS) {
        LOG_WARNING("クライアント数が上限に達しています (最大: %d)", MAX_CLIENTS);
        return ERR_NET_SERVER_FULL;
    }
    
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    SOCKET client_socket = accept(ctx->listen_socket, (struct sockaddr*)&client_addr, &addr_len);
    
    if (client_socket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            LOG_ERROR("接続の受付に失敗しました", error);
            return ERR_NET_CONN_FAILED;
        }
        return -1;  // 保留中の接続なし
    }
    
    /* 新しいクライアント接続を追加 */
    int client_id = ctx->client_count;
    ClientConnection* client = &ctx->clients[client_id];
    
    client->socket = client_socket;
    client->addr = client_addr;
    client->id = client_id;
    client->is_connected = 1;
    client->last_heartbeat = time(NULL);
    
    ctx->client_count++;
    
    /* ノンブロッキングモードに設定 */
    u_long mode = 1;
    if (ioctlsocket(client_socket, FIONBIO, &mode) == SOCKET_ERROR) {
        LOG_WARNING("クライアントソケットのノンブロッキング設定に失敗しました", WSAGetLastError());
    }
    
    LOG_INFO("クライアント %d 接続: %s:%d", 
            client_id, 
            inet_ntoa(client_addr.sin_addr), 
            ntohs(client_addr.sin_port));
    
    /* サーバー状態を更新 */
    if (ctx->state == SERVER_STATE_LISTENING) {
        ctx->state = SERVER_STATE_RUNNING;
    }
    
    return client_id;
}

/**
 * @brief クライアントメッセージを処理する
 * 
 * @param ctx サーバーコンテキスト
 * @return int 処理したメッセージ数 (負の値: エラー)
 * 
 * 処理内容:
 * 1. select()で読み取り可能なソケットを検出
 * 2. 各クライアントからデータを受信
 * 3. メッセージをデシリアライズ
 * 4. メッセージタイプに基づいて処理
 * 
 * 最適化: 10msタイムアウトでCPU使用率を抑制
 */
int server_handle_messages(ServerContext* ctx) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    
    /* ソケットセットを準備 */
    SOCKET max_socket = ctx->listen_socket;
    FD_SET(ctx->listen_socket, &read_fds);
    
    for (int i = 0; i < ctx->client_count; i++) {
        if (ctx->clients[i].is_connected) {
            SOCKET s = ctx->clients[i].socket;
            FD_SET(s, &read_fds);
            if (s > max_socket) max_socket = s;
        }
    }
    
    /* 非ブロッキングチェック (10msタイムアウト) */
    struct timeval timeout = {0, 10000};
    int activity = select(max_socket + 1, &read_fds, NULL, NULL, &timeout);
    
    if (activity == SOCKET_ERROR) {
        LOG_ERROR("ソケット選択エラー", WSAGetLastError());
        return ERR_NET_RECV_FAILED;
    }
    
    /* アクティビティなし */
    if (activity == 0) {
        return 0;
    }
    
    /* 接続要求のチェック */
    if (FD_ISSET(ctx->listen_socket, &read_fds)) {
        server_accept_client(ctx);
        activity--;
    }
    
    /* メッセージ処理 */
    int processed = 0;
    for (int i = 0; i < ctx->client_count && activity > 0; i++) {
        ClientConnection* client = &ctx->clients[i];
        if (!client->is_connected) continue;
        
        if (FD_ISSET(client->socket, &read_fds)) {
            char buffer[BUFFER_SIZE];
            int bytes_received = recv(client->socket, buffer, BUFFER_SIZE, 0);
            
            if (bytes_received <= 0) {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    continue;
                }
                
                LOG_DEBUG("クライアント %d 接続切断 (エラー: %d)", client->id, error);
                closesocket(client->socket);
                client->is_connected = 0;
                activity--;
                continue;
            }
            
            /* メッセージ処理 */
            NetworkMessage msg;
            if (deserialize_message(buffer, bytes_received, &msg) > 0) {
                if (validate_message(&msg)) {
                    switch (msg.header.msg_type) {
                        case MSG_PLAYER_INPUT:
                            // TODO: ゲームロジックに転送
                            LOG_DEBUG("クライアント %d からの入力を受信", client->id);
                            break;
                            
                        case MSG_HEARTBEAT:
                            client->last_heartbeat = time(NULL);
                            LOG_DEBUG("クライアント %d のハートビート更新", client->id);
                            break;
                            
                        case MSG_DISCONNECT:
                            LOG_INFO("クライアント %d から切断要求を受信", client->id);
                            closesocket(client->socket);
                            client->is_connected = 0;
                            break;
                            
                        default:
                            LOG_WARNING("未知のメッセージタイプ: %d", msg.header.msg_type);
                            break;
                    }
                    processed++;
                } else {
                    LOG_WARNING("無効なメッセージを受信 (クライアント %d)", client->id);
                }
            } else {
                LOG_WARNING("メッセージのデシリアライズに失敗 (クライアント %d)", client->id);
            }
            
            activity--;
        }
    }
    
    return processed;
}

/**
 * @brief 全クライアントにメッセージをブロードキャストする
 * 
 * @param ctx サーバーコンテキスト
 * @param msg 送信するメッセージ
 * @return int 成功した送信数 (負の値: エラー)
 * 
 * 注意: 非接続クライアントは自動的にスキップされます
 */
int server_broadcast(ServerContext* ctx, const NetworkMessage* msg) {
    char buffer[BUFFER_SIZE];
    int size = serialize_message(msg, buffer, BUFFER_SIZE);
    
    if (size <= 0) {
        LOG_ERROR("メッセージのシリアライズに失敗", 0);
        return ERR_NET_SEND_FAILED;
    }
    
    int success_count = 0;
    for (int i = 0; i < ctx->client_count; i++) {
        if (ctx->clients[i].is_connected) {
            if (send(ctx->clients[i].socket, buffer, size, 0) != SOCKET_ERROR) {
                success_count++;
            } else {
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    LOG_WARNING("クライアント %d への送信がブロックされました", ctx->clients[i].id);
                } else {
                    LOG_ERROR("クライアント %d への送信に失敗", ctx->clients[i].id);
                    closesocket(ctx->clients[i].socket);
                    ctx->clients[i].is_connected = 0;
                }
            }
        }
    }
    
    if (success_count == 0) {
        LOG_ERROR("全クライアントへの送信に失敗", 0);
        return ERR_NET_SEND_FAILED;
    }
    
    return success_count;
}

/**
 * @brief 特定のクライアントにメッセージを送信する
 * 
 * @param ctx サーバーコンテキスト
 * @param client_id クライアントID
 * @param msg 送信するメッセージ
 * @return int エラーコード (NET_OK: 成功)
 */
int server_send_to_client(ServerContext* ctx, int client_id, const NetworkMessage* msg) {
    if (client_id < 0 || client_id >= ctx->client_count || !ctx->clients[client_id].is_connected) {
        LOG_ERROR("無効なクライアントID: %d", client_id);
        return ERR_NET_INVALID_ARG;
    }
    
    char buffer[BUFFER_SIZE];
    int size = serialize_message(msg, buffer, BUFFER_SIZE);
    
    if (size <= 0) {
        LOG_ERROR("メッセージのシリアライズに失敗", 0);
        return ERR_NET_SEND_FAILED;
    }
    
    if (send(ctx->clients[client_id].socket, buffer, size, 0) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        LOG_ERROR("クライアント %d への送信失敗", client_id);
        
        if (error != WSAEWOULDBLOCK) {
            closesocket(ctx->clients[client_id].socket);
            ctx->clients[client_id].is_connected = 0;
        }
        
        return ERR_NET_SEND_FAILED;
    }
    
    return NET_OK;
}

/**
 * @brief クライアントの接続状態を確認する
 * 
 * @param ctx サーバーコンテキスト
 * @return int 切断されたクライアント数
 * 
 * 処理内容:
 * 1. 各クライアントの最終ハートビート時刻をチェック
 * 2. タイムアウトしたクライアントを切断
 * 3. 切断されたクライアント数を返す
 */
int server_check_heartbeats(ServerContext* ctx) {
    time_t current_time = time(NULL);
    int disconnected = 0;
    
    for (int i = 0; i < ctx->client_count; i++) {
        if (!ctx->clients[i].is_connected) continue;
        
        double seconds_since = difftime(current_time, ctx->clients[i].last_heartbeat);
        if (seconds_since > (HEARTBEAT_INTERVAL * 3) / 1000.0) {
            LOG_WARNING("クライアント %d がタイムアウト (最終確認: %.0f秒前)", 
                      ctx->clients[i].id, seconds_since);
                      
            closesocket(ctx->clients[i].socket);
            ctx->clients[i].is_connected = 0;
            disconnected++;
        }
    }
    
    return disconnected;
}

/**
 * @brief サーバーリソースを解放する
 * 
 * @param ctx サーバーコンテキスト
 * 
 * 処理内容:
 * 1. 全クライアントソケットの切断
 * 2. 待機ソケットの閉鎖
 * 3. Winsockライブラリの終了
 * 
 * 注意: この関数は必ず呼び出される必要があります
 */
void server_cleanup(ServerContext* ctx) {
    LOG_DEBUG("サーバーリソースの解放開始");
    
    /* 全クライアントの切断 */
    for (int i = 0; i < ctx->client_count; i++) {
        if (ctx->clients[i].is_connected) {
            LOG_DEBUG("クライアント %d を切断中", ctx->clients[i].id);
            shutdown(ctx->clients[i].socket, SD_BOTH);
            closesocket(ctx->clients[i].socket);
            ctx->clients[i].is_connected = 0;
        }
    }
    ctx->client_count = 0;
    
    /* 待機ソケットの閉鎖 */
    if (ctx->listen_socket != INVALID_SOCKET) {
        closesocket(ctx->listen_socket);
        ctx->listen_socket = INVALID_SOCKET;
    }
    
    /* Winsock終了 */
    WSACleanup();
    
    ctx->state = SERVER_STATE_SHUTDOWN;
    LOG_INFO("サーバーリソースの解放完了");
}