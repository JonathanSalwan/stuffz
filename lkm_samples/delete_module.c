/* 3.2.0 */

static void delete_module(char *module_name)
{
  struct module *task;

  task = find_module(module_name);
  if (task){
    list_del(&task->list);
    return;
  }
}
