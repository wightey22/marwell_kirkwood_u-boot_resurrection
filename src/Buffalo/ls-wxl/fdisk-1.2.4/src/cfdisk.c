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
#include "../config.h"
#endif

#include "common.h"
#include "hacks.h"
/*#include "command.h"*/
/*#include "ui.h"*/

#define N_(String) String
#if ENABLE_NLS
#  include <libintl.h>
#  include <locale.h>
/* TODO: Fix some localizing issues */
#  define _(String) dgettext (PACKAGE, String)
/* FIXME: This doesn't seem to work */
#  define P_(String) dgettext ("parted", String)
#else
#  define _(String) (String)
#  define P_(String) (String)
#endif /* ENABLE_NLS */

#include <parted/parted.h>
#include <parted/debug.h>


#if HAVE_NCURSES_H
  #include <ncurses.h>
#else
  #include <curses.h>
#endif

/* FIXME: I'm not sure when these are available, and what portability issues they will raise
          Most likely this is not needed at all */
#ifdef KEY_MIN
#define USE_KEYPAD 1
#else
#define USE_KEYPAD 0
/* Like Linux cfdisk */
#define KEY_UP 1
#define KEY_DOWN 2
#define KEY_LEFT 3
#define KEY_RIGHT 4
#define KEY_BACKSPACE '\b'
#define KEY_DC '\177'
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif




static const char* prog_name = "cfdisk (GNU fdisk) " VERSION "\n";
static const char* license_msg = N_(
	"Copyright (C) 2006 Free Software Foundation, Inc.\n"
	"This is free software.  You may redistribute copies of it under the terms of\n"
	"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.\n"
	"There is NO WARRANTY, to the extent permitted by law.\n"
);
static const char* usage_msg = N_(
	"Usage: cfdisk [OPTION]... [DEVICE]\n"
);
/* TODO: Should we move to argp? */
static struct { const char* lopt; const char opt; const char *arg; const char *help; } options_help[] = 
{
	{"help",	'h', NULL,	N_("displays this help message")},
	{"version",	'v', NULL,	N_("displays the version")},
	{"arrow-cursor", 'a', NULL, 	N_("use an arrow cursor instead of reverse video")},
	{"new-table",	'z', NULL,	N_("create a new partition table on the disk")},
	{"units",	'u', N_("UNIT"),N_("sets the default display units to UNIT")},
	{"list-partition-types", 't', NULL, N_("displays a list of supported partition types")},
        {NULL, 0, NULL, NULL}
};

/* Help should go here */
static const char *help = N_(
	"This is a curses-based disk partition manipulation program, which "
	"allows you create, destroy, resize, move and copy partitions on "
	"a hard drive.\n\n"
	"Copyright (C) 2006 Free Software Foundation, Inc.\n\n"
	"Key\tFunction\n"
	"---\t--------\n\n"
	" n\tCreate a new partition residing on the free space.\n"
	" d\tDelete the selected partition.\n"
	" c\tCheck the selected partition for consistency.\n"
	" r\tResize the selected partition or change the size of the "
	"entry in the partition table\n"
	" o\tMove the partition to another place on the drive.\n"
	" y\tCopies another partition over the selected one.\n"
	" s\tLook for deleted file systems in the selected free space.\n"
	" b\tDisplay or change partition flags, such as the bootable flag.\n"
	" t\tChange the system type on the partition in the partition "
	"in case it is currently wrong.\n"
	" m\tChange the partition name, if supported.\n"
	" x\tMaximize the extended partition.\n"
	" z\tMinimize the extended partition.\n"
	" u\tChange the display units.\n"
	" i\tDisplay partition info.\n"
	" w\tWrite the changes to the disk.\n"
	" q\tQuit the program.\n"
	" h\tDisplay this help.\n"
);

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

/* Keys */
#define BELL	'\007'	/* ^G */
#define TAB	'\011'	/* ^I */
#define CR	'\015'	/* ^M */
#define ESC	'\033'	/* ^[ */
#define CTRL_L	'\014'	/* ^L */
#define CTRL_P	'\020'	/* ^P */
#define CTRL_N	'\016'	/* ^N */
#define CTRL_R	'\022'	/* ^R */


#define MENUSIZE 3
#define INFOSIZE 5
#define MENUDIV 1




enum _MenuOptions {
	MENU_DEFAULT = 0,
	MENU_TITLE = 1,  /* Leave one row for menu title */
	MENU_LIST = 2,   /* Unimplemented */
	MENU_ANYKEY = 4,
	MENU_BUTTON = 8,
	MENU_LEAVE_WARNING = 16, /* Don't clear the warning line */
	MENU_NOHIGHLIGHT = 32, /* Do not highlight the selected element in the menu */
	MENU_ARROWS = 64 /* Accept arrow keys as valid */
};

typedef enum _MenuOptions MenuOptions;

#define TINYBUF 16	/* NOTE: Changing this may lead to buffer overflows. Double-check. */
#define FSBUF 19	/* Don't touch unless you know what you are doing!! */
#define SMALLBUF 512
#define MAXFLAGS 63

/* struct for a menu item. I got the idea from Linux cfdisk. Every action is bound to a key. */
struct _MenuItem {
	char key;
	const char *name;
	const char *desc; 
};
typedef struct _MenuItem MenuItem;

/* Struct for the opened device */
struct _Context {
	PedDevice *dev;
	PedDisk *disk;
	int is_devicefile;
	char *devname;
	char *info_size;
	char *info_sectorsize;
	char *info_model;
	char *info_heads;
	char *info_cylinders;
	char *info_sectors;
	char *info_cylsize;
};
typedef struct _Context Context;

Context *init_disk(int new_table, char *devname, PedDevice *dev, PedDiskType *type);

int new_table = 0;
int arrow_cursor = 0;





static void
do_quit(int status, const char *message) {
	endwin();
	if (message) {
		printf("%s\n",message);
	}
	exit(status);
}


static int
get_center(const char *string) {
	int x = strlen(string);
	x = MIN(x,COLS);
	x = (COLS-x)/2;
	return x;
}



static void
clear_lines(int start, int end) {
	int y = start;
	do {
		move (y,0);
		clrtoeol();
	} while(y++ < end);
}
/* Define these as macros */
#define clear_status(warn) clear_lines(LINES - 1 - ((warn) != 0),0)
#define clear_menu() clear_lines(LINES - 2 - MENUSIZE, LINES - 1);

static void
set_status(const char *status, int warn_line) {
	clear_status(warn_line);
	/*attron(A_REVERSE);*/
	move(LINES-1-warn_line,0);
	mvaddstr(LINES-1-warn_line, get_center(status) ,status);
	/*attroff(A_REVERSE);*/
}

static void
print_warning(const char *status, int status_line) {
	set_status(status,!status_line);
	putchar(BELL);
}

static void
menu_title (const char *title) {
	move(LINES - 2 - MENUSIZE, 0);
	clrtoeol();
	mvaddstr(LINES - 2 - MENUSIZE, MENUDIV, title);	
}


enum _MsgOpts {
	DOUBLE_NEW_LINE = 1,
	/* Ignore space at the beginning of the line */
	IGNORE_SPACE = 2,
	/* Use \t to indent the whole text, even after newline */
	TAB_TO_INDENT = 4
	
};
typedef enum _MsgOpts MsgOpts;

/* This is an ancient magic that turns a char* into a StrList, so we can display it */
static StrList*
_message_display_strlist(const char *msg, int *lines, MsgOpts opts) {
	int x = 0, y = 0, lastword = 0, isnewline = 0, i, indent = 0;
	StrList *message = NULL;
	char buf[SMALLBUF];
	/* We save the lines in a string list, counting them */
	/* TODO: It doesn't deal with tabs correctly, this is currently
           only with tabs in the beginning of the text. */
	/* NOTE: We calculate indent, even when we don't use it */
	for (i = 0; msg[i]; i++) {
		/* If we reached the end of the line, move to the next line */
		if (x >= SMALLBUF - 1 || x >= COLS-2) {
			if (isblank(msg[i]) || msg[i] == '\r' ||
			    msg[i] == '\n' || !lastword) {
				buf[x] = '\0';
				message = str_list_append(message,buf);
				y++;
				/* Real new line is two lines. */
				if (msg[i] == '\n') { 
					if (opts & DOUBLE_NEW_LINE)
						message = str_list_append(
								message,"");
					y++;
					isnewline = 1;
				}
			}
			/* If this is not the end of the word, go back to the beginning of the word. */
			else {
				buf[lastword-1] = '\0';
				message = str_list_append(message,buf);
				y++;
				i = i-x+lastword;
			}
			lastword = 0;
			x = 0;
			/* If the text was indented, move it right */
			if (msg[i] == '\n')
				indent = 0;
			else if ((opts & TAB_TO_INDENT) && indent > 0) {
				/* If the line is indented */
				for (; x <= indent && x < SMALLBUF - 1 ; x++) {
					buf[x] = ' ';
				}
			}
		}
		/* We ignore carriage returns */
		if (msg[i] == '\r') {
			continue;
		}
		/* We indent the text right, if we see a tab */
		else if (msg[i] == '\t') {
			lastword = x+1;
			int n = ((x+9)/9)*9;
			n = MIN(COLS-2,n);
			for (; x <= n && x < SMALLBUF - 1; x++) {
				buf[x] = ' ';
			}
			indent += 9;
			continue;
		}
		/* If the line begins with a blank, skip it */
		else if (isblank(msg[i])) {
			if ((opts & IGNORE_SPACE) && x == 0) continue;
			else lastword = x+1;
		}
		else if (msg[i] == '\n') {
			if (!isnewline) {
				buf[x] = '\0';
				message = str_list_append(message,buf);
				if (opts & DOUBLE_NEW_LINE)
					message = str_list_append(message,"");
				y += 2;
			}
			x = 0;
			lastword = 0;
			indent = 0;
			continue;
		}
		
		
		isnewline = 0;
		buf[x] = msg[i];
		x++;
	}
	buf[x] = '\0'; /* This doesn't produce an error, trust me */
	if (strlen(buf)) { 
		message = str_list_append(message,buf);
		y++;
	}
	if (lines) *lines = y;
	return message;
}
static void
strlist_draw (const char *prompt, const StrList *strlist, int opts, const StrList* selected, 
              int selnum, int *start);
static const StrList*
do_strlist (const char *prompt, const StrList *strlist, int opts);



