/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "target.h" 

/** Read  an octet string (or sequence of bytes) from the UART, using a simple
  * length-prefixed, little-endian hexadecimal format.
  * 
  * \param[out] r the destination octet string read
  * \return       the number of octets read
  */

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


/** Initialise an AES-128 encryption, e.g., expand the cipher key k into round
  * keys, or perform randomised pre-computation in support of a countermeasure;
  * this can be left blank if no such initialisation is required, because the
  * same k and r will be passed as input to the encryption itself.
  * 
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes_init(                               const uint8_t* k, const uint8_t* r ) {
  return;
}

/** Perform    an AES-128 encryption of a plaintext m under a cipher key k, to
  * yield the corresponding ciphertext c.
  * 
  * \param[out] c   an   AES-128 ciphertext
  * \param[in]  m   an   AES-128 plaintext
  * \param[in]  k   an   AES-128 cipher key
  * \param[in]  r   some         randomness
  */

void aes     ( uint8_t* c, const uint8_t* m, const uint8_t* k, const uint8_t* r ) {
  return;
}

/** Initialise the SCALE development board, then loop indefinitely, reading a
  * command then processing it:
  *
  * 1. If command is inspect, then
  *
  *    - write the SIZEOF_BLK parameter,
  *      i.e., number of bytes in an  AES-128 plaintext  m, or ciphertext c,
  *      to the UART, 
  *    - write the SIZEOF_KEY parameter,
  *      i.e., number of bytes in an  AES-128 cipher key k,
  *      to the UART, 
  *    - write the SIZEOF_RND parameter,
  *      i.e., number of bytes in the         randomness r.
  *      to the UART.
  *
  * 2. If command is encrypt, then
  *
  *    - read  an   AES-128 plaintext  m from the UART,
  *    - read  some         randomness r from the UART,
  *    - initalise the encryption,
  *    - set the trigger signal to 1,
  *    - execute   the encryption, producing the ciphertext 
  *
  *      c = AES-128.Enc( m, k )
  *
  *      using the hard-coded cipher key k plus randomness r if/when need be,
  *    - set the trigger signal to 0,
  *    - write an   AES-128 ciphertext c to   the UART.
  */

int main( int argc, char* argv[] ) {
  if( !scale_init( &SCALE_CONF ) ) {
    return -1;
  }

  uint8_t cmd[ 1 ], c[ SIZEOF_BLK ], m[ SIZEOF_BLK ], k[ SIZEOF_KEY ] = { 0x80, 0xCE, 0xFC, 0x6C, 0x78, 0x33, 0xDA, 0xB0, 0x8A, 0x31, 0xA5, 0x69, 0x04, 0x70, 0x77, 0x67 }, r[ SIZEOF_RND ];

  while( true ) {
    if( 1 != octetstr_rd( cmd, 1 ) ) {
      break;
    }

    switch( cmd[ 0 ] ) {
      case COMMAND_INSPECT : {
        uint8_t t = SIZEOF_BLK; 
                    octetstr_wr( &t, 1 ); 
                t = SIZEOF_KEY; 
                    octetstr_wr( &t, 1 ); 
                t = SIZEOF_RND; 
                    octetstr_wr( &t, 1 ); 

        break;
      }
      case COMMAND_ENCRYPT : {
        if( SIZEOF_BLK != octetstr_rd( m, SIZEOF_BLK ) ) {
          break;
        }
        if( SIZEOF_RND != octetstr_rd( r, SIZEOF_RND ) ) {
          break;
        }

        aes_init(       k, r );

        scale_gpio_wr( SCALE_GPIO_PIN_TRG,  true );
        aes     ( c, m, k, r );
        scale_gpio_wr( SCALE_GPIO_PIN_TRG, false );

                          octetstr_wr( c, SIZEOF_BLK );

        break;
      }
      default : {
        break;
      }
    }
  }

  return 0;
}
