
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "fifo.h"


void printFifo(struct fifo_t *fifo){
	printf("Ringbuffer enthaelt:\nStart: %p\nEnd: %p\nRead:%p\nWrite: %p\nSize: %d\n",fifo->startPtr,fifo->endPtr,fifo->readPtr,fifo->writePtr,fifo->sz);
	printf("Usage: %d/%d\n",fifo_get_used_bytes(fifo),fifo->sz);
	printf("Buffer: \n");
	size_t bytes_used = fifo_get_used_bytes(fifo);
	fifo_data_t *tmp = fifo->readPtr;
	for (int i = 0; i < bytes_used; i++)
	{
		printf("%c",*tmp++);
		if(tmp > fifo->endPtr)
			tmp = fifo->startPtr;
	}
	printf("\n");
	
}

int main(){
	struct fifo_t fifo = fifo_init(10);
	char str[100];
	
	while(1){
		int i;
		printFifo(&fifo);
		
		printf("Lesen oder schrieben oder lesen bis 'x'? (0/1/3)");
		scanf("%d",&i);
		if(i==0){
			printf("Wie viel?");
			scanf("%d",&i);
			fifo_data_t tmp[i+1];
			i = fifo_get_data(&fifo,tmp,i);
			tmp[i] = 0;
			printf("Gelesen (%d bytes): %s\n",i,tmp);
		}else if(i==1){
			printf("Test: ");
			scanf("%99s",str);
			fifo_put_data(&fifo,str,strlen(str));
		}else if(i==3){
			fifo_data_t tmp[30];
			i = fifo_get_until(&fifo,tmp,'x',29);
			tmp[i] = 0;
			printf("Gelesen (%d bytes): %s\n",i,tmp);
		}
		
	}
	
	return 0;
}