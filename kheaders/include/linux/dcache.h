/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_DCACHE_H
#define __LINUX_DCACHE_H

#include <linux/atomic.h>
#include <linux/list.h>
#include <linux/math.h>
#include <linux/rculist.h>
#include <linux/rculist_bl.h>
#include <linux/spinlock.h>
#include <linux/seqlock.h>
#include <linux/cache.h>
#include <linux/rcupdate.h>
#include <linux/lockref.h>
#include <linux/stringhash.h>
#include <linux/wait.h>
#include <linux/android_kabi.h>

struct path;
struct file;
struct vfsmount;



#define IS_ROOT(x) ((x) == (x)->d_parent)


#ifdef __LITTLE_ENDIAN
 #define HASH_LEN_DECLARE u32 hash; u32 len
 #define bytemask_from_count(cnt)	(~(~0ul << (cnt)*8))
#else
 #define HASH_LEN_DECLARE u32 len; u32 hash
 #define bytemask_from_count(cnt)	(~(~0ul >> (cnt)*8))
#endif


struct qstr {
	union {
		struct {
			HASH_LEN_DECLARE;
		};
		u64 hash_len;
	};
	const unsigned char *name;
};

#define QSTR_INIT(n,l) { { { .len = l } }, .name = n }
#define QSTR(n) (struct qstr)QSTR_INIT(n, strlen(n))

extern const struct qstr empty_name;
extern const struct qstr slash_name;
extern const struct qstr dotdot_name;


#ifdef CONFIG_64BIT
# define DNAME_INLINE_LEN 40 
#else
# ifdef CONFIG_SMP
#  define DNAME_INLINE_LEN 36 
# else
#  define DNAME_INLINE_LEN 44 
# endif
#endif

#define d_lock	d_lockref.lock

struct dentry {
	
	unsigned int d_flags;		
	seqcount_spinlock_t d_seq;	
	struct hlist_bl_node d_hash;	
	struct dentry *d_parent;	
	struct qstr d_name;
	struct inode *d_inode;		
	unsigned char d_iname[DNAME_INLINE_LEN];	
	

	
	const struct dentry_operations *d_op;
	struct super_block *d_sb;	
	unsigned long d_time;		
	void *d_fsdata;			
	
	struct lockref d_lockref;	

	union {
		struct list_head d_lru;		
		wait_queue_head_t *d_wait;	
	};
	struct hlist_node d_sib;	
	struct hlist_head d_children;	
	
	union {
		struct hlist_node d_alias;	
		struct hlist_bl_node d_in_lookup_hash;	
	 	struct rcu_head d_rcu;
	} d_u;

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
};


enum dentry_d_lock_class
{
	DENTRY_D_LOCK_NORMAL, 
	DENTRY_D_LOCK_NESTED
};

enum d_real_type {
	D_REAL_DATA,
	D_REAL_METADATA,
};

struct dentry_operations {
	int (*d_revalidate)(struct dentry *, unsigned int);
	int (*d_weak_revalidate)(struct dentry *, unsigned int);
	int (*d_hash)(const struct dentry *, struct qstr *);
	int (*d_compare)(const struct dentry *,
			unsigned int, const char *, const struct qstr *);
	int (*d_delete)(const struct dentry *);
	int (*d_init)(struct dentry *);
	void (*d_release)(struct dentry *);
	void (*d_prune)(struct dentry *);
	void (*d_iput)(struct dentry *, struct inode *);
	char *(*d_dname)(struct dentry *, char *, int);
	struct vfsmount *(*d_automount)(struct path *);
	int (*d_manage)(const struct path *, bool);
	struct dentry *(*d_real)(struct dentry *, enum d_real_type type);
	int (*d_canonical_path)(const struct path *, struct path *);

	ANDROID_KABI_RESERVE(1);
	ANDROID_KABI_RESERVE(2);
	ANDROID_KABI_RESERVE(3);
	ANDROID_KABI_RESERVE(4);
} ____cacheline_aligned;




#define DCACHE_OP_HASH			BIT(0)
#define DCACHE_OP_COMPARE		BIT(1)
#define DCACHE_OP_REVALIDATE		BIT(2)
#define DCACHE_OP_DELETE		BIT(3)
#define DCACHE_OP_PRUNE			BIT(4)

#define	DCACHE_DISCONNECTED		BIT(5)
     

#define DCACHE_REFERENCED		BIT(6) 

#define DCACHE_DONTCACHE		BIT(7) 

#define DCACHE_CANT_MOUNT		BIT(8)
#define DCACHE_GENOCIDE			BIT(9)
#define DCACHE_SHRINK_LIST		BIT(10)

