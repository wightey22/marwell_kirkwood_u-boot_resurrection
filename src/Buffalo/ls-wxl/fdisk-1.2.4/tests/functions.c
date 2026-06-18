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
#include <stdio.h>
#include <stdarg.h>
#include <check.h>
#include <sys/stat.h>
#include <parted/parted.h>
/* #include <endian.h> */
#include <string.h>
#include "../config.h"
#include "../src/strlist.h"
#include "functions.h"
#include "../src/common.h"


/* NOTE: In the comments we say that we "query" or "ask" something when we
   emulate user query function and pass the parameter through there */

/* We can't seek more... */
#define SIZE 2147483647L
/* We will give the partitions below different sizes. I'll use rand */
#define PART_LENGTH (2097151LL - ((unsigned int) rand() % 1048575))

PedDevice *dev = NULL;
PedDisk *disk = NULL;
UICalls uiquery;

/* We need a seed for the random functions */
int seed = 1;

void
set_seed(int sd) {
	seed = sd;
}

/* No chance someone else decides to call this file this way, I hope */
static char tempfile[] = "/tmp/check_cfdisk_temporary_disk_drive";
static char tempfile2[] = "/tmp/check_cfdisk_temporary_disk_drive2";

const char *
tempfile_name()
{
	return tempfile;
}

const char *
tempfile2_name()
{
	return tempfile2;
}
/* We should try not to ruin a file, or more importantly, some block device */
static int is_blockdev(const char *file) {
	struct stat file_stat;
	if (stat(file,&file_stat) < 0)
		return -1;
	if(S_ISBLK(file_stat.st_mode))
		return 1;
	else
		return 0;
}

void check_safety () {
	int a = is_blockdev(tempfile);
	int b = is_blockdev(tempfile2);
	if (a > 0 || b > 0) {
		printf("ERROR: Found a block device with the name of our temp "
		       "file. Aborting.");
		exit(1);
	}
	else {
		const char msg[] = "%s exists and will be overwritten. "
		                  "Are you sure you want to continue (y/N)?";
		if (!a) {
			printf(msg, tempfile);
			if (getchar() != 'y' && getchar() != 'Y') {
				exit(0);
			}
		}
		if (!b) {
			printf(msg, tempfile2);
			if (getchar() != 'y' && getchar() != 'Y') {
				exit(0);
			}
		}
	}
	/* Well, there should be no problem, TODO: is it needed? */
	if (!getuid() || !geteuid()) {
		printf("You should not run these tests as root.\n"
		       "Are you sure you want to continue (y/N)? ");
		if (getchar() != 'y' && getchar() != 'Y') {
			exit(0);
		}
	}
}




/* We need an exception handler */
PedExceptionOption ex_opts;
PedExceptionType ex_type;
char exception_error[BUFSIZE];// = "%s";

static int exception_count = 0;

int set_exception_error(const char *format, ...) {
	va_list args;
	char buf[BUFSIZE];
	va_start(args, format);
	if(vsnprintf(buf, BUFSIZE, format, args) < 0) {
		return -1;
	}
	va_end(args);
	return snprintf(exception_error,BUFSIZE, "%s: %s", buf, "%s");
}

int reset_exception_error() {
	strncpy(exception_error, "%s", BUFSIZE);
}

PedExceptionOption
exception_handler (PedException* ex) {
	exception_count++;
	ex_opts = ex->options;
	ex_type = ex->type;
	switch (ex->type) {
		case PED_EXCEPTION_ERROR:
		case PED_EXCEPTION_FATAL:
		case PED_EXCEPTION_BUG:
		case PED_EXCEPTION_NO_FEATURE:
			fail(exception_error, ex->message);
	}
	
	if (ex_opts & PED_EXCEPTION_FIX)
		return PED_EXCEPTION_FIX;
	else if (ex_opts & PED_EXCEPTION_RETRY)
		return PED_EXCEPTION_RETRY;
	else if (ex_opts & PED_EXCEPTION_YES)
		return PED_EXCEPTION_YES;
	else if (ex_opts & PED_EXCEPTION_OK)
		return PED_EXCEPTION_OK;
	else if (ex_opts & PED_EXCEPTION_IGNORE)
		return PED_EXCEPTION_IGNORE;
	else
		return PED_EXCEPTION_UNHANDLED;
	
}


/* Look for functions who handled exceptions incorrectly.
   If this does not go to our exception handler, we have a problem.
   Will check this at the end of each test. */
