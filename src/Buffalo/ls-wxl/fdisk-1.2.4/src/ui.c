/*
    fdisk - Linux fdisk clone
    This file originally from GNU Parted.
    Copyright (C) 1999, 2000, 2001, 2002, 2006 Free Software Foundation, Inc.

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

#include <parted/parted.h>
#include <parted/debug.h>

#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "command.h"
#include "strlist.h"
#include "ui.h"

/* compatibility mode flag */
extern int compat_mode;
/* Non NULL when -s option specifyed on command line */
extern const char *fdisk_partsize_device;
extern int fdisk_partsize_part; /* ?? */

/* not 0 when fdisk should print the partition table? */
extern int fdisk_list_table;

#if HAVE_LOCALE_H
# include <locale.h>
#endif /* HAVE_LOCALE_H */

#include "gettext.h"
#define N_(String) String
#define _(String) dgettext (PACKAGE, String)

/* BEGIN readline stuff */
#if HAVE_LIBREADLINE

/* If the macro "HAVE_LIBREADLINE corresponds to a digit other than zero
 * the readline is present along with the respective `readline/readline.h'
 * and `readline/history.h' header file's */


/* termcap library */
#	if HAVE_LIBTERMCAP
	/* Termcap need this buffer to initialize terminal database, I think
	 * the db is stored into this buffer but I'm not sure. */
		enum { TERMCAP_BUFFER_SIZE = 2048 };
		static char termcap_buffer[TERMCAP_BUFFER_SIZE] = {0};

		/* Use this flag to know if termcap is propely initialized
			 and ready or should not be used */
		static enum { TERMCAP_OK, TERMCAP_ERROR } termcap_status;
#		if HAVE_TERMCAP_H
#			include <termcap.h>
# 	else
    	extern int tgetnum (char* key);
#  	endif /* HAVE_TERMCAP_H */
# endif /* HAVE_LIBTERMCAP */

# include <readline/readline.h>
# include <readline/history.h>
  
/* Both `HAVE_RL_COMPLETION_MATCHES' and `completion_matches' are unused */
#if 0
# ifndef HAVE_RL_COMPLETION_MATCHES
#  define rl_completion_matches completion_matches
# endif
#endif /* 0 */

# if !HAVE_RL_COMPENTRY_FUNC_T
	/* cast to (void*) */
#  define rl_compentry_func_t void
# endif

#endif /* HAVE_LIBREADLINE */

/* END readline stuff */

/* global and used in fdisk.c */
char* interface_name = "GNU Fdisk " VERSION;

#define MAX_WORDS	1024

static StrList*		command_line;
static FdiskCommand**	commands;
static StrList*		ex_opt_str [64];
static StrList*		on_list;
static StrList*		off_list;
static StrList*		on_off_list;
static StrList*		fs_type_list;
static StrList*		disk_type_list;

/* Declare readline_state */
static struct {
  const StrList* possibilities;
  const StrList* cur_pos;
  int		 in_readline;
  sigjmp_buf	 jmp_state;
} readline_state;

static PedExceptionOption current_exception_opt = 0;

/* If we don't want the possibilities to appear on the prompt, this
   should be set to 0. TODO: Making this without a global variable
   might be a good idea */
static prompt_possibilities = 1;


#if HAVE_LIBREADLINE
#	if HAVE_LIBTERMCAP
/* This function initialize the termcap library */
void init_termcap_library(void)
{
  char *terminal;
  int status;

  terminal = getenv("TERM");
  if(terminal == NULL)
    {
      fputs(_("can't get `TERM' environment variable.\n"
	      "Please specify a terminal type int the `TERM' environment\n"
	      "variable.\n"), stderr);
      goto _Error;
    }
  status = tgetent(termcap_buffer, terminal);
  if(status <= 0)
    {
      if(status == 0)
	fprintf(stderr, 
		_("%s termcap error: terminal `%s' is not defined in your\n"
		  "terminal database, please update it.\n"),
		PACKAGE, terminal);
      else
	fprintf(stderr, 
		_("%s termcap error: could not access terminal database\n"
		  "please check yor termcap library.\n"),
		PACKAGE);
      goto _Error;
    }
  else
    termcap_status = TERMCAP_OK;
  return;
 
 _Error:
  termcap_status = TERMCAP_ERROR;
}
#endif /* HAVE_LIBTERMCAP */

/* returns matching commands for text */
static char*
command_generator (char* text, int state)
{
  if (!state)
    readline_state.cur_pos = readline_state.possibilities;

  while (readline_state.cur_pos) {
    const StrList* cur = readline_state.cur_pos;
    readline_state.cur_pos = cur->next;
    if (str_list_match_node (cur, text))
      return str_list_convert_node (cur);
  }

  return NULL;
}

/* completion function for readline() */
char**
fdisk_complete_function (char* text, int start, int end)
{
  return rl_completion_matches (text, (rl_compentry_func_t*) command_generator);
}

