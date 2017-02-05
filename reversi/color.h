/**
   @file  color.h
   @brief 石の色 (#WHITE, #BLACK or #NONE)
 */

#ifndef __JIKKEN_C_REVERSI_COLOR__
#define __JIKKEN_C_REVERSI_COLOR__

/** 
    石の色，手番（黒番，白番）の表現にも利用．
*/
typedef unsigned int color_t;

/** マスに石がないことの表現 */
static const color_t NONE     = 0; 
/** 白石 */
static const color_t WHITE    = 1;
/** 黒石 */
static const color_t BLACK    = 2; 

/**
   #WHITE を #BLACK に, そして #BLACK を #WHITE にする. 
   ただし, #NONE はそのまま.
 */
color_t flip_color(const color_t c);

/**
   #WHITE を `'O'`に, #BLACK を `'X'` に, そして #NONE を @c '-' にする.
 */
char    color2char( const color_t c );

/**
   color2char() の逆
 */
color_t char2color( const char c );

#endif
