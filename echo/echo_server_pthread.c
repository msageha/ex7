#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

void dump_addr_info( struct sockaddr* addr, int addr_len ) {
  char host[256];
  char serv[256];
  getnameinfo( addr, addr_len, 
               host, 255, serv, 255, 
               NI_NUMERICSERV | NI_NUMERICHOST);
  fprintf( stderr, "Connected from <%s:%s>\n", host, serv );
}


/**
listen_to( service )はserviceへの接続を受けつけるための
ソケットを作成する．
*/
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
void serve( int fd ) {
  FILE* fp = fdopen( fd, "r+" ); 
  setbuf(fp, NULL); // バッファリングオフ

  char buf[1];
  while ( 1 ) {
    /* 1 byte分読みこむ */
    int rsize = fread( buf, sizeof(char), 1, fp ); 
    if ( rsize <= 0 ) {
      fprintf(stderr, "Connection is closed by the foreign host\n" );
      break;
    }
    
    fprintf(stderr, "Recv: <%02x>\n", buf[0]);

    if ( buf[0] == 0x04 ) { /* 0x04はCtrl+D */
      break;
    }
    /* 送られた文字をそのまま返す */ 
    fwrite( buf, sizeof(char), 1, fp ); 
  }
  fclose( fp );
}


void* worker( void* p ) {
  int fd = *((int*) p);
  free(p);
  serve(fd);
  return NULL;
}

/**
   接続を待つ．
*/
void wait_connection( int sock ) {
  while ( 1 ) {
    struct sockaddr_storage ad;
    socklen_t  adlen = sizeof ( struct sockaddr_storage );
    
    int fd  = -1;
    pthread_t tid;
    int* p;

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
    dump_addr_info( (struct sockaddr*) &ad, adlen );

    p  = malloc( sizeof( int ) );
    *p = fd;
    if ( pthread_create( &tid, NULL, worker, p ) < 0 ) {
      free(p);
      perror("Thread creation failed");
      close( fd );         
      continue;
    }    
    /* スレッドの終了後，スレッドを破棄可能に */
    pthread_detach( tid );
  }
}

int main( int argc, char** argv ) {
  if ( argc < 2 ) {
    fprintf( stderr, "Usage: %s SERVICE\n", argv[0]);
  }
  char* service = argv[1];

  int sock = listen_to( service );
  wait_connection( sock );
  close( sock );
}

