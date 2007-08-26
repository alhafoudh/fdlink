#include <fcntl.h>
#include <errno.h>

#include <stdio.h> // DEBUG
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "flink.h"

int flink(unsigned int fd, const char *path, unsigned int flags)
{
  static int flinkfd = -1;
  
  if (flinkfd < 0)
    flinkfd = open("/dev/flink", O_RDONLY, 0444);

  if (flinkfd < 0)
  {
    errno = EACCES;
    return -1;
  }

  printf("opened /dev/flink\n");
  struct flink flink;

  flink.fd = fd;
  flink.path = path;
  flink.flags = 0;

  return ioctl(flinkfd, 0, &flink);
}
