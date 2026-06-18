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

#include "strlist.h"

#define N_(String) String
#if ENABLE_NLS
#  include <libintl.h>
#  include <locale.h>
#  define _(String) dgettext (PACKAGE, String)
#  define P_(String) dgettext ("parted", String)
#else
#  define _(String) (String)
#  define P_(String) (String)
#endif /* ENABLE_NLS */

#include <parted/parted.h>
#include <parted/debug.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define SMALLBUF 256

/* Christian <mail.kristian@yahoo.it> 
 *
 * This variable is global and referred from fdisk.c and cfdisk.c.
 * Value of this variable is zero when extensions must be avaiable,
 * othervise code must be compatible with linux fdisk and cfdisk. 
 */

int compat_mode = 1; /* Start compatible with fdisk and cfdisk */

/* Here we store the struct with interface functions */
static UICalls *uiquery;

static int MEGABYTE_SECTORS (PedDevice* dev)
{
  return PED_MEGABYTE_SIZE / dev->sector_size;
}



/* TODO: Decide if these should be moved to UICalls */
StrList *disk_type_list;
StrList *fs_type_list;
/*StrList *fs_type_resize;*/
StrList *fs_type_mkfs;
StrList *flag_name_list;

int
init_flag_str ()
{
  PedPartitionFlag walk;
  flag_name_list = NULL;
  for (walk = ped_partition_flag_next(0); 
       walk; walk =ped_partition_flag_next(walk)) 
    {
      flag_name_list = 
	str_list_append(flag_name_list, ped_partition_flag_get_name(walk));
      if (!flag_name_list)
	return 0;
    }
  return 1;
}

int
init_fs_type_str ()
{
	PedFileSystemType*	walk;

	fs_type_list = NULL;

	for (walk = ped_file_system_type_get_next (NULL); walk;
	     walk = ped_file_system_type_get_next (walk))
	{
		fs_type_list = str_list_append (fs_type_list, walk->name);
		if (walk->ops->create != NULL)
			fs_type_mkfs = str_list_append(fs_type_mkfs, walk->name);
		//if (talk->ops->resize != NULL)
		//	fs_type_resize = str_list_append(fs_type_mkfs, walk->name);
		if (!fs_type_list)
			return 0;
	}

	return 1;
}

int
init_disk_type_str ()
{
	PedDiskType*	walk;

	disk_type_list = NULL;

	for (walk = ped_disk_type_get_next (NULL); walk;
	     walk = ped_disk_type_get_next (walk))
	{
		disk_type_list = str_list_append (disk_type_list, walk->name);
		if (!disk_type_list)
			return 0;
	}

	return 1;
}


int
_can_create_primary (const PedDisk* disk)
{
	int	i;

	for (i = 1; i <= ped_disk_get_max_primary_partition_count (disk); i++) {
		if (!ped_disk_get_partition (disk, i))
			return 1;
	}

	return 0;
}

int
_can_create_extended (const PedDisk* disk)
{
	if (!_can_create_primary (disk))
		return 0;
	if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED))
		return 0;
	if (ped_disk_extended_partition (disk))
		return 0;
	return 1;
}

int
_can_create_logical (const PedDisk* disk)
{
	if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED))
		return 0;
	return ped_disk_extended_partition (disk) != 0;
}




static int
_partition_warn_busy (PedPartition* part)
{
	char* path = ped_partition_get_path (part);

	if (ped_partition_is_busy (part)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Partition %s is being used.  You must unmount it "
			  "before you modify it."),
			path);
		PED_FREE (path);
		return 0;
	}
	PED_FREE (path);
	return 1;
}

static int
_disk_warn_busy (PedDisk* disk)
{
	if (ped_device_is_busy (disk->dev)) {
		if (ped_exception_throw (
			PED_EXCEPTION_WARNING,
			PED_EXCEPTION_IGNORE_CANCEL,
			_("Partition(s) on %s are being used."),
			disk->dev->path)
				!= PED_EXCEPTION_IGNORE)
			return 0;
	}
	return 1;
}

static int
_warn_ext_not_empty (PedPartition *part) {
	/* If this is not an extended partition, it is ok */
	if (!(part->type & PED_PARTITION_EXTENDED))
		return 1;
	for (part = part->part_list; part;
	     part = part->next) {
		if (part->type == PED_PARTITION_LOGICAL) {
			if(ped_exception_throw (
				PED_EXCEPTION_WARNING,
				PED_EXCEPTION_YES_NO,
				_("The extended partition is not empty. "
				  "Deleting it will delete any partitions "
				  "inside it. Do you want to continue?"))
					== PED_EXCEPTION_NO)
				return 0;
			else
				return 1;
		}
	}
	return 1;
}


/* Get previous non-METADATA partition. */
PedPartition *
disk_get_prev_nmd_partition(PedDisk *disk, PedPartition *part) {
	PedPartition *walk_a, *walk_b;
	/* walk_a will iterate through partitions that are not METADATA, while
	   walk_b will iterate through all partitions. When walk_b reaches our partition,
	   walk_a is the partition we are looking for. */
	walk_a = NULL;
	walk_b = walk_a;
	do {
		if (ped_disk_next_partition(disk,walk_b) == part)
			break;
		walk_b = ped_disk_next_partition(disk,walk_b);
		if (!walk_b || !(walk_b->type & PED_PARTITION_METADATA))
			walk_a = walk_b;
	} while (walk_b);
	return walk_a;
}

/* This function returns the number of a metadata sectors, which are needed
   after a partition, for example, in msdos partition table, the sector,
   after a logical partition, describes the next logical partition */
static PedSector metadata_tail_sectors(PedPartition *part) {
	/* TODO: Make this support other partition types */
	if (part == NULL || part->disk->type == NULL)
		return 0;

	if (!strcmp(part->disk->type->name, "msdos")
	      && part->type & PED_PARTITION_LOGICAL) {
		return 1;
	}
	return 0;
}


/* This function changes "sector" to "new_sector" if the new value lies
 * within the required range.
 */
static int
snap (PedSector* sector, PedSector new_sector, PedGeometry* range)
{
        PED_ASSERT (ped_geometry_test_sector_inside (range, *sector), return 0);
        if (!ped_geometry_test_sector_inside (range, new_sector))
                return 0;
        *sector = new_sector;
        return 1;
}

typedef enum {
        MOVE_NO         = 0,
        MOVE_STILL      = 1,
        MOVE_UP         = 2,
        MOVE_DOWN       = 4
} EMoves;

enum { /* Don't change these values */
        SECT_START      =  0,
        SECT_END        = -1
};

/* Find the prefered way to adjust the sector s inside range.
 * If a move isn't allowed or is out of range it can't be selected.
 * what contains SECT_START if the sector to adjust is a start sector
 * or SECT_END if it's an end one.
 * The prefered move is to the nearest allowed boundary of the part
 * partition (if at equal distance: to start if SECT_START or to end
 * if SECT_END).
 * The distance is returned in dist.
 */
static EMoves
prefer_snap (PedSector s, int what, PedGeometry* range, EMoves* allow,
             PedPartition* part, PedSector* dist)
{
        PedSector up_dist = -1, down_dist = -1;
        PedSector new_sect;
        EMoves move;

        PED_ASSERT (what == SECT_START || what == SECT_END, return 0);

        if (!(*allow & (MOVE_UP | MOVE_DOWN))) {
                *dist = 0;
                return MOVE_STILL;
        }

        if (*allow & MOVE_UP) {
                new_sect = part->geom.end + 1 + what;
                if (ped_geometry_test_sector_inside (range, new_sect))
                        up_dist = new_sect - s;
                else
                        *allow &= ~MOVE_UP;
        }

        if (*allow & MOVE_DOWN) {
                new_sect = part->geom.start + what;
                if (ped_geometry_test_sector_inside (range, new_sect))
                        down_dist = s - new_sect;
                else
                        *allow &= ~MOVE_DOWN;
        }

        move = MOVE_STILL;
        if ((*allow & MOVE_UP) && (*allow & MOVE_DOWN)) {
                if (down_dist < up_dist || (down_dist == up_dist
                                            && what == SECT_START) )
                        move = MOVE_DOWN;
                else if (up_dist < down_dist || (down_dist == up_dist
                                                 && what == SECT_END) )
                        move = MOVE_UP;
                else
                        PED_ASSERT (0, return 0);
        } else if (*allow & MOVE_UP)
                move = MOVE_UP;
        else if (*allow & MOVE_DOWN)
                move = MOVE_DOWN;

        *dist = ( move == MOVE_DOWN ? down_dist :
                ( move == MOVE_UP   ? up_dist   :
                  0 ) );
        return move;
}

/* Snaps a partition to nearby partition boundaries.  This is useful for
 * gobbling up small amounts of free space, and also for reinterpreting small
 * changes to a partition as non-changes (eg: perhaps the user only wanted to
 * resize the end of a partition).
 *      Note that this isn't the end of the story... this function is
 * always called before the constraint solver kicks in.  So you don't need to
 * worry too much about inadvertantly creating overlapping partitions, etc.
 */