static void
_display_strlist (const char* title, const StrList *message, int y) {
	int x, i;
	/* TODO: Write this, for now I will test it this way */
	y=LINES-3-MENUSIZE-y;
	if (y < INFOSIZE+4)
		y = INFOSIZE+4;
	i=INFOSIZE+1;
	move(i++,0); clrtoeol();
	mvaddch(i,0,' ');
	for (x = 1; x < COLS-1; x++) {
		mvaddch(i,x,'-'); 
        }
	mvaddch(i,0,' ');
	for (i++; i < y; i++) {
		move(i,0); clrtoeol();
	}
	mvaddstr(y-3,1,title);
	for (x = 1; title[x-1]; x++) {
		mvaddch(y-2,x,'=');
	}
	
	const StrList *current;
	for (current = message; current && y < LINES-MENUSIZE-3; current = current->next, y++) {
		char *temp = str_list_convert_node(current);
		move(y,0); clrtoeol();
		mvaddstr(y,1,temp);
		free(temp);
	}

}


static void 
display_message (const char* title, const char *msg) {
	int lines;
	StrList *message;
	message = _message_display_strlist(msg,&lines,IGNORE_SPACE | DOUBLE_NEW_LINE);
	_display_strlist(title, message, lines);
	str_list_destroy(message);
	
}

/* Menu drawing function */
static void
menu_draw (const MenuItem *items, int item_len, MenuOptions opts, const char *keys, int selected) {
	int i, y = LINES - 2 - MENUSIZE + (opts & MENU_TITLE ? 1 : 0), x = MENUDIV;
	int item_size = item_len + (opts & MENU_BUTTON ? 2 : 0);
	const char *desc;
	move(y,0);
	clrtoeol();
	for (i = 0; items[i].name; i++) {
		/* NOTE: Was tolower(items[i].key) */
		if (strchr(keys, items[i].key)) {
			char buf[item_size+2];
			int name_len;
			const char *name;
	
			/* Localize the text. Translate empty string to empty string. */
			name = (items[i].name[0] ? _(items[i].name) : "");
	
			name_len = strlen(name);
			/* Enclose the text if the type is a button. If it is too long, cut. */
			if (name_len >= item_len) {
				snprintf(buf,item_size+1, (opts & MENU_BUTTON) ? "[%s]" : "%s" , name);
				if (opts & MENU_BUTTON) {
					buf[item_size-1] = ']';
					buf[item_size] = '\0';
				}
			}
			else {
				snprintf(buf,item_size+1, (opts & MENU_BUTTON) ? "[%*s%s%*s]" : "%*s%s%*s" ,
				(item_len - name_len)/2, "", name, (item_len - name_len + 1)/2 , "");
			}
	
			/* Write the button. Highlight if is selected. */
			
			if (i == selected && !(opts & MENU_NOHIGHLIGHT)) attron(A_STANDOUT);
			//else if (strchr(keys,tolower(items[i].key))) attron(A_BOLD);
	
			mvaddstr(y,x,buf);
	
			if (i == selected && !(opts & MENU_NOHIGHLIGHT)) attroff(A_STANDOUT);
			//else if (strchr(keys,tolower(items[i].key))) attroff(A_BOLD);
			
			if (x + 2*item_size + 2*MENUDIV < COLS) {
				x += item_size + MENUDIV;
			}
			else if (y + 3 < LINES) { /* y + 1 < LINES - 2 */
				y++;
				x = MENUDIV;
				move(y,0);
				clrtoeol();
			}
			/* If we can't draw all the menu items, end. */
			else break;
		}
	}
	while (++y < LINES-2) {
		move(y,0);
		clrtoeol();
	}
	if (!(opts & MENU_LEAVE_WARNING)) {
		move(LINES-2,0);
		clrtoeol();
	}
	move(LINES-1,0);
	clrtoeol();
	/* If the selected item exits, print its description */
	if (selected < i) {
		desc = _(items[selected].desc);
		set_status(desc,0);
	}
	refresh();
}

/* TODO: Not perfect */
static is_key_ok (int key, const char* ok_keys, MenuOptions opts) {

	if (opts & MENU_ARROWS) {
		switch (key) {
			case KEY_UP:
			case KEY_DOWN:
			case KEY_LEFT:
			case KEY_RIGHT:
				return 1;
		}
	}
#if USE_KEYPAD
	if (key > 255)
		return 0;
#endif
	return strchr(ok_keys,key) != NULL;
}

/* Menu display function. The selected parameter specifies the item that is selected by
 * default and the keys string includes the keys that should be accepted by the interface. */
static int
do_menu (const MenuItem *items, int item_len, MenuOptions opts, const char *keys, int *sel) {
	int i, key = 0, count = 0, redraw = 1, save_selection = 0, selected = 0;
	if (sel) {
		selected = *sel;
	}
	/* Count the items and make sure that the selected item is available */
	for (count = 0; items[count].name; count++);
	if (selected >= count)
		selected = 0;
	i = 0;
	/* NOTE: Was tolower(items[selected].key) */
	while (!strchr(keys,items[selected].key)) {
		selected = (selected + 1) % count;
		if (i++ > count) {
			selected = 0;
			break;
		}
	}
	/* If the selected menu item was not not available, we save the pointer to change it back */
	/* FIXME: The menu acts funny. Re-order the items so that it works better :) */
	//if (selected == saved_selection) save_selection = 1;
	while (!key) {
		if (redraw) {
			menu_draw (items, item_len, opts, keys, selected);
			opts &= ~MENU_LEAVE_WARNING; /* Clear the warning next time */
			redraw = 0;
		}
		key = getch();
#if USE_KEYPAD == 0
		if (key == ESC) {
			key = getch();
			if (key == '[' || key == 'O') {
				key = getch();
				if (key == 'A') { /* ^[[A = Up arrow */
					key = KEY_UP;
				}
				else if (key == 'B') {	/* ^[[B = Down arrow */
					key = KEY_DOWN;
				}
				else if (key == 'C') {	/* ^[[C = Right arrow */
					key = KEY_RIGHT;
				}
				else if (key == 'D') { /* ^[[D = Left arrow */
					key = KEY_LEFT;
				}
			}
		}
#endif
		if (key == KEY_RIGHT) {
			do {
				selected = (selected + 1) % count;
			/* NOTE: Was tolower(items[selected].key) */
			} while (!strchr(keys,items[selected].key));
			key = 0;
			redraw = 1;
			save_selection = 1;
		}
		else if (key == KEY_LEFT) {
			do {
				selected -= 1;
				if (selected < 0) selected += count;
			/* NOTE: Was tolower(items[selected].key) */
			} while (!strchr(keys,items[selected].key));
			key = 0;
			redraw = 1;
			save_selection = 1;
		}
		else if (key == CR) {
			key = items[selected].key;
		}
		/* If the pressed key is in the list of accepted keys, print a warning */
		/* NOTE: Was tolower(key) */
		if(!(opts & MENU_ANYKEY) && !is_key_ok(key,keys,opts)) {
			key = 0;
			print_warning(_("Invalid key"),0);
		}
	}
	if (sel && save_selection)
		*sel = selected;
	return key;
}

/* Lines at the top of the string list drawer */
#define SLD_HEAD 5
/* String list choice */
/* The last two parameters specify the number of the selected element and the first element displayed 
   last time. If opts & 1, we assume it is a fullscreen text, not choice*/
static void
strlist_draw (const char *prompt, const StrList *strlist, int opts, const StrList* selected, 
              int selnum, int *start) {
	char *temp;
	int n,x,y,i;
	const StrList *current;

	if (opts & 1) {
		*start = selnum - (LINES - 2 - SLD_HEAD);
		*start = MAX(0,*start);
		y = 0;
		n = LINES - 2;
	}
	else {

		n = LINES-MENUSIZE-3;
		y = INFOSIZE;
		if (selnum - *start < 0)
			*start = selnum;
		else if (selnum - *start > n - INFOSIZE - SLD_HEAD)
			*start = selnum - n + INFOSIZE + SLD_HEAD;

	}
	move(y++,0); clrtoeol();
	move(y,0); clrtoeol(); mvaddstr(y++,5,prompt);
	mvaddch(y,0,' ');
	for (x = 1; x < COLS-1; x++) {
		mvaddch(y,x,'-');
        }
	mvaddch(y++,x,' ');
	move(y++,0); clrtoeol();
	x = (opts & 1 ? 1 : 7);
	for (current = strlist, i = 0; current && y < n; current = current->next, i++) {
		move(y,0); clrtoeol();
		if (i < *start)
			continue;
		if (!(opts & 1) && current == selected) {
			if (arrow_cursor)
				mvaddstr(y,3,"-->");
			else
				attron(A_STANDOUT);
		}
		
		temp = str_list_convert_node(current);
		mvaddstr(y,x,temp);
		if (temp) free(temp);
		if (!(opts & 1) && current == selected && !arrow_cursor)
			attroff(A_STANDOUT);
		y++;
	}
	/* We clear the next line. No need to clean all lines, unless we implement page up and down */
	while (y < n) {
		move(y,0); clrtoeol();
		y++;
	} 
}
static void show_info(Context *c);
static void plist_draw (Context *c, PedPartition *selected, int selnum, int *start);
static int do_plist (Context *c, PedPartition **part, PedPartitionType have, PedPartitionType havent);

static const StrList*
do_strlist (const char *prompt, const StrList *strlist, int opts) {
	const StrList *selected = strlist, *temp, *next;
	int redraw = 1, key = 0, done = 0, start = 0, selnum = 0, i;
	int move_down_by;
	const char* end_warning = opts & 1 ? 
	                              _("No more text") : _("No more choices");
	if (opts & 1) {
		for (selnum = 0, selected = strlist; 
		     selnum < LINES-2-SLD_HEAD && selected->next;
		     selnum++, selected = selected->next);
	}
	while (!done) {
		if (redraw) {
			clear_menu();
			strlist_draw(prompt, strlist, opts, selected, selnum,&start);
			redraw = 0;
		}
		refresh();
		key = getch();
#if USE_KEYPAD == 0
		if (key == ESC) {
			key = getch();
			if (key == '[' || key == 'O') {
			key = getch();
				if (key == 'B' && !(opts & 2)) { /*Down arrow*/
					key = KEY_UP;
				}
				else
				if (key == 'A' && !(opts & 2)) { /*Up arrow*/
					key = KEY_DOWN;
				}
				else if (opts & 1)
					done = 1;
				else
					print_warning(_("Invalid key"),0);
			}
			else if (opts & 1)
				done = 1;
			else
				print_warning(_("Invalid key"),0);
		}
#endif

		/* We want to move 1 element when an arrow is pressed */
		move_down_by = 1;
		switch (key) {

#if USE_KEYPAD
			case KEY_NPAGE:
				if (opts & 1)
					move_down_by = LINES - SLD_HEAD - 4;
				else 
					move_down_by = LINES - MENUSIZE -
					               INFOSIZE - SLD_HEAD - 5;
#endif
			case KEY_DOWN:
				if (!selected->next) {
					print_warning(end_warning,0);
					break;
				}
				for (i = 0; i < move_down_by && selected->next; i++) {
						selected = selected->next;
				}
				selnum += i;
				redraw = 1;
				break;
#if USE_KEYPAD
			case KEY_PPAGE:
				if (opts & 1)
					move_down_by = LINES - SLD_HEAD - 4;
				else 
					move_down_by = LINES - MENUSIZE -
					               INFOSIZE - SLD_HEAD - 5;
#endif
			case KEY_UP:
				if (strlist == selected || 
				    ((opts & 1) && selnum <= LINES-2-SLD_HEAD)) {
					print_warning(end_warning,0);
					break;
				}
				
				/* FIXME: This might be improved by editing strlist */
				/* (next represents temp->next->next->...->next) */
				/* We set temp and next move_down_by elements apart, and move
				   them both, until next reaches the current element,
				   then temp points to the element n elements before it */
				temp = strlist;
				/* We set temp and next to n elements apart */
				for (i = 0, next = strlist;
				     i < move_down_by && next && next != selected;
				     i++, next = next->next);
				/* Move them together until next is our
				   current element, which means that temp
				   points to the element n elements before
				   it */
				while (temp && next && next != selected) {
					temp = temp->next;
					next = next->next;
				}
				/* This point should be never happen */
				if (next != selected) {
					print_warning(_("Bug in the program. "
							"The programmer is an idiot."),0);
				}
				else {
					selnum -= i;
					selected = temp;
					redraw = 1;
				}
				break;
			case CR:
				done = 1;
				break;
			default:
				if (opts & 1)
					done = 1;
				else
					print_warning(_("Invalid key"),0);
		}
	}
	/* We redraw partition info */
	show_info(NULL);
	plist_draw(NULL, NULL, 0, 0);
	return selected;
}


