#ifndef P6_COMMON
#define P6_COMMON

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Maybe you need the pthread locks or speedup by multi-threads or background GC in task2
// check if your $(CC) add -lfuse -pthread to $(CFLAGS) automatically, if not add them manually.
#include <pthread.h>

#include "disk.h"
#include "logging.h"

#define NAME_LEN 60

/*   on-disk  data structure   */
struct superblock_t{
    // complete it
    uint32_t magic_number;
    uint32_t size;
	uint32_t num_of_inode;
	uint32_t num_of_datablock;
	uint32_t size_of_inode;
	uint32_t size_of_datablock;
	uint32_t start_add_inode;
	uint32_t start_add_datablock;
	uint32_t start_add_inode_bit;
	uint32_t start_add_datablock_bit;
}sb_t;

struct inode_t{
    // complete it
    uint32_t mode;
	uint32_t count;
	uint32_t size;
	uint32_t access_time;
	uint32_t modify_time;
	uint32_t create_time;
	uint32_t direct_ptr[10];
	uint32_t first_layer_ptr;
	uint32_t second_layer_ptr;
};


struct dentry{
    // complete it
    char name[NAME_LEN];
	uint32_t ino;
};

/*  in-memory data structure   */

struct superblock{
    struct superblock_t *sb;
	pthread_mutex_t mutex;
    // Add what you need, Like locks
}sb;

struct inode{
    struct inode_t *inode;
	pthread_mutex_t mutex;
	struct entry_m *tree_node;
    // Add what you need, Like locks
};

/*Your file handle structure, should be kept in <fuse_file_info>->fh
 (uint64_t see fuse_common.h), and <fuse_file_info> used in all file operations  */
struct file_info{
    // complete it
    uint32_t flag;
	uint32_t ino;
	uint32_t used;
};


struct entry_m{
	char name[NAME_LEN];
	uint32_t ino;
	struct entry_m* sibling;
	struct entry_m* first_child;
};


//Interf.  See "fuse.h" <struct fuse_operations>
//You need to implement all the interfaces except optional ones

//dir operations
int p6fs_mkdir(const char *path, mode_t mode);
int p6fs_rmdir(const char *path);
int p6fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
int p6fs_opendir(const char *path, struct fuse_file_info *fileInfo);//optional
int p6fs_releasedir(const char *path, struct fuse_file_info *fileInfo);//optional
int p6fs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);//optional


//file operations
int p6fs_mknod(const char *path, mode_t mode, dev_t dev);
int p6fs_symlink(const char *path, const char *link);
int p6fs_link(const char *path, const char *newpath);
int p6fs_unlink(const char *path);
int p6fs_readlink(const char *path, char *link, size_t size);//optional

int p6fs_open(const char *path, struct fuse_file_info *fileInfo);
int p6fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int p6fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int p6fs_truncate(const char *path, off_t newSize);
int p6fs_flush(const char *path, struct fuse_file_info *fileInfo);//optional
int p6fs_fsync(const char *path, int datasync, struct fuse_file_info *fi);//optional
int p6fs_release(const char *path, struct fuse_file_info *fileInfo);


int p6fs_getattr(const char *path, struct stat *statbuf);
int p6fs_utime(const char *path, struct utimbuf *ubuf);//optional
int p6fs_chmod(const char *path, mode_t mode); //optional
int p6fs_chown(const char *path, uid_t uid, gid_t gid);//optional

int p6fs_rename(const char *path, const char *newpath);
int p6fs_statfs(const char *path, struct statvfs *statInfo);
void* p6fs_init(struct fuse_conn_info *conn);
void p6fs_destroy(void* private_data);//optional

#endif
