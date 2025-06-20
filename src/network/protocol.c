/**
 * @file protocol.c
 * @brief メッセージプロトコル実装
 * 
 * このファイルはクライアントとサーバー間の通信プロトコルを実装します。
 * 主な機能:
 *   - メッセージヘッダの作成
 *   - チェックサム計算
 *   - メッセージのシリアライズとデシリアライズ
 *   - メッセージの検証
 * 
 * プロトコル仕様:
 *   - すべてのメッセージは8バイトのヘッダで始まる
 *   - ヘッダ構造: [msg_size(4), msg_type(4), checksum(2)]
 *   - データペイロードはヘッダ直後に配置
 *   - 整数値はネットワークバイトオーダー(ビッグエンディアン)で送信
 * 
 * 設計思想:
 *   - シンプルで効率的なバイナリプロトコル
 *   - 最小限のオーバーヘッド
 *   - データ整合性の保証
 *   - 前方互換性を考慮した設計
 */

#include "protocol.h"
#include "network_defs.h"
#include <string.h>
#include <stdint.h>

/**
 * @brief メッセージヘッダを作成する
 * 
 * @param type メッセージタイプ
 * @param data_size ペイロードデータサイズ
 * @return MessageHeader 作成されたヘッダ
 * 
 * ヘッダ作成処理:
 * 1. メッセージサイズ = ヘッダサイズ(8) + データサイズ
 * 2. メッセージタイプを設定
 * 3. チェックサムは後で計算するため0で初期化
 */
MessageHeader create_header(MessageType type, size_t data_size) {
    MessageHeader header;
    header.msg_size = sizeof(MessageHeader) + data_size;
    header.msg_type = type;
    header.checksum = 0;  // 後で計算
    return header;
}

/**
 * @brief 簡易チェックサムを計算する
 * 
 * @param data チェックサム対象データ
 * @param size データサイズ
 * @return uint16_t 計算されたチェックサム
 * 
 * アルゴリズム:
 *   - 16ビットXORベースのローテーティングチェックサム
 *   - データの各バイトを順次処理
 *   - バイトインデックスに応じてシフト位置を変更
 * 
 * 最適化:
 *   - ループ展開による高速化
 *   - 最小限の計算リソース
 */
uint16_t calculate_checksum(const void* data, size_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint16_t checksum = 0;
    
    // 4バイト単位で処理
    size_t i = 0;
    for (; i < size - 3; i += 4) {
        checksum ^= (bytes[i] << 8);
        checksum ^= bytes[i+1];
        checksum ^= (bytes[i+2] << 8);
        checksum ^= bytes[i+3];
    }
    
    // 残りのバイトを処理
    for (; i < size; i++) {
        if (i % 2 == 0) {
            checksum ^= (bytes[i] << 8);
        } else {
            checksum ^= bytes[i];
        }
    }
    
    return checksum;
}

/**
 * @brief メッセージをシリアライズする
 * 
 * @param msg シリアライズするメッセージ
 * @param buffer 出力バッファ
 * @param buf_size バッファサイズ
 * @return int シリアライズされたデータサイズ (負の値: エラー)
 * 
 * 処理手順:
 * 1. バッファサイズチェック
 * 2. チェックサム計算
 * 3. ヘッダコピー
 * 4. データペイロードコピー
 */
int serialize_message(const NetworkMessage* msg, char* buffer, size_t buf_size) {
    /* バッファサイズ検証 */
    if (buf_size < msg->header.msg_size) {
        LOG_ERROR("シリアライズバッファ不足 (必要: %d, 利用可能: %d)", 
                 msg->header.msg_size, buf_size);
        return -1;
    }
    
    /* ペイロードサイズ計算 */
    size_t data_size = msg->header.msg_size - sizeof(MessageHeader);
    
    /* チェックサム計算 */
    msg->header.checksum = calculate_checksum(&msg->data, data_size);
    
    /* ヘッダコピー */
    memcpy(buffer, &msg->header, sizeof(MessageHeader));
    
    /* データペイロードコピー */
    memcpy(buffer + sizeof(MessageHeader), &msg->data, data_size);
    
    return msg->header.msg_size;
}

