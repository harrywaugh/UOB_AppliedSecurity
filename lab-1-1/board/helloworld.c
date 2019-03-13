/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "helloworld.h"

uint8_t octetstr_rd( char* r, uint8_t n_r );
void octetstr_wr( const char* x, uint8_t n_x );
void reverse_array( char *array, uint8_t n );

uint8_t hex_to_int(char c);
char int_to_hex(uint8_t i);

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



    char read_buf[128] = {0};
    uint8_t n = octetstr_rd(read_buf, 128);
    // reverse_array(read_buf, n);
    octetstr_wr(read_buf, n);

    
    // int n = strlen( x );

  //   // write x = "hello world" to the UART
  //   for( int i = 0; i < n; i++ ) {
  //     scale_uart_wr( SCALE_UART_MODE_BLOCKING, x[ i ] );
  //   }
  // }

  return 0;
}


uint8_t octetstr_rd(char* r, uint8_t n_r)  {
  // Reads first two characters, these are hex characters
  char c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  char c2 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  // Convert ASCII char to integer
  uint8_t n1 = hex_to_int(c1);
  uint8_t n2 = hex_to_int(c2);
  // First char is an order larger than second, so multiply by 16
  n1*=16;
  uint8_t n = n1+n2;
  //Colon
  //Read n bytes after colon
  scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  for (uint8_t i = 0; i < 2*n; i++)  { //Iterate by 2 each time, as each 2 characters represents a byte
    r[i] = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  }
  return n;
}

void octetstr_wr( const char* x, uint8_t n_x )  {
  uint8_t n1 = n_x / 16;
  uint8_t n2 = n_x % 16;
  char c1 = int_to_hex(n1);
  char c2 = int_to_hex(n2);

  //Print n bytes and a colon to UART
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c1);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, c2);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, 58);

  //Print string
  for (uint8_t i = 0; i < 2*n_x; i++)  {
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, x[i]);
  }
}

void reverse_array( char *array , uint8_t n)  {
  char swp0, swp1;
  for (uint8_t i = 0; i < (2*n)/2; i+=2)  {
    swp0 = array[i];
    swp1 = array[i+1];

    array[i]   = array[2*n-(i+2)];
    array[i+1] = array[2*n-(i+1)];

    array[2*n-(i+2)] = swp0;
    array[2*n-(i+1)] = swp1;
  }
}

uint8_t hex_to_int(char c)  {
  return (c < 65) ? c-48:c-55;
}

char int_to_hex(uint8_t i) {
  return (i<10) ? i+48 : i+55;
}