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
#include <parted/parted.h>
#include "../src/strlist.h"
#include "../src/common.h"
#include "functions.h"



/* FIXME: Make use of rand_r instead of rand ! See functions.h */

/* Here we test mklabel */
START_TEST (test_mklabel)
{

	strncpy(string[0],"msdos", BUFSIZE);
	disk_type = ped_disk_type_get(string[0]);
	/* First we will test with giving it the string */
	uiquery.getstring = getstring;
	uiquery.getdisktype = NULL;
	fail_unless (perform_mklabel(dev, &disk, NULL), 
	             "Failed to create a label with a getstring query");
	fail_unless (disk != NULL, "mklabel reported success, but failed");
	fail_unless (!strcmp(string[0],disk->type->name),
	             "mklabel created wrong disk type");
	/* Then we do it the other way */
	uiquery.getstring = NULL;
	uiquery.getdisktype = getdisktype;
	fail_unless (perform_mklabel(dev,&disk, NULL),
	             "Failed to create a label with a getdisktype query");
	fail_unless (disk != NULL,
	             "mklabel reported success, but failed, getdisktype");
	fail_unless (!strcmp(string[0],disk->type->name),
	             "mklabel created wrong disk type, using getdisktype");
	if (disk) ped_disk_destroy(disk);
	disk = NULL;
	/* Then we do it the third way */	
	uiquery.getdisktype = NULL;
	fail_unless (perform_mklabel(dev,&disk,disk_type),
	             "Failed to create a label with a specified type");
	fail_unless (disk != NULL,
	             "mklabel reported success, but failed, specified");
	fail_unless (!strcmp(string[0],disk->type->name),
	             "mklabel created wrong disk type when we specified it");
	test_exception();
	
}
END_TEST


/* Here we test mkpart and rmpart */
START_TEST (test_jugglepart) {
	PedConstraint *constraint = ped_device_get_constraint(dev);
	PedFileSystemType* walk = NULL;
	PedGeometry *geom = NULL;
	PedPartition *part = NULL;
	PartPos pos = { {0LL, NULL}, {20000LL, NULL} };
	int i;
	/* First we create an extended partition */
	//set_exception_error("Juggling partitions");
	fail_unless(perform_mklabel(dev,&disk,ped_disk_type_get("msdos")),
	            "We could create the label for juggling partitions");
	fail_unless(perform_mkpart(disk,&pos,PED_PARTITION_EXTENDED,NULL,NULL,
	                      UI_DEFAULT),
	            "Failed to create an extended partition");

	/* Check if it is there */
	do {
		part = ped_disk_next_partition (disk, part);
	} while (part && !(part->type & PED_PARTITION_EXTENDED));
	fail_unless(part != NULL, "Can't find the extended partition");

	/* Then we maximize it */
	fail_unless(perform_maximize(disk,part), "Could not maximize the extended");

	/* We check if our partition is "near" to what they should */
	fail_unless(are_near(constraint->start_range->start,part->geom.start)
	            && are_near(constraint->end_range->end,part->geom.end),
	            "Maximize didn't work correctly.\n"
	            "We wanted %lld-%lld, we got %lld-%lld",
	            constraint->start_range->start, constraint->end_range->end,
	            part->geom.start, part->geom.end);
	constraint = constraint_intersect_and_destroy(constraint,
	           constraint_from_start_end(dev,&(part->geom),&(part->geom)));

	
	srand(218872305);
	walk = ped_file_system_type_get_next (NULL);
	/* We first try to create 20 partitions */
	for (i = 0; i < 20; i++) {
		next_part_geometry(&geom, constraint, 1);
		fail_unless(_mkpart (i, geom->start,  geom->end, 
			             PED_PARTITION_LOGICAL, walk, &part,
			             perform_mkpart),
			"Failed to create %s partition. "
			"Number %d, size %llds, start %llds, end %llds",
			walk->name, i, geom->length, geom->start, geom->end);

		fail_unless(are_near(geom->start,part->geom.start) &&
			are_near(geom->end,part->geom.end),
			"Failed to create %s partition where we want. "
			"Number %d, we wanted %lld-%lld, we got %lld-%lld",
			walk->name, i, geom->start, geom->end,
			part->geom.start, part->geom.end);
		ped_geometry_set (geom,part->geom.start,part->geom.length);
		do {
			walk = ped_file_system_type_get_next (walk);
		} while (walk == NULL);
		
	}
	/* Now, this should delete all the partitions */	
	while (part) {
		PedPartition *temp = part;
		do {
			part = part->prev;
		} while (part && part->type & PED_PARTITION_METADATA);
		if (part && (part->type & PED_PARTITION_FREESPACE))
			fail("We reached a hole at %llds-%llds, number %d",
			     part->geom.start, part->geom.end, i);
		fail_unless (perform_rm(disk,temp),
		             "Failed to delete partition after %d",
		             (part ? part->num : -1));
		i--;
	}
	fail_unless(i == 0, "Not all partitions were deleted. %d remain.", i);
	ped_geometry_destroy(geom);
	test_exception();
}
END_TEST


