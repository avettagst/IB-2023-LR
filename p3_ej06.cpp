#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <chrono>
#include "ej6-Queue.h"


int main(int argc, char* argv[]){
    if(argc < 4){
        std::cout << "Faltan argumentos\n";
        std::cout << "Usage: " << argv[0] << "qname cantidad tamaño\n";
        return 1;
    }

    uint64_t N = atoi(argv[2]);
    uint64_t size = atoi(argv[3]);
    uint64_t totalbytes = N*size;
    char* buff = new char[size];
    int i;

    Queue_t* q = QueueCreate(argv[1], 1024, atoi(argv[3]));

    pid_t pid = fork();
    

    switch(pid) {
        case 0: //HIJO
            {           
            clock_t start_time = clock();
            for (i = 0; i < int(N); i++){
                QueueGet(q, buff);
            }

            clock_t end_time = clock();
            double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            double BW = totalbytes/duration;
            
            std::cout << duration << std::endl;
            std::cout << totalbytes << std::endl;            
            printf("\nBW = %4.1lf\n", BW/1000000);
        
            QueueDetach(q);
            break;
            }

        case -1:
            perror("error en el fork:");
            break;
            

        default://PADRE
            
            for (i = 0; i < int(N); i++){
                QueuePut(q, buff, size);
            }         

            QueueDetach(q);
            break;
            

    }    
  
    delete []buff;


    return 0;
}