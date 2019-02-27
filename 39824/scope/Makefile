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
