// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int pagescount;
} kmems[NCPU];

void
kinit()
{
  for(int i=0; i < NCPU; i++){
    char lockname[6] = "kmem";
    lockname[5] = (char) i;
    lockname[6] = 0;
    initlock(&kmems[i].lock, lockname);
    kmems->pagescount = 0;
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;


  push_off();
  int cid = cpuid();
  acquire(&kmems[cid].lock);
  r->next = kmems[cid].freelist;
  kmems[cid].freelist = r;
  kmems[cid].pagescount++;
  release(&kmems[cid].lock);
  pop_off();
}


void
steal_pages(int cid){
  struct run *r;
  int max = 0, maxindex=-1;
  for(int i=0; i < NCPU; i++){
    if(kmems[i].pagescount > max){
      max = kmems[i].pagescount;
      maxindex = i;
    }
  }
  if(maxindex == -1 || maxindex == cid)
    return;
  acquire(&kmems[maxindex].lock);
  for(int i = 0; i < kmems[maxindex].pagescount / 2; i++){
    r = kmems[maxindex].freelist;
    kmems[maxindex].freelist = kmems[maxindex].freelist->next;
    kmems[maxindex].pagescount--;
    r->next = kmems[cid].freelist;
    kmems[cid].freelist = r;
    kmems[cid].pagescount++;
  }
  release(&kmems[maxindex].lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cid = cpuid();
  acquire(&kmems[cid].lock);
  if(kmems[cid].pagescount == 0)
    steal_pages(cid);
  r = kmems[cid].freelist;
  if(r){
    kmems[cid].freelist = r->next;
    kmems[cid].pagescount--;
  }
  release(&kmems[cid].lock);
  pop_off();
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
