#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/path.h>
#include <linux/proc_fs.h>
#include <linux/nsproxy.h>
#include <../fs/mount.h>

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "mymounts"

MODULE_LICENSE("GPL");

static int ft_proc_open(struct inode *inode, struct  file *file);
static int ft_proc_show(struct seq_file *m, void *v);

static struct proc_dir_entry *proc_file;

static const struct proc_ops proc_file_ops = {
        .proc_open = ft_proc_open,
        .proc_read  = seq_read,
        .proc_lseek = seq_lseek,
        .proc_release = single_release,
};

static void show_mount(struct seq_file *m, struct mount *mnt)
{
        if (strcmp(mnt->mnt_devname, "rootfs")) {
                struct path mnt_path = {
                        .dentry = mnt->mnt.mnt_root,
                        .mnt = &mnt->mnt
                };
                seq_printf(m, "%-15s", mnt->mnt_devname);
                seq_path(m, &mnt_path, " \t\n\\");
                seq_putc(m, '\n');
        }
}

static void iter_mount_tree(struct seq_file *m, struct rb_node *node)
{
        if (!node)
                return;
        struct mount *mnt = rb_entry(node, struct mount, mnt_node);
        show_mount(m, mnt);
        iter_mount_tree(m, node->rb_left);
        iter_mount_tree(m, node->rb_right);
}

static int ft_proc_show(struct seq_file *m, void *v)
{
        struct rb_root *root = &current->nsproxy->mnt_ns->mounts;
        iter_mount_tree(m, root->rb_node);
        return 0;
}

static int ft_proc_open(struct inode *inode, struct  file *file)
{
        return single_open(file, ft_proc_show, NULL);
}

static int __init mymounts_init(void)
{
	proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_ops);
	if (proc_file == NULL) {
		proc_remove(proc_file);
		pr_err("could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}
	return 0;
}

static void __exit mymounts_exit(void)
{
        proc_remove(proc_file);
}

module_init(mymounts_init);
module_exit(mymounts_exit);
