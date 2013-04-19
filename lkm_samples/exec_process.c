/* 3.2.0 */

static int exec_process(void)
{
  struct subprocess_info *sub_info;

  char *argv[] = {"/usr/bin/id", NULL};
  static char *envp[] = {"HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL};

  sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC);
  if (sub_info == NULL)
    return -ENOMEM;
  return call_usermodehelper_exec(sub_info, UMH_WAIT_PROC);
}
