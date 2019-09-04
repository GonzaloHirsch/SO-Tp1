//
// Created by root on 04.09.19.
//

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include "../Include/utils.h"
#include "../Include/application_process.h"

int main(int argc, char * argv[]) {

    if (argc < 2) {
        perror("Wrong number of arguments.\n");
        return -1;
    }

    list_directories("../../../");
}