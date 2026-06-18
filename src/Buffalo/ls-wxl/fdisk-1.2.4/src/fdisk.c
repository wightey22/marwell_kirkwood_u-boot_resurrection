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

#if HAVE_CONFIG_H
# include "../config.h"
#endif

#include "common.h"
#include "hacks.h"
#include "command.h"
#include "ui.h"

/* From gettextize */
#include "gettext.h"

#if HAVE_LOCALE_H
# include <locale.h>
#endif /* HAVE_LOCALE_H */

#define N_(String) String
#define _(String) dgettext (PACKAGE, String)

#include <parted/parted.h>
#include <parted/debug.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif

#include <getopt.h>

#if HAVE_READLINE && HAVE_LIBTERMCAP
/* This function is defined in ui.c */
extern void init_termcap_library(void);
#endif /* HAVE_READLINE && HAVE_LIBTERMCAP */

/* Christian <mail.kristian@yahoo.it>
 *
 * This is defined in `common.c' and used from common function
 * to say if compatible mode should be used.
 *
 * A value of 0 mean that extended mode should be used.
 * A value not equal to zero mean that extensions must be excluded 
 *
 * Future changes can make labels for various levels of compatibility */
extern int compat_mode;

/* minimum amount of free space to leave, or maximum amount to gobble up,
 * depending on your POV ;)
 */
#define MIN_FREESPACE		(1000 * 2)	/* 1000k */

#define MEGABYTE_SECTORS (PED_MEGABYTE_SIZE / PED_SECTOR_SIZE_DEFAULT)


#define do_list_devices fdisk_do_list_devices

typedef struct {
	time_t	last_update;
	time_t	predicted_time_left;
} TimerContext;


/* I have created this struct to store options (in short and long form)
 * and help in the same variable. The struct option array, wich is
 * needed for getopt_long is build dynamically by build_option_table()
 * using this array. We can add options here there isn't problems
 * with getopt_long() */
struct {
	char *id;
	int has_arg;
	int *flag;
	int val;
	char *help;
} extd_options[] = {
  /* name   has_arg      flag  val  help */
  { "help", no_argument, NULL, 'h', "displays this help message"},
  { "list", no_argument, NULL, 'l', "list partition table(s)"},
  { "raw-print", no_argument, NULL, 'r', "show the raw data in the partition table(s)"},
  { "size", required_argument, NULL, 's', "show partition size"},
#ifdef GNU_EXT
  {	"linux-fdisk", no_argument, NULL, 'L', "enable Linux fdisk compatibility mode"},
  { "gnu-fdisk", no_argument, NULL, 'G', "disable Linux fdisk compatibility mode"},
#endif /* GNU_EXT */
  { "interactive", no_argument, NULL, 'i', "where necessary, prompts for user intervention"},
  { "script", no_argument, NULL, 'p', "never prompts for user intervention"},
  { "sector-units", no_argument, NULL, 'u', "use sectors instead of cylinder as a default unit"},
  { "sector-size", required_argument, NULL, 'b', "specify the sector size in bytes"},
  { "cilyndres", required_argument, NULL, 'C', "specify the number of cylinders, actually does nothing"},
  { "heads", required_argument, NULL, 'H', "in lfdisk, specify the number of heads of the disk"},
  { "sectors", required_argument, NULL, 'S', "in lfdisk, specify the number of sectors per track"},
  { "list-partition-types", no_argument, NULL, 't', "displays a list of supported partition types"},
  { "version", no_argument, NULL, 'v', "displays the version"},
  
  /* Last element must be NULL */
  { NULL, 0, NULL, 0, NULL}
};


/* String to use whit getopt() or getopt_long()
 *
 * if you update extd_options[] you must also add option in short form
 * to this string 
 *
 * NOTE: I will make function to build dynamic in the future */
static const char* short_option_string = "hlipvLGs:utb:C:H:S:r";



/* #ifdef GNU_EXT */
/* int fdisk_compatibility_mode = 0; */
/* #else */
/* # define compat_mode 1 */
/* #endif */

int fdisk_opt_script_mode;
int fdisk_list_table = 0;
int fdisk_print_raw = 0;

/* Used for -s option, NULL when there was no -s option */
const char *fdisk_partsize_device = NULL;
int fdisk_partsize_part = 0;

int user_cyls = 0, user_sectors = 0, user_heads = 0, user_sectsize = 0;

static char* number_msg = N_(
"NUMBER is the partition number used by Linux.  On MS-DOS disk labels, the "
"primary partitions number from 1 to 4, logical partitions from 5 onwards.\n");

static char* label_type_msg_start = N_("LABEL-TYPE is one of: ");
static char* flag_msg_start =	N_("FLAG is one of: ");
static char* unit_msg_start =	N_("UNIT is one of: ");
static char* part_type_msg =	N_("PART-TYPE is one of: primary, logical, "
			           "extended\n");
static char* fs_type_msg_start = N_("FS-TYPE is one of: ");
static char* start_end_msg =	N_("START and END are disk locations, such as "
		"4GB or 10%.  Negative values count from the end of the disk.  "
                "For example, -1s specifies exactly the last sector.\n");
static char* state_msg =	N_("STATE is one of: on, off\n");
static char* device_msg =	N_("DEVICE is usually /dev/hda or /dev/sda\n");
static char* name_msg =		N_("NAME is any word you want\n");
static char* resize_msg_start = N_("The partition must have one of the "
				   "following FS-TYPEs: ");

static char* label_type_msg;
static char* flag_msg;
static char* unit_msg;

static char* mkfs_fs_type_msg;
static char* mkpart_fs_type_msg;
static char* resize_fs_type_msg;

//static PedTimer* timer;
static TimerContext timer_context;
static FdiskCommand* fdisk_main_menu_commands[256] = {NULL};
static FdiskCommand* fdisk_ex_menu_commands[256] = {NULL};
static FdiskCommand* fdisk_bsd_menu_commands[256] = {NULL};
static int in_menu = 0;

/* 0 = Disk was not altered.
   1 = Disk was altered. */
//static int need_commit = 0;

/* UI Calls structure */
static UICalls uiquery;

/* 0 = Sectors are the default unit.
   1 = Cylinders are the default unit (Default).*/
static int cylinder_unit = 1;

/* When editing a BSD label, we remember the number of logical partitions on
   the original disk, so we display a more plausible device name */
static int logical_offset = 0;

static void _done (PedDevice* dev);

/* Timer handler and other UI functions */
static void
_timer_handler (PedTimer* timer, void* context)
{
	TimerContext*	tcontext = (TimerContext*) context;
	int		draw_this_time;

	if (fdisk_opt_script_mode || !isatty(fileno(stdout)))
		return;

	if (tcontext->last_update != timer->now && timer->now > timer->start) {
		tcontext->predicted_time_left
			= timer->predicted_end - timer->now;
		tcontext->last_update = timer->now;
		draw_this_time = 1;
	} else {
		draw_this_time = 0;
	}

	if (draw_this_time) {
		fdisk_wipe_line ();

		if (timer->state_name)
			printf ("%s... ", timer->state_name);
		printf (_("%0.f%%\t(time left %.2ld:%.2ld)"),
			100.0 * timer->frac,
			tcontext->predicted_time_left / 60,
			tcontext->predicted_time_left % 60);

		fflush (stdout);
	}
}

static int
getstring (const char* prompt, char** value, const StrList* words,
	   const StrList* locwords, int multi_word)
{
	char*		def_str = NULL;
	char*		input;
	StrList*	valid = NULL;
	const StrList*	walk;
	const StrList*	locwalk;
	char*		fix_result = NULL;

	/* TODO: Add a function that does just this to strlist.c */
	valid = str_list_join (str_list_duplicate(words),
	                       str_list_duplicate(locwords));

	if (*value) def_str = strdup(*value);
	input = fdisk_command_line_get_word (prompt, def_str,
				             valid, multi_word);

	str_list_destroy(valid);
	if (def_str) free(def_str);

	/* If the user has choosen a localised string, we should return the
	   non-localized one, corresponding to it */
	for (walk = words, locwalk = locwords; walk && locwalk;
	     walk = walk->next, locwalk = locwalk->next)
	{
		/* If it matches a non-localised string, we are happy */
		if (str_list_match_node(walk, input)) {
			if (fix_result)
				free(fix_result);
			fix_result = NULL;
			break;
		}
		/* If it matches a localised string, we save the non-localised
		   one, but we don't break */
		if (!fix_result && str_list_match_node(locwalk, input)) {
			fix_result = str_list_convert_node(walk);
		}
	}
	if (fix_result) {
		free(input);
		*value = fix_result;
	}
	else
		*value = input;
	return 1;
}

/* We might as well get rid of this one */
static int
getbool (const char* prompt, int* value)
{
	*value = command_line_prompt_boolean_question (prompt);
	return 1;
}
/* We don't need this one
static int
(*getint) (const char* prompt, int* value) = fdisk_command_line_get_integer;
*/

