#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "a1fs.h"
void help(char* s, char*path, char*path_cpy){
//    char *new_dirOrFile = "apple";
    
    char *new_dirOrFile = strrchr(path, '/');
    if (new_dirOrFile != NULL)
    {
        // push pointer to one after
        new_dirOrFile = new_dirOrFile + 1;
        
    }

//    strncpy(s, new_dirOrFile, strlen(new_dirOrFile) + 1);
    strncpy(path_cpy, path, strlen(path) - strlen(new_dirOrFile));
    path_cpy[strlen(path) - strlen(new_dirOrFile)] = '\0';
}


int main(int argc, char *argv[])
{
//    int arr[10] = {1, 2, 3, 4};
//    int arr_2[10];
//
//    int x = 2;
//    x += 1;
//
//    printf("arr %p \n", &arr);
//    printf("arr_2 %p \n", &arr_2);
//    printf(" 1 elemnt is %p \n", &arr[0]);
    /////////
//    char *path = argv[1];
//    char path_dup[128];
//    strncpy(path_dup, path, 128);
//    char *cur_token = strtok(path_dup, "/");
//    int i = 0;
//
//    while (cur_token != NULL){
//        printf("%i, %s \n", i, cur_token);
//        i+=1;
//        cur_token = strtok(NULL, "/");
//    }
    ////////
//    a1fs_inode n;
//    mode_t m;
//    struct timespec mtime;
//    printf("%lu inode \n", sizeof(n));
//    printf("%lu mode_t \n", sizeof(m));
//    printf("%lu mtime \n", sizeof(mtime));
    
    
//    char path_cpy[100];
//    char res[100];
//    char* path = "/file";
//    help(path_cpy, path, res);
//    printf("%s \n", res);
    
    
//    char *cur_token = strtok("/", "/");
//    while (cur_token != NULL){
//        printf("%s hiii \n", cur_token);
//    }
    unsigned int x = 64 * 16;
    unsigned int y = (x + 4096 - 1) & (~4096 + 1);
    printf("y is %i \n", y);
    return 0;
    
}
