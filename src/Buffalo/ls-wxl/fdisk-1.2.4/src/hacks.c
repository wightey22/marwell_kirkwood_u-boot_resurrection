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

#include "../config.h"
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "hacks.h"
#include "sys_types.h"


/* FIXME: As the file is needed for lfdisk, calling it "hacks" might
 * not be the greatest idea. Rename to something.
 */


/* TODO: This might go into libparted */

/* Check if it is a block device. Returns -1 if not found. */
int is_blockdev(const char *file) {
	struct stat file_stat;
	if (stat(file,&file_stat) < 0)
		return -1;
	if(S_ISBLK(file_stat.st_mode))
		return 1;
	else
		return 0;
}

/* We need a function to get disk type-specific info about filesystem */
/* TODO: Decide if we actually need all these... I think we should leave them, at least for the 
   msdos partition type. */

/* NOTE: We will only read the first byte of the pointer, so I don't believe this can blow up
   if someone decides to change the struct, only might begin showing wrong info. */
#if MSDOS_HACK
typedef struct {
        unsigned char   system;
        int             boot;
        int             hidden;
        int             raid;
        int             lvm;
        int             lba;
        int             palo;
        int             prep;
        void*      orig;                   /* We don't need this */
} DosPartitionData;
#endif

#if MAC_HACK
/* FIXME: This could lead to problems. I rely on the belief that the struct will change as little as
   possible. If these strings are stored in memory as I believe they are, there is almost no possibility
   that it leads to crashes if the struct changes */
typedef struct {
	char		volume_name[33];	/* eg: "Games" */
	char		system_name[33];	/* eg: "Apple_Unix_SVR2" */
	char		processor_name[17];

	int		is_boot;
	int		is_driver;
	int		is_root;
	int		is_swap;
	int		is_lvm;
	int		is_raid;

	PedSector	data_region_length;
	PedSector	boot_region_length;

	long long	boot_base_address;
	long long	boot_entry_address;
	int		boot_checksum;

	uint32_t	status;
	uint32_t	driver_sig;
} MacPartitionData;
#endif

#if SUN_HACK
typedef struct {
	u_int8_t		type;
	int			is_boot;
	int			is_root;
	int			is_lvm;
} SunPartitionData;
#endif

#if BSD_HACK
typedef struct {
	uint8_t		type;
} BSDPartitionData;
#endif

#if DVH_HACK
typedef struct  {
	int	type;
	//char	name[VDNAMESIZE + 1];	/* boot volumes only */
	//int	real_file_size;		/* boot volumes only */
} DVHPartitionData;
#endif

#if 0
/* I don't think so... */
typedef struct {
        uint32_t time_low;
        uint16_t time_mid;
        uint16_t time_hi_and_version;
        uint8_t  clock_seq_hi_and_reserved;
        uint8_t  clock_seq_low;
        uint8_t  node[6];
} /* __attribute__ ((packed)) */ efi_guid_t;

typedef struct {
	efi_guid_t	type;
	efi_guid_t	uuid;
	char		name[37];
	int		lvm;
	int		raid;
	int		boot;
	int		hp_service;
        int             hidden;
        int             msftres;
} GPTPartitionData;
#endif

#if PC98_HACK
typedef struct {
	PedSector	ipl_sector;
	int		system;
	int		boot;
	int		hidden;
	char		name [17];
} PC98PartitionData;
#endif

#if AMIGA_HACK
typedef struct {
    uint32_t	pb_ID;			/* Identifier 32 bit word : 'PART' */
    uint32_t	pb_SummedLongs;		/* Size of the structure for checksums */
    int32_t	pb_ChkSum;		/* Checksum of the structure */
    uint32_t	pb_HostID;		/* SCSI Target ID of host, not really used */
    uint32_t	pb_Next;		/* Block number of the next PartitionBlock */
    uint32_t	pb_Flags;		/* Part Flags (NOMOUNT and BOOTABLE) */
    uint32_t	pb_Reserved1[2];
    uint32_t	pb_DevFlags;		/* Preferred flags for OpenDevice */
    char	pb_DriveName[32];	/* Preferred DOS device name: BSTR form */
    uint32_t	pb_Reserved2[15];
    uint32_t	de_TableSize;		/* Size of Environment vector */
    uint32_t	de_SizeBlock;		/* Size of the blocks in 32 bit words, usually 128 */
    uint32_t	de_SecOrg;	     	/* Not used; must be 0 */
    uint32_t	de_Surfaces;		/* Number of heads (surfaces) */
    uint32_t	de_SectorPerBlock;	/* Disk sectors per block, used with SizeBlock, usually 1 */
    uint32_t	de_BlocksPerTrack;	/* Blocks per track. drive specific */
    uint32_t	de_Reserved;		/* DOS reserved blocks at start of partition. */
    uint32_t	de_PreAlloc;		/* DOS reserved blocks at end of partition */
    uint32_t	de_Interleave;		/* Not used, usually 0 */
    uint32_t	de_LowCyl;		/* First cylinder of the partition */
    uint32_t	de_HighCyl;		/* Last cylinder of the partition */
    uint32_t	de_NumBuffers;		/* Initial # DOS of buffers.  */
    uint32_t	de_BufMemType;		/* Type of mem to allocate for buffers */
    uint32_t	de_MaxTransfer;		/* Max number of bytes to transfer at a time */
    uint32_t	de_Mask;		/* Address Mask to block out certain memory */
    int32_t	de_BootPri;		/* Boot priority for autoboot */
    uint32_t	de_DosType;		/* Dostype of the file system */
    uint32_t	de_Baud;		/* Baud rate for serial handler */
    uint32_t	de_Control;		/* Control word for handler/filesystem */
    uint32_t	de_BootBlocks;		/* Number of blocks containing boot code */
    uint32_t	pb_EReserved[12];
} AmigaPartitionData;
#endif