#if 0
static int
_partition_warn_busy (PedPartition* part)
{
	char* path = ped_partition_get_path (part);

	if (ped_partition_is_busy (part)) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR,
			PED_EXCEPTION_CANCEL,
			_("Partition %s is being used.  You must unmount it "
			  "before you modify it with Parted."),
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
	MOVE_NO		= 0,
	MOVE_STILL	= 1,
	MOVE_UP		= 2,
	MOVE_DOWN	= 4
} EMoves;

enum { /* Don't change these values */
	SECT_START	=  0,
	SECT_END	= -1
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
	}

	*dist = ( move == MOVE_DOWN ? down_dist :
	        ( move == MOVE_UP   ? up_dist   :
		  0 ) );
	return move;
}

/* Snaps a partition to nearby partition boundaries.  This is useful for
 * gobbling up small amounts of free space, and also for reinterpreting small
 * changes to a partition as non-changes (eg: perhaps the user only wanted to
 * resize the end of a partition).
 * 	Note that this isn't the end of the story... this function is
 * always called before the constraint solver kicks in.  So you don't need to
 * worry too much about inadvertantly creating overlapping partitions, etc.
 */
static void
snap_to_boundaries (PedGeometry* new_geom, PedGeometry* old_geom,
		    PedDisk* disk,
		    PedGeometry* start_range, PedGeometry* end_range)
{
	PedPartition*	start_part;
	PedPartition*	end_part;
	PedSector	start = new_geom->start;
	PedSector	end = new_geom->end;
	PedSector	start_dist = -1, end_dist = -1;
	EMoves		start_allow, end_allow, start_want, end_want;
	int		adjacent;

	start_want = end_want = MOVE_NO;
	start_allow = end_allow = MOVE_STILL | MOVE_UP | MOVE_DOWN;

	start_part = ped_disk_get_partition_by_sector (disk, start);
	end_part = ped_disk_get_partition_by_sector (disk, end);
	adjacent = (start_part->geom.end + 1 == end_part->geom.start);

	/* If we can snap to old_geom, then we will... */
	/* and this will enforce the snaped positions  */
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
	/* the farest to do so. */
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
	PED_ASSERT (start <= end, return);
	ped_geometry_set (new_geom, start, end - start + 1);
}

/* This functions constructs a constraint from the following information:
 * 	start, is_start_exact, end, is_end_exact.
 *
 * If is_start_exact == 1, then the constraint requires start be as given in
 * "start".  Otherwise, the constraint does not set any requirements on the
 * start.
 */
static PedConstraint*
constraint_from_start_end (PedDevice* dev, PedGeometry* range_start,
                           PedGeometry* range_end)
{
	return ped_constraint_new (ped_alignment_any, ped_alignment_any,
		range_start, range_end, 1, dev->length);
}

static PedConstraint*
constraint_intersect_and_destroy (PedConstraint* a, PedConstraint* b)
{
	PedConstraint* result = ped_constraint_intersect (a, b);
	ped_constraint_destroy (a);
	ped_constraint_destroy (b);
	return result;
}
#endif
static void
help_on (char* topic)
{
        FdiskCommand*	cmd;

        cmd = fdisk_command_get (fdisk_main_menu_commands, topic);
        if (!cmd) return;

        fdisk_command_print_help (cmd);
}


static int
_disk_reread(PedDisk **disk) {

	PedDevice *dev = (*disk)->dev;

	ped_disk_destroy(*disk);

	*disk = ped_disk_new(dev);
	uiquery.need_commit = 0;

}

static int
do_check (PedDisk** disk)
{
	return perform_check(*disk,NULL);
}

static int
do_cp (PedDisk** disk)
{
	return perform_cp(*disk,NULL,UI_WARN_COMMIT);
}

void
fdisk_print_options_help ()
{
	unsigned int i;
	for(i=0; extd_options[i].id != NULL; i++)
		printf("  -%c, --%-23.23s %s\n", extd_options[i].val, extd_options[i].id, _(extd_options[i].help));
}

void
fdisk_print_commands_help (FdiskCommand* cmds[])
{
	 int		i;

	if (cmds) {
		for (i=0; cmds [i]; i++)
			fdisk_command_print_summary (cmds [i]);
	} else {
		if (in_menu == 0) {
			for (i=0; fdisk_main_menu_commands [i]; i++)
				fdisk_command_print_summary (fdisk_main_menu_commands [i]);
		}
		else if (in_menu == 1) {
			for (i=0; fdisk_ex_menu_commands [i]; i++)
				fdisk_command_print_summary (fdisk_ex_menu_commands [i]);
    		}
		else if (in_menu == 2) {
			for (i=0; fdisk_bsd_menu_commands [i]; i++)
				fdisk_command_print_summary (fdisk_bsd_menu_commands [i]);
    		}
	}
}

static int
do_help(PedDisk** disk)
{
	puts(_("Command action"));
	if (in_menu == 0)
		fdisk_print_commands_help(fdisk_main_menu_commands);
	else if (in_menu == 1)
		fdisk_print_commands_help(fdisk_ex_menu_commands);
	else if (in_menu == 2)
	        fdisk_print_commands_help(fdisk_bsd_menu_commands);
	return 1;
}

static int
do_msdos_mklabel (PedDisk** disk)
{
	PedDevice               *dev = (*disk)->dev; /* Save the address of dev,
							because we are going to
							destroy disk. */
	const PedDiskType*	type = ped_disk_type_get ("msdos");

	return perform_mklabel(dev,disk,type);
}

static int
do_sun_mklabel (PedDisk** disk)
{
	PedDevice               *dev = (*disk)->dev; /* Save the address of dev,
							because we are going to
							destroy disk. */
	const PedDiskType*	type = ped_disk_type_get ("sun");

		return perform_mklabel(dev,disk,type);
}

static int
do_dvh_mklabel (PedDisk** disk)
{
	PedDevice               *dev = (*disk)->dev; /* Save the address of dev,
							because we are going to
							destroy disk. */
	const PedDiskType*	type = ped_disk_type_get ("dvh");

	return perform_mklabel(dev,disk,type);
}

static int
do_mklabel (PedDisk** disk)
{
	PedDevice               *dev = (*disk)->dev; /* Save the address of dev,
							because we are going to
							destroy disk. */
	const PedDiskType*	type = ped_disk_probe (dev);


	return perform_mklabel(dev,disk,type);
}

static int
do_mkfs (PedDisk** disk)
{

	return perform_mkfs(*disk,NULL,NULL,UI_WARN_COMMIT);
}

static int
do_mkpart (PedDisk **disk)
{
	PartPos			 pos;
	PedPartition* 		 part;
	PedPartitionType	 part_type = 0;
	const PedFileSystemType	*fs_type = ped_file_system_type_get ("ext2");
	char*			 peek_word;
	UIOpts			 opts = UI_CUSTOM_VALUES;




	if (!get_part_type (_("Partition type"), *disk,
					       &part_type))
		return 0;

	peek_word = fdisk_command_line_peek_word ();
	if (part_type == PED_PARTITION_EXTENDED
	    || (peek_word && isdigit (peek_word[0]))) {
		fs_type = NULL;
	}
	/* gfdisk should ask for filesystem, lfdisk should not */
	else if (!compat_mode) {
		if (!get_fs_type (_("File system type"),
					       &fs_type, 0))
			return 0;
	}
	if (peek_word)
		PED_FREE (peek_word);


	pos.start.sector = 0LL;
	pos.end.sector = 0LL;
	pos.start.range = NULL;
	pos.end.range = NULL;

	/*
	 * Christian <mail.kristian@yahoo.it> bug-fixed:
	 *
	 * (gdb) c
	 * Continuing.
	 * Partition type
	 *   e   extended
	 *   p   primary partition (1-4)
	 * e
	 * 
	 * Program received signal SIGSEGV, Segmentation fault.
	 * 0x0804bb3f in do_mkpart (disk=0xbf8c4150) at fdisk.c:673
	 * 673             if (!fdisk_compatibility_mode && fs_type->ops->create &&
	 */

	/* if (!fdisk_compatibility_mode && fs_typ->ops->create && */

	if (!compat_mode && fs_type && fs_type->ops->create &&
	    part_type != PED_PARTITION_EXTENDED &&
	    command_line_prompt_boolean_question(
		_("Do you want to create the filesystem on the partition?")))
	{
	  return perform_mkpartfs (*disk, &pos, part_type, fs_type,
				   NULL, opts | UI_WARN_COMMIT);
	}
	else
	{
	  return perform_mkpart (*disk, &pos, part_type, fs_type,
				 NULL, opts);
	}
}

#if 0
static int
do_mkpartfs (PedDisk** disk)
{
	PartPos pos;

	pos.start.sector = 0LL;
	pos.end.sector = 0LL;
	pos.start.range = NULL;
	pos.end.range = NULL;
	return perform_mkpartfs (*disk, &pos, 0, NULL, NULL,
	                         UI_CUSTOM_VALUES | UI_WARN_COMMIT |
	                         UI_SPECIFY_PART_TYPE);
}
#endif

