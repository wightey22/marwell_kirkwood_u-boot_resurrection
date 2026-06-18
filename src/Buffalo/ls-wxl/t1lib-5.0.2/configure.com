$ goto start

This command file writes the command files make.com in this directory, make.com
and sysconf.h in [.lib.t1lib], make.com in [.lib.type1], and make.com in
[.xglyph].  Executing the make.com in this directory will use the other
make.com files to compile the C files and put them in the T1 object library in
this directory, and compile and link xglyph.

Xglyph requires the xaw and xmu libraries; I used the libraries that came with
ghostview.  This command file looks for a required header file in
X11_ROOT:[XAW].  If it does not find it looks for an X11 directory tree
parallel to this tree.  If it does not find it there, it creates the command
file, but you have to edit it to tell it where the files and libraries are
before you can execute it.  I linked ghostview using the static version of xmu
and xaw, so I link to object libraries instead of sharable images.  I renamed
the xaw3d directory to xaw in the x11 tree of ghostview so I did not have to
change xglyph.c.  To run xglyph on V6.* of VMS, you need to define T1LIB_CONFIG
to be the T1LIB.CONFIG file in the xglyph directory; it will also not show the
timing information on V6.*.

I have compiled the library and run xglyph on VMS V6.2, both VAX and Alpha; I
used DECC V5.6.  I use IEEE floating on Alpha and G floating on VAX.

John Hasstedt
Physics Department
State University of New York at Stony Brook
John.Hasstedt@sunysb.edu

Nov-1999

- Extended the functionality to check for xaw/xmu also in the X11 path (which 
  is the most Unix compatible way to store them ;-).
- Added tests for shareable version of the Xaw/Xmu libraries, using the 
  shareable images if the appropriate logicals are defined. If not  the object 
  libraries are used as before.
- Added support for type1afm build
- Adapted to t1lib 0.9.2

Dec-1999

- Added support for absolute pathnames for font files under OpenVMS

Aug-2000

- First cut of an incremental build

Jul-2002

- Create a shareable image on OpenVMS Alpha

Feb-2003

- Add missing initialization for xaw_root
- Clarify description of Xglyph as >>test<< application
- Improved method to build options file for shareable image
  (Thanks to Hartmut Becker Hartmut.Becker@compaq.com)
    
Feb-2004

- Add DECC$NO_ROOTED_SEARCH_LISTS definition to get Xglyph compile ok
  on later versions of Compaq C
  
Martin Zinser
zinser@decus.de

$start:
$ ! If a user has a symbol cc or something else, don't use it.
$ set symbol/scope=(nolocal,noglobal)
$ say = "write sys$output"
$ !
$ say "Getting the version number"
$ t1lib_identifier = "?.?"
$ open/read in configure.
$loop:
$ read/end=done in rec
$ if (f$element(0,"=",rec) .nes. "T1LIB_IDENTIFIER") then goto loop
$ t1lib_identifier = f$element(1,"=",rec)
$done:
$ close in
$ !
$ say "Creating make.com"
$ create make.com
$ open/append out make.com
$ if (f$getsyi("ARCH_NAME") .eqs. "VAX")
$ then
$   float = "g_float"
$ else
$   float = "ieee_float"
$ endif
$ defs = "T1LIB_IDENT=""""""""""""""""""""""" + t1lib_identifier  - 
         + """""""""""""""""""""""" + "," -
         + "GLOBAL_CONFIG_DIR=""""""""""""""""""""""""T1_FONT_DIR""""""""""""""""""""""""" + "," -
         + "T1_AA_TYPE16=""""""""short"""""""",T1_AA_TYPE32=""""""""int"""""""""
