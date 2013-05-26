/*
**  Copyright (C) 2013 - Jonathan Salwan - http://twitter.com/JonathanSalwan
** 
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
** 
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
** 
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
**
**  Little script to trace a specific kernel function and display
**  arguments / call stack. Send your function name in the procfs 
**  node to start the tracing. Send 'none' in procfs node to stop 
**  the tracing. Below, a little example.
**
**  # insmod ./trace.ko
**  # cat /proc/trace_func 
**  Function traced : none
**  # printf '__kmalloc' > /proc/trace_func
**  # cat /proc/trace_func 
**  Function traced : __kmalloc
**  # dmesg
**  ...
**  [ 1880.977375] ]=------------------------
**  [ 1880.977378] Function  : __kmalloc
**  [ 1880.977378] 
**  [ 1880.977380] args 0: 000000e0  args 1: 000000d0  args 2: cb0463c0
**  [ 1880.977382] args 3: 00000003  args 4: 00001812  args 5: 00000000
**  [ 1880.977382] 
**  [ 1880.977386] Pid: 6974, comm: dmesg Tainted: G           O 3.5.7-gentoo #3
**  [ 1880.977387] Call Trace:
**  [ 1880.977391]  [<d08b9074>] trace+0x74/0xa0 [trace]
**  [ 1880.977396]  [<c113cbca>] load_elf_binary+0x83a/0x11c0
**  [ 1880.977400]  [<c123627f>] ? _copy_from_user+0x3f/0x60
**  [ 1880.977404]  [<c113c390>] ? elf_map+0xc0/0xc0
**  [ 1880.977408]  [<c10ffa07>] search_binary_handler+0xc7/0x2c0
**  [ 1880.977413]  [<c1101410>] do_execve+0x2f0/0x3a0
**  [ 1880.977418]  [<c1009d62>] sys_execve+0x32/0x70
**  [ 1880.977423]  [<c166e082>] ptregs_execve+0x12/0x18
**  [ 1880.977427]  [<c166dfcc>] ? sysenter_do_call+0x12/0x22
**  [ 1880.977429] ]= EOF -------------------
**  # printf 'none' > /proc/trace_func
**
**
**  See: http://shell-storm.org/blog/Trace-and-debug-the-Linux-Kernel-functons/
**
**  Tested on 3.5.7 custom kernel with gentoo.
**
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>

static void trace(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5);

static struct jprobe jp = {
  .entry = JPROBE_ENTRY(trace),
  .kp = {
    .symbol_name = NULL,
  }
};

static void trace(void *arg0, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5)
{
  printk("]=------------------------\n");
  printk("Function  : %s\n\n", jp.kp.symbol_name);
  printk("Arg0 : %08x    Arg1 : %08x    Arg2 : %08x\n",   (unsigned int)arg0, (unsigned int)arg1, (unsigned int)arg2);
  printk("Arg3 : %08x    Arg4 : %08x    Arg5 : %08x\n\n", (unsigned int)arg3, (unsigned int)arg4, (unsigned int)arg5);
  dump_stack();
  printk("]= EOF -------------------\n");
  jprobe_return();
}

static ssize_t handler_proc_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
  int ret;

  if (jp.kp.symbol_name)
    kfree(jp.kp.symbol_name);

  jp.kp.symbol_name = kzalloc(count +1, GFP_KERNEL);
  if (!jp.kp.symbol_name)
    return -ENOMEM;

  memcpy((void*)jp.kp.symbol_name, (void *)buffer, count);
  if (!memcmp(jp.kp.symbol_name, "none", 4)){
    kfree(jp.kp.symbol_name);
    jp.kp.symbol_name = NULL;
    unregister_jprobe(&jp);
    printk("trace: Tracing stoped\n");
    return count;
  }
  else if ((ret = register_jprobe(&jp)) < 0) {
    printk("trace: register_jprobe failed, returned %d\n", ret);
    kfree(jp.kp.symbol_name);
    jp.kp.symbol_name = NULL;
    return -ENOSYS;
  }
  printk("trace: %s traced\n", jp.kp.symbol_name);
  return count;
}

static ssize_t handler_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
  int ret;

  if (!jp.kp.symbol_name){
    sprintf(page, "Function traced : none\n");
    ret = 23;
  }
  else {
    sprintf(page, "Function traceed : %s\n", jp.kp.symbol_name);
    ret = strlen(jp.kp.symbol_name) + 20;
  }
  return ret;
}

static int __init mod_init(void)
{
  struct proc_dir_entry *proc_entry;

  proc_entry = create_proc_entry("trace", 0666, NULL);
  if (proc_entry){
    proc_entry->write_proc  = handler_proc_write;
    proc_entry->read_proc   = handler_proc_read;
  }

  return 0;
}

static void __exit mod_exit(void)
{
  unregister_jprobe(&jp);
  remove_proc_entry("trace", NULL);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");


