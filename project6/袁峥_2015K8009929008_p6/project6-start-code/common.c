#include "common.h"

/*define global variables here*/

/*
 Use linear table or other data structures as you need.
 
 example:
 struct file_info[MAX_OPEN_FILE] fd_table;
 struct inode[MAX_INODE] inode_table;
 unsigned long long block_bit_map[];
 Your dentry index structure, and so on.
 
 
 //keep your root dentry and/or root data block
 //do path parse from your filesystem ROOT<@mount point>
 
 struct dentry* root;
 */
#define MAX_OPEN_FILE 256
#define MAX_INODE     512*1024
#define MAGIC_NUM     0x5201314
#define FALSE         0
#define TRUE          1
#define ino_bit       2
#define data_bit      18
#define ino_block     50
#define data_block    16434
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
struct file_info fd_table[MAX_OPEN_FILE];
struct inode inode_table[MAX_INODE];
unsigned char block_bit_map[32*SECTOR_SIZE];
unsigned char inode_bit_map[16*SECTOR_SIZE];

uint32_t free_block;
uint32_t free_inode;

struct entry_m *temp_alloc;
struct entry_m *root_dir;


int create_inode(){
	int flag = 0;
	int new_ino = 0;
	char temp_byte;
	int i,j;
	for (i = 0; i < 16*SECTOR_SIZE; i++){
		temp_byte = inode_bit_map[i];
		for (j = 0; j <= 7; j++){
			if ((temp_byte & 0x1) == 0){
				flag = 1;
				inode_bit_map[i] |= 1<<j;
				device_write_sector(inode_bit_map+i/SECTOR_SIZE*SECTOR_SIZE, ino_bit+i/SECTOR_SIZE);
				break;
			}
			new_ino++;
			temp_byte >>= 1;
		}
		if (flag) break;
	}
	if (flag == 0)
		return -1;
	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,ino_block+new_ino/32);
	inode_table[new_ino].inode->create_time = time(NULL);
	memcpy(buf+(new_ino%32)*128,inode_table[new_ino].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+new_ino/32);
	free_inode--;
	return new_ino;
}

int create_datablock(){
	int flag = 0;
	int new_datablock = 0;
	char temp_byte;
	int i,j;
	for (i = 0; i < 32*SECTOR_SIZE; i++){
		temp_byte = block_bit_map[i];
		for (j = 0; j <= 7; j++){
			if ((temp_byte & 0x1) == 0){
				flag = 1;
				block_bit_map[i] |= 1<<j;
				device_write_sector(block_bit_map+i/SECTOR_SIZE*SECTOR_SIZE, data_bit+i/SECTOR_SIZE*SECTOR_SIZE);
				break;
			}
			new_datablock++;
			temp_byte >>= 1;
		}
		if (flag) break;
	}
	if (flag == 0)
		return -1;
	unsigned char buf[SECTOR_SIZE];
	memset(buf,0,SECTOR_SIZE);
	device_write_sector(buf, data_block+new_datablock);
	free_block--;
	return new_datablock;
}

int delete_inode(int number){
	inode_bit_map[number/8] &= ~(1 << (number%8));
	memset(inode_table[number].inode,0,sizeof(struct inode_t));
	memset(inode_table[number].tree_node,0,sizeof(struct entry_m));
	device_write_sector(inode_bit_map+number/(8*SECTOR_SIZE)*SECTOR_SIZE, ino_bit+number/(8*SECTOR_SIZE));
	free_inode++;
	return 0;
}

int delete_datablock(int number){
	block_bit_map[number/8] &= ~(1 << (number%8));
	device_write_sector(block_bit_map+number/(8*SECTOR_SIZE)*SECTOR_SIZE, data_bit+number/(8*SECTOR_SIZE));
	free_block++;
	return 0;
}

int add_dentry(int ino,struct dentry *dentry_t){
	unsigned char buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	struct dentry* den_ptr;

	inode_table[ino].inode->modify_time = time(NULL);
	int now_size = inode_table[ino].inode->size; 
	inode_table[ino].inode->size += sizeof(struct dentry);
	if (now_size < 10*SECTOR_SIZE){
		if (now_size % SECTOR_SIZE != 0){
			device_read_sector(buf,inode_table[ino].inode->direct_ptr[now_size/SECTOR_SIZE]);
			den_ptr = (struct dentry*)buf;
			den_ptr += (now_size % SECTOR_SIZE)/sizeof(struct dentry);
			strcpy(den_ptr->name,dentry_t->name);
			den_ptr->ino = dentry_t->ino;
			device_write_sector(buf,inode_table[ino].inode->direct_ptr[now_size/SECTOR_SIZE]);
		
		}
		else{
			int new_block = create_datablock();
			memset(buf,0,SECTOR_SIZE);
			inode_table[ino].inode->direct_ptr[now_size/SECTOR_SIZE] = data_block+new_block;
			den_ptr = (struct dentry*)buf;
			strcpy(den_ptr->name,dentry_t->name);
			den_ptr->ino = dentry_t->ino;
			device_write_sector(buf,data_block+new_block);
		}
	}
	else{
		now_size -= 10*SECTOR_SIZE;
		if (now_size == 0)
			inode_table[ino].inode->first_layer_ptr = create_datablock()+data_block;
		device_read_sector(first_layer_buf,inode_table[ino].inode->first_layer_ptr);
		uint32_t *first_layer_ptr = (uint32_t *)first_layer_buf;
		if (now_size % SECTOR_SIZE != 0){
			device_read_sector(buf,*(first_layer_ptr+now_size/SECTOR_SIZE));
			den_ptr = (struct dentry*)buf;
			den_ptr += (now_size % SECTOR_SIZE)/sizeof(struct dentry);
			strcpy(den_ptr->name,dentry_t->name);
			den_ptr->ino = dentry_t->ino;
			device_write_sector(buf,*(first_layer_ptr+now_size/SECTOR_SIZE));
		}
		else{
			int new_block = create_datablock();
			memset(buf,0,SECTOR_SIZE);
			*(first_layer_ptr + now_size/SECTOR_SIZE) = data_block+new_block;
			device_write_sector(first_layer_buf,inode_table[ino].inode->first_layer_ptr);
			den_ptr = (struct dentry*)buf;
			strcpy(den_ptr->name,dentry_t->name);
			den_ptr->ino = dentry_t->ino;
			device_write_sector(buf,data_block+new_block);
		}
	}
	
	device_read_sector(buf,ino_block+ino/32);
	memcpy(buf+(ino%32)*128,inode_table[ino].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ino/32);
	
	return 0;
}