void test_exception() {
	//printf("\nSo far we got %d exceptions\n", exception_count);
	exception_count = 0;
	if (ped_exception_throw (PED_EXCEPTION_INFORMATION,
	                         PED_EXCEPTION_OK,
	                         "Test") != PED_EXCEPTION_OK)
		fail ("Exception not handled by our handler");
	if (exception_count != 1)
		fail ("Caught %d exceptions instead of 1", exception_count);

}

/* This functions will return what's needed */
/* NOTE: If the string is DEFAULT_STRING, we return the default string */
/* Also note that we have two strings here, if the last character is 1, we
   should use the second string, if it is 2, we should change it to 1, and
   if 0, we should use the first. I know it's ugly, but it works. */
char string[2][BUFSIZE+1];
int string_gets;
int getstring (const char* prompt, char** value, const StrList* str_list, 
               const StrList* loc_str_list, int multi_word)
{
	/* We count how much times this function is called */
	string_gets++;
	/* We check whether the first or the second string should be 
	   used if there is a "queue" of two strings. */
	char *s;
	if (string[0][BUFSIZE] == 1) {
		s = string[1];
		string[0][BUFSIZE] = 0;
	}
	else
		s = string[0];
	if (string[0][BUFSIZE] == 2)
		string[0][BUFSIZE] = 1;
	
	if (!*value || strcmp(s, DEFAULT_STRING)) {
		if (str_list || loc_str_list) {
			int found = 0;
			char *temp = NULL;
			const StrList *c;
			/* Check if the test string is not found in the string list */
			for (c = str_list; !found && c != NULL; c = c->next) {
				temp = str_list_convert_node(c);
				if (!strcmp(temp,s)) found = 1;
				if (temp) free(temp);
			}
			for (c = loc_str_list; !found && c != NULL; c = c->next) {
				temp = str_list_convert_node(c);
				if (!strcmp(temp,s)) found = 1;
				if (temp) free(temp);
			}
			if (!found) return 0;
		}
		if (*value) PED_FREE (*value);
		*value = strdup(s);
	}
	return 1;
}

int integer = 0;
int getint (const char* prompt, int *value)
{
	*value = integer;
	return 1;
}

int bool = 1;
int getbool (const char* prompt, int *value)
{
	*value = bool;
	return 1;
}

PedDevice *device = NULL;
int getdev (const char* prompt, PedDevice** value)
{
	*value = device;
	return 1;
}

PedPartition *partition;
int getpart (const char* prompt, PedDisk **disk, PedPartition** value)
{
	*value = partition;
	return 1;
}

PedDiskType *disk_type;
int getdisktype (const char* prompt, PedDiskType **value)
{
	*value = disk_type;
	return 1;
}

PedFileSystemType *fs_type;
int getfstype (const char* prompt, PedFileSystemType **value)
{
	*value = fs_type;
	return 1;
}

int partpos;
int pos_gets;
char pos_poss[POS_POSS_SIZE];
int getpartpos (const char* prompt, const void* context,
                       const char *possibilities)
{
	/* We count how much times this was called */
	pos_gets++;
	/* And save the possibilities */
	strncpy(pos_poss, possibilities, sizeof(pos_poss));

	return partpos;
}


int
init_tempfile() {
	int i;
	FILE *fp;
	fp = fopen(tempfile,"w");
	if (!fp) return 0;
	fseek (fp, SIZE, SEEK_SET);
	/* NOTE: We want a ~21 GB file, this takes about 22 MB from the disk, this should be 41943039s */
	for (i = 0; i < 9; i++) {
		fseek (fp, SIZE, SEEK_CUR);
	}
	fwrite ("", 1, 1, fp);
	fclose (fp);
	return 1;
	
}

void
unlink_tempfile() {
	unlink(tempfile);
}

int open_device() {
	dev = ped_device_get(tempfile);
	if (!ped_device_open(dev))
		return 0;
	return dev != NULL;
}


#define FAR 128LL
/* If these are too far apart, return false */
int
are_near(long long a, long long b) {
	a -= b;
	if (a < 0LL) a = -a;
	return (a < FAR); 
}

/* I don't believe our tests require this now, but I leave this here */
#if 0
long long llrand() {

	/* We assume that long long is always 64 bits and that int is always 32 bits,
	   as per /usr/include/bits/types.h */

	long long result;
	result = (long long) rand();
	result *= 4294967296;
	result += (long long) rand();
	
	return result;

}
#endif





void
next_part_geometry(PedGeometry **geom,
                   PedConstraint *constraint, int factor) {
	PedSector start;
	PedGeometry *temp;
	if (!*geom)
		 start = 0LL;
	else {
		start = (*geom)->end+1LL;
		ped_geometry_destroy(*geom);
	}
	temp = ped_geometry_new(dev, start, PART_LENGTH*(long long)factor);
	*geom = ped_constraint_solve_nearest(constraint,temp);
	ped_geometry_destroy(temp);
}


