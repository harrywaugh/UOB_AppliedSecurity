/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */
#include "target.h" 

#define Nb 4

typedef uint8_t aes_gf28_t;
typedef uint16_t aes_poly_t;

unsigned char sbox[256] = 
{
  0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
  0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD, 0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0,
  0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34, 0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15,
  0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07, 0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75,
  0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52, 0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84,
  0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A, 0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF,
  0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45, 0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8,
  0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC, 0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2,
  0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4, 0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73,
  0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46, 0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB,
  0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2, 0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79,
  0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C, 0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08,
  0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8, 0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A,
  0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61, 0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E,
  0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B, 0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF,
  0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16
};

void aes_enc(uint8_t *c, uint8_t *m, uint8_t *k);
void aes_enc_mix_columns(aes_gf28_t *s);
void aes_enc_shift_rows(aes_gf28_t* s);
void aes_enc_sub_bytes(aes_gf28_t* s);
void aes_enc_key_add( aes_gf28_t* s, aes_gf28_t* rk );
void aes_enc_exp_step( aes_gf28_t* rk, uint8_t rc );
aes_gf28_t gf28_t_sbox( aes_gf28_t a );
aes_gf28_t gf28_t_inv( aes_gf28_t a );
aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b );
aes_gf28_t gf28_t_mulx( aes_gf28_t a );


uint8_t octetstr_rd( uint8_t* r, uint8_t n_r );
void octetstr_wr( const uint8_t* x, uint8_t n_x );
void reverse_array( uint8_t *array, uint8_t n );
uint8_t hex_to_int(char c);
uint8_t hex_to_int2(char c0, char c1);
char int_to_hex(uint8_t i);
void int_to_hex2(char* hex_chars, uint8_t i);
void my_print(char *string);

#define AES_ENC_MIX_STEP(a,b,c,d) {      \
  aes_gf28_t __a1 = s[ a ];                  \
  aes_gf28_t __b1 = s[ b ];                  \
  aes_gf28_t __c1 = s[ c ];                  \
  aes_gf28_t __d1 = s[ d ];                  \
                                             \
  aes_gf28_t __a2 = gf28_t_mulx( __a1 );     \
  aes_gf28_t __b2 = gf28_t_mulx( __b1 );     \
  aes_gf28_t __c2 = gf28_t_mulx( __c1 );     \
  aes_gf28_t __d2 = gf28_t_mulx( __d1 );     \
                                             \
  aes_gf28_t __a3 = __a1 ^ __a2;             \
  aes_gf28_t __b3 = __b1 ^ __b2;             \
  aes_gf28_t __c3 = __c1 ^ __c2;             \
  aes_gf28_t __d3 = __d1 ^ __d2;             \
                                             \
  s[a] = __a2 ^ __b3 ^ __c1 ^ __d1;          \
  s[b] = __a1 ^ __b2 ^ __c3 ^ __d1;          \
  s[c] = __a1 ^ __b1 ^ __c2 ^ __d3;          \
  s[d] = __a3 ^ __b1 ^ __c1 ^ __d2;          \
}

#define AES_ENC_SHIFT_STEP(a, b, c, d, e, f, g, h)  { \
  aes_gf28_t __a1 = s[ a ];                  \
  aes_gf28_t __b1 = s[ b ];                  \
  aes_gf28_t __c1 = s[ c ];                  \
  aes_gf28_t __d1 = s[ d ];                  \
                                             \
  s[ e ] = __a1;                             \
  s[ f ] = __b1;                             \
  s[ g ] = __c1;                             \
  s[ h ] = __d1;                             \
}

#define AES_KEY_ADD_STEP(a, b, c, d)  { \
  s[a] = s[a] ^ rk[a];                       \
  s[b] = s[b] ^ rk[b];                       \
  s[c] = s[c] ^ rk[c];                       \
  s[d] = s[d] ^ rk[d];                       \
}

