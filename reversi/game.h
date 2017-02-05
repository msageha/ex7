/**
   @file  game.h
   @brief リバーシのゲームルールの実装

   このファイルはリバーシの1ゲームを行う game() を提供する
 */
#ifndef __JIKKEN_C_REVERSI_GAME__
#define __JIKKEN_C_REVERSI_GAME__

#include "player.h"

/**
   与えられた二人のプレイヤーによりリバーシを1ゲームを実行する．

   この関数によって，合法手がない盤面に対しそのプレイヤーの play() が呼ばれることはない．

   @param[in,out] b_player  黒石プレイヤー（先手）
   @param[in,out] w_player  白石プレイヤー（後手）
   @param[out]    board     もし \c NULL でなければ, 最終局面を指すポインタが格納される (destruct_board()を後で呼ぶ必要あり) 
   @return 0  if ゲームが無事終了したとき
           -1 if いずれかのプレイヤーが違法手を着手したとき
           -2 if play()においてエラーが発生したとき
*/
int game( struct player_t* b_player, struct player_t* w_player, 
          board_t** board);

#endif