static void
snap_to_boundaries (PedGeometry* new_geom, PedGeometry* old_geom,
                    PedDisk* disk,
                    PedGeometry* start_range, PedGeometry* end_range)
{
        PedPartition*   start_part;
        PedPartition*   end_part;
        PedSector       start = new_geom->start;
        PedSector       end = new_geom->end;
        PedSector       start_dist = -1, end_dist = -1;
        EMoves          start_allow, end_allow, start_want, end_want;
        int             adjacent;

        start_want = end_want = MOVE_NO;
        start_allow = end_allow = MOVE_STILL | MOVE_UP | MOVE_DOWN;

        start_part = ped_disk_get_partition_by_sector (disk, start);
        end_part = ped_disk_get_partition_by_sector (disk, end);
        adjacent = (start_part->geom.end + 1 == end_part->geom.start);

        /* If we can snap to old_geom, then we will... */
        /* and this will enforce the snapped positions  */
        if (old_geom) {
                if (snap (&start, old_geom->start, start_range))
                        start_allow = MOVE_STILL;
                if (snap (&end, old_geom->end, end_range))
                        end_allow = MOVE_STILL;
        }

        /* If start and end are on the same partition, we */
        /* don't allow them to cross. */
        if (start_part == end_part) {
                start_allow &= ~MOVE_UP;
                end_allow &= ~MOVE_DOWN;
        }

        /* Let's find our way */
        start_want = prefer_snap (start, SECT_START, start_range, &start_allow,
                                  start_part, &start_dist );
        end_want = prefer_snap (end, SECT_END, end_range, &end_allow,
                                end_part, &end_dist );

        PED_ASSERT (start_dist >= 0 && end_dist >= 0, return);

        /* If start and end are on adjacent partitions,    */
        /* and if they would prefer crossing, then refrain */
        /* the farthest to do so. */
        if (adjacent && start_want == MOVE_UP && end_want == MOVE_DOWN) {
                if (end_dist < start_dist) {
                        start_allow &= ~MOVE_UP;
                        start_want = prefer_snap (start, SECT_START,
                                                  start_range, &start_allow,
                                                  start_part, &start_dist );
                        PED_ASSERT (start_dist >= 0, return);
                } else {
                        end_allow &= ~MOVE_DOWN;
                        end_want = prefer_snap (end, SECT_END,
                                                end_range, &end_allow,
                                                end_part, &end_dist );
                        PED_ASSERT (end_dist >= 0, return);
                }
        }

        /* New positions */
        start = ( start_want == MOVE_DOWN ? start_part->geom.start :
                ( start_want == MOVE_UP ? start_part->geom.end + 1 :
                  start ) );
        end = ( end_want == MOVE_DOWN ? end_part->geom.start - 1 :
              ( end_want == MOVE_UP ? end_part->geom.end :
                end ) );
        PED_ASSERT (ped_geometry_test_sector_inside(start_range,start), return);
        PED_ASSERT (ped_geometry_test_sector_inside (end_range, end), return);
        PED_ASSERT (start <= end,
                    PED_DEBUG (0, "start = %d, end = %d\n", start, end));
        ped_geometry_set (new_geom, start, end - start + 1);
}

/* This function makes a range less strict... */
/* FIXME: Make this in a better way */
static void
fuzzify (PedGeometry *geom, PedDevice *dev, PedConstraint *constraint,
	 PedSector before, PedSector after) {
	PedGeometry *new_geom;
	if (!constraint)
		constraint = ped_device_get_constraint(dev);
	PedSector start = geom->start-before;
	start = (start > 0 ? start : 0);
	PedSector end = geom->start+after;
	end = (end < dev->length-1LL ? end : dev->length-1LL);
	ped_geometry_set (geom, start, end-start+1);
	new_geom = ped_constraint_solve_nearest (constraint, geom);
	ped_geometry_set (geom, new_geom->start, new_geom->length);
	ped_geometry_destroy(new_geom);

}
/* This functions constructs a constraint from the following information:
 *      start, is_start_exact, end, is_end_exact.
 *
 * If is_start_exact == 1, then the constraint requires start be as given in
 * "start".  Otherwise, the constraint does not set any requirements on the
 * start.
 */
PedConstraint*
constraint_from_start_end (PedDevice* dev, PedGeometry* range_start,
                           PedGeometry* range_end)
{
        return ped_constraint_new (ped_alignment_any, ped_alignment_any,
                range_start, range_end, 1, dev->length);
}

PedConstraint*
constraint_intersect_and_destroy (PedConstraint* a, PedConstraint* b)
{
        PedConstraint* result = ped_constraint_intersect (a, b);
        ped_constraint_destroy (a);
        ped_constraint_destroy (b);
        return result;
}

/* This function returns the closest possible start and end for a partition
   in the region first-last */
int
fix_part_position(PedDevice *dev, PedSector *start, PedSector *end,
                  PedSector first, PedSector last)
{
	/* Current geometry */
	PedGeometry *geom = ped_geometry_new (dev,*start,*end-*start+1);
	if (!geom) return 0;
	/* Possible range */
	PedGeometry *range = ped_geometry_new (dev, first, last-first+1);
	if (!range) {
		ped_geometry_destroy(geom);
		return 0;
	}
	PedConstraint *constraint = constraint_intersect_and_destroy(
			constraint_from_start_end (dev, range, range),
			ped_device_get_constraint (dev));

	PedGeometry *new_geom = ped_constraint_solve_nearest(constraint, geom);
	ped_geometry_destroy(geom);
	ped_geometry_destroy(range);
	if (new_geom) {
		if (*start != new_geom->start || *end != new_geom->end) {
			*start = new_geom->start;
			*end = new_geom->end;

		}
		ped_geometry_destroy(new_geom);
		return 1;
	}

	return 0;

}


/* PedSector is signed, so we can use this to move in both directions */
static int
move_sector(PedDevice *dev, SectPos *value, PedSector shift) {
	/* FIXME: I don't believe that we will have problems with llint overflow
                  but please note that it is possible. */

	/* We will store the new values in temporary variables */
	PedSector start, end, sector;

	sector = value->sector + shift;

	/* If there is no range, we simply move the sector */
	if (!value->range) {
		if (sector >= dev->length || sector < 0LL)
			return 0;
		value->sector = sector;
		return 1;
	}

	start = value->range->start + shift;
	end = value->range->end + shift;

	/* We can't move the sector out of the device */
	if (end < 0LL) return 0;
	if (start >= dev->length) return 0;

	sector = value->sector + shift;

	/* We ensure that all values are within the device */
	if (start < 0LL) {
		start = 0LL;
		if (sector < 0LL)
			sector = 0LL;
	}

	if (end >= dev->length) {
		end = dev->length - 1LL;
		if (sector >= dev->length)
			sector = dev->length - 1LL;
	}

	if (!ped_geometry_init(value->range, dev, start, end - start + 1LL))
		return 0;
	value->sector = sector;
	return 1;

}

#define ESC	'\033'	/* ^[ */
/* TODO: This is not quite OK at the moment... */
/* We ask the user where to put the partition in a region. Used for creating and resizing partitions */
/* first and last mark the margins of the region, start and end mark the current values */
int
query_part_position(const char* prompt, const void* context, PartPos *pos,
                    PedSector first, PedSector last, PedDevice *dev,
                    PedConstraint *constraint, UIOpts *opts) {

	PedSector length = pos->end.sector - pos->start.sector + 1LL;
	PedSector min_len = 0, max_len = dev->length;
	PedGeometry *range = NULL;
	int where,i,done;
	char *temp,*temp2,*def_str, possibilities[6], buf[SMALLBUF];

	/* We reset the ranges */
	pos->start.range = NULL;
	pos->end.range = NULL;

	i = 0;
	/* If we have a constraint, check whether we can move the partition start */
	if (constraint && (constraint->start_align->grain_size == 0 ||
	                   constraint->start_range->length == 1))
		where = 's';
	else {
	/* If there the first possible sector is not the start of the selected partition,
	we need one choice in the menu to make the start fixed and one to move it back.
	When it is, 'b' and 's' do the same. We ditch 'b' for more friendly resize menu,
	action_new should use 's' as it doesn't matter there.
	FIXME: I wrote this fragment better, but ruined it. Now I'm too lazy to fix it */
		if (first < pos->start.sector) possibilities[i++] = 'b';
		possibilities[i++] = 's';
		possibilities[i++] = 'e';
		possibilities[i++] = 'c';
		possibilities [i++] = ESC;
		possibilities [i] = '\0';
		where = uiquery->getpartpos(prompt, context, possibilities);
	}
	if (where == ESC)
		return 0;
	else {
		if (where == 'c') {
			*opts |= UI_CUSTOM_VALUES;
		} else {
			*opts &= ~UI_CUSTOM_VALUES;
			if (where == 's')
				max_len = last - pos->start.sector + 1LL;
			else
				max_len = last - first + 1LL;
			if (constraint) {
				if (constraint->max_size < max_len)
					max_len = constraint->max_size;
				min_len = constraint->min_size;
			}
			done = 0;
			while (!done) {
				/* Put the max and min size on the prompt */
				temp = ped_unit_format(dev,min_len);
				temp2 = ped_unit_format(dev,max_len);
				if (temp && temp2) {
					snprintf(buf,SMALLBUF,_("Size (min %s, max %s)"),
						temp, temp2);
				} else {
					strncpy(buf,_("Size"),SMALLBUF);
				}
				if (temp) PED_FREE(temp);
				if (temp2) PED_FREE(temp2);
				def_str = ped_unit_format(dev,length);
				if (def_str) temp = strdup(def_str);
				if (!uiquery->getstring(buf, &temp, NULL, NULL, 1)) {
					if (temp) PED_FREE(temp);
					if (def_str) PED_FREE(def_str);
					return 0;
				}

				done = 1;
				/* Get the new size only if the user modified it */
				if(strcmp(temp,def_str)) {
					if (!ped_unit_parse(temp,dev,&length,&range)) {
						done = !ped_exception_throw(
							PED_EXCEPTION_ERROR,
							PED_EXCEPTION_RETRY_CANCEL,
							_("%s is an invalid partition size"),
							temp);
						if (done)
							return 0;
					}
					else if (length > max_len) {
						length = max_len;
					} else if (length < min_len) {
						length = min_len;
					}
				}
				if (temp) PED_FREE(temp);
				if (def_str) PED_FREE(def_str);
			}
			/* If we place it at the beginning, we will have range
			   with the end sector and conversly if we place it at
			   the end. We set the desired range and then we move
			   the sector with the range so that the sector is equal
			   to the desired result */
			if (where == 'b') {
				/* As the start is not equal to the end of
				   the partition, we will add some fuzz */
				/* FIXME */
				pos->start.range = ped_geometry_new(dev,
							first, 64LL);

				pos->start.sector = first;
				/* Desired: end = first+length-1LL */
				pos->end.sector = length;
				pos->end.range = range;
				move_sector(dev,&(pos->end),first-1LL);
			}
			else if (where == 'e') {
				/* If the end has changed, we will add some fuzz */
				if (pos->end.sector != last)
					pos->end.range = ped_geometry_new(dev,
							last-63LL, 64LL);

				pos->end.sector = last;
				/* Desired: start = last-length+1LL */
				pos->start.sector = length;
				pos->start.range = range;
				move_sector(dev,&(pos->start),last-2*length+1LL);
			}
			else if (where == 's') {
				/* Desired: end = start+length-1LL */
				pos->end.sector = length;
				pos->end.range = range;
				move_sector(dev, (&pos->end),
				            pos->start.sector-1LL);

			}
		}
	}
	return 1;

}

