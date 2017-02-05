#include "color.h"

color_t flip_color(const color_t c) {
  return (2 - c) + 1;
}

char color2char ( const color_t c ) {
  if ( c == WHITE ) {
    return 'O';
  }
  else if ( c == BLACK ) {
    return 'X';
  }
  return '-';
}

color_t char2color( const char c ) {
  if ( c == 'X' ) {
    return BLACK;
  }
  else if ( c == 'O' ) {
    return WHITE;
  }
  return NONE;
}
