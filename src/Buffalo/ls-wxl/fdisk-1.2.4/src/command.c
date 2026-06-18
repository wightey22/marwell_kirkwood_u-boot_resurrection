/*
    GNU fdisk - a clone of Linux fdisk.
    This file originally from GNU Parted.

    Copyright (C) 1999, 2000, 2006 Free Software Foundation, Inc.

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

#include "command.h"
#include "ui.h"

#include <parted/debug.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

FdiskCommand*
fdisk_command_create (const StrList* names,
		int (*method) (PedDisk** disk),
		const StrList* summary,
		const StrList* help,
                const int non_interactive)
{
	FdiskCommand*	cmd;

	cmd = malloc (sizeof (FdiskCommand));

	assert(cmd != NULL);

        if (non_interactive)
                cmd->non_interactive = 1;
        else
                cmd->non_interactive = 0;
                
	cmd->names = (StrList*) names;
	cmd->method = method;
	cmd->summary = (StrList*) summary;
	cmd->help = (StrList*) help;

	return cmd;
}

void
fdisk_command_destroy (FdiskCommand* cmd)
{
	str_list_destroy (cmd->names);
	str_list_destroy (cmd->summary);
	str_list_destroy (cmd->help);
	free (cmd);
}

void
fdisk_command_register (FdiskCommand** list, FdiskCommand* cmd)
{
	int	i;

	for (i = 0; list [i]; i++);
	
	list [i] = cmd;
	list [i + 1] = (FdiskCommand*) NULL;
}

FdiskCommand*
fdisk_command_get (FdiskCommand** list, char* name)
{
	int		i;
	int		partial_match = -1;
	int		ambiguous = 0;

	if (!name)
		return NULL;

	for (i=0; list [i]; i++) {
		switch (str_list_match_any (list [i]->names, name)) {
		case 2:
			return list [i];

		case 1:
			if (!ambiguous) {
				if (partial_match == -1) {
					partial_match = i;
				} else {
					partial_match = -1;
					ambiguous = 1;
				}
			}
		}
	}

	if (partial_match == -1)
		return NULL;
	else
		return list [partial_match];
}

StrList*
fdisk_command_get_names (FdiskCommand** list)
{
	FdiskCommand**	walk;
	StrList*	result = NULL;

	for (walk = list; *walk; walk++)
		result = str_list_join (result,
					str_list_duplicate ((*walk)->names));
	return result;
}

void
fdisk_command_print_summary (FdiskCommand* cmd)
{
          if (cmd) {
	          printf ("  ");
	          str_list_print_wrap (cmd->summary, fdisk_screen_width(), 2, 8);
   	  	  printf ("\n");
	  }
}

void
fdisk_command_print_help (FdiskCommand* cmd)
{
	fdisk_command_print_summary (cmd);
	if (cmd->help) {
		printf ("\n\t");
		str_list_print_wrap (cmd->help, fdisk_screen_width(), 8, 8);
	}
}

int
fdisk_command_run (FdiskCommand* cmd, PedDisk **disk)
{
        return cmd->method (disk);
}

