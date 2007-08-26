/*
 * $HeadURL$
 *
 * $Id$
 *
 * flink kernel module
 */

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/namei.h>

#include <asm/uaccess.h>

#include "flink.h"

static int flink_ioctl(struct inode *inode, struct file *file,
                       unsigned int cmd, unsigned long arg)
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
	if (f->f_vfsmnt != nd.mnt)
		goto release_nd;

	new_dentry = lookup_create(&nd, 0);
	error = PTR_ERR(new_dentry);
	if (!IS_ERR(new_dentry)) {
		error = vfs_link(old_dentry, nd.dentry->d_inode, new_dentry);
		dput(new_dentry);
	}
	mutex_unlock(&nd.dentry->d_inode->i_mutex);

release_nd:
	path_release(&nd);
release_f:
	fput(f);
exit:
	putname(to);
	return error;
}

/*
 * The only file operation we care about is ioctl.
 */

static const struct file_operations flink_fops = {
	.owner		= THIS_MODULE,
	.ioctl		= flink_ioctl,
};

static struct miscdevice flink_dev = {
	/*
	 * We don't care what minor number we end up with, so tell the
	 * kernel to just pick one.
	 */
	MISC_DYNAMIC_MINOR,
	/*
	 * Name ourselves /dev/flink.
	 */
	"flink",
	/*
	 * What functions to call when a program performs file
	 * operations on the device.
	 */
	&flink_fops
};

static int __init
flink_init(void)
{
	int ret;

	/*
	 * Create the "flink" device in the /sys/class/misc directory.
	 * Udev will automatically create the /dev/flink device using
	 * the default rules.
	 */
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