static int
do_move (PedDisk** disk)
{
	PartPos pos;
	/* TODO: Make default position more friendly */
	pos.start.sector = 0LL;
	pos.end.sector = 0LL;
	pos.start.range = NULL;
	pos.end.range = NULL;
	if(!perform_move (*disk, NULL, &pos, UI_CUSTOM_VALUES|UI_WARN_COMMIT)) {
		ped_exception_throw(PED_EXCEPTION_ERROR,
		                    PED_EXCEPTION_CANCEL,
		         _("Partition move failed"));
		if (uiquery.need_commit)
				_disk_reread(disk);
		return 0;
	}
	return 1;
}

static int
do_name (PedDisk** disk)
{
	return perform_name(*disk, NULL, NULL);
}

/* TODO: REMOVE */
#if 0
static void
partition_print_flags (PedPartition* part)
{
	PedPartitionFlag	flag;
	int			first_flag;

	first_flag = 1;
	for (flag = ped_partition_flag_next (0); flag;
	     flag = ped_partition_flag_next (flag)) {
		if (ped_partition_get_flag (part, flag)) {
			if (first_flag)
				first_flag = 0;
			else
				printf (", ");
			printf (_(ped_partition_flag_get_name (flag)));
		}
	}
}
#endif

/* Prints a sector out, first in compact form, and then with a percentage.
 * Eg: 32Gb (40%)
 */
static void
print_sector_compact_and_percent (PedSector sector, PedDevice* dev)
{
	char* compact;
	char* percent;

	if (ped_unit_get_default() == PED_UNIT_PERCENT)
		compact = ped_unit_format (dev, sector);
	else
		compact = ped_unit_format_custom (dev, sector,
						  PED_UNIT_COMPACT);

	percent = ped_unit_format_custom (dev, sector, PED_UNIT_PERCENT);

	printf ("%s (%s)\n", compact, percent);

	PED_FREE (compact);
	PED_FREE (percent);
}

static int
partition_print (PedPartition* part)
{
	PedFileSystem*	fs;
	PedConstraint*	resize_constraint;

	fs = ped_file_system_open (&part->geom);
	if (!fs)
		return 1;

        printf ("\n");

	printf (_("Minor: %d\n"), part->num);
	printf (_("Flags: ")); partition_print_flags (part); printf("\n");
	printf (_("File System: %s\n"), fs->type->name);
	printf (_("Size:         "));
	print_sector_compact_and_percent (part->geom.length, part->geom.dev);

	resize_constraint = ped_file_system_get_resize_constraint (fs);
	if (resize_constraint) {
		printf (_("Minimum size: "));
		print_sector_compact_and_percent (resize_constraint->min_size,
			part->geom.dev);
		printf (_("Maximum size: "));
		print_sector_compact_and_percent (resize_constraint->max_size,
			part->geom.dev);
		ped_constraint_destroy (resize_constraint);
	}

        printf ("\n");

	ped_file_system_close (fs);
	return 1;
}

static int
do_quit (PedDisk** disk)
{

	_done ((*disk)->dev);

	ped_disk_destroy (*disk);

        exit (0);
}

/* FIXME: This is ugly. Won't always work. And it is untested. */
static int
do_edit_bsd_disklabel (PedDisk** disk)
{

	if (uiquery.need_commit) {
	        ped_exception_throw (
				     PED_EXCEPTION_ERROR,
				     PED_EXCEPTION_CANCEL,
				     _("Editing a BSD label, before writing the msdos "
			               "partition table is not supported in GNU fdisk."));
		return 0;
	}

	PedPartition *part;
	int sect_size = (*disk)->dev->sector_size;

	/* We enable external access to the device */
	if (!ped_device_begin_external_access((*disk)->dev))
		return 0;

	/* We check if there is a BSD disklabel */
	for (part = ped_disk_next_partition(*disk,NULL); part;
	     part = ped_disk_next_partition(*disk,part))
	{
		if (part->type)
			continue;
		if (is_part_type_bsd(part)) {
			/* Warn if there is a filesystem on the partition */
			if (ped_file_system_probe(&(part->geom))) {
				if (ped_exception_throw(PED_EXCEPTION_WARNING,
				                        PED_EXCEPTION_YES_NO,
				     _("There is a BSD partition on the disk "
				       "but it seems to contain a filesystem. "
				       "This is going to destroy it. "
			               "Are you sure you want to continue?"))
					== PED_EXCEPTION_NO) {
					ped_device_end_external_access((*disk)->dev);
					return 0;
				}
			}
			/* If there is no already created BSD label created,
			   and there is a filesystem on the partition, OR
			   we are in gfdisk, we warn the user about it */
			else if (!is_bsd_partition((*disk)->dev->path,
				part->geom.start * sect_size, sect_size) &&
				(!compat_mode)) {
				if (ped_exception_throw(PED_EXCEPTION_WARNING,
				                        PED_EXCEPTION_YES_NO,
				     _("There is a BSD partition on the disk, "
				       "but there is no BSD disklabel on it. "
				       "Do you want to create one?"))
					== PED_EXCEPTION_NO) {
					ped_device_end_external_access((*disk)->dev);
					return 0;
				}
			}
			break;
		}
	}
	/* FIXME: And here comes our biggest problem... */
	if (part) {
		PedDevice *label_dev;
		PedDisk *label_disk;
		char label_path[1024];
		get_partition_device(label_path, sizeof(label_path), part, 0);
		label_dev = ped_device_get(label_path);
		if (!label_dev || ! ped_device_open(label_dev)) {
			printf(_("There was an error opening the *BSD "
			         "partition on %s.\n"), (*disk)->dev->path);
			ped_device_end_external_access((*disk)->dev);
			return 0;
		}
		printf("Reading disklabel of %s at sector %lld.\n",
			(*disk)->dev->path, part->geom.start);
		ped_exception_fetch_all ();
		label_disk = ped_disk_new(label_dev);
		/* FIXME: This might ruin a partition on the disk */
		if (!label_disk) ped_exception_catch();
		ped_exception_leave_all();
		if (!label_disk || strcmp(label_disk->type->name, "bsd")) {
			label_disk = ped_disk_new_fresh (label_dev,
					ped_disk_type_get ("bsd"));
		}
		in_menu = 2;
		logical_offset = count_logical_partition(*disk);
		fdisk_interactive_menu(&label_disk, fdisk_bsd_menu_commands, 2);
		ped_disk_destroy(label_disk);
		ped_device_destroy(label_dev);
		logical_offset = 0;
		in_menu = 0;
	} else {
		printf(_("There is no *BSD partition on %s.\n"),
			(*disk)->dev->path);
		ped_device_end_external_access((*disk)->dev);
		return 0;
	}

	ped_device_end_external_access((*disk)->dev);
	uiquery.need_commit = 0;
	return 1;

}
static int
do_print (PedDisk **disk)
{
	char			buf[256];
	PedPartition*		part;
	int			has_extended;
	int                     sect_size;
	char* 			cyl_size;
	PedCHSGeometry* 	chs;
	PedSector               heads;
	PedSector               sectors;
	PedSector               cylinders;
	PedSector		start;
	PedSector		end;
	PedSector		cyl_start;
	PedSector		cyl_end;
	PedSector               unit = ped_unit_get_default ();
	PedSector		blocks;
	PedSector		sects_nbytes;

	chs          = &((*disk)->dev)->bios_geom;
	heads        = chs->heads;
	sectors      = chs->sectors;
	cylinders    = chs->cylinders;
	sect_size    = (*disk)->dev->sector_size;
	sects_nbytes = heads * sectors * sect_size;

	cyl_size  = ped_unit_format_custom ((*disk)->dev,
				heads * sectors,
				PED_UNIT_BYTE);
	cyl_size[strlen (cyl_size) - 1] = '\0';

	PedSector total_drive_size = heads * sectors * cylinders * sect_size;
	printf (_("\nDisk %s: %lld %s, %lld bytes\n"), (*disk)->dev->path,
		 total_drive_size >= 1000000000 ?
		 total_drive_size  / 1000000000 :
		 total_drive_size  / 1000000,
		(total_drive_size >= 1000000000) ? "GB" : "MB",
		heads * sectors * cylinders * sect_size);

	if (unit == PED_UNIT_SECTOR)
		printf (_("%lld heads, %lld sectors/track, %lld cylinders, total %lld sectors\n"
		  	"Units = sectors of %d * %d = %d bytes\n"),
			heads, sectors, cylinders, (heads * sectors * cylinders), 1,
			sect_size, sect_size);
	else
		printf (_("%lld heads, %lld sectors/track, %lld cylinders\n"
		  	"Units = cylinders of %lld * %d = %s bytes\n"),
			heads, sectors, cylinders, (heads * sectors),
			sect_size, cyl_size);

        printf ("\n");

	has_extended = ped_disk_type_check_feature ((*disk)->type,
			       		 PED_DISK_TYPE_EXTENDED);

  	unsigned int i;

	unsigned int pathlen;

	/* TODO: Make this output the same output as util-linux fdisk(?),
	   possibly split into seperate functions */
	if (compat_mode && !strcmp((*disk)->type->name,"bsd")) {
		printf ("%s %10s %11s %11s %4s %7s ", _("#   "),
		      _("start"), _("end"), _("blocks"), _("id"), _("system"));
		printf ("\n");
	}
	else {
		pathlen  = strlen ((*disk)->dev->path);
		unsigned int fix = 3 - !isdigit((*disk)->dev->path[pathlen-1]);
		pathlen += fix;
		for (i = 0; i < (pathlen - 5 - fix); i++)
  		printf (" ");
		printf ("%s %s %10s %11s %11s %4s %7s ", _("Device"),
			 _("Boot"), _("Start"), _("End"), _("Blocks"), _("Id"),
			 _("System"));
	printf ("\n");
	}
	PedSector total_cyl = heads * sectors;

	for (part = ped_disk_next_partition (*disk, NULL); part;
	     part = ped_disk_next_partition (*disk, part)) {


		cyl_start     = (part->geom.start / (total_cyl)) + 1;
		cyl_end       = (part->geom.end   / (total_cyl)) + 1;

	  	if (unit == PED_UNIT_SECTOR) {
			/* In Linux fdisk compatibility mode, display exact */
			if (compat_mode) {
				start = part->geom.start;
				end = part->geom.end;
			}
			/* In fdisk, round up the sectors to cylinders */
			else {
				start = (part->geom.start / total_cyl
				    		* total_cyl) + sectors;
		        	end   = part->geom.end / total_cyl * total_cyl;
			}
			blocks    = ((cyl_end * total_cyl)
						- ((part->num == 1) ?
			(cyl_start * sectors) : (cyl_start * total_cyl)))
					/ (1024 / sect_size);
		} else {
		  	start     = cyl_start;
			end       = cyl_end;
			blocks    = ((end * total_cyl) - ((part->num == 1) ?
			(start * sectors) : (start * total_cyl))) / (1024 / sect_size);
		}

		if (!ped_partition_is_active (part))
	       		continue;


		if (compat_mode && !strcmp((*disk)->type->name,"bsd")) {
			printf("  %c: ", 'a' + part->num - 1);
		}
		else {
			get_partition_device(buf, sizeof(buf), part, logical_offset);
			printf("%2$-*1$.*1$s", pathlen, buf);

		if (ped_partition_get_flag(part,PED_PARTITION_BOOT))
			printf("  *  ");
		else
			printf("     ");

		}


		printf ("%10lld %11lld %11lld ", start, end, blocks);

		int type_size;
		unsigned int part_type = get_disk_specific_system_type (part, &type_size);
		type_size *= 2;
		char *type_name = _(get_disk_specific_system_name(part,0));
		if (type_size)
			printf("  %*x  %s", type_size, part_type, type_name);
		else if(type_name)
			printf("  %s", type_name);
		/* FIXME: This should not be here anymore */
		else {
			ped_device_begin_external_access((*disk)->dev);
			char *type = (char *)ped_partition_type_get_name (part->type);
			if (part->fs_type
				&& !strncmp (part->fs_type->name, "linux-swap", 10))
				printf ("%4s %21s ", _("82"), _("Linux Swap / Solaris"));
			else if (is_bsd_partition ((*disk)->dev->path,
				part->geom.start * sect_size, sect_size))
				printf ("%4s %17s ", _("a5"), _("Free/Net/OpenBSD"));
			else if (!strcmp (type, "primary"))
				printf ("%4s %6s ", _("83"), _("Linux"));
			else if (!strcmp (type, "extended"))
				printf ("%4s %9s ", _("5"), _("Extended"));
			ped_device_end_external_access((*disk)->dev);
		}
		printf ("\n");
		start = end = blocks = 0;
		/* At the end we check the partition consistency for lfdisk */
		if (compat_mode)
			check_partition_consistency(part);
	}
	PED_FREE (cyl_size);

	/*if (fdisk_list_table == 1)
	  do_quit (disk);*/

	return 1;

error:
	return 0;
}

