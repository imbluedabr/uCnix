#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <dirent.h>

uint8_t* fs_buffer;

struct ucfs_superblock {
    char magic[8];
    uint16_t block_size; //in bytes
    uint16_t block_count; //amount of data blocks
    uint16_t block_used; //amount of data blocks used
    uint16_t inode_table_size; //in bytes
    uint32_t block_bitmap[8]; //bitmap of used blocks
} disk;


#define FS_IFREG     (0b0001 << 12)
#define FS_IFDIR     (0b0010 << 12)
#define FS_IFDEV     (0b0011 << 12)
#define FS_IFMNT     (0b0100 << 12)
#define FS_IFSOCK    (0b0101 << 12)

#define FS_ISUID     (0b100 << 9)
#define FS_ISGID     (0b010 << 9)

#define FS_IRUSR     (0b100 << 6)
#define FS_IWUSR     (0b010 << 6)
#define FS_IXUSR     (0b001 << 6)
#define FS_IRGRP     (0b100 << 3)
#define FS_IWGRP     (0b010 << 3)
#define FS_IXGRP     (0b001 << 3)
#define FS_IROTH     (0b100 << 0)
#define FS_IWOTH     (0b010 << 0)
#define FS_IXOTH     (0b001 << 0)

struct permissions {
    uint8_t user;
    uint8_t group;
    uint16_t mode;
};


struct ucfs_inode {
    uint32_t size;
    struct permissions perm;
    uint32_t mtime;
    uint8_t nlinks;
    uint8_t indirect_block; //indirect block points directly to a dir block if it is a dir
};

struct ucfs_file {
    char name[11];
    uint8_t ino;
};


void disk_init()
{
    strcpy(disk.magic, "ucfs");
    for (int i = 0; i < 8; i++) {
        disk.block_bitmap[i] = 0xFFFFFFFF;
    }
    disk.block_size = 512;
    disk.block_count = 16;
    disk.block_used = 0;
    disk.inode_table_size = 4096;
    
    int total_blocks = (disk.inode_table_size/disk.block_size) + disk.block_count + 1;
    fs_buffer = malloc(total_blocks*disk.block_size);
}

void disk_sync()
{
    memcpy(fs_buffer + 512, &disk, sizeof(struct ucfs_superblock));
}

uint8_t block_alloc()
{
    for (int i = 0; i < 256; i++) {
        int a = i >> 4;
        int b = i & 0b1111;
        if (!(disk.block_bitmap[a] & (1 << b))) {
            disk.block_bitmap[a] |= (1 << b);
            disk.block_used++;
            return i;
        }
    }
    return 255;
}

uint8_t inode_alloc()
{
    struct ucfs_inode* itable = (struct ucfs_inode*)(fs_buffer + 1024);
    for (int ino = 0; ino < 255; ino++) {
        struct ucfs_inode* i = &itable[ino];

        if (i->nlinks == 0) {
            return ino;
        }
    }
    return 255;
}

struct ucfs_inode* inode_read(uint8_t ino)
{
    return &((struct ucfs_inode*)(fs_buffer + 1024))[ino];
}

struct ucfs_file* ucfs_mklink(struct ucfs_file* dir, uint8_t ino, const char* name)
{
    struct ucfs_file* entries = (struct ucfs_file*)(fs_buffer + inode_read(dir->ino)->indirect_block*512 + 6*512);

    for (int i = 0; i < 42; i++) {
        if (entries[i].ino == 255) {

            entries[i].ino = ino;
            strcpy(entries[i].name, name);
            inode_read(ino)->nlinks++;

            return &entries[i];
        }
    }
    return NULL;
}

struct ucfs_file* ucfs_mkfile(struct ucfs_file* dir, const char* name, struct permissions perm)
{
    uint8_t ino = inode_alloc();
    struct ucfs_inode* i = inode_read(ino);
    i->perm = perm;
    i->size = 0;
    i->nlinks = 0;
    i->mtime = 0;
    i->indirect_block = block_alloc();

    return ucfs_mklink(dir, ino, name);
}

void ucfs_initdir(struct ucfs_file* dir)
{
    struct ucfs_inode* i = inode_read(dir->ino);
    struct ucfs_file* entries = (struct ucfs_file*)(fs_buffer + i->indirect_block*512 + 6*512);

    for (int i = 0; i < 42; i++) {
        entries[i].ino = 255;
    }
}

void ucfs_initfile(struct ucfs_file* file)
{
    struct ucfs_inode* i = inode_read(file->ino);
    uint8_t* indirect_table = fs->buffer + i->indirect_block*512 + 6*512;

    for (int i = 0; i < 256; i++) {
        indirect_table[i] = 255;
    }
}

/*
void generate_fs(const char* path, uint8_t root_dir)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    uint8_t current_dir = root_dir;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char new_path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
            struct fatfs_file* new_dir = create_file(current_dir, entry->d_name, (struct permissions) { .user = 0, .group = 0, .mode = FAT_IFDIR | FAT_IROTH | FAT_IXOTH});
            initialize_dir(new_dir->fat_index);
            printf("entering directory %s\n", entry->d_name);
            generate_fs(new_path, new_dir->fat_index);
            printf("leaving directory %s\n", entry->d_name);
        } else {
            struct fatfs_file* new_file = create_file(current_dir, entry->d_name, (struct permissions) { .user = 0, .group = 0, .mode = FAT_IFREG | FAT_IROTH | FAT_IXOTH});
            printf("file %s\n", entry->d_name);
        }
    }
    closedir(dir);
}
*/

int main(int argc, char** argv)
{
    disk_init();

    

    disk_sync();
    int imgfd = open("rootfs.bin", O_CREAT | O_RDWR, S_IFREG | S_IRUSR | S_IWUSR );

    write(imgfd, fs_buffer, (disk.block_count + 2)*disk.block_size);
    
    close(imgfd);
    return 0;
}



