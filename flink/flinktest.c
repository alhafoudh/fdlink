#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "flink.h"

int main(int argc, char *argv[])
{
  char *oldname, *newname;

  if (argc != 3)
  {
    fputs("Usage: flink old-filename new-filename\n", stderr);
    exit(1);
  }

  oldname = argv[1];
  newname = argv[2];

  int fd = open(oldname, O_RDONLY, 0666);
  if (fd < 0)
  {
    fprintf(stderr, "open: ");
    perror(oldname);
    exit(1);
  }

  printf ("flink: %s->%s\n", newname, oldname);

  if (flink(fd, newname, 0))
  {
    perror("flink");
    exit(1);
  }

  exit(0);
}
