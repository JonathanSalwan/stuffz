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
**  For more information about this module, 
**  see : http://shell-storm.org/blog/Simple-Hook-detection-Linux-module/
**
*/

#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

#define PATH_PYTHON "/usr/bin/python2.7"
#define PATH_SCRIPT "/opt/scripts/hook_detected.py"

#define TIME_SLEEP 30000 /* in msec */

static struct timer_list timer_s;
static struct workqueue_struct *wq;
static unsigned int syscall_table_size;
static unsigned long *addr_syscall_table;
static unsigned long *dump_syscall_table;

static int exec_python_script(unsigned int sys_num)
{
  char s_num[32];
  char *argv[] = {PATH_PYTHON, PATH_SCRIPT, s_num, NULL};
  static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};
  struct subprocess_info *sub_info;

  sprintf(s_num, "%d", sys_num);
  sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC);
  if (sub_info == NULL)
    return -ENOMEM;
  call_usermodehelper_exec(sub_info, UMH_WAIT_PROC);
  return 0;
}

static unsigned long *get_syscalls_table(void)
{
  unsigned long *start;

  /* hack :/ */
  for (start = (unsigned long *)0xc0000000; start < (unsigned long *)0xffffffff; start++)
    if (start[__NR_close] == (unsigned long)sys_close){
      return start;
    }
  return NULL;
}

static unsigned int get_size_syscalls_table(void)
{
  unsigned int size = 0;

  while (addr_syscall_table[size++]);
  return size * sizeof(unsigned long *);
}

static void check_diff_handler(struct work_struct *w)
{
  unsigned int sys_num = 0;

  while (addr_syscall_table[sys_num]){
    if (addr_syscall_table[sys_num] != dump_syscall_table[sys_num]){
      printk(KERN_INFO "hook_detection: Hook detected ! (syscall %d)\n", sys_num);
      write_cr0(read_cr0() & (~0x10000));
      addr_syscall_table[sys_num] = dump_syscall_table[sys_num];
      write_cr0(read_cr0() | 0x10000);
      exec_python_script(sys_num);
      printk(KERN_INFO "hook_detection: syscall %d is restored\n", sys_num);
    }
    sys_num++;
  }
}
static DECLARE_DELAYED_WORK(check_diff, check_diff_handler);

static void timer_handler(unsigned long data)
{
  unsigned long onesec;

  onesec = msecs_to_jiffies(1000);
  queue_delayed_work(wq, &check_diff, onesec);
  if (mod_timer(&timer_s, jiffies + msecs_to_jiffies(TIME_SLEEP)))
    printk(KERN_INFO "hook_detection: Failed to set timer\n");
}

static int __init hook_detection_init(void)
{
  addr_syscall_table = get_syscalls_table();
  if (!addr_syscall_table){
    printk(KERN_INFO "hook_detection: Failed - Address of syscalls table not found\n");
    return -ECANCELED;
  }

  syscall_table_size = get_size_syscalls_table();
  dump_syscall_table = kmalloc(syscall_table_size, GFP_KERNEL);
  if (!dump_syscall_table){
    printk(KERN_INFO "hook_detection: Failed - Not enough memory\n");
    return -ENOMEM;
  }
  memcpy(dump_syscall_table, addr_syscall_table, syscall_table_size);

  wq = create_singlethread_workqueue("hook_detection_wq");

  setup_timer(&timer_s, timer_handler, 0);
  if (mod_timer(&timer_s, jiffies + msecs_to_jiffies(TIME_SLEEP))){
    printk(KERN_INFO "hook_detection: Failed to set timer\n");
    return -ECANCELED;
  }

  printk(KERN_INFO "hook_detection: Init OK\n");
  return 0;
}

static void __exit hook_detection_exit(void)
{
  if (wq)
    destroy_workqueue(wq);
  kfree(dump_syscall_table);
  del_timer(&timer_s);
  printk(KERN_INFO "hook_detection: Exit\n");
}

module_init(hook_detection_init);
module_exit(hook_detection_exit);

MODULE_AUTHOR("Jonathan Salwan");
MODULE_DESCRIPTION("Hook Detection");
MODULE_LICENSE("GPL");