int
get_device(const char* prompt, PedDevice** value)
{
	char*		dev_name = *value ? strdup((*value)->path) : NULL;
	PedDevice*	dev;
	if (!uiquery->getdev) {
		if (!uiquery->getstring (prompt, &dev_name, NULL, NULL, 1))
			return 0;
		if (!dev_name)
			return 0;
		dev = ped_device_get (dev_name);
		free (dev_name);
		if (!dev)
			return 0;
	}
	else {
		if (!uiquery->getdev(prompt, &dev))
			return 0;
	}
	*value = dev;
	return 1;
}

int
get_disk (const char* prompt, PedDisk** value)
{
	PedDevice*	dev = *value ? (*value)->dev : NULL;
	if (!get_device(prompt, &dev))
		return 0;
	if (dev != (*value)->dev) {
		PedDisk* new_disk = ped_disk_new (dev);
		if (!new_disk)
			return 0;
		*value = new_disk;
	}
	return 1;
}

/* These two use a different aproach, it leads to a little lack of consistency, but
 * this enables the use of a shiny interface in the UI */

int
get_partition (const char* prompt, PedDisk* disk, PedPartition** value)
{
	PedPartition*	part;
	int		num;
	char		buf[SMALLBUF];
	if (!disk)
		return 0;
	if (!uiquery->getpart) {
		snprintf(buf,SMALLBUF,_("%s number (1-%d)"),
		         prompt,ped_disk_get_last_partition_num (disk));
		num = (*value) ? (*value)->num : 0;
		if (!uiquery->getint (buf, &num)) {
			ped_exception_throw (PED_EXCEPTION_ERROR,
					     PED_EXCEPTION_CANCEL,
					    _("Expecting a partition number."));
			return 0;
		}
		part = ped_disk_get_partition (disk, num);
		if (!part) {
			ped_exception_throw (PED_EXCEPTION_ERROR,
					     PED_EXCEPTION_CANCEL,
					     _("Partition doesn't exist."));
			return 0;
		}
		*value = part;
	} else {
		if (!uiquery->getpart(prompt, &disk, &part))
			return 0;
		*value = part;
	}
	return 1;
}

int
get_disk_type (const char* prompt, const PedDiskType** value)
{
	char*		disk_type_name = NULL;
	PedDiskType*	disk_type;
	int n;
	if (!uiquery->getdisktype) {
		if (*value) {
			n = strlen((*value)->name);
			disk_type_name = (char *) calloc(n,sizeof(char));
			strncpy(disk_type_name,(*value)->name, n);
		}
		if (!uiquery->getstring (prompt, &disk_type_name, disk_type_list, NULL, -1))
			return 0;
		if (!disk_type_name) {
			ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				     	_("Expecting a disk label type."));
			return 0;
		}
		disk_type = ped_disk_type_get (disk_type_name);
		free (disk_type_name);
		if (!disk_type)
			return 0;

	} else {
		if (!uiquery->getdisktype(prompt, &disk_type)) return 0;
	}
	*value = disk_type;
	PED_ASSERT (*value != NULL, return 0);
	return 1;
}


int
_get_part_type (const char* prompt, const PedDisk* disk, PedPartitionType* type)
{
	StrList*	opts = NULL;
	StrList*	locopts = NULL;
	char*		type_name = NULL;
	int		status;

	if (_can_create_primary (disk)) {
		opts = str_list_append (opts, "primary");
		locopts = str_list_append (locopts, _("primary"));
	}
	if (_can_create_extended (disk)) {
		opts = str_list_append (opts, "extended");
		locopts = str_list_append (locopts, _("extended"));
	}
	if (_can_create_logical (disk)) {
		opts = str_list_append (opts, "logical");
		locopts = str_list_append (locopts, _("logical"));
	}
	if (!opts) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Can't create any more partitions."));
		return 0;
	}

	status = uiquery->getstring(prompt,&type_name,opts,locopts,-1);

	str_list_destroy (opts);
	str_list_destroy (locopts);
	if (!status) {
		if(type_name) free(type_name);
		return 0;
	}


	if (!type_name) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Expecting a partition type."));
		return 0;
	}

	if (!strcmp (type_name, "primary")) {
		*type = 0;
	}
	if (!strcmp (type_name, "extended")) {
		*type = PED_PARTITION_EXTENDED;
	}
	if (!strcmp (type_name, "logical")) {
		*type = PED_PARTITION_LOGICAL;
	}

	free (type_name);
	return 1;
}

int
(*get_part_type) (const char* prompt, const PedDisk* disk,
                  PedPartitionType* type) = _get_part_type;

/* FIXME: I don't like this mkfs thing this way. Find a better one. */
int
get_fs_type (const char* prompt, const PedFileSystemType **value, int mkfs)
{


	char*			fs_type_name = NULL;
	PedFileSystemType*	fs_type;
	int			n;
	if (!uiquery->getfstype) {
		if(*value) {
			n = strlen((*value)->name);
			fs_type_name = (char *) calloc(n,sizeof(char));
			strncpy(fs_type_name,(*value)->name,n);
		}
		if (!uiquery->getstring(prompt, &fs_type_name,
		                        mkfs ? fs_type_mkfs : fs_type_list,
		                        NULL, -1))
			return 0;
		if (!fs_type_name) {
			ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
					     _("Expecting a file system type."));
			return 0;
		}
		fs_type = ped_file_system_type_get (fs_type_name);
		if (!fs_type) {
			ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
					     _("Unknown file system type \"%s\"."),
					     fs_type_name);
			return 0;
		}
		free (fs_type_name);
	}
	else {
		if (!uiquery->getfstype(prompt, &fs_type)) return 0;
	}
	*value = fs_type;
	return 1;
}

int
get_part_flag (const char* prompt, PedPartition *part, PedPartitionFlag *value)
{

	StrList			*opts = NULL, *locopts = NULL;
	char*			flag_name = NULL;
	PedPartitionFlag	flag = 0;
	while ( (flag = ped_partition_flag_next (flag)) ) {
		if (ped_partition_is_flag_available (part, flag)) {
			const char*	walk_name;

			walk_name = ped_partition_flag_get_name (flag);
			opts = str_list_append (opts, walk_name);
			locopts = str_list_append (locopts, P_(walk_name));
		}
	}
	if(*value)
		flag_name = strdup(ped_partition_flag_get_name(*value));
	if (!uiquery->getstring(prompt,&flag_name,opts,locopts,-1))
		return 0;
	if (!flag_name) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
					_("Expecting a flag."));
		return 0;
	}
	flag = ped_partition_flag_get_by_name (flag_name);
	if (!flag) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
					_("Unknown flag \"%s\"."),
					flag_name);
		return 0;
	}
	free (flag_name);

	*value = flag;
	return 1;
}


/* GET_SECT should have the same value as UI_CUSTOM_VALUES, as
   we want to query sector when UI_CUSTOM_VALUES is specified */
typedef enum {
	/* Query the user for a new sector */
	GET_SECT = UI_CUSTOM_VALUES,
	/* Initialise the range */
	NO_RANGE = UI_CUSTOM_VALUES*2,
	/* We are looking where to place the start of a new partition */
	//PLACE_START_NEWPART = UI_CUSTOM_VALUES*4,
	/* We are looking where to place the end of a new partition */
	PLACE_END_NEWPART = UI_CUSTOM_VALUES*8
} GSOpts;



/* This function gets sector. The prev option specifies
   the previous selected sector, used if the user
   wants to specify a size, not a position */
