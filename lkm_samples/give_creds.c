/* 3.2.0 */

static void give_creds(void)
{
  struct cred *new;

  new = prepare_creds(); /* Prepare a new set of credentials for modification */
  new->euid = 0; /* Give root creds */
  commit_creds(new); /* commit_creds - Install new credentials upon the current task */
}
