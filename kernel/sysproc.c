#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "file.h"
#include "fcntl.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
 
// lab10
uint64 sys_mmap(void) {
  uint64 addr;
  int len, prot, flags, offset;
  struct file *f;
  struct vm_area *vma = 0;
  struct proc *p = myproc();
  int i;

  argaddr(0, &addr);
  argint(1, &len);
  argint(2, &prot);
  argint(3, &flags);
  argfd(4, 0, &f);
  rgint(5, &offset);
  if (flags != MAP_SHARED && flags != MAP_PRIVATE) {
    return -1;
  }
  // the file must be written when flag is MAP_SHARED
  if (flags == MAP_SHARED && f->writable == 0 && (prot & PROT_WRITE)) {
    return -1;
  }
  // offset must be a multiple of the page size
  if (len < 0 || offset < 0 || offset % PGSIZE) {
    return -1;
  }

  // allocate a VMA for the mapped memory
  for (i = 0; i < NVMA; ++i) {
    if (!p->vma[i].addr) {
      vma = &p->vma[i];
      break;
    }
  }
  if (!vma) {
    return -1;
  }

  // assume that addr will always be 0, the kernel 
  //choose the page-aligned address at which to create
  //the mapping
  addr = MMAPMINADDR;
  for (i = 0; i < NVMA; ++i) {
    if (p->vma[i].addr) {
      // get the max address of the mapped memory  
      addr = max(addr, p->vma[i].addr + p->vma[i].len);
    }
  }
  addr = PGROUNDUP(addr);
  if (addr + len > TRAPFRAME) {
    return -1;
  }
  vma->addr = addr;   
  vma->len = len;
  vma->prot = prot;
  vma->flags = flags;
  vma->offset = offset;
  vma->f = f;
  filedup(f);     // increase the file's reference count

  return addr;
}

// lab10
uint64 sys_munmap(void) {
  uint64 addr, va;
  int len;
  struct proc *p = myproc();
  struct vm_area *vma = 0;
  uint maxsz, n, n1;
  int i;

  argaddr(0, &addr);
  argint(1, &len);

  if (addr % PGSIZE || len < 0) {
    return -1;
  }

  // find the VMA
  for (i = 0; i < NVMA; ++i) {
    if (p->vma[i].addr && addr >= p->vma[i].addr
        && addr + len <= p->vma[i].addr + p->vma[i].len) {
      vma = &p->vma[i];
      break;
    }
  }
  if (!vma) {
    return -1;
  }

  if (len == 0) {
    return 0;
  }

  if ((vma->flags & MAP_SHARED)) {
    // the max size once can write to the disk
    maxsz = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * 1024;
    for (va = addr; va < addr + len; va += PGSIZE) {
      if (uvmgetdirty(p->pagetable, va) == 0) {
        continue;
      }
      // only write the dirty page back to the mapped file
      n = min(PGSIZE, addr + len - va);
      for (i = 0; i < n; i += n1) {
        n1 = min(maxsz, n - i);
        begin_op();
        ilock(vma->f->ip);
        if (writei(vma->f->ip, 1, va + i, va - vma->addr + vma->offset + i, n1) != n1) {
          iunlock(vma->f->ip);
          end_op();
          return -1;
        }
        iunlock(vma->f->ip);
        end_op();
      }
    }
  }
  uvmunmap(p->pagetable, addr, (len - 1) / PGSIZE + 1, 1);
  // update the vma
  if (addr == vma->addr && len == vma->len) {
    vma->addr = 0;
    vma->len = 0;
    vma->offset = 0;
    vma->flags = 0;
    vma->prot = 0;
    fileclose(vma->f);
    vma->f = 0;
  } else if (addr == vma->addr) {
    vma->addr += len;
    vma->offset += len;
    vma->len -= len;
  } else if (addr + len == vma->addr + vma->len) {
    vma->len -= len;
  } else {
    panic("unexpected munmap");
  }
  return 0;
}
