GCC=gcc
GCCFLAGS=-Wall -g
LDFLAGS=-lrt -pthread

all : application slaveProcess view

application : source/application.c source/queueBuffer.c include/constants.h include/queueBuffer.h
	$(GCC) $(GCCFLAGS) -I./include source/application.c source/queueBuffer.c -o application $(LDFLAGS)
slaveProcess : source/slaveProcess.c source/queueBuffer.c include/constants.h include/queueBuffer.h
	$(GCC) $(GCCFLAGS) -I./include source/slaveProcess.c source/queueBuffer.c -o slaveProcess $(LDFLAGS)
view : source/view.c source/queueBuffer.c include/constants.h include/queueBuffer.h
	$(GCC) $(GCCFLAGS) -I./include source/view.c source/queueBuffer.c -o view $(LDFLAGS)

clean:
	rm -rf application slaveProcess view


.PHONY: clean all