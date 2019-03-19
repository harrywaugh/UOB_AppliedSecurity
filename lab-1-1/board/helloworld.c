/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "helloworld.h"

uint8_t octetstr_rd( uint8_t* r, uint8_t n_r );
void octetstr_wr( const uint8_t* x, uint8_t n_x );
void reverse_array( uint8_t *array, uint8_t n );
void my_print(char *string)  {
  int len = strlen(string);
  for(int i = 0; i < len; i++)  {
    scale_uart_wr( SCALE_UART_MODE_BLOCKING, string[ i ] );
  }
}

uint8_t hex_to_int(char c);
uint8_t hex_to_int2(char c0, char c1);
char int_to_hex(uint8_t i);
void int_to_hex2(char* hex_chars, uint8_t i);

int main( int argc, char* argv[] ) {
  // initialise the development board, using the default configuration
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }
  uint8_t read_buf[128] = {0};

  while( true ) {
    // my_print("Enter Octet String: ");
    
    scale_gpio_wr( SCALE_GPIO_PIN_TRG, true  );



    uint8_t n = octetstr_rd(read_buf, 128);
    reverse_array(read_buf, n);
    octetstr_wr(read_buf, n);

    
    // my_print( "\x0D\x0A" );

    scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );

  }

  return 0;
}


uint8_t octetstr_rd(uint8_t* r, uint8_t n_r)  {
  // Reads first two characters, these are hex characters
  char c0 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  char c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  // Convert ASCII char to integer
  uint8_t n = hex_to_int2(c0, c1);
  //Colon
  scale_uart_rd(SCALE_UART_MODE_BLOCKING);

  //Read n bytes after colon
  for (uint8_t i = 0; i < n; i++)  { //Iterate by 2 each time, as each 2 characters represents a byte
    c0 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);

    uint8_t byte = hex_to_int2(c0, c1);

    r[i] = byte;
  }

  scale_uart_rd(SCALE_UART_MODE_BLOCKING);
  // scale_uart_rd(SCALE_UART_MODE_BLOCKING);

  return n;
}

void octetstr_wr( const uint8_t* x, uint8_t n_x )  {
  char hex_chars[2];

  int_to_hex2(hex_chars, n_x);

  //Print n bytes and a colon to UART
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_chars[0]);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_chars[1]);
  scale_uart_wr(SCALE_UART_MODE_BLOCKING, 58);

  //Print string
  for (uint8_t i = 0; i < n_x; i++)  {
    int_to_hex2(hex_chars, x[i]);
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_chars[0]);
    scale_uart_wr(SCALE_UART_MODE_BLOCKING, hex_chars[1]);
  }
  my_print( "\x0D" );
  // my_print( "\x0D\x0A" );

}

void reverse_array( uint8_t *array , uint8_t n)  {
  char swp;
  for (uint8_t i = 0; i < n/2; i++)  {
    swp = array[i];
    array[i] = array[n-i-1];
    array[n-i-1] = swp;
  }
}

uint8_t hex_to_int2(char c0, char c1)  {
  uint8_t n1 = hex_to_int(c0);
  uint8_t n2 = hex_to_int(c1);
  // First char is an order larger than second, so multiply by 16
  n1*=16;
  return n1+n2;
}

void int_to_hex2(char* hex_chars, uint8_t i)  {
  uint8_t i0 = i / 16;
  uint8_t i1 = i % 16;

  hex_chars[0] = int_to_hex(i0);
  hex_chars[1] = int_to_hex(i1);

}

uint8_t hex_to_int(char c)  {
  return (c < 65) ? c-48:c-55;
}

char int_to_hex(uint8_t i) {
  return (i<10) ? i+48 : i+55;
}