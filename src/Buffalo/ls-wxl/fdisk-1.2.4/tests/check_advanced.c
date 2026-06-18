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

#include <stdlib.h>
#include <check.h>
#include <stdio.h>
#include <string.h>
#include "../src/strlist.h"
#include "../src/common.h"
#include "functions.h"

/* Well, just in case we need it ... */
#if 0
START_TEST (test_null)
{
	fail_unless(1 == 1, "If you are reading this message, you have "
	                  "just won a brand-new pink-coloured supercomputer");
}
END_TEST
#endif
/* */
static void
warn () {
	printf("We are about to run a very CPU and disk intensive test.\n"
	       "It can take about 500 MB of free space.\n"
               "Are you sure you want to continue (y/N)? ");
	if (getchar() != 'y' && getchar() != 'Y') {
		exit(0);
	}
}
/* We need to create a new tempfile for each test, so we can save disk space */
static void
open_tempfile(int mklabel) {
	if (!init_tempfile()) fail("Could not init tempfile");
	if (!open_device()) {
		unlink_tempfile();
		fail("Could not open the tempfile");
	}
	if (mklabel && !perform_mklabel(dev,&disk,ped_disk_type_get("msdos")))
		fail("Could not create a label on the tempfile");
	
}
static void
close_tempfile() {
	ped_disk_destroy(disk);
	ped_device_destroy(dev);
	unlink_tempfile();
}

/* This takes ~160MB of disk space. */
START_TEST (test_resize) {
	open_tempfile(0);
	change_size("msdos", UI_DEFAULT);
	test_exception();
	/* We will use the result file for testing partition copy */
	rename(tempfile_name(),tempfile2_name());
	//close_tempfile();
}
END_TEST

/* This takes ~200MB of disk space */
#define CAN_USE(x) ((x) && ((x)->type & PED_PARTITION_FREESPACE))
START_TEST (test_move) {
	int i;
	PedGeometry where;
	PedPartition *part;
	PedFileSystemType *fs_type = ped_file_system_type_get("fat32");
	open_tempfile(1);
	//PedSector maxlen = dev->length/4LL;
	PedSector start = 0LL;
	PedSector end = dev->length/4LL - 1;
	PedSector oldstart,oldend;
	fail_unless(_mkpart (0, start,  end,
				PED_PARTITION_NORMAL,
 				fs_type, &part, perform_mkpartfs),
	            "Failed to create the partition");
	/* We seed and move 16 times */
	srand(173813981);
	for (i = 0; i < 16; i++) {
		/* We move the partition to the larger free space before
		   or after it */
		if (CAN_USE(part->next)) {
			if (CAN_USE(part->prev) &&
			    part->prev->geom.length > part->next->geom.length)
				where = part->prev->geom;
			else
				where = part->next->geom;
		} else
			where = part->prev->geom;
		oldstart = start;
		oldend = end;
		randomize_position(&start, &end, &where, 1);
		set_exception_error("Trying to move %d from %llds-%llds "
		                    "to %llds-%llds in %llds-%llds",
		                     i, oldstart, oldend, start, end,
		                     where.start, where.end);
		fail_unless(_resmov(i, part, start, end, UI_DEFAULT,
		                    perform_move),
		            "Failed to move %d partition "
		            "from %llds-%llds to %llds-%llds in %llds-%llds",
		            i, oldstart, oldend, start, end,
		            where.start, where.end);
		fail_unless(are_near(start,part->geom.start) && 
		            are_near(end,part->geom.end),
		            "Partition %d was moved from %llds-%llds to "
		            "%llds-%llds instead of %llds-%llds in "
		            "%llds-%llds", i,
	                    oldstart, oldend, part->geom.start, part->geom.end, 
		            start, end, where.start, where.end);
	}
	test_exception();
	close_tempfile();
}
END_TEST

/* This uses about 30 MB */
START_TEST (test_copy) {
	PedPartition *part = NULL;
	PedGeometry *geom = NULL;
	PedFileSystem *fs;
	PedFileSystemType *fs_type = ped_file_system_type_get("fat32");
	open_tempfile(0);
	/* Why not amiga? */
	fail_unless(perform_mklabel(dev,&disk,ped_disk_type_get("msdos")),
	            "Failed to create partition table for partition copy");
	PedConstraint *constraint = ped_device_get_constraint(dev);
	
	/* Opening the disk with our partitions */
	PedDevice *dev2 = ped_device_get(tempfile2_name());
	fail_unless(ped_device_open(dev2), 
		            "Failed to open the device for copying");
	PedDisk	*disk2 = NULL;
	/* This throws an exception on failure, we should fail */
	disk2 = ped_disk_new(dev2);
	//fail_unless(1, "hm");


	srand(129312);
	int i;
	/* We copy all partitions to the new disk */
	for (i = 0; i < 4; i++) {
		/* We create a partition on the empty  disk */
		next_part_geometry(&geom, constraint, 1);
		PUT_STRING("Label");
		set_exception_error("Creating partition %d for copy at "
			"%llds-%llds", i, geom->start, geom->end);
		fail_unless(_mkpart (0, geom->start,  geom->end,
				PED_PARTITION_NORMAL,
 				fs_type, &part, perform_mkpart),
			"Failed to create %d partition for copy. "
			"Size %llds, start %llds, end %llds",
			i, geom->length, geom->start, geom->end);
		/* We will query the source disk and partition */
		uiquery.getdev = getdev;
		device = dev2;
		uiquery.getpart = NULL;
		uiquery.getint = getint;
		integer = i+1;
		set_exception_error("Copying %d to %llds-%llds",
		                    i, part->geom.start, part->geom.end);
		fail_unless(perform_cp (disk, part, UI_DEFAULT), 
		            "%d partition copy failed (from %llds-%llds "
		            "to %llds-%llds)", i, 0LL, 0LL, part->geom.start,
		            part->geom.end);
		fs = ped_file_system_open(&(part->geom));
		fail_unless(fs != NULL, "No filesystem on partition %d", i);
		fail_unless(ped_file_system_check(fs,NULL),
		            "Error on partition %d", i);
		ped_file_system_close(fs);

	}
	/* Now we try to copy from a partition on the same disk */
	device = dev;
	integer = 1;
	fail_unless(perform_cp (disk, part, UI_DEFAULT), 
	            "Partition copy on a single disk failed");
	ped_disk_destroy(disk2);
	ped_device_destroy(dev2);
	test_exception();
	/* We don't need this anymore. We won't unlink the tempfile, because *
         * we will test the rescue on it.                                    */
	//close_tempfile();
	unlink(tempfile2_name());
}
END_TEST


