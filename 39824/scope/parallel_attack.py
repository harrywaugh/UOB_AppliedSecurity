import numpy, struct, sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from scipy.stats import norm
import scipy
from matplotlib.patches import Rectangle
import multiprocessing
import time
from multiprocessing import Process, Value, Array
numpy.seterr(divide='ignore', invalid='ignore')
import timeit
# %load_ext line_profiler

import binascii, select, serial, socket, sys, picoscope.ps2000a as ps2000a, time, numpy, argparse

PS2000A_RATIO_MODE_NONE      = 0 # Section 3.18.1

def scope_adc2volts( range, x ) :
    return ( float( x ) / float( scope_adc_max ) ) * range;

def scope_volts2adc( range, x ) :
    return ( float( x ) * float( scope_adc_max ) ) / range;

def str2seq( x ) :
    return          [ ord( t ) for t in x ]

def seq2str( x ) :
    return ''.join( [ chr( t ) for t in x ] )


def str2octetstr( x ) :
  return ( '%02X' % ( len( x ) ) ) + ':' + ( binascii.b2a_hex( x ) )

def octetstr2str( x ) :
  t = x.split( ':' ) ; n = int( t[ 0 ], 16 ) ; x = binascii.a2b_hex( t[ 1 ] )

  if( n != len( x ) ) :
    raise ValueError
  else :
    return x

def board_open() :
  if   ( args.mode == 'uart'   ) :
    fd = serial.Serial( port = args.uart, baudrate = 9600, bytesize = serial.EIGHTBITS, parity = serial.PARITY_NONE, stopbits = serial.STOPBITS_ONE, timeout = None )
  elif ( args.mode == 'socket' ) :
    fd = socket.socket( socket.AF_INET, socket.SOCK_STREAM ) ; fd.connect( ( args.host, args.port ) ) ; fd = fd.makefile( mode = 'rwb', bufsize = 1024 )

  return fd

def board_close( fd ) :
    fd.close()


def board_rdln( fd    ) :
    r = ''

    while( True ):
        t = fd.read( 1 )
        if( t == '\x0D' ) :
            break
        else:
            r += t
    return r

def board_wrln( fd, x ) :
  fd.write( x + '\x0D' ) ; fd.flush()


def get_traces(ntraces) :


    # Section 3.32, Page 60; Step  1: open  the oscilloscope
    scope = ps2000a.PS2000a()

    fd = board_open()

    t = ntraces
    s = 10000
    T = numpy.zeros((t, s))
    M = numpy.zeros((t, 16), dtype=np.uint8)
    C = numpy.zeros((t, 16), dtype=np.uint8)
    time.sleep( 1 )

        # # Section 3.28, Page 56
    scope_adc_min = scope.getMinValue()
    # # Section 3.30, Page 58
    scope_adc_max = scope.getMaxValue()

    # # Section 3.39, Page 69; Step  2: configure channels
    scope.setChannel( channel = 'A', enabled = True, coupling = 'DC', VRange =   5.0E-0 )
    scope_range_chan_a =   5.0e-0
    scope.setChannel( channel = 'B', enabled = True, coupling = 'DC', VRange = 500.0E-3 )
    scope_range_chan_b = 500.0e-3

    sample_interval = 4.0e-9

    # Section 3.13, Page 36; Step  3: configure timebase
    ( _, samples, samples_max ) = scope.setSamplingInterval( 4.0e-9, 2*s * sample_interval)
# 
    # # Section 3.  56, Page 93; Step  4: configure trigger
    scope.setSimpleTrigger( 'A', threshold_V = 2.0E-0, direction = 'Rising', timeout_ms = 0 )

    for i in range(t):

        print("Getting Trace", i)



        m = np.random.randint(256, size=16)

        # Section 3.37, Page 65; Step  5: start acquisition
        scope.runBlock()

        board_wrln( fd, '01:01' )
        board_wrln( fd, str2octetstr( seq2str( m ) ) )
        board_wrln( fd, '00:' )
        # Section 3.26, Page 54; Step  6: wait for acquisition to complete
        while ( not scope.isReady() ) : time.sleep( 1 )

        c = str2seq( octetstr2str( board_rdln( fd ) ) )

        M[i] = m
        C[i] = c

        # print("Message", m)
        # print("Ciphertext", c)


        # Section 3.40, Page 71; Step  7: configure buffers
        # Section 3.18, Page 43; Step  8; transfer  buffers
        ( A, _, _ ) = scope.getDataRaw( channel = 'A', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )
        ( B, _, _ ) = scope.getDataRaw( channel = 'B', numSamples = samples, downSampleMode = PS2000A_RATIO_MODE_NONE )

        # # Section 3.2,  Page 25; Step 10: stop  acquisition
        scope.stop()

        T[i] = B[:s]

        # fig, ax = plt.subplots()
        # ax.plot(T[i])
        # plt.show()
    scope.close() 

    board_close( fd )

    # Section 3.2,  Page 25; Step 13: close the oscilloscope

    return t, s, M, C, T


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