int del_dentry(int father_ino, struct dentry *dentry_t){

	//delete the node in dir tree
	struct entry_m *pre_node = inode_table[father_ino].tree_node, *node = inode_table[father_ino].tree_node->first_child;
	if (strcmp(node->name,dentry_t->name) == 0 && node->ino == dentry_t->ino){
		pre_node->first_child = node->sibling;
		free(node);
	}
	else{ 
		while (strcmp(node->name,dentry_t->name) != 0 || node->ino != dentry_t->ino){
			pre_node = node;
			node = node->sibling;
		}
		pre_node->sibling = node->sibling;
		free(node);
	}

	//delete the dentry in father dir(replace the deleting one with last one)
	unsigned char buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	struct dentry *den_ptr,temp_entry;
	int flag = 0;
	inode_table[father_ino].inode->modify_time = time(NULL);
	int now_size = inode_table[father_ino].inode->size; 
	inode_table[father_ino].inode->size -= sizeof(struct dentry);

	//copy the last one and delete it
	if (now_size <= 10*SECTOR_SIZE){
		device_read_sector(buf,inode_table[father_ino].inode->direct_ptr[(now_size-1)/SECTOR_SIZE]);
		den_ptr = (struct dentry*)buf;
		den_ptr += ((now_size-1) % SECTOR_SIZE)/sizeof(struct dentry);
		temp_entry.ino = den_ptr->ino;
		strcpy(temp_entry.name,den_ptr->name);
		memset(den_ptr,0,sizeof(struct dentry));
		if (temp_entry.ino == dentry_t->ino && strcmp(temp_entry.name,dentry_t->name) == 0)
			flag = 1;
		if (now_size % SECTOR_SIZE == sizeof(struct dentry))
			delete_datablock(inode_table[father_ino].inode->direct_ptr[(now_size-1)/SECTOR_SIZE]-data_block);
	}
	else {
		device_read_sector(first_layer_buf,inode_table[father_ino].inode->first_layer_ptr);
		uint32_t *first_layer_ptr = (uint32_t *)first_layer_buf;
		first_layer_ptr += (now_size-1)/SECTOR_SIZE-10;
		device_read_sector(buf,*(first_layer_ptr));
		den_ptr = (struct dentry*)buf;
		den_ptr += ((now_size-1) % SECTOR_SIZE)/sizeof(struct dentry);
		temp_entry.ino = den_ptr->ino;
		strcpy(temp_entry.name,den_ptr->name);
		memset(den_ptr,0,sizeof(struct dentry));
		if (temp_entry.ino == dentry_t->ino && strcmp(temp_entry.name,dentry_t->name) == 0)
			flag = 1;
		if (now_size % SECTOR_SIZE == sizeof(struct dentry))
			delete_datablock(*(first_layer_ptr)-data_block);
		if (now_size == 10*SECTOR_SIZE+sizeof(struct dentry))
			delete_datablock(inode_table[father_ino].inode->first_layer_ptr-data_block);
	}
	
	now_size = inode_table[father_ino].inode->size;

	if (flag == 0){
	//find the deleting one and replace it
		int i,j;
		int direct_layer_num = min(10,(now_size-1)/SECTOR_SIZE+1);
		for (i = 0; i < direct_layer_num; i++){
			device_read_sector(buf,inode_table[father_ino].inode->direct_ptr[i]);
			den_ptr = (struct dentry*) buf;
			int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(now_size-i*SECTOR_SIZE)/sizeof(struct dentry));
			for (j = 0; j < dentry_num; j++)
				if (den_ptr->ino == dentry_t->ino && strcmp(den_ptr->name,dentry_t->name) == 0){
					flag = 1;
					den_ptr->ino = temp_entry.ino;
					strcpy(den_ptr->name,temp_entry.name);
					device_write_sector(buf,inode_table[father_ino].inode->direct_ptr[i]);
					break;
				}
				else
					den_ptr++;
			if (flag == 1)
				break;
		}
		if (flag == 0 && now_size > SECTOR_SIZE*10){
			now_size -= SECTOR_SIZE*10;
			int first_layer_num = min(1024,(now_size-1)/SECTOR_SIZE+1);
			device_read_sector(first_layer_buf,inode_table[father_ino].inode->first_layer_ptr);
			uint32_t *first_layer_ptr = (uint32_t *)first_layer_buf;
			for (i = 0; i < first_layer_num; i++){
				device_read_sector(buf,*first_layer_ptr);
				den_ptr = (struct dentry*) buf;
				int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(now_size-i*SECTOR_SIZE)/sizeof(struct dentry));
				for (j = 0; j < dentry_num; j++)
					if (den_ptr->ino == dentry_t->ino && strcmp(den_ptr->name,dentry_t->name) == 0){
						flag = 1;
						den_ptr->ino = temp_entry.ino;
						strcpy(den_ptr->name,temp_entry.name);
						device_write_sector(buf,*first_layer_ptr);
						break;
					}
					else
						den_ptr++;
				if (flag == 1)
					break;	
				first_layer_ptr++;
			}

		}	
	}
	
	device_read_sector(buf,ino_block+father_ino/32);
	memcpy(buf+(father_ino%32)*128,inode_table[father_ino].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+father_ino/32);
	return 0;
}


int del_file(int ino){ //delete the file(dir)
	int file_size = inode_table[ino].inode->size;
	int i,j;
	unsigned char first_layer_buf[SECTOR_SIZE];
	unsigned char second_layer_buf[SECTOR_SIZE];
	int first_layer_num,second_layer_num;
	uint32_t *first_layer_ptr,*second_layer_ptr;
	
	if (file_size > 0){
		for (i=0; i<=min(9,(file_size-1)/SECTOR_SIZE); i++)
			delete_datablock((inode_table[ino].inode->direct_ptr[i])-data_block);
		if (file_size > 10*SECTOR_SIZE){
			file_size -= 10*SECTOR_SIZE;
			first_layer_num = min(1024,(file_size-1)/SECTOR_SIZE+1);
			device_read_sector(first_layer_buf,inode_table[ino].inode->first_layer_ptr);
			first_layer_ptr = (uint32_t *)first_layer_buf;
			for (i = 0; i < first_layer_num; i++){
				delete_datablock((*first_layer_ptr)-data_block);
				first_layer_ptr++;
			}
			delete_datablock(inode_table[ino].inode->first_layer_ptr-data_block);
		}
		if (file_size > 1024*SECTOR_SIZE){
			file_size -= 1024*SECTOR_SIZE;
			first_layer_num = min(1024,(file_size-1)/(1024*SECTOR_SIZE)+1);
			device_read_sector(first_layer_buf,inode_table[ino].inode->second_layer_ptr);
			first_layer_ptr = (uint32_t *)first_layer_buf;
			for (i = 0; i < first_layer_num; i++){
				second_layer_num = min(1024,(file_size - i*1024*SECTOR_SIZE-1)/SECTOR_SIZE+1);
				device_read_sector(second_layer_buf,*first_layer_ptr);
				second_layer_ptr = (uint32_t *)second_layer_buf;
				for (j = 0; j < second_layer_num; j++){
					delete_datablock((*second_layer_ptr)-data_block);
					second_layer_ptr++;
				}
				delete_datablock(*first_layer_ptr-data_block);
				first_layer_ptr++;
			}
			delete_datablock(inode_table[ino].inode->second_layer_ptr-data_block);
		}
	}
	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,ino_block+ino/32);
	inode_table[ino].inode->access_time = time(NULL);
	inode_table[ino].inode->modify_time = inode_table[ino].inode->access_time;
	memcpy(buf+(ino%32)*128,inode_table[ino].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ino/32);
	return 0;
}



