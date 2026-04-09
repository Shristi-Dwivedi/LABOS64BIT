#ifndef FS_H
#define FS_H

#define MAX_FILES 16
#define MAX_FILENAME 32
#define MAX_CONTENT 4096

typedef struct{
    int used;
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
} File;

void fs_init(void);
int fs_create(const char *name);
int fs_exists(const char *name);
int fs_write(const char *name , const char *data);
int fs_read(const char *name, char *out);
int fs_delete(const char *name);
int fs_get_count(void);
const File* fs_get_file(int index);

#endif