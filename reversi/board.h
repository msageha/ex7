/**
   @file   board.h
   @brief リバーシの盤面とその操作．
   
   
   盤面や手の処理の他に，このファイルは write_board(), read_board(),
   write_move() 及び read_move() という関数も提供する．
   これらは，盤面や手をシリアライズするために用意されている．
 */

#ifndef __JIKKEN_C_REVERSI_BOARD__
#define __JIKKEN_C_REVERSI_BOARD__

#include "color.h"
#include <stdbool.h>

/** 
   
    型 board_t は color_t の別名である. 
    ただし，我々は常に board_t* を100（10×10）要素の color_t 
    の配列を指すポインタの型として使用する．
*/
typedef color_t board_t;


/**
   構造体 #move_t はリバーシにおける手を表す
*/
struct move_t {
  /** 
      もし真ならばパスを表す
   */  
  bool passed;
  /** 行インデックス: 1-8 */
  int i; 
  /** 列インデックス: 1-8 */ 
  int j; 
};

/**
   (i,j)へ石を置くことを表す #move_t 構造体を作成する．
 */
struct move_t make_move( const int i, const int j );
/**
   パスを表す #move_t 構造体を作成する．
 */
struct move_t passed();


/**
   #move_t 構造体をバイト列に書き出す

   @param[in]  mv      書き出す手
   @param[out] buf     書き出し先のバッファへのポインタ
*/

void write_move( const struct move_t mv, char* buf ); 

/**
   バイト列から #move_t 構造体を読み出す．

   @param[in]  buf    読み出し元のバッファへのポインタ
   @param[out] mv     move_t 構造体へのポインタ（NULLではない）
   @return     成功時は呼んだバイト数を, 失敗時は0を返す
*/
int read_move( const char* buf, struct move_t* mv );

/**
   与えられた場所にある石の色を返す．

   @param[in] board 盤面
   @param[in] i     行インデックス
   @param[in] j     列インデックス
 */
color_t color_at( const board_t* board, const int i, const int j );

/**
   リバーシの盤面を作成する.
   盤面は必ず destruct_board() により解放されなければならない.
 */
board_t* create_board ();

/**
   リバーシの盤面を破棄する
 */
void destruct_board( board_t* board );

/**
   与えられた盤面 \p board と手番 \p color において, 
   手 \p mv が合法か否かを判定する．

   @param[in] board  着手する盤面
   @param[in] color  手番の色 (#WHITE or #BLACK)
   @param[in] mv     手

   @return    合法手か否か
 */
bool is_legal_move(const board_t* board, const color_t color,
                   const struct move_t mv);

/**
   与えられた盤面 \p board と手番 \p color における全ての合法手を計算する．

   @param[in]  board   着手する盤面
   @param[in]  color   手番の色 (#WHITE or #BLACK)
   @param[out] moves   求められた手が書き込まれる配列へのポインタ
   @param[out] len     求めた合法手の数 (<60). 
 */
void legal_moves( const board_t* board, const color_t color,
                  struct move_t* moves, int* len );

/**
   与えられた盤面と手番において，着手可能な手が存在するかどうかを判定

   @param[in] board  盤面
   @param[in] color  手番の色 (#WHITE or #BLACK).
 */
bool is_playable( const board_t* board, const color_t color );

/**
   与えられた盤面が終局しているかどうか, 
   つまりいずれのプレイヤーにとっても着手可能でないことを判定する
   
   式
   @code
    is_end_game( board ) 
   @endcode
   は以下の式と等価である．
   @code
   is_playable( board, WHITE ) && is_playable( board, BLACK )
   @endcode


   @param[in] board 盤面
   @return          終局しているかどうか
 */
bool is_end_game( const board_t* board );


/**
   盤面をバイト列として書き出す．書き出されるのは常に64バイトである．
   
   @param[in]  board  書き出す盤面
   @param[out] buf    書き出し先のバッファ
*/
void write_board( const board_t* board, char* buf );


/**
   盤面をバイト列から読み出す．

   @param[in]  buf    読み出し元のバッファ（少なくともbuf+64が有効でなければならない）
   @param[out] board  読み出した盤面を書き出すための有効なポインタ (NULLであってはならない).
   @return     成功時は64を，そうでなければ0を返す．
*/
int read_board( const char* buf, board_t* board ); 


/**
   実際に着手する．

   @param[in,out] board  着手する前および後の盤面
   @param[in]     color  着手する石の色
   @param[in]     mv     着手する手

   @return 成功時は0，そうでなければ -1 を返す．
 */
int do_move( board_t* board, const color_t color,
             const struct move_t mv );

/**
   盤面を標準出力にプリントする．

   @param[in]    board プリントする盤面
 */
void print_board( const board_t* board );
/**
   盤面を合法手の情報付きで標準出力にプリントする．

   @param[in]    board プリントする盤面
   @param[in]    color 手番の色 (#WHITE or #BLACK).
 */
void print_board_with_legal_moves( const board_t* board, const color_t color );


#endif
