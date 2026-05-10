#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <dirent.h>


struct fatfs_disk_descriptor {
    uint8_t fat_table[256];
    uint16_t block_size;
    uint16_t block_count;
    uint16_t block_used;
} disk;


#define FAT_FREE 255 //free fat block
#define FAT_END 254  //end of fat chain
#define FAT_BBLK 253 //bad block
#define FAT_DIR_LEN (disk.block_size/sizeof(struct fatfs_file))

#define FAT_DATA_START disk.block_size

#define CALC_SADRES(FAT_IDX) (disk.block_size + FAT_IDX*disk.block_size)

#define FAT_IFREG     (0b0001 << 12)
#define FAT_IFDIR     (0b0010 << 12)
#define FAT_IFDEV     (0b0011 << 12)
#define FAT_IFMNT     (0b0100 << 12)
#define FAT_IFSOCK    (0b0101 << 12)

#define FAT_ISUID     (0b100 << 9)
#define FAT_ISGID     (0b010 << 9)

#define FAT_IRUSR     (0b100 << 6)
#define FAT_IWUSR     (0b010 << 6)
#define FAT_IXUSR     (0b001 << 6)
#define FAT_IRGRP     (0b100 << 3)
#define FAT_IWGRP     (0b010 << 3)
#define FAT_IXGRP     (0b001 << 3)
#define FAT_IROTH     (0b100 << 0)
#define FAT_IWOTH     (0b010 << 0)
#define FAT_IXOTH     (0b001 << 0)

struct permissions {
    uint8_t user;
    uint8_t group;
    uint16_t mode;
};


struct fatfs_file {
    char name[10 + 1];
    uint8_t fat_index;
    struct permissions perm;
    uint32_t time;
    uint16_t size;
};

uint8_t* fs_buffer;

uint8_t alloc_sector()
{
    for (int i = 0; i < 256; i++) {
        if (disk.fat_table[i] == FAT_BBLK) continue;;
        if (disk.fat_table[i] == FAT_FREE) {
            disk.block_used++;
            return i;
        }
    }
    return FAT_BBLK;
}

void fat_init()
{
    memset(disk.fat_table, FAT_FREE, disk.block_count);
    memset(disk.fat_table + disk.block_count, FAT_BBLK, 256 - disk.block_count);
    disk.fat_table[0] = FAT_END;
    disk.block_used = 2; //block 0(disk descriptor) and block 1(root dir) are used
}

void fat_sync()
{
    memcpy(fs_buffer, &disk, sizeof(disk));
}

//allocate a new file and add it to a directory
struct fatfs_file* create_file(uint8_t dir_index, const char* name, struct permissions perm)
{
    struct fatfs_file* dir = (struct fatfs_file*) &fs_buffer[CALC_SADRES(dir_index)];
    for (int i = 0; i < FAT_DIR_LEN; i++) {
        struct fatfs_file* entry = &dir[i];
        if (entry->fat_index == FAT_FREE) {
            strncpy(entry->name, name, 10);
            uint8_t fat_index = alloc_sector();
            if (fat_index == FAT_BBLK) printf("error!\n");
            entry->fat_index = fat_index;
            entry->perm = perm;
            entry->time = 0;
            disk.fat_table[fat_index] = FAT_END;
            return entry;
        }
    }
    return NULL;
}


//initialize a directory with empty entries
void initialize_dir(uint8_t fat_index)
{
    struct fatfs_file* dir = (struct fatfs_file*) &fs_buffer[CALC_SADRES(fat_index)];
    for (int i = 0; i < FAT_DIR_LEN; i++) {
        struct fatfs_file* entry = &dir[i];
        memset(entry, 0, sizeof(struct fatfs_file));
        entry->fat_index = FAT_FREE;
    }
}

void list_dir(uint8_t fat_index)
{
    struct fatfs_file* dir = (struct fatfs_file*) &fs_buffer[CALC_SADRES(fat_index)];
    for (int i = 0; i < FAT_DIR_LEN; i++) {
        struct fatfs_file* entry = &dir[i];
        if (entry->fat_index != FAT_FREE) {
            printf("%o\t%d\t%d\t%d\t%s\t%d\n", entry->perm.mode, entry->perm.user, entry->perm.group, entry->size, entry->name, entry->fat_index);
            if (entry->perm.mode & FAT_IFDIR) {
                printf("entering dir %s\n", entry->name);
                list_dir(entry->fat_index);
                printf("leaving directory %s\n", entry->name);
            }

        }
    }
}

int write_file(struct fatfs_file* file, uint8_t* buffer, int count)
{
    int bytes_written = 0;
    uint8_t fat_index = file->fat_index;
    int to_read = count & (disk.block_size - 1);

    while (to_read > 0) {
        uint8_t* sector = &fs_buffer[CALC_SADRES(fat_index)];
        memcpy(sector, buffer, to_read);
        to_read = (count - bytes_written) & (disk.block_size - 1);
        bytes_written += to_read;

        uint8_t new_fat_index = alloc_sector();
        if (new_fat_index == FAT_BBLK) return -1;
        disk.fat_table[fat_index] = new_fat_index;
        fat_index = new_fat_index;
    }
    disk.fat_table[fat_index] = FAT_END;
    file->size = count;
    return 0;
}

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

int main(int argc, char** argv)
{
    disk.block_count = 32;
    disk.block_size = 512;
    fs_buffer = calloc(disk.block_count, disk.block_size);
    fat_init();
    initialize_dir(0);
    if (argc != 2) {
        printf("usage: mkfs path/to/staging\n");
        return -1;
    }
    generate_fs(argv[1], 0);
    fat_sync();
    
    printf("fs statistics:\nblock count: %d\nblock size %d\nblocks used %d\n", disk.block_count, disk.block_size, disk.block_used);
    list_dir(0);

    int imgfd = open("rootfs.bin", O_CREAT | O_RDWR, S_IFREG | S_IRUSR | S_IWUSR );

    write(imgfd, fs_buffer, disk.block_count*disk.block_size);
    
    close(imgfd);
    return 0;
}



