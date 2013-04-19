/* 3.2.0 */

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/syscalls.h>

static void func_vm_open(struct vm_area_struct *area)
{
  printk(KERN_ALERT "vm open\n");
}

static void func_vm_close(struct vm_area_struct *area)
{
  printk(KERN_ALERT "vm close\n");
}

static int func_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
  printk(KERN_ALERT "vm fault at %p\n", vmf->virtual_address);
  return 0;
}

static const struct vm_operations_struct ralloc_vm_ops = {
  .open =  func_vm_open,
  .close = func_vm_close,
  .fault = func_vm_fault,
};

static unsigned long foo(void)
{
  struct vm_area_struct *vma;
  unsigned long addr = 0;
  
  vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
  if (vma == NULL)
    goto out;

  addr = 0x10000;
  INIT_LIST_HEAD(&vma->anon_vma_chain);
  vma->vm_mm = current->mm;
  vma->vm_start = addr;
  vma->vm_end = addr + 0x1000;
  vma->vm_ops = &ralloc_vm_ops;
  vma->vm_flags = VM_READ | VM_WRITE | VM_MIXEDMAP;
  vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
  vma->vm_pgoff = 0x10000 >> PAGE_SHIFT;
  insert_vm_struct(current->mm, vma);

out:
  return addr;
}
