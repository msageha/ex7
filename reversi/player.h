/**
   @file  player.h
   @brief プレイヤーを表す型と幾つかのプレイヤーの実装

   このファイルは #player_t 構造体を定義する.
   この構造体は，与えられた盤面と手番における手を返す, 
   プレイヤーを表現する．プレイヤーはリバーシのロジックには関与しない. 
   盤面の操作やら合法手のチェックはプレイヤーの責任ではない.

   また，このファイルではいくつかのプレイヤーの実装を提供する.
   make_random_player()は合法手をランダムに選択するランダムプレイヤーを作成する.
   make_human_player()は打つべき手をインタラクティブにユーザに問い合わせるプレイヤーを作成する.
   make_slow_random_player()は着手の遅いランダムプレイヤーを作成する.
   この関数自体が有用なものというではなく,
   player_t::_hidden_state の使用例を示すためだけ実装されている.
   make_replaying_random_player()もやはりランダムプレイヤーを作成するが乱数列をファイルから読みこむ.
   こちらも player_t::_hidden_state の使用例のひとつである.
 */

#ifndef __JIKKEN_C_REVERSI_PLAYER__
#define __JIKKEN_C_REVERSI_PLAYER__

#include "color.h"
#include "board.h"
#include <stdlib.h>
#include <stdbool.h>

/**
   プレイヤーを表す構造体．
   プレイヤーは与えられば盤面と手番に対しどの手を打つかを知っている.

   この型のフィールドに直接アクセスすることは推奨されない．
*/
struct player_t {
  /**
     _play() からアクセス可能なポインタ．
     _play() 時に参照したいデータを格納するのに用いる．
   */
  void* _hidden_state;

  /**
     実際に手を指す関数への関数ポインタ．
     
     @param[in]     board 着手前の盤面
     @param[in]     color 手番の色
     @param[out]    mp    着手した手を格納するためのポインタ
     @param[in,out] state このプレイヤーの #_hidden_state.
     @return 成功すれば0，失敗すれば負数
   */
  int  (*_play)(const board_t* board, const color_t color, 
                struct move_t* mp, void* state);

  /**
     このプレイヤーの確保したリソースの解放等を行うための
     関数への関数ポインタ．

     もし，このプレイヤーの #_hidden_state が malloc() されているなら, 
     ユーザはこの関数ポインタを free() を指すようにすべきかもしれない．

     あるいは, #_hidden_state の初期値を作成するのにその他のリソースを確保しているのならば, 
     この関数ポインタをその解放を行う関数を指すようにすべきであろう．

     @param[in,out] state このプレイヤーの #_hidden_state
   */
  void (*_clean_up)( void* state );
};

/**
   プレイヤーの破棄
*/ 
static inline void destruct_player( struct player_t* p ) {
  p->_clean_up( p->_hidden_state );
  free( p );
}

/**
   @brief プレイヤーに次の手を聞く

   @param[in]  p     プレイヤー
   @param[in]  board 着手前の盤面
   @param[in]  color 着手時の手番の色
   @param[out] mp    プレイヤーが着手した手を格納するポインタ

   @return 成功すれば0を，エラー時には負数を返す．
 */
static inline int play( struct player_t* p, const board_t* board, 
                        const color_t color, struct move_t* mp) {
  return p->_play( board, color, mp, p->_hidden_state);
}

/**
   人間のプレイヤーを作成する．

   このプレイヤーは play() 時に標準入出力を通してユーザに手を問い合わせる. 
 */
struct player_t* make_human_player();

/**
   ランダムプレイヤーを作成する．

   このプレイヤーは play() 時にランダムに合法手を着手する．
 */
struct player_t* make_random_player();

/**
   @brief 着手の遅いランダムプレイヤーを作成する．

   @param wait 着手の遅れ（マイクロ秒）
 */
struct player_t* make_slow_random_player( int wait );

/**
   @brief 保存された乱数列を元に「ランダムに」プレイするランダムプレイヤーを作成する

   ファイルは十分に大きい必要があるが，<tt>sizeof(int) * 64</tt> bytes程度あれば十分である．
   @c /dev/urandom を使えば（Cの乱数を使うよりかはよい）ランダムプレイヤーになるかもしれない．

   @param file ファイルへのパス
*/

struct player_t* make_replaying_random_player( const char* filepath );  

// リモートプレイヤー用の関数

struct player_t* make_remote_player();

// ランダムプレイ
int random_play( const board_t* board, const color_t color, struct move_t* mp, void* state  );
#endif