static void
_add_history_unique (const char* line)
{
  HIST_ENTRY** p;

  /* Check if line is empty */
  if (!line || !line[0])
    return;

  /* Get all entryes of history and find if current line 
   * is already in one history entry. */
  p = history_list();
  
  if(p != NULL)
    /* if the history is not empty we search for line in all entryes */
    {
      unsigned int i;
      for(i = 0; p[i] != NULL; i++)
	if(strcmp(p[i]->line, line) == 0)
	  /* match */
	  return;
    }
  /* no match */
  add_history(line);
}

#endif /* HAVE_LIBREADLINE && HAVE_LIBTERMCAP_H */

static void
interrupt_handler (int signum)
{
  int in_readline = readline_state.in_readline;

  readline_state.in_readline = 0;

  signal (signum, &interrupt_handler);

  if (in_readline) {
    putchar(0x0A);
    siglongjmp (readline_state.jmp_state, 1);
  }
}

/* check if line is only spaces */
static int
is_empty_line(const char *s)
{
  while(*s)
    if(!isspace(*s++))
      return 0;
  return 1;
}

static char*
_readline (const char* prompt, const StrList* possibilities)
{
  char* line = NULL;

  readline_state.possibilities = possibilities;
  readline_state.cur_pos = NULL;
  readline_state.in_readline = 1;

  if (sigsetjmp (readline_state.jmp_state, 1))
    return NULL;

  fdisk_wipe_line ();

  if (!fdisk_opt_script_mode)
#if HAVE_LIBREADLINE
    { /* !fdisk_opt_script_mode && HAVE_READLINE */
      line = readline(prompt);
      if(line && *line)
	_add_history_unique (line);
    }
#else
    { /* !fdisk_opt_script_mode */
      printf ("%s", prompt);
      fflush (stdout);
      line = (char*) calloc(sizeof(char), 256);
      if (fgets (line, 256, stdin) == NULL || is_empty_line(line))
	{
	  free (line);
	  line = NULL;
	}
      else
	{
	  char *end = strchr(line, 0x0a);
	  if(end)
	    *end = 0;
	}
    }
#endif
  /* MMM... I assume that `disk_opt_script_mode' is true when fdisk
   * would be used whit a script, but this script where come from,
   * a file or stdin? the following codo works on stdin
   * but we kan do better with an option and a file... ? */
  else
    { /* disk_opt_script_mode */
      line = (char*) calloc(sizeof(char), 256);
      if (fgets (line, 256, stdin) == NULL || is_empty_line(line))
	{
	  free (line);
	  line = NULL;
	}
      else
	{
	  char *end = strchr(line, 0x0a);
	  if(end)
	    *end = 0;
	}
    }
  readline_state.in_readline = 0;
  return line;
}

static PedExceptionOption
option_get_next (PedExceptionOption options, PedExceptionOption current)
{
  PedExceptionOption i;

  i = 
    current == 0 ? PED_EXCEPTION_OPTION_FIRST : current * 2;

#if 0
  if (current == 0)
    i = PED_EXCEPTION_OPTION_FIRST;
  else
    i = current * 2;
#endif 

  for (; i <= options; i *= 2) 
    {
      if (options & i)
	return i;
    }
  return 0;
}

static void
_print_exception_text (PedException* ex)
{
  StrList*		text;
  char* bug_msg = _(
"You found a bug in GNU Fdisk.\n\
This may have been fixed in the last version of \
GNU Parted that you can find at:\n\
\thttp://ftp.gnu.org/gnu/parted/\n\n\
Please check this version prior to bug reporting.\n\n\
If this has not been fixed yet or if you don't \
know how to check, please email:\n\
\tbug-parted@gnu.org\n\
or (preferably) file a bug report at:\n\
\thttp://parted.alioth.debian.org/bugs/\n\n\
Your report should contain the version of this release (%s) along with the\n\
error message below, the output of\n\
\tparted DEVICE unit co print unit s print\n\
and additional information about your setup you consider important.\n");

  fdisk_wipe_line ();

  if (ex->type == PED_EXCEPTION_BUG) 
    {
      printf (bug_msg, VERSION);
      text = str_list_create ("\n", ex->message, "\n", NULL);
    } 
  else 
    text = str_list_create (_(ped_exception_get_type_string (ex->type)),
			    ": ", ex->message, "\n", NULL);
  str_list_print_wrap (text, fdisk_screen_width (), 0, 0);
  str_list_destroy (text);
}

static PedExceptionOption
exception_handler (PedException* ex)
{
  PedExceptionOption opt;

  _print_exception_text (ex);

  /* only one choice?  Take it ;-) */
  opt = option_get_next (ex->options, 0);
  if (!option_get_next (ex->options, opt))
    return opt;

  /* script-mode: don't handle the exception */
  if (fdisk_opt_script_mode)
    return PED_EXCEPTION_UNHANDLED;

  do {
    opt = fdisk_command_line_get_ex_opt ("", ex->options);
  } while (opt == PED_EXCEPTION_UNHANDLED && isatty (0));
  return opt;
}