#define sector(s)	((s) & 0x3f)
#define cylinder(s, c)	((c) | (((s) & 0xc0) << 2))


void
part_xprint (PedDisk **disk, int extend)
{
	PedPartition *part;
	PedDevice *dev = (*disk)->dev;
	PedCHSGeometry *chs = &(dev->bios_geom);

	int is_boot, start_cyl, start_head, start_sector,
	             end_cyl, end_head, end_sector;

	char *part_chs = NULL;

	printf(_("\nDisk %s: %d heads, %d sectors, %d cylinders\n\n"),
	       dev->path, chs->heads, chs->sectors, chs->cylinders);
	printf(_("Nr AF  Hd Sec     Cyl  Hd Sec     Cyl      Start       Size ID\n"));

	for (part = ped_disk_next_partition (*disk, NULL); part;
	     part = ped_disk_next_partition (*disk, part)) {
		if (!ped_partition_is_active(part))
			continue;

		/* We skip primary partitions, if needed */
		if (extend && !part->type)
			continue;


		/* TODO: Calculate, instead of parsing the string */
		part_chs = ped_unit_format_custom(dev, part->geom.start,
	                                          PED_UNIT_CHS);
		sscanf(part_chs, "%d,%d,%d", &start_cyl, &start_head, &start_sector);
		PED_FREE (part_chs);

		part_chs = ped_unit_format_custom(dev, part->geom.end,
	                                          PED_UNIT_CHS);
		sscanf(part_chs, "%d,%d,%d", &end_cyl, &end_head, &end_sector);
		PED_FREE (part_chs);


		is_boot = ped_partition_get_flag(part,PED_PARTITION_BOOT);

		printf("%2d %02x%4d%4d%8d%4d%4d%8d%11lld%11lld %02x\n",
		       part->num, is_boot ? 0x80 : 0,
		       start_head, start_sector, start_cyl,
		       end_head, end_sector, end_cyl,
		       part->geom.start, part->geom.length,
		       get_disk_specific_system_type(part, NULL));


		check_partition_consistency(part);
	}
}

/* This is only run in Linux fdisk compatibility mode */
static int
do_xprint (PedDisk **disk)
{
	part_xprint(disk,0);
	return 1;
}

static int
do_eprint (PedDisk **disk)
{
	part_xprint(disk,1);
	return 1;
}

/* This prints a sector, in the style Linux fdisk prints raw partition
   tables, in hex, with 16 bytes per line */
static void
print_sector (PedDevice *dev, PedSector sector)
{
	char *buf;
	int size = dev->sector_size, i, pos = 0;
	buf = malloc(size);
	if (ped_device_read(dev, buf, sector, 1)) {
		for (i = 0; i < size; i++) {
			/* We print the line prefix */
			if (pos == 0)
				printf("0x%03X:", i);

			printf(" %02hhX", buf[i]);

			/* We print the line suffix */
			if (pos == 15) {
				printf("\n");
				pos = 0;
			}
			else
				pos++;
		}
		if (pos != 0)
			printf("\n");
		printf("\n");
	}
	free(buf);
}

static int
do_raw (PedDisk **disk)
{
	/* TODO: Make this for other disklabels, too */
	if (strcmp((*disk)->type->name, "msdos")) {
		fprintf(stderr,
		        _("Raw printing on %s not supported\n"),
		        (*disk)->type->name);
		return 0;
	}

	printf(_("Device: %s\n"), (*disk)->dev->path);

	/* This is for DOS partition table */
	PedPartition *part;
	int i;

	/* First we print the MBR */
	print_sector((*disk)->dev,0);

	/* Then we print the partition table sectors for the logical partitions */
	for (part = ped_disk_get_partition(*disk, i = 5); part;
	     part = ped_disk_get_partition(*disk, ++i))
	{
		print_sector((*disk)->dev, part->prev->geom.start);
	}

}

void
fdisk_print_partition_size(PedDisk *disk)
{
	PedPartition *part = ped_disk_get_partition(disk,fdisk_partsize_part);
	if (!part) {
		printf(_("There is no partition %d on %s\n"), fdisk_partsize_part, disk->dev->path);
		exit(1);
	}
	else {
		PedCHSGeometry* 	chs;
		PedSector               sectors;
		PedSector		cyl_size;
		PedSector		cyl_start;
		PedSector		cyl_end;
		PedSector		sect_size;
		PedSector		heads;
		PedSector		total_cyl;

		chs          = &(disk->dev)->bios_geom;
		sectors      = chs->sectors;
		heads        = chs->heads;
		total_cyl    = heads * sectors;
		sect_size    = disk->dev->sector_size;
		cyl_start     = (part->geom.start / (total_cyl)) + 1;
		cyl_end       = (part->geom.end   / (total_cyl)) + 1;
		/* ped_unit_set_default(PED_UNIT_CYLINDER); */

		printf("%lld\n",
			((cyl_end * total_cyl) - ((part->num == 1) ?
			(cyl_start * sectors) : (cyl_start * total_cyl))) /
			(1024 / sect_size)
		);
		exit(0);
	}

}

