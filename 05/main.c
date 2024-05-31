#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>

#define LOGIN "ulayus"

MODULE_LICENSE("GPL");

static ssize_t misc_write(struct file *fileptr, const char __user *buf, size_t len,
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

static ssize_t misc_read(struct file *fileptr, char __user *buf, size_t len,
                  loff_t *off)
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

struct file_operations fops = {
    .write = &misc_write,
    .read = &misc_read,
};

struct miscdevice misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "fortytwo",
    .fops = &fops,
};

static int __init module_start(void)
{
        int err = misc_register(&misc_device);
        if (err) {
                pr_err("misc_register failed\n");
                return err;
        }
	return 0;
}

static void __exit module_end(void)
{
        misc_deregister(&misc_device);
}

module_init(module_start);
module_exit(module_end);