static int
read_char (const char* prompt, char **value) {
	char buf[SMALLBUF];
	int y = LINES - 2 - MENUSIZE, i, key = 0, done = 0,x;
	while (y < LINES) {
		move(y,0);
		clrtoeol();
		y++;
	}
	y = LINES - 1 - MENUSIZE;
	int n = SMALLBUF;
	if (*value) {
		i = strlen(*value);
		i = MIN(i,n);
		strncpy(buf,*value,i);
	}
	else i = 0;
	buf[i] = '\0';
	mvaddstr(y,MENUDIV,prompt); addstr(": ");
	getyx(stdscr,y,x);
	n = MIN(n,COLS-x);
	addstr(buf);
	getyx(stdscr,y,x);
	
	refresh();
	while(!done) {
		key = getch();
		if (key == ESC) {
#if USE_KEYPAD == 0
			if (getch() == ESC) 
				return 0;
			else getch();
#else
			return 0;
#endif
		}
		else if (key == KEY_BACKSPACE || 
		         key == KEY_DC ||
		         /* Fix for misconfigured terminals */
		         key == '\b' ||
		         key == '\177') {
			if (i > 0) {
				buf[--i] = '\0';
				move(y,--x);
				delch();
			}
			else
				print_warning(_("No more characters to delete"),1);
		}
		else if (key == CR) {
			done = 1;
		}
		else if (isprint(key)) {
			if (i < n-1) {
				buf[i++] = key;
				buf[i] = '\0';
				mvaddch(y,x++,key);
			}
			else
				print_warning(_("String too long"),1);
		}
		refresh();
	}
	if (*value) PED_FREE (*value);
	*value = strdup(buf);
	return 1;
	
}


static UICalls uiquery;
static int
getbool(const char* prompt, int* value) {
	int result;
	int menudef = !*value;
	MenuItem yes_no[] = {
		{'y', N_("Yes"), "" },
		{'n', N_("No"), "" },
		{0, NULL, NULL }
	};
	menu_title(prompt);
	result = do_menu(yes_no, 5, MENU_BUTTON | MENU_TITLE, "yn", &menudef);
	*value = (result == 'y');
	return 1;
}

static int
getstring (const char* prompt, char **value, const StrList* words, const StrList* locwords, int mword) {

	const StrList *selected, *walk, *locwalk;
	if (words || locwords) {
		if(locwords) {
			selected = do_strlist(prompt,locwords,0);
			if (words) {
				for (walk = words, locwalk = locwords;
				     walk && locwalk;
				     walk = walk->next, locwalk = locwalk->next)
				{
					if (selected == locwalk) {
						selected = walk;
						break;
					}
				}
			}
		}
		else
			selected = do_strlist(prompt,words,0);
		if (*value) PED_FREE(*value);
		*value = str_list_convert_node(selected);
		return (*value != NULL);
	}
	else

		return read_char(prompt,value);
}

static int
getpart (const char* prompt, PedDisk **disk, PedPartition **value) {
	/* NOTE: We should prompt the user for a disk in this case,
	   but this will never happen in cfdisk. Also note that *disk
	   is untouched in this function. */
	if (!*disk) {
		
		return 0;
	}
	Context choice_context;
	choice_context.disk = *disk;
	menu_title(prompt);
	return do_plist(&choice_context, value, 0, 
	                PED_PARTITION_EXTENDED | PED_PARTITION_FREESPACE);
}

static int
getpartpos (const char* prompt, const void* context, const char *possibilities)
{
	menu_title(prompt);
	return do_menu((MenuItem*) context, 11, MENU_BUTTON | MENU_TITLE,
	               possibilities, NULL);
}

static PedExceptionOption
exception_handler (PedException* ex) {
	static MenuItem ex_choices[] = {
		{ 'f', N_("Fix"), "" },
		{ 'y', N_("Yes"), "" },
		{ 'n', N_("No"), "" },
		{ 'o', N_("OK"), "" },
		{ 'r', N_("Retry"), "" },
		{ 'i', N_("Ignore"), "" },
		{ 'c', N_("Cancel"), "" },
		{ 0, NULL, NULL }
	};
	/* Adding new elements to ex_key may lead to segfaults. Double check. */
	static const struct { char key; PedExceptionOption option; }  ex_keys [] = {
		{ 'f', PED_EXCEPTION_FIX },
		{ 'y', PED_EXCEPTION_YES },
		{ 'n', PED_EXCEPTION_NO },
		{ 'o', PED_EXCEPTION_OK },
		{ 'r', PED_EXCEPTION_RETRY },
		{ 'i', PED_EXCEPTION_IGNORE },
		{ 'c', PED_EXCEPTION_CANCEL },
		{ 0, 0 }
	};
	char keys[TINYBUF];
	int res,i,k = 0;
	char *title;
	switch (ex->type) {
		case PED_EXCEPTION_INFORMATION:
			title = _("Information");
			break;
		case PED_EXCEPTION_WARNING:
			title = _("Warning!");
			break;
		case PED_EXCEPTION_ERROR:
			title = _("Error!");
			break;
		case PED_EXCEPTION_FATAL:
			title = _("Fatal exception!");
			break;
		case PED_EXCEPTION_BUG:
			title = _("Bug in the program");
			break;
		case PED_EXCEPTION_NO_FEATURE:
			title = _("Unsupported");
			break;
	}
	for (i = 0; ex_keys[i].key; i++)
		if (ex->options & ex_keys[i].option) {
#if 0
			/* NOTE: This is not needed now, but have in mind */
			if (k >= TINYBUF) break; 
#endif
			keys[k++] = ex_keys[i].key;
		}
#if 0
	if (k < TINYBUF)
#endif
	keys[k] = '\0';
	display_message(title,ex->message);
	res = do_menu(ex_choices, 8, MENU_BUTTON, keys, NULL);
	for (i = 0; ex_keys[i].key; i++)
		if (ex_keys[i].key == res)
			return ex_keys[i].option;
	return PED_EXCEPTION_UNHANDLED;
}

typedef struct {
	time_t	last_update;
	time_t	predicted_time_left;
} TimerContext;

static TimerContext timer_context;

static void
_timer_handler (PedTimer* timer, void* context) {
	char buf[SMALLBUF];
	TimerContext*	tcontext = (TimerContext*) context;
	int		draw;


	if (tcontext->last_update != timer->now && timer->now > timer->start) {
		tcontext->predicted_time_left = timer->predicted_end - timer->now;
		tcontext->last_update = timer->now;
		draw = 1;
	} else {
		draw = 0;
	}

        if (draw) {
		if(timer->state_name)
			snprintf(buf,SMALLBUF,_("We are now %s."), timer->state_name);
		else
			snprintf(buf,SMALLBUF,_("An operation is now taking place."));
		set_status(buf,0);
		snprintf(buf,SMALLBUF,_("Progress: %3.1f%%   Time left: %4.2ld:%.2ld"),
		         100.0 * timer->frac,
		         tcontext->predicted_time_left / 60,
		         tcontext->predicted_time_left % 60); 
		set_status(buf,1);
		refresh();
        }

}


static void
init_calls() {
	uiquery.getbool = getbool;
	uiquery.getstring = getstring;
	uiquery.getpart = getpart;
	uiquery.getpartpos = getpartpos;
	uiquery.getparttype = NULL;
	uiquery.getdisktype = NULL;
	uiquery.getint = NULL;
	uiquery.getdev = NULL;
	uiquery.getfstype = NULL;
	uiquery.need_commit = 0;
	uiquery.timer = ped_timer_new (_timer_handler, &timer_context);
	ped_exception_set_handler(exception_handler);
	set_uicalls(&uiquery);
	init_fs_type_str ();
	init_disk_type_str ();
}

static void
notice_waitkey(const char *warning) {
	static MenuItem waitkey[] = {
		{ 'k', "", N_("Press any key to continue") },
		{ 0, NULL, NULL }
	};
	if (warning)
		waitkey[0].name = warning;
	do_menu(waitkey,COLS-MENUDIV*2,MENU_ANYKEY | MENU_NOHIGHLIGHT,"k",NULL);
}

static void
warning_waitkey(const char *warning) {
	putchar(BELL);
	notice_waitkey(warning);
}


/* Emergency function. If some copy, move or resize fail, reread the disk from the device
   They commited it before failing, anyway. It is a bit like a hack, I might make the functions
   make it themselves. I actually did, but I had a little trouble with the UI, so for now it is this way
 */
/* FIXME: It would be better if we created a copy of the disk, or implemented queuing */
static int
_disk_reread(Context *c, PedPartition **part) {
	int partnum;
	if (*part) {
		partnum = (*part)->num;
	}
	ped_disk_destroy(c->disk);
	c->disk = ped_disk_new(c->dev);
	uiquery.need_commit = 0;
	if (part) {
		*part = ped_disk_get_partition(c->disk,partnum);
	}
}


