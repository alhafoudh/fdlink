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

static long flink_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct flink __user *p = (struct flink __user *)arg;
	struct dentry *old_dentry;
	struct file *f;
	struct nameidata nd;
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

	error = path_lookup(to, LOOKUP_PARENT, &nd);
	if (error)
		goto release_f;

	error = -EXDEV;
	if (f->f_vfsmnt != nd.path.mnt)
		goto release_nd;

	new_dentry = lookup_create(&nd, 0);
	error = PTR_ERR(new_dentry);
	if (!IS_ERR(new_dentry)) {
		error = vfs_link(old_dentry, nd.path.dentry->d_inode, new_dentry);
		dput(new_dentry);
	}
	mutex_unlock(&nd.path.dentry->d_inode->i_mutex);

release_nd:
	path_put(&nd.path);
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
