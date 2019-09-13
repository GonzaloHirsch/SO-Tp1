GCC = "-gcc"
GCCFLAGS = "-Wall -g"
LDFLAGS = "-lrt -pthread"


application : source/application.c source/queueBuffer.c include/constants.h include/queueBuffer.h
    $(GCC) $(GCCFLAGS) $(LDFLAGS) -I./include source/application.c source/queueBuffer.c -o application
slaveProcess : source/slaveProcess.c source/queueBuffer.c include/constants.h include/queueBuffer.h
    $(GCC) $(GCCFLAGS) $(LDFLAGS) -I./include source/slaveProcess.c source/queueBuffer.c -o slaveProcess
view : source/view.c source/queueBuffer.c include/constants.h include/queueBuffer.h
    $(GCC) $(GCCFLAGS) $(LDFLAGS) -I./include source/view.c source/queueBuffer.c -o view