/**
   @file  debug.h
   @brief デバッグのための #debug_print マクロを提供
 */


#ifndef __JIKKEN_C_REVERSI_DEBUG__
#define __JIKKEN_C_REVERSI_DEBUG__

#ifndef DEBUG
/**
デバッグフラグ.
このヘッダのインクルードより前に設定 (たとえばコンパイラオプション\c -D で)
されていない限りは0
 */
#define DEBUG 0 
#endif

/**
   標準エラー出力にデバッグメッセージを出力する．

   このマクロの使い方は printf() と同じである.
   printf() との違いは，このマクロは #stdout でなく #stderr に出力することと,
   呼び出されたソースファイルの位置情報も出力することである．

 */ 
#define debug_print(format, ...) \
  do { if (DEBUG) fprintf(stderr, ("[%s %s:%d] " format), __FILE__, __func__ , __LINE__, ##__VA_ARGS__); } while(0)

#endif 
