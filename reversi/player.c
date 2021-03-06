#include "player.h"
#include "board.h"
#include "color.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>



static void do_nothing( void* state ) {}



static int get_input( const board_t* board, const color_t color,
                int* i, int* j ) {

  while ( true ) {
    printf( (color==BLACK) ? "BLACK> " : "WHITE> " );
    fflush( stdout );

    char* buf  = NULL;
    size_t len = 0;
    ssize_t read;

    if ( (read = getline(&buf, &len, stdin) ) > 0 ) {
      if ( read > 2 ) {
        *j = buf[0] - 'A' + 1;
        *i = buf[1] - '1' + 1;

        if ( is_legal_move(board, color, make_move(*i,*j)) ) {
          break;
        }
        else {
          puts("Illegal move! Input another.");
        }
      }
      free( buf );
    }
    else {
      return -1;
    }
  }
  return 0;
}

static int human_play( const board_t* board, const color_t color, struct move_t* mp, void* dummy ) {
  print_board_with_legal_moves( board, color );
  int i = 4; int j = 4;
  if ( !is_playable( board, color ) ) {
    puts("You have no chance to move!");
    *mp = passed();
  }
  else {
    if ( get_input( board, color, &i, &j ) < 0 ) {
      return -1;
    }
    *mp = make_move( i, j );
  }
  return 0;
}

struct player_t* make_human_player() {
  struct player_t* t = 
    (struct player_t*) malloc( sizeof( struct player_t ) );
  t->_hidden_state = NULL;
  t->_play         = human_play;
  t->_clean_up     = do_nothing;
  
  return t;
}

int random_play( const board_t* board, const color_t color, struct move_t* mp, void* state  ) {
  struct move_t buf[64];
  int  len;
  legal_moves( board, color, buf, &len );
  if ( len > 0 ) {
    int k = random() % len;
    *mp = buf[ k ];
  }
  else {
    *mp = passed();
  }
  if ( state != NULL ) { 
    int microsec = *((int*) state);
    struct timespec tspec; 
    tspec.tv_sec  = microsec / 1000000;;
    tspec.tv_nsec = microsec * 1000;
    nanosleep( &tspec, NULL ); 
  }
  return 0;
}

struct player_t* make_random_player() {
  struct player_t* t = 
    (struct player_t*) malloc( sizeof( struct player_t ) );
  t->_hidden_state = NULL;
  t->_play         = random_play;
  t->_clean_up     = free;
  
  return t;
}

struct player_t* make_slow_random_player( int wait ) {
    struct player_t* p = malloc( sizeof(struct player_t) );
    int*   q = malloc( sizeof( int ) );
    *q = wait;
    p->_hidden_state = q;
    p->_play = random_play;
    p->_clean_up = free;
    return p;
}


static int replaying_random_play( const board_t* board, const color_t color, struct move_t* mp, void* state  ) {
  FILE* fp = (FILE*) state; 

  struct move_t buf[64];
  int  len;
  legal_moves( board, color, buf, &len );
  if ( len > 0 ) {
    unsigned int r = 0;
    int ret = fread( &r, sizeof(unsigned int), 1, fp );
    if ( ret < 0  ) {
      fputs( "The file ends too early", stderr );
      return -1;      
    }

    int k = r % len;
    *mp = buf[ k ];
  }
  else {
    *mp = passed();
  }
  return 0;
}

static void myfclose( void* p ) {
  fclose( (FILE*) p );
}

struct player_t* make_replaying_random_player( const char* filepath ) {
  FILE* fp = fopen( filepath, "r" );
  if ( fp == NULL ) {
    exit(1); 
  }

  struct player_t* p = malloc( sizeof(struct player_t) );

  p->_hidden_state = fp;
  p->_play = replaying_random_play;
  p->_clean_up = myfclose;
  return p;
}

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


static int remote_play(const board_t* board, const color_t color, struct move_t* mp, void* state) {
  FILE* fp = (FILE*) state;
  char buf[73];
  buf[0] = 'M';
  buf[1] = 'O';
  buf[2] = 'V';
  buf[3] = 'E';
  buf[4] = ' ';
//  fwrite("MOVE ", sizeof(char), 5, fp);
  write_board(board, &buf[5]);
  buf[69] = ' ';
  buf[70] = color2char(color);
  buf[71] = '\r';
  buf[72] = '\n';
  //puts(buf);

  fwrite(buf , sizeof(char), 73, fp);
  fread(buf, sizeof(char), 4, fp);

  read_move(buf, mp);
  return 0;
}

struct player_t* make_remote_player(const char* host, const char* port) {
  int sock = connect_to(host, port);
  FILE* fp = fdopen(sock, "r+");
  setbuf(fp, NULL);

  struct player_t* t = (struct player_t*) malloc(sizeof( struct player_t) );
  t->_hidden_state = fp;
  t->_play         = remote_play;
  t->_clean_up     = myfclose;

  return t;

}
