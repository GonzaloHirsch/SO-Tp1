#!/bin/bash
gcc -g ./source/application.c ./source/queueBuffer.c -Iinclude -o application -lrt -pthread
gcc -g ./source/slaveProcess.c -Iinclude -o slaveProcess -lrt -pthread
gcc -g ./source/view.c ./source/queueBuffer.c -Iinclude -o view -lrt -pthread