int parse_path(const char *path){
	struct entry_m *now_entry,*child;
	struct dentry* temp_dentry;
	int path_len = strlen(path);
	int path_index = 0;
	int flag = 0;  // 0: not found in memory  1: found in memory  2: not found in disk  3: found in disk   4: have no such path
	if (path[0]=='/'){
		now_entry = root_dir;
		path_index++;
	}
	else 
		flag = 4;

	if (path[path_len-1] == '/')
		path_len--;
	
	char temp_name[NAME_LEN];
	unsigned char buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	int temp_name_len;
	int dir_size;
	int i,j;
	
	while (path_index < path_len && flag != 4){
		temp_name_len = 0;
		while (path[path_index] != '/' && path_index < path_len)
			temp_name[temp_name_len++]  = path[path_index++];
		temp_name[temp_name_len] = 0;
		path_index++;
		if (flag == 0){
			child = now_entry->first_child;
			while (child){
				if (strcmp(child->name,temp_name) == 0){
					now_entry = child;
					flag = 1;
					break;
				}
				else 
					child = child->sibling;
			}
			if (flag == 0)
				flag = 2;
			else
				flag = 0;
		}
		
		if (flag == 2){
			dir_size = inode_table[now_entry->ino].inode->size;
			if (dir_size != 0){
				int direct_layer_num = min(10,(dir_size-1)/SECTOR_SIZE+1);
				for (i = 0; i < direct_layer_num; i++){
					device_read_sector(buf,inode_table[now_entry->ino].inode->direct_ptr[i]);
					temp_dentry = (struct dentry*) buf;
					int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(dir_size-i*SECTOR_SIZE)/sizeof(struct dentry));
					for (j = 0; j < dentry_num; j++)
						if (strcmp(temp_dentry->name,temp_name) == 0){
							flag = 3;
							temp_alloc = (struct entry_m*) malloc(sizeof(struct entry_m));
							temp_alloc->ino = temp_dentry->ino;
							strcpy(temp_alloc->name,temp_dentry->name);
							temp_alloc->first_child = NULL;
							temp_alloc->sibling = NULL;
							inode_table[temp_dentry->ino].tree_node = temp_alloc;
							if (now_entry->first_child){
								temp_alloc->sibling = now_entry->first_child;
								now_entry->first_child =temp_alloc; 
							}
							else 
								now_entry->first_child =temp_alloc; 
							now_entry = temp_alloc;
							break;
						}
						else
							temp_dentry++;
					if (flag == 3)
						break;
				}
			}
			if (flag == 2 && dir_size > SECTOR_SIZE*10){
				dir_size -= SECTOR_SIZE*10;
				int first_layer_num = min(1024,(dir_size-1)/SECTOR_SIZE+1);
				device_read_sector(first_layer_buf,inode_table[now_entry->ino].inode->first_layer_ptr);
				uint32_t *first_layer_ptr = (uint32_t *)first_layer_buf;
				for (i = 0; i < first_layer_num; i++){
					device_read_sector(buf,*first_layer_ptr);
					temp_dentry = (struct dentry*) buf;
					int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(dir_size-i*SECTOR_SIZE)/sizeof(struct dentry));
					for (j = 0; j < dentry_num; j++)
						if (strcmp(temp_dentry->name,temp_name) == 0){
							flag = 3;
							temp_alloc = (struct entry_m*) malloc(sizeof(struct entry_m));
							temp_alloc->ino = temp_dentry->ino;
							strcpy(temp_alloc->name,temp_dentry->name);
							temp_alloc->first_child = NULL;
							temp_alloc->sibling = NULL;
							inode_table[temp_dentry->ino].tree_node = temp_alloc;
							if (now_entry->first_child){
								temp_alloc->sibling = now_entry->first_child;
								now_entry->first_child =temp_alloc; 
							}
							else 
								now_entry->first_child =temp_alloc; 
							now_entry = temp_alloc;
							break;
						}
						else
							temp_dentry++;
					if (flag == 3)
						break;
					else
						first_layer_ptr++;
				}
			}
			if (flag == 2)
				flag = 4;
			else 
				flag = 2;
		}
	}
	if (flag == 4 && path_index < path_len)
		return -2;   //the front layer not exist
	else if (flag == 4)
		return -1;   //the last layer not exist
	else 
		return now_entry->ino;
}

int p6fs_mkdir(const char *path, mode_t mode)
{
     /*do path parse here
      create dentry  and update your index*/
    int ans = parse_path(path);
	if (ans >=0)
	{
		DEBUG("dir %s has exist!!!",path);
		return -EEXIST;
	}
	
	char temp_name[NAME_LEN];
	int len = strlen(path);
	int index = len - 2;

	while (index >=0 && path[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,path,index);
	temp_name[index] = 0;
	ans = parse_path(temp_name);
	if (ans < 0)
		return -ENOENT;
	if ((inode_table[ans].inode->mode & S_IFDIR) == 0)
		return -ENOTDIR;
	
	//allocate a inode and a datablock
	int new_ino = create_inode();
	if (new_ino == -1)
		return -EOVERFLOW;
	
	int new_datablock = create_datablock();
	if (new_datablock == -1)
		return -EOVERFLOW;
	

	unsigned char buf[SECTOR_SIZE];

	//write new_datablock
	memset(buf,0,SECTOR_SIZE);
	struct dentry* den_ptr;
	den_ptr = (struct dentry*)buf;
	strcpy(den_ptr->name,".");
	den_ptr->ino = new_ino;
	den_ptr++;
	strcpy(den_ptr->name,"..");
	den_ptr->ino = ans;
	device_write_sector(buf,data_block+new_datablock);

	//write new_inode
	device_read_sector(buf,ino_block+new_ino/32);
	struct inode_t* inode_ptr;
	inode_ptr = (struct inode_t*)(buf + 128*(new_ino%32));
	memset(inode_ptr,0,128);
	inode_ptr->size = 2*sizeof(struct dentry);
	inode_ptr->access_time = time(NULL);
	inode_ptr->modify_time = inode_ptr->access_time;
	inode_ptr->create_time = inode_ptr->access_time;
	inode_ptr->mode = S_IFDIR | 0755;
	inode_ptr->count = 1;
	inode_ptr->direct_ptr[0] = data_block+new_datablock;
	device_write_sector(buf,ino_block+new_ino/32);
	memcpy(inode_table[new_ino].inode,inode_ptr,sizeof(struct inode_t));

	//updata the dir tree
	
	//updata the father datablock and inode
	struct dentry dentry_t;
	dentry_t.ino = new_ino;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,path+index);

	add_dentry(ans,&dentry_t);
	parse_path(path);
    return 0;
}

int p6fs_rmdir(const char *path)
{
	int ans = parse_path(path);
	if (ans < 0)
	{
		DEBUG("dir %s not exist!!!",path);
		return -ENOENT;
	}
	if (inode_table[ans].inode->size != 2*sizeof(struct dentry))
		return -ENOTEMPTY;
	if ((inode_table[ans].inode->mode & S_IFDIR) == 0)
		return -ENOTDIR;
	
	char temp_name[NAME_LEN];
	int len = strlen(path);
	int index = len - 2;

	while (index >=0 && path[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,path,index);
	temp_name[index] = 0;
	int father_ans = parse_path(temp_name);
	
	//delete datablock and inode
	del_file(ans);

	//updata the father datablock and inode	
	struct dentry dentry_t;
	dentry_t.ino = ans;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,path+index);
	del_dentry(father_ans,&dentry_t);
	delete_inode(ans);

    return 0;
}

int p6fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{
	int ans = parse_path(path);
	int i,j;
	struct dentry* temp_dentry;
	unsigned char temp_buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	if (ans < 0) 
		return -ENOTDIR; 
	int dir_size = inode_table[ans].inode->size;
	if (dir_size != 0){
		int direct_layer_num = min(10,(dir_size-1)/SECTOR_SIZE+1);
		for (i = 0; i < direct_layer_num; i++){
			device_read_sector(temp_buf,inode_table[ans].inode->direct_ptr[i]);
			temp_dentry = (struct dentry*) temp_buf;
			int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(dir_size-i*SECTOR_SIZE)/sizeof(struct dentry));
			for (j = 0; j < dentry_num; j++){
				filler(buf, temp_dentry->name, NULL, 0);
				temp_dentry++;
			}
		}
		if (dir_size > SECTOR_SIZE*10){
			dir_size -= SECTOR_SIZE*10;
			int first_layer_num = min(1024,(dir_size-1)/SECTOR_SIZE+1);
			device_read_sector(first_layer_buf,inode_table[ans].inode->first_layer_ptr);
			uint32_t *first_layer_ptr = (uint32_t *)first_layer_buf;
			for (i = 0; i < first_layer_num; i++){
				device_read_sector(temp_buf,*first_layer_ptr);
				temp_dentry = (struct dentry*) temp_buf;
				int dentry_num = min(SECTOR_SIZE/sizeof(struct dentry),(dir_size-i*SECTOR_SIZE)/sizeof(struct dentry));
				for (j = 0; j < dentry_num; j++){
					filler(buf, temp_dentry->name, NULL, 0);
					temp_dentry++;
				}
				first_layer_ptr++;
			}
		}
	}
    return 0;
}

//optional
//int p6fs_opendir(const char *path, struct fuse_file_info *fileInfo)
//int p6fs_releasedir(const char *path, struct fuse_file_info *fileInfo)
//int p6fs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo)


