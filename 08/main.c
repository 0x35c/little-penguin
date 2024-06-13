#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Useless module");

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size,
                         loff_t *offs);
static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
                          loff_t *offs);

static struct file_operations myfd_fops = {
    .owner = THIS_MODULE,
    .read = &myfd_read,
    .write = &myfd_write,
};

static struct miscdevice myfd_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "reverse",
    .fops = &myfd_fops,
};

char str[PAGE_SIZE];

static int __init myfd_init(void)
{
	misc_register(&myfd_device);
        str[0] = 0x0;
	return 0;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
}

ssize_t myfd_read(struct file *fp, char __user *buf, size_t size, loff_t *offs)
{
	size_t len;
        char *tmp;

        tmp = kmalloc((PAGE_SIZE + 1) * sizeof(char), GFP_KERNEL);
        if (!tmp)
                return -ENOMEM;
        len = strlen(str);
	for (size_t i = 0; i < len; i++)
		tmp[i] = str[len - 1 - i];
	int rv = simple_read_from_buffer(buf, size, offs, tmp, len);
        kfree(tmp);
        return rv;
}

ssize_t myfd_write(struct file *fp, const char __user *buf, size_t size,
                   loff_t *offs)
{
	ssize_t res;

	res = simple_write_to_buffer(str, sizeof(str), offs, buf, PAGE_SIZE);
	str[size] = 0x0;
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);