/* Here we define our menu operations */
static int
do_new (Context *c, PedPartition **part) {
	char bsd_disklabel = 0x00;

	PedPartition *temp = disk_get_prev_nmd_partition (c->disk,*part);
	static MenuItem part_position[] = {
		{ 's', N_("Begining"), N_("Create the partition at the begining of the free space") },
		{ 'e', N_("End"), N_("Create the partition at the end of the free space") },
		{ 'c', N_("Custom"), N_("Select custom start and end position of the partition") },
		{ ESC, N_("Cancel"), N_("Do not create a partition") },
		{ 0, NULL, NULL }
	};
	static MenuItem part_type[] = {
		{ 'p', N_("Primary"), N_("Create primary partition") },
		{ 'e', N_("Extended"), N_("Create extended partition") },
		{ 'b', N_("BSD disklabel"), N_("Create a new BSD disklabel") },
		{ ESC, N_("Cancel"), N_("Do not create a partition") },
		{ 0, NULL, NULL }
	};
	static MenuItem bsd_type[] = {
		{ 'f', N_("FreeBSD"), N_("Create a FreeBSD partition type") },
		{ 'o', N_("OpenBSD"), N_("Create a OpenBSD partition type") },
		{ 'n', N_("NetBSD"), N_("Create a NetBSD partition type") },
		{ ESC, N_("Cancel"), N_("Do not create a partition") },
		{ 0, NULL, NULL }
	};
	int i = 0;
	const char bsd_keys[] = { 'f', 'o', 'n', ESC, '\0' };
	char type_keys[5];
	type_keys[i++] = ESC;
	type_keys[i++] = 'p'; /* We can always create a regular partition */
	type_keys[i++] = 'b'; /* The BSD disklabel can be counted as a regular partition */
	PedPartitionType type = 0;
	PartPos pos;
	int key,success = 1,done;
	UIOpts opts = UI_DEFAULT;
	if (!((*part)->type & PED_PARTITION_FREESPACE)) {
		warning_waitkey(N_("Report a bug in the function menu_new and win a cookie."));
		return 0;
	}
	/* If the free space rests inside the extended partition, this is true, make use of it */
	if((*part)->type & PED_PARTITION_LOGICAL) 
		type = PED_PARTITION_LOGICAL;
	else {
		if(!can_create_primary(c->disk)) {
			warning_waitkey(N_("Cannot create more primary partitions"));
			return 0;
		}
		else if (can_create_extended(c->disk)) {
			type_keys[i++] = 'e';
		}
		type_keys[i] = '\0';
			menu_title(_("Type of partition you want to create"));
			key = do_menu(part_type, 8, MENU_BUTTON | MENU_TITLE, type_keys, NULL);
			if (key == ESC)	return 1;
			else if (key == 'p') type = PED_PARTITION_NORMAL;
			else if (key == 'e') type = PED_PARTITION_EXTENDED;
		else if (key == 'b') {
			type = PED_PARTITION_NORMAL;
			key = do_menu(bsd_type, 8, MENU_BUTTON | MENU_TITLE, bsd_keys, NULL);
			if (key == ESC) return 1;
			else if (key == 'f') bsd_disklabel = 0xa5;
			else if (key == 'o') bsd_disklabel = 0xa6;
			else if (key == 'n') bsd_disklabel = 0xa9;
		}

	}

	pos.start.sector = (*part)->geom.start;
	pos.end.sector = (*part)->geom.end;
	if(!query_part_position(_("Where do you want to put the partition"),
	                        part_position,&pos,pos.start.sector,
	                        pos.end.sector,c->dev,NULL,&opts))
		return 0;

	PedFileSystemType *fs_type = ped_file_system_type_get("ext2");
	if (!perform_mkpart(c->disk,&pos,type,bsd_disklabel ? fs_type : NULL,part,opts)) {
		if (!*part) {
			do {
				temp = ped_disk_next_partition(c->disk,temp);
			} while (temp && temp->type & PED_PARTITION_METADATA);
			*part = temp;
		}
		warning_waitkey(N_("Can't create partition."));
		return 0;
	}
	if (bsd_disklabel) {
		(*part)->fs_type = NULL;
		set_disk_specific_system_type(*part, bsd_disklabel);
	}
	return 1;	
}

do_edit (Context *c, PedPartition **part) {
	char devname[SMALLBUF];
	PedDevice *dev;
	if (uiquery.need_commit) {
	        ped_exception_throw (
				     PED_EXCEPTION_ERROR,
				     PED_EXCEPTION_CANCEL,
				     _("Editing a BSD label, before writing the msdos "
			               "partition table is not supported in GNU fdisk."));
		return 0;
	}
	if (!ped_device_begin_external_access(c->disk->dev))
		return 0;
	/* Warn if there is a filesystem on the partition */
	if (ped_file_system_probe(&((*part)->geom))) {
		if (ped_exception_throw(PED_EXCEPTION_WARNING,
					PED_EXCEPTION_YES_NO,
		     _("This partition seems to contain a filesystem. "
		       "This is going to destroy it. "
		       "Are you sure you want to continue?")) 
			== PED_EXCEPTION_NO) {
			ped_device_end_external_access(c->disk->dev);
			return 0;
		}
	}
	/* If there is no already created BSD label created,
	   we warn the user about it */
	else if (!is_bsd_partition(c->disk->dev->path, 
		(*part)->geom.start * c->disk->dev->sector_size, 
		c->disk->dev->sector_size)) {
		if (ped_exception_throw(PED_EXCEPTION_WARNING,
					PED_EXCEPTION_YES_NO,
		     _("There is no BSD label on this partition. "
		       "Do you want to create one?")) 
			== PED_EXCEPTION_NO) {
			ped_device_end_external_access(c->disk->dev);
			return 0;
		}
	}
	get_partition_device(devname, SMALLBUF, *part, 0);
	Context * bsd_context = init_disk(0, devname, NULL, ped_disk_type_get ("bsd"));
	show_info(bsd_context);
	do_plist(bsd_context,NULL,0,0);

	/* We reset this back to its default state */
	uiquery.need_commit = 0;
	ped_device_end_external_access(c->disk->dev);
}
static int
do_resize (Context *c, PedPartition **part) {
	static MenuItem part_position[] = {
		{ 's', N_("Fixed start"), N_("Don't move the beginning of the partition") },
		/* FIXME: Wording. */
		{ 'b', N_("Begining"), N_("Place it as back as possible on partitions that support it") },
		{ 'e', N_("End"), N_("Place it as forward as possible on partitions that support it") },
		{ 'c', N_("Custom"), N_("Select custom start and end position of the partition") },
		{ ESC, N_("Cancel"), N_("Do not resize the partition") },
		{ 0, NULL, NULL }
	};
	static MenuItem resize_menu[] = {
		{ 'r', N_("Resize"), N_("Resize the filesystem") },
		{ 'c', N_("Change size"), N_("Change the size of the partition (if you know what you are doing)") },
		{ ESC, N_("Cancel"), N_("Do not resize the partition") },
		{ 0, NULL, NULL }
	};
	const char resize_keys[] = { 'r', 'c', ESC, '\0' };
	int key;
	PedConstraint *constraint = NULL;
	PedFileSystem *fs = NULL;
	PedSector first,last;
	PartPos pos;
	UIOpts opts = UI_WARN_COMMIT;
	if (!((*part)->type & PED_PARTITION_EXTENDED)) {
		if ((*part)->fs_type && (*part)->fs_type->ops->resize) {
			menu_title(_("What do you want to do?"));
			key = do_menu(resize_menu, 11, MENU_BUTTON | MENU_TITLE,resize_keys, NULL);
		}
		else {
			key = 0;
			getbool(_("Can't resize the partition. Alter the size? (For experts only)"),&key);
			key = (key ? 'c' : ESC);
		}
		if (key == ESC)
			return 0;
		if (key == 'r') {
			fs = ped_file_system_open(&(*part)->geom);
			if (!fs) {
				warning_waitkey(_("Could not open the filesystem for resizing"));
				return 0;
			}
			constraint = ped_file_system_get_resize_constraint(fs);
			if (!constraint || constraint->min_size == constraint->max_size) {
				warning_waitkey(_("We can't resize this filesystem type"));
				ped_file_system_close (fs);
				if (constraint) ped_constraint_destroy(constraint);
				return 0;
			}
			ped_file_system_close (fs);
		} else {
			opts |= UI_NO_FS_RESIZE;
		}
	}
	char buf[SMALLBUF];

	pos.start.sector = (*part)->geom.start;
	pos.end.sector = (*part)->geom.end;

	PedPartition *temp;
	/* We want to look for free space fo the same type */
	PedPartitionType desired = PED_PARTITION_FREESPACE |
	                           ((*part)->type & PED_PARTITION_LOGICAL);
	
	temp = part_list_prev(*part,PED_PARTITION_METADATA);

	if (temp && (temp->type == desired))
		first = temp->geom.start;
	else 
		first = (*part)->geom.start;

	temp = part_list_next(*part,PED_PARTITION_METADATA);
	
	if (temp && (temp->type == desired))
		last = temp->geom.end;
	else 
		last = (*part)->geom.end;

	if(!query_part_position(_("Where to place the resized partition"),
	                        part_position,&pos,first,last,c->dev,
	                        constraint,&opts)) {
		if (constraint) ped_constraint_destroy (constraint);
		return 0;
	}
	if (constraint) ped_constraint_destroy (constraint);
	

	if (opts & UI_CUSTOM_VALUES || pos.start.sector != (*part)->geom.start 
			|| pos.end.sector != (*part)->geom.end)
		if (!perform_resize(c->disk,*part,&pos,opts)) {
			warning_waitkey(N_("Couldn't resize partition."));
			/* Ooopsy */
			if (!(opts & UI_NO_FS_RESIZE) && uiquery.need_commit)
				_disk_reread(c, part);
			return 0;
		}
	return 1;
}

static int
do_move (Context *c, PedPartition **part) {
	/* Combine this with new partition? */
	static MenuItem part_position[] = {
		{ 's', N_("Begining"), N_("Move the partition to the begining of the free space") },
		{ 'e', N_("End"), N_("Move the partition to the end of the free space") },
		{ 'c', N_("Custom"), N_("Select custom start and end position of the partition") },
		{ ESC, N_("Cancel"), N_("Do not move the partition") },
		{ 0, NULL, NULL }
	};
	PedPartition *dst = NULL;
	PedSector start,end,length;
	PartPos pos;
	PedConstraint *constraint = NULL;
	PedFileSystem *fs = NULL;
	UIOpts opts = UI_WARN_COMMIT;
	fs = ped_file_system_open(&(*part)->geom);
	if (!fs) {
		warning_waitkey(N_("Can't open the filesystem"));
		return 0;
	}
	constraint = ped_file_system_get_copy_constraint(fs,c->dev);
	if (!constraint) {
		warning_waitkey(N_("We can't move this partition"));
		return 0;
	}
	ped_file_system_close (fs);
	menu_title(_("Select free space to move the partition to"));
	if (!do_plist(c, &dst, PED_PARTITION_FREESPACE, 0)) {
		ped_constraint_destroy (constraint);
		return 0;
	}
	if (!dst) {
		ped_constraint_destroy (constraint);
		return 0; 
	}
	start = dst->geom.start;
	end = dst->geom.end;
	length = dst->geom.length;
	
	if (length < constraint->min_size) {
		ped_constraint_destroy (constraint);
		warning_waitkey(N_("You can't move this partition here"));
		return 0;
	}
	length = MIN(length,(*part)->geom.length);
	length = MAX(length,constraint->min_size);
	end = start+length-1;
	pos.start.sector = start;
	pos.end.sector = end;
	if(!query_part_position(_("Where do you want to move the partition"),
	                        part_position,&pos,start,dst->geom.end,
	                        c->dev,constraint,&opts)) {
		ped_constraint_destroy (constraint);
		return 0;
	}
	ped_constraint_destroy (constraint);
	if (!perform_move(c->disk,*part,&pos,opts)) {
		warning_waitkey(N_("Partition move failed"));
		/* Oooops */
		if (uiquery.need_commit) _disk_reread(c, part);
		return 0;
	}
	return 1;
}

