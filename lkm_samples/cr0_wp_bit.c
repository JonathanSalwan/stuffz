/* 3.2.0 */

write_cr0(read_cr0() & (~0x10000)); /* disable bit WP */
write_cr0(read_cr0() | 0x10000); /* enable bit WP */