static int
get_sector (const char* prompt, PedDevice* dev, SectPos *value,
            PedSector prev, GSOpts opts)
{
	char*	def_str;
	char*	input = NULL;
	char*	buf;
	int	valid;

	/* If the user should be queried about the sector and a range needs to
	   be initialised, initialise one with a length of 1 sector, but please,
	   when using this function, NOTE that this is not always enough */
	if (!(opts & GET_SECT)) {
		if (!value->range && !(opts & NO_RANGE)) {
			value->range = ped_geometry_new(dev,value->sector,1);
			return value->range != NULL;
		}
		return 1;
	}

	def_str = ped_unit_format (dev, value->sector);
	if (def_str)
		input = strdup(def_str);
	/* uiquery->getstring free input if it's not NULL */
	uiquery->getstring(prompt,&input,NULL,NULL,1);

	/* Christian: 
	 * uiquery->getstring free input and set it to NULL (inserting `printf("**** INPUT: %p\n", input);' here)
	 *
	 * Command (m for help): n
	 * Partition type                                                            
   *	e   extended
   *	p   primary partition (1-4)
	 * p
	 * First cylinder  (default 0cyl):
	 * **** INPUT: (nil)
	 * Command (m for help):
	 *
	 * Force */

	if(input == NULL && def_str != NULL)
		input = strdup(def_str);

	/* def_str might have rounded sector a little bit.  If the user picked
	 * the default, make sure the selected sector is identical to the
	 * default.
	 */
	if (input && def_str && !strcmp (input, def_str)) {
		if (!(opts & NO_RANGE)) {
			if (!value->range)
				value->range = ped_geometry_new (dev,value->sector,1);
			PED_FREE (def_str);
			PED_FREE (input);
			return value->range != NULL;
		}
		PED_FREE (def_str);
		PED_FREE(input);
		return 1;
	}

	PED_FREE (def_str);
	if (!input) {
		value->sector = 0;
		if (!(opts & NO_RANGE)) {
			if (value->range)
				ped_geometry_destroy(value->range);
			value->range = NULL;
		}
		return 0;
	}

	/* If the string starts with +, interpret as size */
	if (prev >= 0 && input[0] == '+') {
		/* TODO: Improve */
		SectPos pos;
		/* We parse it into a SectPos */
		buf = calloc(128,sizeof(char));
		valid = sscanf(input, "+%127s", buf);
		free(input);
		if (!valid) {
			free(buf);
			return 0;
		}
		valid = ped_unit_parse(buf, dev, &(pos.sector), &(pos.range));
		free(buf);
		if (!valid) {
			if (pos.range) ped_geometry_destroy(pos.range);
			return 0;
		}
		/* We move the sector, so it represents the end sector */
		valid = move_sector(dev,&pos,prev-1LL);
		if (valid) {
			if (!(opts & NO_RANGE))
				value->range = pos.range;
			value->sector = pos.sector;
		} else {
			ped_geometry_destroy(pos.range);
		}

	} else {
		valid = ped_unit_parse(input, dev, &(value->sector),
		                                   &(value->range));
		PED_FREE (input);
	}
	return valid;
}

/* Looks for the first suitable free space for a new partition */
place_part_start(PedDisk *disk, PartPos *pos, PedPartitionType type) {
	PedPartition *part = NULL;
	type = (type & PED_PARTITION_LOGICAL) | PED_PARTITION_FREESPACE;
	for (part = ped_disk_next_partition(disk, NULL);
	     part; part = ped_disk_next_partition(disk,part)) {
		if (part->type == type) {
			pos->start.sector = part->geom.start;
			pos->start.range = ped_geometry_new (disk->dev,
				 		part->geom.start, 8LL);
			return 1;
		}
	}
	return 0;
}

/* This function looks where to place the end of a new partition */
static int
place_part_end(PedDisk *disk, PartPos *pos) {
	PedPartition *part = ped_disk_get_partition_by_sector(disk,
							pos->start.sector);
	if (part->type & PED_PARTITION_FREESPACE) {
		pos->end.sector = part->geom.end;
		pos->end.range = ped_geometry_new (disk->dev,
				 	part->geom.end - 7LL, 8LL);
		return 1;
	}
	else
		return 0;
}


/* Do not confuse this with query_part_position, this queries the user for
   start and end of the partition and is called from the functions in here */
static int
get_position(PartPos* pos, PedDisk *disk, GSOpts opts)
{
  char *start;
  char *end;
  if (ped_unit_get_default() == PED_UNIT_CYLINDER) 
    { 
      start = _("First cylinder");
      end = 
	(compat_mode == 0) ? 
	    _("Last cilynder or +size or +sizeMB or +sizeKB or +size%")
	  : _("Last cylinder or +size or +sizeMB or +sizeKB");
    }
  else if (ped_unit_get_default() == PED_UNIT_SECTOR) 
    {
      start = _("First sector");
      end = 
	(compat_mode == 0) ?
	  _("Last sector or +size or +sizeMB or +sizeKB or +size%")
	: _("Last sector or +size or +sizeMB or +sizeKB");
    }
  else {
    start = _("Start");
    end = _("End or +size");
  }
  
  if (!get_sector (start, disk->dev, &(pos->start), -1LL, opts))
    return 0;
  
  /* If we are creating a new partition and want to look where to
     place its end ... */
  if (opts & PLACE_END_NEWPART)
    place_part_end(disk,pos);
  
  /* We give the sector from the first query, so that the user can
     specify size */
  if (!get_sector (end, disk->dev, &(pos->end), pos->start.sector, opts))
    return 0;
  
  return 1;
}


static int
ask_boolean_question(const char* prompt) {
	int answer = 0;
	uiquery->getbool(prompt,&answer);
	return answer;
}

/* Check partition consistency */
int
perform_check (PedDisk* disk, PedPartition* part)
{
        PedFileSystem*  fs;
	if (!part) {
		if (!get_partition (_("Partition"),disk, &part))
			return 0;
	}
        if (!_partition_warn_busy (part))
                return 0;

        if (!ped_disk_check (disk))
                return 0;

        fs = ped_file_system_open (&part->geom);
        if (!fs)
                return 0;
        if (!ped_file_system_check (fs, uiquery->timer)) {
		ped_file_system_close (fs);
		return 0;
	}

        ped_file_system_close (fs);
        return 1;
}

/* Copy a partition.
   The parameters specify the destination partition.
   The source partition is get using a callback function prompt.
   Writes partition table to disk */
/* If warn is set to 1, warns the user TODO: get rid of warn */
int
perform_cp (PedDisk* dst_disk, PedPartition* dst, UIOpts opts)
{
        PedDisk*                src_disk = NULL;
        PedPartition*           src = NULL;
        PedFileSystem*          src_fs;
        PedFileSystem*          dst_fs;
        PedFileSystemType*      dst_fs_type;

	if (!dst_disk)
                goto error;

	if (opts & UI_WARN_COMMIT)
		if (!ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
	/* It doesn't hurt to commit at that point, right? */
	if (uiquery->need_commit && !perform_commit(dst_disk,UI_DEFAULT)) goto error;


        src_disk = dst_disk;
	if (!get_disk (_("Source device"), &src_disk))
		goto error_destroy_disk;


        if (!get_partition (_("Source partition"),src_disk, &src))
                goto error_destroy_disk;
        if (src->type == PED_PARTITION_EXTENDED) {
                ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                        _("Can't copy an extended partition."));
                goto error_destroy_disk;
        }
        if (!_partition_warn_busy (src))
                goto error_destroy_disk;
	if (!dst) {
		if (!get_partition (_("Destination partition"),dst_disk, &dst))
			goto error_destroy_disk;
	}
        if (!_partition_warn_busy (dst))
                goto error_destroy_disk;

/* do the copy */
        src_fs = ped_file_system_open (&src->geom);
        if (!src_fs)
                goto error_destroy_disk;
	/* At this point we do something worth commiting. If we fail, the caller should clean the mess */
	uiquery->need_commit = 1;
        dst_fs = ped_file_system_copy (src_fs, &dst->geom, uiquery->timer);
        if (!dst_fs)
                goto error_close_src_fs;
        dst_fs_type = dst_fs->type;     /* may be different to src_fs->type */
        ped_file_system_close (src_fs);
        ped_file_system_close (dst_fs);

/* update the partition table, close disks */
        if (!ped_partition_set_system (dst, dst_fs_type))
                goto error_destroy_disk;
        if (!ped_disk_commit (dst_disk)) {
		//if (uiquery->need_commit) uiquery->need_commit = -1;
                goto error_destroy_disk;
	}
	uiquery->need_commit = 0;
        if (src_disk != dst_disk)
                ped_disk_destroy (src_disk);
        /*ped_disk_destroy (dst_disk);*/
        return 1;

error_close_src_fs:
        ped_file_system_close (src_fs);
error_destroy_disk:
        if (src_disk && src_disk != dst_disk)
                ped_disk_destroy (src_disk);
        /*ped_disk_destroy (dst_disk);*/
error:
        return 0;
}

/* Create a new partition table. Takes dev and disk as parameters,
   in addition it takes a label type, if NULL, queries... */