static int
_str_is_spaces (const char* str)
{
	while (isspace (*str)) str++;
	return *str == 0;
}


static char*
realloc_and_cat (char* str, const char* append)
{
  int length = strlen (str) + strlen (append) + 1;
  char*	new_str = realloc (str, length);
  strcat (new_str, append);
  return new_str;
}

static char*
_construct_prompt (const char* head, const char* def,
		   const StrList* possibilities)
{
  char*	prompt = strdup (head);

  if (def && possibilities)
    PED_ASSERT (str_list_match_any (possibilities, def), return NULL);

  if (possibilities &&
      prompt_possibilities &&
      str_list_length (possibilities) < 8) 
    {
      const StrList* walk;
      if (strlen (prompt))
	prompt = realloc_and_cat (prompt, " ");
      
      for (walk = possibilities; walk; walk = walk->next) 
	{
	  if (walk != possibilities)
	    prompt = realloc_and_cat (prompt, "/");
	  if (def && str_list_match_node (walk, def) == 2) 
	    {
	      prompt = realloc_and_cat (prompt, "[");
	      prompt = realloc_and_cat (prompt, def);
	      prompt = realloc_and_cat (prompt, "]");
	    } 
	  else 
	    {
	      char* text = str_list_convert_node (walk);
	      prompt = realloc_and_cat (prompt, text);
	      free (text);
	    }
	}
      prompt = realloc_and_cat (prompt, ": ");
    } 
  else if (def) 
    {
      size_t len = strlen (prompt);
      if (len && prompt[len-1] != '\n')
	prompt = realloc_and_cat (prompt, "  ");
      prompt = realloc_and_cat (prompt, "(default ");
      prompt = realloc_and_cat (prompt, def);
      prompt = realloc_and_cat (prompt, "): ");
    } 
  else 
    {
      size_t len = strlen (prompt);
      if (len && prompt[--len] != '\n') 
	{
	  if (prompt[len] == ':' || prompt[len] == '?')
	    prompt = realloc_and_cat (prompt, " ");
	  else
	    prompt = realloc_and_cat (prompt, ": ");
	}
    }
  return prompt;
}

static int
_can_create_primary (const PedDisk* disk)
{
  int i;

  for (i = 1; i <= ped_disk_get_max_primary_partition_count (disk); i++) 
    {
      if (!ped_disk_get_partition (disk, i))
	return 1;
    }
  return 0;
}

static int
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

static int
_can_create_logical (const PedDisk* disk)
{
  if (!ped_disk_type_check_feature (disk->type, PED_DISK_TYPE_EXTENDED))
    return 0;
  return ped_disk_extended_partition (disk) != 0;
}

static int
init_ex_opt_str ()
{
  int i;
  PedExceptionOption opt;

  for (i = 0; (1 << i) <= PED_EXCEPTION_OPTION_LAST; i++) 
    {
      opt = (1 << i);
      ex_opt_str[i] = 
	str_list_create (ped_exception_get_option_string (opt),
			 _(ped_exception_get_option_string (opt)), NULL);
      if (!ex_opt_str [i])
	return 0;
    }
  ex_opt_str [i] = NULL;
  return 1;
}

static void
done_ex_opt_str ()
{
  int i;

  for (i=0; ex_opt_str [i]; i++)
    str_list_destroy (ex_opt_str [i]);
}

static int
init_state_str ()
{
  on_list = str_list_create_unique (_("on"), "on", NULL);
  off_list = str_list_create_unique (_("off"), "off", NULL);
  on_off_list = str_list_join (str_list_duplicate (on_list),
			       str_list_duplicate (off_list));
  return 1;
}

static void
done_state_str ()
{
  str_list_destroy (on_list);
  str_list_destroy (off_list);
  str_list_destroy (on_off_list);
}

static int
init_fs_type_str ()
{
  PedFileSystemType* walk;

  fs_type_list = NULL;

  for (walk = ped_file_system_type_get_next (NULL); walk;
       walk = ped_file_system_type_get_next (walk))
    {
      fs_type_list = str_list_insert (fs_type_list, walk->name);
      if (!fs_type_list)
	return 0;
    }
  return 1;
}

static int
init_disk_type_str ()
{
  PedDiskType* walk;

  disk_type_list = NULL;

  for (walk = ped_disk_type_get_next (NULL); walk;
       walk = ped_disk_type_get_next (walk))
    {
      disk_type_list = str_list_insert (disk_type_list, walk->name);
      if (!disk_type_list)
	return 0;
    }
  return 1;
}

void
fdisk_usage_msg()
{
  char *s = _("Usage: fdisk [OPTION]... [DEVICE]");
  puts(s);
}

void
fdisk_help_msg ()
{
  fdisk_usage_msg();

  printf ("\n%s\n", _("OPTIONs:"));
  fdisk_print_options_help ();
/*
	printf ("\n%s\n", _("COMMANDs:"));
	fdisk_print_commands_help ((FdiskCommand**)NULL);
*/
  exit (0);
}

