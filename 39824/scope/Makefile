# Copyright (C) 2018 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

DEVICE := $(shell if [ -e "/dev/scale-scope" -a -r "/dev/scale-scope" -a -w "/dev/scale-scope" ] ; then echo "/dev/scale-scope" ; else echo "" ; fi)

ifeq "${DEVICE}" ""
  $(warning /dev/scale-scope does not exist, or cannot be read from or written to)
endif

TRACES = traces.dat

attack-39824    :
	@./39824.elf
attack-39824-ld :
	@./39824.elf --traces-ld ${TRACES}
attack-39824-st :
	@./39824.elf --traces-st ${TRACES}

CC        = g++
CC_FLAGS  = -Wall -W -O3 -fopenmp
CC_PATHS  = 
CC_LIBS   =

# uncomment to use the default (probably older) OpenSSL installation
#CC_PATHS +=
#CC_LIBS  +=              -lcrypto
# uncomment to use the bespoke (probably newer) OpenSSL installation
 CC_PATHS += -I ~page/local/linux.x86_64/openssl-1.0.1m/include/
 CC_PATHS += -L ~page/local/linux.x86_64/openssl-1.0.1m/lib/
 CC_LIBS  += -Wl,-Bstatic -lcrypto -Wl,-Bdynamic

all: 
	g++ ${CC_FLAGS} ${CC_PATHS} attack.cpp -o attack ${CC_LIBS}
program-attack-file :
	@python parallel_attack.py --mode="uart" --data="04:DEADBEEF" --uart="/dev/scale-board" --file=${file}
program-attack-board :
	@python parallel_attack.py --mode="uart" --data="04:DEADBEEF" --uart="/dev/scale-board"

	