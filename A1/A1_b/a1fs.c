/**
 * CSC369 Assignment 1 - a1fs driver implementation.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <math.h>

/* ****************************** */
/* ********** Helpers *********** */
/* ****************************** */

/*
 * updates data bitmap. note start is the blk number not index.
 */
void set_bitmap(unsigned int bitmap_ptr, unsigned int start, unsigned int len, fs_ctx *fs, int clear)
{
	// find the byte b to seek to
	bool check = is_aligned(start, 8);
	int x = start / 8;
	int b;
	if (check)
	{
		b = x - 1;
	}
	else
	{
		b = x;
	}

	unsigned char *bitmap = (unsigned char *)((fs->image + bitmap_ptr * A1FS_BLOCK_SIZE) + b);
	unsigned int count = 0;

	bool a = start >= align_up(start, 8);
	unsigned int t = a ? start - align_up(start, 8) : start - 8 * x - 1;

	while (count < len)
	{
		unsigned char *byte = (unsigned char *)bitmap;
		while (t < 8)
		{
			if (count < len)
			{
				if (clear == 0)
				{
					*byte = *byte | (1 << t);
					count += 1;
				}
				else
				{
					*byte = *byte & ~(1 << t);
					count += 1;
				}
			}
			t++;
		}
		t = 0;
		// go 1 byte ahead
		bitmap = bitmap + 1;
	}
}

/*
* frees inode_number in inode bitmap.
*/
void free_inode_bitmap(short *inode_bitmap, int inode_number)
{
	*(inode_bitmap + inode_number) = *(inode_bitmap + inode_number) & 127;
}

/*
* Removes the new file/dir from the path and puts it into path_cpy.
*/
int remove_new_token(const char *path, char *path_cpy, char *new_dir)
{

	char *new_dirOrFile;

	strncpy(path_cpy, path, strlen(path));

	new_dirOrFile = strrchr(path, '/');
	if (new_dirOrFile != NULL)
	{
		// push pointer to one after
		new_dirOrFile = new_dirOrFile + 1;

		if (strlen(new_dirOrFile) > A1FS_NAME_MAX)
		{
			return -ENAMETOOLONG;
		}
	}

	strncpy(new_dir, new_dirOrFile, strlen(new_dirOrFile) + 1);
	// remove the new dir/file name from the path_cpy
	int len = strlen(path) - strlen(new_dirOrFile);
	strncpy(path_cpy, path, len);
	path_cpy[len] = '\0';

	return 0;
}

/*
* Updates super block with the changes in fs_ctx object.
*/
int update_superblock(fs_ctx *fs)
{
	struct a1fs_superblock *super_block_ptr = (struct a1fs_superblock *)fs->image;
	unsigned int num_of_inodes = fs->inodes_count;
	super_block_ptr->s_inodes_count = num_of_inodes;
	super_block_ptr->s_blocks_count = fs->blocks_count;
	super_block_ptr->s_free_inodes_count = fs->free_inodes_count;
	super_block_ptr->s_free_blocks_count = fs->free_blocks_count;
	return 0;
}

/* 
* finds and validates the cur_token in parent inode by looking for cur_token int it and fills cur_token's inode # in inode_num
*/
int lookup_dentry(char *cur_token, a1fs_ino_t *inode_num, struct a1fs_inode *inode, fs_ctx *fs)
{
	if ((inode->mode & S_IFMT) != S_IFDIR)
	{
		// not a dir
		return -ENOTDIR;
	}

	unsigned int extent_count = inode->i_extent_count;
	// find the dentry for this inode
	struct a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + (inode->i_extents * A1FS_BLOCK_SIZE));

	for (unsigned int i = 0; i < extent_count; i++)
	{
		struct a1fs_extent e = extents_list[i];
		a1fs_blk_t start = e.start;
		a1fs_blk_t count = e.count;
		// find this extent in image
		struct a1fs_dentry *dentry_list = (struct a1fs_dentry *)(fs->image + ((start - 1) * A1FS_BLOCK_SIZE));
		int size = (count * A1FS_BLOCK_SIZE) / sizeof(struct a1fs_dentry);

		for (int t = 0; t < size; t++)
		{
			struct a1fs_dentry dentry = dentry_list[t];
			if (strcmp(dentry.name, cur_token) == 0)
			{
				*inode_num = dentry.ino;
				return 0;
			}
		}
	}
	return -ENOENT;
}

