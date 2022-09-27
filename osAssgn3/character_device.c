#include <linux/miscdevice.h>i
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/init.h>
#define FIFO_SIZE 128

struct kfifo_rec_ptr_1 test[100];
spinlock_t mylock[100];
pid_t tgid[100];
int flag;
static int myopen(struct inode *inode, struct file *file)
{
	printk("myopen called in module\n");
	
	int pid = get_current()->tgid;
	int ret=kfifo_alloc(&(test[flag]),FIFO_SIZE,GFP_KERNEL);
	if(ret){
		printk(KERN_ERR "error kfifo allocation\n");
		return ret;
	}
	tgid[flag]=get_current()->tgid;
	flag++;
	return 0;
}

static int myclose(struct inode *inodep, struct file *filp)
{
	printk("myclose called in module\n");
	int i;
	for(i=0;i<flag;i++){
		if(tgid[i]==get_current()->tgid){
			kfifo_free(&(test[i]));
			break;
		}
	}
	flag--;
	return 0;
}

static ssize_t mywrite(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	char *module_buf;
	printk("mywrite function called in module\n");
	pid_t id=get_current()->tgid;
	module_buf = (char *)kmalloc(len, GFP_USER);
	if(!module_buf) 
		return -1; //kmalloc failed

	copy_from_user(module_buf, buf, len);

	int i,ret;
	for(i=0;i<flag;i++){
		if(tgid[i]!=id){
			ret = kfifo_in_spinlocked(&(test[i]),module_buf,len,&(mylock[i]));
		}
	}
	kfree(module_buf); // clean up

	return ret; 

}

static ssize_t myread(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	
	char *module_buf;
	
	module_buf = (char *)kmalloc(len,GFP_USER);
	if(!module_buf)
		return -1;
	
	pid_t id=get_current()->tgid;

	int i,length, ret;
	for(i=0;i<flag;i++){

		if(tgid[i]==id){
			ret=kfifo_out_spinlocked(&(test[i]),module_buf,len,&(mylock[i]));
			if(ret>0){
				length = copy_to_user(buf,module_buf,len);
			} else {
				module_buf[0]='\0';
				length = copy_to_user(buf,module_buf,len);
			}	
		}
	}
	kfree(module_buf);
	return length;
}

static const struct file_operations myfops = {
    .owner	= THIS_MODULE,
    .read	= myread,
    .write	= mywrite,
    .open	= myopen,
    .release	= myclose,
    .llseek 	= no_llseek,
};

struct miscdevice mydevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "chatroom",
    .fops = &myfops,
    .mode = S_IRUGO | S_IWUGO,
};

static int __init my_init(void)
{
	printk("my_init called\n");

	// register the character device
	if (misc_register(&mydevice) != 0) {
		printk("device registration failed\n");
		return -1;
	}

	printk("character device registered\n");
	return 0;
}

static void __exit my_exit(void)
{
	printk("my_exit called\n");
	misc_deregister(&mydevice);
}

module_init(my_init)
module_exit(my_exit)
MODULE_DESCRIPTION("Miscellaneous character device module\n");
MODULE_AUTHOR("Kartik Gopalan");
MODULE_LICENSE("GPL");