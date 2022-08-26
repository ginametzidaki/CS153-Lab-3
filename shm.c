#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
  int i = 0; //for loop variable

  //you write this
  //acquire lock on shm_table
  acquire(&(shm_table.lock));

  //loop through all 64 pages in shm_table and...
  for(i=0; i < 64; i++) {
    //...search if there exists a page whose id is same as id given as an argument
    if(id == shm_table.shm_pages[i].id) { //if found
      //map it to the existing page using mappages()
      mappages(myproc()->pgdir, (char*)PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      //increment the ref cnt
      shm_table.shm_pages[i].refcnt++;
      //return the address of the page through the pointer variable *pointer = (char*) size;
      *pointer = (char*)PGROUNDUP(myproc()->sz);
      //update current process size
      myproc()->sz = PGROUNDUP(myproc()->sz + PGSIZE);
    }

    else { //else if one of the 64 pages is available
      //find the index of first available page and assign id to the page
      shm_table.shm_pages[i].id = id;
      //allocate a new physical page and store its address page.frame=kalloc()
      shm_table.shm_pages[i].frame = kalloc();
      //increment the refcnt
      shm_table.shm_pages[i].refcnt++;
      //initialize the grame allocated to 0 (use memset())
      memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
      //map the new page (using mappages())
      mappages(myproc()->pgdir, (char*)PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      //return the address of the page through the pointer variable *pointer = (char*) size;
      *pointer = (char*)PGROUNDUP(myproc()->sz);
      //update current process size
      myproc()->sz = PGROUNDUP(myproc()->sz + PGSIZE);

    }
  }
  //release lock
  release(&(shm_table.lock));

return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
  int i = 0; //variable for for loop
  //acquire a lock on shm_table
  acquire(&(shm_table.lock));
  //loop through all 64 pages in shm_table...
  for(i=0; i<64; i++) {
    //...and search if there exists a page whose id is same as id given as an argument
    if(shm_table.shm_pages[i].id == id) {
      //if found
      if(shm_table.shm_pages[i].refcnt != 0) { //and refcnt is not 0
        //decrement refcnt
        shm_table.shm_pages[i].refcnt--;
      }
      if(shm_table.shm_pages[i].refcnt == 0) { //if refcnt is 0
        //set all values of the page back to 0
        shm_table.shm_pages[i].id = 0;
        shm_table.shm_pages[i].frame = 0;
      }
    }
  }
  //release lock
  release(&(shm_table.lock));



return 0; //added to remove compiler warning -- you should decide what to return
}