int
perform_mklabel (PedDevice* dev, PedDisk** disk, const PedDiskType* type)
{
	/*
        ped_exception_fetch_all ();

        if (!*disk) ped_exception_catch ();
        ped_exception_leave_all ();*/

        if (*disk) 
	  {
	    if (!_disk_warn_busy (*disk)) 
	      {
		/* http://lists.gnu.org/archive/html/bug-fdisk/2008-12/msg00010.html*/
		/* ped_disk_destroy (*disk); */
		goto error;
	      }
                ped_disk_destroy (*disk);
        }
	if (!type) 
	  {
	    if (!get_disk_type (_("New disk label type"), &type))
	      goto error;
	  }

        *disk = ped_disk_new_fresh (dev, type);
        if (!*disk)
	  goto error;

        /*if (!ped_disk_commit (*disk))
                goto error;*/
	uiquery->need_commit = 1;
        return 1;

error:
        return 0;
}

/* Create a filesystem. Takes filesystem type as an optional parameter */
/* If warn is set to 1, warns the user TODO: get rid of warn */
int
perform_mkfs (PedDisk* disk, PedPartition* part, const PedFileSystemType* type, UIOpts opts)
{

        PedFileSystem*          fs;
	PedConstraint*		constraint = NULL;
	if (opts & UI_WARN_COMMIT) {
		if (!ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
	}

        if (!disk)
                return 0;
	if (!part) {
		if (!get_partition (_("Partition"),disk, &part))
			return 0;
	}
        if (!_partition_warn_busy (part))
                return 0;
	if (!type) {
		type = ped_file_system_type_get ("ext2");
        	if (!get_fs_type (_("File system"), &type, 1))
	                return 0;
	}
	/* We check whether we can create this partition here */
	constraint = ped_file_system_get_create_constraint(type,disk->dev);
	//if (!constraint) return 0;
	if (constraint)
		if (!ped_constraint_is_solution(constraint,&(part->geom))) {
			if (part->geom.length < constraint->min_size) {
				ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The partition is too small to create %s filesystem on it."), type->name);
				ped_constraint_destroy(constraint);
				return 0;
			}
			else if (part->geom.length > constraint->max_size) {
				ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				_("The partition is too big to create %s filesystem on it"), type->name);
				ped_constraint_destroy(constraint);
				return 0;
			}
			/* FIXME: WHAT THE?.. */
			//else {
			//	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			//        _("You can't create %s filesystem on this partition."), type->name);
			//}
			//ped_constraint_destroy(constraint);
			//return 0;
		}
	ped_constraint_destroy(constraint);

        fs = ped_file_system_create (&part->geom, type, uiquery->timer);
        if (!fs)
                return 0;
        ped_file_system_close (fs);

        if (!ped_partition_set_system (part, type))
                return 0;
        if (ped_partition_is_flag_available (part, PED_PARTITION_LBA))
                ped_partition_set_flag (part, PED_PARTITION_LBA, 1);
        if (!ped_disk_commit (disk)) {
		//if (uiquery->need_commit) uiquery->need_commit = -1;
               return 0;
	}
	uiquery->need_commit = 0;
        return 1;


}

/* The parameter custom is 1, then the user is asked about the start and end sector *
 * If the parameter newpart is not NULL, it is set to the newly created partition */
/* NOTE: On some occassions it changes the *newpart to NULL! */
int
perform_mkpart (PedDisk* disk, PartPos *pos, PedPartitionType part_type,
	const PedFileSystemType* fs_type, PedPartition **newpart, UIOpts opts)
{
        PedPartition*            part;
        PedConstraint*           user_constraint;
        PedConstraint*           dev_constraint;
        PedConstraint*           final_constraint;
	PedConstraint*		 fs_constraint;
       /* char*                    peek_word;*/
        char*                    part_name = NULL;
        char                     *start_usr = NULL, *end_usr = NULL;
        char                     *start_sol = NULL, *end_sol = NULL;

        if (!disk)
                goto error;

        if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED)) {
		if (part_type && part_type != PED_PARTITION_NORMAL)
			goto error;
                part_type = PED_PARTITION_NORMAL;
        } else if (opts & UI_SPECIFY_PART_TYPE) {
                if (!get_part_type (_("Partition type"), disk, &part_type))
                        goto error;
        }


        if (part_type == PED_PARTITION_EXTENDED) {
		if (opts & UI_MAKEPARTFS) {
                	ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                        	_("An extended partition cannot hold a file system."));
                	return 0;
        	}
		else
                	fs_type = NULL;
        } else if (!fs_type) {
                if (!get_fs_type (_("File system type"), &fs_type, 0))
                        goto error;
        }




        if (ped_disk_type_check_feature (disk->type,
                                         PED_DISK_TYPE_PARTITION_NAME))
                uiquery->getstring(_("Partition name"), &part_name, NULL, NULL, 1);


	if ((opts & UI_CUSTOM_VALUES) &&
			pos->start.sector == 0LL && pos->end.sector == 0LL) {
		place_part_start(disk,pos,part_type);
		if (!get_position(pos, disk, GET_SECT | PLACE_END_NEWPART))
			goto error;
	}
	else {
		if (!get_position(pos, disk, opts & UI_CUSTOM_VALUES))
			goto error;
	}

        /* processing starts here */
        part = ped_partition_new (disk, part_type, fs_type,
	                          pos->start.sector, pos->end.sector);
        if (!part)
                goto error;

	/* Check if the partition occupying the space before the place we
	   are trying to put this one, requires a metadata sectors after it,
	   and if so, increase the range of the constraint */


	fuzzify(pos->start.range, disk->dev, NULL, 0,
		metadata_tail_sectors(
			disk_get_prev_nmd_partition(
				disk,
				ped_disk_get_partition_by_sector(
					disk,
					pos->start.sector
				)
			)
		)
	);

        snap_to_boundaries (&part->geom, NULL, disk,
	                    pos->start.range, pos->end.range);

        /* create constraints */
        user_constraint = constraint_from_start_end (disk->dev,
                        pos->start.range, pos->end.range);
        PED_ASSERT (user_constraint != NULL, return 0);

        dev_constraint = ped_device_get_constraint (disk->dev);
        PED_ASSERT (dev_constraint != NULL, return 0);

        final_constraint = ped_constraint_intersect (user_constraint,
                        dev_constraint);
        if (!final_constraint)
                goto error_destroy_simple_constraints;

        /* subject to partition constraint */
        ped_exception_fetch_all();
        if (!ped_disk_add_partition (disk, part, final_constraint)) {
		ped_exception_catch();
                ped_exception_leave_all();

                if (ped_disk_add_partition (disk, part,
                                        ped_constraint_any (disk->dev))) {
                        start_usr = ped_unit_format (disk->dev, pos->start.sector);
                        end_usr   = ped_unit_format (disk->dev, pos->end.sector);
                        start_sol = ped_unit_format (disk->dev, part->geom.start);
                        end_sol   = ped_unit_format (disk->dev, part->geom.end);
			/* TODO: With the changes from 06/08/06, we might remove
			   the "if" and warn the user always if this is out of
			   range */
			if (opts & UI_CUSTOM_VALUES)
				switch (ped_exception_throw (
					PED_EXCEPTION_WARNING,
					PED_EXCEPTION_YES_NO,
					_("You requested a partition from %s to %s.\n"
					"The closest location we can manage is "
					"%s to %s.  "
					"Is this still acceptable to you?"),
					start_usr, end_usr, start_sol, end_sol))
				{
					case PED_EXCEPTION_YES:
						/* all is well in this state */
						break;
					case PED_EXCEPTION_NO:
					case PED_EXCEPTION_UNHANDLED:
					default:
						/* undo partition addition */
						goto error_remove_part;
				}
                } else {
                        goto error_remove_part;
                }
        } else {
		ped_exception_leave_all();
	}

	/* We check whether we can create this filesystem here */
	if (fs_type)
		fs_constraint = ped_file_system_get_create_constraint(fs_type,disk->dev);
	else
		fs_constraint = NULL;
	if (fs_constraint) {
		if (!ped_constraint_is_solution(fs_constraint,&(part->geom))) {
			const char *error = NULL;
			if (part->geom.length < fs_constraint->min_size) {
				error = _("The size of the partition is too small for %s filesystem");
			}
			else if (part->geom.length > fs_constraint->max_size) {
				error = _("The size of the partition is too big for %s filesystem");
			}
			/* FIXME: WHAT THE?.. */
			/* OK, I got it, but I will leave it here for a while */
			//else {
			//	error = _("You can't use this partition for %s filesystem");
			//}
			if (error)
				switch (ped_exception_throw (
					PED_EXCEPTION_WARNING,
					PED_EXCEPTION_IGNORE_CANCEL,
					error, fs_type->name)) {
					case PED_EXCEPTION_IGNORE:
						/* User doesn't care */
						break;
					case PED_EXCEPTION_CANCEL:
					case PED_EXCEPTION_UNHANDLED:
					default:
						ped_constraint_destroy(fs_constraint);
						goto error_remove_part;
				}
		}
		ped_constraint_destroy(fs_constraint);
	}

        /* set minor attributes */
        if (part_name)
                PED_ASSERT (ped_partition_set_name (part, part_name), return 0);
        if (!ped_partition_set_system (part, fs_type))
                goto error;
        if (ped_partition_is_flag_available (part, PED_PARTITION_LBA))
                ped_partition_set_flag (part, PED_PARTITION_LBA, 1);

        /*if (!ped_disk_commit (disk))
                goto error;*/
	uiquery->need_commit = 1;

        /* clean up */
        ped_constraint_destroy (final_constraint);
        ped_constraint_destroy (user_constraint);
        ped_constraint_destroy (dev_constraint);


        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);

        if (start_usr != NULL)
                PED_FREE (start_usr);
        if (end_usr != NULL)
                PED_FREE (end_usr);
        if (start_sol != NULL)
                PED_FREE (start_sol);
        if (end_sol != NULL)
                PED_FREE (end_sol);
	if (newpart) *newpart = part;
        return 1;

