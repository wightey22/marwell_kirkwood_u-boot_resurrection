/*
    GNU fdisk - a clone of Linux fdisk.

    Copyright (C) 2006
    Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/


#ifndef FUNCTION_H_INCLUDED
#define FUNCTION_H_INCLUDED

#include <parted/parted.h>
#include "../src/common.h"

#define DEFAULT_STRING "RESERVED"
#define BUFSIZE 4093

/* The names of the tempfiles */
extern const char* tempfile_name();
extern const char* tempfile2_name();


/* We have our device here */
extern PedDevice *dev;
extern PedDisk *disk;
extern UICalls uiquery;

/* Random seed */
extern int seed;
extern void set_seed(int);
/* TODO: Substitute this in the code and remove these, when finalized */
#define srand(x) set_seed(x)
#define rand() rand_r(&seed)

/* We will use common exception handling for tests */
extern PedExceptionOption ex_opts;
extern PedExceptionType ex_type;
extern PedExceptionOption exception_handler (PedException*);
extern void test_exception();

/* This are functions for UICalls */
/* NOTE: This macros are only for reference */
#define PUT_STRING(x) string[0][BUFSIZE] = 0; \
                      uiquery.getstring = getstring; \
                      strncpy(string[0], x, BUFSIZE-1)
#define PUT_TWO_STRINGS(x,y) string[0][BUFSIZE] = 2; \
                             uiquery.getstring = getstring; \
                             strncpy(string[0], x, BUFSIZE-1); \
                             strncpy(string[1], y, BUFSIZE-1)

			
extern char string[2][BUFSIZE+1];
extern int string_gets;
extern int getstring (const char* prompt, char** val, const StrList* str_list,
                      const StrList* loc_str_list, int multi_word);

extern int integer;
extern int getint (const char* prompt, int *value);

extern int bool;
extern int getbool (const char* prompt, int *value);

extern PedDevice *device;
extern int getdev (const char* prompt, PedDevice** value);

extern PedPartition *partition;
extern int getpart (const char* prompt, PedDisk**disk, PedPartition** value);

extern PedDiskType *disk_type;
extern int getdisktype (const char* prompt, PedDiskType **value);

extern PedFileSystemType *fs_type;
extern int getfstype (const char* prompt, PedFileSystemType **value);


#define POS_POSS_SIZE 10
extern int partpos;
extern int pos_gets;
extern char pos_poss[POS_POSS_SIZE];
extern int getpartpos (const char* prompt, const void* context,
                       const char *possibilities);

/* Device init functions, we should make them one */
extern int init_tempfile();
extern void unlink_tempfile();

extern int open_device();

/* Other functions */
extern int are_near(long long a, long long b);
/*extern long long llrand();*/

/* Common check things for both test programs */
extern void next_part_geometry(PedGeometry **, PedConstraint *, int factor);
/* This one takes the partition making function as a parameter   *
 * We use the fact that mkpart and mkpartfs take the same params */
extern int _mkpart (int how, PedSector start, PedSector end,
                      PedPartitionType type, const PedFileSystemType* fs_type,
                      PedPartition **newpart,
                      int mkpart (PedDisk*, PartPos *pos, PedPartitionType,
                                  const PedFileSystemType*,
                                  PedPartition **, UIOpts));
/* This is a function that both resizes and moves */
extern int _resmov (int how, PedPartition *part, PedSector start, PedSector end,
                    UIOpts opts, int action (PedDisk*, PedPartition*,
                                             PartPos *pos, UIOpts));

extern void get_max_geometry(PedGeometry **geom, PedConstraint *constraint, 
                      PedPartition *part);
extern void
randomize_position(PedSector *start, PedSector *end, PedGeometry *where, int i);
extern void change_size (const char *label_type, UIOpts opts);


#endif
