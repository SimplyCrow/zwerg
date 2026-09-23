#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <libdwarf/dwarf.h>

void hex_dump(const void* _data, size_t size);

void *memory_map_file(const char *path)
{
        int fd = open(path, O_RDONLY);
        if(fd < 0) {
                perror("Error opening file");
                return NULL;
        }

        struct stat st;
        if(fstat(fd, &st) < 0) {
                perror("Error getting file stats");
                close(fd);
                return NULL;
        }

        void *map_base = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if(map_base == MAP_FAILED) {
                perror("Error mapping file");
                close(fd);
                return NULL;
        }

        close(fd);
        return map_base;
}

bool parse_elf_sections(struct dwarf_sections_t *sections, void *elf)
{
        assert(elf && sections);

        *sections = (struct dwarf_sections_t){0};

        Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf;
        if(ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1
                || ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
                fprintf(stderr, "Invalid ELF file\n");
                return false;
        }

        Elf64_Shdr *shdr = (Elf64_Shdr *)(elf + ehdr->e_shoff);
        char *strtab = (char *)(elf + shdr[ehdr->e_shstrndx].sh_offset);

        printf("%-4s %-25s %-12s %-12s\n", "Idx", "Name", "Offset", "Size");
        printf("----------------------------------------------------------\n");

        for (int i = 0; i < ehdr->e_shnum; i++) {
                char *name = strtab + shdr[i].sh_name;
                if(strcmp(name, ".debug_info") == 0) {
                        if(sections->info.data) {
                                fprintf(stderr, "duplicated section .debug_info\n");
                                return false;
                        }
                        //hex_dump(elf + shdr[i].sh_offset, shdr[i].sh_size);
                        sections->info.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->info.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_abbrev") == 0) {
                        if(sections->abbrev.data) {
                                fprintf(stderr, "duplicated section .debug_abbrev\n");
                                return false;
                        }
                        sections->abbrev.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->abbrev.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_line") == 0) {
                        if(sections->line.data) {
                                fprintf(stderr, "duplicated section .debug_line\n");
                                return false;
                        }
                        sections->line.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->line.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_line_str") == 0) {
                        if(sections->line_str.data) {
                                fprintf(stderr, "duplicated section .debug_line_str\n");
                                return false;
                        }
                        sections->line_str.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->line_str.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_loclists") == 0) {
                        if(sections->loclists.data) {
                                fprintf(stderr, "duplicated section .debug_loclists\n");
                                return false;
                        }
                        sections->loclists.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->loclists.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_rnglists") == 0) {
                        if(sections->rnglists.data) {
                                fprintf(stderr, "duplicated section .debug_rnglists\n");
                                return false;
                        }
                        sections->rnglists.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->rnglists.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_str") == 0) {
                        if(sections->str.data) {
                                fprintf(stderr, "duplicated section .debug_str\n");
                                return false;
                        }
                        sections->str.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->str.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_str_offsets") == 0) {
                        if(sections->str_offsets.data) {
                                fprintf(stderr, "duplicated section .debug_str_offsets\n");
                                return false;
                        }
                        sections->str_offsets.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->str_offsets.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_names") == 0) {
                        if(sections->names.data) {
                                fprintf(stderr, "duplicated section .debug_names\n");
                                return false;
                        }
                        sections->names.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->names.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_macro") == 0) {
                        if(sections->macro.data) {
                                fprintf(stderr, "duplicated section .debug_macro\n");
                                return false;
                        }
                        sections->macro.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->macro.size = shdr[i].sh_size;
                } else if(strcmp(name, ".debug_frame") == 0) {
                        if(sections->frame.data) {
                                fprintf(stderr, "duplicated section .debug_frame\n");
                                return false;
                        }
                        sections->frame.data = (uint8_t*)(elf + shdr[i].sh_offset);
                        sections->frame.size = shdr[i].sh_size;
                } else {
                        continue;
                }

                printf("[%2d] %-25s 0x%08lx   0x%08lx\n",
                                i,
                                *name ? name : "<null>",
                                (unsigned long)shdr[i].sh_offset,
                                (unsigned long)shdr[i].sh_size
                );
        }

        return true;
}

int main(int argc, char **argv)
{
        (void)argc;
        void *elf = memory_map_file(argv[0]);
        if(elf == NULL) {
                fprintf(stderr, "Could not read elf\n");
                return 1;
        }

        struct dwarf_sections_t sections = {0};
        if(!parse_elf_sections(&sections, elf)) {
                fprintf(stderr, "could not parse section table\n");
                return 1;
        }

        struct dwarf_context_t *context = dwarf_init_context(sections);
        if(context == NULL) {
                fprintf(stderr, "could not initialize dwarf context\n");
                return 1;
        }
}
