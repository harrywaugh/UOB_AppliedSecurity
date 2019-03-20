/* Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
 * which can be found via http://creativecommons.org (and should be included as 
 * LICENSE.txt within the associated archive or repository).
 */

#ifndef __ENCRYPT_H
#define __ENCRYPT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef uint8_t aes_gf28_t;
typedef uint16_t aes_poly_t;

#endif



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