static int
do_mkfs (Context *c, PedPartition **part) {
	char buf[SMALLBUF];
	const PedFileSystemType *type = NULL;
	int go;
	if ((*part)->fs_type) {
		/* If we have an fs_type, we hope we can create it */
		if ((*part)->fs_type->ops->create != NULL) {
			go = 1;
			snprintf(buf,SMALLBUF,_("The partition has %s set as a filesystem. Use it?"),
			         (*part)->fs_type->name);
			getbool(buf,&go);
			if (go) type = (*part)->fs_type;
		}
		else {
			snprintf(buf,SMALLBUF, N_("Can't create filesystem %s, you'll have to choose another"), (*part)->fs_type->name);
			warning_waitkey(buf);
		}
	}
	if (!perform_mkfs (c->disk, *part, type, UI_WARN_COMMIT)) {
		warning_waitkey(N_("The filesystem was not created successfully"));
		return 0;
	}
	notice_waitkey(N_("The filesystem was created successfully."));
	return 1;
}

static int
do_check (Context *c, PedPartition **part) {
	if (!perform_check(c->disk,*part)) {
		warning_waitkey(N_("The consistency check failed."));
		return 0;
	}
	notice_waitkey(N_("The consistency of the partition seems to be OK."));
	return 1;
}

static int
do_rescue(Context *c, PedPartition **part) {
	static MenuItem rescue_menu[] = {
		{ 'g', N_("Continue"), N_("Proceed with the rescue") },
		{ 'c', N_("Custom"), N_("Select custom area to look for partitions (for experts only)") },
		{ ESC, N_("Cancel"), N_("Return to the main menu") },
		{ 0, NULL, NULL }
	};
	const char keys[] = { 'g', 'c', ESC, '\0' };
	int key;
	PedPartition *temp;


	menu_title(_("This will try to rescue broken or deleted partitions")); 
	key = do_menu(rescue_menu, 8, MENU_BUTTON | MENU_TITLE, keys, NULL);
	if (key == ESC) return 0;
#if 0
	/* Remember either the previous partition so we can select the free space */
	temp = disk_get_prev_nmd_partition(disk, *part);
	if (temp && temp->type & PED_PARTITION_FREESPACE)
		temp = disk_get_prev_nmd_partition(disk, temp); 
#endif
	if(!perform_rescue (c->disk, (*part)->geom.start, (*part)->geom.end, 
	           UI_WARN_COMMIT | (key == 'c' ? UI_CUSTOM_VALUES : 0))) {
		warning_waitkey(N_("There was an error during rescue"));
		return 0;
	}
#if 0
	do {
		temp = ped_disk_next_partition(disk,temp);
	} while (temp && temp->type & PED_PARTITION_METADATA); 
	*part = temp;
#endif
	*part = NULL;
	do {
		*part = ped_disk_next_partition(c->disk,*part);
	} while (*part && (*part)->type & PED_PARTITION_METADATA);
	notice_waitkey(N_("Finished looking for partitions"));
	return 1;
}

static int
do_copy(Context *c, PedPartition **part) {
	if (!perform_cp(c->disk,*part, UI_WARN_COMMIT)) {
		if (uiquery.need_commit) _disk_reread(c, part);
		warning_waitkey(N_("Partition not copied successfully"));
		return 0;
	}
	notice_waitkey(N_("Partition copied successfully"));
	return 1;
}

static int
do_delete(Context *c, PedPartition **part) {
	int go = 1;
	PedPartition *temp;
	/* Don't delete the first partition on a mac partition table */
	if (!strcmp(c->disk->type->name,"mac") && (*part)->num == 1) {
		warning_waitkey(N_("You should not delete this partition."));
		return 0;
	}
	/* Remember either the previous partition so we can select the free space */
	temp = disk_get_prev_nmd_partition(c->disk, *part);
	if (temp && temp->type & PED_PARTITION_FREESPACE)
		temp = disk_get_prev_nmd_partition(c->disk, temp); 

	getbool(_("Do you want to delete this partition?"),&go);
	if (!go) return 0;
	if (!perform_rm(c->disk,*part)) {
		warning_waitkey(N_("Can't delete partition"));
		return 0;
	}
	do {
		temp = ped_disk_next_partition(c->disk,temp);
	} while (temp && temp->type & PED_PARTITION_METADATA); 
	*part = temp;
	return 1;
}

static int
do_maximize(Context *c, PedPartition **part) {
	if (!perform_maximize(c->disk,*part)) {
		warning_waitkey(N_("Couldn't maximize this partition"));
		return 0;
	}
	return 1;
}

static int
do_minimize(Context *c, PedPartition **part) {
	PedPartition *temp;
	temp = disk_get_prev_nmd_partition(c->disk, *part);
	if (temp && temp->type & PED_PARTITION_FREESPACE)
		temp = disk_get_prev_nmd_partition(c->disk, temp); 

	if (!(*part)->type & PED_PARTITION_EXTENDED)
		return 0;
	if (!ped_disk_minimize_extended_partition(c->disk)) {
		warning_waitkey(N_("Couldn't minimize the extended partition"));
		return 0;
	}
	if (!ped_disk_extended_partition(c->disk)) {
		*part = temp;
		do {
			temp = ped_disk_next_partition(c->disk,temp);
		} while (temp && temp->type & PED_PARTITION_METADATA);
		if (temp) *part = temp;
	}
	return 1;
}

static int
do_commit (Context *c) {
	if (!perform_commit(c->disk,UI_WARN_COMMIT)) {
		warning_waitkey(N_("Commit failed."));
		return 0;
	} 
	notice_waitkey(N_("Partition table successfully written."));
	return 1;
}

static int
do_units () {
	static MenuItem units[] = {
		{ 's' , N_("Sectors"), N_("Show the sizes in sectors") },
		{ 'b' , N_("Bytes"), N_("Show the sizes in bytes") },
		{ 'k' , N_("Kilobytes"), N_("Use 1,000 bytes as unit size") },
		{ 'm' , N_("Megabytes"), N_("Use 1,000,000 bytes as unit size") },
		{ 'g' , N_("Gigabytes"), N_("Use 1,000,000,000 bytes as unit size") },
		{ 't' , N_("Terabytes"), N_("Use 1,000,000,000,000 bytes as unit size") },
		{ 'c', N_("Percents"), N_("Show the sizes in percents") },
		{ 'a', N_("Compact"), N_("Show the size in most appropriate units") },
		{ 'K' , N_("Kibibytes"), N_("Use 1,024 bytes as unit size") },
		{ 'M' , N_("Mebibytes"), N_("Use 1,048,576 bytes as unit size") },
		{ 'G' , N_("Gibibytes"), N_("Use 1,073,741,824 bytes as unit size") },
		{ 'T', N_("Tebibytes"), N_("Use 1,099,511,627,776 bytes as unit size") },
		{ 'y', N_("Cylinders"), N_("Show the sizes in cylinders") },
		{ 'C', N_("CHS"), N_("Show the sizes in CHS units") },
		{ 0, NULL, NULL }
	};
	int key;
	/* FIXME: If we add another way of chnaging the unit type, change this :) */
	static int units_menu = 3;
	//menu_title(_("Choose the display unit type")); /* Heh, no place for a title, heh */
	key = do_menu(units, 9, MENU_BUTTON, "sbkmgtcaKMGTyh", &units_menu);
	switch (key) {
		case 's': ped_unit_set_default(PED_UNIT_SECTOR); return 1;
		case 'b': ped_unit_set_default(PED_UNIT_BYTE); return 1;
		case 'k': ped_unit_set_default(PED_UNIT_KILOBYTE); return 1;
		case 'm': ped_unit_set_default(PED_UNIT_MEGABYTE); return 1;
		case 'g': ped_unit_set_default(PED_UNIT_GIGABYTE); return 1;
		case 't': ped_unit_set_default(PED_UNIT_TERABYTE); return 1;
		case 'c': ped_unit_set_default(PED_UNIT_PERCENT); return 1;
		case 'a': ped_unit_set_default(PED_UNIT_COMPACT); return 1;
		case 'K': ped_unit_set_default(PED_UNIT_KIBIBYTE); return 1;
		case 'M': ped_unit_set_default(PED_UNIT_MEBIBYTE); return 1;
		case 'G': ped_unit_set_default(PED_UNIT_GIBIBYTE); return 1;
		case 'T': ped_unit_set_default(PED_UNIT_TEBIBYTE); return 1;
		case 'y': ped_unit_set_default(PED_UNIT_CYLINDER); return 1;
		case 'C': ped_unit_set_default(PED_UNIT_CHS); return 1;
		
	}
}

/* I'm not quite sure we should let the user do this... */
/* Actually, I'm still not quite sure if it does what it says to do :) */
static int
do_type (Context *c, PedPartition **part) {
	static MenuItem type_menu[] = {
		{ 'a', N_("Auto"), N_("Try to automatically set the correct type") },
		{ 'c', N_("Custom"), N_("Select a custom filesystem type (for experts only)") },
		{ ESC, N_("Cancel"), N_("Do not change the filesystem type") },
		{ 0, NULL, NULL }
	};
	static char keys [] = { 'a', 'c', ESC, '\0' };
	const PedFileSystemType *type = NULL;
	int key;
	menu_title(_("Set the filesystem type of the partition")); 
	key = do_menu(type_menu, 8, MENU_BUTTON | MENU_TITLE, keys, 0);

	if (key == ESC)
		return 0;
	else if (key == 'a')
		type = ped_file_system_probe(&(*part)->geom);
	else if (!perform_set_system(c->disk,*part,type)) {
		warning_waitkey(N_("Couldn't change the filesystem type"));
		return 0;
	}
	return 1;
}

