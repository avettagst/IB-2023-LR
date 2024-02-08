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
using namespace std;

int main(int argc, char* argv[]){

    if(argc < 3){
        std::cout << "Faltan argumentos\n";
        std::cout << "Usage: " << argv[0] << " cantidad tamaño\n";
        return 1;
    }

    uint64_t N = atoi(argv[1]);
    uint64_t size = atoi(argv[2]);
    uint64_t totalbytes = N*size;
    char* buff = new char[size];
    int i, n;

    for(int j = 0; j<int(size); j++){
        buff[j] = j&0xff;
    }

    int pah[2]; //vamos a escribir los bloques desde el padre hasta el hijo
    
    int ret = pipe(pah);
    if(ret == -1){
        perror("error en el pipe:");
    }
    
    pid_t pid = fork();
    

    switch(pid) {
        case 0: //HIJO
            {
            close(pah[1]); //cierro la escritura
            
            clock_t start_time = clock();
            while( true ){
                n = read(pah[0], buff, size);
                if( n == 0 ) {
                    break;
                }
            }
            clock_t end_time = clock();
            double duration = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            double BW = totalbytes/duration;
            
            cout << "Tiempo: " << duration << " s\n";
            cout << "MBytes transmitidos: " << totalbytes << endl;            
            cout << "BW = " << BW/1000000 << "MB/s" << endl;
        
            break;
            }

        case -1:
            perror("error en el fork:");
            break;
            

        default://PADRE
            close(pah[0]); //cierro la lectura
            
            for (i = 0; i < int(N); i++){
                int n = write(pah[1], buff, size);
                assert(n==int(size));
            }         

            break;
            

    }    

    delete []buff;
    return 0;
}