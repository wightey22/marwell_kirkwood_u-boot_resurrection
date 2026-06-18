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

#ifndef HACKS_H_INCLUDED
#define HACKS_H_INCLUDED
#include <parted/parted.h>


#ifndef SYS_TYPES_H_INCLUDED
struct _SysType {
	unsigned char type;
	const char *name;
};
typedef struct _SysType SysType;
#endif

#define MSDOS_HACK 1
#define MAC_HACK 1
#define SUN_HACK 1
#define BSD_HACK 1
#define DVH_HACK 1
#define GPT_HACK 0 /* No, thank you */
#define PC98_HACK 0
#define AMIGA_HACK 0


extern int is_blockdev(const char *file);
extern SysType *get_disklabel_system_types (const PedDiskType *type);
extern const char* get_disk_specific_system_name (const PedPartition*, int brief);
extern unsigned int get_disk_specific_system_type (const PedPartition *part, int *size);

extern void set_disk_specific_system_type (const PedPartition *part, unsigned char type);
struct bsdlabel {
  unsigned int magic;
  char unused[128];
  unsigned int magic2;
};

#define BSD_DISKMAGIC 	((u_int32_t)0x82564557)
#define BSD_BSIZE	8192

extern int is_part_type_bsd (const PedPartition* part);
extern int is_bsd_partition (char* device, PedSector start, PedSector s_size);

extern int cut_device_partnum(char *dest, size_t n, const char *devname);
extern char * get_partition_device(char *dest, size_t n, PedPartition* part, int bsd_offset);

/*
extern SysType msdos_systypes[];
extern SysType bsd_systypes[];
*/
#endif
