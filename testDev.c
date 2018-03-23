#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#define DEVICE_NAME "testDev"
#define CLASS_NAME "test"
#define BUFF_LEN 256

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christofer Padilla, Richard Tsai, Matthew Winchester");
MODULE_DESCRIPTION("COP4600 - Programming Assignment 2");
MODULE_VERSION("1.0");

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
  .open = dev_open,
  .read = dev_read,
  .write = dev_write,
  .release = dev_release
};

static int majorNum;
static int numberOpens = 0;
static char * message;
static short messageSize;
static struct class* charClass = NULL;
static struct device* charDevice = NULL;
static char * temp;

static int __init char_init(void)
{
  message = (char *) vmalloc(sizeof(char) * BUFF_LEN);

  printk(KERN_INFO "testDev: Initializing the testDev LKM\n");

  majorNum = register_chrdev(0, DEVICE_NAME, &fops);

  if(majorNum < 0)
  {
    printk(KERN_ALERT "testDev failed to register a majorNum number\n");
    return majorNum;
  }

  printk(KERN_INFO "testDev: registered correctly with majorNum number %d\n", majorNum);

  charClass = class_create(THIS_MODULE, CLASS_NAME);
  if(IS_ERR(charClass))
  {
    unregister_chrdev(majorNum, DEVICE_NAME);
    printk(KERN_ALERT "Failed to register device class.\n");
    return PTR_ERR(charClass);
  }

  printk(KERN_INFO "testDev: device class registered correctly\n");

  charDevice = device_create(charClass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
  if(IS_ERR(charDevice))
  {
    class_destroy(charClass);
    unregister_chrdev(majorNum, DEVICE_NAME);
    printk(KERN_ALERT "Failed to create the device.\n");
    return PTR_ERR(charDevice);
  }

  printk(KERN_INFO "testDev: device class created correctly\n");

  return 0;
}

static void __exit char_exit(void)
{
  device_destroy(charClass, MKDEV(majorNum, 0));
  class_unregister(charClass);
  class_destroy(charClass);
  unregister_chrdev(majorNum, DEVICE_NAME);
  printk(KERN_INFO "testDev: Goodbye from the LKM!");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
  //numberOpens++;
  printk(KERN_INFO "testDev: Device has been opened %d time(s)\n", ++numberOpens);
  return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
  printk(KERN_INFO "testDev: Device successfully closed\n");
  return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
  int error_count = 0;

  if(messageSize > 0)
  {

    if (len > BUFF_LEN)
    {
        printk(KERN_INFO "testDev: User requested %zu bytes, more bytes than maximum buffer size: %d. Only %d bytes will be returned.\n", len, BUFF_LEN, messageSize);
    
        error_count = copy_to_user(buffer, message, messageSize);
        temp = (char *) vmalloc(sizeof(char) * BUFF_LEN);
	temp[0] = '\0';
	vfree(message);
	message=temp;
	if(error_count == 0)
        {
	  printk(KERN_INFO "testDev: Sent %d characters to the user\n", messageSize);
	  messageSize = 0;
          printk(KERN_INFO "testDev: after send messagesize: %d\n",messageSize);
          return 0;

        }  
    }

    else
    {     
        error_count = copy_to_user(buffer, message, len); 
	printk(KERN_INFO "testDev: moving strings around...\n");
	temp = (char *) vmalloc(sizeof(char) * BUFF_LEN);
	strncpy(temp, message+len, messageSize-len);
	vfree(message);
	message = temp;
	message[messageSize-len] = '\0';

        if(error_count == 0)
        {
           printk(KERN_INFO "testDev: Sent %d characters to the user\n", messageSize);
	   messageSize = messageSize - len;
           printk(KERN_INFO "testDev: after send messagesize: %d\n",messageSize);
           return 0;

        }

    }

    

      printk(KERN_INFO "testDev: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;
  }

  printk(KERN_INFO "testDev: User tried to read the empty buffer\n");
  copy_to_user(buffer, message, 0);
  return 0;
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
  // Make sure the incoming message will not overflow us (-1 because we need space for \0)
  if (len + messageSize < BUFF_LEN-1)
  {
    strcat(message, buffer);

    printk(KERN_INFO "testDev: current messagesize: %d\n",messageSize);
    printk(KERN_INFO "testDev: Received %zu characters from the user\n", len);

    strcat(message, "\0");
    messageSize = strlen(message);
    printk(KERN_INFO "testDev: after copy messagesize: %d\n",messageSize);
    return len;
  }
  
  // The buffer will be too big, only read in the space we have left
  else
  {
    int space_available = BUFF_LEN - messageSize;
    printk(KERN_INFO "testDev: current messagesize: %d\n",messageSize);
    printk(KERN_INFO "testDev: Buffer would overflow with %zu additional bytes, only attepmting to write %d bytes instead." ,len, space_available);

    strncat(message, buffer, space_available);
    

    printk(KERN_INFO "testDev: Received %d characters from the user\n", space_available);

    strcat(message, "\0");
    messageSize = strlen(message);
        printk(KERN_INFO "testDev: after copy messagesize: %d\n",messageSize);    
    return space_available;
  }
}


module_init(char_init);
module_exit(char_exit);