$ write out "$ cflags = ""/prefix=all/float=''float'"""
$ write out "$ cflags = cflags + ""/define=(", defs, ")"""
$ copy sys$input: out
$ deck
$ set default [.lib.t1lib]
$ @make.com
$ write sys$output "Compiling in [.lib.type1]"
$ set default [-.type1]
$ @make.com
$ set default [--]
$ write sys$output "Creating t1.olb"
$ library/create t1.olb [.lib.t1lib]*.obj,[.lib.type1]*.obj
$ write sys$output "Creating xglyph.exe"
$ set default [.xglyph]
$ @make.com
$ set default [-]
$ write sys$output "Creating type1afm.exe"
$ set default [.type1afm]
$ @make.com
$ set default [-]
$!
$! Establish the Compiling Environment
$!
$ Cpu_Model = F$GetSYI("HW_MODEL")
$ open/write optf t1.opt
$T1_L:
$ file = f$search("[.lib.t1lib]*.obj")
$ if file .eqs. "" then goto TY_L
$ write optf file
$ goto t1_l
$TY_L:
$ file = f$search("[.lib.type1]*.obj")
$ if file .eqs. "" then goto end_opt
$ write optf file
$ goto ty_l
$END_OPT:
$ close/nolog optf
$!
$! Alpha gets a shareable image
$!
$ If Cpu_Model .gt. 1024
$ Then
$   write sys$output "Creating t1shr.exe"
$   call anal_obj_axp t1.opt _link.opt
$   open/append optf t1.opt
$   write optf "sys$share:decw$xlibshr.exe/share"
$   close optf
$   LINK_/NODEB/SHARE=t1shr.exe t1.opt/opt,_link.opt/opt
$ endif
$!
$ dele/noconf *.opt;*
$ write sys$output "Done"
$ exit
$!
$!------------------------------------------------------------------------------
$!
$! Analyze Object files for OpenVMS AXP to extract Procedure and Data 
$! information to build a symbol vector for a shareable image
$! All the "brains" of this logic was suggested by Hartmut Becker
$! (Hartmut.Becker@compaq.com). All the bugs were introduced by me
$! (zinser@decus.de), so if you do have problem reports please do not
$! bother Hartmut/HP, but get in touch with me
$!
$ ANAL_OBJ_AXP: Subroutine   
$ V = 'F$Verify(0)
$ SAY := "WRITE_ SYS$OUTPUT"
$ 
$ IF F$SEARCH("''P1'") .EQS. ""
$ THEN
$    SAY "ANAL_OBJ_AXP-E-NOSUCHFILE:  Error, inputfile ''p1' not available"
$    goto exit_aa
$ ENDIF
$ IF "''P2'" .EQS. ""
$ THEN
$    SAY "ANAL_OBJ_AXP:  Error, no output file provided"
$    goto exit_aa
$ ENDIF
$
$ open/read in 'p1
$ create a.tmp
$ open/append atmp a.tmp
$ loop:
$ read/end=end_loop in line
$ f= f$search(line)
$ if f .eqs. ""
$ then
$	write sys$output "ANAL_OBJ_AXP-w-nosuchfile, ''line'"
$	goto loop
$ endif
$ def/user sys$output nl:
$ def/user sys$error nl:
$ anal/obj/gsd 'f /out=x.tmp
$ open/read xtmp x.tmp
$ XLOOP:
$ read/end=end_xloop xtmp xline
$ xline = f$edit(xline,"compress")
$ write atmp xline
$ goto xloop
$ END_XLOOP:
$ close xtmp
$ goto loop
$ end_loop:
$ close in
$ close atmp
$ if f$search("a.tmp") .eqs. "" -
	then $ exit
