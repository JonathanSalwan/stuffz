/* 3.2.0 */
#include <linux/proc_fs.h>

static ssize_t handler_proc_write(struct file *file, const char __user *buffer, unsigned long count, void *data)
{

}

static ssize_t handler_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{

}

static int create_proc_node(void)
{
  struct proc_dir_entry *proc_entry;

  proc_entry = create_proc_entry("proc_name", 0666, NULL);
  if (proc_entry){
    proc_entry->write_proc  = handler_proc_write;
    proc_entry->read_proc   = handler_proc_read;
  }
  return 0;
}

static void remove_proc_node(void)
{
  remove_proc_entry("proc_name", NULL);
}
