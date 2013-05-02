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
**  See http://shell-storm.org/blog/Linux-process-execution-and-the-useless-ELF-header-fields/
*/

#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

Elf32_Ehdr    *pElf_Header;
Elf32_Phdr    *pElf32_Phdr;
Elf32_Shdr    *pElf32_Shdr;
char          *pMapElf;
struct stat   filestat;

static unsigned char *set_header(char *file)
{
  int fd;
  unsigned char *data;

  fd = open(file, O_RDONLY, 0644);
  stat(file, &filestat);
  printf("[+] Binary size : %d octets\n", (int)filestat.st_size);
  data = malloc(filestat.st_size * sizeof(char));
  read(fd, data, filestat.st_size);
  pMapElf = mmap(0, filestat.st_size, PROT_READ, MAP_SHARED, fd, 0);
  pElf_Header = (Elf32_Ehdr *)data;
  pElf32_Shdr = (Elf32_Shdr *)((char *)data + pElf_Header->e_shoff);
  pElf32_Phdr = (Elf32_Phdr *)((char *)data + pElf_Header->e_phoff);
  close(fd);

  return (data);
}

int main(int argc, char **argv)
{
  unsigned char *data;
  unsigned nb_section;
  Elf32_Shdr *current;
  int i, fd;

  if (argc < 2){
    printf("Syntax: ./%s <bin>\n", argv[0]);
    return -1;
  }

  data = set_header(argv[1]);

  printf("--- Step 1 ---\n");
  printf("[+] Clean sections...\n");
  nb_section = pElf_Header->e_shnum;
  for (i = 0 ; i < nb_section ; i++){
    pElf32_Shdr->sh_name = 0;
    pElf32_Shdr->sh_type = 0;
    pElf32_Shdr->sh_flags = 0;
    pElf32_Shdr->sh_addr = 0;
    pElf32_Shdr->sh_offset = 0;
    pElf32_Shdr->sh_size = 0;
    pElf32_Shdr->sh_link = 0;
    pElf32_Shdr->sh_info = 0;
    pElf32_Shdr->sh_addralign = 0;
    pElf32_Shdr->sh_entsize = 0;
    pElf32_Shdr++;
  }
  printf("[+] Clean section [DONE]\n");

  printf("--- Step 2 ---\n");
  printf("[+] Clean elf header...\n");
  pElf_Header->e_shnum = 0;
  pElf_Header->e_shstrndx = 0;
  pElf_Header->e_shentsize = 0;
  pElf_Header->e_version = 0;
  pElf_Header->e_ehsize = 0;
  pElf_Header->e_shoff = 123;
  printf("[+] Clean elf header [DONE]\n");

  printf("--- Step 3 ---\n");
  printf("[+] Writting binary...\n");
  fd = open(argv[1], O_WRONLY, 0644);
  write(fd, data, filestat.st_size);
  close(fd);
  printf("[+] Writting binary [DONE]\n");

  free(data);
  return 0;
}