SysType *
get_disklabel_system_types (const PedDiskType *type) {
	#if MSDOS_HACK
	if (!strcmp(type->name,"msdos")) 
		return msdos_systypes;
	else
	#endif
	#if SUN_HACK
	if (!strcmp(type->name,"sun"))
		return sun_systypes;
	else
	#endif
	#if BSD_HACK
	if (!strcmp(type->name,"bsd"))
		return bsd_systypes;
	else
	#endif
	return NULL;
	
}


/* FIXME: This is temporary, make it better, split data into a seperate file */
/* Returns the name of the filesystem. If short is 1, it removes the
   "Apple_" from the name on mac partition table */
const char*
get_disk_specific_system_name (const PedPartition* part, int brief) {
	if (!part->disk_specific) return NULL;
	#if MSDOS_HACK
	if (!strcmp(part->disk->type->name,"msdos")) {
		int i;
		DosPartitionData *pd = (DosPartitionData *)part->disk_specific;
		for (i = 0; msdos_systypes[i].name; i++) {
			if(msdos_systypes[i].type == pd->system) {
				return msdos_systypes[i].name;
			}
		}
	}
	else
	#endif
	/* I believe these should be the same, but I'm not sure */
	#if SUN_HACK
	if (!strcmp(part->disk->type->name,"sun")) {
		int i;
		SunPartitionData *pd = (SunPartitionData *)part->disk_specific;
		for (i = 0; sun_systypes[i].name; i++) {
			if(sun_systypes[i].type == pd->type) {
				return sun_systypes[i].name;
			}
		}
	}
	else
	#endif
	#if BSD_HACK
	if (!strcmp(part->disk->type->name,"bsd")) {
		int i;
		BSDPartitionData *pd = (BSDPartitionData *)part->disk_specific;
		for (i = 0; bsd_systypes[i].name; i++) {
			if(bsd_systypes[i].type == pd->type) {
				return bsd_systypes[i].name;
			}
		}
	}
	else
	#endif
	#if MAC_HACK
	if (!strcmp(part->disk->type->name,"mac")) {
		MacPartitionData *pd = (MacPartitionData *)part->disk_specific;
		const char* type = pd->system_name;
		if (strncmp(type,"Apple_",6))
			return NULL;
		else {
			if (brief)
				return type+6;
			else
				return type;
		}
	}
	else
	#endif
	return NULL;
}



/* I'm lazy to write */
#define return_system_type(ltype,stype,type_var) \
	if (!strcmp(part->disk->type->name,(ltype))) { \
		stype *pd = (stype *) part->disk_specific; \
		if (size) \
			*size = sizeof(pd->type_var); \
		return pd->type_var; \
	}


/* TODO: The size gets set to the size of the field storing the type. Maybe we
   should make this a seperate function */
unsigned int
get_disk_specific_system_type (const PedPartition *part, int *size) {
	if (!part || !part->disk_specific) {
		if (size)
			*size = 0;
		return 0;
	}
	#if MSDOS_HACK
	return_system_type("msdos",DosPartitionData,system);
	#endif
	#if SUN_HACK
	return_system_type("sun",SunPartitionData,type);
	#endif
	#if BSD_HACK
	return_system_type("bsd",BSDPartitionData,type);
	#endif
	#if DVH_HACK
	/* OK, here type is int... Hope we don't break anything... */
	return_system_type("dvh",DVHPartitionData,type);
	#endif
	#if PC98_HACK
	/* OK, here type is int... Hope we don't break anything... */
	return_system_type("pc98",PC98PartitionData,system);
	#endif
	#if AMIGA_HACK
	return_system_type("amiga",AmigaPartitionData,de_DosType);
	#endif
	{
		if (size)
			*size = 0;
		return 0;
	}
	
}
/* Needed for lfdisk */
void
set_disk_specific_system_type (const PedPartition *part, unsigned char type) {
	/* TODO: I have no idea how we actually should do this */
	#if MSDOS_HACK
	if (!strcmp(part->disk->type->name,"msdos")) {
		DosPartitionData *pd = (DosPartitionData *)part->disk_specific;
		pd->system = type;
	}
	else
	#endif
	#if SUN_HACK
	if (!strcmp(part->disk->type->name,"sun")) {
		SunPartitionData *pd = (SunPartitionData *)part->disk_specific;
		pd->type = type;
	}
	else
	#endif
	#if BSD_HACK
	if (!strcmp(part->disk->type->name,"bsd")) {
		BSDPartitionData *pd = (BSDPartitionData *)part->disk_specific;
		pd->type = type;
	}
	else
	#endif
	return;
}