static int
do_name (Context *c, PedPartition **part) {
	if (!ped_disk_type_check_feature(c->disk->type,PED_DISK_TYPE_PARTITION_NAME)) {
		warning_waitkey(N_("The partition label doesn't support partition names"));
		return 0;
	}
	if (!perform_name(c->disk,*part, NULL)) {
		warning_waitkey(N_("Name wasn't changed successfully."));
		return 0;
	} 
	notice_waitkey(N_("Name was changed successfully."));
	return 1;
}

/* Here we begin with flag editing */
static void
printaflag (int *y, const PedPartition *part, PedPartitionFlag flag, PedPartitionFlag selected) {



	/* TODO: I decided we should not have the names here. */
#if 0
	const struct { const PedPartitionFlag flag; const char* text; } labels[] = {
		{ PED_PARTITION_BOOT, _("Bootable") },
		{ PED_PARTITION_ROOT, _("Root") },
		{ PED_PARTITION_SWAP, _("Swap") },
		{ PED_PARTITION_HIDDEN, _("Hidden") },
		{ PED_PARTITION_RAID, _("RAID") },
		{ PED_PARTITION_LVM, _("LVM") },
		{ PED_PARTITION_LBA, _("LBA") },
		{ PED_PARTITION_HPSERVICE, _("HP Service") },
		{ PED_PARTITION_PALO, _("Palo") }, /* What the hell is this? */
		{ PED_PARTITION_PREP, _("Prep") }, /* Hm, ok, I retract the last question */
		{ PED_PARTITION_MSFT_RESERVED, _("MSFT Reserved") }, /* Please, play with this on any partition you happen to find */
		{ 0, NULL }
	};
#endif
	int i;

	if (!ped_partition_is_flag_available(part,flag))
		return;
	move(*y,0); clrtoeol();
	if (selected == flag) {
		if (arrow_cursor)
			mvaddstr(*y,3,"-->");
		else
			attron(A_STANDOUT);
	}
	/* Find suitable text */
#if 0
	for (i = 0; labels[i].flag; i++) {
		if (flag == labels[i].flag) {
			text = labels[i].text;
			break;
		}
	}
	if (!text)
		text = ped_partition_flag_get_name(flag);
#endif
	
	mvaddch((*y),7,'[');
	mvaddch((*y),8,ped_partition_get_flag (part, flag) ? 'X' : ' ');
	mvaddstr((*y),9,"] ");
	mvaddstr((*y)++,11,P_(ped_partition_flag_get_name(flag)));
	if (selected == flag && !arrow_cursor)
		attroff(A_STANDOUT);
	

}
static void
flag_draw (const PedPartition *part, PedPartitionFlag selected) {

	PedPartitionFlag walk;
	int n,x,y = INFOSIZE;

	n = LINES-MENUSIZE-3;

	move(y++,0); clrtoeol();
	move(y,0); clrtoeol();
	mvaddstr(y++,5,_("Partition flags (press Esc two times to end)"));

	mvaddch(y,0,' ');
	for (x = 1; x < COLS-1; x++) {
		mvaddch(y,x,'-');
        }
	mvaddch(y++,x,' ');
	move(y++,0); clrtoeol();
	printaflag(&y,part,PED_PARTITION_BOOT,selected);
	printaflag(&y,part,PED_PARTITION_HIDDEN,selected);
	move(y++,0); clrtoeol();
	move(y,0); clrtoeol();
	mvaddstr(y++,5,_("Other partition flags (for experts only):"));
	for (walk = ped_partition_flag_next(0); walk && y < n; walk = ped_partition_flag_next(walk)) {
		if (walk != PED_PARTITION_BOOT && walk != PED_PARTITION_HIDDEN) {
			printaflag(&y,part,walk,selected);
		}
	}
	while (y < n) {
		move(y++,0); clrtoeol();
	}
	refresh();
}

static int
do_flag (Context *c, PedPartition **part) {
	PedPartitionFlag walk;
	/* We will put the flags in this array */
	PedPartitionFlag flags[MAXFLAGS+1];
	int key, done = 0, redraw = 1, i = 0;

	/* Put the boot and hidden flags at the beginning */
	if (ped_partition_is_flag_available(*part,PED_PARTITION_BOOT))
		flags[i++] = PED_PARTITION_BOOT; 
	if (ped_partition_is_flag_available(*part,PED_PARTITION_HIDDEN))
		flags[i++] = PED_PARTITION_HIDDEN;

	/* Put the other flags */
	for (walk = ped_partition_flag_next(0); walk && i < MAXFLAGS;
	     walk = ped_partition_flag_next(walk))
	{
		if (walk == PED_PARTITION_BOOT || walk == PED_PARTITION_HIDDEN)
			continue;
		if (ped_partition_is_flag_available(*part,walk))
			flags[i++] = walk;
	}
	flags[i] = 0;
	
	/* If there are no flags available, sorry */
	if (i == 0) {
		warning_waitkey(_("No flags can be changed for this partition"));
		return 0;
	}

	/* We select the first one */
	i = 0;

	while (!done) {
		if (redraw) {
			flag_draw(*part,flags[i]);
			clear_menu();
			redraw = 0;
		}
		refresh();
		key = getch();
#if USE_KEYPAD == 0
		if (key == ESC) {
			key = getch();
			if (key == '[' || key == 'O') {
				key = getch();
				if (key == 'B') { /* That's down arrow */
					key = KEY_DOWN;
				}
				else if (key == 'A') { /* That's up arrow */
					key = KEY_UP;
				}
				else
					print_warning(_("Invalid key"),0);
			}
			else if (key != ESC)
				print_warning(_("Invalid key"),0);
		}
#endif
		if (key == ESC)
			return 1;
		else if (key == KEY_DOWN) {
			if (!flags[i+1])
				print_warning(_("No more flags"),0);
			else {
				i++;
				redraw = 1;
			}
		}
		else if (key == KEY_UP) {
			if (i == 0)
				print_warning(_("No more flags"),0);
			else {
				i--;
				redraw = 1;
			}
		}
		else if (key == CR || key == ' ') {
			perform_set(c->disk,*part,flags[i],UI_FLAG_TOGGLE);
			redraw = 1;
		}
		else 
			print_warning(_("Invalid key"),0);
	}
	return 1;
}

static int
do_help () {
	StrList *message = _message_display_strlist(help,NULL,TAB_TO_INDENT);
	do_strlist(_("cfdisk help"),message,1);
	str_list_destroy(message);
	return 1;
}

static int
do_pinfo (Context *c, PedPartition **part) {
	StrList *info = NULL;
	char buf[SMALLBUF], buf2[SMALLBUF];
	const char *temp;
	int n;

	unsigned int system_type;
	int system_type_size;


	/* TODO: I did like it is done in Linux cfdisk. 
	         This won't be always correct */
	n = strlen(c->dev->path);
	if (!(0 && c->is_devicefile) && !((*part)->type & PED_PARTITION_FREESPACE)) {
		
		get_partition_device(buf2, SMALLBUF, *part, 0);
		snprintf(buf,SMALLBUF, "%30s: %s",
			_("Possible partition device"), buf2);
		info = str_list_append(info,buf);
	}
	if ((*part)->type & PED_PARTITION_FREESPACE) {
		if ((*part)->type & PED_PARTITION_LOGICAL)
			temp = _("Free space inside an extended partition");
		else
			temp = _("Free space");
	}
	else if ((*part)->type & PED_PARTITION_LOGICAL)
		temp = _("Logical");
	else if ((*part)->type & PED_PARTITION_EXTENDED)
		temp = _("Extended");
	else
		temp = _("Primary");
	snprintf(buf,SMALLBUF,"%30s: %s", _("Partition type"), temp);
	info = str_list_append(info,buf);
	if (ped_disk_type_check_feature(c->disk->type,PED_DISK_TYPE_PARTITION_NAME)) {
		snprintf(buf,SMALLBUF,"%30s: %s", _("Partition name"), ped_partition_get_name(*part));
		info = str_list_append(info,buf);
	}
	info = str_list_append(info,"");

	/* Partition size */
	snprintf(buf,SMALLBUF,"%30s: %s", _("Partition size in bytes"),
	         ped_unit_format_custom ((*part)->disk->dev, (*part)->geom.length, PED_UNIT_BYTE));
	info = str_list_append(info,buf);
	snprintf(buf,SMALLBUF,"%30s: %s", _("Partition size in sectors"),
	         ped_unit_format_custom ((*part)->disk->dev, (*part)->geom.length, PED_UNIT_SECTOR));
	info = str_list_append(info,buf);
	snprintf(buf,SMALLBUF,"%30s: %s", _("Portion of the hard disk"),
	         ped_unit_format_custom ((*part)->disk->dev, (*part)->geom.length, PED_UNIT_PERCENT));
	info = str_list_append(info,buf);
	info = str_list_append(info,"");

	/* Filesystem info */
	if (!((*part)->type & PED_PARTITION_FREESPACE)) {
		if ((*part)->fs_type) {
			snprintf(buf,SMALLBUF,"%30s: %s", _("Filesystem type"), (*part)->fs_type->name);
			info = str_list_append(info,buf);
		}
	
		system_type = get_disk_specific_system_type(*part,&system_type_size);
		if (system_type_size) {
			system_type_size *= 2;
			snprintf(buf,SMALLBUF,"%30s: 0x%0*x", _("System type"), system_type_size, system_type);
			info = str_list_append(info,buf);
		}
		temp = get_disk_specific_system_name(*part,0);
		if (temp) {
			snprintf(buf,SMALLBUF,"%30s: %s", _("System type name"), temp);
			info = str_list_append(info,buf);
		}
	}
	info = str_list_append(info,"");
	/* Position in sectors */
	snprintf(buf,SMALLBUF,"%30s: %llds-%llds", _("Position"), 
	         (*part)->geom.start, (*part)->geom.end);
	info = str_list_append(info,buf);
	/* Start and end */
	snprintf(buf,SMALLBUF,"%30s: %s", _("Start (cyl,heads,sector)"),
	         ped_unit_format_custom ((*part)->disk->dev, (*part)->geom.start, PED_UNIT_CHS));
	info = str_list_append(info,buf);
	snprintf(buf,SMALLBUF,"%30s: %s", _("End (cyl,heads,sector)"),
	         ped_unit_format_custom ((*part)->disk->dev, (*part)->geom.end, PED_UNIT_CHS));
	info = str_list_append(info,buf);
	info = str_list_append(info,"");
	
	/* Flags */
	if (!((*part)->type & PED_PARTITION_FREESPACE)) {
		snprintf(buf,SMALLBUF,"%30s: %s", _("Flags"), partition_print_flags(*part));
		info = str_list_append(info,buf);
		info = str_list_append(info,"");
	}

	do_strlist(_("Partition info"),info,3);
	str_list_destroy(info);
	
	return 1;
}

/* We remember last disk listed here */
static struct {
	Context *c;
	PedPartition *selected;
	int selnum;
	int start;
} last_plist = { NULL, NULL, 0, 0 };