error_remove_part:
	/* We need to make this NULL here, for cfdisk's usage */
	if (newpart) *newpart = NULL;
        ped_disk_remove_partition (disk, part);
error_destroy_all_constraints:
        ped_constraint_destroy (final_constraint);
error_destroy_simple_constraints:
        ped_constraint_destroy (user_constraint);
        ped_constraint_destroy (dev_constraint);
error_destroy_part:
        ped_partition_destroy (part);
error:
        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);

        if (start_usr != NULL)
                PED_FREE (start_usr);
        if (end_usr != NULL)
                PED_FREE (end_usr);
        if (start_sol != NULL)
                PED_FREE (start_sol);
        if (end_sol != NULL)
                PED_FREE (end_sol);

        return 0;
}

int
perform_mkpartfs (PedDisk* disk, PartPos *pos, PedPartitionType part_type,
	const PedFileSystemType* fs_type, PedPartition **newpart, UIOpts opts)
{

        PedPartition*            part = NULL;


        if (!disk)
                return 0;

	if (opts & UI_WARN_COMMIT) {
		if (!ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
	}

        if (!fs_type) {
                if (!get_fs_type (_("File system type"), &fs_type, 1))
                        return 0;
        }
	if (!perform_mkpart(disk,pos,part_type,fs_type,&part,opts | UI_MAKEPARTFS))
		return 0;
	if (!perform_mkfs(disk,part,fs_type,opts & ~UI_WARN_COMMIT))
		return 0;
	if (newpart) *newpart = part;
	return 1;


}

/* FIXME: This function seems to be problematic, deal with it appropriately in the ui */
int
perform_move (PedDisk *disk,  PedPartition* part, PartPos *pos, UIOpts opts)
{

        PedFileSystem*  fs;
        PedFileSystem*  fs_copy;
        PedConstraint*  constraint;
        PedGeometry     old_geom, new_geom;


        if (!disk)
                goto error;

	if (opts & UI_WARN_COMMIT) {
		if (!ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
	}
	/* So the best we can do here is to commit */
	if (uiquery->need_commit)
		if (!perform_commit(disk,UI_DEFAULT))
			goto error;
	if (!part)
        	if (!get_partition (_("Partition"), disk, &part))
                	goto error;
        if (!_partition_warn_busy (part))
                goto error;
        if (part->type == PED_PARTITION_EXTENDED) {
                ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                        _("Can't move an extended partition."));
                goto error;
        }
        old_geom = part->geom;
        fs = ped_file_system_open (&old_geom);
        if (!fs)
                goto error;

        /* get new target */
	if ((opts & UI_CUSTOM_VALUES) &&
			pos->start.sector == 0LL && pos->end.sector == 0LL) {
		place_part_start(disk,pos,part->type);
		if (!get_position(pos, disk, GET_SECT | PLACE_END_NEWPART))
			goto error_close_fs;
	}
	else {
		if (!get_position(pos, disk, opts & UI_CUSTOM_VALUES))
			goto error_close_fs;
	}


        /* set / test on "disk" */
        if (!ped_geometry_init (&new_geom, disk->dev, pos->start.sector,
                                pos->end.sector - pos->start.sector + 1LL))
                goto error_close_fs;
        snap_to_boundaries (&new_geom, NULL, disk,
                            pos->start.range, pos->end.range);

        constraint = constraint_intersect_and_destroy (
                        ped_file_system_get_copy_constraint (fs, disk->dev),
                        constraint_from_start_end (disk->dev, pos->start.range,
                                                   pos->end.range));
	/* So here we do something worth commiting and dangerous. UI *must*
	   do something if we fail and need_commit is true... */
	uiquery->need_commit = 1;
        if (!ped_disk_set_partition_geom (disk, part, constraint,
                                          new_geom.start, new_geom.end))
                goto error_destroy_constraint;
        ped_constraint_destroy (constraint);
        if (ped_geometry_test_overlap (&old_geom, &part->geom)) {
                ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
                        _("Can't move a partition onto itself.  Try using resize, perhaps?"));
                goto error_close_fs;
        }

        /* do the move */
        fs_copy = ped_file_system_copy (fs, &part->geom, uiquery->timer);
        if (!fs_copy) {
		#if 0
		ped_constraint_destroy (constraint);
		constraint = ped_constraint_any(disk->dev);
		ped_disk_set_partition_geom(disk,part,constraint,
		                            old_geom.start,old_geom.end);
		#endif
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Couldn't run the partition copy"));
                goto error_close_fs;
	}
        ped_file_system_close (fs_copy);
        ped_file_system_close (fs);
        if (!ped_disk_commit (disk)) {
		//if (uiquery->need_commit) uiquery->need_commit = -1;
                goto error;
	}
	uiquery->need_commit = 0;
        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);
        return 1;

error_destroy_constraint:
        ped_constraint_destroy (constraint);
error_close_fs:
        ped_file_system_close (fs);
error:
        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);
        return 0;
}

int
perform_name (PedDisk* disk, PedPartition* part, char *name)
{
	const char *temp;
	int n;
        if (!disk)
                goto error;
	if (!part)
        	if (!get_partition (_("Partition"), disk, &part))
                	goto error;
	if (!name) {
		temp = ped_partition_get_name(part);
		n = strlen(temp);
		name = (char *) calloc(n,sizeof(char));
		strncpy(name,temp,n);
        	if (!uiquery->getstring(_("Partition name"), &name, NULL, NULL, 0))
			goto error;
	}

        if (!name)
                goto error;
        if (!ped_partition_set_name (part, name))
                goto error_free_name;
        free (name);

        /*if (!ped_disk_commit (disk))
                goto error_destroy_disk;*/
	uiquery->need_commit = 1;
        return 1;

error_free_name:
        free (name);
error:
        return 0;
}



static PedPartitionType
_disk_get_part_type_for_sector (PedDisk* disk, PedSector sector)
{
        PedPartition*   extended;

        extended = ped_disk_extended_partition (disk);
        if (!extended
            || !ped_geometry_test_sector_inside (&extended->geom, sector))
                return 0;

        return PED_PARTITION_LOGICAL;
}

/* This function checks if "part" contains a file system, and returs
 *      0 if either no file system was found, or the user declined to add it.
 *      1 if a file system was found, and the user chose to add it.
 *      -1 if the user chose to cancel the entire search.
 */
static int
_rescue_add_partition (PedPartition* part)
{
        const PedFileSystemType*        fs_type;
        PedGeometry*                    probed;
        PedExceptionOption              ex_opt;
        PedConstraint*                  constraint;
        char*                           found_start;
        char*                           found_end;

        fs_type = ped_file_system_probe (&part->geom);
        if (!fs_type)
                return 0;
        probed = ped_file_system_probe_specific (fs_type, &part->geom);
        if (!probed)
                return 0;

        if (!ped_geometry_test_inside (&part->geom, probed)) {
                ped_geometry_destroy (probed);
                return 0;
        }

        constraint = ped_constraint_exact (probed);
        if (!ped_disk_set_partition_geom (part->disk, part, constraint,
                                          probed->start, probed->end)) {
                ped_constraint_destroy (constraint);
                return 0;
        }
        ped_constraint_destroy (constraint);

        found_start = ped_unit_format (probed->dev, probed->start);
        found_end = ped_unit_format (probed->dev, probed->end);
        ex_opt = ped_exception_throw (
                PED_EXCEPTION_INFORMATION,
                PED_EXCEPTION_YES_NO_CANCEL,
                _("A %s %s partition was found at %s -> %s.  "
                  "Do you want to add it to the partition table?"),
                fs_type->name, ped_partition_type_get_name (part->type),
                found_start, found_end);
        ped_geometry_destroy (probed);
        PED_FREE (found_start);
        PED_FREE (found_end);

        switch (ex_opt) {
                case PED_EXCEPTION_CANCEL: return -1;
                case PED_EXCEPTION_NO: return 0;
        }

        ped_partition_set_system (part, fs_type);
        ped_disk_commit (part->disk);
	uiquery->need_commit = 0;
        return 1;
}

/* hack: we only iterate through the start, since most (all) fs's have their
 * superblocks at the start.  We'll need to change this if we generalize
 * for RAID, or something...
 */