//file operations
int p6fs_mknod(const char *path, mode_t mode, dev_t dev)
{
 /*do path parse here
  create file*/
  	int ans = parse_path(path);
	if (ans >= 0)
	{
		DEBUG("file %s has exist!!!",path);
		return -EEXIST;
	}
	
	char temp_name[NAME_LEN];
	int len = strlen(path);
	int index = len - 2;

	while (index >=0 && path[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,path,index);
	temp_name[index] = 0;
	ans = parse_path(temp_name);
	if (ans < 0)
		return -ENOENT;
	if ((inode_table[ans].inode->mode & S_IFDIR) == 0)
		return -ENOTDIR;
	
	//allocate a inode and a datablock
	int new_ino = create_inode();
	if (new_ino == -1)
		return -EOVERFLOW;
	

	unsigned char buf[SECTOR_SIZE];
	//write new_inode
	device_read_sector(buf,ino_block+new_ino/32);
	struct inode_t* inode_ptr;
	inode_ptr = (struct inode_t*)(buf + 128*(new_ino%32));
	memset(inode_ptr,0,128);
	inode_ptr->size = 0;
	inode_ptr->access_time = time(NULL);
	inode_ptr->modify_time = inode_ptr->access_time;
	inode_ptr->create_time = inode_ptr->access_time;
	inode_ptr->mode = S_IFREG | 0644;
	inode_ptr->count = 1;
	device_write_sector(buf,ino_block+new_ino/32);
	memcpy(inode_table[new_ino].inode,inode_ptr,sizeof(struct inode_t));

	//updata the father datablock and inode
	struct dentry dentry_t;
	dentry_t.ino = new_ino;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,path+index);

	add_dentry(ans,&dentry_t);
	parse_path(path);
  	return 0;
}

int p6fs_readlink(const char *path, char *link, size_t size)
{
	int ans = parse_path(path);
	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;
	if ((inode_table[ans].inode->mode & S_IFLNK) == 0)
		return -EINVAL;

	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,inode_table[ans].inode->direct_ptr[0]);
	memcpy(link,buf,size);
	
	device_read_sector(buf,ino_block+ans/32);
	inode_table[ans].inode->access_time = time(NULL);
	memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ans/32);
	
	return 0;
}

int p6fs_symlink(const char *path, const char *link)
{
	int ans = parse_path(link);
	if (ans >=0)
	{
		DEBUG("file %s has exist!!!",link);
		return -EEXIST;
	}
	
	char temp_name[NAME_LEN];
	int len = strlen(link);
	int index = len - 2;

	while (index >=0 && link[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,link,index);
	temp_name[index] = 0;
	ans = parse_path(temp_name);
	if (ans < 0)
		return -ENOENT;
	if ((inode_table[ans].inode->mode & S_IFDIR) == 0)
		return -ENOTDIR;
	
	//allocate a inode and a datablock
	int new_ino = create_inode();
	if (new_ino == -1)
		return -EOVERFLOW;
	
	int new_datablock = create_datablock();
	if (new_datablock == -1)
		return -EOVERFLOW;
	

	unsigned char buf[SECTOR_SIZE];

	//write new_datablock
	memset(buf,0,SECTOR_SIZE);
	char* char_ptr;
	char_ptr = (char *)buf;
	strcpy(char_ptr,path);
	device_write_sector(buf,data_block+new_datablock);

	//write new_inode
	device_read_sector(buf,ino_block+new_ino/32);
	struct inode_t* inode_ptr;
	inode_ptr = (struct inode_t*)(buf + 128*(new_ino%32));
	memset(inode_ptr,0,128);
	inode_ptr->size = strlen(path);
	inode_ptr->access_time = time(NULL);
	inode_ptr->modify_time = inode_ptr->access_time;
	inode_ptr->create_time = inode_ptr->access_time;
	inode_ptr->mode = S_IFLNK | 0755;
	inode_ptr->count = 1;
	inode_ptr->direct_ptr[0] = data_block+new_datablock;
	device_write_sector(buf,ino_block+new_ino/32);
	memcpy(inode_table[new_ino].inode,inode_ptr,sizeof(struct inode_t));

	//updata the father datablock and inode
	struct dentry dentry_t;
	dentry_t.ino = new_ino;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,link+index);

	add_dentry(ans,&dentry_t);
	parse_path(link);
    return 0;
}

int p6fs_link(const char *path, const char *newpath)
{

	int new_ans = parse_path(newpath);
	if (new_ans >= 0)
		return -EEXIST;
	if (new_ans == -2)
		return -ENOENT;

	int ans = parse_path(path);
	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;

	if ((inode_table[ans].inode->mode & S_IFDIR) != 0)
		return -EISDIR;
			
	char temp_name[NAME_LEN];
	int len = strlen(newpath);
	int index = len - 2;
	while (index >=0 && newpath[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,newpath,index);
	temp_name[index] = 0;
	new_ans = parse_path(temp_name);
	if (new_ans < 0)
		return -ENOENT;
	if ((inode_table[new_ans].inode->mode & S_IFDIR) == 0)
		return -ENOTDIR;
		
	
	//write new_inode
	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,ino_block+ans/32);
	inode_table[ans].inode->count++;
	inode_table[ans].inode->modify_time = time(NULL);
	memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ans/32);

	
	//updata the father datablock and inode
	struct dentry dentry_t;
	dentry_t.ino = ans;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,newpath+index);
	add_dentry(new_ans,&dentry_t);
	parse_path(newpath);
	return 0;

}

int p6fs_unlink(const char *path)
{
    int ans = parse_path(path);
	if (ans < 0)
	{
		DEBUG("file %s not exist!!!",path);
		return -ENOENT;
	}
	
	if ((inode_table[ans].inode->mode & S_IFDIR) != 0)
		return -EISDIR;


	char temp_name[NAME_LEN];
	int len = strlen(path);
	int index = len - 2;

	while (index >=0 && path[index] != '/')
		index--;
	if (index == 0)
		index = 1;
	memcpy(temp_name,path,index);
	temp_name[index] = 0;
	int father_ans = parse_path(temp_name);

	struct dentry dentry_t;
	dentry_t.ino = ans;
	if (index != 1)
		index++;
	strcpy(dentry_t.name,path+index);
	

	
	if (inode_table[ans].inode->count > 1){
		unsigned char buf[SECTOR_SIZE];
		device_read_sector(buf,ino_block+ans/32);
		inode_table[ans].inode->count--;
		inode_table[ans].inode->modify_time = time(NULL);
		memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
		device_write_sector(buf,ino_block+ans/32);
		del_dentry(father_ans,&dentry_t);
		return 0;
	}
	

	
	
	//delete datablock and inode
	del_file(ans);

	//updata the father datablock and inode	
	del_dentry(father_ans,&dentry_t);
	delete_inode(ans);

    return 0;
}


int p6fs_open(const char *path, struct fuse_file_info *fileInfo)
{
 /*
  Implemention Example:
  S1: look up and get dentry of the path
  S2: create file handle! Do NOT lookup in read() or write() later
  */
    int ans = parse_path(path);
 	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;

	if ((inode_table[ans].inode->mode & S_IFDIR) != 0)
		return -EISDIR;


    //assign and init your file handle
    struct file_info *fi;
	int i,flag = -1;
	for (i = 0; i < MAX_OPEN_FILE; i++)
		if (fd_table[i].used == FALSE){
			flag = 1;
			fd_table[i].used = TRUE;
			fd_table[i].flag = fileInfo->flags;
			flag = i;
			fd_table[i].ino = ans;
			break;
		}
	if (flag == -1)
		return -ENFILE;


    //check open flags, such as O_RDONLY
    //O_CREATE is tansformed to mknod() + open() by fuse ,so no need to create file here
    

    fi = fd_table+flag;
 
    fileInfo->fh = (uint64_t)fi;
	return 0;

}