/* move all in a function so we reduce stack size */
static void
do_banner_message(void)
{
  char* banner_msg = 
"Copyright (C) 1998 - 2006 Free Software Foundation, Inc.\n\
This program is free software, covered by the GNU General Public License.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\n";

  StrList* list;
  
  list = str_list_create (_(banner_msg), NULL);
  str_list_print_wrap (list, fdisk_screen_width (), 0, 0);
  str_list_destroy (list);
}

/*
 * Begin interactive mode
 */

/* Changed */
/* 
int
fdisk_interactive_mode (PedDisk** disk, FdiskCommand* cmd_list[]) 
*/

int
fdisk_interactive_mode(PedDevice **dev, FdiskCommand* cmd_list[])
{
  char*	line;
  PedDisk *disk;
  StrList* list;
  StrList* command_names = fdisk_command_get_names (cmd_list);
    
  puts (interface_name);
  do_banner_message();

  /* Christian <mail.kristian@yahoo.it>:
   * FIRST try open device as a disk with a partition table, if this doesn't
   * work device has not partition table so we must make it */
 
   
  if(ped_disk_probe(*dev) == NULL)
    {
      if(compat_mode)
	{
	  /* If we are in compat mode we must create a dos partition table */
	  puts( _("Device contains neither a valid DOS partition table, "
		  "nor Sun, SGI or OSF disklabel\n"
		  "Building a new DOS disklabel. Changes will remain in memory only,\n"
		  "until you decide to write them. After that, of course, the previous\n"
		  "content won't be recoverable.\n"));
	  if((disk = ped_disk_new_fresh(*dev, ped_disk_type_get("msdos"))) == NULL)
	    return EXIT_FAILURE;
	}
      else
	{
	  /* In extended mode we can create a partition table with all supported modes */
	  PedDiskType *label_type;
	  StrList *supp = NULL;
	  char *supported_types;
	  char *str;

	  for(label_type = ped_disk_type_get_next(NULL);
	      label_type = ped_disk_type_get_next(label_type); )
	    {
	      supp = str_list_append(supp, label_type->name);
	      supp = str_list_append(supp, " ");
	    }

	  supported_types = str_list_convert(supp);
	  str_list_destroy(supp);

	  /* List resources was freed */

	  puts(_("Device contains neither a valid DOS partition table, "
		 "nor Sun, SGI or OSF disklabel.\n"
		 "You must create a disk label first!\n"));
	  
	  str = malloc(32);

	get_label_type_str:
	  
	  printf(_("What type of disk label would you create (q to quit)?\n[ %s] "), 
		 supported_types);

	retry_without_print_message:
	  
	  if(scanf("%31s", str) < 1)
	    {
	      goto retry_without_print_message;
	    }
	  else if((*str == 'q' || *str == 'Q') && str[1] == 0)
	    {
	      return EXIT_SUCCESS;
	    }
	  else if((label_type = ped_disk_type_get(str)) == NULL)
	    {
	      fputs(_("*** Invalid label!\n"), stderr);
	      goto get_label_type_str;	      
	    }

	  disk = ped_disk_new_fresh(*dev, label_type);

	  if(!disk)
	    return EXIT_FAILURE; /* what bad? :( */

	  /* Free other resources */
	  free(str);
	  free(supported_types);
	  
	}
    } /* if(!disk) */
  else
    {
      disk = ped_disk_new(*dev);
      if(!disk)
	return EXIT_FAILURE;
    }
  
  /***
   ** OK, We have a disk with a partition table :) 
   */
  
  /* Show the size of the partition */
  if(fdisk_partsize_device)
    fdisk_print_partition_size(disk);
  
  /* List the specified disk. */
  if (fdisk_list_table)
    /* http://lists.gnu.org/archive/html/bug-fdisk/2008-12/msg00011.html */
    /* fdisk_do_list_devices(&disk); */
    fdisk_do_list_devices(disk);

  fdisk_print_using_dev (disk->dev);

  while (1) 
    {
      char* word;
      FdiskCommand* cmd;

      commands = cmd_list;	/* FIXME yucky, nasty, evil hack */

      while (!fdisk_command_line_get_word_count ()) 
	{
	  if (feof (stdin))
	    {
	      printf ("\n");
	      return 1;
	    }
	  prompt_possibilities = 0;
	  fdisk_command_line_prompt_words ("Command (m for help):", 
					   NULL, command_names, 1);
	  prompt_possibilities = 1;
	}
      word = fdisk_command_line_pop_word ();
      if (word) 
	{
	  cmd = fdisk_command_get (commands, word);
	  free (word);
	  if (cmd) 
	    {
	      if (!fdisk_command_run (cmd, &disk))
		fdisk_command_line_flush ();
	    } 
	  else 
	    {
	      fdisk_print_commands_help (commands);
	    }
	}
    }
  return 1;
}

/* If menu equals:
 * 0 - expert menu.
 * 1 - filesystem menu.
 * 2 - bsd menu
 */
