#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* For memset */
#include <time.h>
#include <ncurses.h>  /* PDCursesライブラリ */
#include "game.h"

/* ボードの初期化関数 - 最適化版 */
void init_board() {
    /* memsetを使用してボード全体を一度に0で初期化 */
    memset(board, 0, sizeof(board));
}

/* ゲームループを実行する関数 */
void game_loop() {
    spawn_new_block();  /* 新しいブロックの生成 */
    /* ゲームループ開始 */
    while (1) {
        /* 画面出力 */
        /* clear(); 画面を消去しない(変更された部分のみを更新) */
        /* ゲームボードの更新:変更された部分のみを更新 */
        print_board();  /* 現在のボードを表示 */
        /* ゲームオーバー判定 */
        if (check_game_over()) {
            /* 画面中央に「GAME OVER」のメッセージを表示 */
            int x = (BOARD_WIDTH - 9) / 2;  /* テキストのサイズを考慮して中央を計算 */
            int y = BOARD_HEIGHT / 2;
            mvprintw(y, x, "GAME OVER");
            refresh();
            break;  /* ゲーム終了 */
        }
        /* 入力処理 */
        int ch = getch();  /* ユーザーの入力を受け取る */
        switch (ch) {
            case 'q':  /* 'q'キーで終了 */
                {
                    int quit_x = (BOARD_WIDTH - 5) / 2;
                    int quit_y = BOARD_HEIGHT / 2 + 1;
                    mvprintw(quit_y, quit_x, "QUIT");
                    refresh();
                    return;
                }
            case KEY_LEFT:  /* 左矢印キー */
                move_block(MOVE_LEFT);
                break;
            case KEY_RIGHT:  /* 右矢印キー */
                move_block(MOVE_RIGHT);
                break;
            case KEY_DOWN:  /* 下矢印キー */
                drop_block();
                break;
            case 'r':  /* 'r'キーでブロックを回転 */
                rotate_block();
                break;
            default:
                break;
        }
        /* ブロックの落下:drop_block()を1回だけ呼び出す */
        drop_block();
        refresh();
               
        /* ゲーム速度調整(usleepはプラットフォーム依存の可能性がある) */
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000L;  /* 100ms(nanosleepを使ってより正確な制御) */
        nanosleep(&ts, NULL);  /* より正確なゲーム速度調整 */
    }
}

/* プログラムの初期化および実行関数 */
int main() {
    /* PDCursesの初期化 */
    initscr();               /* ncursesの開始 */
    raw();                   /* 入力モードの設定 */
    keypad(stdscr, TRUE);    /* キーボード入力処理 */
    noecho();                /* 入力内容を画面に表示しない */
    curs_set(0);             /* カーソルを非表示 */
    start_color();           /* 色のサポート開始 */
    init_colors();           /* 色の初期化 */
    srand(time(NULL));       /* ランダムシードの初期化 */
    
    /* ゲームボードの初期化 */
    init_board();
    
    /* ゲームループ実行 */
    game_loop();
    
    /* PDCursesの終了処理 */
    endwin();                /* ncursesの終了 */
    return 0;
}