static int
_rescue_pass (PedDisk* disk, PedGeometry* start_range, PedGeometry* end_range)
{
        PedSector               start;
        PedGeometry             start_geom_exact;
        PedGeometry             entire_dev;
        PedConstraint           constraint;
        PedPartition*           part;
        PedPartitionType        part_type;

        part_type = _disk_get_part_type_for_sector (
                        disk, (start_range->start + end_range->end) / 2);

        ped_geometry_init (&entire_dev, disk->dev, 0, disk->dev->length);

        ped_timer_reset (uiquery->timer);
        ped_timer_set_state_name (uiquery->timer, _("searching for file systems"));
        for (start = start_range->start; start <= start_range->end; start++) {
                ped_timer_update (uiquery->timer, 1.0 * (start - start_range->start)
                                         / start_range->length);

                ped_geometry_init (&start_geom_exact, disk->dev, start, 1);
                ped_constraint_init (
                        &constraint, ped_alignment_any, ped_alignment_any,
                        &start_geom_exact, &entire_dev,
                        1, disk->dev->length);
                part = ped_partition_new (disk, part_type, NULL, start,
                                end_range->end);
                if (!part) {
                        ped_constraint_done (&constraint);
                        continue;
                }

                ped_exception_fetch_all ();
                if (ped_disk_add_partition (disk, part, &constraint)) {
                        ped_exception_leave_all ();
                        switch (_rescue_add_partition (part)) {
                        case 1:
                                ped_constraint_done (&constraint);
                                return 1;

                        case 0:
                                ped_disk_remove_partition (disk, part);
                                break;

                        case -1:
                                goto error_remove_partition;
                        }
                } else {
                        ped_exception_leave_all ();
                }
                ped_partition_destroy (part);
                ped_constraint_done (&constraint);
        }
        ped_timer_update (uiquery->timer, 1.0);

        return 1;

error_remove_partition:
        ped_disk_remove_partition (disk, part);
error_partition_destroy:
        ped_partition_destroy (part);
error_constraint_done:
        ped_constraint_done (&constraint);
error:
        return 0;
}

/* TODO: This might be ok without commiting */
int
perform_rescue (PedDisk* disk, PedSector start, PedSector end, UIOpts opts)
{

        PedSector               fuzz;
        PedGeometry             probe_start_region;
        PedGeometry             probe_end_region;


        if (!disk)
                goto error;
	if (opts & UI_WARN_COMMIT)
		if (!ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
	/* Why the hell I use two seperate ifs, instead of one? I wish I knew */
	if (uiquery->need_commit)
		if (!perform_commit(disk,UI_DEFAULT))
			goto error;
	if (opts & UI_CUSTOM_VALUES) {
		PartPos pos;
		pos.start.sector = start;
		pos.end.sector = end;
		if (!get_position(&pos, disk, GET_SECT | NO_RANGE ))
			goto error;
		start = pos.start.sector;
		end = pos.end.sector;
	}
        fuzz = PED_MAX (PED_MIN ((end - start) / 10, MEGABYTE_SECTORS(disk->dev)),
                        MEGABYTE_SECTORS(disk->dev) * 16);

        ped_geometry_init (&probe_start_region, disk->dev,
                           PED_MAX(start - fuzz, 0),
                           PED_MIN(2 * fuzz, (disk->dev)->length - (start - fuzz)));
        ped_geometry_init (&probe_end_region, disk->dev,
                           PED_MAX(end - fuzz, 0),
                           PED_MIN(2 * fuzz, (disk->dev)->length - (end - fuzz)));
	/* I don't believe this is dangerous, but still */
	uiquery->need_commit = 1;
        if (!_rescue_pass (disk, &probe_start_region, &probe_end_region))
                goto error;


        return 1;


error:
        return 0;
}



int
perform_resize (PedDisk *disk, PedPartition *part, PartPos *pos, UIOpts opts)
{

        PedFileSystem           *fs;
        PedConstraint           *constraint;
        PedGeometry             new_geom;


        if (!disk)
                goto error;

	if (!part)
        	if (!get_partition (_("Partition"), disk, &part))
                	goto error;

	if (!(part->type == PED_PARTITION_EXTENDED || (opts & UI_NO_FS_RESIZE)))  {
		if (opts & UI_WARN_COMMIT && !ask_boolean_question
		(_("WARNING: This writes all data to disk automatically, continue?")))
			return 0;
		if (uiquery->need_commit)
			if (!perform_commit(disk,UI_DEFAULT))
				goto error;
	}

        if (part->type != PED_PARTITION_EXTENDED) {
                if (!_partition_warn_busy (part))
                        goto error;
        }

 	if (!get_position(pos, disk,opts & UI_CUSTOM_VALUES))
		goto error;

	/* FIXME: This is most likely not needed */
	//if (!(opts & UI_CUSTOM_VALUES) && (opts & UI_NO_FS_RESIZE)) {
	if (opts & UI_RESIZE_INEXACT) {
		if (part->geom.start != pos->start.sector)
			fuzzify(pos->start.range,disk->dev,NULL, 127, 127);
		if (part->geom.end != pos->end.sector)
			fuzzify(pos->end.range,disk->dev,NULL, 127, 127);
	}
	//}
	if (!ped_geometry_init (&new_geom, disk->dev, pos->start.sector,
	                        pos->end.sector - pos->start.sector + 1LL))
                goto error;
        snap_to_boundaries (&new_geom, &part->geom, disk,
                            pos->start.range, pos->end.range);
	/* If the partition is extended or we have UI_NO_FS_RESIZE,
	   don't try to resize the fs... FIXME? */
        if (part->type == PED_PARTITION_EXTENDED || (opts & UI_NO_FS_RESIZE)) {
		/* I really hope this part isn't dangerous */
                constraint = constraint_from_start_end (disk->dev,
                                pos->start.range, pos->end.range);
                if (!ped_disk_set_partition_geom (disk, part, constraint,
                                                  new_geom.start, new_geom.end))
                        goto error_destroy_constraint;
		if (part->type == PED_PARTITION_EXTENDED)
                	ped_partition_set_system (part, NULL);

		uiquery->need_commit = 1;
        } else {
		fs = ped_file_system_open (&part->geom);
		if (!fs)
			goto error;
		constraint = constraint_intersect_and_destroy (
				ped_file_system_get_resize_constraint (fs),
				constraint_from_start_end (disk->dev,
					pos->start.range, pos->end.range));
		/* We will play with the geometry */
		uiquery->need_commit = 1;
                if (!ped_disk_set_partition_geom (disk, part, constraint,
                                                  new_geom.start, new_geom.end))
                        goto error_close_fs;
                if (!ped_file_system_resize (fs, &part->geom, uiquery->timer))
                        goto error_close_fs;
                /* may have changed... eg fat16 -> fat32 */
                ped_partition_set_system (part, fs->type);
                ped_file_system_close (fs);

		ped_disk_commit (disk);
		uiquery->need_commit = 0;
        }

        ped_constraint_destroy (constraint);
        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);
        return 1;

error_close_fs:
        ped_file_system_close (fs);
error_destroy_constraint:
        ped_constraint_destroy (constraint);
error:
        if (pos->start.range != NULL)
                ped_geometry_destroy (pos->start.range);
        if (pos->end.range != NULL)
                ped_geometry_destroy (pos->end.range);
        return 0;
}

int
perform_rm (PedDisk* disk, PedPartition* part)
{



        if (!disk)
                goto error;
	if (!part)
        	if (!get_partition (_("Partition"), disk, &part))
                	goto error;
        if (!_partition_warn_busy (part))
                goto error;
	if (!_warn_ext_not_empty (part))
		goto error;

        ped_disk_delete_partition (disk, part);
        uiquery->need_commit = 1;
        return 1;


error:
        return 0;
}

int
perform_maximize (PedDisk* disk, PedPartition* part)
{
	PedConstraint *constraint = NULL;
	if (!disk)
		return 0;
	if (!part)
		if (!get_partition(_("Partition"), disk, &part))
			return 0;
	if (!_partition_warn_busy(part))
		return 0;
	constraint = ped_constraint_any(disk->dev);
	if (!constraint)
		return 0;
	if (!ped_disk_maximize_partition(disk,part,constraint)) {
		ped_constraint_destroy (constraint);
		return 0;
	}
	ped_constraint_destroy(constraint);
	return 1;
}

int
perform_set_system (PedDisk* disk, PedPartition* part, const PedFileSystemType *type)
{
	if (!disk)
		return 0;
	if (!part)
		if (!get_partition(_("Partition"), disk, &part))
			return 0;
	if (!type) {
		if (!get_fs_type (_("File system type"), &type, 0))
			return 0;
	}
	/* FIXME: Is this needed? We don't touch the partition itself */
	if (!_partition_warn_busy(part))
		return 0;
	if (!ped_partition_set_system(part,type))
		return 0;
	uiquery->need_commit = 1;
	return 1;
}


int
perform_set (PedDisk *disk, PedPartition *part, PedPartitionFlag flag, UIOpts opts)
{
        int                     state;

        if (!disk)
                goto error;
        if (!part)
		if (!get_partition (_("Partition"), disk, &part))
			goto error;
	if (!flag)
		if (!get_part_flag (_("Flag to change"), part, &flag))
			goto error;
        state = (ped_partition_get_flag (part, flag) == 0 ? 1 : 0);

        if (!(opts & UI_FLAG_TOGGLE)) {
                if (!uiquery->getbool (_("New state"), &state))
		            goto error;
        }

        if (!ped_partition_set_flag (part, flag, state))
	        	goto error;
    	uiquery->need_commit = 1;

	return 1;

error:
        return 0;
}

int
perform_commit (PedDisk* disk, UIOpts opts)
{
	if (opts & UI_WARN_COMMIT)
		if (!ask_boolean_question
			(_("WARNING: This writes all data to disk, continue?")))
				return 0;
	if (ped_disk_commit(disk)) {
		uiquery->need_commit = 0;
		return 1;
	}
	else return 0;
}

/* Experimental fixing of the partition order */

/* We will copy this part of the DosPartitionData */
struct DosPDCopyMem {
	unsigned char	system;
	int		boot;
	int		hidden;
	int		raid;
	int		lvm;
	int		lba;
	int		palo;
	int		prep;
};
#define DOS_DATA_SIZE sizeof(struct DosPDCopyMem)