int
fdisk_interactive_menu (PedDisk** disk, FdiskCommand* cmd_list[], int menu)
{
	char*		line;
	StrList*	list;
	StrList*	command_names = fdisk_command_get_names (cmd_list);

	/* FIXME yucky, nasty, evil hack */
	FdiskCommand **saved_commands = commands;
	commands = cmd_list;


	while (1) {
		char*		word;
		FdiskCommand*	cmd;

		while (!fdisk_command_line_get_word_count ()) {
			if (feof (stdin)) {
				printf ("\n");
				commands = saved_commands;
				return 1;
			}
			prompt_possibilities = 0;
			if (!menu)
			  fdisk_command_line_prompt_words ("Expert command (m for help)", NULL,
							   command_names, 1);
			else if (menu == 1)
			  fdisk_command_line_prompt_words ("Filesystem (m for help)", NULL,
							   command_names, 1);
			else if (menu == 2)
			  fdisk_command_line_prompt_words ("BSD disklabel command (m for help)", NULL,
							   command_names, 1);
			prompt_possibilities = 1;
		}

		word = fdisk_command_line_pop_word ();
		if (word) {
			cmd = fdisk_command_get (commands, word);
			free (word);
			if (cmd) {
				if (strcmp ((char *)cmd->names->str, "r")) {
			    		if (!fdisk_command_run (cmd, disk))
			      			fdisk_command_line_flush ();
			  		} else {
						commands = saved_commands;
			    			return 1;
			  		}
			} else {
				fdisk_print_commands_help ((FdiskCommand **)NULL);
			}
		}
	}

	commands = saved_commands;
	return 1;
}

/* Prompt the user to answer a yes or no question. */
int
command_line_prompt_boolean_question (const char* prompt) {
	char        *user_ans;
	StrList *possibilities = str_list_create (_("yes"), _("no"), NULL);
	user_ans = fdisk_command_line_get_word (_(prompt), _("no"), possibilities, 0);

	if (strcmp (user_ans, _("yes")) == 0)
		return 1;

	/* user answered no */
	return 0;
}

int
fdisk_screen_width ()
{
  int width = 0;
  if (fdisk_opt_script_mode)
    return 32768;	/* no wrapping ;) */

/* HACK: don't specify termcap separately - it'll annoy the users. */
#if HAVE_LIBREADLINE && HAVE_LIBTERMCAP
  if(termcap_status == TERMCAP_OK)
    width = tgetnum ("co");
#endif
  if (width <= 0)
    width = 80;
  return width;
}

void
fdisk_wipe_line ()
{
	if (fdisk_opt_script_mode)
		return;

	/* yuck */
	printf (
"\r                                                                          \r"
	);
}


PedExceptionOption
fdisk_command_line_get_ex_opt (const char* prompt, PedExceptionOption options)
{
	Option			optlist[10];
	PedExceptionOption	opt;
	const char*		opt_name;
	int			i = 0, result;

	for (opt = option_get_next (options, 0); opt && i < sizeof(optlist) - 1;
	     opt = option_get_next (options, opt), i++) {
		opt_name = ped_exception_get_option_string (opt);
		optlist[i].option = tolower(opt_name[0]);
		/* FIXME: Localization here won't work this way */
		optlist[i].description = _(ped_exception_get_option_string (opt));
	}
	optlist[i].option = 0;
	result = fdisk_command_line_get_option (prompt, optlist);
	if (!result)
		return PED_EXCEPTION_UNHANDLED;

	for (opt = option_get_next (options, 0); opt;
	     opt = option_get_next (options, opt)) {
		opt_name = ped_exception_get_option_string (opt);
		if (result == tolower(opt_name[0]))
			break;
	}
	return opt;
}

void
fdisk_print_using_dev (PedDevice* dev)
{
	printf (_("Using %s\n"), dev->path);
}

int
fdisk_command_line_get_word_count ()
{
	return str_list_length (command_line);
}

void
fdisk_command_line_prompt_words (const char* prompt, const char* def,
				 const StrList* possibilities, int multi_word)
{
	char*	line;
	char*	real_prompt;
	char*	_def = (char*) def;
	int	_def_needs_free = 0;

	if (!def && str_list_length (possibilities) == 1) {
		_def = str_list_convert_node (possibilities);
		_def_needs_free = 1;
	}

	if (fdisk_opt_script_mode) {
		if (_def)
			fdisk_command_line_push_line (_def, 0);
		return;
	}

	do {
		real_prompt = _construct_prompt (prompt, _def, possibilities);
		line = _readline (real_prompt, possibilities);
		free (real_prompt);
		if (!line)
			break;

		if (!strlen (line)) {
			if (_def)
				fdisk_command_line_push_line (_def, 0);
		} else {
			fdisk_command_line_push_line (line, multi_word);
		}
		free (line);
	} while (!fdisk_command_line_get_word_count () && !_def);

	if (_def_needs_free)
		free (_def);
}

void
fdisk_command_line_flush ()
{
	str_list_destroy (command_line);
	command_line = NULL;
}

