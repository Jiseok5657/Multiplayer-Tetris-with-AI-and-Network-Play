/**
 * @file network_defs.h
 * @brief ネットワーク通信の基本定義
 * 
 * このファイルはネットワーク通信に関連する定数、マクロ、データ型を定義します。
 * サーバーとクライアントの両方で使用される共通の定義を含みます。
 * 
 * 主な内容:
 *   - ネットワーク設定定数
 *   - メッセージタイプ定義
 *   - プレイヤーコマンド定義
 *   - ネットワークエラーコード
 *   - 便利なマクロ
 * 
 * 設計思想:
 *   - 一貫性のあるネットワーク処理
 *   - プラットフォーム非依存の定義
 *   - エラー処理の標準化
 *   - デバッグ機能のサポート
 */

#ifndef NETWORK_DEFS_H
#define NETWORK_DEFS_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

/* ネットワーク設定定数 */
#define DEFAULT_PORT        5555    /**< デフォルトのポート番号 */
#define MAX_CLIENTS         2       /**< 最大クライアント接続数 */
#define BUFFER_SIZE         1024    /**< ネットワークバッファサイズ */
#define SERVER_TIMEOUT_MS   5000    /**< サーバー操作タイムアウト(ms) */
#define HEARTBEAT_INTERVAL  1000    /**< ハートビート間隔(ms) */
#define MAX_RETRIES         3       /**< 接続リトライ最大回数 */

/**
 * @brief メッセージタイプ定義
 * 
 * クライアントとサーバー間で送信されるメッセージの種類を識別します。
 */
typedef enum {
    MSG_INVALID = 0,        /**< 無効メッセージ (予約) */
    MSG_CONNECT_REQUEST,    /**< 接続要求 (クライアント→サーバー) */
    MSG_CONNECT_ACCEPT,     /**< 接続許可 (サーバー→クライアント) */
    MSG_CONNECT_REJECT,     /**< 接続拒否 (サーバー→クライアント) */
    MSG_GAME_STATE,         /**< ゲーム状態 (サーバー→クライアント) */
    MSG_PLAYER_INPUT,       /**< プレイヤー入力 (クライアント→サーバー) */
    MSG_HEARTBEAT,          /**< 接続確認 (双方向) */
    MSG_DISCONNECT,         /**< 切断通知 (双方向) */
    MSG_GAME_EVENT          /**< ゲームイベント (サーバー→クライアント) */
} MessageType;

/**
 * @brief プレイヤー入力コマンド定義
 * 
 * ゲーム操作のための入力コマンドを表します。
 * ビットマップ形式で送信されるため、複数の操作を同時に送信できます。
 */
typedef enum {
    CMD_NONE = 0,           /**< 無操作 */
    CMD_MOVE_LEFT,          /**< 左移動 */
    CMD_MOVE_RIGHT,         /**< 右移動 */
    CMD_ROTATE_CW,          /**< 時計回り回転 */
    CMD_ROTATE_CCW,         /**< 反時計回り回転 */
    CMD_SOFT_DROP,          /**< ソフトドロップ */
    CMD_HARD_DROP,          /**< ハードドロップ */
    CMD_HOLD,               /**< ホールド */
    CMD_PAUSE               /**< 一時停止 */
} PlayerCommand;

/**
 * @brief ネットワークエラーコード
 * 
 * ネットワーク操作の結果を表すエラーコードです。
 */
typedef enum {
    NET_OK = 0,             /**< 成功 */
    ERR_NET_INIT_FAILED,    /**< ネットワーク初期化失敗 */
    ERR_NET_CONN_FAILED,    /**< 接続失敗 */
    ERR_NET_SEND_FAILED,    /**< 送信失敗 */
    ERR_NET_RECV_FAILED,    /**< 受信失敗 */
    ERR_NET_TIMEOUT,        /**< タイムアウト */
    ERR_NET_DISCONNECTED,   /**< 切断検出 */
    ERR_NET_INVALID_MSG,    /**< 無効メッセージ */
    ERR_NET_SERVER_FULL,    /**< サーバー満杯 */
    ERR_NET_INVALID_ARG     /**< 無効な引数 */
} NetError;

/* ネットワークマクロ定義 */
#define ADDRESS_FAMILY      AF_INET     /**< IPv4アドレスファミリ */
#define SOCKET_TYPE         SOCK_STREAM /**< ストリームソケット */
#define PROTOCOL            IPPROTO_TCP /**< TCPプロトコル */
#define SOCKET_ERROR        -1          /**< ソケットエラー値 */

/**
 * @brief ソケットアドレス構造体を初期化するマクロ
 * 
 * @param addr 初期化するsockaddr_in構造体
 * @param port 使用するポート番号
 * 
 * 使用方法:
 *   struct sockaddr_in server_addr;
 *   INIT_ADDR(server_addr, 5555);
 */
#define INIT_ADDR(addr, port) \
    do { \
        memset(&(addr), 0, sizeof(addr)); \
        (addr).sin_family = ADDRESS_FAMILY; \
        (addr).sin_port = htons(port); \
        (addr).sin_addr.s_addr = INADDR_ANY; \
    } while(0)

/**
 * @brief ネットワークエラーログ出力マクロ
 * 
 * @param msg エラーメッセージ
 * @param err エラーコード
 * 
 * エラー発生時にファイル名と行番号を含む詳細を出力します。
 */
#define NET_LOG_ERROR(msg, err) \
    fprintf(stderr, "[NET_ERROR] %s: %d (File: %s, Line: %d)\n", \
            msg, err, __FILE__, __LINE__)

/**
 * @brief ネットワークデバッグログマクロ
 * 
 * デバッグビルド時のみ有効になります。
 * NET_DEBUGが定義されていない場合は何もしません。
 */
#ifdef NET_DEBUG
    #define NET_LOG_DEBUG(fmt, ...) \
        printf("[NET_DEBUG] " fmt " (File: %s, Line: %d)\n", \
               ##__VA_ARGS__, __FILE__, __LINE__)
#else
    #define NET_LOG_DEBUG(fmt, ...) (void)0
#endif

/* プラットフォーム固有の型定義 */
#ifdef _WIN32
    typedef SOCKET socket_t;        /**< Windowsソケット型 */
    #define INVALID_SOCKET_VALUE INVALID_SOCKET /**< 無効ソケット値 */
#else
    typedef int socket_t;           /**< POSIXソケット型 */
    #define INVALID_SOCKET_VALUE (-1) /**< 無効ソケット値 */
#endif

/**
 * @brief ネットワークアドレス情報構造体
 * 
 * IPv4アドレスとポート番号を保持します。
 */
typedef struct {
    char ip[16];    /**< IPv4アドレス文字列 (xxx.xxx.xxx.xxx形式) */
    int port;       /**< ポート番号 */
} NetAddress;

#endif /* NETWORK_DEFS_H */