/*
* find next free extent index in extent table. Since we might delete exetnts from extent table 
and this creates holes, so we need to find a free index when adding one extent.
*/
int find_next_extent_index(struct a1fs_extent *extents_list)
{
	// find an extent hole
	for (unsigned int i = 0; i < 512; i++)
	{
		if (extents_list[i].count == 0)
		{
			return i;
		}
	}
	return -1;
}

int update_extent(struct a1fs_inode *inode, unsigned int extent_index, fs_ctx *fs)
{
	struct a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + inode->i_extents * A1FS_BLOCK_SIZE);

	extents_list[extent_index].count = extents_list[extent_index].count - 1;

	return 0;
}

/*
* Deallocate all extends of inode starting from start up to end.
*/
void deallocate_all_extents(struct a1fs_inode *inode, unsigned int start, unsigned int end, unsigned int *deleted_blocks_count, fs_ctx *fs)
{

	struct a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + ((inode->i_extents) * A1FS_BLOCK_SIZE));
	unsigned int extent_deleted = 0;
	for (unsigned int i = start; i < end; i++)
	{
		if (extents_list[i].count != 0)
		{
			set_bitmap(fs->data_bitmap_ptr, extents_list[i].start, extents_list[i].count, fs, 1);
			extent_deleted += extents_list[i].count;
			memset(&extents_list[i], 0, sizeof(struct a1fs_extent));
			if (inode->i_next_extent > i)
			{
				inode->i_next_extent = i;
			}
		}
		*deleted_blocks_count = extent_deleted;
	}
	inode->size -= *deleted_blocks_count * A1FS_BLOCK_SIZE;
}
/*
 * Shifts all the dentries in extent in index extent_index, starting at dentry in index dentry_index.
 * This is to avoid holes in extent.
 */
void shift_dentries(unsigned int extent_index, unsigned int dentry_index, struct a1fs_inode *inode, fs_ctx *fs)
{
	struct a1fs_extent *extent_list = (struct a1fs_extent *)(fs->image + inode->i_extents * A1FS_BLOCK_SIZE);
	unsigned int size = extent_list[extent_index].count;
	struct a1fs_dentry *dentry_list = (struct a1fs_dentry *)(fs->image + (extent_list[extent_index].start - 1) * A1FS_BLOCK_SIZE);
	unsigned int i = 0;
	unsigned int max_dentries = A1FS_BLOCK_SIZE * size / sizeof(a1fs_dentry);
	for (i = (int)dentry_index; i < max_dentries; i++)
	{
		if (dentry_list[i + 1].ino >= 1)
		{
			dentry_list[i] = dentry_list[i + 1];
			memset(&dentry_list[i + 1], 0, sizeof(struct a1fs_dentry));
		}
		else
		{
			break;
		}
	}

	// Note since we always do this shifting when removing a dentry:
	// if dentry_index is the first index of last blk of the extent in extent_index, then removing
	// that dentry will cause its blk in this extent to be de-allocated which will be the last blk
	if ((i == dentry_index ) & (dentry_index == 0))
	{
		// Update the extent to be 1 block less, and free the block in the data bitmap.
		struct a1fs_extent empty_extent = extent_list[extent_index];
		int start = empty_extent.start;
		int length = empty_extent.count;

		int block_to_free = start + length - 1;
		set_bitmap(fs->data_bitmap_ptr, block_to_free, 1, fs, 1);
		// update this change in inode
		if (length == 1)
		{
			inode->i_extent_count -= 1;
			unsigned int deleted_blocks_count = 0;
			deallocate_all_extents(inode, extent_index, extent_index + 1, &deleted_blocks_count, fs);
		}
		else if (length > 1)
		{
			// update_extent(extent_index, fs);
			update_extent(inode, extent_index, fs);
		}
		// change time of inode
		clock_gettime(CLOCK_REALTIME, &(inode->mtime));
	}
}