int p6fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    /* get inode from file handle and do operation*/
	int ino = ((struct file_info*)(fileInfo->fh))->ino;

	if ((((struct file_info*)(fileInfo->fh))->flag & 0x3)== O_WRONLY)
		return -EACCES;

	int i,j;
	int have_read = 0 ,actual_size = min(inode_table[ino].inode->size-offset,size);
	unsigned char temp_buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	unsigned char second_layer_buf[SECTOR_SIZE];
	int first_layer_num,second_layer_num;
	uint32_t *first_layer_ptr,*second_layer_ptr;

	int flag = 0;
	if (offset < 10*SECTOR_SIZE){
		for (i = offset/SECTOR_SIZE; i < 10; i++){
			device_read_sector(temp_buf,inode_table[ino].inode->direct_ptr[i]);
			if (have_read == 0){
				memcpy(buf, temp_buf + offset%SECTOR_SIZE,min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE));
				have_read += min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE);
			}
			else if (actual_size-have_read < SECTOR_SIZE){
				memcpy(buf + have_read, temp_buf,actual_size-have_read);
				have_read += actual_size-have_read;
			}
			else{
				memcpy(buf + have_read, temp_buf,SECTOR_SIZE);
				have_read += SECTOR_SIZE;
			}
			if (have_read == actual_size){
				flag = 1;
				break;
			}
		}
	}
	if (offset < (10+1024)*SECTOR_SIZE && flag == 0){
		if (offset < 10*SECTOR_SIZE)
			first_layer_num = 0;
		else
			first_layer_num = offset/SECTOR_SIZE-10;
		device_read_sector(first_layer_buf, inode_table[ino].inode->first_layer_ptr);
		first_layer_ptr = (uint32_t *)first_layer_buf;
		for (i = first_layer_num; i < 1024; i++){
			device_read_sector(temp_buf,*(first_layer_ptr+i));
			if (have_read == 0){
				memcpy(buf, temp_buf + offset%SECTOR_SIZE,min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE));
				have_read += min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE);
			}
			else if (actual_size-have_read < SECTOR_SIZE){
				memcpy(buf + have_read, temp_buf,actual_size-have_read);
				have_read += actual_size-have_read;
			}
			else{
				memcpy(buf + have_read, temp_buf,SECTOR_SIZE);
				have_read += SECTOR_SIZE;
			}
			if (have_read == actual_size){
				flag = 1;
				break;
			}
		}
	}
	if (flag == 0){
		if (offset < (10+1024)*SECTOR_SIZE)
			second_layer_num = 0;
		else
			second_layer_num = (offset/SECTOR_SIZE-(10+1024))/1024;
		device_read_sector(second_layer_buf, inode_table[ino].inode->second_layer_ptr);
		second_layer_ptr = (uint32_t *)second_layer_buf;
		for (i = second_layer_num; i < 1024; i++){
			device_read_sector(first_layer_buf,*(second_layer_ptr+i));
			first_layer_ptr = (uint32_t *)first_layer_buf;
			if (have_read == 0)
				first_layer_num = ((offset-(10+1024)*SECTOR_SIZE)%(1024*SECTOR_SIZE))/SECTOR_SIZE;
			else 
				first_layer_num = 0;
			for (j = first_layer_num; j < 1024; j++){
				device_read_sector(temp_buf,*(first_layer_ptr+j));
				if (have_read == 0){
					memcpy(buf, temp_buf + offset%SECTOR_SIZE,min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE));
					have_read += min(actual_size,SECTOR_SIZE-offset%SECTOR_SIZE);
				}
				else if (actual_size-have_read < SECTOR_SIZE){
					memcpy(buf + have_read, temp_buf,actual_size-have_read);
					have_read += actual_size-have_read;
				}
				else{
					memcpy(buf + have_read, temp_buf,SECTOR_SIZE);
					have_read += SECTOR_SIZE;
				}
				if (have_read == actual_size){
					flag = 1;
					break;
				}
			}
			if (flag == 1)
				break;
		}
	}
	//memset(buf+have_read,0,size-actual_size);
	device_read_sector(temp_buf,ino_block+ino/32);
	inode_table[ino].inode->access_time = time(NULL);
	memcpy(temp_buf+(ino%32)*128,inode_table[ino].inode,sizeof(struct inode_t));
	device_write_sector(temp_buf,ino_block+ino/32);
    return have_read;
}

