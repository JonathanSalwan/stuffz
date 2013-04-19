/* 3.2.0 */

#include <linux/crypto.h>
#include <linux/module.h>

char pkey[] 	 = "yourKey";
char plaintext[] = "Your clear text here. ";

char crypted[512];

static void dump(char *str, char *ptr)
{
	int i;

	printk("%s", str);
	for (i = 0 ; i < sizeof(plaintext) ; i++){
		printk("%02x ", (unsigned char)ptr[i]);
	}
	printk("\n");
}

int mod_init(void)
{
	struct crypto_cipher *tfm;

	tfm = crypto_alloc_cipher("aes", 4, CRYPTO_ALG_ASYNC);
	if (!tfm)
		return -1;

	crypto_cipher_setkey(tfm, pkey, sizeof(pkey));
	
	dump("PlainText:\t", plaintext);

	crypto_cipher_encrypt_one(tfm, &crypted[0], &plaintext[0]);
	crypto_cipher_encrypt_one(tfm, &crypted[16], &plaintext[16]);

	dump("Crypted:\t\t", crypted);

	crypto_cipher_decrypt_one(tfm, &crypted[0], &crypted[0]);
	crypto_cipher_decrypt_one(tfm, &crypted[16], &crypted[16]);

	dump("Uncrypted:\t", crypted);

	crypto_free_cipher(tfm);
	
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

