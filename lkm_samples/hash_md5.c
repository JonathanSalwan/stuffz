/* 3.2.0 */

#include <linux/module.h>
#include <crypto/internal/hash.h>

static void dump(char *hash_md5)
{
	int i;

	for (i = 0; i < 16 ; i++){
		printk("%02x", (unsigned char)hash_md5[i]);
	}
	printk("\n");
}

int mod_init(void)
{
	char hash_md5[16];
	struct crypto_shash *md5;
	struct shash_desc *shash;

	md5 = crypto_alloc_shash("md5", 0, 0);
	if (IS_ERR(md5))
		return -1;

	shash = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(md5), 
			GFP_KERNEL);
	if (!shash)
		return -ENOMEM;

	shash->tfm = md5;
	shash->flags = 0;

	if (crypto_shash_init(shash))
		return -1;

	if (crypto_shash_update(shash, "test", 4))
		return -1;

	if (crypto_shash_final(shash, hash_md5))
		return -1;

	kfree(shash);
	crypto_free_shash(md5);

	dump(hash_md5);

	return 0;
}

void mod_exit(void)
{
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_AUTHOR("n/a");
MODULE_DESCRIPTION("n/a");
MODULE_LICENSE("GPL");