/* Define column positions. The first three will be fixed. */
#define col_name 3
#define col_flags 12
#define col_type 23
#define col_fs 35

/* TODO: Use printw, instead of buffer where possible ? */

/* Partition list drawing function */
static void
plist_draw (Context *c, PedPartition *selected, int selnum, int *start) {
	if (!c) {
		c = last_plist.c;
		selected = last_plist.selected;
		selnum = last_plist.selnum;
		start = &last_plist.start;
	}
	else {
		last_plist.c = c;
		last_plist.selected = selected;
		last_plist.selnum = selnum;
		last_plist.start = *start;
	}
	if (!c || !c->disk) {
		return;
	}
	PedPartition *part;
	const char* temp;
	char buf[SMALLBUF],fsbuf[FSBUF];

	int col_label = ((double)(53-col_fs)/(80-col_fs))*(COLS-col_fs)+col_fs;
	int col_size = ((double)(66-col_fs)/(80-col_fs))*(COLS-col_fs)+col_fs;
	int n,i,x,y = INFOSIZE;

	int can_primary = can_create_primary(c->disk);
	int can_extended = can_create_extended(c->disk);
	/* TODO: Should we make can_name global? */
	int can_name = ped_disk_type_check_feature(c->disk->type,PED_DISK_TYPE_PARTITION_NAME);

	n = LINES-3-MENUSIZE;

	if (selnum - *start < 0)
		*start = selnum;
	else if (selnum - *start >= n - INFOSIZE - 4)
		*start = selnum - n + INFOSIZE + 5;
	move(y++,0); clrtoeol();

	/* Display header */
	move(y,0); clrtoeol();
	mvaddstr(y,col_name,_("Number"));
	mvaddstr(y,col_flags,_("Flags"));
	mvaddstr(y,col_type,_("Part Type"));
	mvaddstr(y,col_fs,_("Filesystem"));
	mvaddstr(y,col_label,_("Label"));

	/* We want size to be right aligned */
	temp = _("Size");
	x = COLS-strlen(temp)-2;
	x = MAX(x,col_size);
	mvaddstr(y++,x,temp);

	buf[0] = ' ';
	for (i = 1; i < COLS-1 && i < SMALLBUF; i++) {
		buf[i] = '-';
	}
	if (i < SMALLBUF) buf[i++] = ' ';
	if (i < SMALLBUF) buf[i] = '\0';
	mvaddstr(y++,0,buf);

	n = LINES-3-MENUSIZE;
	for (part = ped_disk_next_partition (c->disk, NULL), i=0; part && y < n;
	     part = ped_disk_next_partition (c->disk, part)) {
		if (part->type & PED_PARTITION_METADATA) continue; /* We skip METADATA partitions */

		i++;
		if (i < *start) continue;	/* If the user has scrolled down, skip hidden */

		int free = part->type & PED_PARTITION_FREESPACE;
		if (part == selected) {
			if (arrow_cursor)
				mvaddstr(y,0,"-->");
			else {
				attron(A_STANDOUT);
				mvaddstr(y,0,"   ");
			}
		} else
			mvaddstr(y,0,"   ");
		if (!free)
			snprintf(buf, SMALLBUF, "%*d%*s", 4,part->num, col_flags-col_name-4, "");
		else
			snprintf(buf, SMALLBUF, "%*s", col_flags, "");
		mvaddstr(y,col_name,buf);
		/* ped_partition_get_flag(part, PED_PARTITION_BOOT) throws an exception when ran on a
		   free space. */
		snprintf(buf, SMALLBUF, "%-*s", col_type-col_flags,
		         (!free && ped_partition_is_flag_available(part, PED_PARTITION_BOOT) && 
		         ped_partition_get_flag(part, PED_PARTITION_BOOT) ? N_("Bootable") : ""));
		mvaddstr(y,col_flags,buf);

		if (part->type & PED_PARTITION_EXTENDED)
			temp = _("Extended");
		else if (part->type & PED_PARTITION_LOGICAL)
			temp = _("Logical");
		else if (free) {
			if (can_primary)
				temp = (can_extended ? _("Pri/Ext") : _("Primary"));
			else
				temp = _("None");
		}
		else temp = _("Primary");

		snprintf(buf, SMALLBUF, "%-*s", col_fs-col_type, temp);
		mvaddstr(y,col_type,buf);
		temp = NULL;
		if (free)
			temp = _("Free space");
		else if (part->type & PED_PARTITION_EXTENDED)
			temp = "";
		else if(part->fs_type)
			temp = part->fs_type->name;
		if (temp) {
			snprintf(buf, SMALLBUF, "%-*s", col_label-col_fs, temp);
		} else {
			temp = get_disk_specific_system_name(part,1);
			if (temp) {
				snprintf(fsbuf, FSBUF, "[%s]", temp);
				temp = fsbuf;
			} else {
				temp = "";
			}
		}
		snprintf(buf, SMALLBUF, "%-*s", col_label-col_fs, temp);
			

		
		mvaddstr(y,col_fs,buf);
		temp = NULL;
		if (can_name && !free && !(part->type & PED_PARTITION_EXTENDED))
			temp = ped_partition_get_name(part);
		if (!temp) temp = "";
		snprintf(buf, SMALLBUF, "%-*s", col_size-col_label, temp);
		mvaddstr(y,col_label,buf);

		/* Display the size of the disk right-aligned. */
		snprintf(buf, SMALLBUF, "%*s ", COLS-col_size-1,
		         ped_unit_format (part->disk->dev, part->geom.length));
		mvaddstr(y++,col_size,buf);

		if (part == selected && !arrow_cursor) attroff(A_STANDOUT);
	}
	/* Please note that we clean one more line */
	while (y < n+1) {
		move(y++,0); clrtoeol();
	}
	refresh();
}




static MenuItem main_menu[] = {
	{ 'b', N_("Flags"), N_("Change the flags of the current partition") },
	{ 'n', N_("New"), N_("Create new partition from free space") },
	{ 'e', N_("Edit"), N_("Edit this BSD disklabel") },
	{ 's', N_("Rescue"), N_("Look for deleted and corrupted partitions in the free space") },
	{ 'f', N_("Make FS"), N_("Creates a filesystem on the partition") },
	{ 'c', N_("Check"), N_("Check partition for consistency") },
	{ 'm', N_("Rename"), N_("Change partition name") },
	{ 'y', N_("Copy"), N_("Write another partition over this one (requires commit)") },
	{ 'r', N_("Resize"), N_("Resizes the current partition (requires commit)") },
	{ 'x', N_("Maximize"), N_("Enlarges the partition to the maximum possible size") },
	{ 'z', N_("Minimize"), N_("Shrinks the partition to the minimum possible size") },
	{ 'o', N_("Move"), N_("Moves the current partition (requires commit)") },
	{ 'd', N_("Delete"), N_("Delete the current partition") },
	{ 't', N_("Type"), N_("Set the filesystem type (doesn't convert the filesystem)") },
	{ 'u', N_("Units"), N_("Change units of the partition size display") },
	{ 'w', N_("Commit"), N_("Write the changes to the disk") },
	{ 'q', N_("Quit"), N_("End editing this partition table") },
	{ 'i', N_("Info"), N_("Display additional partition information") },
	{ 'h', N_("Help"), N_("Display help") },
        { 0, NULL, NULL }
};

const char keys_ext[] = { 'm','d','u','w','q','i','h','r','x','z', ESC, '\0' };
const char keys_free[] = { 'n','s','u','w','q','h','i', ESC, '\0' };
const char keys_part[] = { 'b','f','c','m','y','r','o','d','t','u','w','q','i','h', ESC, '\0' };
/*bfcayrodtuwqph*/
const char keys_bsd[] = { 'b', 'e', 'd', 't', 'u', 'w', 'q', 'i', 'h', ESC, '\0' };

static MenuItem partselect_menu[] = {
	{ 's', N_("Select"), N_("Select this as the source partition") },
	{ ESC, N_("Cancel"), N_("Abort partition copy") },
	{ 0, NULL, NULL }
};

const char keys_partselect[] = { 's', ESC, '\0' };
const char keys_cantselect[] = { ESC, '\0' };

