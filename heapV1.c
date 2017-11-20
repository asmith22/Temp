#include <stdlib.h>
#include <stdio.h>
#include "heaplib.h"
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

/* You must implement these functions according to the specification
 * given in heaplib.h. You can define any types you like to this file.
 *
 * Student 1 Name:Samuel Opoku-Agyemang
 * Student 1 NetID:so373
 * Student 2 Name:Willventchy Celestin
 * Student 2 NetID:wc424
 *
 * Include a description of your approach here.
 *
 */

/* This is an example of a helper print function useful for
 * programming and debugging purposes.  Always wrap print statments
 * inside an #ifdef so that the default compile does not include them.
 * Function calls and printfs will kill your performance and create
 * huge output files, for which we have ZERO TOLERANCE.
 */


/*
 *Meta data for each free block
 */
typedef struct _block_header{
    struct  _block_header *next;  //pointer to the next free block
    struct  _block_header *prev;  //pointer to the previous block
    unsigned int free;
    unsigned int size;
} block_header ;



void print_block_header(block_header *block) {
    printf("block starts at addr %p\n"   // cute little C printing trick.
           "size = %d\n"            // Notice there are no commas between these lines
           "free? %s\n", block, block->size, block->free? "Yes" : "No");
}


/*
 *footer for each block
 */
typedef struct _footer{
    unsigned int size;  //size of the block
    unsigned int free;  //is the block free?
} footer ;


/* Useful shorthand: casts a pointer to a (char *) before adding */
#define ADD_BYTES(base_addr, num_bytes) (((char *)(base_addr)) + (num_bytes))
#define SUBTRACT_BYTES(base_addr, num_bytes) (((char *)(base_addr)) - (num_bytes))

#define BLOCK_HEADER_SIZE sizeof(block_header)
#define FOOTER_SIZE sizeof(footer)

block_header * getAligned(block_header *);

/* Sets up a new heap. 'heap_ptr' that points to a chunk of memory
 * that was -- allocated prior to this call -- which we will refer to
 * as the heap of size 'heap_size' bytes.  Returns 0 if setup fails,
 * nonzero if success.
 */
int hl_init(void *heap_ptr, unsigned int heap_size) {
    
    
    //checking if heap_ptr is 8 byte aligned
    if (((unsigned long) heap_ptr & (ALIGNMENT - 1)) != 0){
        return 0;
    }
    
    
    
    //checking that heap_size < size of meta data
    if (heap_size < BLOCK_HEADER_SIZE){
        return 0;
    }
    
    
    block_header *heap_block = (block_header*) heap_ptr;
    
    //heap_block->next = getAligned((block_header *) ADD_BYTES(heap_ptr, BLOCK_HEADER_SIZE));
    heap_block->next = NULL;
    heap_block->prev = NULL;
    heap_block->size = heap_size - BLOCK_HEADER_SIZE; //remaining space for the heap to occupy
    heap_block->free = 1;
    
    return 1;
    
}


/* Allocates a block of memory of the given size from the heap starting
 * at 'heap_ptr'. Returns a pointer to the payload on success; returns
 * 0 if the allocator cannot satisfy the request.
 */
void *hl_alloc(void *heap_ptr, unsigned int payload_size) {
    
    //checking if heap_ptr is 8 byte aligned
    if (((unsigned long) heap_ptr & (ALIGNMENT - 1)) != 0){
        return 0;
    }
    
    
    block_header *block = (block_header *) heap_ptr;
    
    if (payload_size + BLOCK_HEADER_SIZE +FOOTER_SIZE > block->size ) {
        //not enough space to be allocated
        return 0;
    }
    
    
    block_header *preferred_block = block->next; //first free
    block_header *previous = NULL;
    
    if (preferred_block == NULL) {
        //either no block allocated yet
        
        if (block->size >= BLOCK_HEADER_SIZE + payload_size + FOOTER_SIZE ) {
            preferred_block =  (block_header *) ADD_BYTES(block, BLOCK_HEADER_SIZE);
            block->next = preferred_block;
            
            preferred_block->size = payload_size;
            preferred_block->next = NULL;
            preferred_block->prev = block;
            preferred_block->free = 0;
            
            //set the footer to free
            footer *foot = (footer *) ADD_BYTES(getAligned(preferred_block) , preferred_block->size);
            foot->free = 1;
            
            return getAligned(preferred_block);
        }else{
            return 0;
        }
        
    }
    
    while(block != NULL){
        //checkinf if block is not in use, and , the payload size can fit into the block, and the current block size is smaller than the preferred
        
        if (block->free == 1 && (payload_size + BLOCK_HEADER_SIZE +FOOTER_SIZE )< block->size && (block->size < preferred_block->size)){
            //found a new preferred block
            previous = preferred_block;
            preferred_block = block;
            
        }
        
        //move to the next block
        block = block->next;
        
    }
    
    if (block == NULL && preferred_block->free == 0) {
        return 0;
    }
    
    
    //success
    //preferred_block->size = payload_size; //dont change the size
    
    return getAligned(preferred_block);
    
    
    
    
}


