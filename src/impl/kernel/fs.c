#include "fs.h"

static File files[MAX_FILES];

static int streq_local(const char *a, const char *b){
    while(*a && *b){
        if(*a != *b) return 0;
        a++;
        b++;
    }
    return *a == 0 && *b == 0;
}

static void strcopy_local(char *dst, const char *src, int max_len){
    int i=0;
    while (src[i] && i < max_len - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static int fs_find(const char *name){
    for(int i=0; i<MAX_FILES; i++){
        if(files[i].used && streq_local(files[i].name, name)){
            return i;
        }
        return -1;
    }
}

void fs_init(void){
    for(int i=0; i<MAX_FILES; i++){
        files[i].used = 0;
        files[i].name[0] = '\0';
        files[i].content[0] = '\0';
    }
}

int fs_create(const char *name){
    if(fs_find(name) != -1){
        return -1;
    }
    for(int i=0; i<MAX_FILES;i++){
        if(!files[i].used){
            files[i].used = 1;
            strcopy_local(files[i].name, name, MAX_FILENAME);
            files[i].content[0] = '\0';
            return 0;
        }
    }
    return -1;
}

int fs_exists(const char *name){
    return fs_find(name) != -1;
}

int fs_write(const char *name, const char *data)
{
    int idx = fs_find(name);

    if (idx == -1) {
        if (fs_create(name) != 0)
            return -1;
        idx = fs_find(name);
        if (idx == -1)
            return -1;
    }

    strcopy_local(files[idx].content, data, MAX_CONTENT);
    return 0;
}

int fs_read(const char *name, char *out)
{
    int idx = fs_find(name);
    if (idx == -1)
        return -1;

    strcopy_local(out, files[idx].content, MAX_CONTENT);
    return 0;
}

int fs_delete(const char *name)
{
    int idx = fs_find(name);
    if (idx == -1)
        return -1;

    files[idx].used = 0;
    files[idx].name[0] = '\0';
    files[idx].content[0] = '\0';
    return 0;
}

int fs_get_count(void){
    int count = 0;
    for(int i=0; i<MAX_FILES; i++){
        if(files[i].used) count++;
    }
    return count;
}

const File* fs_get_file(int index){
    int count = 0;
    for(int i=0; i<MAX_FILES; i++){
        if(files[i].used){
            if(count == index){
                return &files[i];
            }
            count++;
        }
    }
    return 0;
}