#define DCACHE_OP_WEAK_REVALIDATE	BIT(11)

#define DCACHE_NFSFS_RENAMED		BIT(12)
     
#define DCACHE_FSNOTIFY_PARENT_WATCHED	BIT(14)
     

#define DCACHE_DENTRY_KILLED		BIT(15)

#define DCACHE_MOUNTED			BIT(16) 
#define DCACHE_NEED_AUTOMOUNT		BIT(17) 
#define DCACHE_MANAGE_TRANSIT		BIT(18) 
#define DCACHE_MANAGED_DENTRY \
	(DCACHE_MOUNTED|DCACHE_NEED_AUTOMOUNT|DCACHE_MANAGE_TRANSIT)

#define DCACHE_LRU_LIST			BIT(19)

#define DCACHE_ENTRY_TYPE		(7 << 20) 
#define DCACHE_MISS_TYPE		(0 << 20) 
#define DCACHE_WHITEOUT_TYPE		(1 << 20) 
#define DCACHE_DIRECTORY_TYPE		(2 << 20) 
#define DCACHE_AUTODIR_TYPE		(3 << 20) 
#define DCACHE_REGULAR_TYPE		(4 << 20) 
#define DCACHE_SPECIAL_TYPE		(5 << 20) 
#define DCACHE_SYMLINK_TYPE		(6 << 20) 

#define DCACHE_NOKEY_NAME		BIT(25) 
#define DCACHE_OP_REAL			BIT(26)

#define DCACHE_PAR_LOOKUP		BIT(28) 
#define DCACHE_DENTRY_CURSOR		BIT(29)
#define DCACHE_NORCU			BIT(30) 

extern seqlock_t rename_lock;


extern void d_instantiate(struct dentry *, struct inode *);
extern void d_instantiate_new(struct dentry *, struct inode *);
extern void __d_drop(struct dentry *dentry);
extern void d_drop(struct dentry *dentry);
extern void d_delete(struct dentry *);
extern void d_set_d_op(struct dentry *dentry, const struct dentry_operations *op);


extern struct dentry * d_alloc(struct dentry *, const struct qstr *);
extern struct dentry * d_alloc_anon(struct super_block *);
extern struct dentry * d_alloc_parallel(struct dentry *, const struct qstr *,
					wait_queue_head_t *);
extern struct dentry * d_splice_alias(struct inode *, struct dentry *);
extern struct dentry * d_add_ci(struct dentry *, struct inode *, struct qstr *);
extern bool d_same_name(const struct dentry *dentry, const struct dentry *parent,
			const struct qstr *name);
extern struct dentry *d_find_any_alias(struct inode *inode);
extern struct dentry * d_obtain_alias(struct inode *);
extern struct dentry * d_obtain_root(struct inode *);
extern void shrink_dcache_sb(struct super_block *);
extern void shrink_dcache_parent(struct dentry *);
extern void d_invalidate(struct dentry *);


extern struct dentry * d_make_root(struct inode *);

extern void d_mark_tmpfile(struct file *, struct inode *);
extern void d_tmpfile(struct file *, struct inode *);

extern struct dentry *d_find_alias(struct inode *);
extern void d_prune_aliases(struct inode *);

extern struct dentry *d_find_alias_rcu(struct inode *);


extern int path_has_submounts(const struct path *);


extern void d_rehash(struct dentry *);

extern void d_add(struct dentry *, struct inode *);


extern void d_move(struct dentry *, struct dentry *);
extern void d_exchange(struct dentry *, struct dentry *);
extern struct dentry *d_ancestor(struct dentry *, struct dentry *);

extern struct dentry *d_lookup(const struct dentry *, const struct qstr *);
extern struct dentry *d_hash_and_lookup(struct dentry *, struct qstr *);

static inline unsigned d_count(const struct dentry *dentry)
{
	return dentry->d_lockref.count;
}

ino_t d_parent_ino(struct dentry *dentry);


extern __printf(3, 4)
char *dynamic_dname(char *, int, const char *, ...);

extern char *__d_path(const struct path *, const struct path *, char *, int);
extern char *d_absolute_path(const struct path *, char *, int);
extern char *d_path(const struct path *, char *, int);
extern char *dentry_path_raw(const struct dentry *, char *, int);
extern char *dentry_path(const struct dentry *, char *, int);




static inline struct dentry *dget_dlock(struct dentry *dentry)
{
	dentry->d_lockref.count++;
	return dentry;
}



static inline struct dentry *dget(struct dentry *dentry)
{
	if (dentry)
		lockref_get(&dentry->d_lockref);
	return dentry;
}

