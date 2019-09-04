//
// Created by root on 04.09.19.
//

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include "../Include/utils.h"

static void list_directories_rec(char *, int);

void list_directories(char * name){
    list_directories_rec(name, 0);
}

static void list_directories_rec(char * name, int tabs){

    DIR * directory = opendir(name);
    struct dirent * file;

    while((file = readdir(directory))){

        if(file->d_type == DT_DIR){
            if(strcmp(file->d_name, ".") ==0 || strcmp(file->d_name, "..") ==0){
                continue;
            }
            char path[MAX_PATH];
            //Print the name of this directory
            printf("%*s\n", tabs, file->d_name);
            //Save path of this dir in path variable
            snprintf(path, sizeof(path), "%s/%s", name, file->d_name);
            list_directories_rec(path, tabs+8);
        }
        else{
            printf("%*s\n", tabs, file->d_name);
        }

    }
}