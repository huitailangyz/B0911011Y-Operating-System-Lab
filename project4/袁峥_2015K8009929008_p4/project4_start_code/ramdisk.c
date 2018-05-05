#include "ramdisk.h"
#include "util.h"

#include "files.h"

Process ramdisk_find(const char *filename)
{
  int i;
  for(i=0; i<get_num_files(); ++i)
  {
    File *file = get_nth_file(i);
    if( same_string( (char *)file->filename, (char *)filename ) )
      return file->process;
  }
  return 0;
}

File *ramdisk_find_File(const char *filename)
{
  int i;
  for(i=0; i<get_num_files(); ++i)
  {
    File *file = get_nth_file(i);
    if( same_string( (char *)file->filename, (char *)filename ) )
      return file;
  }
  return 0;
}
