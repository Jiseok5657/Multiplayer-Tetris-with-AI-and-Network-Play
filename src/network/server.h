/**
 * @file server.h
 * @brief TCPサーバー機能の宣言
 * 
 * このファイルはマルチプレイヤーテトリスゲームのサーバー機能を宣言します。
 * 主な内容:
 *   - サーバー状態の列挙型定義
 *   - クライアント接続構造体の定義
 *   - サーバー制御構造体の定義
 *   - サーバー関数のプロトタイプ宣言
 * 
 * 設計思想:
 *   - 最小限の依存関係
 *   - スレッドセーフな設計
 *   - 単純なインターフェース
 *   - 拡張可能な構造
 */

#ifndef SERVER_H
#define SERVER_H

#include "network_defs.h"
#include "protocol.h"

/**
 * @brief サーバーの状態を表す列挙型
 */
typedef enum {
    SERVER_STATE_IDLE,          /**< 初期状態/待機中 */
    SERVER_STATE_LISTENING,     /**< クライアント接続待機中 */
    SERVER_STATE_RUNNING,       /**< ゲーム実行中 */
    SERVER_STATE_SHUTDOWN       /**< シャットダウン中/終了 */
} ServerState;

/**
 * @brief クライアント接続情報構造体
 */
typedef struct {
    SOCKET socket;              /**< クライアントソケット記述子 */
    struct sockaddr_in addr;    /**< クライアントアドレス情報 */
    int id;                     /**< クライアントID (0から開始) */
    int is_connected;           /**< 接続状態フラグ (1:接続中, 0:切断) */
    time_t last_heartbeat;      /**< 最終ハートビート受信時刻 */
} ClientConnection;

/**
 * @brief サーバー制御構造体
 * 
 * サーバーの実行状態とクライアント接続を管理します。
 */
typedef struct {
    SOCKET listen_socket;       /**< 待機ソケット記述子 */
    struct sockaddr_in addr;    /**< サーバーアドレス情報 */
    ServerState state;          /**< 現在のサーバー状態 */
    ClientConnection clients[MAX_CLIENTS]; /**< クライアント接続配列 */
    int client_count;           /**< 接続中のクライアント数 */
} ServerContext;

/* 関数プロトタイプ宣言 */

/**
 * @brief サーバーを初期化する
 * @param ctx サーバーコンテキスト
 * @param port 使用するポート番号
 * @return エラーコード (NET_OK: 成功)
 */
int server_init(ServerContext* ctx, int port);

/**
 * @brief サーバーを起動する
 * @param ctx サーバーコンテキスト
 * @return エラーコード (NET_OK: 成功)
 */
int server_start(ServerContext* ctx);

/**
 * @brief クライアント接続を受け付ける
 * @param ctx サーバーコンテキスト
 * @return クライアントID (負の値: エラー)
 */
int server_accept_client(ServerContext* ctx);

/**
 * @brief クライアントメッセージを処理する
 * @param ctx サーバーコンテキスト
 * @return 処理したメッセージ数 (負の値: エラー)
 */
int server_handle_messages(ServerContext* ctx);

/**
 * @brief 全クライアントにメッセージをブロードキャストする
 * @param ctx サーバーコンテキスト
 * @param msg 送信するメッセージ
 * @return 成功した送信数 (負の値: エラー)
 */
int server_broadcast(ServerContext* ctx, const NetworkMessage* msg);

/**
 * @brief 特定のクライアントにメッセージを送信する
 * @param ctx サーバーコンテキスト
 * @param client_id クライアントID
 * @param msg 送信するメッセージ
 * @return エラーコード (NET_OK: 成功)
 */
int server_send_to_client(ServerContext* ctx, int client_id, const NetworkMessage* msg);

/**
 * @brief クライアントの接続状態を確認する
 * @param ctx サーバーコンテキスト
 * @return 切断されたクライアント数
 */
int server_check_heartbeats(ServerContext* ctx);

/**
 * @brief サーバーリソースを解放する
 * @param ctx サーバーコンテキスト
 */
void server_cleanup(ServerContext* ctx);

#endif /* SERVER_H */