/*
* removes dentry with name cur_token in inode dir inode.
*/
int remove_dentry(char *cur_token, struct a1fs_inode *inode, fs_ctx *fs)
{

	unsigned int extent_count = inode->i_extent_count;
	struct a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + ((inode->i_extents) * A1FS_BLOCK_SIZE));
	for (unsigned int i = 0; i < extent_count; i++)
	{
		struct a1fs_extent e = extents_list[i];
		a1fs_blk_t start = e.start;
		a1fs_blk_t count = e.count;
		// find this extent in image
		struct a1fs_dentry *dentry_list = (struct a1fs_dentry *)(fs->image + ((start - 1) * A1FS_BLOCK_SIZE));
		int size = count * A1FS_BLOCK_SIZE / sizeof(struct a1fs_dentry);

		for (int t = 0; t < size; t++)
		{
			struct a1fs_dentry dentry = dentry_list[t];
			if (strcmp(dentry.name, cur_token) == 0)
			{
				memset(&dentry_list[t], 0, sizeof(struct a1fs_dentry));
				shift_dentries(i, t, inode, fs);
				return dentry.ino;
			}
		}

		return -1;
	}

	// update parent inode's links
	inode->links = inode->links - 1;
	clock_gettime(CLOCK_REALTIME, &(inode->mtime));

	return -1;
}

/* Assign the goal_inode to point to the inode struct of the element at the end of the path
 * if it exists.  Returns:
 * 0 on success,
 * -ENAMETOOLONG  the path or one of its components is too long.
 * -ENOENT        a component of the path does not exist.
 * -ENOTDIR       a component of the path prefix is not a directory.
 */
int lookup_path(char *path, struct a1fs_inode *goal_inode, fs_ctx *fs, a1fs_ino_t *ino_num)
{
	char directories[A1FS_PATH_MAX];
	strncpy(directories, path, A1FS_PATH_MAX);

	// search through directories in the path
	char *cur_token = strtok(directories, "/");
	a1fs_ino_t inode_num;
	*ino_num = a1fs_ROOT_INO;
	// find the inode table
	struct a1fs_inode *inode_table = (struct a1fs_inode *)(fs->image + fs->inode_table_ptr * A1FS_BLOCK_SIZE);
	struct a1fs_inode *inode = inode_table + a1fs_ROOT_INO;
	int res = 0;

	while (cur_token != NULL)
	{
		if ((res = lookup_dentry(cur_token, &inode_num, inode, fs)) != 0)
		{
			return res;
		}
		// update the cur_token
		cur_token = strtok(NULL, "/");
		// find the next inode number in this path
		inode = inode_table + inode_num;
		*ino_num = inode_num;
	}

	// now inode and inode_num are the pointer to the last token's inode
	*goal_inode = *inode;

	return 0;
}

/*
* Checks to see if fs has enough space for inode to add extents to.
*/
bool fs_has_enough_space(struct a1fs_inode *inode, fs_ctx *fs)
{
	// size of meta data is (super block + inodes bitmap + inodes table)
	unsigned int extent_count = inode->i_extent_count;
	if (extent_count >= 512 || fs->free_inodes_count <= 0)
	{ // is that enough for validity?!
		return false;
	}
	return true;
}

int find_free_dentry(unsigned int start_block, unsigned int blocks_len, fs_ctx *fs)
{
	// Get the directory entries
	struct a1fs_dentry *directory_entries = (struct a1fs_dentry *)(fs->image + ((start_block - 1) * A1FS_BLOCK_SIZE));
	// Go through all the directory entries in this extent
	int size = blocks_len * A1FS_BLOCK_SIZE / sizeof(struct a1fs_dentry);
	for (int t = 0; t < size; t++)
	{
		struct a1fs_dentry entry = directory_entries[t];
		if (!(entry.ino >= 1))
		{
			return t;
		}
	}
	return -1;
}

/*
* Allocates given dentry in the last extent of the parent inode.
*/
int write_dentry_to_extent(int last_extent_blk, struct a1fs_dentry new_dentry, fs_ctx *fs)
{
	// // write the new_dentry in the new place where the new extent refers to
	int next = find_free_dentry(last_extent_blk, 1, fs);

	struct a1fs_dentry *entry_lst = (struct a1fs_dentry *)(fs->image + ((last_extent_blk - 1) * A1FS_BLOCK_SIZE));

	entry_lst[next] = new_dentry;

	return 0;
}

/*
* finds the last extent of the inode.
*/
unsigned int find_last_extent(struct a1fs_inode *inode, fs_ctx *fs)
{
	a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + (inode->i_extents * A1FS_BLOCK_SIZE));
	unsigned int extent_count = inode->i_extent_count;
	return extent_count ? extents_list[extent_count - 1].start : 0;
}

/* Create a new inode, update meta data and assign the inode to point to the new inode struct in the inode table.
 * if it exists.  Returns:
 * new inode index on success,
 * -1 if there is no enough space in inode table. 
 */
