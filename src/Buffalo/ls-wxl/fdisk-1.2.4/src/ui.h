/*
    parted - a frontend to libparted
    Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.

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

#ifndef FDISK_UI_H_INCLUDED
#define FDISK_UI_H_INCLUDED

#include "strlist.h"
#include "ui.h"
#include "command.h"


/* Struct for a single option, Linux fdisk-style */
struct _Option {
	int option;
	char *description;
};
typedef struct _Option Option;

extern char*	fdisk_prog_name;
extern int	fdisk_opt_script_mode;
extern char *interface_name;
extern void fdisk_usage_msg();
extern void fdisk_help_msg ();
/* extern int fdisk_interactive_mode (PedDisk** disk, FdiskCommand* cmd_list[]); */
extern int fdisk_interactive_mode (PedDevice** device, FdiskCommand* cmd_list[]);
extern int fdisk_interactive_menu (PedDisk** disk, FdiskCommand* cmd_list[], int menu);
extern int fdisk_screen_width ();
extern void fdisk_wipe_line ();
extern PedExceptionOption
fdisk_command_line_get_ex_opt (const char* prompt, PedExceptionOption options);
extern void fdisk_print_options_help ();
extern void fdisk_print_commands_help (FdiskCommand* cmds[]);
extern void fdisk_print_using_dev (PedDevice* dev);
extern int fdisk_command_line_get_word_count ();
extern void fdisk_command_line_prompt_words (const char* prompt, const char* def,
					     const StrList* possibilities, int multi_word);
extern char* fdisk_command_line_pop_word ();
extern void fdisk_command_line_flush ();
extern char* fdisk_command_line_get_word (const char* prompt, const char* def,
					  const StrList* possibilities, int multi_word);
extern void fdisk_command_line_push_line (const char* line, int multi_word);
extern char* fdisk_command_line_pop_word ();
extern void fdisk_command_line_push_word (const char* word);
extern int fdisk_command_line_get_partition (const char* prompt, PedDisk* disk,
					     PedPartition** value);
extern int fdisk_command_line_is_integer ();
extern int command_line_prompt_boolean_question (const char* prompt);
/*extern int fdisk_command_line_get_disk (const char* prompt, PedDisk** value);*/
/*extern int fdisk_command_line_get_partition (const char* prompt, PedDisk* disk,
				      PedPartition** value);*/
/*extern int fdisk_command_line_get_fs_type (const char* prompt,
					   const PedFileSystemType*(* value));*/
/*extern int fdisk_command_line_get_disk_type (const char* prompt, const PedDiskType*(* value));*/
/*extern int fdisk_command_line_get_device (const char* prompt, PedDevice** value);*/
extern int fdisk_command_line_get_integer (const char* prompt, int* value);
extern int fdisk_command_line_get_llinteger (const char* prompt, long long* value);
extern int fdisk_command_line_get_part_type (const char* prompt, const PedDisk* disk,
				      PedPartitionType* type);
extern char* fdisk_command_line_peek_word ();
extern int fdisk_command_line_get_option (const char* head, const Option* opts);
extern int fdisk_get_partpos (const char* prompt, const void* context,
                              const char *possibilities);
/*extern int fdisk_command_line_get_sector (const char* prompt, PedDevice* dev, PedSector* value,
					  PedGeometry** range);*/
extern int fdisk_init_ui ();
extern void fdisk_done_ui ();
extern int fdisk_command_line_get_part_flag (const char* prompt, const PedPartition* part,
				  PedPartitionFlag* flag);
extern int fdisk_command_line_get_unit (const char* prompt, PedUnit* unit);
extern int fdisk_command_line_get_state (const char* prompt, int* value);
#endif /* FDISK_UI_H_INCLUDED */

