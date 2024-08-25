#include "loader.h"

#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

u32 load_elf(X3DS* system, char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        eprintf("no such file\n");
        return 0;
    }

    Elf32_Ehdr ehdr;
    if (fread(&ehdr, sizeof ehdr, 1, fp) < 1) {
        fclose(fp);
        return 0;
    }

    Elf32_Phdr* phdrs = calloc(ehdr.e_phnum, ehdr.e_phentsize);
    fseek(fp, ehdr.e_phoff, SEEK_SET);
    if (fread(phdrs, ehdr.e_phentsize, ehdr.e_phnum, fp) < ehdr.e_phnum) {
        fclose(fp);
        return 0;
    }
    for (int i = 0; i < ehdr.e_phnum; i++) {
        if (phdrs[i].p_type != PT_LOAD) continue;
        void* segment = x3ds_mmap(system, phdrs[i].p_vaddr, phdrs[i].p_memsz);
        fseek(fp, phdrs[i].p_offset, SEEK_SET);
        if (fread(segment, 1, phdrs[i].p_filesz, fp) < phdrs[i].p_filesz) {
            fclose(fp);
            free(phdrs);
            return 0;
        }
        int prot = 0;
        if (phdrs[i].p_flags & PF_R) prot |= PROT_READ;
        if (phdrs[i].p_flags & PF_W) prot |= PROT_WRITE;
        mprotect(segment, phdrs[i].p_memsz, prot);

        linfo("loaded elf segment at %08x", phdrs[i].p_vaddr);
    }
    free(phdrs);

    fclose(fp);
    return ehdr.e_entry;
}