/* Here we test (*only*) the size changing with perform_resize */
START_TEST (test_chsize_msdos) {
	change_size("msdos", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_amiga) {
	change_size("amiga", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_mac) {
	change_size("mac", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

/* FIXME: This test fails! */
START_TEST (test_chsize_dvh) {
	change_size("dvh", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_pc98) {
	change_size("pc98", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_bsd) {
	change_size("bsd", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_sun) {
	change_size("sun", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

START_TEST (test_chsize_gpt) {
	change_size("gpt", UI_NO_FS_RESIZE);
	test_exception();
}
END_TEST

int poss_compare(char* pos1, char* pos2) {
	char all_poss[] = { 'b', 's', 'e', 'c', '\0' };
	int i;
	for (i = 0; all_poss[i]; i++) {
		if ((strchr(pos1,all_poss[i]) == NULL) != 
		    (strchr(pos2,all_poss[i]) == NULL))
			return 0;
	}
	return 1;
}

/* TODO: Make this a function */
#define run_pos_test(part,constraint,desired_opts,poss_ok,posg,strings) \
	pos.start.sector = 2000LL; \
	pos.end.sector = 21999LL; \
	pos.start.range = NULL; \
	pos.end.range = NULL; \
	opts = UI_DEFAULT; \
	pos_gets = 0; string_gets = 0; \
	fail_unless(query_part_position("", NULL, &pos,  \
	                                where.start, where.end, \
	                                dev, (constraint), &opts), \
	            "Query partition position %s failed", (part)); \
	fail_unless(pos.start.sector == desired.start && \
	            pos.end.sector == desired.end, \
	            "Partition position %s wrong. Was %llds-%llds, " \
	            "should be %llds-%llds.", \
	            (part), pos.start.sector, pos.end.sector, \
	            desired.start, desired.end); \
	fail_unless(opts == (desired_opts), "Test %s opts wrong. " \
	            "Was %d, should be %d.", opts, (desired_opts)); \
	fail_unless(string_gets == (strings), "%s prompted wrong times." \
	            "Strings %d, should be %d. Pos %d should be %d.", \
	             (part), string_gets, (strings), pos_gets, (posg)); \
	fail_unless(poss_compare(pos_poss,(poss_ok)), \
	            "%s: incorrect possibilities. Got %s, should be %s.", \
	             (part), pos_poss, (poss_ok))

START_TEST (test_partpos) {
	PedGeometry desired;
	PedGeometry where;
	PartPos pos;

	PedConstraint *constraint = ped_device_get_constraint(dev);

	UIOpts opts = UI_DEFAULT;

	uiquery.getpartpos = getpartpos;
	uiquery.getstring = getstring;

	string[0][BUFSIZE] = 0;
	/* A:    We test without constraint */

	/*   A1: We test with where = partition*/
	ped_geometry_init(&where,dev,2000LL,20000LL);
	/* Start */
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 's';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("A1s",NULL,UI_DEFAULT,"sec",1,1);
	/* End */
	ped_geometry_init(&desired,dev,where.end-4000LL+1LL,4000LL);
	partpos = 'e';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("A1e",NULL,UI_DEFAULT,"sec",1,1);
	/* Custom */
	ped_geometry_init(&desired,dev,where.start,where.length);
	partpos = 'c';
	run_pos_test("A1c",NULL,UI_CUSTOM_VALUES,"sec",1,0);
	
	/*   A2: We test with where = big */
	ped_geometry_init(&where,dev,0LL,24000LL);
	/* Start */
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 's';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("A2s",NULL,UI_DEFAULT,"bsec",1,1);
	/* Beginning */
	ped_geometry_init(&desired,dev,where.start,4000LL);
	partpos = 'b';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("A1s",NULL,UI_DEFAULT,"bsec",1,1);
	/* End */
	ped_geometry_init(&desired,dev,where.end-4000LL+1LL,4000LL);
	partpos = 'e';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("A2e",NULL,UI_DEFAULT,"bsec",1,1);
	/* Custom */
	ped_geometry_init(&desired,dev,2000LL,20000LL);
	partpos = 'c';
	run_pos_test("A2c",NULL,UI_CUSTOM_VALUES,"bsec",1,0);

	/* B: We test with some constraint */
	/*   B1: We test with where = partition, Start*/
	ped_geometry_init(&where,dev,2000LL,20000LL);
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 's';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("B1s",constraint,UI_DEFAULT,"sec",1,1);
	/*   B2: We test with where = big, End */
	ped_geometry_init(&where,dev,0LL,24000LL);
	ped_geometry_init(&desired,dev,where.end-4000LL+1LL,4000LL);
	partpos = 'e';
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	run_pos_test("B2e",constraint,UI_DEFAULT,"bsec",1,1);
	
	/* C: We test with constraint of a partition with unmovable start */
	constraint->start_align->grain_size = 0;
	/*   C1s: We test with where = partition*/
	ped_geometry_init(&where,dev,2000LL,20000LL);
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 'j'; /* We don't need to serve this right, right? */
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	strncpy(pos_poss,"bsc",sizeof(pos_poss));
	run_pos_test("C1s",constraint,UI_DEFAULT,"bsc",0,1);
	/* C2s: We test with where = big */
	ped_geometry_init(&where,dev,0LL,24000LL);
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 'j'; /* We don't need to serve this right, right? */
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	strncpy(pos_poss,"sc",sizeof(pos_poss));
	run_pos_test("C2s",constraint,UI_DEFAULT,"sc",0,1);

	/* D: Another constraint */
	ped_constraint_destroy(constraint);
	constraint = ped_device_get_constraint(dev);
	ped_geometry_set(constraint->start_range,2000LL,1LL);
	ped_geometry_init(&where,dev,2000LL,20000LL);
	ped_geometry_init(&desired,dev,2000LL,4000LL);
	partpos = 'j'; /* We don't need to serve this right, right? */
	snprintf(string[0],BUFSIZE,"%llds",desired.length);
	strncpy(pos_poss,"bsc",sizeof(pos_poss));
	run_pos_test("D1s",constraint,UI_DEFAULT,"bsc",0,1);


}
END_TEST

Suite *common_suite(void)
{
	Suite *s = suite_create("Common functions");
	TCase *tc_label = tcase_create("Label");
	TCase *tc_juggle = tcase_create("Creation and deletion of partitions");
	TCase *tc_chsize = tcase_create("Changing partiton sizes");
	suite_add_tcase (s, tc_label);
	suite_add_tcase (s, tc_juggle);
	suite_add_tcase (s, tc_chsize);
	tcase_add_test(tc_label, test_mklabel);
	tcase_add_test(tc_juggle, test_jugglepart);
	tcase_set_timeout(tc_chsize, 10);
	tcase_add_test(tc_chsize, test_partpos);
	tcase_add_test(tc_chsize, test_chsize_msdos);
	tcase_add_test(tc_chsize, test_chsize_bsd);
	//tcase_add_test(tc_chsize, test_chsize_pc98);
	//tcase_add_test(tc_chsize, test_chsize_amiga);
	tcase_add_test(tc_chsize, test_chsize_gpt);
	//tcase_add_test(tc_chsize, test_chsize_dvh);
	tcase_add_test(tc_chsize, test_chsize_mac);
	return s;
}

int main (void)
{
	check_safety ();
	int nf;
	if (!init_tempfile()) return 1;
	if (!open_device()) {
		unlink_tempfile;
		return 1;
	}
	
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

	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