/* We need to separate this to use the partition list for other purposes like partition copy */
/* FIXME Make use of redraw or get rid of it completely */
static int
main_plist(Context *c, PedPartition **part, int key) {
	switch (key) {
		case ESC:
		case 'q':
			if (uiquery.need_commit) {
				key = 0;
				getbool(N_("Partition table has changed, are you sure you want to quit?"),
					&key);
			}
			else if (key == ESC) {
				key = 0;
				getbool(N_("Are you sure you want to quit?"), &key);
			}
			break;
		case 'n':
			do_new(c, part);
			//redraw = 1;
			key = 0;
			break;
		case 'e':
			do_edit(c, part);
			//redraw = 1;
			key = 0;
			break;
		case 'w':
			do_commit(c);
			//redraw = 1;
			key = 0;
			break;
		case 'f':
			do_mkfs(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'c':
			do_check(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'u':
			do_units();
			//redraw = 1;
			key = 0;
			break;
		case 's':
			do_rescue(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'y':
			do_copy(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'r':
			do_resize(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'd':
			do_delete(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'o':
			do_move(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'x':
			do_maximize(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'z':
			do_minimize(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 'b':
			do_flag(c,part);
			//redraw = 1;
			key = 0;
			break;
		case 't':
			do_type(c,part);
			//redraw = 0; //??
			key = 0;
			break;
		case 'h':
			do_help();
			//redraw = 0;
			key = 0;
			break;
		case 'm':
			do_name(c,part);
			//redraw = 0;
			key = 0;
			break;
		case 'i':
			do_pinfo(c,part);
			//redraw = 0;
			key = 0;
			break;
		default:
			/* NOTE: I think this can't be reached */
			warning_waitkey("Unimplemented");
			key = 0;
	}
	return key;
}

/* TODO: This doesn't need a page up/down, but leave it as a TODO */
/* Okay, this is our partition list. If there is no pointer to partition given, ASSume main menu
   have and havent aren't perfect, but work for our purposes */
static int
do_plist (Context *c, PedPartition **part, PedPartitionType have, PedPartitionType havent) {
	int key = 0, i, redraw = 1, selnum = 0, start = 0;
	const char* keys;
	PedPartition *selected = NULL;
	PedPartition *temp,*memp;
	MenuOptions menu_opts = MENU_BUTTON | MENU_ARROWS;
	MenuItem *part_menu;
	int menupos = 0;
	if (!part) 
		part_menu = main_menu;
	else {
		part_menu = partselect_menu;
		menu_opts |= MENU_TITLE;
	}
	do {
		selected = ped_disk_next_partition(c->disk,selected);
	} while (selected && selected->type & PED_PARTITION_METADATA);
	while (!key) {
		if(redraw) {
			show_info(c);
			plist_draw(c,selected, selnum, &start);
			redraw = 0;
		}
		if (part) {
			if ((!have || selected->type & have) && !(havent && selected->type & havent))
				keys = keys_partselect;
			else
				keys = keys_cantselect;
		}
		else if (selected->type & PED_PARTITION_EXTENDED)
			keys = keys_ext;
		else if (selected->type & PED_PARTITION_FREESPACE)
			keys = keys_free;
		else if (is_part_type_bsd(selected))
			keys = keys_bsd;
		else keys = keys_part;
		
		key = do_menu(part_menu, 8, menu_opts, keys, &menupos);
		menu_opts &= ~MENU_LEAVE_WARNING;
		if (key == KEY_UP) {

			temp = disk_get_prev_nmd_partition(c->disk,selected);
			if (temp) {
				selected = temp;
				selnum--;
				redraw = 1;
			}
			
			else {
				menu_opts |= MENU_LEAVE_WARNING;
				print_warning(N_("No more partitions"),0);
			}
			key = 0;
		}
		else if (key == KEY_DOWN) {
			temp = ped_disk_next_partition(c->disk,selected);
			while (temp && temp->type & PED_PARTITION_METADATA)
				temp = ped_disk_next_partition(c->disk,temp);
			if (temp) { 
				selnum++;
				selected = temp;
				redraw = 1;
			}
			else {
				menu_opts |= MENU_LEAVE_WARNING;
				print_warning(N_("No more partitions"),0);
			}
			key = 0;
		}
		else {
			if (part) {
				if (key == ESC) {
					return 0;
				}
				else if (key == 's') {
					*part = selected;
					return 1;
				}
				key = 0;
			} else {
				key = main_plist(c, &selected, key);
				/* Count which partition is now selected */
				PedPartition *temp;
				selnum = 0;
				for(temp = ped_disk_next_partition(c->disk,NULL);
				    temp && temp != selected;
				    temp = ped_disk_next_partition(c->disk,temp))
					selnum++;
				redraw = 1;
			}
		}
		refresh();
	}
	return 1;
	
}




/* This prints the information at the top */
static void
show_info(Context *c) {
	if (!c) {
		c = last_plist.c;
		if (!c) {
			return;
		}
	}
	int y;
	char buf[SMALLBUF];
	for (y = 0; y < INFOSIZE+1; y++) {
		move(y,0);
		clrtoeol();
	}
	y = 0;
	snprintf(buf,SMALLBUF,"GNU cfdisk %s",VERSION);
	mvaddstr(y++,get_center(buf),buf);
	y++;
	
	if (c->info_model) 
		mvaddstr(y++,get_center(c->info_model),c->info_model);
	
	if (c->info_size) {
		snprintf(buf,SMALLBUF,_("Disk: %s   Disk type: %s    Size: %s"), 
		         c->devname, c->disk->type->name, c->info_size);
	}
	else {
		snprintf(buf,SMALLBUF,_("Disk: %s   Disk type: %s"), c->devname, c->disk->type->name);
	}
	mvaddstr(y++,get_center(buf),buf);
	
	if (c->info_heads && c->info_sectors && c->info_cylinders) {
		snprintf(buf,SMALLBUF, _("Heads: %s   Sectors per track: %s   Cylinders: %s"),
				c->info_heads, c->info_sectors, c->info_cylinders);
		mvaddstr(y++,get_center(buf),buf);
	}
	
	
}




Context *
init_disk(int new_table, char *devname, PedDevice *dev, PedDiskType *type) {
	char buf[SMALLBUF];
	int n;
	PedDisk *disk = NULL;
	int is_devicefile = 0;
	/*
	char *info_size;
	char *info_sectorsize;
	char *info_model;
	char *info_heads;
	char *info_cylinders;
	char *info_sectors;
	char *info_cylsize;
	*/
	static MenuItem custom_part[] = {
		{ 'c', N_("Choose"), N_("Prompt for device address") },
		{ 'q', N_("Quit"), N_("Quit program") },
		{ 0, NULL, NULL }
	};
	static MenuItem custom_label[] = {
		{ 'c', N_("Create"), N_("Create new partition table") },
		{ 'q', N_("Quit"), N_("Quit program") },
		{ 0, NULL, NULL }
	};
	/* If we can't find the device automatically. FIXME: This is not tested. */
	if (!dev) {
		menu_title(_("Cannot find a device automatically"));
		if (devname) {
			dev = ped_device_get(devname);
			if (!dev || !ped_device_open(dev)) {
				do_quit(1,_("Invalid device"));
			}
		}
		else if ('c' == do_menu(custom_part, 8, MENU_BUTTON | MENU_TITLE, "cq", 0)) {
			read_char(_("Enter path to the device"),&devname);
			if (!devname) {
				do_quit(0,NULL);
			}
			dev = ped_device_get(devname);
			if (!dev || !ped_device_open(dev)) {
				do_quit(1,_("Invalid device"));
			}
		}
		else do_quit(0,NULL);
	}
	is_devicefile = !is_blockdev(dev->path);
	if (!new_table) {
		ped_exception_fetch_all ();
		disk = ped_disk_new(dev);
		/* If there is no disk label on the disk, ask the user to create one */
		if (!disk) {
			ped_exception_catch();
			ped_exception_leave_all();
			menu_title(_("Cannot find a partition table on the disk"));
			if (type || 'c' == do_menu(custom_label, 8, MENU_BUTTON | MENU_TITLE, "cq", 0)) {
				if (!perform_mklabel (dev, &disk, type))
					do_quit(1,_("Creation of partition table failed"));
			}
			else do_quit (0,NULL);
		} else {
			ped_exception_leave_all();
		}
	} else {
		if (!perform_mklabel (dev, &disk, NULL))
			do_quit(1,_("Creation of partition table failed"));
	}

	//ped_unit_set_default(PED_UNIT_MEGABYTE);

	Context *context = malloc(sizeof(Context));

	context->dev = dev;
	context->disk = disk;
	context->is_devicefile = is_devicefile;
	context->devname = devname;

	
	/* Initialize the info variables */
	snprintf(buf,SMALLBUF,_("%s, %s"),
		ped_unit_format_custom_byte (dev, dev->length * dev->sector_size-1, PED_UNIT_BYTE),
		ped_unit_format_custom_byte (dev, dev->length * dev->sector_size-1, PED_UNIT_MEGABYTE));
	n = strlen(buf);
	context->info_size = calloc(n,sizeof(char));
	strncpy(context->info_size,buf,n);

	if (is_devicefile) {
		char * model_file = _("Device file");
		context->info_model = calloc(n,sizeof(char));
		strncpy(context->info_model,model_file,n);
	}
	else {
		n = strlen(dev->model);
		context->info_model = calloc(n,sizeof(char));
		strncpy(context->info_model,dev->model,n);
	}
	
	snprintf(buf,SMALLBUF,"%lldB/%lldB\n",dev->sector_size, dev->phys_sector_size);
	n = strlen(buf);
	context->info_sectorsize = calloc(n,sizeof(char));
	strncpy(context->info_sectorsize,buf,n);

	PedCHSGeometry* chs = &(dev->bios_geom);
        context->info_cylsize = ped_unit_format_custom (dev, 
			chs->heads * chs->sectors, PED_UNIT_KILOBYTE);

	snprintf(buf, SMALLBUF, "%d", chs->cylinders);
	n = strlen(buf);
	context->info_cylinders = calloc(n,sizeof(char));
	strncpy(context->info_cylinders,buf,n);

	snprintf(buf, SMALLBUF, "%d", chs->heads);
	n = strlen(buf);
	context->info_heads = calloc(n,sizeof(char));
	strncpy(context->info_heads,buf,n);
	
	snprintf(buf, SMALLBUF, "%d",  chs->sectors);
	n = strlen(buf);
	context->info_sectors = calloc(n,sizeof(char));
	strncpy(context->info_sectors,buf,n);
	
	return context;

}



static void
do_ui (char *devname, PedDevice *dev) {
	initscr();
	/* TODO: Maybe we should enable colouring? */
	//start_color();
#if USE_KEYPAD
	keypad(stdscr,TRUE);
#endif
	cbreak();
	noecho();
	nonl();

	init_calls();

	Context * global_context = init_disk(new_table, devname, dev, NULL);
	show_info(global_context);
	do_plist(global_context,NULL,0,0);
	endwin();
}


static void
print_usage() {
	int i;
	char buf[SMALLBUF];
	fputs(_(usage_msg), stdout);
	printf("\n%s\n",_("OPTIONs:"));
	for (i = 0; options_help[i].opt; i++) {
		if (options_help[i].arg)
			snprintf(buf,SMALLBUF,"%s=%s",options_help[i].lopt,_(options_help[i].arg));
		else
			strncpy(buf,options_help[i].lopt,SMALLBUF);
		printf ("  -%c, --%-23.23s %s\n", options_help[i].opt, buf, _(options_help[i].help));
	}
	exit(0);
}


static void
print_version() {
	fputs(prog_name, stdout);
	fputs(_(license_msg), stdout);
	exit(0);
}

int
main (int argc, char **argv) {
	PedUnit unit = -1;
	char *devname;
	PedDevice *dev;
#ifdef HAVE_GETOPT_H
	static struct option long_options[] = {
		{ "new-table", no_argument, NULL, 'z' },
		{ "version", no_argument, NULL, 'v' },
		{ "arrow-cursor", no_argument, NULL, 'a' },
		{ "units", required_argument, NULL, 'u' },
		{ "help", no_argument, NULL, 'h' },
		{ "list-partition-types", no_argument, NULL, 't' },
		{ NULL, 0, NULL, 0 }
	};

	int opt,option_index;
	while ((opt = getopt_long(argc,argv, "hazvtu:", long_options, &option_index)) != -1) {
#else
	int opt;
	while ((opt = getopt(argc,argv,"hazvtu:")) != -1) {
#endif		
		switch (opt) {
			case 'v':
				print_version();
			case 'h':
				print_usage();
			case 't':
				print_partition_types();
			case 'a':
				arrow_cursor = 1;
				break;
			case 'z': 
				new_table = 1;
				break;
			case 'u':
				unit = ped_unit_get_by_name(optarg);
				break;


		}
	}
	if (unit == -1) {
		ped_unit_set_default(PED_UNIT_MEGABYTE);
	}
	else {
		ped_unit_set_default(unit);
	}
	if (argc-optind == 1) {
		devname = argv[optind];
		dev = ped_device_get(devname);
		if (!dev) {
			printf(_("Invalid device\n"));
			exit(1);
			
		}
	}
	else {
		ped_device_probe_all();
		dev = ped_device_get_next(NULL);
		if (dev) devname= dev->path;	
	}
	if (dev) if (!ped_device_open(dev)) {
		printf(_("Invalid device\n"));
		exit(1);
	}
	do_ui(devname, dev);
}