extern struct dentry *dget_parent(struct dentry *dentry);


static inline int d_unhashed(const struct dentry *dentry)
{
	return hlist_bl_unhashed(&dentry->d_hash);
}

static inline int d_unlinked(const struct dentry *dentry)
{
	return d_unhashed(dentry) && !IS_ROOT(dentry);
}

static inline int cant_mount(const struct dentry *dentry)
{
	return (dentry->d_flags & DCACHE_CANT_MOUNT);
}

static inline void dont_mount(struct dentry *dentry)
{
	spin_lock(&dentry->d_lock);
	dentry->d_flags |= DCACHE_CANT_MOUNT;
	spin_unlock(&dentry->d_lock);
}

extern void __d_lookup_unhash_wake(struct dentry *dentry);

static inline int d_in_lookup(const struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_PAR_LOOKUP;
}

static inline void d_lookup_done(struct dentry *dentry)
{
	if (unlikely(d_in_lookup(dentry)))
		__d_lookup_unhash_wake(dentry);
}

extern void dput(struct dentry *);

static inline bool d_managed(const struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_MANAGED_DENTRY;
}

static inline bool d_mountpoint(const struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_MOUNTED;
}


static inline unsigned __d_entry_type(const struct dentry *dentry)
{
	return dentry->d_flags & DCACHE_ENTRY_TYPE;
}

static inline bool d_is_miss(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_MISS_TYPE;
}

static inline bool d_is_whiteout(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_WHITEOUT_TYPE;
}

static inline bool d_can_lookup(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_DIRECTORY_TYPE;
}

static inline bool d_is_autodir(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_AUTODIR_TYPE;
}

static inline bool d_is_dir(const struct dentry *dentry)
{
	return d_can_lookup(dentry) || d_is_autodir(dentry);
}

static inline bool d_is_symlink(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_SYMLINK_TYPE;
}

static inline bool d_is_reg(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_REGULAR_TYPE;
}

static inline bool d_is_special(const struct dentry *dentry)
{
	return __d_entry_type(dentry) == DCACHE_SPECIAL_TYPE;
}

static inline bool d_is_file(const struct dentry *dentry)
{
	return d_is_reg(dentry) || d_is_special(dentry);
}

static inline bool d_is_negative(const struct dentry *dentry)
{
	// TODO: check d_is_whiteout(dentry) also.
	return d_is_miss(dentry);
}

static inline bool d_flags_negative(unsigned flags)
{
	return (flags & DCACHE_ENTRY_TYPE) == DCACHE_MISS_TYPE;
}

static inline bool d_is_positive(const struct dentry *dentry)
{
	return !d_is_negative(dentry);
}


static inline bool d_really_is_negative(const struct dentry *dentry)
{
	return dentry->d_inode == NULL;
}


static inline bool d_really_is_positive(const struct dentry *dentry)
{
	return dentry->d_inode != NULL;
}

static inline int simple_positive(const struct dentry *dentry)
{
	return d_really_is_positive(dentry) && !d_unhashed(dentry);
}

extern int sysctl_vfs_cache_pressure;

static inline unsigned long vfs_pressure_ratio(unsigned long val)
{
	return mult_frac(val, sysctl_vfs_cache_pressure, 100);
}


static inline struct inode *d_inode(const struct dentry *dentry)
{
	return dentry->d_inode;
}


static inline struct inode *d_inode_rcu(const struct dentry *dentry)
{
	return READ_ONCE(dentry->d_inode);
}


static inline struct inode *d_backing_inode(const struct dentry *upper)
{
	struct inode *inode = upper->d_inode;

	return inode;
}


static inline struct dentry *d_real(struct dentry *dentry, enum d_real_type type)
{
	if (unlikely(dentry->d_flags & DCACHE_OP_REAL))
		return dentry->d_op->d_real(dentry, type);
	else
		return dentry;
}


static inline struct inode *d_real_inode(const struct dentry *dentry)
{
	
	return d_inode(d_real((struct dentry *) dentry, D_REAL_DATA));
}

struct name_snapshot {
	struct qstr name;
	unsigned char inline_name[DNAME_INLINE_LEN];
};
void take_dentry_name_snapshot(struct name_snapshot *, struct dentry *);
void release_dentry_name_snapshot(struct name_snapshot *);

static inline struct dentry *d_first_child(const struct dentry *dentry)
{
	return hlist_entry_safe(dentry->d_children.first, struct dentry, d_sib);
}

static inline struct dentry *d_next_sibling(const struct dentry *dentry)
{
	return hlist_entry_safe(dentry->d_sib.next, struct dentry, d_sib);
}

#endif	