int create_new_inode(fs_ctx *fs, struct a1fs_inode *inode)
{
	// create a new inode in the inode table
	struct a1fs_inode *inode_table = (struct a1fs_inode *)(fs->image + fs->inode_table_ptr * A1FS_BLOCK_SIZE);
	if (fs->free_inodes_count <= 0)
	{
		return -1;
	}
	int new_inode_index = (fs->inodes_count) - (fs->free_inodes_count);
	inode_table = inode_table + new_inode_index;
	*inode_table = *inode;

	// update inode bitmap
	set_bitmap(fs->inode_bitmap, new_inode_index + 1, 1, fs, 0);

	// update fs_ctx
	fs->free_inodes_count -= 1;

	// update super block
	update_superblock(fs);

	// note that in the caller function, you have to do this:

	// inode_table_ptr[new_inode_index].mode = S_IFDIR | 0777; // should be different for each inode!
	// inode_table_ptr[new_inode_index].size = 0;
	// inode_table_ptr[new_inode_index].links = 0;
	// inode_table_ptr[new_inode_index].i_extent_count = 0;
	// inode_table_ptr[new_inode_index].i_block_count = 0;
	// struct timespec time_struct;
	// clock_gettime(CLOCK_REALTIME, &time_struct);
	// inode_table_ptr[new_inode_index].mtime = time_struct;

	return new_inode_index;
}

/*
 * frees the data bitmap
 */
void free_data_bitmap(unsigned int start, unsigned int len, fs_ctx *fs)
{
	unsigned int *data_bitmap = (unsigned int *)(fs->image + (fs->data_bitmap_ptr + start * A1FS_BLOCK_SIZE));
	unsigned int count = 0;
	while (count < len)
	{
		unsigned char *byte = (unsigned char *)data_bitmap;
		*byte = *byte & 127;
		data_bitmap = data_bitmap + 1;
		count += 1;
	}
}

/*
 * Looks in the data block table and finds an extent of size length.
 * Returns the block number of the starting block of the largest possible extent, that is equal or smaller than the length
 */
unsigned int
find_new_extent(unsigned int length, fs_ctx *fs, unsigned int *found_len)
{
	unsigned char *bm = (unsigned char *)(fs->image + fs->data_bitmap_ptr * A1FS_BLOCK_SIZE);

	unsigned int consecutive = 0;
	// unsigned int block_candidate = 0;
	unsigned int biggest_so_far_start_blk = 0;
	unsigned int biggest_so_far_len = 0;
	unsigned int blks = 0;

	for (unsigned int i = 0; i < fs->blocks_count / 8; i++)
	{
		unsigned char byte = bm[i]; //returns 0xff and stuff like that;

		// start from the begining of the byte which is its LSB
		for (int t = 0; t < 8; t++)
		{
			int bit_value = (byte & (1 << t));
			blks++;
			if (bit_value == 0)
			{
				consecutive++;
				if (consecutive == length)
				{
					*found_len = length;
					set_bitmap(fs->data_bitmap_ptr, blks - length + 1, length, fs, 0);
					return blks - length + 1;
				}
			}
			else
			{
				if (biggest_so_far_len < consecutive)
				{
					biggest_so_far_len = consecutive;
					biggest_so_far_start_blk = blks - length + 1;
				}
				consecutive = 0;
			}
		}
	}
	set_bitmap(fs->data_bitmap_ptr, biggest_so_far_start_blk, biggest_so_far_len, fs, 0);
	*found_len = biggest_so_far_len;
	return biggest_so_far_start_blk;
}

/*
 * Clears the extent.
 */
void clear_extent(struct a1fs_extent *extent, fs_ctx *fs)
{
	// Clear the data blocks
	unsigned int start = extent->start;
	for (int i = 0; i < (int)extent->count; i++)
	{
		unsigned char *block = (unsigned char *)(fs->image + A1FS_BLOCK_SIZE * (start + i));
		memset(block, 0, A1FS_BLOCK_SIZE);
	}
}

