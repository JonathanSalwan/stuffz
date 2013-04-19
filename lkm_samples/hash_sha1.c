/* 3.2.0 */

#include <linux/module.h>
#include <crypto/internal/hash.h>

static void dump(char *hash_sha1)
{
	int i;

	for (i = 0; i < 20 ; i++){
		printk("%02x", (unsigned char)hash_sha1[i]);
	}
	printk("\n");
}

int mod_init(void)
{
	char hash_sha1[20];
	struct crypto_shash *sha1;
	struct shash_desc *shash;

	sha1 = crypto_alloc_shash("sha1", 0, 0);
	if (IS_ERR(sha1))
		return -1;

	shash = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(sha1), 
			GFP_KERNEL);
	if (!shash)
		return -ENOMEM;

	shash->tfm = sha1;
	shash->flags = 0;

	if (crypto_shash_init(shash))
		return -1;

	if (crypto_shash_update(shash, "test", 4))
		return -1;

	if (crypto_shash_final(shash, hash_sha1))
		return -1;

	kfree(shash);
	crypto_free_shash(sha1);

	dump(hash_sha1);

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

