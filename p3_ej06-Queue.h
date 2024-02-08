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


struct Queue_t {
    char name[100];
    uint32_t N;    //cantidad de lugares
    uint32_t head;
    uint32_t tail;
    size_t blksize;
    sem_t sem_sync; //sincroniza el acceso
    sem_t sem_put;  //permite agregar elementos
    sem_t sem_get;  //permite sacar elementos


    int aca[0]; //funciona para indicarme dónde empiezo a guardar elementos de la cola
};


// Crea una región de memoria compartida e inicializa una cola
Queue_t *QueueCreate(const char *qname, uint32_t qsize, size_t blksize);
// se “attacha” a la cola con nombre dado
Queue_t *QueueAttach(const char *qname);
// se “detacha” a la cola con nombre dado
void QueueDetach(Queue_t *q);
// Destruye el contenedor, liberando recursos
void QueueDestroy(Queue_t *pQ);
// Agrega un Nuevo elemento. Bloquea si no hay espacio
void QueuePut(Queue_t *pQ, char* elem, size_t size);
// Remueve y retorna un elemento, bloquea si no hay elementos
void QueueGet(Queue_t *pQ, char* buff);
// recupera la cantidad de elementos en la cola
int QueueCnt(Queue_t *pQ);
