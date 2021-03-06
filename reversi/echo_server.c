#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "board.h"
#include "color.h"


/**
listen_to( service )はserviceへの接続を受けつけるための
ソケットを作成する．
*/

int montecarlo(const board_t* board, const color_t color, struct move_t* mp, void* state);

int listen_to( char* service ) {
  struct addrinfo* ans;
  struct addrinfo  hints;

  memset(&hints, 0, sizeof(hints));
  
  hints.ai_flags    = AI_PASSIVE;  
  hints.ai_family   = AF_UNSPEC;   /* IPv4 or IPv6 */ 
  hints.ai_socktype = SOCK_STREAM; /* TCP */
  hints.ai_protocol = IPPROTO_TCP; /* Accepts only TCP */

  int error = getaddrinfo( NULL, service, &hints, &ans );
  if ( error != 0 ) {
    exit(1);
  }
  if ( ans == NULL ) {
    fprintf( stderr, "No addresses are available\n");
    exit(1);
  }
  int sock = -1;

  for ( struct addrinfo *p = ans; p != NULL; p = p->ai_next ) {
    sock = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if ( sock == -1 ) { continue; }

    int enabled = 1;
    if ( ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int) ) < 0 ) 
         || ( bind( sock, p->ai_addr, p->ai_addrlen ) < 0 ) ) {
      close(sock);    
      sock = -1; 
    }
    else {
      break;
    }
  }
  freeaddrinfo(ans);

  if ( sock < 0 ) {
    fprintf(stderr, "Listen and bind failed.\n");
    exit(1);
  }
  fprintf( stderr, "Listening %s...\n",  service );
  listen( sock , 5 );
  return sock;
}

/** 
    接続相手と通信を行う 
*/
void serve( int fd, char* ai ) {
  FILE* fp = fdopen( fd, "r+" ); 
  setbuf(fp, NULL); // バッファリングオフ
  char buf[74];
  struct move_t move;
  board_t* board = create_board();
  while ( 1 ) {
    /* 1 byte分読みこむ */
    int rsize = fread( buf, sizeof(char), 73, fp );

    if ( rsize <= 0 ) {
      fprintf(stderr, "Connection is closed by the foreign host\n" );
      break;
    }
    buf[73] = '\0';
    if ( buf[0] == 0x04 ) { /* 0x04はCtrl+D */
      break;
    }
    if(read_board(&buf[5], board)==0) {  
      puts("something error: read board");
      break;
    }
    print_board(board);
    const color_t color = char2color(buf[70]);
    if(is_end_game(board)) {
      puts("game over");
      break;
    }
    if(strcmp(ai, "montecarlo")==0) {
      montecarlo(board, color, &move, NULL);
    }else if(strcmp(ai, "random")==0) {
      random_play(board, color, &move, NULL);
    }else{
      printf("AI_NAME is 'montecarlo' or 'random'");
      fclose( fp );
      return ;
    }
    
    write_move(move, buf);
    buf[2] = '\r';
    buf[3] = '\n';
    //puts(te);
    /* 送られた文字をそのまま返す */ 
    fwrite( buf, sizeof(char), 4, fp );
  }
  fclose( fp );
}

int montecarlo(const board_t* board, const color_t color, struct move_t* mp, void* state) {
  struct move_t move_buf[64];
  struct move_t temp_move;
  board_t* temp_board = create_board();
  board_t* temp2_board = create_board();
  int win_count[64];
  int win_max = 0;
  int len;
  char temp_char[64];
  legal_moves(board, color, move_buf, &len);
  if(len<=0) {
    *mp = passed();
    return 0;
  }
  for(int i=0; i<len; i++) {
    win_count[i] = 0;
    write_board(board, temp_char);
    read_board(temp_char, temp_board);
    
    do_move(temp_board, color, move_buf[i]);
    for(int j=0; j<100; j++) {
      write_board(temp_board, temp_char);
      read_board(temp_char, temp2_board);
      while(!is_end_game(temp2_board)) {
	  random_play(temp2_board, flip_color(color), &temp_move, NULL);
	  do_move(temp2_board, flip_color(color), temp_move);
	  random_play(temp2_board, color, &temp_move, NULL);
	  do_move(temp2_board, color, temp_move);
      }
//      printf("-------------\n");
//      print_board(temp2_board);
//      printf("-------------\n");
      write_board(temp2_board, temp_char);
      char mycolor = color2char(color);
      int count = 0;
      for(int i=0; i<64; i++) {
	if(temp_char[i] == mycolor) count++;
      }
      //printf("\n-----%d\n", count);
      if(count>32) win_count[i]++;
    }
    if(win_max<win_count[i]) win_max = win_count[i];
    printf("%d\n", win_count[i]);
  }
  int max_index = 0;
  for(max_index = 0; max_index<len; max_index++) {
    if(win_max == win_count[max_index]) break;
  }
  *mp = move_buf[max_index];
  return 0;
}


/**
   接続を待つ．
*/
void wait_connection( int sock, char* ai ) {
  while ( 1 ) {
    struct sockaddr_storage ad;
    socklen_t  adlen = sizeof ( struct sockaddr_storage );
    
    int fd = -1;
    while ( 1 ) {
      fd = accept(sock, (struct sockaddr*) &ad, &adlen );
      /* acceptがシグナルによって中断されたのでリトライ */ 
      if ( fd == -1 && errno == EINTR ) continue;      
      break;
    }     
    if ( fd == -1 ) {
      perror("accept failed");
      continue;
    }
    // fprintf( stderr, "Sock: %d, FD: %d\n", sock, fd );

    {
      char host[256];
      char serv[256];
      getnameinfo( (struct sockaddr*) &ad, adlen, 
                   host, 255, serv, 255, 
                   NI_NUMERICSERV | NI_NUMERICHOST);
      fprintf( stderr, "Connected from <%s:%s>\n", host, serv );
    }

    /* やりとり開始 */
    serve( fd, ai);
  }
}

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    fprintf( stderr, "Usage: %s AI_NAME PORT\n", argv[0]);
  }
  char* ai = argv[1];
  char* service = argv[2];

  int sock = listen_to( service );
  wait_connection( sock, ai );
  close( sock );
}