struct DOSGeomList {
	PedGeometry geom;
	void *part_data;

	/* These are not necessary */
	const PedFileSystemType * fs_type;
	PedPartitionType type;

	struct DOSGeomList *next;
};

static int
_fix_order_add_part (PedDisk *disk, struct DOSGeomList *data) {
	int success;
	PedConstraint *constraint;
	PedGeometry *range_start, *range_end;
	PedPartition *part;
	part = ped_partition_new (disk, data->type, data->fs_type,
					data->geom.start, data->geom.end);

	range_start = ped_geometry_new (disk->dev,data->geom.start, 1);
	range_end = ped_geometry_new (disk->dev,data->geom.end, 1);
	constraint = constraint_from_start_end(disk->dev,
						range_start,range_end);
	memcpy(part->disk_specific, data->part_data, DOS_DATA_SIZE);
	success = ped_disk_add_partition (disk, part, constraint);

	if (constraint) ped_constraint_destroy(constraint);
	if (range_start) ped_geometry_destroy(range_start);
	if (range_end) ped_geometry_destroy(range_end);
	if (!success) ped_partition_destroy(part);

	return success;
}

int
fix_partition_order (PedDisk *disk)
{
	if (strcmp(disk->type->name, "msdos")) {
		ped_exception_throw(PED_EXCEPTION_NO_FEATURE,
		                    PED_EXCEPTION_CANCEL,
		                    _("Fixing partition order on %s partition "
		                      "label type not implemented."),
		                    disk->type->name);
		return 0;
	}


	PedPartition *walk;
	int i = 0, j = 4;

	/* We check that order of the partitions is really wrong */
	for (walk = ped_disk_next_partition(disk,NULL); walk;
	     walk = ped_disk_next_partition(disk,walk)) {
		if (!ped_partition_is_active(walk))
			continue;
		if (walk->type & PED_PARTITION_LOGICAL) {
			j++;
			if (walk->num != j) {
				j = -1;
				break;
			}
		}
		else {
			i++;
			if (walk->num != i) {
				i = -1;
				break;
			}
		}
	}
	if (i != -1 && j != -1)
		return 1;

	/* We make sure that there are no mounted partitions */
	for (i = 1; i <= 4; i++) {
		walk = ped_disk_get_partition(disk,i);
		if (walk && !_partition_warn_busy(walk))
			return 0;
	}

	/* We will issue warning to the user for now */
	if (PED_EXCEPTION_CANCEL == ped_exception_throw(PED_EXCEPTION_WARNING,
							PED_EXCEPTION_IGNORE_CANCEL,
							"Fixing partition order "
							"is experimental. Use "
							"at your own risk."))
		return 0;

	struct DOSGeomList *partitions = NULL;
	struct DOSGeomList *pdata = NULL;
	struct DOSGeomList **ptr = &partitions;




	/* We save the partitions */
	for (walk = ped_disk_next_partition(disk,NULL); walk;
	     walk = ped_disk_next_partition(disk,walk)) {
		if (!ped_partition_is_active(walk))
			continue;

		/* We save the partition in the list */
		*ptr = malloc(sizeof(struct DOSGeomList));
		(*ptr)->geom = walk->geom;
		(*ptr)->part_data = malloc(DOS_DATA_SIZE);
		memcpy((*ptr)->part_data, walk->disk_specific, DOS_DATA_SIZE);
		(*ptr)->fs_type = walk->fs_type;
		(*ptr)->type = walk->type;
		(*ptr)->next = NULL;
		ptr = &((*ptr)->next);

	}

	/* We delete the partitions */
	for (i = 1; i <= 4; i++) {
		if ((walk = ped_disk_get_partition(disk,i)) != NULL)
			ped_disk_delete_partition (disk, walk);
	}

	/* FIXME: We assume this doesn't fail. Fix ! */
	/* We recreate the partitions again */
	pdata = partitions;
	while (pdata) {
		_fix_order_add_part(disk, pdata);
		pdata = pdata->next;
	}

	/* We clear the saved data */
	pdata = partitions;
	while (pdata) {
		struct DOSGeomList *next = pdata->next;
		free(pdata->part_data);
		free(pdata);
		pdata = next;
	}

	/* We're done */
	uiquery->need_commit = 1;
	return 1;
}


void
check_partition_consistency (PedPartition *part)
{
	int end_cyl,end_head,end_sector;
	PedSector tail = 0;
	char *part_chs = NULL;
	/* TODO: Calculate, instead of parsing the string */
	part_chs = ped_unit_format_custom(part->disk->dev,part->geom.end,
	                                  PED_UNIT_CHS);
	sscanf(part_chs, "%d,%d,%d", &end_cyl, &end_head, &end_sector);
	PED_FREE(part_chs);

	/* If, for example, this is a msdos logical partition, it has a
           tail of one sector, specifying the next partition. */
	tail = metadata_tail_sectors(part);

	/* We check that the partition ends at the cylinder boundary */
	if (end_head != part->disk->dev->bios_geom.heads - 1 - tail ||
	    end_sector != part->disk->dev->bios_geom.sectors - 1)
		ped_exception_throw(PED_EXCEPTION_WARNING, PED_EXCEPTION_OK,
			_("Partition %i does not end on cylinder boundary."),
			part->num);
}

void
verify_partition_table (PedDisk *disk)
{
	/* One sector is used for the master boot record */
	PedSector used_sectors = 1LL;
	PedPartition *part;

	/* We count the allocated sectors */
	for (part = ped_disk_next_partition(disk,NULL); part;
	     part = ped_disk_next_partition(disk,part)) {
		if (!ped_partition_is_active(part))
			continue;
		check_partition_consistency(part);

		/* We don't want to count the extended partition */
		if (part->type & PED_PARTITION_EXTENDED)
			continue;

		used_sectors += part->geom.length;
		/* The logical partitions take an additional sector */
		if (part->type & PED_PARTITION_LOGICAL)
			used_sectors++;
	}

	/* If the used sectors are more than the available, issue a warning */
	if (used_sectors > disk->dev->length)
		ped_exception_throw(PED_EXCEPTION_WARNING, PED_EXCEPTION_OK,
				_("Total allocated sectors %lld greater than "
				"the maximum %lld"),
				used_sectors, disk->dev->length);
	else if (used_sectors < disk->dev->length)
		ped_exception_throw(PED_EXCEPTION_INFORMATION, PED_EXCEPTION_OK,
		                    _("%lld unallocated sectors"),
		                    disk->dev->length - used_sectors);
}

char*
partition_print_flags (PedPartition* part)
{
        PedPartitionFlag        flag;
        int                     first_flag;
        const char*             name;
        char*                   res = ped_malloc(1);
        void*                   _res = res;

        *res = '\0';

        first_flag = 1;
        for (flag = ped_partition_flag_next (0); flag;
             flag = ped_partition_flag_next (flag)) {
                if (ped_partition_get_flag (part, flag)) {
                        if (first_flag)
                                first_flag = 0;
                        else {
                                _res = res;
                                ped_realloc (&_res, strlen (res)
                                                           + 1 + 2);
                                res = _res;
                                strncat (res, ", ", 2);
                        }

                        name = _(ped_partition_flag_get_name (flag));
                        _res = res;
                        ped_realloc (&_res, strlen (res) + 1
                                                   + strlen (name));
                        res = _res;
                        strncat (res, name, 21);
                }
        }

        return res;
}


/* This is the function that sets UICalls */
void
set_uicalls(UICalls *uiq) {
	uiquery = uiq;
	/* TODO: Maybe we do everything else like this */
	if (uiquery->getparttype)
		get_part_type = uiquery->getparttype;
}


PedPartition *
part_list_prev (PedPartition *part, PedPartitionType skip)
{
	if (!part)
		return NULL;
	do {
		part = part->prev;
	} while (part && (part->type & skip));
	return part;
}

PedPartition *
part_list_next (PedPartition *part, PedPartitionType skip)
{
	if (!part)
		return NULL;
	do {
		part = part->next;
	} while (part && (part->type & skip));
	return part;
}


static void
_print_flag (int *i, const char *name) {
	printf(*i == 0 ? "%s" : ",%s", name);
	(*i)++;
}


void
print_partition_types() {
	PedFileSystemType *walk;
	char buf[SMALLBUF];
	for (walk = ped_file_system_type_get_next(NULL); walk; walk = ped_file_system_type_get_next(walk)) {
		int i = 0;
		printf ("%-15s",walk->name);
		if (walk->ops->open) _print_flag(&i,_("open"));
		if (walk->ops->create) _print_flag(&i,_("create"));
		if (walk->ops->check) _print_flag(&i,_("check"));
		if (walk->ops->copy) _print_flag(&i,_("copy"));
		if (walk->ops->resize) _print_flag(&i,_("resize"));
		printf("\n");
	}
	exit(0);
}


int count_logical_partition(PedDisk *disk)
{
	int count = 0;
	PedPartition *walk;
	for (walk = ped_disk_next_partition (disk, NULL); walk;
	     walk = ped_disk_next_partition (disk, walk)) {
		if (walk->type & (PED_PARTITION_FREESPACE | PED_PARTITION_METADATA))
			continue;
		if (walk->type & PED_PARTITION_LOGICAL)
			count++;
	}
	return count;
}
