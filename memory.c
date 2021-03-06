#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>



#define BAD_OPEN_MESSAGE					\
"Error: /proc must be mounted\n"				\
"  To mount /proc at boot you need an /etc/fstab line like:\n"	\
"      /proc   /proc   proc    defaults\n"			\
"  In the meantime, mount /proc /proc -t proc\n"

#define BUFSIZE 2048
static char buf[BUFSIZE];

#define MEMINFO_FILE "/proc/meminfo"
static int meminfo_fd = -1;

#define FILE_TO_BUF(filename, fd) do{				\
    static int local_n;						\
    if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) {	\
    fprintf(stderr, BAD_OPEN_MESSAGE);			\
    fflush(NULL);						\
    _exit(102);						\
    }								\
    lseek(fd, 0L, SEEK_SET);					\
    if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {	\
    perror(filename);					\
    fflush(NULL);						\
    _exit(103);						\
    }								\
    buf[local_n] = '\0';					\
}while(0)


typedef struct mem_table_struct {
  const char *name;     /* memory type name */
  unsigned *slot; /* slot in return struct */
} mem_table_struct;

static int compare_mem_table_structs(const void *a, const void *b){
  return strcmp(((const mem_table_struct*)a)->name,((const mem_table_struct*)b)->name);
}

/* example data, following junk, with comments added:
 *
 * MemTotal:        61768 kB    old
 * MemFree:          1436 kB    old
 * MemShared:           0 kB    old (now always zero; not calculated)
 * Buffers:          1312 kB    old
 * Cached:          20932 kB    old
 * Active:          12464 kB    new
 * Inact_dirty:      7772 kB    new
 * Inact_clean:      2008 kB    new
 * Inact_target:        0 kB    new
 * HighTotal:           0 kB
 * HighFree:            0 kB
 * LowTotal:        61768 kB
 * LowFree:          1436 kB
 * SwapTotal:      122580 kB    old
 * SwapFree:        60352 kB    old
 * Inactive:        20420 kB    2.5.41+
 * Dirty:               0 kB    2.5.41+
 * Writeback:           0 kB    2.5.41+
 * Mapped:           9792 kB    2.5.41+
 * Slab:             4564 kB    2.5.41+
 * Committed_AS:     8440 kB    2.5.41+
 * PageTables:        304 kB    2.5.41+
 * ReverseMaps:      5738       2.5.41+
 */

/* obsolete */
unsigned kb_main_shared;
/* old but still kicking -- the important stuff */
unsigned kb_main_buffers;
unsigned kb_main_cached;
unsigned kb_main_free;
unsigned kb_main_total;
unsigned kb_swap_free;
unsigned kb_swap_total;
/* recently introduced */
unsigned kb_high_free;
unsigned kb_high_total;
unsigned kb_low_free;
unsigned kb_low_total;
/* 2.4.xx era */
unsigned kb_active;
unsigned kb_inact_dirty;
unsigned kb_inact_clean;
unsigned kb_inact_target;
unsigned kb_swap_cached;  /* late 2.4 only */
/* derived values */
unsigned kb_swap_used;
unsigned kb_main_used;
/* 2.5.41+ */
unsigned kb_writeback;
unsigned kb_slab;
unsigned nr_reversemaps;
unsigned kb_committed_as;
unsigned kb_dirty;
unsigned kb_inactive;
unsigned kb_mapped;
unsigned kb_pagetables;

void meminfo(void){
  char namebuf[16]; /* big enough to hold any row name */
  mem_table_struct findme = { namebuf, NULL};
  mem_table_struct *found;
  char *head;
  char *tail;
  static const mem_table_struct mem_table[] = {
  {"Active",       &kb_active},
  {"Buffers",      &kb_main_buffers},
  {"Cached",       &kb_main_cached},
  {"Committed_AS", &kb_committed_as},
  {"Dirty",        &kb_dirty},
  {"HighFree",     &kb_high_free},
  {"HighTotal",    &kb_high_total},
  {"Inact_clean",  &kb_inact_clean},
  {"Inact_dirty",  &kb_inact_dirty},
  {"Inact_target", &kb_inact_target},
  {"Inactive",     &kb_inactive},
  {"LowFree",      &kb_low_free},
  {"LowTotal",     &kb_low_total},
  {"Mapped",       &kb_mapped},
  {"MemFree",      &kb_main_free},
  {"MemShared",    &kb_main_shared},
  {"MemTotal",     &kb_main_total},
  {"PageTables",   &kb_pagetables},
  {"ReverseMaps",  &nr_reversemaps},
  {"Slab",         &kb_slab},
  {"SwapCached",   &kb_swap_cached},
  {"SwapFree",     &kb_swap_free},
  {"SwapTotal",    &kb_swap_total},
  {"Writeback",    &kb_writeback}
  };
  const int mem_table_count = sizeof(mem_table)/sizeof(mem_table_struct);

  FILE_TO_BUF(MEMINFO_FILE,meminfo_fd);



  head = buf;
  for(;;){
    tail = strchr(head, ':');
    if(!tail) break;
    *tail = '\0';
    if(strlen(head) >= sizeof(namebuf)){
      head = tail+1;
      goto nextline;
    }
    strcpy(namebuf,head);
    found = bsearch(&findme, mem_table, mem_table_count,
        sizeof(mem_table_struct), compare_mem_table_structs
    );
    head = tail+1;
    if(!found) goto nextline;
    *(found->slot) = strtoul(head, &tail, 10);
nextline:
    tail = strchr(head, '\n');
    if(!tail) break;
    head = tail+1;
  }


  kb_swap_used = kb_swap_total - kb_swap_free;
  kb_main_used = kb_main_total - kb_main_free;
}