int
is_part_type_bsd (const PedPartition* part) {
	int type = get_disk_specific_system_type(part, NULL);
	/* We leave these for a reference */
	if (   type == 0xa5 || type == (0xa5 ^ 0x10) /* FreeBSD */
	    || type == 0xa6 || type == (0xa6 ^ 0x10) /* OpenBSD */
	    || type == 0xa9 || type == (0xa9 ^ 0x10) /* NetBSD  */
           ) return 1;
	return 0;
}

int
is_bsd_partition (char* device, PedSector start, PedSector s_size) {
	int fd;
	struct bsdlabel *lbl;
	u_char bootarea[BSD_BSIZE];

	if (!device)
		return 0;

	fd = open (device, O_RDONLY);
	if (fd < 0)
		return 0;
  
	(void)lseek (fd, (off_t)start, SEEK_SET);
  
	/* read in the boot block area. */
	if (read (fd, bootarea, BSD_BSIZE) != BSD_BSIZE) 
		return 0;
	close (fd);

	lbl = (struct bsdlabel *)&(bootarea[s_size]);
  
	if (lbl->magic != BSD_DISKMAGIC
	    || lbl->magic2 != BSD_DISKMAGIC) 
		return 0;

	return 1;
}

/* Cut the number number from a devicename. Stores the name in dest and return the number. */
int cut_device_partnum(char *dest, size_t n, const char *devname)
{

	int t,s;


	t = strlen(devname);
	/* We count the digits at the end */
	for (s = 0; s < t && isdigit(devname[t-s-1]); s++); 

	/* No digits at the end */
	if (s == 0) {
#if NAMING_BSD
		/* In BSD we have a character after the digits, specifying the partition */
		for (s = 0; s < t-1 && isdigit(devname[t-s-2]); s++); 
		if (s > 0) {
			snprintf(dest, PED_MIN(t-1,n), "%s", devname);
			return devname[t] - 'a' + 1;
		}
#endif
		strncpy(dest,devname,n);
		return 0;
		
	}
	/* For names like ad0s0, cut two chars */
	if (t > 2 && (devname[t-s-1] == 's' || 
		      devname[t-s-1] == 'p')
		  && isdigit(devname[t-s-2]))
	{
		snprintf(dest, PED_MIN(t-s,n), "%s", devname);
	}
	/* Otherwise, cut one char */
	else //if (t > 1 && !isdigit(fdisk_part_size[t-2]))
	{
		snprintf(dest, PED_MIN(t-s+1,n), "%s", devname);
	}
	
	/* We return the number */
	return atoi(devname + t - s);
}

/* Store the path to device of a part in dest. bsd_offset is used when we calculate
   the device path of a partition on a BSD disklabel on a GNU/Linux system,
   it should be equal to the number of logical partitions in most cases. */
char * get_partition_device(char *dest, size_t n, PedPartition* part, int bsd_offset)
{

	if (!strcmp(part->disk->type->name,"bsd")) {
#if NAMING_BSD
		n = snprintf(dest, n, "%s%c", part->disk->dev->path, 'a' + part->num - 1);
#else /* if NAMING_LINUX */
	/* In GNU/Linux partitions on a BSD label appear after the ones in the extended partition */
		char buf[256];
		cut_device_partnum(buf, sizeof(buf), part->disk->dev->path);
		n = snprintf(dest, n, isdigit(buf[strlen(buf)-1]) ? "%sp%d" : "%s%d",
	                         buf, part->num+bsd_offset+4);
#endif	
	}
	else{
#if NAMING_BSD
		n = snprintf(dest, n, "%ss%d", part->disk->dev->path, part->num);
#else /* if NAMING_LINUX */
		int t = strlen(part->disk->dev->path);
		n = snprintf(dest, n, isdigit(part->disk->dev->path[t-1]) ? "%sp%d" : "%s%d",
	                         part->disk->dev->path, part->num);
#endif
	}
	return n < 0 ? NULL : dest;
}