$ ! all global definitions
$ search a.tmp "symbol:","EGSY$V_DEF 1","EGSY$V_NORM 1"/out=b.tmp
$ ! all procedures
$ search b.tmp "EGSY$V_NORM 1"/wind=(0,1) /out=c.tmp
$ search c.tmp "symbol:"/out=d.tmp
$ def/user sys$output nl:
$ edito/edt/command=sys$input d.tmp
sub/symbol: "/symbol_vector=(/whole
sub/"/=procedure)/whole
exit
$ ! all data
$ search b.tmp "EGSY$V_DEF 1"/wind=(0,1) /out=e.tmp
$ search e.tmp "symbol:"/out=f.tmp
$ def/user sys$output nl:
$ edito/edt/command=sys$input f.tmp
sub/symbol: "/symbol_vector=(/whole
sub/"/=data)/whole
exit
$ sort/nodupl d.tmp,f.tmp 'p2'
$ delete a.tmp;*,b.tmp;*,c.tmp;*,d.tmp;*,e.tmp;*,f.tmp;*
$ if f$search("x.tmp") .nes. "" -
	then $ delete x.tmp;*
$!
$ EXIT_AA:
$ if V then set verify
$ endsubroutine 
$ eod
$ close out
$ !
$ say "Creating [.lib.t1lib]sysconf.h"
$ cc sys$input:/float='float'/object=test.obj
#include <stdlib.h>
#include <stdio.h>
main()
{
  FILE *f=fopen("[.lib.t1lib]sysconf.h", "w");
  if (!f) exit(EXIT_FAILURE);
  fprintf(f, "#define SIZEOF_CHAR %d\n", sizeof(char));
  fprintf(f, "#define SIZEOF_SHORT %d\n", sizeof(short));
  fprintf(f, "#define SIZEOF_INT %d\n", sizeof(int));
  fprintf(f, "#define SIZEOF_LONG %d\n", sizeof(long));
#ifdef __VAX
  fprintf(f, "#define SIZEOF_LONG_LONG %d\n", sizeof(long));
#else
  fprintf(f, "#define SIZEOF_LONG_LONG %d\n", sizeof(long long));
#endif
  fprintf(f, "#define SIZEOF_FLOAT %d\n", sizeof(float));
  fprintf(f, "#define SIZEOF_DOUBLE %d\n", sizeof(double));
#ifdef __VAX
  fprintf(f, "#define SIZEOF_LONG_DOUBLE %d\n", sizeof(double));
#else
  fprintf(f, "#define SIZEOF_LONG_DOUBLE %d\n", sizeof(long double));
#endif
  fprintf(f, "#define SIZEOF_VOID_P %d\n", sizeof(void *));
  exit(EXIT_SUCCESS);
}
$ link test
$ run test
$ delete test.obj;*,test.exe;*
$ !
$ say "Creating [.lib.t1lib]make.com"
$ copy sys$input: [.lib.t1lib]make.com
$ deck
$ call make parseafm.obj "cc ''cflags' parseafm" - 
            parseafm.c
$ call make t1aaset.obj "cc ''cflags' t1aaset" - 
            t1aaset.c
$ call make t1afmtool.obj "cc ''cflags' t1afmtool" - 
            t1afmtool.c
$ call make t1base.obj "cc ''cflags' t1base" - 
            t1base.c 
$ call make t1delete.obj "cc ''cflags' t1delete" - 
            t1delete.c
$ call make t1enc.obj "cc ''cflags' t1enc" - 
            t1enc.c
$ call make t1env.obj "cc ''cflags' t1env" - 
            t1env.c
$ call make t1finfo.obj "cc ''cflags' t1finfo" - 
            t1finfo.c
$ call make t1load.obj "cc ''cflags' t1load" - 
            t1load.c
$ call make t1set.obj "cc ''cflags' t1set" - 
            t1set.c
$ call make t1trans.obj "cc ''cflags' t1trans" - 
            t1trans.c
$ call make t1x11.obj "cc ''cflags' t1x11" - 
            t1x11.c
$ call make t1outline.obj "cc ''cflags' t1outline" - 
            t1outline.c
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
$eod
$ !
$ say "Creating [.lib.type1]make.com"
$ create [.lib.type1]make.com
$ deck
$!
$! Just some general variables
$! 
$ true   = 1
$ false  = 0
$ tmpnam = "temp_" + f$getjpi("","pid")
$ tc     = tmpnam + ".c"
$!
$ gosub check_function
$!
$ call make arith.obj "cc ''cflags' arith" - 
            arith.c