int p6fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    /* get inode from file handle and do operation*/
	int ino = ((struct file_info*)(fileInfo->fh))->ino;
	if ((((struct file_info *)(fileInfo->fh))->flag & 0x3)== O_RDONLY)
		return -EACCES;


	//allocate enough space for write
	unsigned char temp_buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	unsigned char second_layer_buf[SECTOR_SIZE];
	uint32_t *first_layer_ptr,*second_layer_ptr;
	int flag = 0,i,j;
	int raw_size = inode_table[ino].inode->size;
	int now_size;
		if (raw_size == 0)
			now_size = 0;
		else 
			now_size = ((raw_size-1)/SECTOR_SIZE +1)*SECTOR_SIZE;
	if (size+offset > now_size){
		int direct_layer;
		if (now_size == 0)
			direct_layer = 0;
		else if (now_size <= 9*SECTOR_SIZE)
			direct_layer = (now_size-1)/SECTOR_SIZE+1;
		else 
			direct_layer = 10;
		for (i = direct_layer; i < 10; i++){
			inode_table[ino].inode->direct_ptr[i] = create_datablock()+data_block;
			now_size += SECTOR_SIZE;
			if (now_size >= size+offset){
				flag = 1;
				break;
			}
		}
		if (flag == 0){
			int first_layer;
			if (raw_size <= 10*SECTOR_SIZE){
				inode_table[ino].inode->first_layer_ptr = create_datablock()+data_block;
				first_layer = 0;
			}
			else if (now_size <= (10+1023)*SECTOR_SIZE)
				first_layer = (now_size-1)/SECTOR_SIZE+1-10;
			else 
				first_layer = 1024;
			device_read_sector(first_layer_buf,inode_table[ino].inode->first_layer_ptr);
			first_layer_ptr = (uint32_t *)first_layer_buf;
			for (i = first_layer; i < 1024; i++){
				*(first_layer_ptr + i) = create_datablock()+data_block;
				now_size += SECTOR_SIZE;
				if (now_size >= size+offset){
					flag = 1;
					break;
				}
			}
		}
		if (flag == 0){
			int second_layer;
			if (raw_size <= (10+1024)*SECTOR_SIZE){
				inode_table[ino].inode->second_layer_ptr = create_datablock()+data_block;
				second_layer = 0;
			}
			else
				second_layer = ((now_size-1)/SECTOR_SIZE+1-10-1024)/1024;

			device_read_sector(second_layer_buf,inode_table[ino].inode->second_layer_ptr);
			second_layer_ptr = (uint32_t *)second_layer_buf;
			for (i = second_layer; i < 1024; i++){
				if (raw_size <= (i*1024+1024+10)*SECTOR_SIZE)
					*(second_layer_ptr+i) = create_datablock()+data_block;
				
				device_read_sector(first_layer_buf,*(second_layer_ptr+i));
				first_layer_ptr = (uint32_t *)first_layer_buf;
				int first_layer;
				if (raw_size <= (i*1024+1024+10)*SECTOR_SIZE)
					first_layer = 0;
				else
					first_layer = (now_size/SECTOR_SIZE-10-1024)%1024;
				for (i = first_layer; i < 1024; i++){
					*(first_layer_ptr + i) = create_datablock()+data_block;
					now_size += SECTOR_SIZE;
					if (now_size >= size+offset){
						flag = 1;
						break;
					}
				}
				if (flag == 1)
					break;
			}
		}
	}


	int first_layer_num,second_layer_num;
	int have_write = 0;
	flag = 0;
	if (offset < 10*SECTOR_SIZE){
		for (i = offset/SECTOR_SIZE; i < 10; i++){
			memset(temp_buf,0,SECTOR_SIZE);
			
			if (have_write == 0){
				device_read_sector(temp_buf,inode_table[ino].inode->direct_ptr[i]);
				memcpy(temp_buf + offset%SECTOR_SIZE,buf,min(size,SECTOR_SIZE-offset%SECTOR_SIZE));
				have_write += min(size,SECTOR_SIZE-offset%SECTOR_SIZE);
			}
			else if (size-have_write < SECTOR_SIZE){
				memcpy(temp_buf,buf + have_write, size-have_write);
				have_write += size-have_write;
			}
			else{
				memcpy(temp_buf,buf + have_write,SECTOR_SIZE);
				have_write += SECTOR_SIZE;
			}
			device_write_sector(temp_buf,inode_table[ino].inode->direct_ptr[i]);
			if (have_write == size){
				flag = 1;
				break;
			}
		}
	}
	if (offset < (10+1024)*SECTOR_SIZE && flag == 0){
		if (offset < 10*SECTOR_SIZE)
			first_layer_num = 0;
		else
			first_layer_num = offset/SECTOR_SIZE-10;
		device_read_sector(first_layer_buf, inode_table[ino].inode->first_layer_ptr);
		first_layer_ptr = (uint32_t *)first_layer_buf;
		for (i = first_layer_num; i < 1024; i++){
			memset(temp_buf,0,SECTOR_SIZE);
			if (have_write == 0){
				device_read_sector(temp_buf,*(first_layer_ptr+i));
				memcpy(temp_buf + offset%SECTOR_SIZE,buf,min(size,SECTOR_SIZE-offset%SECTOR_SIZE));
				have_write += min(size,SECTOR_SIZE-offset%SECTOR_SIZE);
			}
			else if (size-have_write < SECTOR_SIZE){
				memcpy(temp_buf,buf + have_write,size-have_write);
				have_write += size-have_write;
			}
			else{
				memcpy(temp_buf,buf + have_write, SECTOR_SIZE);
				have_write += SECTOR_SIZE;
			}
			device_write_sector(temp_buf,*(first_layer_ptr+i));
			if (have_write == size){
				flag = 1;
				break;
			}
		}
	}
	if (flag == 0){
		if (offset < (10+1024)*SECTOR_SIZE)
			second_layer_num = 0;
		else
			second_layer_num = (offset/SECTOR_SIZE-(10+1024))/1024;
		device_read_sector(second_layer_buf, inode_table[ino].inode->second_layer_ptr);
		second_layer_ptr = (uint32_t *)second_layer_buf;
		for (i = second_layer_num; i < 1024; i++){
			device_read_sector(first_layer_buf,*(second_layer_ptr+i));
			first_layer_ptr = (uint32_t *)first_layer_buf;
			if (have_write == 0)
				first_layer_num = ((offset-(10+1024)*SECTOR_SIZE)%(1024*SECTOR_SIZE))/SECTOR_SIZE;
			else 
				first_layer_num = 0;
			for (j = first_layer_num; j < 1024; j++){
				memset(temp_buf,0,SECTOR_SIZE);
				if (have_write == 0){
					device_read_sector(temp_buf,*(first_layer_ptr+j));
					memcpy(temp_buf + offset%SECTOR_SIZE,buf,min(size,SECTOR_SIZE-offset%SECTOR_SIZE));
					have_write += min(size,SECTOR_SIZE-offset%SECTOR_SIZE);
				}
				else if (size-have_write < SECTOR_SIZE){
					memcpy(temp_buf,buf + have_write,size-have_write);
					have_write += size-have_write;
				}
				else{
					memcpy(temp_buf,buf + have_write,SECTOR_SIZE);
					have_write += SECTOR_SIZE;
				}
				device_write_sector(temp_buf,*(first_layer_ptr+j));
				if (have_write == size){
					flag = 1;
					break;
				}
			}
			if (flag == 1)
				break;
		}
	}

	device_read_sector(temp_buf,ino_block+ino/32);
	inode_table[ino].inode->size = max(inode_table[ino].inode->size,offset+size);
	inode_table[ino].inode->modify_time = time(NULL);
	inode_table[ino].inode->access_time = inode_table[ino].inode->modify_time;
	memcpy(temp_buf+(ino%32)*128,inode_table[ino].inode,sizeof(struct inode_t));
	device_write_sector(temp_buf,ino_block+ino/32);
	
    return have_write;
}

