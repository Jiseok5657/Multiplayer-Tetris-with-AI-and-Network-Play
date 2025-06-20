/**
 * @file client.h
 * @brief TCPクライアント機能の宣言
 * 
 * このファイルはマルチプレイヤーテトリスゲームのクライアント機能を宣言します。
 * 主な内容:
 *   - クライアント状態の列挙型定義
 *   - クライアント制御構造体の定義
 *   - クライアント関数のプロトタイプ宣言
 * 
 * 設計思想:
 *   - サーバーとのシンプルなインターフェース
 *   - ステートフルな接続管理
 *   - ノンブロッキング通信
 *   - 最小限の依存関係
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "network_defs.h"
#include "protocol.h"

/**
 * @brief クライアントの状態を表す列挙型
 */
typedef enum {
    CLIENT_STATE_DISCONNECTED,  /**< サーバー未接続状態 */
    CLIENT_STATE_CONNECTING,    /**< サーバー接続中 */
    CLIENT_STATE_CONNECTED,     /**< サーバー接続済み */
    CLIENT_STATE_GAMING,        /**< ゲーム実行中 */
    CLIENT_STATE_DISCONNECTING  /**< 切断処理中 */
} ClientState;

/**
 * @brief クライアント制御構造体
 * 
 * クライアントの接続状態とサーバー情報を管理します。
 */
typedef struct {
    SOCKET socket;              /**< サーバー接続ソケット */
    struct sockaddr_in server_addr; /**< サーバーアドレス情報 */
    ClientState state;          /**< 現在のクライアント状態 */
    int player_id;              /**< サーバーから割り当てられたプレイヤーID */
    time_t last_heartbeat;      /**< 最終ハートビート受信時刻 */
} ClientContext;

/* 関数プロトタイプ宣言 */

/**
 * @brief クライアントを初期化する
 * @param ctx クライアントコンテキスト
 * @return エラーコード (NET_OK: 成功)
 */
int client_init(ClientContext* ctx);

/**
 * @brief サーバーに接続する
 * @param ctx クライアントコンテキスト
 * @param ip サーバーIPアドレス
 * @param port サーバーポート番号
 * @return エラーコード (NET_OK: 成功)
 */
int client_connect(ClientContext* ctx, const char* ip, int port);

/**
 * @brief メッセージを送信する
 * @param ctx クライアントコンテキスト
 * @param msg 送信するメッセージ
 * @return エラーコード (NET_OK: 成功)
 */
int client_send_message(ClientContext* ctx, const NetworkMessage* msg);

/**
 * @brief メッセージを受信する
 * @param ctx クライアントコンテキスト
 * @param msg [出力] 受信したメッセージ
 * @return >0: 受信メッセージサイズ, 0: 受信データなし, <0: エラー
 */
int client_receive_message(ClientContext* ctx, NetworkMessage* msg);

/**
 * @brief 接続状態を確認する
 * @param ctx クライアントコンテキスト
 * @return エラーコード (NET_OK: 接続良好)
 */
int client_check_connection(ClientContext* ctx);

/**
 * @brief クライアントリソースを解放する
 * @param ctx クライアントコンテキスト
 */
void client_cleanup(ClientContext* ctx);

#endif /* CLIENT_H */