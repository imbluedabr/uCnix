#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
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
        disk.block_bitmap[i] = 0;
    }
    disk.block_size = 512;
    disk.block_count = 16;
    disk.block_used = 0;
    disk.inode_table_size = 4096;
    
    int total_blocks = (disk.inode_table_size/disk.block_size) + disk.block_count + 1;
    fs_buffer = calloc(total_blocks, disk.block_size);
}

void disk_load(const char* file)
{
    int img = open(file, O_RDONLY);
    if (img == -1) {
        printf("mkfs: no such file or directory\n");
        return;
    }
    
    lseek(img, 512, SEEK_SET);
    read(img, &disk, sizeof(struct ucfs_superblock));
    printf("disk parameters:\n\tinode_table_size: %d\n\tblock_size: %d\n\tblock_count: %d\n\tblock_used: %d\n", disk.inode_table_size, disk.block_size, disk.block_count, disk.block_used);
    int total_blocks = (disk.inode_table_size/disk.block_size) + disk.block_count + 1;
    fs_buffer = calloc(total_blocks, disk.block_size);
    lseek(img, 0, SEEK_SET);
    read(img, fs_buffer, total_blocks*disk.block_size);
}

void disk_sync()
{
    memcpy(fs_buffer + 512, &disk, sizeof(struct ucfs_superblock));
}

uint8_t block_alloc()
{
    for (int i = 0; i < disk.block_count; i++) {
        int a = i >> 4;
        int b = i & 0b1111;
        if (!(disk.block_bitmap[a] & (1 << b))) {
            printf("blk: %d\n", i);
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

#define CALC_BADDR(BLOCK) (fs_buffer + (BLOCK*512 + 6*512))

struct ucfs_file* ucfs_mklink(struct ucfs_file* dir, uint8_t ino, const char* name)
{
    struct ucfs_file* entries = (struct ucfs_file*)(CALC_BADDR(inode_read(dir->ino)->indirect_block));

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
    struct ucfs_file* entries = (struct ucfs_file*)(CALC_BADDR(i->indirect_block));
    i->size = disk.block_size;
    for (int i = 0; i < 42; i++) {
        entries[i].ino = 255;
    }
}

void ucfs_mkroot(struct ucfs_file* root)
{
    uint8_t ino = inode_alloc();
    struct ucfs_inode* i = inode_read(ino);
    i->indirect_block = block_alloc();
    i->nlinks = 1;
    i->perm.mode = FS_IFDIR | FS_IRUSR | FS_IXUSR;
    i->mtime = 0;

    root->ino = ino;

    ucfs_initdir(root);
}

void ucfs_initfile(struct ucfs_file* file)
{
    struct ucfs_inode* i = inode_read(file->ino);
    uint8_t* indirect_table = CALC_BADDR(i->indirect_block);

    for (int i = 0; i < 256; i++) {
        indirect_table[i] = 255;
    }
}

void ucfs_writefile(struct ucfs_file* file, uint8_t* buffer, int count)
{
    printf("writing file: size=%d\n", count);
    struct ucfs_inode* i = inode_read(file->ino);
    uint8_t* indirect_table = CALC_BADDR(i->indirect_block);
    int bytes_read = 0;
    
    while (bytes_read < count) {
        int block_idx = block_alloc();
        indirect_table[bytes_read >> 9] = block_idx;

        int to_read = ((count - bytes_read) > 512) ? 512 : (count - bytes_read);
        memcpy(CALC_BADDR(block_idx), buffer + bytes_read, to_read);
        bytes_read += to_read;
    }

    i->size = bytes_read;
}

void ucfs_listdir(struct ucfs_file* file, int depth)
{
    struct ucfs_inode* dir = inode_read(file->ino);

    struct ucfs_file* entries = (struct ucfs_file*)(fs_buffer + dir->indirect_block*512 + 6*512);

    for (int i = 0; i < 42; i++) {
        if (entries[i].ino != 255) {
            struct ucfs_inode* f = inode_read(entries[i].ino);
            char timebuff[32] = {0};
            struct tm ts;
            time_t time = f->mtime;
            localtime_r(&time, &ts);
            strftime(timebuff, sizeof(timebuff), "%a %b %d %Y", &ts);
            printf("%*s%o %d\t%d:%d\t%d\t%s\t%s %d\n", depth*8, "",f->perm.mode, f->nlinks, f->perm.group, f->perm.user, f->size, timebuff, entries[i].name, entries[i].ino);
            if (f->perm.mode & FS_IFDIR) {
                if (strcmp(entries[i].name, "..") == 0 || strcmp(entries[i].name, ".") == 0) continue;
                printf("%*s\\-> %s\n", depth*8, "", entries[i].name);
                ucfs_listdir(&entries[i], depth + 1);
                printf("\n");
            }
        }
    }
}

void generate_fs(const char* path, struct ucfs_file* root_dir, int parent_ino)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    struct ucfs_file* current_dir = root_dir;

    ucfs_mklink(current_dir, parent_ino, ".."); //create .. link

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "..") == 0) continue;
        if (strcmp(entry->d_name, ".") == 0) continue;

        char new_path[1024];
        snprintf(new_path, sizeof(new_path), "%s/%s", path, entry->d_name);
        
        if (entry->d_type == DT_DIR) {
            struct ucfs_file* new_dir = ucfs_mkfile(current_dir, entry->d_name, (struct permissions) {
                .user = 100,
                .group = 100,
                .mode = FS_IFDIR | FS_IRUSR | FS_IXUSR
                });
            ucfs_initdir(new_dir);
            printf("entering directory %s\n", entry->d_name);
            generate_fs(new_path, new_dir, current_dir->ino);
            printf("leaving directory %s\n", entry->d_name);
        } else {
            struct ucfs_file* new_file = ucfs_mkfile(current_dir, entry->d_name, (struct permissions) {
                    .user = 100,
                    .group = 100,
                    .mode = FS_IFREG | FS_IRUSR | FS_IXUSR
                    });
            int fd = open(new_path, O_RDONLY);
            struct stat st;
            fstat(fd, &st);
            uint8_t* buffer = malloc(st.st_size);
            read(fd, buffer, st.st_size);
            close(fd);
            ucfs_writefile(new_file, buffer, st.st_size);
            free(buffer);
        }
    }

    closedir(dir);
}


struct ucfs_file root;
int main(int argc, char** argv)
{
    int mode = -1;
    const char* target = NULL;
    const char* dest = NULL;

    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];
        if (strcmp(arg, "-c") == 0) { //create file system
            mode = 0;
        } else if (strcmp(arg, "-e") == 0) { //examine file system
            mode = 1;
        } else if (mode == 0 && !target) {
            target = arg;
        } else if (!dest) {
            dest = arg;
        } else {
            printf("mkfs: USAGE: mkfs [-ce] [TARGET] DEST\n");
            return -1;
        }
    }
    if (!dest) {
        printf("mkfs: no destination file specified\n");
        return -1;
    }
    if (mode == 0 && !target) {
        printf("mkfs: no taget directory specified\n");
        return -1;
    }

    if (mode == 0) {
        disk_init();
        ucfs_mkroot(&root);
        
        generate_fs(target, &root, root.ino);
    
        disk_sync();
        int imgfd = open(dest, O_CREAT | O_RDWR, S_IFREG | S_IRUSR | S_IWUSR );
        write(imgfd, fs_buffer, (disk.block_count + 2)*disk.block_size + disk.inode_table_size);
        close(imgfd);
    } else {
        disk_load(dest);
    }

    ucfs_listdir(&root, 0);

    return 0;
}