int
_mkpart (int how, PedSector start, PedSector end,
                      PedPartitionType type, const PedFileSystemType* fs_type,
                      PedPartition **newpart,
                      int ch_mkpart (PedDisk*, PartPos *pos,
                                     PedPartitionType, const PedFileSystemType*,
                                     PedPartition **, UIOpts))
{
	int status;
	string_gets = 0;
	int strings;
	PartPos pos = { { 0LL, NULL }, { 0LL, NULL } };
	switch(how % 3) {
		/* We create the partition by specifying start and end */
		case 0:
			strings = 1;	
			if (string[0][BUFSIZE] == 1) {
				strings++;
				string[0][BUFSIZE] = 2;
			}
			else
				string[0][BUFSIZE] = 0;
			uiquery.getstring = getstring;
			strncpy(string[0], fs_type->name, BUFSIZE-1);
			
			pos.start.sector = start;
			pos.end.sector = end;
			status = ch_mkpart(disk,&pos,type,NULL,newpart,
			                 UI_DEFAULT);
			fail_unless(string_gets == strings,
				"User queried for %d strings instead of %d",
				string_gets, strings);
			break;
		/* We test that giving the default string is OK */
		case 1:
			strncpy(string[0],DEFAULT_STRING,BUFSIZE);
			uiquery.getstring = getstring;

			pos.start.sector = start;
			pos.end.sector = end;
			status = ch_mkpart(disk,&pos,type,fs_type,newpart,
			                 UI_CUSTOM_VALUES);
			fail_unless(string_gets == 2,
				"User queried for %d strings instead of 2",
				string_gets);
			break;
		/* We query the start and end */
		case 2:
			uiquery.getstring = getstring;
			string[0][BUFSIZE] = 2;
			snprintf(string[0],BUFSIZE,"%llds",start);
			snprintf(string[1],BUFSIZE,"%llds",end);
			status = ch_mkpart(disk,&pos,type,fs_type,newpart,
			                 UI_CUSTOM_VALUES);
			fail_unless(string_gets == 2,
				"User queried for %d strings instead of 2",
				string_gets);
			break;
	}
	return status;
	
}

/* We use this for both resize and move, as they take the same params */
int
_resmov (int how, PedPartition *part, PedSector start, PedSector end,
         UIOpts opts, int action (PedDisk*, PedPartition*,
                                  PartPos *pos, UIOpts))
{
	PartPos pos = { { 0LL, NULL }, { 0LL, NULL } };
	switch (how % 3) {
		/* Specify start, end and partition */
		case 0:
			pos.start.sector = start;
			pos.end.sector = end;
			return action (disk, part, &pos, opts);
		/* We specify start and end, ask for partition */
		case 1:
			uiquery.getint = NULL;
			uiquery.getpart = getpart;
			partition = part;

			pos.start.sector = start;
			pos.end.sector = end;
			return action (disk, NULL, &pos, opts);
		/* We ask for everything, the partition with number */
		case 2:
			uiquery.getpart = NULL;
			uiquery.getstring = getstring;
			uiquery.getint = getint;
			integer = part->num;
			string[0][BUFSIZE] = 2;
			snprintf(string[0],BUFSIZE,"%llds",start);
			snprintf(string[1],BUFSIZE,"%llds",end);
			opts |= UI_CUSTOM_VALUES;
			return action (disk, NULL, &pos, opts);
	}
}

void
get_max_geometry(PedGeometry **geom, PedConstraint *constraint, 
                 PedPartition *part)
{
	PedGeometry *temp = NULL;
	if (*geom) ped_geometry_destroy(*geom);
	PedSector start, end;
	int i = 0;
	/* We check if we have free space before or after the partition */
	if (part->prev && part->prev->type & PED_PARTITION_FREESPACE)
		start = part->prev->geom.start;
	else
		start = part->geom.start;
	if (part->next && part->next->type & PED_PARTITION_FREESPACE)
		end = part->next->geom.end;
	else
		end = part->geom.end;	

	temp = ped_geometry_new(dev, start, end-start+1LL);
	*geom = ped_constraint_solve_nearest(constraint,temp);
	ped_geometry_destroy(temp);

}
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/* NOTE: If you create a rather big test disk, you can make this use llrand 
         from functions.c instead. */