/* I'm lazy to type too much, so */
#define CREATE_PART(num,start,end) \
	fail_unless(_mkpart(0, start, end, PED_PARTITION_NORMAL, \
	                    fs_type, NULL, perform_mkpartfs), \
	            "Failed to create num partition for rescue")
/* This does not take any additional disk space */
START_TEST (test_rescue) {
	int i;
	PedPartition *part = NULL;
	PedGeometry geom;
	PedFileSystemType *fs_type = ped_file_system_type_get("ext2");
#if 0	
	dev = ped_device_get(tempfile_name());
	fail_unless(ped_device_open(dev), 
		            "Failed to open the device for rescue");
	disk = ped_disk_new(dev);
	/* We create the partitions */
#endif
	open_tempfile(1);

	CREATE_PART(1,0LL,400LL);
	CREATE_PART(2,401LL,1000LL);
	CREATE_PART(3,1001LL,1200LL);
	CREATE_PART(4,1201LL,2000LL);


	/* Then we delete the partitions */
	part = ped_disk_get_partition(disk,
		ped_disk_get_last_partition_num (disk));
	while(part->type)
		part = part->prev;
	geom = part->geom;
	while (part) {
		PedPartition *temp = part;
		part = part->prev;
		if (!temp->type) {
			fail_unless(perform_rm(disk, part),
			            "Rescue could not delete %d partition",
			            part->num);
		}
	}
	
	/* Then we try to undelete the last one */
	fail_unless(perform_rescue(disk, geom.start, geom.end, UI_DEFAULT),
	            "Failed to rescue a single partition at %llds-%llds",
	            geom.start, geom.end);
	/* part should be the undeleted partition */
	part = ped_disk_get_partition(disk,
		ped_disk_get_last_partition_num (disk));
	while (part && part->type) {
		part = part->prev;
	}

	fail_unless(geom.start==part->geom.start && geom.end==part->geom.end,
                    "Rescued partition is not the same as the deleted. "
                    "Deleted was %llds-%llds, we got %llds-%llds.",
	            geom.start, geom.end, part->geom.start, part->geom.end);
	/* We delete it again and try to rescue all */
	fail_unless(perform_rm(disk, part),
	            "Rescue could not delete %d partition",
	            part->num);

	/* The end of the last partition is before 8388604s */
	fail_unless(perform_rescue(disk, 0LL, geom.end, UI_DEFAULT),
	            "Failed to rescue the partitions");
	/* Now we count them, if they aren't four, we did something wrong */
	for (part = ped_disk_next_partition(disk,NULL), i = 0;
	     part; part = ped_disk_next_partition(disk,part)) {
		if (!part->type)
			i++;
	}
	fail_unless(i == 4, "We got %d partitions instead of 4", i);
	test_exception();
	close_tempfile();
}
END_TEST

Suite *common_suite(void)
{
	Suite *s = suite_create("Advanced checks");
	TCase *tc_resize = tcase_create("Resize");
	TCase *tc_move = tcase_create("Move");
	TCase *tc_copy = tcase_create("Copy");
	TCase *tc_rescue = tcase_create("Rescue");
	suite_add_tcase (s, tc_resize);
	suite_add_tcase (s, tc_move);
	suite_add_tcase (s, tc_copy);
	suite_add_tcase (s, tc_rescue);
	tcase_set_timeout(tc_resize, 360);
	tcase_set_timeout(tc_move, 360);
	tcase_set_timeout(tc_copy, 360);
	/* This could take a lot of time, I'm not sure that's enough */
	tcase_set_timeout(tc_rescue, 1200);
	tcase_add_test(tc_resize, test_resize);
	tcase_add_test(tc_move, test_move);
	tcase_add_test(tc_copy, test_copy);
	tcase_add_test(tc_rescue, test_rescue);
	return s;
}


int main (void)
{
	warn();
	check_safety ();
	int nf;
	ped_exception_set_handler(exception_handler);
	uiquery.getpartpos = NULL;
	set_uicalls(&uiquery);
	reset_exception_error();
	string[0][BUFSIZE] = 0;
	Suite *s = common_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all (sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	unlink_tempfile();
	unlink(tempfile2_name());
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
