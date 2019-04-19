import numpy, struct, sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from scipy.stats import norm
import scipy
import mpld3
import brewer2mpl
from matplotlib.patches import Rectangle
import multiprocessing
# %load_ext line_profiler

hamming_weights = [0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8] ##Number of 1's in binary string [0..255]

sbox = numpy.array([ 0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30, 0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76,
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
                     0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41, 0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16 ], dtype=numpy.uint8)

def traces_ld( f ) :
  fd = open( f, "rb" )

  def rd( x ) :
    ( r, ) = struct.unpack( x, fd.read( struct.calcsize( x ) ) ) ; return r

  t = rd( '<I' )
  s = rd( '<I' )

  M = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  C = numpy.zeros( ( t, 16 ), dtype = numpy.uint8 )
  T = numpy.zeros( ( t,  s ), dtype = numpy.int16 )

  for i in range( t ) :
    for j in range( 16 ) :
      M[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( 16 ) :
      C[ i, j ] = rd( '<B' )

  for i in range( t ) :
    for j in range( s  ) :
      T[ i, j ] = rd( '<h' )

  fd.close()

  return t, s, M, C, T

def correlation_fn(H_col_diff, T_col_diff):
    HT_col_sum = np.sum(H_col_diff*T_col_diff)

    H_col_sq_sum  = np.sum(np.square(H_col_diff))
    T_col_sq_sum  = np.sum(np.square(T_col_diff))
    HT_sq_col     = H_col_sq_sum*T_col_sq_sum

    return HT_col_sum / (np.sqrt(HT_sq_col))

def crack_aes(t, s, M, C, T, ntraces=1000, key_bytes=16):
    start_sample = 0
    end_sample = 10000
    nsamples = end_sample-start_sample
    nkeys = 256
    best_samples, key_guess, best_corrs=[], [], []
    H = numpy.zeros((ntraces, 256), dtype = numpy.uint8) # Hypothetical power consumption values
    
    ## Precompute T col diffs for correlation
    T_col_diffs = np.empty((end_sample, ntraces))
    for si in range(start_sample, end_sample):
        col = T[:ntraces, si]
        T_mean = np.mean(col)
        T_col_diffs[si] = col-T_mean
    
    window_start = start_sample
    window_end   = end_sample
    ## For each byte in the key
    for b in range(key_bytes):
        print("\n\nBREAKING BYTE", b)
        
        ## Get hypothetical power consumptions
        for ti in range(ntraces):
            for ki in range(nkeys):
                H[ti, ki] = hamming_weights[sbox[M[ti, b] ^ ki]]

        max_correlation = 0
        byte = 0
        sample = 0
        ## Check each key's correlation across the sample range
        for ki in range(nkeys):
            # Precompute H column for correlation
            H_col = H[:, ki]
            H_col_diff = H_col - np.mean(H_col)
            for si in range(window_start, window_end):
                ## Check correlation
                correlation = numpy.abs(correlation_fn(H_col_diff, T_col_diffs[si]))
                if (correlation > max_correlation):
                    max_correlation = correlation
                    byte = ki
                    sample=si
        window_start = sample-500
        window_end = sample+500
        key_guess.append(byte)
        best_samples.append(sample)
        best_corrs.append(max_correlation)
        print(key_guess)        
        print(best_samples)        
        print(best_corrs)        
    print(key_guess)        
    print(best_samples) 
    print(best_corrs)  
        
    print("KEY GUESS", key_guess)    
    print("BEST_SAMPLES", best_samples)

def attack( argc, argv ):
  print("Loading data")
  t, s, M, C, T = traces_ld( argv[1] );

  print("Loaded data")
  print("Message Shape: ", M.shape)
  print("Traces Shape: ", T.shape)

  crack_aes(t, s, M, C, T, 200, 16)



if ( __name__ == '__main__' ) :
  attack( len( sys.argv ), sys.argv )
