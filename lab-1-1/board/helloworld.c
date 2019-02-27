/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "helloworld.h"

int octetstr_rd( char* r, int n_r );
void octetstr_wr( const char* x, int n_x );

int main( int argc, char* argv[] ) {
  // initialise the development board, using the default configuration
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  // char x[] = "hello world";

  // while( true ) {
    // read  the GPI     pin, and hence switch : t   <- GPI
    bool t = scale_gpio_rd( SCALE_GPIO_PIN_GPI        );
    // write the GPO     pin, and hence LED    : GPO <- t
             scale_gpio_wr( SCALE_GPIO_PIN_GPO, t     );

    // write the trigger pin, and hence LED    : TRG <- 1 (positive edge)
             scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );
    // write the trigger pin, and hence LED    : TRG <- 0 (negative edge)
             scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );
    // delay for 500 ms = 1/2 s
    scale_delay_ms( 500 );



    char read_buf[128];
    int n_octs = octetstr_rd(read_buf, 128);
    octetstr_wr(read_buf, n_octs);

    
    // int n = strlen( x );

  //   // write x = "hello world" to the UART
  //   for( int i = 0; i < n; i++ ) {
  //     scale_uart_wr( SCALE_UART_MODE_BLOCKING, x[ i ] );
  //   }
  // }

  return 0;
}


int octetstr_rd(char* r, int n_r)  {

  char c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  char c2 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  int n1 = (c1 < 65)? c1-48:c1-55;
  n1*=16;
  int n2 = (c2 < 65)? c2-48:c2-55;
  int n = n1+n2;
  //Colon
  scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  for (int i = 0; i < 2*n; i+=2)  {
    r[i] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    r[i+1] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  }
  return n;
}

void octetstr_wr( const char* x, int n_x )  {
  int n1 = n_x / 16;
  int n2 = n_x % 16;
  char c1 = (n1<10)?n1+48:n1+55;
  char c2 = (n2<10)?n2+48:n2+55;

  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c1);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c2);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, 58);

  for (int i = 2*n_x-2; i>=0; i-=2)  {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i]);
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i+1]);
  }
}