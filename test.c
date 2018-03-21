#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define BUFFER_LENGTH 1024

static char receive[BUFFER_LENGTH];

int main()
{
  int ret, fd, exit;
  char stringToSend[BUFFER_LENGTH];
  printf("Staring device test code example...\n");

  fd = open("/dev/testDev", O_RDWR);

  if(fd < 0)
  {
    perror("failed to open the device...");
    return errno;
  }

  do
  {
    printf("Type in a string to send to the kernel module:\n");
    fgets(stringToSend, BUFFER_LENGTH, stdin);

    if(stringToSend[strlen(stringToSend) - 1] == '\n')
      stringToSend[strlen(stringToSend) - 1] = '\0';
    if(strcmp(stringToSend, "") != 0)
    {
      printf("writing message to the device [%s].\n", stringToSend);
      ret = write(fd, stringToSend, strlen(stringToSend));
    }
    else break;

  } while(strcmp(stringToSend, "") != 0);

  if(ret < 0)
  {
    perror("Failed to write the message to the device.");
    return errno;
  }

  printf("press Enter to read from the device, or e to exit...\n");
  exit = getchar();

  if(exit != 'e')
  {
    printf("Reading from the device...\n");
    ret = read(fd, receive, BUFFER_LENGTH);

    if(ret < 0)
    {
      perror("Failed to write the message to the device.");
      return errno;
    }
    printf("The received message is [%s]\n", receive);
  }

  printf("End of the program\n");
  return 0;
}