def traces_ld( f, ntraces=1000) :
  fd = open( f, "rb" )

  def rd( x ) :
    ( r, ) = struct.unpack( x, fd.read( struct.calcsize( x ) ) ) ; return r

  t = rd( '<I' )
  s = rd( '<I' )

  # if(override_n != None):
  	# t = override_n

  M = numpy.zeros( ( ntraces, 16 ), dtype = numpy.uint8 )
  C = numpy.zeros( ( ntraces, 16 ), dtype = numpy.uint8 )
  T = numpy.zeros( ( ntraces,  s ), dtype = numpy.int16 )

  for i in range( ntraces ) :
    for j in range( 16 ) :
      M[ i, j ] = rd( '<B' )

  for i in range( ntraces ) :
    for j in range( 16 ) :
      C[ i, j ] = rd( '<B' )

  for i in range( ntraces ) :
    for j in range( s  ) :
      T[ i, j ] = rd( '<h' )

  fd.close()

  return ntraces, s, M, C, T

def correlation_fn(H_col_diff, T_col_diff):
    HT_col_sum = np.sum(H_col_diff*T_col_diff)

    H_col_sq_sum  = np.sum(np.square(H_col_diff))
    T_col_sq_sum  = np.sum(np.square(T_col_diff))
    HT_sq_col     = H_col_sq_sum*T_col_sq_sum

    return HT_col_sum / (np.sqrt(HT_sq_col))

def corr2_coeff(A,B):
    # Rowwise mean of input arrays & subtract from input arrays themeselves
    A_mA = A - A.mean(1)[:,None]
    B_mB = B - B.mean(1)[:,None]

    # Sum of squares across rows
    ssA = (A_mA**2).sum(1);
    ssB = (B_mB**2).sum(1);

    # Finally get corr coeff
    return np.dot(A_mA,B_mB.T)/np.sqrt(np.dot(ssA[:,None],ssB[None]))


def generate_correlation_map(x, y):
    """Correlate each n with each m.

    Parameters
    ----------
    x : np.array
      Shape N X T.

    y : np.array
      Shape M X T.

    Returns
    -------
    np.array
      N X M array in which each element is a correlation coefficient.

    """
    mu_x = x.mean(axis=1)
    mu_y = y.mean(axis=1)



    n = x.shape[1]
    if n != y.shape[1]:
        raise ValueError('x and y must have the same number of timepoints.')
    s_x = x.std(axis=1, ddof=n-1)
    s_y = y.std(axis=1, ddof=n-1)

    xy = np.matmul(x, y.T)

    mu_xy = np.dot(mu_x[:, np.newaxis], mu_y[np.newaxis, :])


    cov = xy - n * mu_xy
    return cov / np.dot(s_x[:, np.newaxis], s_y[np.newaxis, :])


def crack_aes(M, T, ntraces=1000, start_byte=0, end_byte=16, key_guess=[]):
    start_sample = 0
    end_sample = 8000
    nsamples = end_sample-start_sample
    nkeys = 256
    window_behind = 1000
    window_ahead = 1000
    best_samples, best_corrs=[], []
    H = numpy.zeros((ntraces, 256), dtype = numpy.uint8) # Hypothetical power consumption values
    
    # Precompute T col diffs for correlation
    T_col_diffs = np.empty((end_sample, ntraces))
    for si in range(start_sample, end_sample):
        col = T[:ntraces, si]
        T_mean = np.mean(col)
        T_col_diffs[si] = col-T_mean
    
    window_start = start_sample
    window_end   = end_sample

    
    ## For each byte in the key
    for b in range(start_byte, end_byte):
        
        ## Get hypothetical power consumptions
        for ti in range(ntraces):
            for ki in range(nkeys):
                H[ti, ki] = hamming_weights[sbox[M[ti, b] ^ ki]]

        max_correlation = 0
        byte = 0
        sample = 0

        T2 = T[:ntraces, window_start:window_end]

        jorge   = np.empty((nkeys, window_end-window_start))
        # pearson = np.empty((nkeys, window_end-window_start))
        # mine    = np.empty((nkeys, window_end-window_start))

        # for ki in range(nkeys):
        #   for si in range(window_end-window_start):
        #       pearson[ki, si] = scipy.stats.pearsonr(H[:, ki], T[:ntraces, si])[0]

        ## Check each key's correlation across the sample range
        # start = timeit.default_timer()
        jorge = np.abs(corr2_coeff(H.T, T2.T))
        # stop = timeit.default_timer()
        # print("Jorje time", stop-start)
        # start = timeit.default_timer()

        # for ki in range(nkeys):
        #     # Precompute H column for correlation
        #     H_col = H[:, ki]
        #     H_col_diff = H_col - np.mean(H_col)
        #     for si in range(window_start, window_end):
        #         ## Check correlation
        #         correlation = np.abs(correlation_fn(H_col_diff, T_col_diffs[si]))
        #         mine[ki, si-window_start] = correlation
        #         if (correlation > max_correlation):
        #             max_correlation = correlation
        #             byte            = ki
        #             sample          = si
        # stop = timeit.default_timer()
        # print("Mine time", stop-start)


        # print("My Best Sample", sample)
        byte = np.nanargmax(np.nanmax(jorge, axis=1))
        print("Jorje Best Key", byte)

        print("Jorje",   jorge.shape)
        # print("Pearson", pearson.shape)
        # print("Mine",    mine.shape)


        # for ki in range(nkeys):
        #   plt.plot(mine[ki, :],    'b')
        #   # plt.plot(pearson[ki, :], 'r')
        #   plt.plot(jorge[ki, :],   'g')
        # plt.show()  

        # for ki in range(nkeys):
        #   for si  in range(window_end -window_start):
        #     if((not np.isinf(R[ki, si])) and R[ki, si] > max_correlation):
        #       s = R[ki, si]
        #       byte = ki  

        # print("Equal", np.testing.assert_array_almost_equal(R, desired))

        

        # window_start = sample-window_behind
        # window_end = sample+window_ahead
        key_guess[b%(end_byte-start_byte)] = byte
        # best_samples.append(sample)
        # best_corrs.append(max_correlation)
        print("BROKEN BYTE", b , "=", byte)
        
    # print("\n\nKEY GUESS", key_guess[:])    
    # print("BEST_SAMPLES", best_samples)
    # print("BEST_CORRS", best_corrs)