void
fdisk_do_list_devices (PedDisk* disk) {
        if (disk == NULL) {
                PedDevice* dev = NULL;

                ped_device_probe_all ();

                while ((dev = ped_device_get_next (dev))) {
                        if (!ped_device_open(dev))
	                        break;

                        PedDisk *disk = ped_disk_new(dev);
			if (!disk) // Not fatal error
				continue;

			if (fdisk_print_raw)
				do_raw (&disk);
			else
                        	do_print (&disk);

                        ped_disk_destroy(disk);
                }
        } else {
		if (fdisk_print_raw)
			do_raw(&disk);
		else
                	do_print(&disk);
	}
        exit(0);
}

static PedPartitionType
_disk_get_part_type_for_sector (PedDisk* disk, PedSector sector)
{
	PedPartition*	extended;

	extended = ped_disk_extended_partition (disk);
	if (!extended
	    || !ped_geometry_test_sector_inside (&extended->geom, sector))
		return 0;

	return PED_PARTITION_LOGICAL;
}
#if 0
/* This function checks if "part" contains a file system, and returs
 * 	0 if either no file system was found, or the user declined to add it.
 * 	1 if a file system was found, and the user chose to add it.
 * 	-1 if the user chose to cancel the entire search.
 */
static int
_rescue_add_partition (PedPartition* part)
{
	const PedFileSystemType*	fs_type;
	PedGeometry*			probed;
	PedExceptionOption		ex_opt;
	PedConstraint*			constraint;
	char*				found_start;
	char*				found_end;

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
	return 1;
}

/* hack: we only iterate through the start, since most (all) fs's have their
 * superblocks at the start.  We'll need to change this if we generalize
 * for RAID, or something...
 */
