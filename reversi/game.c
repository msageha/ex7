#include "game.h"
#include "debug.h"

#include <stdio.h>

int game( struct player_t* b_player, struct player_t* w_player, 
          board_t** final_board ) {
  board_t* b = create_board();

  struct player_t* p1 = b_player;
  struct player_t* p2 = w_player;
  color_t c1 = BLACK;
  int ret = 0;

  while ( !is_end_game( b ) ) {
    if ( DEBUG ) {
      print_board_with_legal_moves(b, c1);
    }
    if ( is_playable(b, c1) ) {
      struct move_t mp;

      if ( play( p1, b, c1, &mp ) < 0 ) {
        // An error occurs
        ret = -2;
        goto RETURN;
      }
      
      int r = do_move( b, c1, mp );
      if ( r < 0 ) { // illegal move
        ret = -1;
        goto RETURN;
      }
    }
    struct player_t* t = p1;
    p1 = p2;
    p2 = t;
    c1 = flip_color( c1 );
  }

  RETURN:
  if ( final_board != NULL ) {
      *final_board = b;
  }
  else {
      destruct_board(b);
  }
  return ret;
}