def worker(M, T, ntraces, key_start, key_end, keys):
  crack_aes( M, T, ntraces=ntraces, start_byte=key_start, end_byte=key_end, key_guess=keys)

def attack(argc, argv):
  
  ntraces = 200
  if(args.file == None):
    print("Getting traces from board")
    t, s, M, C, T = get_traces(ntraces=ntraces)
  else:
    print("Loading traces from file")
    t, s, M, C, T = traces_ld( args.file, ntraces=1000);




  print("Loaded data")
  print("Message Shape: ", M.shape)
  print("Traces Shape: ", T.shape)


  start = timeit.default_timer()
  nworkers = 2
  keys0= Array('i', range(int(16/nworkers)))
  # keys1= Array('i', range(int(16/nworkers)))
  # keys2= Array('i', range(int(16/nworkers)))
  keys3= Array('i', range(int(16/nworkers)))
  w0 = Process(target=worker, args=(M, T, ntraces, 0, 8, keys0))
  w0.start()
  # w1 = Process(target=worker, args=(M, T, ntraces, 4, 8, keys1))
  # w1.start()
  # w2 = Process(target=worker, args=(M, T, ntraces, 8, 12, keys2))
  # w2.start()
  crack_aes(M, T, ntraces=ntraces, start_byte=8, end_byte=16, key_guess=keys3)
  w0.join()
  # w1.join()
  # w2.join()
  print(keys0[:])
  # print(keys1[:])
  # print(keys2[:])
  print(keys3[:])

  # final_list = keys0[:] + keys1[:] + keys2[:] + keys3[:]
  final_list = keys0[:] + keys3[:]
  print("FINAL KEY GUESS", final_list)
  print("FINAL KEY GUESS", [hex(x) for x in final_list])
  end = timeit.default_timer()
  print("\nTime taken to crack 16 bytes: ", end - start)
  actual_key = [128, 206, 252, 108, 120, 51, 218, 176, 138, 49, 165, 105, 4, 112, 119, 103]
  actual_key_mine  = [ 0xCD, 0X97, 0X16, 0XE9, 0X5B, 0X42, 0XDD, 0X48, 0X69, 0X77, 0X2A, 0X34, 0X6A, 0X7F, 0X58, 0X13]
  correct = True
  for i in range(len(final_list)):
    if (actual_key[i] != final_list[i]):
       correct = False
  if(correct):
    print("\nCORRECT KEY")
  else:
    print("\nINCORRECT KEY")


if ( __name__ == '__main__' ) :

  
  parser = argparse.ArgumentParser()


  parser.add_argument( '--uart', dest = 'uart', type = str, action = 'store', default = '/dev/scale-board' )
  parser.add_argument( '--mode', dest = 'mode',             action = 'store', choices = [ 'uart', 'socket' ], default = 'uart')
  parser.add_argument( '--file', dest = 'file',             action = 'store', default = None)
  parser.add_argument( '--host', dest = 'host', type = str, action = 'store' )
  parser.add_argument( '--port', dest = 'port', type = int, action = 'store' )
  parser.add_argument( '--data', dest = 'data', type = str, action = 'store' )

  args = parser.parse_args()
  attack( len( sys.argv ), sys.argv )

  # execute client implementation