int p6fs_truncate(const char *path, off_t newsize)
{
	int ans = parse_path(path);
 	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;

	if ((inode_table[ans].inode->mode & S_IFDIR) != 0)
		return -EISDIR;

	if (inode_table[ans].inode->size < newsize){	
		unsigned char temp_buf[SECTOR_SIZE];
		device_read_sector(temp_buf,ino_block+ans/32);
		inode_table[ans].inode->size = newsize;
		inode_table[ans].inode->modify_time = time(NULL);
		inode_table[ans].inode->access_time = inode_table[ans].inode->modify_time;
		memcpy(temp_buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
		device_write_sector(temp_buf,ino_block+ans/32);
		return 0;
	}
	
	unsigned char buf[SECTOR_SIZE];
	unsigned char first_layer_buf[SECTOR_SIZE];
	unsigned char second_layer_buf[SECTOR_SIZE];
	int direct_layer,first_layer,second_layer;
	uint32_t *first_layer_ptr, *second_layer_ptr;
	int i,j,flag = 0, file_size = inode_table[ans].inode->size;
	if (newsize == 0){
		del_file(ans);
		flag = 1;
	}
	else if (newsize < 10*SECTOR_SIZE){
		if (newsize % SECTOR_SIZE != 0){
			direct_layer = newsize/SECTOR_SIZE;
			device_read_sector(buf,inode_table[ans].inode->direct_ptr[direct_layer]);
			memset(buf+newsize%SECTOR_SIZE,0,SECTOR_SIZE-newsize%SECTOR_SIZE);
			device_write_sector(buf,inode_table[ans].inode->direct_ptr[direct_layer]);
			if (direct_layer*SECTOR_SIZE >= file_size){
				flag = 1;
			}
			else
			for ( i = direct_layer+1; i < 10; i++){
				delete_datablock(inode_table[ans].inode->direct_ptr[i]-data_block);
				if (i*SECTOR_SIZE >= file_size){
					flag = 1;
					break;
				}
			}
		}
		else{
			direct_layer = newsize/SECTOR_SIZE;
			for ( i = direct_layer; i < 10; i++){
				delete_datablock(inode_table[ans].inode->direct_ptr[i]-data_block);
				if (i*SECTOR_SIZE >= file_size){
					flag = 1;
					break;
				}
			}
		}
	}
	if (flag == 0 && newsize < (10+1024)*SECTOR_SIZE){
		if (newsize % SECTOR_SIZE != 0 && newsize > 10*SECTOR_SIZE){
			device_read_sector(first_layer_buf,inode_table[ans].inode->first_layer_ptr);
			first_layer_ptr = (uint32_t *)first_layer_buf;
			first_layer = newsize/SECTOR_SIZE - 10;
			device_read_sector(buf,*(first_layer_ptr+first_layer));
			memset(buf+newsize%SECTOR_SIZE,0,SECTOR_SIZE-newsize%SECTOR_SIZE);
			device_write_sector(buf,*(first_layer_ptr+first_layer));
			if ((first_layer+10)*SECTOR_SIZE >= file_size){
				flag = 1;
			}
			else
			for ( i = first_layer+1; i < 1024; i++){
				delete_datablock(*(first_layer_ptr+i)-data_block);
				if ((i+10)*SECTOR_SIZE >= file_size){
					flag = 1;
					break;
				}
			}
		}
		else{
			device_read_sector(first_layer_buf,inode_table[ans].inode->first_layer_ptr);
			first_layer_ptr = (uint32_t *)first_layer_buf;
			first_layer = max(0,newsize/SECTOR_SIZE - 10);		
			for ( i = first_layer; i < 1024; i++){
				delete_datablock(*(first_layer_ptr+i)-data_block);
				if ((i+10)*SECTOR_SIZE >= file_size){
					flag = 1;
					break;
				}
			}
			if (newsize == 10*SECTOR_SIZE)
				delete_datablock(inode_table[ans].inode->first_layer_ptr-data_block);
		}
	}
	if (flag == 0){
		if (newsize <= (10+1024)*SECTOR_SIZE){
			device_read_sector(second_layer_buf,inode_table[ans].inode->second_layer_ptr);
			second_layer_ptr = (uint32_t *)second_layer_buf;
			for (i = 0; i < 1024; i++){
				device_read_sector(first_layer_buf,*(second_layer_ptr+i));
				first_layer_ptr = (uint32_t *)first_layer_buf;
				for (j = 0; j < 1024; j++){
					delete_datablock(*(first_layer_ptr+j)-data_block);
					if ((i*1024+j+10+1024)*SECTOR_SIZE >= file_size){
						flag = 1;
						break;
					}
				}
				delete_datablock(*(second_layer_ptr+i)-data_block);
				if (flag == 1)
					break;
			}
			delete_datablock(inode_table[ans].inode->second_layer_ptr-data_block);
		}
		else{
			device_read_sector(second_layer_buf,inode_table[ans].inode->second_layer_ptr);
			second_layer_ptr = (uint32_t *)second_layer_buf;
			second_layer = (newsize/SECTOR_SIZE - 10- 1024)/1024;
			if ((newsize-(10+1024)*SECTOR_SIZE)%(1024*SECTOR_SIZE) != 0){
				if (newsize % SECTOR_SIZE != 0){
					device_read_sector(first_layer_buf,*(second_layer_ptr+second_layer));
					first_layer_ptr = (uint32_t *)first_layer_buf;
					first_layer = ((newsize-(10+1024)*SECTOR_SIZE)/SECTOR_SIZE)%1024;
					device_read_sector(buf,*(first_layer_ptr+first_layer));
					memset(buf+newsize%SECTOR_SIZE,0,SECTOR_SIZE-newsize%SECTOR_SIZE);
					device_write_sector(buf,*(first_layer_ptr+first_layer));
					if ((second_layer*1024+first_layer+10+1024)*SECTOR_SIZE >= file_size){
							flag = 1;
					}
					else
					for (j = first_layer + 1; j < 1024; j++){
						delete_datablock(*(first_layer_ptr+j)-data_block);
						if ((second_layer*1024+j+10+1024)*SECTOR_SIZE >= file_size){
							flag = 1;
							break;
						}
					}
					if (flag == 0){
						for (i = second_layer+1; i < 1024; i++){
							device_read_sector(first_layer_buf,*(second_layer_ptr+i));
							first_layer_ptr = (uint32_t *)first_layer_buf;
							for (j = 0; j < 1024; j++){
								delete_datablock(*(first_layer_ptr+j)-data_block);
								if ((i*1024+j+10+1024)*SECTOR_SIZE >= file_size){
									flag = 1;
									break;
								}
							}
							delete_datablock(*(second_layer_ptr+i)-data_block);
							if (flag == 1)
								break;
						}
					}
				}
				else {
					device_read_sector(first_layer_buf,*(second_layer_ptr+second_layer));
					first_layer_ptr = (uint32_t *)first_layer_buf;
					first_layer = ((newsize-(10+1024)*SECTOR_SIZE)/SECTOR_SIZE)%1024;
					for (j = first_layer; j < 1024; j++){
						delete_datablock(*(first_layer_ptr+j)-data_block);
						if ((second_layer*1024+j+10+1024)*SECTOR_SIZE >= file_size){
							flag = 1;
							break;
						}
					}
					if (flag == 0){
						for (i = second_layer+1; i < 1024; i++){
							device_read_sector(first_layer_buf,*(second_layer_ptr+i));
							first_layer_ptr = (uint32_t *)first_layer_buf;
							for (j = 0; j < 1024; j++){
								delete_datablock(*(first_layer_ptr+j)-data_block);
								if ((i*1024+j+10+1024)*SECTOR_SIZE >= file_size){
									flag = 1;
									break;
								}
							}
							delete_datablock(*(second_layer_ptr+i)-data_block);
							if (flag == 1)
								break;
							
						}
					}
				}			
			}
			else{
				for (i = second_layer; i < 1024; i++){
					device_read_sector(first_layer_buf,*(second_layer_ptr+i));
					first_layer_ptr = (uint32_t *)first_layer_buf;
					for (j = 0; j < 1024; j++){
						delete_datablock(*(first_layer_ptr+j)-data_block);
						if ((i*1024+j+10+1024)*SECTOR_SIZE >= file_size){
							flag = 1;
							break;
						}
					}
					delete_datablock(*(second_layer_ptr+i)-data_block);
					if (flag == 1)
						break;
				}
			}
		}

	}

	
	device_read_sector(buf,ino_block+ans/32);
	inode_table[ans].inode->size = newsize;
	inode_table[ans].inode->modify_time = time(NULL);
	inode_table[ans].inode->access_time = inode_table[ans].inode->modify_time;
	memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ans/32);

    return 0;
}

//optional
//p6fs_flush(const char *path, struct fuse_file_info *fileInfo)
//int p6fs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
int p6fs_release(const char *path, struct fuse_file_info *fileInfo)
{
    /*release fd*/
	((struct file_info*)(fileInfo->fh))->used = FALSE;
	fileInfo->fh = NULL;
	return 0;
}


int p6fs_getattr(const char *path, struct stat *statbuf)
{
    /*stat() file or directory */
	memset(statbuf, 0, sizeof(struct stat));

	int ans = parse_path(path);
	if (ans == -1)
		return -ENOENT;
	statbuf->st_mode = (inode_table[ans].inode)->mode;
	statbuf->st_nlink = (inode_table[ans].inode)->count;
	statbuf->st_size = (inode_table[ans].inode)->size;
	statbuf->st_mtime = (inode_table[ans].inode)->modify_time;
	statbuf->st_atime = (inode_table[ans].inode)->access_time;
	statbuf->st_ctime = (inode_table[ans].inode)->create_time;
	struct fuse_context *fuse_con = fuse_get_context();
	statbuf->st_uid = fuse_con->uid;
	statbuf->st_gid = fuse_con->gid;
		
	return 0;
}

int p6fs_utime(const char *path, struct utimbuf *ubuf)//optional
{
	int ans = parse_path(path);
 	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;
	
	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,ino_block+ans/32);
	inode_table[ans].inode->modify_time = ubuf->modtime;
	inode_table[ans].inode->access_time = ubuf->actime;
	memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ans/32);

	return 0;
}

int p6fs_chmod(const char *path, mode_t mode) //optional
{
	int ans = parse_path(path);
 	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;
	
	unsigned char buf[SECTOR_SIZE];
	device_read_sector(buf,ino_block+ans/32);
	inode_table[ans].inode->mode = mode;
	memcpy(buf+(ans%32)*128,inode_table[ans].inode,sizeof(struct inode_t));
	device_write_sector(buf,ino_block+ans/32);

	return 0;
}

//int p6fs_chown(const char *path, uid_t uid, gid_t gid);//optional


int p6fs_rename(const char *path, const char *newpath)
{

	int new_ans = parse_path(newpath);
	if (new_ans >= 0)
		return -EEXIST;
	if (new_ans == -2)
		return -ENOENT;

	int ans = parse_path(path);
	if (ans == -1)
		return -ENOENT;
	else if (ans == -2)
		return -ENOTDIR;


			
	char temp_name_new[NAME_LEN];
	int len_new = strlen(newpath);
	int index_new = len_new - 2;
	while (index_new >=0 && newpath[index_new] != '/')
		index_new--;
	if (index_new == 0)
		index_new = 1;
	memcpy(temp_name_new,newpath,index_new);
	temp_name_new[index_new] = 0;
	new_ans = parse_path(temp_name_new);
	if (new_ans < 0)
		return -ENOENT;
		
	
	//updata the father datablock and inode of new_path
	struct dentry dentry_t;
	dentry_t.ino = ans;
	if (index_new != 1)
		index_new++;
	strcpy(dentry_t.name,newpath+index_new);
	add_dentry(new_ans,&dentry_t);
	parse_path(newpath);


	char temp_name_old[NAME_LEN];
	int len_old = strlen(path);
	int index_old = len_old - 2;

	while (index_old >=0 && path[index_old] != '/')
		index_old--;
	if (index_old == 0)
		index_old = 1;
	memcpy(temp_name_old,path,index_old);
	temp_name_old[index_old] = 0;
	int father_ans = parse_path(temp_name_old);
	
	//updata the father datablock and inode of old_path
	dentry_t.ino = ans;
	if (index_old != 1)
		index_old++;
	strcpy(dentry_t.name,path+index_old);
	del_dentry(father_ans,&dentry_t);

	return 0;
}

