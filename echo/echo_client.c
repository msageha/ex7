#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
connect_to(host, service)はhostのserviceに接続し，
通信のためのファイル記述子を返す．

hostはホスト名もしくは，IPv4かIPv6アドレス
serviceはポート番号．
詳しくは，man 2 getaddrinfo
*/
int connect_to(const char* host, const char* service) {
  struct addrinfo* ans;
  struct addrinfo  hints;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags    = 0;
  hints.ai_family   = PF_UNSPEC;   /* Both IPv4 and IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* TCP */
  hints.ai_protocol = 0;

  int error = getaddrinfo( host, service, &hints, &ans );
  if ( error < 0 ) {
    fprintf( stderr, "Oops getaddrinfo failed!\n");
    exit(1);
  }
  if ( ans == NULL ) goto ERROR;

  int sock = -1; 
  int res  = -1;
  for ( struct addrinfo *p = ans; p != NULL; p = p->ai_next ) {
    sock = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
    if ( sock == -1 ) { continue; }

    while ( 1 ) {
      res = connect( sock, p->ai_addr, p->ai_addrlen );
      /* connectがシグナルによって中断されたのでリトライ */ 
      if ( res == -1 && errno == EINTR ) continue;
      break;
    }
    
    if ( res != -1 ) {
      break;      
    }
    fprintf( stderr, "Error: %s\n", strerror(errno) );
    if ( p->ai_next != NULL ) {
      fprintf( stderr, "Retrying...\n" );
    }
    close( sock );
  }
  freeaddrinfo( ans ); 

  if ( res == -1 ) goto ERROR;

  return sock;

 ERROR: 
  fprintf( stderr, "Cannot connect to %s:%s.\n", host, service );
  exit(1);
}

/**
標準入力から文字を読みとり，それをそのままサーバに送る．
*/
void interact_user( FILE* fp ) {
  int ch;
  unsigned char buf[1];
  while ( (ch = getchar()) != EOF ) {
    buf[0] = ch;

    /* サーバに読みとった文字を送る．*/ 
    fwrite( buf, sizeof(char), 1, fp );

    /* サーバから文字を読みとる */ 
    int rsize = fread( buf, sizeof(char), 1, fp );

    /* 正常もしくは異常に終了した場合 */ 
    if ( rsize <= 0 ) break;

    /* サーバから送られてきた文字を表示 */ 
    putchar( buf[0] );
  }  
}

int main( int argc, char** argv ) {
  if ( argc < 3 ) {
    fprintf( stderr, "Usage: %s HOSTNAME SERVICE\n", argv[0]);
  }
  char* host    = argv[1];
  char* service = argv[2];

  int sock = connect_to( host, service );
  FILE* fp = fdopen( sock, "r+" );
  setbuf(fp, NULL); // バッファリングオフ
  interact_user( fp );
  fclose( fp );
}