void
randomize_position(PedSector *start, PedSector *end, PedGeometry *where, int i)
{
	/* This chooses random partition position within where */
	/* If i is 0, we place it at the beginning, if i is 3, at the end */
	if (i == 0) {
		*start = where->start;
		do {
			*end = *start+((unsigned int)rand() % where->length)-1;
			*end = MIN(*end,where->end);
		} while (are_near(*start,*end));
		
	}
	else if (i == 3) {
		*end = where->end;
		do {
			*start = *end-((unsigned int)rand() % where->length)+1;
			*start = MAX(*start,where->start);
		} while (are_near(*start,*end));
	}
	else {
		PedSector mid = where->start +
		                ((unsigned int) rand() % where->length)-1;
		do {
			*start = where->start+
				((unsigned int) rand() % (mid - where->start));
			*end = mid +
				((unsigned int) rand() % (where->end - mid));
			*start = MAX(*start,where->start);
			*end = MIN(*end,where->end);
		} while (are_near(*start,*end));
	}
}


/* This is for our size changing checks. We check on all label types */
void
change_size (const char *ltype, UIOpts opts) {

	PedConstraint *constraint = ped_device_get_constraint(dev);
	PedGeometry *geom = NULL;
	PedSector start, end, oldstart = -1, oldend = -1;
	PedPartition *part = NULL;
	PedFileSystemType *fs_type = ped_file_system_type_get("linux-swap");
	int is_mac = !strcmp(ltype,"mac");
	int i;
	fail_unless(perform_mklabel(dev,&disk,ped_disk_type_get(ltype)),
	            "We could create the %s label for resizing partitions",
	            ltype);
	
	/* NOTE: This might blow up if ltype is less than three chars */
	/* FIXME: This checks works differently on LE and BE platforms */
	srand(2 + opts + ((int) ltype));
	/* We will resize create 4 partitions */
	if (is_mac) {
		part = ped_disk_get_partition(disk,1);
		geom = ped_geometry_new(dev,part->geom.start,part->geom.length);
		part = NULL;
		
	}
	for (i = 0; i < 4; i++) {		
		next_part_geometry(&geom, constraint, 1);
		if (ped_disk_type_check_feature(disk->type,
			PED_DISK_TYPE_PARTITION_NAME)) {
			/* 1 means get second string on queue */
			string[0][BUFSIZE] = 1;
			strncpy(string[1],"Label",BUFSIZE);
		}
		else
			string[0][BUFSIZE] = 0;
		uiquery.getstring = getstring;
		set_exception_error("Creating partition %d at %llds-%llds "
			"with previous one at %llds-%llds on %s",
			i, geom->start, geom->end, oldstart, oldend, ltype);
		fail_unless(_mkpart (0, geom->start,  geom->end,
				PED_PARTITION_NORMAL,
 				fs_type, &part, (opts & UI_NO_FS_RESIZE ?
				           perform_mkpart : perform_mkpartfs)),
			"Failed to create partition for resize on %s. "
			"Number %d, size %llds, start %llds, end %llds",
			ltype, i, geom->length, geom->start, geom->end);
		reset_exception_error();
		oldstart = geom->start;
		oldend = geom->end;
	}
	/* And we will resize them, 4 times each, moving them as near possible
	   to end of the disk as a final result */
	while (part && (!is_mac || part->num != 1)) {
		get_max_geometry(&geom,constraint,part);
		for (i = 0; i < 4; i++) {
			randomize_position(&start,&end,geom,i);
			set_exception_error("Resize %d of partition %d "
			            "from %llds-%llds to %llds-%llds "
			            "in region %llds-%llds on %s",
		                    i, part->num, part->geom.start,
			            part->geom.end, start, end, geom->start,
			            geom->end, ltype);
			/* Note that exact values fail on some partition 
			   types, and when we are querying the user,
			   exact values should be used... Or? FIXME? */
			//fail_unless(_resmov (strncmp("ms",ltype,2) ? 0 : i, 
			fail_unless(_resmov (i, part, start,
			                     end, opts, perform_resize),
			            "Could not do %d resize of partition %d "
			            "from %llds-%llds to %llds-%llds "
			            "in region %llds-%llds on %s",
			            i, part->num, part->geom.start,
			            part->geom.end, start, end, geom->start,
			            geom->end, ltype);
			fail_unless(are_near(part->geom.start,start) &&
			           are_near(part->geom.end,end),
			           "The %d resize of partition %d on %s wrong."
			           "we wanted %llds-%llds, we got %llds-%llds",
			           i, part->num, ltype, start, end,
			           part->geom.start, part->geom.end);
			reset_exception_error();
		}
		do {
			part = part->prev;
		} while (part && ((part->type & PED_PARTITION_METADATA) ||
		         (part->type & PED_PARTITION_FREESPACE)));
	}
}