/* Releases the block of previously allocated memory pointed to by
 * payload_ptr. NOP if payload_ptr == 0.
 */
void hl_release(void *heap_ptr, void *payload_ptr) {
    
    
    
    if (payload_ptr == NULL) {
        return;
    }
    
    if (heap_ptr == NULL) {
        return;
    }
    
    //checking if heap_ptr is 8 byte aligned
    if (((unsigned long) heap_ptr & (ALIGNMENT - 1)) != 0){
        return;
    }
    
    //checking if heap_ptr is 8 byte aligned
    if (((unsigned long) payload_ptr & (ALIGNMENT - 1)) != 0){
        return;
    }
    
    block_header *pointer = (block_header *) SUBTRACT_BYTES(payload_ptr, BLOCK_HEADER_SIZE);
    pointer->free = 1;
    
    
    //set the footer to free
    footer *foot = (footer *) ADD_BYTES(payload_ptr, pointer->size);;
    foot->free = 1;
    
    
    
    ///loop through linked list of frees and make this the tail
    
    block_header *tail = (block_header *) heap_ptr;
    
    while(tail->next != NULL){
        //checkinf if block is not in use, and , the payload size can fit into the block, and the current block size is smaller than the preferred
        tail = tail->next;
    }
    
    tail->next = pointer;
    pointer->prev = tail;
    pointer->next = NULL;
}


/* Changes the size of the payload pointed to by payload_ptr,
 * returning a pointer to the new payload, or 0 if the request cannot
 * be satisfied. The contents of the payload should be preserved (even
 * if the location of the payload changes).  If payload_ptr == 0,
 * function should behave like hl_alloc().
 */
void *hl_resize(void *heap_ptr, void *payload_ptr, unsigned int new_size) {
    
    if (payload_ptr == NULL || payload_ptr ==0 ) {
        return hl_alloc(heap_ptr, new_size);
    }
    
    
    block_header *pointer = (block_header *) SUBTRACT_BYTES(payload_ptr, BLOCK_HEADER_SIZE);
    
    
    
    if (  new_size < pointer->size) {
        
        //new size is smaller
        if ((pointer->size - new_size) < (BLOCK_HEADER_SIZE + FOOTER_SIZE)) {
            //space not enough to store free meta data
            //pretend to have succeeded
            return payload_ptr;
            
        }else{
            //it can fit a new free block in the free space
            
            //shift footer to the left
            pointer->size = new_size;
            
            footer *foot = (footer *) ADD_BYTES(pointer, pointer->size);
            foot->size = new_size;
            foot->free = 0;
            
            //create new block
            block_header *new_free_block = (block_header *) ADD_BYTES(foot, FOOTER_SIZE);
            new_free_block->next = NULL;
            new_free_block->prev = NULL;
            new_free_block->free = 1;
            
            
            
            //fix size of new block
            new_free_block->size = pointer->size - new_size;
            new_free_block->size = new_free_block->size - (BLOCK_HEADER_SIZE + FOOTER_SIZE);
            
            //set footer of new block to free
            footer *new_foot = (footer *) ADD_BYTES(new_free_block, new_free_block->size);
            new_foot->free = 0;
            new_foot->size = new_free_block->size;
            
            return getAligned( pointer );
        }
    }else if (pointer->size < new_size){
        //current size is too small
        
        block_header *new_block = (block_header *) hl_alloc(heap_ptr, new_size);
        
        
        if (new_block == 0 || new_block == NULL) {
            return NULL;
        }
        
        
        memcpy(new_block, getAligned( pointer ), pointer->size);
        pointer->free = 1;
        
        //set the footer to 1 as well
        footer *foot = (footer *) ADD_BYTES( pointer, pointer->size);
        foot->free = 1;
        
        return getAligned(new_block);
        
    }else{
        
        //resizing to the same size
        return getAligned(pointer);
    }
    
    
    return NULL;
}


/*to convert to 8 bytes aligned
 */
block_header * getAligned(block_header *block){
    
    if (ADD_BYTES(block, BLOCK_HEADER_SIZE) == 0) {
        return (block_header *) ADD_BYTES(block, BLOCK_HEADER_SIZE);
    }
    return(block_header *) (ADD_BYTES(block, BLOCK_HEADER_SIZE) + (ALIGNMENT - ((unsigned long) ADD_BYTES(block, BLOCK_HEADER_SIZE) & (ALIGNMENT-1))) ) ; // Success!
}