static int
_rescue_pass (PedDisk* disk, PedGeometry* start_range, PedGeometry* end_range)
{
	PedSector		start;
	PedGeometry		start_geom_exact;
	PedGeometry		entire_dev;
	PedConstraint		constraint;
	PedPartition*		part;
	PedPartitionType	part_type;

	part_type = _disk_get_part_type_for_sector (
			disk, (start_range->start + end_range->end) / 2);

	ped_geometry_init (&entire_dev, disk->dev, 0, disk->dev->length);

	ped_timer_reset (timer);
	ped_timer_set_state_name (timer, _("searching for file systems"));
	for (start = start_range->start; start <= start_range->end; start++) {
		ped_timer_update (timer, 1.0 * (start - start_range->start)
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
	ped_timer_update (timer, 1.0);

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

#endif
static int
do_rescue (PedDisk** disk)
{
	return perform_rescue(*disk, 0LL, 0LL,
	                      UI_CUSTOM_VALUES | UI_WARN_COMMIT);
#if 0
	PedSector		start = 0, end = 0;
	PedSector		fuzz;
	PedGeometry		probe_start_region;
	PedGeometry		probe_end_region;

	if (!*disk)
		goto error;

	if (!command_line_prompt_boolean_question
	(_("WARNING: rescue writes all data to disk automatically, continue")))
		return 1;

	if (!fdisk_command_line_get_sector (_("Start?"), (*disk)->dev, &start, NULL))
		goto error;
	if (!fdisk_command_line_get_sector (_("End?"), (*disk)->dev, &end, NULL))
		goto error;

	fuzz = PED_MAX (PED_MIN ((end - start) / 10, MEGABYTE_SECTORS),
		        MEGABYTE_SECTORS * 16);

	ped_geometry_init (&probe_start_region, (*disk)->dev,
			   PED_MAX(start - fuzz, 0),
			   PED_MIN(2 * fuzz, ((*disk)->dev)->length - (start - fuzz)));
	ped_geometry_init (&probe_end_region, (*disk)->dev,
			   PED_MAX(end - fuzz, 0),
			   PED_MIN(2 * fuzz, (*disk)->dev->length - (end - fuzz)));

	if (!_rescue_pass (*disk, &probe_start_region, &probe_end_region))
		goto error;

	return 1;

error:
	return 0;
#endif
}

static int
do_resize (PedDisk** disk)
{
	Option query_opts[] = {
		{ 's', _("don't move the beginning of the partition") },
		{ 'b', _("place it at the beginning of the free space before it") },
		{ 'e', _("place it at the end") },
		{ 'c', _("select custom start and end") },
		{ '\0', NULL }

	};
	PartPos pos;
	PedPartition *part = NULL, *temp;
	PedSector first,last;
	PedConstraint *constraint = NULL;
	PedPartitionType desired;
	UIOpts ui_opts = UI_WARN_COMMIT;

	if (!get_partition (_("Partition"), *disk, &part))
                	return 0;

	/* TODO: Almost shared between fdisk and cfdisk, move to common */
	if (part->fs_type) {
		if (!part->fs_type->ops->resize) {
			ped_exception_throw(PED_EXCEPTION_ERROR,
			                    PED_EXCEPTION_CANCEL,
			                    _("You can't resize this filesystem type."));
			return 0;
		}
		else {
			PedFileSystem *fs = ped_file_system_open(&part->geom);
			constraint = ped_file_system_get_resize_constraint(fs);
			if (!constraint || constraint->min_size == constraint->max_size)
			{
				ped_exception_throw(PED_EXCEPTION_ERROR,
				                    PED_EXCEPTION_CANCEL,
				       _("We can't resize this filesystem type"));
				if (constraint) ped_constraint_destroy(constraint);
				if (fs) ped_file_system_close (fs);
				return 0;
			}
			if (fs) ped_file_system_close (fs);
		}
	}
	else if (part->type != PED_PARTITION_EXTENDED) {
		ped_exception_throw(PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
		                    _("No filesystem detected on the partition."));
		return 0;
	}
	pos.start.sector = part->geom.start;
	pos.end.sector = part->geom.end;

	desired = PED_PARTITION_FREESPACE |
	                           (part->type & PED_PARTITION_LOGICAL);

	temp = part_list_prev(part,PED_PARTITION_METADATA);

	if (temp && (temp->type & desired))
		first = temp->geom.start;
	else
		first = part->geom.start;

	temp = part_list_next(part,PED_PARTITION_METADATA);

	if (temp && (temp->type & desired))
		last = temp->geom.end;
	else
		last = part->geom.end;

	if(!query_part_position(_("Place for the resized partition"),
	                        query_opts,&pos,first,last,(*disk)->dev,
	                        constraint,&ui_opts)) {
		if (constraint) ped_constraint_destroy (constraint);
		return 0;
	}
	if (!perform_resize (*disk, part, &pos, ui_opts)) {
		ped_exception_throw(PED_EXCEPTION_ERROR,
		                    PED_EXCEPTION_CANCEL,
		         _("Resize of the partition failed"));
		if (!(ui_opts & UI_NO_FS_RESIZE) && uiquery.need_commit)
				_disk_reread(disk);
		return 0;
	}
	return 1;
#if 0
	PedPartition		*part = NULL;
	PedFileSystem		*fs;
	PedConstraint		*constraint;
	PedSector		start, end;
	PedGeometry             *range_start = NULL, *range_end = NULL;
	PedGeometry		new_geom;

	if (!*disk)
		goto error;

	if (!command_line_prompt_boolean_question
	(_("WARNING: resize writes all data to disk automatically, continue")))
	        return 1;

	if (!fdisk_command_line_get_partition (_("Partition number?"), *disk, &part))
		goto error;
	if (part->type != PED_PARTITION_EXTENDED) {
		if (!_partition_warn_busy (part))
			goto error;
	}

	start = part->geom.start;
	end = part->geom.end;
	if (!fdisk_command_line_get_sector (_("Start?"), (*disk)->dev, &start, &range_start))
		goto error;
	if (!fdisk_command_line_get_sector (_("End?"), (*disk)->dev, &end, &range_end))
		goto error;

	if (!ped_geometry_init (&new_geom, (*disk)->dev, start, end - start + 1))
		goto error;
	snap_to_boundaries (&new_geom, &part->geom, *disk,
			    range_start, range_end);

	if (part->type == PED_PARTITION_EXTENDED) {
		constraint = constraint_from_start_end ((*disk)->dev,
				range_start, range_end);
		if (!ped_disk_set_partition_geom (*disk, part, constraint,
						  new_geom.start, new_geom.end))
			goto error_destroy_constraint;
		ped_partition_set_system (part, NULL);
	} else {
		fs = ped_file_system_open (&part->geom);
		if (!fs)
			goto error;
		constraint = constraint_intersect_and_destroy (
				ped_file_system_get_resize_constraint (fs),
				constraint_from_start_end (
					(*disk)->dev, range_start, range_end));
		if (!ped_disk_set_partition_geom (*disk, part, constraint,
						  new_geom.start, new_geom.end))
			goto error_close_fs;
		if (!ped_file_system_resize (fs, &part->geom, timer))
			goto error_close_fs;
		/* may have changed... eg fat16 -> fat32 */
		ped_partition_set_system (part, fs->type);
		ped_file_system_close (fs);
	}

	ped_constraint_destroy (constraint);
	if (range_start != NULL)
		ped_geometry_destroy (range_start);
	if (range_end != NULL)
		ped_geometry_destroy (range_end);
	return 1;

error_close_fs:
	ped_file_system_close (fs);
error_destroy_constraint:
	ped_constraint_destroy (constraint);
error:
	if (range_start != NULL)
		ped_geometry_destroy (range_start);
	if (range_end != NULL)
		ped_geometry_destroy (range_end);
	return 0;
#endif
}

static int
do_rm (PedDisk** disk)
{
	return perform_rm (*disk, NULL);
}

static int
do_select (PedDisk** disk)
{
	PedDevice*      dev;
	if (!get_device (_("New device?"), &dev))
		return 0;
	if (!ped_device_open (dev))
		return 0;

	if (uiquery.need_commit)
	        if (!command_line_prompt_boolean_question
		(_("WARNING: changes were made to the disk, "
		   "are you sure you want to discard them")))
	                return 1;

	/* Destroy the current disk and open the new.*/
	ped_disk_destroy (*disk);
	if (!(*disk = ped_disk_new (dev)))
	        return 0;

	/* Tell the user we are using the new device. */
	fdisk_print_using_dev (dev);

	if (uiquery.need_commit) uiquery.need_commit = 0;

	return 1;
}

static int
do_set (PedDisk** disk)
{
  return perform_set (*disk, NULL, 0, UI_DEFAULT);
}

static int
do_toggle_boot_flag (PedDisk** disk)
{
  return perform_set (*disk, NULL, PED_PARTITION_BOOT, UI_FLAG_TOGGLE);
}

static int
do_unit (PedDisk** disk)
{
  if (cylinder_unit) {
    printf ("%s\n", _("Changing display/entry units to sectors"));
    ped_unit_set_default(PED_UNIT_SECTOR);
    cylinder_unit = 0;
    return 1;
  } else {
    printf ("%s\n", _("Changing display/entry units to cylinders"));
    ped_unit_set_default(PED_UNIT_CYLINDER);
    cylinder_unit = 1;
    return 1;
  }
}

/*
 * do_commit: write all configuration changes to disk.
 */
static int
do_commit (PedDisk **disk)
{
	/* TODO: Move these to common! */
	if ((*disk)->dev->boot_dirty && (*disk)->dev->type != PED_DEVICE_FILE) {
	        ped_exception_throw (
				     PED_EXCEPTION_WARNING,
				     PED_EXCEPTION_OK,
				     _("You should reinstall your boot loader before "
				       "rebooting.  Read section 4 of the Parted User "
				       "documentation for more information."));
	}
	/* TODO: dev->type is deprecated, use is_blockdevice instead */
	if ((*disk)->dev->type != PED_DEVICE_FILE && !fdisk_opt_script_mode) {
	        ped_exception_throw (
				     PED_EXCEPTION_INFORMATION, PED_EXCEPTION_OK,
				     _("Don't forget to update /etc/fstab, if "
				       "necessary.\n"));
	}

	printf (_("\nWriting all changes to %s.\n"), _((*disk)->dev->path));
	if (!ped_disk_commit (*disk))
	        goto error;

	/* FIXME: This is not needed */
	if (!ped_disk_commit_to_os (*disk))
                goto error;

	/* After writing changes exit. */
  	if (!in_menu)
		do_quit (disk);

        return 1;

 error:
	return 0;
}



static int
do_copy(PedDisk** disk)
{
	if (!perform_cp (*disk, NULL, UI_WARN_COMMIT)) {
		ped_exception_throw(PED_EXCEPTION_ERROR,
		                    PED_EXCEPTION_CANCEL,
		         _("Partition copy failed"));
		if (uiquery.need_commit)
				_disk_reread(disk);
		return 0;
	}
	return 1;
}



static int
do_ex_menu (PedDisk** disk) {
	int status,old_menu;
        old_menu = in_menu;
	in_menu = 1;

	status = fdisk_interactive_menu (disk,
		fdisk_ex_menu_commands, 0);

	in_menu = old_menu;
	return status;
}

static int
do_list_systypes (PedDisk** disk)
{
	SysType* types = get_disklabel_system_types((*disk)->type);
	if (!types) {
		printf(_("System types for this disk label type are not "
		         "available.\n"));
		return 1;
	}
	int count, per_row = 4, per_col, i, j = 0;

	/* We calculate the system types count */
	for (count = 0; types[count].name; count++);

	/* We calculate the number of columns for the currect screen size */
	per_row = fdisk_screen_width()/20;
	/* We calculate the maximum number of elements per column */
	per_col = count/per_row + 1;

	/* We arrange the types in columns */
	for (i = 0; i <  per_col; i++) {
		for (j = 0; j < per_row; j++) {
			if (i+j*per_col >= count)
				break;
			printf("%2x  %-15.15s ",
				(unsigned int) types[i+j*per_col].type,
				_(types[i+j*per_col].name));
		}
		printf("\n");
	}

	return 1;
}

/* This changes the filesystem type. For now, only on msdos partition type */
static int
do_change_system_type (PedDisk **disk)
{
	int type = 0, i;
	PedPartition *part = NULL;
	char *input = NULL;
	const char *type_name = NULL;
	SysType *types = get_disklabel_system_types((*disk)->type);
	if (!types) {
		if (!command_line_prompt_boolean_question
			(_("WARNING: System types for this disk label type "
			    "seem unavailable, continue?")))
			return 0;
	}
	const char prompt[] = "Hex code (type L to list codes)";

	if(!get_partition(_("Partition"), *disk, &part) || !part)
		return 0;
	do {
		input = fdisk_command_line_get_word (prompt, NULL, NULL, 1);
		if (!input)
			return 0;
		if (toupper(*input) == 'L') {
			free(input);
			do_list_systypes(disk);
		}
		else
			break;
	} while(1);
	if (!sscanf(input,"%x",&type)) {
		free(input);
		return 0;
	}
	set_disk_specific_system_type(part,type);
	if (types)
		for (i = 0; types[i].name; i++) {
			if (types[i].type == type)
				type_name = _(types[i].name);
		}
	if (!type_name)
		type_name = _("Unknown");

	printf(_("Changed type of partition %d to %x (%s)\n"),
		part->num, type, type_name);
	return 1;
}



static int
do_fix_partition_order (PedDisk **disk)
{
	fix_partition_order(*disk);
}

/* The next three are for lfdisk compatibility with fdisk */
static int
do_change_sectors(PedDisk **disk)
{
	int sectors = (*disk)->dev->bios_geom.sectors;
	if (fdisk_command_line_get_integer(_("Number of sectors"), &sectors)) {
		(*disk)->dev->bios_geom.sectors = sectors;
		return 1;
	}
	else
		return 0;
}

static int
do_change_heads(PedDisk **disk)
{
	int heads = (*disk)->dev->bios_geom.heads;
	if (fdisk_command_line_get_integer(_("Number of heads"), &heads)) {
		(*disk)->dev->bios_geom.heads = heads;
		return 1;
	}
	else
		return 0;
}

static int
do_change_cylinders(PedDisk **disk)
{
	int cylinders = (*disk)->dev->bios_geom.cylinders;
	if (fdisk_command_line_get_integer(_("Number of cylinders"), &cylinders)) {
		(*disk)->dev->bios_geom.cylinders = cylinders;
		return 1;
	}
	else
		return 0;
}

static int
do_move_partition_beginning(PedDisk **disk)
{
	PedPartition *part = NULL;
	PartPos pos;
	if (!get_partition(_("Partition"), *disk, &part))
		return 0;
	pos.start.sector = part->geom.start;
	pos.end.sector = part->geom.end;

	pos.start.range = NULL;
	pos.end.range = NULL;

	/* This gets the new start in sectors */
	while (1) {
		if (!fdisk_command_line_get_llinteger(_("New beginning of data"),
						 &pos.start.sector))
			return 0;
		if (pos.start.sector < part->geom.start ||
				pos.start.sector >= part->geom.end)
			printf(_("Value out of range."));
		else
			break;
	}

	return perform_resize(*disk,part,&pos,UI_NO_FS_RESIZE);
}

static int
do_verify(PedDisk **disk)
{
	verify_partition_table (*disk);
	return 1;
}

static void
_init_messages ()
{
	StrList*		list;
	int			first;
	PedFileSystemType*	fs_type;
	PedDiskType*		disk_type;
	PedPartitionFlag	part_flag;
	PedUnit			unit;

/* flags */
	first = 1;
	list = str_list_create (_(flag_msg_start), NULL);
	for (part_flag = ped_partition_flag_next (0); part_flag;
	     		part_flag = ped_partition_flag_next (part_flag)) {
		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list,
				 _(ped_partition_flag_get_name (part_flag)));
	}
	str_list_append (list, "\n");

	flag_msg = str_list_convert (list);
	str_list_destroy (list);

/* units */
	first = 1;
	list = str_list_create (_(unit_msg_start), NULL);
	for (unit = PED_UNIT_FIRST; unit <= PED_UNIT_LAST; unit++) {
		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list, ped_unit_get_name (unit));
	}
	str_list_append (list, "\n");

	unit_msg = str_list_convert (list);
	str_list_destroy (list);

/* disk type */
	list = str_list_create (_(label_type_msg_start), NULL);

	first = 1;
	for (disk_type = ped_disk_type_get_next (NULL);
	     disk_type; disk_type = ped_disk_type_get_next (disk_type)) {
		if (disk_type->ops->write == NULL)
			continue;

		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list, disk_type->name);
	}
	str_list_append (list, "\n");

	label_type_msg = str_list_convert (list);
	str_list_destroy (list);

/* mkfs - file system types */
	list = str_list_create (_(fs_type_msg_start), NULL);

	first = 1;
	for (fs_type = ped_file_system_type_get_next (NULL);
	     fs_type; fs_type = ped_file_system_type_get_next (fs_type)) {
		if (fs_type->ops->create == NULL)
			continue;

		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list, fs_type->name);
	}
	str_list_append (list, "\n");

	mkfs_fs_type_msg = str_list_convert (list);
	str_list_destroy (list);

