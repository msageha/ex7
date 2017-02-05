#include "board.h"
#include "color.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "debug.h"


void write_move( const struct move_t mv, char* buf ) {
  char* p = buf; 

  if ( mv.passed ) {
    p[0] = 'X';
    p[1] = 'X';    
  }
  else {
    char col = 'A' + (mv.j - 1);
    char row = '1' + (mv.i - 1);

    p[0] = col;
    p[1] = row; 
  }
}

int read_move( const char* buf, struct move_t* mv ) {
  const char* p = buf;

  if ( mv == NULL ) {
    return 0;
  }
  
  if ( p[0] == 'X' && p[1] == 'X') {
    mv->passed = true; 
    return 2;
  }
  /* bug fix */
  if ( p[0] >= 'A' && p[0] <= 'H' &&
       p[1] >= '1' && p[1] <= '8' ) {
    mv->passed = false;
    mv->i      = 1 + (p[1] - '1'); 
    mv->j      = 1 + (p[0] - 'A');
    return 2; 
  }
  return 0;
}


static inline int ix(const int i, const int j) {
  return 10*i + j;
}

color_t color_at( const board_t* board, const int i, const int j ) {
  return board[ ix(i,j) ];
}

board_t* create_board () {
  board_t* board = calloc(100, sizeof( color_t ));
  board[ ix(4,4) ] = WHITE;
  board[ ix(5,5) ] = WHITE;
  board[ ix(4,5) ] = BLACK;
  board[ ix(5,4) ] = BLACK;

  return board;
}

void destruct_board( board_t* board ) {
  free( board );
}


int read_board( const char* buf, board_t* board ) {
  const char* p = buf;

  for ( int i = 0; i < 10; i++ ) {
    for ( int j = 0; j < 10; j++ ) {
      board[ 10*i + j ] = NONE; 
    }
  }

  for ( int i = 0; i < 8; i++ ) {
    for ( int j = 0; j < 8; j++ ) {
      board[ 10*(i+1) + (j+1) ] = char2color( p[ 8*i + j ] );
    }
  }
  return 64;
}

void write_board( const board_t* board, char* buf ) {
  char* p = buf; 

  for ( int i = 0; i < 8; i++ ) {
    for ( int j = 0; j < 8; j++ ) {
      p[ 8*i + j ] = color2char( board[ 10*(i+1) + (j+1) ] );
    }
  }
}



struct move_t make_move( const int i, const int j ) {
    return (struct move_t){ false, i, j };
}

struct move_t passed() {
  return (struct move_t){ true, 4, 4 };
}




void legal_moves( const board_t* board, const color_t color,
                  struct move_t* moves, int* len ) {
  *len = 0;
  for ( int i = 1; i <= 8; i++ ) {
    for ( int j = 1; j <= 8; j++ ) {
      struct move_t mv = (struct move_t){false, i, j};
      if ( is_legal_move( board, color, mv) ) {
        *moves++ = mv;
        (*len)++;
      }
    }
  }
}

bool is_playable( const board_t* board, const color_t color ) {
  for ( int i = 1; i <= 8; i++ ) {
    for ( int j = 1; j <= 8; j++ ) {
      if ( is_legal_move( board, color, (struct move_t){false,i,j}) ) {
        return true;
      }
    }
  }
  return false;
}

bool is_end_game( const board_t* board ) {
  return (!is_playable( board, WHITE ) && !is_playable( board, BLACK ));
}




static void flippable_indices_line( const board_t* board, const color_t color,
                             int i,  int j,
                             const int di, const int dj,
                             struct move_t* moves, int* len ) {
  const int start_len = *len;

  const int oc = flip_color( color );
  while ( true ) {
    i += di; j += dj;
    if ( board[ ix(i,j) ] == oc ) {
      *moves = (struct move_t){false,i,j};
      *len += 1; moves++;
    }
    else {
      break;
    }
  }
  if ( board[ ix(i,j) ] != color ) {
    *len = start_len;
  }
}

static void flippable_indices( const board_t* board, const color_t color,
                               const int i, const int j,
                               struct move_t* moves, int* len ) {
  *len = 0;
  for ( int di = -1; di <= 1; di++ ) {
    for ( int dj = -1; dj <= 1; dj++ ) {
      if ( (di == 0) && (dj == 0) ) continue;
      int l = 0;
      flippable_indices_line( board, color,
                              i, j, di, dj,
                              moves, &l);
      moves += l;
      *len  += l;
    }
  }
}

static bool is_effective_move(const board_t* board, const color_t color,
                              const int i, const int j ) {
  struct move_t buf[64]; // #(legal moves) must be less than 64;
  int   length=0;

  flippable_indices( board, color, i, j, buf, &length);

  return ( length > 0 );
}

bool is_legal_move(const board_t* board, const color_t color,
                   const struct move_t mv) {
  if ( mv.passed ) {
      return is_playable( board, color );
  }
  if ( (mv.i < 0) || (mv.i > 8) || (mv.j < 0) || (mv.j > 8) ) {
    return false;
  }
  if ( board[ ix(mv.i, mv.j) ] != NONE ) {
    return false;
  }
  return is_effective_move( board, color, mv.i, mv.j );
}



int do_move( board_t* board, const color_t color,
             const struct move_t mv ) {
  if ( !is_legal_move( board, color, mv ) ) {
    return -1;
  }
  if ( mv.passed ) {
      return 0;
  }
  struct move_t moves[64];
  int  len  = 0;
  flippable_indices( board, color, mv.i, mv.j, moves, &len );
  for ( int k = 0; k < len; k++ ) {
    board[ ix( moves[k].i, moves[k].j ) ] = color;
  }
  board[ ix( mv.i, mv.j ) ] = color;
  return 0;
}


static int count_disks( const board_t* board, const color_t color ) {
  int s = 0;
  for ( int i = 1; i <= 8; i++ ) {
    for ( int j = 1; j <= 8; j++ ) {
      if ( board[ ix(i,j) ] == color ) {
        s++;
      }
    }
  }
  return s;
}

void print_board_with_legal_moves( const board_t* board, const color_t color ) {
  puts(" |A B C D E F G H ");
  puts("-+----------------");

  for (int i = 1; i <= 8; i++) {
    printf( "%d|", i );
    for (int j = 1; j <= 8; j++) {
      if ( is_legal_move( board, color, (struct move_t){false,i,j} ) ) {
        printf( ". " );
      }
      else {
        printf( "%c ", color2char(board[ ix(i,j) ]) );
      }
    }
    puts("");
  }
  printf( "Black: %d\n", count_disks( board, BLACK ) );
  printf( "White: %d\n", count_disks( board, WHITE ) );
}

void print_board( const board_t* board ) {
  puts(" |A B C D E F G H ");
  puts("-+----------------");

  for (int i = 1; i <= 8; i++) {
    printf( "%d|", i );
    for (int j = 1; j <= 8; j++) {
      printf( "%c ", color2char(board[ ix(i,j) ]) );
    }
    puts("");
  }
  printf( "Black: %d\n", count_disks( board, BLACK ) );
  printf( "White: %d\n", count_disks( board, WHITE ) );
}
