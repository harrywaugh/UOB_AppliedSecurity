/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#include "encrypt.h"

void aes_enc_exp_step(aes_gf28_t* rk, gf_28_k rc);
aes_gf28_t gf28_t_sbox( aes_gf28_t a );
aes_gf28_t gf28_t_inv( aes_gf28_t a );
aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b);
aes_gf28_t gf28_t_mulx( aes_gf28_t a );

int main( int argc, char* argv[] ) {
  uint8_t k[ 16 ] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                      0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };
  uint8_t m[ 16 ] = { 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D,
                      0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34 };
  uint8_t c[ 16 ] = { 0x39, 0x25, 0x84, 0x1D, 0x02, 0xDC, 0x09, 0xFB,
                      0xDC, 0x11, 0x85, 0x97, 0x19, 0x6A, 0x0B, 0x32 };
  uint8_t t[ 16 ];

  AES_KEY rk;

  AES_set_encrypt_key( k, 128, &rk );
  AES_encrypt( m, t, &rk );  

  if( !memcmp( t, c, 16 * sizeof( uint8_t ) ) ) {
    printf( "AES.Enc( k, m ) == c\n" );
  }
  else {
    printf( "AES.Enc( k, m ) != c\n" );
  }
}


void aes_enc_exp_step(aes_gf28_t* rk, gf_28k rc)  {
  rk[0]  = rc ^ gf_28k_sbox(rk[13]) ^ rk[0];
  rk[1]  =      gf_28k_sbox(rk[14]) ^ rk[1];
  rk[2]  =      gf_28k_sbox(rk[15]) ^ rk[2];
  rk[3]  =      gf_28k_sbox(rk[12]) ^ rk[3];

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

aes_gf28_t gf28_t_sbox( aes_gf28_t a ) {
  a = gf28_t_inv(a);
  a =   0x63     ^
      ( a      ) ^
      ( a << 1 ) ^
      ( a << 2 ) ^
      ( a << 3 ) ^
      ( a << 4 ) ^
      ( a >> 7 ) ^
      ( a >> 6 ) ^
      ( a >> 5 ) ^
      ( a >> 4 );
  return a;
}

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

aes_gf28_t gf28_t_mul( aes_gf28_t a,  aes_gf28_t b)  {
  aes_poly_t t = 0; // Initialise polynomial result, needs a max of 16 bits

  for ( uint8_t i = 7; i >= 0; --i )  {
    t = gf28_t_mulx(t); //Multiply result by x

    if ( (b >> i) & 1 ) // Right shift polyn b by i, if there is a 1. XOR= result with a
      t ^= a;
  }
}

aes_gf28_t gf28_t_mulx(aes_gf28_t a) {
  if( (a & 0x80) == 0x80) //If 8th bit is on, then it'll be shifted out of range, so reduce
    return (a << 1) ^ 0x1b; //0x1b is identity mod p(x), x^8 = x^4+x^3+x+1
  else 
    return (a << 1);
}