$ call make curves.obj "cc ''cflags' curves" - 
            curves.c
$ call make fontfcn.obj "cc ''cflags' fontfcn" - 
            fontfcn.c
$ call make hints.obj "cc ''cflags' hints" - 
            hints.c
$ call make lines.obj "cc ''cflags' lines" - 
            lines.c
$ call make objects.obj "cc ''cflags' objects" - 
            objects.c
$ call make paths.obj "cc ''cflags' paths" - 
            paths.c
$ call make regions.obj "cc ''cflags' regions" - 
            regions.c
$ call make scanfont.obj "cc ''cflags' scanfont" - 
            scanfont.c
$ call make spaces.obj "cc ''cflags' spaces" - 
            spaces.c
$ call make t1io.obj "cc ''cflags' t1io" - 
            t1io.c
$ call make t1snap.obj "cc ''cflags' t1snap" - 
            t1snap.c
$ call make t1stub.obj "cc ''cflags' t1stub" - 
            t1stub.c
$ call make token.obj "cc ''cflags' token" - 
            token.c
$ call make type1.obj "cc ''cflags' type1" - 
            type1.c
$ call make util.obj "cc ''cflags' util" - 
            util.c
$ exit
$!
$!------------------------------------------------------------------------------
$!
$! Check if a specific function needs to be implemented
$!
$CHECK_FUNCTION:
$!
$! bstring --> memset ...
$!
$ func = "bstring"
$ hfile = ""
$ open/write tmpc 'tc
$ write tmpc "#include <string.h>"
$ write tmpc "#include <stdlib.h>"
$ write tmpc "int main(){"
$ write tmpc "  memset(NULL,1,1);"
$ write tmpc "}"
$ close tmpc
$ gosub cc_prop_check
$!
$ return
$!
$!------------------------------------------------------------------------------
$!
$! Check for properties of C/C++ compiler
$!
$CC_PROP_CHECK:
$ cc_prop = false
$ set message/nofac/noident/nosever/notext
$ on error then continue
$ cc 'all_cflags' 'tmpnam'
$ if $status then cc_prop = true
$ set message/fac/ident/sever/text
$ delete/nolog 'tmpnam'.*;*
$ if cc_prop 
$ then
$   if (func .nes. "") 
$   then
$     write sys$output "Function ''func' implemented in C RTL"
$   else
$     write sys$output "h-file ''hfile' part of C RTL"
$   endif
$ else 
$   if (func .nes. "")
$   then  
$     CALL MAKE 'func'.OBJ  "CC ''cflags' ''func'.C" 'func'.C
$   endif
$   if (hfile .nes. "") then copy 'hfile'.h_in 'hfile'.h
$ endif
$ return
$!
$!------------------------------------------------------------------------------
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
$eod
$ !
$ say "Creating [.xglyph]make.com"
$ xaw_root = ""
$ check_xaw = "X11_ROOT,X11"
$ check_rel = f$parse("[-.x11]") - "].;"
$ if (check_rel .nes. "") 
$ then 
$   define rel_loc "''check_rel'.]"
$   check_xaw = check_xaw + ",rel_loc"
$ endif
$ i = 0
$SEARCH_XAW:
$ dir = f$element(i,",",check_xaw)
$ if (dir.nes.",")
$ then
$   if ( f$search("''dir':[xaw]label.h") .nes. "" ) then xaw_root = dir
$   i = i + 1
$   goto search_xaw
$ endif
$ create [.xglyph]make.com
$ open/append out [.xglyph]make.com
$ if (xaw_root .eqs. "")
$ then
$   say "I could not locate the xaw and xmu files. In case you want" 
$   say "to build the Xglyph test application you need to edit" 
$   say "[.xglyph]make.com to add appropriate paths."
$   write out "$ type/nopage sys$input"
$   write out "In case you want to build the Xglyph test application you"
$   write out "need to edit this file to delete the exit command and define"
$   write out "the dir symbol for the location of the xmu and xaw libraries."
$   write out "$ exit"
$   write out "$ xaw_root = ""<disk>:[dir.X11"""
$ else
$   write out "$ xaw_root = ""''xaw_root'"""
$ endif
$ copy sys$input: out
$ deck
$ ! If a user has a symbol cc or something else, don't use it.
$ set symbol/scope=(nolocal,noglobal)
$ if (f$getsyi("ARCH_NAME") .eqs. "VAX")
$ then
$   float = "g_float"
$   olb_dir = "exe"
$ else
$   float = "ieee_float"
$   olb_dir = "exe_alpha"
$ endif
$ if (xaw_root.nes."X11") then define x11 'xaw_root',decw$include:
$ define x11_directory decw$include:
$ define xmu_directory 'xaw_root':[xmu]
$ define xaw_directory 'xaw_root':[xaw]
$ define olb_directory 'xaw_root':['olb_dir']
$ define DECC$NO_ROOTED_SEARCH_LISTS 1
$ open/write  optf xglyph.opt
$ write optf "xglyph.obj"
$ write optf "[-]t1.olb/library"
$ write optf "sys$share:decc$shr.exe/share"
$ write optf "sys$share:decw$xextlibshr/share"
$ write optf "sys$share:decw$xlibshr/share"
$ write optf "sys$share:decw$xtlibshrr5/share"
$ if (f$trnlnm("xaw3dlibshr").nes."")
$ then
$   write optf "xaw3dlibshr/share"
$ else
$   write optf "olb_directory:xaw3dlib/library"
$ endif
$ if (f$trnlnm("xmulibshr").nes."")
$ then
$   write optf "xmulibshr/share"
$ else
$   write optf "olb_directory:xmulib/library"
$ endif
$ close optf
$ call make xglyph.obj "cc /prefix=all/float='float' xglyph.c" -
            xglyph.c