/**
 * Get a word from command line.
 *
 * \param possibilities a StrList of valid strings, NULL if all are valid.
 * \param multi_word whether multiple words are allowed.
 *
 * \return The word(s), or NULL if empty.
 */
char*
fdisk_command_line_get_word (const char* prompt, const char* def,
		       const StrList* possibilities, int multi_word)
{
	do {
		if (fdisk_command_line_get_word_count ()) {
			char*		result = fdisk_command_line_pop_word ();
			StrList*	result_node;

			if (!possibilities)
				return result;
			result_node = str_list_match (possibilities, result);
			free (result);
			if (result_node)
				return str_list_convert_node (result_node);
			fdisk_command_line_flush ();
		}

		fdisk_command_line_prompt_words (prompt, def, possibilities,
						multi_word);
	} while (fdisk_command_line_get_word_count ());

	return NULL;
}

/* "multi_word mode" is the "normal" mode... many words can be typed,
 * delimited by spaces, etc.
 * 	In single-word mode, only one word is parsed per line.
 * Leading and trailing spaces are removed.  For example: " a b c "
 * is a single word "a b c".  The motivation for this mode is partition
 * names, etc.  In single-word mode, the empty string is a word.
 * (but not in multi-word mode).
 */
void
fdisk_command_line_push_line (const char* line, int multi_word)
{
	int	quoted = 0;
	char	quote_char = 0;
	char	this_word [256];
	int	i;

	do {
		while (*line == ' ')
			line++;

		i = 0;
		for (; *line; line++) {
			if (*line == ' ' && !quoted) {
				if (multi_word)
					break;

			/* single word: check for trailing spaces + eol */
				if (_str_is_spaces (line))
					break;
			}

			if (!quoted && strchr ("'\"", *line)) {
				quoted = 1;
				quote_char = *line;
				continue;
			}
			if (quoted && *line == quote_char) {
				quoted = 0;
				continue;
			}

			/* hack: escape characters */
			if (quoted && line[0] == '\\' && line[1])
				line++;

			this_word [i++] = *line;
		}
		if (i || !multi_word) {
			this_word [i] = 0;
			fdisk_command_line_push_word (this_word);
		}
	} while (*line && multi_word);
}

char*
fdisk_command_line_pop_word ()
{
	char*		result;
	StrList*	next;

	PED_ASSERT (command_line != NULL, return NULL);

	result = str_list_convert_node (command_line);
	next = command_line->next;

	str_list_destroy_node (command_line);
	command_line = next;
	return result;
}

void
fdisk_command_line_push_word (const char* word)
{
	command_line = str_list_append (command_line, word);
}

#if 0
int
fdisk_command_line_get_disk (const char* prompt, PedDisk** value)
{
	PedDevice*	dev = *value ? (*value)->dev : NULL;
	if (!fdisk_command_line_get_device (prompt, &dev))
		return 0;
	if (dev != (*value)->dev) {
		PedDisk* new_disk = ped_disk_new (dev);
		if (!new_disk)
			return 0;
		*value = new_disk;
	}
	return 1;
}
#endif

#if 0
int
fdisk_command_line_get_partition (const char* prompt, PedDisk* disk,
			    PedPartition** value)
{
	PedPartition*	part;
	int		num = (*value) ? (*value)->num : 0;

	if (!fdisk_command_line_get_integer (prompt, &num)) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				     _("Expecting a partition number."));
		return 0;
	}
	part = ped_disk_get_partition (disk, num);
	if (!part) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				     _("Partition doesn't exist."));
		return 0;
	}
	*value = part;
	return 1;
}
#endif

#if 0
int
fdisk_command_line_get_fs_type (const char* prompt, const PedFileSystemType*(* value))
{
	char*			fs_type_name;
	PedFileSystemType*	fs_type;

	fs_type_name = fdisk_command_line_get_word (prompt,
						    *value ? (*value)->name : NULL,
						    fs_type_list, 1);
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
	*value = fs_type;
	return 1;
}
#endif

#if 0
int
fdisk_command_line_get_disk_type (const char* prompt, const PedDiskType*(* value))
{
	char*		disk_type_name;
	PedDiskType*	disk_type;

	disk_type_name = fdisk_command_line_get_word (prompt,
						*value ? (*value)->name : NULL,
						disk_type_list, 1);
	if (!disk_type_name) {
		ped_exception_throw (PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
				     _("Expecting a disk label type."));
		return 0;
	}
	*value = ped_disk_type_get (disk_type_name);
	free (disk_type_name);
	PED_ASSERT (*value != NULL, return 0);
	return 1;
}
#endif

#if 0
int
fdisk_command_line_get_device (const char* prompt, PedDevice** value)
{
	char*		def_dev_name = *value ? (*value)->path : NULL;
	char*		dev_name;
	PedDevice*	dev;

	dev_name = fdisk_command_line_get_word (prompt, def_dev_name, NULL, 1);
	if (!dev_name)
		return 0;
	dev = ped_device_get (dev_name);
	free (dev_name);
	if (!dev)
		return 0;
	*value = dev;
	return 1;
}
#endif

