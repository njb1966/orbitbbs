/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/

#ifndef _VARS_H_
#define _VARS_H_

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <dos.h>
#include <alloc.h>
#include <time.h>
#ifdef __OS2__
#include "os2lib.h"
#endif

#include "fcns.h"

/****************************************************************************/
#ifdef _DEFINE_GLOBALS_

char *ctypes[] = {
  "IBM PC (8088)",
  "IBM AT (80286)",
  "IBM 80386/80486",
  "IBM PS/2",
  "Apple 2",
  "Apple Mac",
  "Commodore Amiga",
  "Commodore",
  "Atari",
  "Other",
  0L,
};

unsigned char *translate_letters[] = {
  "abcdefghijklmnopqrstuvwxyzáÑÜÇÅî§",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZÄéèêöô•",
  0L,
};

unsigned char *valid_letters=
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZÄÅÇÉÑÖÜáàâäãåçéèêíìîïñóòôö°¢£§•";

#define __EXTRN__

/****************************************************************************/
#else
/****************************************************************************/

extern char *ctypes[];
extern char *valid_letters;
extern unsigned char *translate_letters[];

#define __EXTRN__ extern


#endif
/****************************************************************************/


__EXTRN__ char ansistr[81], cdir[81], charbuffer[161], chatreason[81],
               crttype, curspeed[81], dc[81], dcd[81], dszlog[81],
               dtc[81], endofline[81], gatfn[81], irt[81], irt_name[205],
               *languagedir, *net_data, *net_name, net_email_name[205],
               newprompt[161], odc[81], *quotes_nrm, *quotes_ind,
               search_pattern[81], *sp, str_no[20], str_pause[80],
               str_quit[20], str_yes[20], tc[81], ver_no1[51],
               ver_no2[51],wwiv_net_no[20], xdate[9], *xenviron[50],
               dlfn[81],edlfn[81], nete[5], irt_sub[81];

__EXTRN__ unsigned char actsl, andwith, checksum;


__EXTRN__ int abortext, already_on, ansiptr, async_irq, base,
              change_color, charbufferpointer, chatcall,
              chatting, chat_file, checkit, curatr, curdir,
              curdloads, curldir, curlsub, cursub, cur_lang,
              curconfdir, curconfsub, cur_lang_idx, c_dir, c_sub,
              daysmax, daysmin, defscreenbottom, do_event, echo, emchg,
              endday, express, expressabort, flow_control, fsenttoday,
              fwaiting, gat_section, global_handle, global_xx, hangup, helpl,
              high_speed, hungup, ihelp, incom, input_extern, in_extern,
              in_fsed, lastcon, lecho, lines_listed, live_user, ltime,
              mailcheck, max_subs, max_dirs, msgreadlogon, net_num,
              net_num_max, net_type, noklevel, no_hangup, numbatch,
              numbatchdl, numchain, numed, numextrn, numf, nummsgs,
              num_dirs, num_languages, num_sec, num_subs,
              num_sys_list, oklevel, okmacro, okskey, ok_modem_stuff,
              oldx, oldy, ooneuser, outcom, questused[20],
              restoring_shrink, save_dos, screenbottom, screenlen,
              screenlinest, subchg, smwcheck, sysop_alert, tempio,
              topdata, topline, two_color, usernum, useron,
              use_workspace, using_modem, wfc, x_only,
              instance,debuglevel,multitasker,tagging,tagptr,cursormove,
              titled, abortext;

__EXTRN__ unsigned short com_speed, *csn_index, crc, *gat, modem_flag,
                         modem_mode, modem_speed, net_sysnum, curloc, eflags;

__EXTRN__ unsigned int subconfnum, dirconfnum;

__EXTRN__ long hanguptime1, last_time_c, nscandate, this_date,
               timelastchar1, quotes_nrm_l, quotes_ind_l,
               mult_time, num_listed;

__EXTRN__ unsigned long *dir_dates, *sub_dates, g_flags;

__EXTRN__ unsigned long *qsc, *qsc_n, *qsc_q, *qsc_p;


__EXTRN__ float batchtime;

__EXTRN__ double extratimecall, last_time, timeon, time_event, xtime;


__EXTRN__ userrec thisuser;
__EXTRN__ configrec syscfg;
__EXTRN__ configoverrec syscfgovr;
__EXTRN__ statusrec status;
__EXTRN__ smalrec huge *smallist;
__EXTRN__ subboardrec *subboards;
__EXTRN__ directoryrec huge *directories;
__EXTRN__ usersubrec *usub, *udir;
__EXTRN__ userconfrec *uconfsub, *uconfdir;
__EXTRN__ screentype screensave;
__EXTRN__ messagerec helps[50], menus[30],menus1[30],menus2[30];
__EXTRN__ batchrec *batch;
__EXTRN__ tagrec *filelist;
__EXTRN__ chainfilerec *chains;
__EXTRN__ chainregrec *chains_reg;
__EXTRN__ newexternalrec *externs, *over_intern;
__EXTRN__ editorrec *editors;
__EXTRN__ gfiledirrec *gfilesec;
__EXTRN__ modem_info *modem_i;
__EXTRN__ net_system_list_rec huge *csn;
__EXTRN__ net_networks_rec *net_networks;

__EXTRN__ volatile int head,tail;
__EXTRN__ volatile char buffer[MAX_BUF];

__EXTRN__ void far *funcs[25];
__EXTRN__ char far *scrn;

__EXTRN__ char far *point_shrink;

__EXTRN__ languagerec *languages;
__EXTRN__ char *cur_lang_name;

__EXTRN__ confrec far *subconfs, *dirconfs;

__EXTRN__ int iia;

__EXTRN__ long last_iia;

__EXTRN__ system_operation_rec sysinfo;

#ifdef __OS2__
__EXTRN__ FILE *stdprn;
__EXTRN__ HFILE PortHandle;
__EXTRN__ TID RecvThreadID;
#endif

/* from version.c */
extern char *wwiv_version;
extern char *wwiv_date;
extern unsigned int wwiv_num_version;

/* RIP specific variables */
__EXTRN__ int rmenu, rip_pause, end_submenu, autox, rip_ver, rip_popup;
__EXTRN__ char rip, cleared, formery, smally, ripext[4], ripcode,
               rip_subset, user_menus, ripdrive;
__EXTRN__ ripmsgrec menus3[50],menusl[30];

#endif

/****************************************************************************/

#define CONF_SUBS 1
#define CONF_DIRS 2

