#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>
#include <cstring>
#include <filesystem>
#include <stdio.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>

using namespace std;

#define BUF_SIZE 1024

int main(int argc, char *argv[]){
    cout << "Ejecutable: " << argv[0] << endl;

    pid_t pid;
    int pah[2], hap[2];
    
    int ret = pipe(pah);
    if(ret == -1){
        perror("error en el pipe:");
    }


    ret = pipe(hap);
    if(ret == -1){
        perror("error en el pipe:");
    }

    pid = fork();

    switch(pid) {
        case 0: //HIJO
            close(pah[1]);
            close(hap[0]);

            {
                dup2(pah[0], STDIN_FILENO);
                dup2(hap[1], STDOUT_FILENO);
                close(pah[0]);
                close(hap[1]);

                execl("/usr/bin/perl", "perl", "-pe", "$_ = uc $_", NULL);
                assert(0);
            }
            break;
            
        case -1:
            perror("error en el fork:");
            break;
            
        default://PADRE
            close(pah[0]);
            close(hap[1]);

            {                   
                fd_set rfds;
                char buff[BUF_SIZE];
                int n;
                int stdin_on = 1;

                while( 1 ) {
                /* Watch stdin (fd 0) to see when it has input. */
                    FD_ZERO(&rfds);
                    FD_SET(hap[0], &rfds);

                    if(stdin_on){
                        FD_SET(STDIN_FILENO, &rfds);
                    }


                   


                    int retval = select(hap[0] + 1, &rfds, NULL, NULL, NULL);

                    if( retval == -1 ) {
                        perror("select()");
                        return 1;
                    }

                    if(FD_ISSET(STDIN_FILENO, &rfds) ) {
                        if( ! (n = read(STDIN_FILENO, buff, BUF_SIZE)) ) {
                            //se cerró STDIN
                            stdin_on = 0;
                            close(pah[1]);
                        }
                        else{
                            write(pah[1], buff, n);                           
                        }
                    }

                    if(FD_ISSET(hap[0], &rfds) ) {
                        if( ! (n = read(hap[0], buff, BUF_SIZE)) ) {
                            //se cerró hap[0]
                            break;
                        }
                        else{
                            write(STDOUT_FILENO, buff, n);                           
                        }
                    }

                }
                close(hap[0]);
                int st;
                wait(&st);
            }



            break;
    }




    return 0;
}