/*
 * Internal header file for device mapper
 *
 * Copyright (C) 2001, 2002 Sistina Software
 * Copyright (C) 2004-2006 Red Hat, Inc. All rights reserved.
 *
 * This file is released under the LGPL.
 */

#ifndef DM_INTERNAL_H
#define DM_INTERNAL_H

#include <linux/fs.h>
#include <linux/device-mapper.h>
#include <linux/list.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

/*
 * Suspend feature flags
 */
#define DM_SUSPEND_LOCKFS_FLAG		(1 << 0)
#define DM_SUSPEND_NOFLUSH_FLAG		(1 << 1)

/*
 * Type of table and mapped_device's mempool
 */
#define DM_TYPE_NONE		0
#define DM_TYPE_BIO_BASED	1
#define DM_TYPE_REQUEST_BASED	2

//
#define MAX_DEPTH 16
//
//
/*
 * List of devices that a metadevice uses and should open/close.
 */
struct dm_dev_internal {
	struct list_head list;
	atomic_t count;
	struct dm_dev dm_dev;
};

#if 0
struct kobject {

    const char  *name;
    struct list_head entry;
    struct kobject *parent;
    struct kset *kset;
    struct kobj_type *ktype;
    struct sysfs_dirent *sd;
    struct kref kref;
    unsigned int state_initialized:1;
    unsigned int state_in_sysfs:1;
    unsigned int state_add_uevent_sent:1;
    unsigned int state_remove_uevent_sent:1;
    unsigned int uevent_suppress:1;

};
#endif

struct dm_table {
        struct mapped_device *md;
        atomic_t holders;
        unsigned type;

        /* btree table */
        unsigned int depth;
        unsigned int counts[MAX_DEPTH]; /* in nodes */
        sector_t *index[MAX_DEPTH];

        unsigned int num_targets;
        unsigned int num_allocated;
        sector_t *highs;
        struct dm_target *targets;

        /*
 *          * Indicates the rw permissions for the new logical
 *                   * device.  This should be a combination of FMODE_READ
 *                            * and FMODE_WRITE.
 *                                     */
        fmode_t mode;

        /* a list of devices used by this table */
        struct list_head devices;

        /* events get handed up using this callback */
        void (*event_fn)(void *);
        void *event_context;

        struct dm_md_mempools *mempools;
};


#if 0
struct mapped_device
{
	struct rw_semaphore io_lock;
	struct mutex suspend_lock;
	rwlock_t map_lock;
	atomic_t holders;
	atomic_t open_count;

	unsigned long flags;

	struct request_queue *queue;
	struct gendisk *disk;
	char name[16];

	void *interface_ptr;

	/*
	 * A list of ios that arrived while we were suspended.
	 */
	atomic_t pending[2];
	wait_queue_head_t wait;
	struct work_struct work;
	struct bio_list deferred;
	spinlock_t deferred_lock;

	/*
	 * An error from the barrier request currently being processed.
	 */
	int barrier_error;

	/*
	 * Processing queue (flush/barriers)
	 */
	struct workqueue_struct *wq;

	/*
	 * The current mapping.
	 */
	struct dm_table *map;

	/*
	 * io objects are allocated from here.
	 */
	mempool_t *io_pool;
	mempool_t *tio_pool;

	struct bio_set *bs;

	/*
	 * Event handling.
	 */
	atomic_t event_nr;
	wait_queue_head_t eventq;
	atomic_t uevent_seq;
	struct list_head uevent_list;
	spinlock_t uevent_lock; /* Protect access to uevent_list */

	/*
	 * freeze/thaw support require holding onto a super block
	 */
	struct super_block *frozen_sb;
	struct block_device *bdev;

	/* forced geometry settings */
	struct hd_geometry geometry;

	/* marker of flush suspend for request-based dm */
	struct request suspend_rq;

	/* For saving the address of __make_request for request based dm */
	make_request_fn *saved_make_request_fn;

	/* sysfs handle */
	struct kobject kobj;

	/* zero-length barrier that will be cloned and submitted to targets */
	struct bio barrier_bio;
};
#endif

//struct dm_table;
struct dm_md_mempools;

//YOUNGMIN

void print_val(struct dm_target *ti, struct bio *bio, sector_t sec);


/*-----------------------------------------------------------------
 * Internal table functions.
 *---------------------------------------------------------------*/
void dm_table_destroy(struct dm_table *t);
void dm_table_event_callback(struct dm_table *t,
			     void (*fn)(void *), void *context);
struct dm_target *dm_table_get_target(struct dm_table *t, unsigned int index);
struct dm_target *dm_table_find_target(struct dm_table *t, sector_t sector);
int dm_calculate_queue_limits(struct dm_table *table,
			      struct queue_limits *limits);
void dm_table_set_restrictions(struct dm_table *t, struct request_queue *q,
			       struct queue_limits *limits);
struct list_head *dm_table_get_devices(struct dm_table *t);
void dm_table_presuspend_targets(struct dm_table *t);
void dm_table_postsuspend_targets(struct dm_table *t);
int dm_table_resume_targets(struct dm_table *t);
int dm_table_any_congested(struct dm_table *t, int bdi_bits);
int dm_table_any_busy_target(struct dm_table *t);
int dm_table_set_type(struct dm_table *t);
unsigned dm_table_get_type(struct dm_table *t);
bool dm_table_request_based(struct dm_table *t);
int dm_table_alloc_md_mempools(struct dm_table *t);
void dm_table_free_md_mempools(struct dm_table *t);
struct dm_md_mempools *dm_table_get_md_mempools(struct dm_table *t);

/*
 * To check the return value from dm_table_find_target().
 */
#define dm_target_is_valid(t) ((t)->table)

/*
 * To check whether the target type is request-based or not (bio-based).
 */
#define dm_target_request_based(t) ((t)->type->map_rq != NULL)

/*-----------------------------------------------------------------
 * A registry of target types.
 *---------------------------------------------------------------*/
int dm_target_init(void);
void dm_target_exit(void);
struct target_type *dm_get_target_type(const char *name);
void dm_put_target_type(struct target_type *tt);
int dm_target_iterate(void (*iter_func)(struct target_type *tt,
					void *param), void *param);

int dm_split_args(int *argc, char ***argvp, char *input);

/*
 * The device-mapper can be driven through one of two interfaces;
 * ioctl or filesystem, depending which patch you have applied.
 */
int dm_interface_init(void);
void dm_interface_exit(void);

/*
 * sysfs interface
 */
int dm_sysfs_init(struct mapped_device *md);
void dm_sysfs_exit(struct mapped_device *md);
struct kobject *dm_kobject(struct mapped_device *md);
struct mapped_device *dm_get_from_kobject(struct kobject *kobj);

/*
 * Targets for linear and striped mappings
 */
int dm_linear_init(void);
void dm_linear_exit(void);

int dm_stripe_init(void);
void dm_stripe_exit(void);

int dm_open_count(struct mapped_device *md);
int dm_lock_for_deletion(struct mapped_device *md);

void dm_kobject_uevent(struct mapped_device *md, enum kobject_action action,
		       unsigned cookie);

int dm_kcopyd_init(void);
void dm_kcopyd_exit(void);

/*
 * Mempool operations
 */
struct dm_md_mempools *dm_alloc_md_mempools(unsigned type);
void dm_free_md_mempools(struct dm_md_mempools *pools);

#endif