/* mkpart - file system types */
	list = str_list_create (_(fs_type_msg_start), NULL);

	first = 1;
	for (fs_type = ped_file_system_type_get_next (NULL);
	     fs_type; fs_type = ped_file_system_type_get_next (fs_type)) {
		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list, fs_type->name);
	}
	str_list_append (list, "\n");

	mkpart_fs_type_msg = str_list_convert (list);
	str_list_destroy (list);

/* resize - file system types */
	list = str_list_create (_(resize_msg_start), NULL);

	first = 1;
	for (fs_type = ped_file_system_type_get_next (NULL);
	     fs_type; fs_type = ped_file_system_type_get_next (fs_type)) {
		if (fs_type->ops->resize == NULL)
			continue;

		if (first)
			first = 0;
		else
			str_list_append (list, ", ");
		str_list_append (list, fs_type->name);
	}
	str_list_append (list, "\n");

	resize_fs_type_msg = str_list_convert (list);
	str_list_destroy (list);
}

static void
_done_messages ()
{
	free (flag_msg);
	free (mkfs_fs_type_msg);
	free (mkpart_fs_type_msg);
	free (resize_fs_type_msg);
	free (label_type_msg);
}

static void
_init_bsd_menu_commands () {


	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("d", _("d"), NULL),
		do_rm,
		str_list_create (
_(" d   delete a BSD partition"),
NULL), NULL, 1));



	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("l", _("l"), NULL),
		do_list_systypes,
		str_list_create (
_(" l   list known filesystem types"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("m", _("m"), NULL),
		do_help,
		str_list_create (
_(" m   print this menu"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("n", _("n"), NULL),
		do_mkpart,
		str_list_create (
_(" n   add a new BSD partition"),
NULL), NULL, 1));



	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("p", _("p"), NULL),
		do_print,
		str_list_create (
_(" p   print the BSD partition table"),
NULL), NULL, 1));

#if 0
	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("q", _("q"), NULL),
		do_quit,
		str_list_create (
_(" q   quit without saving changes"),
NULL), NULL, 1));
#endif

        fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("r", _("r"), NULL),
		NULL,
		str_list_create (
_(" r   return to the main menu"),
NULL), NULL, 1));

#if 0
	/* TODO: This should be available only on msdos and sun disklabels */
	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("t", _("t"), NULL),
		do_change_system_type,
		str_list_create (
_(" t   change a partition's filesystem id"),
NULL), NULL, 1));
#endif


	fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("u", _("u"), NULL),
		do_unit,
		str_list_create (
_(" u   change display/entry units"),
NULL), NULL, 1));

        fdisk_command_register (fdisk_bsd_menu_commands, fdisk_command_create (
		str_list_create_unique ("w", _("w"), NULL),
		do_commit,
		str_list_create (
_(" w   write disklabel to disk"),
NULL), NULL, 1));


}
static void
_init_ex_menu_commands () {


  if (!compat_mode)
  {

	fdisk_command_register (
		fdisk_ex_menu_commands,
			fdisk_command_create (
				str_list_create_unique ("v", _("v"), NULL),
				do_move,
				str_list_create (_(" v   move a partition"), NULL),
				str_list_create (_(number_msg), _(start_end_msg), NULL),
			1)
		);

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("c", _("c"), NULL),
		do_rescue,
		str_list_create (
_(" c   rescue a lost partition"),
NULL),
		str_list_create (_(start_end_msg), NULL), 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("z", _("z"), NULL),
		do_resize,
		str_list_create (
_(" z   resize a partition and its file system"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("h", _("h"), NULL),
		do_check,
		str_list_create (
_(" h   check the consistency of a partition"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("o", _("o"), NULL),
		do_copy,
		str_list_create (
_(" o   copy the partition over another partition"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("k", _("k"), NULL),
		do_mkfs,
		str_list_create (
_(" k   creates a filesystem on a partition"),
NULL), NULL, 1));


  }
  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("b", _("b"), NULL),
		do_move_partition_beginning,
		str_list_create (
_(" b   move beginning of data in a partition"),
NULL), NULL, 1));

  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("c", _("c"), NULL),
		do_change_cylinders,
		str_list_create (
_(" c   change number of cylinders"),
NULL), NULL, 1));

  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("e", _("e"), NULL),
		do_eprint,
		str_list_create (
_(" e   list extended partitions"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("f", _("f"), NULL),
		do_fix_partition_order,
		str_list_create (
_(" f   fix partition order"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("g", _("g"), NULL),
		do_dvh_mklabel,
		str_list_create (
_(" g   create an IRIX (SGI) partition table"),
NULL), NULL, 1));

  if (compat_mode)
  {
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("h", _("h"), NULL),
		do_change_heads,
		str_list_create (
_(" h   change number of heads"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("m", _("m"), NULL),
		do_help,
		str_list_create (
_(" m   print this menu"),
NULL), NULL, 1));
 }
  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("p", _("p"), NULL),
		do_xprint,
		str_list_create (
_(" p   print the partition table"),
NULL), NULL, 1));
  else
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("p", _("p"), NULL),
		do_print,
		str_list_create (
_(" p   print the partition table"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("q", _("q"), NULL),
		do_quit,
		str_list_create (
_(" q   quit without saving changes"),
NULL), NULL, 1));


        fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("r", _("r"), NULL),
		NULL,
		str_list_create (
_(" r   return to the main menu"),
NULL), NULL, 1));

  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("s", _("s"), NULL),
		do_change_sectors,
		str_list_create (
_(" s   change number of sectors/track"),
NULL), NULL, 1));

  if (compat_mode)
	fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("v", _("v"), NULL),
		do_verify,
		str_list_create (
_(" v   verify the partition table"),
NULL), NULL, 1));

        fdisk_command_register (fdisk_ex_menu_commands, fdisk_command_create (
		str_list_create_unique ("w", _("w"), NULL),
		do_commit,
		str_list_create (
_(" w   write table to disk and exit"),
NULL), NULL, 1));


}