int p6fs_statfs(const char *path, struct statvfs *statInfo)
{
    /*print fs status and statistics */

	statInfo->f_bsize = 4096;
	statInfo->f_frsize = 4096;
	statInfo->f_blocks = 1024*1024;
	statInfo->f_bfree = free_block;
	statInfo->f_bavail = free_block;
	statInfo->f_files = 1024*512;
	statInfo->f_ffree = free_inode;
	statInfo->f_favail = free_inode;

	return 0;
}

void* p6fs_init(struct fuse_conn_info *conn)
{
    /*init fs
     think what mkfs() and mount() should do.
     create or rebuild memory structures.
     
     e.g
     S1 Read the magic number from disk
     S2 Compare with YOUR Magic
     S3 if(exist)
        then
            mount();
        else
            mkfs();
     */
		
     int i,j,k;
     struct superblock_t* sb_ptr;
	 int flag = 1;
	 unsigned char buf[SECTOR_SIZE];

	 // initial file descriptor table and inode table
	 for (i = 0; i < MAX_OPEN_FILE; i++)
	 	fd_table[i].used = FALSE;
	 for (i = 0; i < MAX_INODE; i++)
	 	pthread_mutex_init(&(inode_table[i].mutex), NULL);

	 free_block = 1032142;
	 free_inode = 512*1024;

	 device_read_sector(buf,0);
	 sb_ptr = (struct superblock_t *)buf;
	 if (sb_ptr->magic_number != MAGIC_NUM){
		device_read_sector(buf,1);
	    sb_ptr = (struct superblock_t *)buf;
	 	if (sb_ptr->magic_number != MAGIC_NUM)
			flag = 0;
	 }
	 
	 if (flag == 1){   //mount
	 	sb_t.magic_number = MAGIC_NUM;
		sb_t.num_of_datablock = sb_ptr->num_of_datablock;
	 	sb_t.num_of_inode = sb_ptr->num_of_inode;
		sb_t.size = sb_ptr->size;
		sb_t.size_of_datablock = sb_ptr->size_of_datablock;
		sb_t.size_of_inode = sb_ptr->size_of_inode;
		sb_t.start_add_datablock = sb_ptr->start_add_datablock;
		sb_t.start_add_datablock_bit = sb_ptr->start_add_datablock_bit;
		sb_t.start_add_inode = sb_ptr->start_add_inode;
		sb_t.start_add_inode_bit = sb_ptr->start_add_inode_bit;

		sb.sb = &sb_t;
		pthread_mutex_init(&(sb.mutex), NULL);
	    for (i = 0; i < 16384; i++)
	    {
	    	device_read_sector(buf,ino_block+i);
			for (j = 0; j < 32; j++){
				inode_table[i*32+j].inode = (struct inode_t *) malloc(sizeof(struct inode_t));
				memcpy(inode_table[i*32+j].inode,&buf[j*128],sizeof(struct inode_t));
			}
	    }

		for (i = ino_bit; i < ino_bit+16; i++)
		{
			device_read_sector(buf,i);
			memcpy(inode_bit_map+(i-ino_bit)*SECTOR_SIZE,buf,SECTOR_SIZE);
			for (j = 0; j < SECTOR_SIZE; j++)
			{
				for (k = 0; k < 8; k++)
				{
					if (buf[j] & 0x1) free_inode--;
					buf[j] = buf[j] >> 1;
				}
			}
		}

		for (i = data_bit; i < data_bit+32; i++)
		{
			device_read_sector(buf,i);
			memcpy(block_bit_map+(i-data_bit)*SECTOR_SIZE,buf,SECTOR_SIZE);
			for (j = 0; j < SECTOR_SIZE; j++)
			{
				for (k = 0; k < 8; k++)
				{
					if (buf[j] & 0x1) free_block--;
					buf[j] = buf[j] >> 1;
				}
			}
		}
		DEBUG("mount finished!!!");
	 }
	 else{           //mkfs
	    memset(buf,0,SECTOR_SIZE);
		sb_ptr = (struct superblock_t *)buf;
		sb_ptr->magic_number = MAGIC_NUM;
	 	sb_ptr->num_of_datablock = 1032142;
		sb_ptr->num_of_inode = 512*1024;
		sb_ptr->size = 4*1024*1024;   //KB
		sb_ptr->size_of_datablock = 4096;
		sb_ptr->size_of_inode = 128;
		sb_ptr->start_add_inode_bit = SECTOR_SIZE*ino_bit;
		sb_ptr->start_add_datablock_bit = SECTOR_SIZE*data_bit;
		sb_ptr->start_add_inode = SECTOR_SIZE*ino_block;
		sb_ptr->start_add_datablock = SECTOR_SIZE*data_block;
		memcpy(&sb_t,buf,sizeof(sb_t));
		sb.sb = &sb_t;
		pthread_mutex_init(&sb.mutex, NULL);
		device_write_sector(buf,0);
		device_write_sector(buf,1);
		
		memset(buf,0,SECTOR_SIZE);
		for (i = ino_bit; i <ino_block; i++)
			device_write_sector(buf,i);
		for (i = 0; i < free_inode; i++)
			inode_table[i].inode = (struct inode_t *) malloc(sizeof(struct inode_t));

		memset(inode_bit_map,0,16*SECTOR_SIZE);
		memset(block_bit_map,0,32*SECTOR_SIZE);

		//write root_dir datablock
		int block_id = create_datablock();
		struct dentry* den_ptr;
	 	den_ptr = (struct dentry*)buf;
	 	strcpy(den_ptr->name,".");
	 	den_ptr->ino = block_id;
	 	den_ptr++;
	 	strcpy(den_ptr->name,"..");
	 	den_ptr->ino = block_id;
	 	device_write_sector(buf,data_block);

		//write root_dir inode
		int inode_id = create_inode();
		memset(buf,0,SECTOR_SIZE);
		struct inode_t* inode_ptr;
		inode_ptr = (struct inode_t*)buf;
		inode_ptr->size = 2*sizeof(struct dentry);
		inode_ptr->access_time = time(NULL);
		inode_ptr->modify_time = inode_ptr->access_time;
		inode_ptr->mode = S_IFDIR | 0755;
		inode_ptr->count = 1;
		inode_ptr->direct_ptr[inode_id] = data_block;
		device_write_sector(buf,ino_block);

		memcpy(inode_table[inode_id].inode,buf,sizeof(struct inode_t));

		DEBUG("mkfs finished!!!");
	 }
	 
	 root_dir = (struct entry_m *) malloc(sizeof(struct entry_m));
	 strcpy(root_dir->name,"/");
	 root_dir->ino = 0;
	 root_dir->sibling = NULL;
	 root_dir->first_child = NULL;

	 inode_table[0].tree_node = root_dir;
    /*HOWTO use @return
     struct fuse_context *fuse_con = fuse_get_context();
     fuse_con->private_data = (void *)xxx;
     return fuse_con->private_data;
     
     the fuse_context is a global variable, you can use it in
     all file operation, and you could also get uid,gid and pid
     from it.
     
     */
    return NULL;
}

void p6fs_destroy(void* private_data)
{
    /*
     flush data to disk
     free memory
     */
    int i;
    for (i = 0; i < MAX_INODE; i++)
		free(inode_table[i].inode);
    device_close();
    logging_close();
}
