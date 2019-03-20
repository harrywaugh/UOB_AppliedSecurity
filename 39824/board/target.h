/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __TARGET_H
#define __TARGET_H

#include <scale/scale.h>

#define COMMAND_INSPECT ( 0x00 )
#define COMMAND_ENCRYPT ( 0x01 )

#define SIZEOF_BLK      (   16 )
#define SIZEOF_KEY      (   16 )
#define SIZEOF_RND      (    0 )

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t aes_gf28_t;
typedef uint16_t aes_poly_t;

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
  s[a] = gf28_t_sbox(s[a]);                       \
  s[b] = gf28_t_sbox(s[b]);                       \
  s[c] = gf28_t_sbox(s[c]);                       \
  s[d] = gf28_t_sbox(s[d]);                       \
}


#endif