$ call make xglyph.obj "link xglyph.opt/option" -
            xglyph.obj [-]t1.olb
$ deassign DECC$NO_ROOTED_SEARCH_LISTS
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
$eod
$ close out
$ say "Creating [.type1afm]make.com"
$ create [.type1afm]make.com
$ open/append out [.type1afm]make.com
$ copy sys$input: out
$ deck
$ call make type1afm.obj "cc/prefix=all type1afm.c" -
            type1afm.c
$ call make type1afm.exe "link type1afm,[-]t1/library" -
            type1afm.obj [-]t1.olb
$ exit
$!
$MAKE: SUBROUTINE   !SUBROUTINE TO CHECK DEPENDENCIES
$ V = 'F$Verify(0)
$! P1 = What we are trying to make
$! P2 = Command to make it
$! P3 - P8  What it depends on
$
$ If F$Search(P1) .Eqs. "" Then Goto Makeit
$ Time = F$CvTime(F$File(P1,"RDT"))
$arg=3
$Loop:
$       Argument = P'arg
$       If Argument .Eqs. "" Then Goto Exit
$       El=0
$Loop2:
$       File = F$Element(El," ",Argument)
$       If File .Eqs. " " Then Goto Endl
$       AFile = ""
$Loop3:
$       OFile = AFile
$       AFile = F$Search(File)
$       If AFile .Eqs. "" .Or. AFile .Eqs. OFile Then Goto NextEl
$       If F$CvTime(F$File(AFile,"RDT")) .Ges. Time Then Goto Makeit
$       Goto Loop3
$NextEL:
$       El = El + 1
$       Goto Loop2
$EndL:
$ arg=arg+1
$ If arg .Le. 8 Then Goto Loop
$ Goto Exit
$
$Makeit:
$ VV=F$VERIFY(0)
$ write sys$output P2
$ 'P2
$ VV='F$Verify(VV)
$Exit:
$ If V Then Set Verify
$ENDSUBROUTINE
$eod
$ close out
$ exit
