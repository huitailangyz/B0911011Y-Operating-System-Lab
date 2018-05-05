#include "files.h"
#include "processes.h"

/* This file represents a table
 * mapping file names to entry
 * points.  It is analogous to
 * tasks.c in the previous project.
 */

static File files[] =
  {
    {.filename = (const char *)"init", .process = (Process)PROC4_ADDR, .task_type = PROCESS },
    {.filename = (const char *)"PROC1", .process = (Process)PROC1_ADDR ,.task_type = PROCESS},
    {.filename = (const char *)"PROC2", .process = (Process)PROC2_ADDR, .task_type = PROCESS },
  };

#define NUM_FILES     ( sizeof(files) / sizeof(File) )

int get_num_files()
{
  return NUM_FILES;
}

File *get_nth_file(int n)
{
  return &files[n];
}



