#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/mutex.h>

#define LOGIN "ulayus"

MODULE_LICENSE("GPL");

static DEFINE_MUTEX(foo_lock);

static char data[PAGE_SIZE];
static size_t data_len;

static struct dentry *id_file;
static struct dentry *foo_file;
static struct dentry *subdir;

static ssize_t id_read(struct file *fileptr, char *buf, size_t len,
                loff_t *off);
static ssize_t id_write(struct file *fileptr, const char *buf, size_t len,
                loff_t *off);
static ssize_t foo_read(struct file *fileptr, char *buf, size_t len,
                loff_t *off);
static ssize_t foo_write(struct file *fileptr, const char *buf, size_t len,
                loff_t *off);

struct file_operations id_fops = {
        .owner = THIS_MODULE,
        .write = &id_write,
        .read = &id_read,
};

struct file_operations foo_fops = {
        .owner = THIS_MODULE,
        .read = &foo_read,
        .write = &foo_write,
};

static int __init debugfs_init(void)
{
        subdir = debugfs_create_dir("fortytwo", NULL);
        if (!subdir) {
                pr_err("dir creation failed: %p\n", subdir);
                return -ENOMEM;
        }
        pr_info("subdir created: %p", subdir);
        id_file = debugfs_create_file("id", 0666, subdir, NULL, &id_fops);
        if (!id_file)
                goto exit;
        debugfs_create_size_t("jiffies", 0444, subdir, (size_t *)&jiffies);
        foo_file = debugfs_create_file("foo", 0644, subdir, NULL, &foo_fops);
        if (!foo_file)
                goto exit;
	return 0;
exit:
        debugfs_remove_recursive(subdir);
        return -ENOMEM;
}

static void __exit debugfs_exit(void)
{
        debugfs_remove_recursive(subdir);
}

static ssize_t id_write(struct file *fileptr, const char *buf, size_t len,
                loff_t *off)
{
        char kbuf[sizeof(LOGIN)];

        int err = copy_from_user(kbuf, buf, len);
        if (err) {
                pr_err("copy_from_user failed\n");
                return err;
        }
        if (strcmp(kbuf, LOGIN))
                return -EINVAL;
	return len;
}

static ssize_t id_read(struct file *fileptr, char *buf, size_t len, loff_t *off)
{
        const size_t size = strlen(LOGIN);

        if (*off > 0)
                return 0;
        int err = copy_to_user(buf, LOGIN, size);
        if (err) {
                pr_err("copy_to_user failed\n");
                return err;
        }
        *off += size; 
	return size;
}

static ssize_t foo_read(struct file *fileptr, char *buf, size_t len,
                loff_t *off)
{
        if (*off > 0)
                return 0;
        mutex_lock(&foo_lock);
        size_t size = data_len;
        if (size > sizeof(data))
                pr_err("bozo\n");
        pr_info("data: %s, size: %ld\n", data, size);
        int err = copy_to_user(buf, data, size);
        mutex_unlock(&foo_lock);
        if (err) {
                pr_err("copy_to_user failed\n");
                return err;
        }
        *off += size; 
	return size;
}

static ssize_t foo_write(struct file *fileptr, const char *buf, size_t len,
                loff_t *off)
{
        if (len > PAGE_SIZE)
                return -EINVAL;
        mutex_lock(&foo_lock);
        int rv = copy_from_user(data, buf, len);
        if (rv) {
                mutex_unlock(&foo_lock);
                pr_err("copy_from_user failed\n");
                return rv;
        }
        data_len = len;
        mutex_unlock(&foo_lock);
	return len;
}

module_init(debugfs_init);
module_exit(debugfs_exit);