#define AES_ENC_SUB_STEP(a, b, c, d)  { \
  s[a] = sbox[s[a]];                       \
  s[b] = sbox[s[b]];                       \
  s[c] = sbox[s[c]];                       \
  s[d] = sbox[s[d]];                       \
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
  aes_gf28_t AEC_RC[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
  aes_gf28_t rk[ 16 ], s[ 16 ];  // Declare 'current' round key and state matrices
  
  aes_gf28_t *rcp = AEC_RC;              // Declare pointer to the round constant
  aes_gf28_t* rkp = rk;                  // Declare pointer to rk current round key matrix


  memcpy(s, m, 16);
  memcpy(rkp, k, 16);


  aes_enc_key_add(s, rkp);


  for ( int r = 1; r < 10; ++r )  {
    aes_enc_sub_bytes(s);
    aes_enc_shift_rows(s);
    aes_enc_mix_columns(s);

    aes_enc_exp_step(rkp, *(++rcp));
    aes_enc_key_add(s, rkp);
  }

  aes_enc_sub_bytes(s);
  aes_enc_shift_rows(s);

  aes_enc_exp_step(rkp, *(++rcp));
  aes_enc_key_add(s, rkp);
  
  memcpy(c, s, 16);
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

  uint8_t cmd[ 1 ], c[ SIZEOF_BLK ], m[ SIZEOF_BLK ], r[ SIZEOF_RND ];
  uint8_t k[ SIZEOF_KEY ] = { 0xCD, 0X97, 0X16, 0XE9, 0X5B, 0X42, 0XDD, 0X48, 0X69, 0X77, 0X2A, 0X34, 0X6A, 0X7F, 0X58, 0X13};

  while( true ) {
    // my_print("Inspect (01:00) or ENCRYPT (01:01): ");

    if( 1 != octetstr_rd( cmd, 1 ) ) {
      break;
    }
    switch( cmd[ 0 ] ) {
      case COMMAND_INSPECT : {
        // my_print("Inspect: ");

        uint8_t t = SIZEOF_BLK; 
        octetstr_wr( &t, 1 ); 
        t = SIZEOF_KEY; 
        octetstr_wr( &t, 1 ); 
        t = SIZEOF_RND; 
        octetstr_wr( &t, 1 ); 

        break;
      }
      case COMMAND_ENCRYPT : {
        // my_print("Encrypt: ");

        if( SIZEOF_BLK != octetstr_rd( m, SIZEOF_BLK ) ) {
          break;
        }
        if( SIZEOF_RND != octetstr_rd( r, SIZEOF_RND ) ) {
          break;
        }

        aes_init( k, r );

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

void my_print(char *string)  {
  int len = strlen(string);
  for(int i = 0; i < len; i++)  {
    scale_uart_wr( SCALE_UART_MODE_BLOCKING, string[ i ] );
  }
  scale_uart_wr( SCALE_UART_MODE_BLOCKING, '\x0D');
  scale_uart_wr( SCALE_UART_MODE_BLOCKING, '\x0A');


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
  for (uint8_t i = 0; i < n && i < n_r; i++)  { //Iterate by 2 each time, as each 2 characters represents a byte
    c0 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);
    c1 = scale_uart_rd(SCALE_UART_MODE_BLOCKING);

    uint8_t byte = hex_to_int2(c0, c1);
    r[i] = byte;
  }

  scale_uart_rd(SCALE_UART_MODE_BLOCKING);

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
  scale_uart_wr( SCALE_UART_MODE_BLOCKING, '\x0D');
  // scale_uart_wr( SCALE_UART_MODE_BLOCKING, '\x0A');
}





// // int main( int argc, char* argv[] ) {
// //   uint8_t k[ 16 ] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
// //                       0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
// //   uint8_t m[ 16 ] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
// //                       0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };

// //   uint8_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
// //                       0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };

// //   // uint8_t k1[ 16 ] = { 0x54, 0x68, 0x61, 0x74, 0x73, 0x20, 0x6D, 0x79, 
// //   //                     0x20, 0x4B, 0x75, 0x6E, 0x67, 0x20, 0x46, 0x75 };
// //   // uint8_t m1[ 16 ] = { 0x54, 0x77, 0x6F, 0x20, 0x4F, 0x6E, 0x65, 0x20,
// //   //                     0x4E, 0x69, 0x6E, 0x65, 0x20, 0x54, 0x77, 0x6F };

// //   // uint8_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
// //   //                     0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };

// //   uint8_t t[ 16 ];

// //   AES_KEY rk;

// //   AES_set_encrypt_key( k, 128, &rk );
// //   AES_encrypt( m, t, &rk );
// //   // aes_enc(t, m, k);

// //   if( !memcmp( t, c, 16 * sizeof( uint8_t ) ) ) {
// //     printf( "AES.Enc( k, m ) == c\n" );
// //   }
// //   else {
// //     printf( "AES.Enc( k, m ) != c\n" );
// //   }
// // }


// void aes_enc(uint8_t* c, uint8_t* m, uint8_t* k)  {
//   aes_gf28_t AEC_RC[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};
//   aes_gf28_t rk[ 16 ], s[ 16 ];  // Declare 'current' round key and state matrices
  
//   aes_gf28_t *rcp = AEC_RC;              // Declare pointer to the round constant
//   aes_gf28_t* rkp = rk;                  // Declare pointer to rk current round key matrix


//   memcpy(s, m, 16);
//   memcpy(rkp, k, 16);


//   aes_enc_key_add(s, rkp);


//   for ( int r = 1; r <= 9; ++r )  {
//     aes_enc_sub_bytes(s);
//     aes_enc_shift_rows(s);
//     aes_enc_mix_columns(s);
//     aes_enc_exp_step(rkp, *(++rcp));
    
//     aes_enc_key_add(s, rkp);


//   }

//   aes_enc_sub_bytes(s);
//   aes_enc_shift_rows(s);
//   aes_enc_exp_step(rkp, *(++rcp));

//   aes_enc_key_add(s, rkp);
//   memcpy(c, s, 16);
// }

void aes_enc_mix_columns(aes_gf28_t *s)  {
  for ( int i = 0; i < 4; i++ , s+=4 ) 
    AES_ENC_MIX_STEP(0, 1, 2, 3);
}

void aes_enc_shift_rows(aes_gf28_t *s)  {
  AES_ENC_SHIFT_STEP(1,  5,  9, 13,
                     13,  1, 5,  9);
  AES_ENC_SHIFT_STEP(2,  6, 10, 14,
                     10, 14, 2,  6);
  AES_ENC_SHIFT_STEP(3, 7, 11, 15,
                     7, 11, 15, 3);
} 

//Performs sbox element-wise on state matrix 
void aes_enc_sub_bytes(aes_gf28_t* s)  {
  AES_ENC_SUB_STEP(0, 1, 2, 3);
  AES_ENC_SUB_STEP(4, 5, 6, 7);
  AES_ENC_SUB_STEP(8, 9, 10, 11);
  AES_ENC_SUB_STEP(12, 13, 14, 15);
  // for ( uint8_t i = 0; i < 16; ++i)
  //   s[i] = gf28_t_sbox(s[i]);
}

//Takes state matrix and a round key, and performs element-wise XOR
void aes_enc_key_add(aes_gf28_t* s, aes_gf28_t* rk )  {
  AES_KEY_ADD_STEP(0, 1, 2, 3);
  AES_KEY_ADD_STEP(4, 5, 6, 7);
  AES_KEY_ADD_STEP(8, 9, 10, 11);
  AES_KEY_ADD_STEP(12, 13, 14, 15);
  // for(uint8_t = 0; i < 16; ++i ) 
  //   s[i] ^= rk[i];
}

//Given ith round key and constant, it calculates the (i+1)th round key 
void aes_enc_exp_step(aes_gf28_t* rk, uint8_t rc)  {
  rk[0]  = rc ^ sbox[(rk[13])] ^ rk[0];
  rk[1]  =      sbox[(rk[14])] ^ rk[1];
  rk[2]  =      sbox[(rk[15])] ^ rk[2];
  rk[3]  =      sbox[(rk[12])] ^ rk[3];

  rk[4]  =      rk[0]               ^ rk[4];
  rk[5]  =      rk[1]               ^ rk[5];
  rk[6]  =      rk[2]               ^ rk[6];
  rk[7]  =      rk[3]               ^ rk[7];

  rk[8]  =      rk[4]               ^ rk[8];
  rk[9]  =      rk[5]               ^ rk[9];
  rk[10] =      rk[6]               ^ rk[10];
  rk[11] =      rk[7]               ^ rk[11];

  rk[12] =      rk[8]               ^ rk[12];
  rk[13] =      rk[9]               ^ rk[13];
  rk[14] =      rk[10]              ^ rk[14];
  rk[15] =      rk[11]              ^ rk[15];
}

// //Calculates sbox of a, this is the inverse of a, followed by affine transformation, f
// aes_gf28_t gf28_t_sbox( aes_gf28_t a ) {
//   a = gf28_t_inv(a);

//   a =   0x63     ^
//       ( a      ) ^
//       ( a << 1 ) ^
//       ( a << 2 ) ^
//       ( a << 3 ) ^
//       ( a << 4 ) ^
//       ( a >> 7 ) ^
//       ( a >> 6 ) ^
//       ( a >> 5 ) ^
//       ( a >> 4 );
//   return a;
// }

// aes_gf28_t gf28_t_sbox( aes_gf28_t a ) { 

// }

//Calculates inverse of a
aes_gf28_t gf28_t_inv( aes_gf28_t a )  {

  //Aim is to find inverse of a, a^q = a, a^(q-1) = 1, a^(q-2) = a^(-1). q = 2^8(FF)
  aes_gf28_t t0 = gf28_t_mul(a, a); //t0 = a^2

  aes_gf28_t t1 = gf28_t_mul(t0, a); //t1 = a^3

  t0 = gf28_t_mul(t0, t0);           //t0 = a^4
  t1 = gf28_t_mul(t1, t0);           //t1 = a^7
  t0 = gf28_t_mul(t0, t0);           //t0 = a^8
  t0 = gf28_t_mul(t1, t0);           //t0 = a^15
  t0 = gf28_t_mul(t0, t0);           //t0 = a^30
  t0 = gf28_t_mul(t0, t0);           //t0 = a^60
  t1 = gf28_t_mul(t1, t0);           //t1 = a^67
  t0 = gf28_t_mul(t1, t0);           //t1 = a^127
  return gf28_t_mul(t0, t0);         //t1 = a^254 = a^(2^8)
} 

//Multiplies ff a and ff b
aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b)  {
  aes_gf28_t t = 0; // Initialise polynomial result, needs a max of 16 bits

  for ( uint8_t i = 7; i < 8; --i )  {
    t = gf28_t_mulx(t); //Multiply result by x

    if ( (b >> i) & 1 ) // Right shift polyn b by i, if there is a 1. XOR= result with a
      t ^= a;
  }
  return t;
}

//Multiplies ff a by x
aes_gf28_t gf28_t_mulx(aes_gf28_t a) {
  if( (a & 0x80) == 0x80) //If 8th bit is on, then it'll be shifted out of range, so reduce
    return (a << 1) ^ 0x1b; //0x1b is identity mod p(x), x^8 = x^4+x^3+x+1
  else 
    return (a << 1);
}





