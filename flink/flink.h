struct flink
{
  unsigned int fd;
  const char *path;
  unsigned int flags;
};

extern int flink(unsigned int fd, const char *path, unsigned int flags);