int
fdisk_command_line_get_integer (const char* prompt, int* value)
{
	char	def_str [12];
	char*	input;
	int	valid;

	snprintf (def_str, sizeof(def_str), "%d", *value);
	input = fdisk_command_line_get_word (prompt, *value ? def_str : NULL,
				       NULL, 1);
	if (!input)
		return 0;
	valid = sscanf (input, "%d", value);
	free (input);
	return valid;
}

fdisk_command_line_get_llinteger (const char* prompt, long long* value)
{
	char	def_str [22];
	char*	input;
	int	valid;

	snprintf (def_str, sizeof(def_str), "%lld", *value);
	input = fdisk_command_line_get_word (prompt, *value ? def_str : NULL,
				       NULL, 1);
	if (!input)
		return 0;
	valid = sscanf (input, "%lld", value);
	free (input);
	return valid;
}

int
fdisk_command_line_get_option (const char* head, const Option* opts)
{
	StrList *possibilities = NULL;
	char *prompt = strdup(head);
	char *result;
	char buf[128];
	int i;

	buf[1] = '\0';

	for (i = 0; opts[i].option; i++) {
		/* Initialise the possibilities StrList */
		buf[0] = opts[i].option;
		buf[1] = '\0';

		possibilities = str_list_append (possibilities, buf);

		/* Initialise the prompt string */
		if (prompt[0] == '\0')
			snprintf(buf,sizeof(buf),"   %c   %s",
		        	 opts[i].option, opts[i].description);
		else
			snprintf(buf,sizeof(buf),"\n   %c   %s",
		        	 opts[i].option, opts[i].description);
		prompt = realloc_and_cat (prompt, buf);
	}
	/* We add a newline at the end, so _construct_prompt
           handles it as it should */
	prompt = realloc_and_cat (prompt, "\n");


	/* Queries the user about the option, we turn off the automatic
	   adding of possibilities to the prompt */
	prompt_possibilities = 0;
	result = fdisk_command_line_get_word (prompt, NULL, possibilities, 1);
	prompt_possibilities = 1;

	/* We can use i, not very intuitive to the coder, but who cares */
	if (result) {
		i = result[0];
		free(result);
	}
	else {
		i = 0;
	}
	free(prompt);
	return i;
}

int
fdisk_get_partpos (const char* prompt, const void* context, const char *possibilities)
{

	const Option *orig_opts = (Option *) context;
	Option opts[5];
	int i,j;
	for (i=0, j=0; orig_opts[i].option && j < 5; i++) {
		if (strchr(possibilities,orig_opts[i].option)) {
			opts[j].option = orig_opts[i].option;
			opts[j].description = orig_opts[i].description;
			j++;
		}
	}
	opts[j].option = 0;

	return fdisk_command_line_get_option(prompt, opts);
}
int
fdisk_command_line_get_part_type (const char* prompt, const PedDisk *disk,
                                  PedPartitionType *type)
{
	Option opts[4];
	int i = 0, temp;
	if (_can_create_extended (disk)) {
		opts[i].option = 'e';
		opts[i].description = _("extended");
		i++;
	}
	if (_can_create_logical (disk)) {
		temp = PED_PARTITION_LOGICAL;
		opts[i].option = 'l';
		opts[i].description = _("logical (5 or over)");
		i++;
	}
	if (_can_create_primary (disk)) {
		opts[i].option = 'p';
		opts[i].description = _("primary partition (1-4)");
		i++;
	}
	opts[i].option = 0;
	if (i == 0)
		return 0;
	else if (i == 1)
		temp = opts[0].option;
	else
		temp = fdisk_command_line_get_option(prompt, opts);
	if (temp == 'e')
		*type = PED_PARTITION_EXTENDED;
	else if (temp == 'l')
		*type = PED_PARTITION_LOGICAL;
	else if (temp == 'p')
		*type = PED_PARTITION_NORMAL;
	else
		return 0;
	return 1;
}
#if 0
int
fdisk_command_line_get_part_type (const char* prompt, const PedDisk* disk,
	       		    PedPartitionType* type)
{
	StrList*	opts = NULL;
	char*		type_name;

	if (_can_create_primary (disk)) {
		opts = str_list_append_unique (opts, "primary");
		opts = str_list_append_unique (opts, _("primary"));
	}
	if (_can_create_extended (disk)) {
		opts = str_list_append_unique (opts, "extended");
		opts = str_list_append_unique (opts, _("extended"));
	}
	if (_can_create_logical (disk)) {
		opts = str_list_append_unique (opts, "logical");
		opts = str_list_append_unique (opts, _("logical"));
	}
	if (!opts) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Can't create any more partitions."));
		return 0;
	}

	type_name = fdisk_command_line_get_word (prompt, NULL, opts, 1);
	str_list_destroy (opts);

	if (!type_name) {
		ped_exception_throw (
			PED_EXCEPTION_ERROR, PED_EXCEPTION_CANCEL,
			_("Expecting a partition type."));
		return 0;
	}

	if (!strcmp (type_name, "primary")
			|| !strcmp (type_name, _("primary"))) {
		*type = 0;
	}
	if (!strcmp (type_name, "extended")
			|| !strcmp (type_name, _("extended"))) {
		*type = PED_PARTITION_EXTENDED;
	}
	if (!strcmp (type_name, "logical")
			|| !strcmp (type_name, _("logical"))) {
		*type = PED_PARTITION_LOGICAL;
	}

	free (type_name);
	return 1;
}
#endif

