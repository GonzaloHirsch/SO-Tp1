#!/bin/bash
gcc -g -Wall ./source/application.c ./source/queueBuffer.c -Iinclude -o application -lrt -pthread
gcc -g -Wall ./source/slaveProcess.c -Iinclude -o slaveProcess -lrt -pthread
gcc -g -Wall ./source/view.c ./source/queueBuffer.c -Iinclude -o view -lrt -pthread
