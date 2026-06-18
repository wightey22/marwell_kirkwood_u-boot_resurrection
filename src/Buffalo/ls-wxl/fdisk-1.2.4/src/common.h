/*
    GNU fdisk - a clone of Linux fdisk.

    Copyright (C) 2006
    Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
*/



#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <parted/parted.h>
#include "strlist.h"

typedef struct _UICalls UICalls;
struct _UICalls {
	/* The first parameter specifies the prompt text */
	/* All functions should return 1 on success, 0 otherwise */
	/* getstring: 	The second parameter is the string that is read. *
	 * There are two valid string lists, the second is localized     *
	 * The function *MUST* return the string from the non-localized  *
	 * list corresponding to the localized one.                      *
	 * The integer specified how to treat multiwords, TODO: explain  */
	int (*getstring)(const char* prompt, char** value, const StrList*,
	                 const StrList* loc, int multi_word);
	int (*getint)(const char* prompt, int* value);
	int (*getbool)(const char* prompt, int* value);
	int (*getdev)(const char* prompt, PedDevice** value);
	/* getpart, if the disk is NULL, it should be specified.
	            This won't actually be used */
	int (*getparttype) (const char* prompt, const PedDisk*,
                            PedPartitionType*);
	int (*getpart)(const char* prompt, PedDisk**, PedPartition** value);
	int (*getdisktype)(const char* prompt, PedDiskType** value);
	int (*getfstype)(const char* prompt, PedFileSystemType** value);
	int (*getpartpos)(const char* prompt, const void* context, 
	                   const char *possibilities);
	PedTimer* timer;
	int need_commit;
};


typedef enum _UIOpts UIOpts;
enum _UIOpts {
	UI_DEFAULT = 0,
	/* Warn the user if the operation commits the data to the disk */
	UI_WARN_COMMIT = 1,
	/* Allow the user to adjust start/end on resize, etc */
	UI_CUSTOM_VALUES = 2,
	/* Allow the user to specify the partition type */
	UI_SPECIFY_PART_TYPE = 4,
	/* Option specific for each function */
	UI_FUNCTION_SPECIFIC_1 = 8,
	UI_FUNCTION_SPECIFIC_2 = 16
};

/* Structures used to specify partition position */
typedef struct _SectPos SectPos;
struct _SectPos {
	PedSector sector;
	PedGeometry *range;
};

typedef struct _PartPos PartPos;
struct _PartPos {
	SectPos start;
	SectPos end;
};

/* Specific for perform_resize: */
#define UI_NO_FS_RESIZE UI_FUNCTION_SPECIFIC_1	/* Will only change geometry */
#define UI_RESIZE_INEXACT UI_FUNCTION_SPECIFIC_2 /* Exact resize */
/* Specific for perform_set: */
#define UI_FLAG_TOGGLE UI_FUNCTION_SPECIFIC_1	/* perform_set must toggle flags */
/* Specific for perform_mkfs */
#define UI_MAKEPARTFS UI_FUNCTION_SPECIFIC_1

/* Setting of UICalls */
extern void set_uicalls(UICalls *);

/* Lists. Should be initialized. */
extern StrList *disk_type_list;
extern StrList *fs_type_list;
//extern StrList *fs_type_resize;
extern StrList *fs_type_mkfs;
extern StrList *flag_name_list;

/* List init functions */
extern int init_flag_str();
extern int init_fs_type_str();
extern int init_disk_type_str();

/* We will also use this functions in cfdisk */
extern int _can_create_primary (const PedDisk*);
extern int _can_create_extended (const PedDisk*);
extern int _can_create_logical (const PedDisk*);

#define can_create_primary(disk) _can_create_primary(disk)
#define can_create_extended(disk) _can_create_extended(disk)
#define can_create_logical(disk) _can_create_logical(disk)

/* Main functions */

/*
 * NOTE: resize, move and perhaps copy are dangerous.
 * Please, make your ui checks their status. If they FAIL and leave need_commit
 * to true, please reread the partition table from the device.
 * Sorry for the inconvinience. I might change this in the future so that it is
 * not needed
 */
extern int perform_check (PedDisk*, PedPartition*);
/* The parameters are the *destination* disk and partition */
extern int perform_cp (PedDisk*, PedPartition*, UIOpts);
extern int perform_mklabel (PedDevice*, PedDisk**, const PedDiskType*);

extern int perform_mkfs (PedDisk*, PedPartition*,
                         const PedFileSystemType*, UIOpts);
/* NOTE: The PartPos structs *are* modified when calling these function */
extern int perform_mkpart (PedDisk*, PartPos* pos,
                      PedPartitionType,	const PedFileSystemType*,
                      PedPartition **newpart, UIOpts);

extern int perform_mkpartfs (PedDisk*, PartPos* pos,
                        PedPartitionType, const PedFileSystemType*,
                        PedPartition **newpart, UIOpts);

extern int perform_move (PedDisk*, PedPartition*, PartPos* pos,
                    UIOpts);
extern int perform_name (PedDisk*, PedPartition*, char*);
extern int perform_rescue (PedDisk*, PedSector start, PedSector end, UIOpts);
extern int perform_resize (PedDisk*, PedPartition*, PartPos* pos,
                      UIOpts);
extern int perform_rm (PedDisk*, PedPartition*);
extern int perform_set_system (PedDisk*, PedPartition*, const PedFileSystemType*);
extern int perform_set (PedDisk*, PedPartition*, PedPartitionFlag, UIOpts);
extern int perform_commit (PedDisk*, UIOpts);
extern int perform_maximize (PedDisk*, PedPartition*);
extern int fix_partition_order (PedDisk*);
extern void check_partition_consistency (PedPartition *part);
extern void verify_partition_table (PedDisk *disk);



/* Additional functions */
extern char* partition_print_flags (PedPartition*);

extern PedConstraint*
constraint_from_start_end (PedDevice*, PedGeometry* range_start,
				PedGeometry* range_end);
extern PedConstraint*
constraint_intersect_and_destroy (PedConstraint* a, PedConstraint* b);
/* This function returns the closest possible start and end for a partition
   in the region first-last */
extern int fix_part_position(PedDevice *dev, PedSector *start, PedSector *end,
                             PedSector first, PedSector last);

/* Choose the partition position */
extern int
query_part_position(const char* prompt, const void* context, PartPos* pos,
                    PedSector first, PedSector last, PedDevice *dev, 
                    PedConstraint *constraint, UIOpts *opts);

/* Functions used by fdisk at the moment */
/* TODO: Some of these do things close to the UICalls functions */
extern int get_device(const char* prompt, PedDevice**);
extern int get_disk (const char* prompt, PedDisk**);
extern int get_partition (const char* prompt, PedDisk*, PedPartition**);
extern int get_disk_type (const char* prompt, const PedDiskType**);
extern int (*get_part_type) (const char* prompt, const PedDisk*,
                          PedPartitionType*);
extern int get_fs_type (const char* prompt, const PedFileSystemType**,
                        int mkfs);
extern int get_part_flag (const char* prompt, PedPartition*, PedPartitionFlag*);

extern PedPartition* part_list_prev (PedPartition*, PedPartitionType skip);
extern PedPartition* part_list_next (PedPartition*, PedPartitionType skip);

extern PedPartition *disk_get_prev_nmd_partition(PedDisk *disk,
                                                 PedPartition *part);
/* Other cfdisk/fdisk common functions */
extern void print_partition_types();
extern int count_logical_partition(PedDisk *disk);

#endif
