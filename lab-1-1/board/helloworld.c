/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "helloworld.h"

int octetstr_rd( char* r, int n_r );
void octetstr_wr( const char* x, int n_x );
void reverse_array( char *array, int n );

int char_to_int(char c);
char int_to_char(int i);

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
    reverse_array(read_buf, n_octs);
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
  // Reads first two characters, these are hex characters
  char c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  char c2 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  // Convert ASCII char to integer
  int n1 = char_to_int(c1);
  int n2 = char_to_int(c2);
  // First char is an order larger than second, so multiply by 16
  n1*=16;
  int n = n1+n2;
  //Colon
  //Read n bytes
  scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  for (int i = 0; i < 2*n; i+=2)  { //Iterate by 2 each time, as each 2 characters represents a byte
    r[i] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    r[i+1] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  }
  return n;
}

void octetstr_wr( const char* x, int n_x )  {
  int n1 = n_x / 16;
  int n2 = n_x % 16;
  char c1 = int_to_char(n1);
  char c2 = int_to_char(n2);

  //Print n bytes and a colon to UART
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c1);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c2);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, 58);

  //Print string
  for (int i = 0; i < 2*n_x; i+=2)  {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i]);
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i+1]);
  }
}

void reverse_array( char *array , int n)  {
  char swp;
  for (int i = 0; i  < n/2; i++)  {
    swp = array[i];
    array[i] = array[n-i];
    array[n-i] = swp;
  }
}

int char_to_int(char c)  {
  return (c < 65) ? c-48:c-55;
}

char int_to_char(int i) {
  return (i<10) ? i+48 : i+55
}