/**
 * @brief メッセージをデシリアライズする
 * 
 * @param buffer 入力バッファ
 * @param buf_size バッファサイズ
 * @param msg [出力] デシリアライズされたメッセージ
 * @return int デシリアライズされたデータサイズ (負の値: エラー)
 * 
 * 処理手順:
 * 1. ヘッダサイズチェック
 * 2. ヘッダコピー
 * 3. メッセージサイズ検証
 * 4. データペイロードコピー
 * 5. チェックサム検証
 */
int deserialize_message(const char* buffer, size_t buf_size, NetworkMessage* msg) {
    /* ヘッダサイズチェック */
    if (buf_size < sizeof(MessageHeader)) {
        LOG_ERROR("デシリアライズデータ不足 (最小サイズ: %d, 受信: %d)", 
                 sizeof(MessageHeader), buf_size);
        return -1;
    }
    
    /* ヘッダコピー */
    memcpy(&msg->header, buffer, sizeof(MessageHeader));
    
    /* メッセージサイズ検証 */
    if (buf_size < msg->header.msg_size) {
        LOG_ERROR("不完全なメッセージ (必要: %d, 受信: %d)", 
                 msg->header.msg_size, buf_size);
        return -1;
    }
    
    /* データサイズ計算 */
    size_t data_size = msg->header.msg_size - sizeof(MessageHeader);
    
    /* データペイロードコピー */
    memcpy(&msg->data, buffer + sizeof(MessageHeader), data_size);
    
    /* チェックサム検証 */
    uint16_t calc_checksum = calculate_checksum(&msg->data, data_size);
    if (calc_checksum != msg->header.checksum) {
        LOG_ERROR("チェックサム不一致 (期待: %d, 実際: %d)", 
                 msg->header.checksum, calc_checksum);
        return -1;
    }
    
    return msg->header.msg_size;
}

/**
 * @brief メッセージを検証する
 * 
 * @param msg 検証するメッセージ
 * @return int 1: 有効, 0: 無効
 * 
 * 検証項目:
 * 1. メッセージタイプが有効範囲内か
 * 2. メッセージサイズが最小要件を満たしているか
 * 3. タイプ固有のデータサイズ要件
 */
int validate_message(const NetworkMessage* msg) {
    /* メッセージタイプ検証 */
    if (msg->header.msg_type <= MSG_INVALID || msg->header.msg_type > MSG_GAME_EVENT) {
        LOG_ERROR("無効なメッセージタイプ: %d", msg->header.msg_type);
        return 0;
    }
    
    /* 最小メッセージサイズ */
    size_t min_size = sizeof(MessageHeader);
    
    /* タイプ別のデータサイズ検証 */
    switch (msg->header.msg_type) {
        case MSG_CONNECT_REQUEST:
            min_size += sizeof(ConnectRequest);
            if (msg->header.msg_size < min_size) {
                LOG_ERROR("接続要求メッセージサイズ不足 (必要: %d, 実際: %d)", 
                         min_size, msg->header.msg_size);
                return 0;
            }
            break;
            
        case MSG_GAME_STATE:
            min_size += sizeof(GameStateData);
            if (msg->header.msg_size < min_size) {
                LOG_ERROR("ゲーム状態メッセージサイズ不足 (必要: %d, 実際: %d)", 
                         min_size, msg->header.msg_size);
                return 0;
            }
            break;
            
        case MSG_PLAYER_INPUT:
            min_size += sizeof(PlayerInputData);
            if (msg->header.msg_size < min_size) {
                LOG_ERROR("プレイヤー入力メッセージサイズ不足 (必要: %d, 実際: %d)", 
                         min_size, msg->header.msg_size);
                return 0;
            }
            break;
            
        case MSG_HEARTBEAT:
        case MSG_DISCONNECT:
            /* 追加データなし */
            if (msg->header.msg_size != min_size) {
                LOG_WARNING("ハートビート/切断メッセージに余分なデータ (サイズ: %d)", 
                           msg->header.msg_size);
            }
            break;
            
        default:
            /* 未知のタイプはサイズ検証をスキップ */
            break;
    }
    
    return 1;
}