char*
fdisk_command_line_peek_word ()
{
	if (command_line)
		return str_list_convert_node (command_line);
	else
		return NULL;
}

#if 0
int
fdisk_command_line_get_sector (const char* prompt, PedDevice* dev, PedSector* value,
			       PedGeometry** range)
{
	char*	def_str;
	char*	input;
	int	valid;

	def_str = ped_unit_format (dev, *value);
	input = fdisk_command_line_get_word (prompt, *value ? def_str : NULL,
				       NULL, 1);

	/* def_str might have rounded *value a little bit.  If the user picked
	 * the default, make sure the selected sector is identical to the
	 * default.
	 */
	if (input && *value && !strcmp (input, def_str)) {
		if (range) {
			*range = ped_geometry_new (dev, *value, 1);
			PED_FREE (def_str);
			return *range != NULL;
		}

		PED_FREE (def_str);
		return 1;
	}

	PED_FREE (def_str);
	if (!input) {
		*value = 0;
		if (range)
			*range = NULL;
		return 0;
	}

	valid = ped_unit_parse (input, dev, value, range);

	free (input);
	return valid;
}
#endif

int
fdisk_init_ui ()
{
/* Readline support */

/* End readline support */
  fdisk_opt_script_mode = !isatty (0);
  
  if (!init_ex_opt_str ()
      || !init_state_str ()
      || !init_fs_type_str ()
      || !init_disk_type_str ())
    return 0;
  ped_exception_set_handler (exception_handler);

#if HAVE_LIBREADLINE
  rl_initialize ();
	rl_attempted_completion_function = (CPPFunction*) fdisk_complete_function;
#endif /* HAVE_LIBREADLINE */

  readline_state.in_readline = 0; 
  signal (SIGINT, &interrupt_handler);

  return 1;
}

void
fdisk_done_ui ()
{
  ped_exception_set_handler (NULL);
  done_ex_opt_str ();
  done_state_str ();
  str_list_destroy (fs_type_list);
  str_list_destroy (disk_type_list);
}

int
fdisk_command_line_get_part_flag (const char* prompt, const PedPartition* part,
			    PedPartitionFlag* flag)
{
	StrList*		opts = NULL;
	PedPartitionFlag	walk = 0;
	char*			flag_name;

	while ( (walk = ped_partition_flag_next (walk)) ) {
		if (ped_partition_is_flag_available (part, walk)) {
			const char*	walk_name;

			walk_name = ped_partition_flag_get_name (walk);
			opts = str_list_append (opts, walk_name);
			opts = str_list_append_unique (opts, _(walk_name));
		}
	}

	flag_name = fdisk_command_line_get_word (prompt, NULL, opts, 1);
	str_list_destroy (opts);

	if (flag_name) {
		*flag = ped_partition_flag_get_by_name (flag_name);
		PED_FREE (flag_name);
		return 1;
	} else {
		return 0;
	}
}

int
fdisk_command_line_get_state (const char* prompt, int* value)
{
	char*	def_word;
	char*	input;

	if (*value)
		def_word = str_list_convert_node (on_list);
	else
		def_word = str_list_convert_node (off_list);
	input = fdisk_command_line_get_word (prompt, def_word, on_off_list, 1);
	free (def_word);
	if (!input)
		return 0;
	if (str_list_match_any (on_list, input))
		*value = 1;
	else
		*value = 0;
	free (input);
	return 1;
}

int
fdisk_command_line_get_unit (const char* prompt, PedUnit* unit)
{
	StrList*	opts = NULL;
	PedUnit		walk;
	char*		unit_name;
	const char*	default_unit_name;

	for (walk = PED_UNIT_FIRST; walk <= PED_UNIT_LAST; walk++)
		opts = str_list_append (opts, ped_unit_get_name (walk));

	default_unit_name = ped_unit_get_name (ped_unit_get_default ());
	unit_name = fdisk_command_line_get_word (prompt, default_unit_name, opts, 1);
	str_list_destroy (opts);

	if (unit_name) {
		*unit = ped_unit_get_by_name (unit_name);
		free (unit_name);
		return 1;
	} else {
		return 0;
	}
}

int
fdisk_command_line_is_integer ()
{
	char*	word;
	int	is_integer;
	int	scratch;

	word = fdisk_command_line_peek_word ();
	if (!word)
		return 0;

	is_integer = sscanf (word, "%d", &scratch);
	free (word);
	return is_integer;
}

