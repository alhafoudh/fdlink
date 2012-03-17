/*
 * $HeadURL$
 *
 * $Id$
 *
 * flink_dev kernel module
 */

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/proc_fs.h>

#include <asm/uaccess.h>

#include "flink.h"

#include "linux/fsnotify.h"

//copied from kernel namei.c
static inline int may_create(struct inode *dir, struct dentry *child)
{
	if (child->d_inode)
		return -EEXIST;
	if (IS_DEADDIR(dir))
		return -ENOENT;
	return inode_permission(dir, MAY_WRITE | MAY_EXEC);
}

//copied from kernel namei.c revision 83e92ba, removed security call
static int flink_vfs_link_83e92ba(struct dentry *old_dentry, struct inode *dir, struct dentry *new_dentry)
{
	struct inode *inode = old_dentry->d_inode;
	int error;

	if (!inode)
		return -ENOENT;

	error = may_create(dir, new_dentry);
	if (error)
		return error;

	if (dir->i_sb != inode->i_sb)
		return -EXDEV;

	/*
	 * A link to an append-only or immutable file cannot be created.
	 */
	if (IS_APPEND(inode) || IS_IMMUTABLE(inode))
		return -EPERM;
	if (!dir->i_op->link)
		return -EPERM;
	if (S_ISDIR(inode->i_mode))
		return -EPERM;

	//-------------------Removed security_inode_link call because it is not exported 
	//error = security_inode_link(old_dentry, dir, new_dentry);
	//if (error)
	//	return error;

	mutex_lock(&inode->i_mutex);
	error = dir->i_op->link(old_dentry, dir, new_dentry);
	mutex_unlock(&inode->i_mutex);
	if (!error)
		fsnotify_link(dir, inode, new_dentry);
	return error;
}


static long flink_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct flink __user *p = (struct flink __user *)arg;
	struct dentry *old_dentry;
	struct file *f;
	struct path new_path;
	struct dentry *new_dentry;

	char *to = getname(p->path);
	int error = PTR_ERR(to);

	if (IS_ERR(to))
		return error;

	f = fget(p->fd);
	if (!f) {
		error = -EBADF;
		goto exit;
	}

	old_dentry = f->f_dentry;
	error = 0;

	error = kern_path(to, LOOKUP_PARENT, &new_path);
	if (error) {
		goto release_f;
	}

	error = -EXDEV;
	if (f->f_vfsmnt != new_path.mnt)
		goto release_nd;

        struct path unused;
	new_dentry = kern_path_create(AT_FDCWD, to, &unused, 0);
	error = PTR_ERR(new_dentry);
	if (!IS_ERR(new_dentry)) {
		error = flink_vfs_link_83e92ba(old_dentry, new_path.dentry->d_inode, new_dentry);
		dput(new_dentry);
	}
	mutex_unlock(&new_path.dentry->d_inode->i_mutex);

release_nd:
	path_put(&new_path);
release_f:
	fput(f);
exit:
	putname(to);
	return error;
}

static const struct file_operations flink_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl		= flink_ioctl,
};

static struct miscdevice flink_dev = {
	MISC_DYNAMIC_MINOR,
	"flink",
	&flink_fops
};

static int __init
flink_init(void)
{
	int ret;
	ret = misc_register(&flink_dev);
	if (ret)
		printk(KERN_ERR
		       "Unable to register \"flink\" misc device\n");

	return ret;
}

module_init(flink_init);

static void __exit
flink_exit(void)
{
	misc_deregister(&flink_dev);
}

module_exit(flink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Amos Shapira <amos.shapira@gmail.com>");
MODULE_DESCRIPTION("flink module");
MODULE_VERSION("dev");