static void
_init_main_menu_commands () {
	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("a", _("a"), NULL),
		do_toggle_boot_flag,
		str_list_create (
_(" a   toggle bootable flag"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("b", _("b"), NULL),
		do_edit_bsd_disklabel,
		str_list_create (
_(" b   edit bsd disklabel"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("d", _("d"), NULL),
		do_rm,
		str_list_create (
_(" d   delete a partition"),
NULL), NULL, 1));



	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("l", _("l"), NULL),
		do_list_systypes,
		str_list_create (
_(" l   list known partition types"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("m", _("m"), NULL),
		do_help,
		str_list_create (
_(" m   print this menu"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("n", _("n"), NULL),
		do_mkpart,
		str_list_create (
_(" n   add a new partition"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("o", _("o"), NULL),
		do_msdos_mklabel,
		str_list_create (
_(" o   create a new empty DOS partition table"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("p", _("p"), NULL),
		do_print,
		str_list_create (
_(" p   print the partition table"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("q", _("q"), NULL),
		do_quit,
		str_list_create (
_(" q   quit without saving changes"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("s", _("s"), NULL),
		do_sun_mklabel,
		str_list_create (
_(" s   create a new empty Sun disklabel"),
NULL), NULL, 1));


	/* TODO: This should be available only on msdos and sun disklabels */
	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("t", _("t"), NULL),
		do_change_system_type,
		str_list_create (
_(" t   change a partition's system id"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("u", _("u"), NULL),
		do_unit,
		str_list_create (
_(" u   change display/entry units"),
NULL), NULL, 1));

  if (compat_mode)
	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("v", _("v"), NULL),
		do_verify,
		str_list_create (
_(" v   verify the partition table"),
NULL), NULL, 1));

        fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("w", _("w"), NULL),
		do_commit,
		str_list_create (
_(" w   write table to disk and exit"),
NULL), NULL, 1));

	fdisk_command_register (fdisk_main_menu_commands, fdisk_command_create (
		str_list_create_unique ("x", _("x"), NULL),
		do_ex_menu,
		str_list_create (
_(" x   extra functionality (experts only)"),
NULL), NULL, 1));
}

static void
_done_commands ()
{
	FdiskCommand**	walk;

	for (walk = fdisk_main_menu_commands; *walk; walk++) {
		fdisk_command_destroy (*walk);
		*walk = NULL;
	}

	for (walk = fdisk_ex_menu_commands; *walk; walk++) {
		fdisk_command_destroy (*walk);
		*walk = NULL;
	}
	for (walk = fdisk_bsd_menu_commands; *walk; walk++) {
		fdisk_command_destroy(*walk);
		*walk = NULL;
	}
}

static void
_init_i18n ()
{
/* intialize i18n */
#if ENABLE_NLS && HAVE_LOCALE_H
  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif /* ENABLE_NLS && HAVE_LOCALE_H */
}

/* initialise UI calls structure */
static void
_init_uicalls()
{
	//uiquery = malloc(sizeof(UICalls));
	/* TODO: Get rid of getstring and getint functions */
	uiquery.getstring = getstring;
	uiquery.getint = fdisk_command_line_get_integer;
	uiquery.getbool = getbool;
	uiquery.getparttype = fdisk_command_line_get_part_type;
	uiquery.getdev = NULL;
	uiquery.getpart = NULL;
	uiquery.getdisktype = NULL;
	uiquery.getfstype = NULL;
	/* TODO: Will we use this? */
	uiquery.getpartpos = fdisk_get_partpos;
	uiquery.need_commit = 0;
	uiquery.timer = ped_timer_new (_timer_handler, &timer_context);
	set_uicalls(&uiquery);
}

static void
_version ()
{
  puts(interface_name);
  exit (0);
}

/**
 * build a struct option array from extd_options table
 * \return a pointer to NULL terminated struct option array
 * allocated with malloc */
static struct option *
build_option_table(void)
{
  unsigned int i, len = sizeof(extd_options) / sizeof(extd_options[0]);
  struct option *option = malloc(sizeof(struct option) * len);
  if(option == NULL)
    {
      perror(_("dynamic memory allocation failure"));
      exit(EXIT_FAILURE);
    }
  for(i=0; i < len; i++)
    memcpy(&(option[i]), &(extd_options[i]), sizeof(struct option));
  return option;
}

static int
_parse_options (int* argc_ptr, char*** argv_ptr)
{
  int opt;
  struct option *options_table, *ptr;

  /* Build option table we need two pointers because getopt long
   * modify `ptr' and we can't feree it */
  options_table = ptr = build_option_table();

  while((opt = getopt_long(*argc_ptr, *argv_ptr, 
			   short_option_string, ptr, NULL)) != -1)
    {
      /* NOTE: b, C, H, S are ignored, for Linux fdisk compatibility */
      switch (opt) 
	{
	case 'h': 
	  fdisk_help_msg ();
	  break;
	case 'l': 
	  fdisk_list_table = 1;
	  break;
	case 'r': 
	  fdisk_list_table = 1;
	  fdisk_print_raw = 1;
	  break;
	case 'i':
	  fdisk_opt_script_mode = 0;
	  break;
	case 'p':
	  fdisk_opt_script_mode = 1;
	  break;
#ifdef GNU_EXT
	case 'G':
	  compat_mode = 0;
	  break;
	case 'L':
	  compat_mode = 1;
	  break;
#endif
	case 's':
	  fdisk_partsize_device = optarg;
	  break;
	case 't':
	  print_partition_types();
	  break;
	case 'u':
	  ped_unit_set_default(PED_UNIT_SECTOR);
	  cylinder_unit = 0;
	  break;
	case 'v':
	  _version();
	  break;
	case 'b':
	  user_sectsize = atoll(optarg);
	  if (user_sectsize <= 0 || user_sectsize % 512)
	    {
	      fdisk_usage_msg();
	      return 0;
	    }
	  break;
	case 'C':
	  user_cyls = atoi(optarg);
	  break;
	case 'H':
	  user_heads = atoi(optarg);
	  break;
	case 'S':
	  user_sectors = atoi(optarg);
	  break;
	}
    }
  
  *argc_ptr -= optind;
  *argv_ptr += optind;
  
  free(options_table);
  
  return 1;
/* what is? */
error:
  return 0;
}

static PedDevice*
_choose_device (int* argc_ptr, char*** argv_ptr)
{
  PedDevice* dev;
  const char* path = NULL;

  /* if we want partition size */
  if (fdisk_partsize_device) 
    {
      static char buf[512];
      fdisk_partsize_part = 
	cut_device_partnum(buf, sizeof(buf), fdisk_partsize_device);
      path = buf;
    }
  else if (*argc_ptr) 
    path = (*argv_ptr) [0];
  /* specified on comand line? */
  if (path) 
    {
      dev = ped_device_get (path);
      if (!dev) 
	{
	  printf(_("Unable to open %s\n"), path);
	  return NULL;
	}
      /***************************************************************/
      /* TODO: I have no idea why this is here, but I leave it as is */
      /***************************************************************/
      if (*argc_ptr) 
	{
	  (*argc_ptr)--;
	  (*argv_ptr)++;
	}
      /* If sector size, cylnders, heads or sectors are specified on
	 the command line */
      if (compat_mode) 
	{
	  if (user_sectsize) 
	    {
	      /* This should be multiple of 512, checked at
		 option parsing */
	      PedSector bytes = dev->sector_size * dev->length;
	      dev->sector_size = user_sectsize;
	      dev->length = bytes / user_sectsize;
	    }
	  if (user_sectors || user_heads) 
	    {
	      /* NOTE: PedSector is long long */
	      long long llcyl;
	      if (user_sectors)
		dev->bios_geom.sectors = user_sectors;
	      if (user_heads)
		dev->bios_geom.heads = user_heads;
	      llcyl = dev->length / 
		((long long)dev->bios_geom.heads
		 * dev->bios_geom.sectors);
	      dev->bios_geom.cylinders = llcyl;
	      /* If it was truncated, use the max value,
		 as Linux fdisk */
	      /* FIXME: Linux fdisk uses uint */
	      if (dev->bios_geom.cylinders != llcyl)
		dev->bios_geom.cylinders = INT_MAX;
	      if (!dev->bios_geom.cylinders)
		dev->bios_geom.cylinders = user_cyls;
	    }
	}
      if (!ped_device_open (dev)) 
	{
	  printf(_("Unable to open %s\n"), path);
	  return NULL;
	}
      return dev;
    }
  return NULL;
}

static PedDevice*
_init (int* argc_ptr, char*** argv_ptr)
{
  PedDevice*	dev;


  /* Christian <mail.kristian@yahoo.it>
   * 
   * Before global `compat_mode' fdisk use a variable only for this
   * file. If GNU_EXT isn't defined identifyer of this variable was
   * defined as a macro wich expand to a value of 1 
   */
#if !defined GNU_EXT || !GNU_EXT 
  compat_mode = 1; 
#endif

#ifdef ENABLE_MTRACE
  mtrace();
#endif

#if HAVE_READLINE && HAVE_LIBTERMCAP
  init_termcap_library();
#endif /* HAVE_READLINE && HAVE_LIBTERMCAP */

  _init_i18n ();
  
  if (!fdisk_init_ui ())
    goto error;

  _init_messages ();

  /* The default units are cylinders */
  ped_unit_set_default(PED_UNIT_CYLINDER);

  /* TODO: Make _init_messages init these, remove messages from ui.c */
  init_flag_str();
  init_fs_type_str();
  init_disk_type_str();

  if (!_parse_options (argc_ptr, argv_ptr))
    goto error_done_commands;
  
  dev = _choose_device (argc_ptr, argv_ptr);

  _init_ex_menu_commands ();
  _init_bsd_menu_commands ();
  _init_main_menu_commands ();
  
  if (!dev) {
    goto error_done_commands;
  }
  _init_uicalls();
  
  if (!uiquery.timer)
    goto error_done_commands;
  timer_context.last_update = 0;
  
  return dev;
  
 error_done_commands:
  _done_commands ();
  _done_messages ();
  
 error_done_ui:
  fdisk_done_ui ();
  
 error:
  return NULL;
}

static void
_done (PedDevice* dev)
{
  ped_device_close (dev);
  
  /* TODO: _done_uicalls? */
  ped_timer_destroy (uiquery.timer);
  _done_commands ();
  _done_messages ();
  fdisk_done_ui();
}

/**
 * Program entry point
 * \param argc number of command line arguments
 * \param argv array of command line arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int
fdisk (int argc, char** argv)
{
  PedDevice* dev;
  PedDisk* disk;
  
  if (argc <= 1)
    {
      /* Missing device parameter */
      fdisk_usage_msg();
      return EXIT_FAILURE;
    }
  
#ifdef GNU_EXT
  /* See whether we should enable Linux fdisk compatibility mode */
  if (argv[0]) 
    {
      char *program_name = strrchr(argv[0], '/');
      if (program_name)
	program_name++;
      else
	program_name = argv[0];
      if(strcmp(program_name,"lfdisk") == 0)
	compat_mode = 1;
    }
  else
    compat_mode = 0;
#endif /* GNU_EXT */
  
  dev = _init (&argc, &argv);

  if(!dev)
    {
      /* List all devices */
      if(fdisk_list_table)
	do_list_devices(NULL);
      else
	return EXIT_FAILURE;
    }

  return (fdisk_interactive_mode(&dev, fdisk_main_menu_commands) 
	  ? EXIT_SUCCESS : EXIT_FAILURE);
}
