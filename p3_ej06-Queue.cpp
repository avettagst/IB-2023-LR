#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ej6-Queue.h"

// Crea una región de memoria compartida e inicializa una cola
Queue_t *QueueCreate(const char *qname, uint32_t qsize, size_t blksize){
    shm_unlink(qname); //porque no sé si hacer destroy en padre o hijo, lo hago cuando vuelvo a crear la shmem

    //Creo el espacio de memoria compartida
    int fd = shm_open(qname, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR); //preguntar si necesito más permisos
    if(fd < 0){
        perror("Problema abriendo la shmem");
        exit(0);
    }

    //Defino el tamaño que debe tener esa memoria compartida
    int bytesize = sizeof(Queue_t) + qsize*blksize;

    //Limpio el espacio necesario para la memoria compartida
    int trunc = ftruncate(fd, bytesize);
    if(trunc < 0){
        perror("Problema truncando");
        close(fd);
        exit(0);
    }

    //Mapeo la memoria compartida (fd) al espacio de memoria de mi proceso
    Queue_t *q = (Queue_t*)mmap(NULL, bytesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(q == MAP_FAILED){
        perror("Falló mmap");
        exit(0);
    }

    strcpy(q->name,qname);
    q->head = 0;
    q->tail = 0;
    q->N = qsize;
    q->blksize = blksize;

    sem_init(&q->sem_get, 1, 0);
    sem_init(&q->sem_put, 1, qsize);
    sem_init(&q->sem_sync, 1, 1);

    return q;
}


// se “attacha” a la cola con nombre dado
Queue_t *QueueAttach(const char *qname){

    int fd = shm_open(qname, O_RDWR, S_IRUSR | S_IWUSR);
    if(fd < 0){
        perror("Problema attachando la shmem");
        exit(0);
    }

    struct stat st_buf;
    int fs = fstat(fd, &st_buf);
    if(fs < 0){
        perror("Falla fstat");
        exit(0);
    }

    Queue_t *q = (Queue_t*)mmap(NULL, st_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(q == MAP_FAILED){
        perror("Falló mmap");
        exit(0);
    }

    return q;
}


// se “detacha” a la cola con nombre dado
void QueueDetach(Queue_t *q){

    uint32_t bytesize = sizeof(Queue_t) + q->N*sizeof(int);
    int mu = munmap(q, bytesize);
    if(mu < 0){
        perror("Falló munmap");
        exit(0);
    }
}

// Destruye el contenedor, liberando recursos
void QueueDestroy(Queue_t *pQ){

    shm_unlink(pQ->name);  //cierro la shmem
    QueueDetach(pQ);       //después la borro de mi address space
    
    //si la borrara antes del address space, ya no tendría forma de cerrarla
}

// Agrega un Nuevo elemento. Bloquea si no hay espacio
void QueuePut(Queue_t *pQ, char* elem, size_t size){
    assert(size <= pQ->blksize); //elementos que entren en los lugares de la cola
    sem_wait(&pQ->sem_put);

    char *comienzo = (char*)&pQ[1]; //pQ[0] tengo la estructura. pQ[1] es el primer lugar después de la estructura, medido en bytes    
    sem_wait(&pQ->sem_sync);
    memcpy(&comienzo[(pQ->blksize)*(pQ->tail)%(pQ->N)], elem, size);
    pQ->tail++;
    sem_post(&pQ->sem_sync);

    sem_post(&pQ->sem_get);
}

// Remueve y retorna un elemento, bloquea si no hay elementos
void QueueGet(Queue_t *pQ, char* buff){
    sem_wait(&pQ->sem_get);

    char *comienzo = (char*)&pQ[1]; 
    
    sem_wait(&pQ->sem_sync);
    memcpy(buff, &comienzo[(pQ->blksize)*(pQ->tail)%(pQ->N)], pQ->blksize);
    pQ->head++;
    sem_post(&pQ->sem_sync);

    sem_post(&pQ->sem_put);
}

// recupera la cantidad de elementos en la cola
int QueueCnt(Queue_t *pQ){
    
    sem_wait(&pQ->sem_sync);
    uint32_t cant = pQ->tail - pQ->head;
    sem_post(&pQ->sem_sync);
    
    assert(cant <= pQ->N);

    return cant;
}