/*
* Adds the new extent new_extent to the inode inode.
*/
int add_new_extent(struct a1fs_inode *inode, fs_ctx *fs, unsigned int extent_len, unsigned int *index)
{
	unsigned int found_len = 0;
	unsigned int extent_count = inode->i_extent_count;
	unsigned int next_extent = inode->i_next_extent;

	if (extent_count >= 512)
	{
		// an inode can have at most 512 extent
		return -ENOSPC;
	}

	if (extent_count == 0)
	{
		// There isn't an extent table, so we want to create one
		// find a single block for the inode to be its extents table
		unsigned int ex_table_start_block = find_new_extent(1, fs, &found_len);
		if (!found_len)
		{
			return -ENOSPC;
		}
		inode->i_extents = ex_table_start_block - 1;
		fs->free_blocks_count--;
	}

	struct a1fs_extent *extents_list = (struct a1fs_extent *)(fs->image + (inode->i_extents * A1FS_BLOCK_SIZE));

	unsigned int added_blks = 0;

	// look into the data bitmap and find one extent of len extent_len or as many that sums up to the length needed
	found_len = 0;
	unsigned int start_blk = find_new_extent(extent_len, fs, &found_len);
	if (!found_len)
	{
		return -ENOSPC;
	}

	if (index != NULL)
	{
		*index = start_blk;
	}

	unsigned int remain_len = extent_len - found_len;
	struct a1fs_extent new_extent;
	// assign this new extent to extents_list
	new_extent.count = found_len;
	new_extent.start = start_blk;
	extents_list[next_extent] = new_extent;
	next_extent = find_next_extent_index(extents_list);
	extent_count += 1;
	added_blks = added_blks + found_len;
	// Now clear the extent so we don't have junk data in it
	// clear_extent(&new_extent, fs);

	// repeat to find a new extent to have number of blocks equal to requested extent_len
	while (remain_len)
	{
		start_blk = find_new_extent(remain_len, fs, &found_len);
		if (!found_len)
		{
			return -ENOSPC;
		}
		struct a1fs_extent new_extent;
		// assign this new extent to extents_list
		new_extent.count = found_len;
		new_extent.start = start_blk;
		extents_list[next_extent] = new_extent;
		next_extent = find_next_extent_index(extents_list);

		extent_count += 1;
		remain_len = remain_len - found_len;
		added_blks = added_blks + found_len;

		// Clear the extent to ensure that there isn't junk data in it
		// clear_extent(&new_extent, fs);
	}

	inode->i_extent_count = extent_count;
	inode->i_next_extent = next_extent;
	inode->links = inode->links + 1;
	inode->size = inode->size + added_blks * A1FS_BLOCK_SIZE;
	clock_gettime(CLOCK_REALTIME, &(inode->mtime));
	inode->i_block_count = (inode->i_block_count) + added_blks;

	// update fs_ctx *fs
	// fs->blocks_count = fs->blocks_count + added_blks;
	fs->free_blocks_count = fs->free_blocks_count - added_blks;

	return 0;
}

/* ****************************** */
/* ****** End of Helpers ******** */
/* ****************************** */

/**
 * Initialize the file system.
 *
 * Called when the file system is mounted. NOTE: we are not using the FUSE
 * init() callback since it doesn't support returning errors. This function must
 * be called explicitly before fuse_main().
 *
 * @param fs    file system context to initialize.
 * @param opts  command line options.
 * @return      true on success; false on failure.
 */
static bool a1fs_init(fs_ctx *fs, a1fs_opts *opts)
{
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

	return fs_ctx_init(fs, image, size);
}

/**
 * Cleanup the file system.
 *
 * Called when the file system is unmounted. Must cleanup all the resources
 * created in a1fs_init().
 */
static void a1fs_destroy(void *ctx)
{
	fs_ctx *fs = (fs_ctx *)ctx;
	/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */
}

/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */

static int a1fs_getattr(const char *path, struct stat *st)
{
	if (strlen(path) >= A1FS_PATH_MAX)
		return -ENAMETOOLONG;
	fs_ctx *fs = get_fs();

	memset(st, 0, sizeof(*st));

	// lookup the inode for given path and, if it exists, fill in the
	// required fields based on the information stored in the inode
	a1fs_ino_t inode_num;
	char path_cpy[A1FS_PATH_MAX];
	strncpy(path_cpy, path, A1FS_PATH_MAX);
	struct a1fs_inode inode = {0};
	int result = lookup_path(path_cpy, &inode, fs, &inode_num);
	if (result != 0)
	{
		return result;
	}

	st->st_mode = inode.mode;
	st->st_nlink = inode.links;
	st->st_size = inode.size;
	st->st_blocks = inode.i_block_count * A1FS_BLOCK_SIZE / 512;
	st->st_mtim = inode.mtime;

	return 0;
}

/* As this is a public repo, I am required to remove some parts. The complete implementation can be viewed on my private repo. */