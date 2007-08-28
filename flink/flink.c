/*
 * $HeadURL$
 * $Id$
 */
#include <fcntl.h>
#include <errno.h>

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
    errno = ENODEV;
    return -1;
  }

  struct flink data;

  data.fd = fd;
  data.path = path;
  data.flags = 0;

  return ioctl(flinkfd, 0, &data);
}
