/* RIPspeed support functions, copyright (C) 1994 by Zachary G. Ives.   *
 * All rights reserved.                                                 *
 *                                                                      *
 * Version 1.02, 6/6/94.                                                *
 * Version 1.50, 6/20/94.                                               *
 * For WWIV 4.24, 8/01/94.                                              */

#include "vars.h"
#include "ripint.h"

#pragma hdrstop

#include <dir.h>
#include <dos.h>
#include <ctype.h>

int ex_line = 0;
char icon_extension[4];

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

/* Function: autodetect RIPscrip caller by sending out the RIP_VER code and
   looking for "RIP" to be returned.  If nothing for 2 seconds, goes on and
   assumes non-RIP caller.
 */
int rip_check(void)
{
  char c, verstr[10];
  int pos;

#ifdef RIPDRIVE
  if (localrip_detect()) {
	ripdrive = -1;
	puts(get_string(1608));
  }
#endif

  end_submenu = -1;
  while (inkey() && (!hangup));
  outstr(get_string(1459));
  pl("[!");
  c = delaykey(2);
  
  if (c == 'R') {
	if (delaykey(1) == 'I')
	  if (delaykey(1) == 'P') {     /* Check for the beginning of OPT_RIP */

		c = delaykey(1);
                for (pos = 0; (pos < 11) && (c != 0) && (!hangup); pos++) {
		  c = delaykey(1);
		  if (pos > 3)
			verstr[pos - 4] = c;
		}
		if (pos == 11) {
			verstr[pos - 4] = 0;
			rip_ver = atoi(verstr);
                        printf("%s %f\n", get_string(1638), rip_ver / 10000.0);
			strcpy(icon_extension, "ICN");
			if (rip_ver == 15450) {              /* Softerm has only */
				rip_subset = -1;                        /* partial support */
				pl(get_string(1440));
				pl(get_string(1441));
				user_menus = 2;
			} else if (rip_ver < 15400) {
				pl(get_string(1463));
				rip_subset = -1;
			} else {
				rip_subset = 0;

				if (rip_ver == 15410)
				  pl(get_string(1460));
                                else if (rip_ver == 15420) {
				  pl(get_string(1461));
                                  rip_subset = -1;
                                  user_menus = 2;
                                } else if (rip_ver == 15430) {
				  pl(get_string(1609));
				  rip_subset = -1;                        /* partial support */
				  user_menus = 2;
                                } else if (rip_ver >= 20000) {
				  pl(get_string(1610));
				  strcpy(icon_extension,"BMP");
                                } else {
				  pl(get_string(1462));
                                  rip_subset = -1;
                                  user_menus = 2;
				}
			}
		}

#ifdef RIPDRIVE
		if (ripdrive) {
		  topdata = 0;
		  puts(get_string(1608));
		  if (sysinfo.flags & OP_FLAGS_RIPDRIVE_ON)
			localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
		}
#endif

		return (-1);
	  }
  }
  ripdrive=0;
  return (0);
}

/* Function: display instructions and allow user to download the menus.
   Uses external PRESEND_.BAT and SEND_.BAT batch files.
 */
int transmit(char menutype)
{
        char f[67], ch, iconflag = 0;
	char files[256][13];
	int numfiles = 0;
	int snd;
	double pct;
	FILE *namefile, *mfile;
//    userrec tuser;

	if (rip_subset)
	  return 0;

	rip_cls();
	sprintf(f, "%sNAME.MNU", sysinfo.ripdir);
	if ((namefile = fopen(f, "w")) != NULL) {
	  if (rip_ver >= 20000) {
		sprintf(f, "%sNAME.M%c", sysinfo.ripdir, menutype);

		if ((mfile = fopen(f, "r")) != NULL) {
		  fgets(f, 60, mfile);
		  fputs(f, namefile);
		  fclose(mfile);
		}
	  }
	  fprintf(namefile, "!|T%s\n", syscfg.systemname);
	  fclose(namefile);
	}
	
	tmp_disable_pause(1);
	sprintf(f, "%sMENUS.TXT", sysinfo.ripdir);
	printfile(f);
	tmp_disable_pause(0);
	rip_show("menu332.mn1");        // Pause
	getkey();
	if (hangup)
	  return 0;
	rip_show("menu333.mn1");        // Clear

	sprintf(f, "MENU0.MN%c", menutype);
	switch (current_menu(f)) {
	  case 1:
//        pl(get_string(1611));
		pl(get_string(1473));
		if (yn() == 0) {
			// Simply send them a new RIPEXT.MNU and NAME.MNU file
			sprintf(f, "ripext.mn%c", menutype);
			numfiles = todownload(files, 0, f, sysinfo.ripdir);
			numfiles = todownload(files, numfiles, "name.mnu", sysinfo.ripdir);
			if (numfiles) {
			  pl(get_string(1612));
                          comstr("|905020000<>\r\r");           /* Block transfer */
			  menuxfer(sysinfo.ripdir, files, 2, &snd, &pct);
			  delay(1);
			  out_cport(4);
			  out_cport(26);
                          while (((ch = delaykey(5)) != 0) && (ch != 6) && (!hangup));
			}
			return 1;
		}
		break;
	  case 0:
		pl(get_string(1470));
		break;
	  default:
		break;
	}

	printmenu(351);
	do {
	  ch = toupper(delaykey(30));

	  if (ch == 0)     /* After 30 seconds, default to Yes */
		ch = 'Y';
        } while ((ch != 'Y') && (ch != 'N') && (!hangup));
	rip_cls();
	
	sprintf(f,"%siclose.%2.2s%c", sysinfo.ripdir, icon_extension, menutype);
	if (exist(f) && ch != 'Y') {
		pl(get_string(1442));
		pl(get_string(1443));
		pl(get_string(1444));

		if (yn()) {
		  iconflag = -1;
		  ch = 'Y';
		}
	}
	rip_cls();
	
	if (ch == 'Y') {
          //comstr("|905020000<>\r\r");           /* Block transfer */
	  pl(get_string(1613));
	  if (iconflag == 0) {
		sprintf(f, "*.mn%c", menutype);
		numfiles = todownload(files, 0, f, sysinfo.ripdir);
	  }
	  numfiles = todownload(files, numfiles, "name.mnu", sysinfo.ripdir);
	  numfiles = todownload(files, numfiles, "ripterm.key", sysinfo.ripdir);
	  numfiles = todownload(files, numfiles, "*.icn", sysinfo.ripdir);
	  numfiles = todownload(files, numfiles, "*.hic", sysinfo.ripdir);
	  sprintf(f, "*.%2.2s%c", icon_extension, menutype);
	  numfiles = todownload(files, numfiles, f, sysinfo.ripdir);

	  pl(get_string(1612));
          comstr("|905020000<>\r\r");           /* Block transfer */
	  menuxfer(sysinfo.ripdir, files, numfiles, &snd, &pct);
	  delay(1);
	  out_cport(4);
	  out_cport(26);
          while (((ch = delaykey(5)) != 0) && (ch != 6) && (!hangup));

	  if (iconflag)
		return 2;
	  else
		return 1;
	} else {
	  // Send them a new RIPEXT.MNU and NAME.MNU file
	  sprintf(f, "ripext.mn%c", menutype);
	  numfiles = todownload(files, 0, f, sysinfo.ripdir);
	  numfiles = todownload(files, numfiles, "name.mnu", sysinfo.ripdir);
	  if (numfiles) {
		pl(get_string(1612));
                comstr("|905020000<>\r\r");           /* Block transfer */
		menuxfer(sysinfo.ripdir, files, 2, &snd, &pct);
		delay(1);
		out_cport(4);
		out_cport(26);
                while (((ch = delaykey(5)) != 0) && (ch != 6) && (!hangup));
	  }

	  sprintf(f,"|1F000000ICLOSE.IC%c|#\r\r", menutype);
	  comstr(f);    /* Check for an icon file */
	  ch = delaykey(1);
	  if (ch == '1') 
		return 2;
	  else              /* User has no icons */
		return 0;
	}
}

/* Function: set the RIP menu extension to ext.
 */
void setmenu(char *ext)
{
	strcpy(ripext, ext);
}

/* Function: set the user name and phone info on his/her machine for auto-
   entry on the next call.
 */
void set_name(void)
{
  char s[161];
  pl(get_string(1446));
  pl(get_string(1447));

  sprintf(s,"!|1D00500%s_ID,30:?%s%s|#\r", sysinfo.ripbbsid, get_string(1448), thisuser.name);
  comstr(s);
  if (rip_ver >= 20000)
	sprintf(s,"!|1D00500PHONE,5:?%s%s|#\r", get_string(1449), &thisuser.phone[8]);
  else
	sprintf(s,"!|1D00700PHONE_LAST4,4:?%s%s|#\r", get_string(1449), &thisuser.phone[8]);
  comstr(s);
}

/* Function: allow the user to select a menu set, and then decide whether
   he/she wants to download it.
 */
void choosemenu(void)
{
  char ext[4], ch = 0, s[67], quitflag = 0;
  int xmit, oldstat;

  strcpy(ripext, "RIP");
  oldstat = thisuser.sysstatus;
  thisuser.sysstatus &= ~sysstatus_expert;
  do {
	printmenu(350);     /* Menu of choices */
	rip_coff();
	ch = delaykey(30);
	rip_con();

	switch (ch) {
	  case '0':
		rip_cls();
		end_rip();
#ifdef RIPDRIVE
		if (rd_on())
		  localrip_deactivate();
#endif
		thisuser.sysstatus = oldstat;
		return;
	  case 0:
		ch = M_DEFAULT; /* Set menu to default if no keypress */
	  default:
		rip_cls();
		 /* See if we have a menu set for this choice */
		sprintf(s,"%smenu0.mn%c", sysinfo.ripdir, ch);
		if (exist(s) == 0)
		  break;

		sprintf(ext, "MN%c", ch);

		rip_pause = pausecheck(1, ext);
		
		if ((xmit = transmit(ch)) == 1)
		  strcpy(ext,"REM");
		else if (xmit == 0)       /* No icons on user's machine */
		  sprintf(ext, "MN%c", M_DEFAULT);

		delay(2);
		out_cport(4);
		delay(8);
		rip_cls();
                while (inkey() && (!hangup));

		quitflag = -1;
		break;
	  }
	} while (quitflag == 0 && (hangup == 0));

	comstr("|*|w00122716|#\r\r");
	setmenu(ext);
	user_menus = ch;
	thisuser.sysstatus = oldstat;
	if (thisuser.sysstatus & sysstatus_expert)
	  pl (get_string(1614));
}

/* Check for file fn on user's machine.  If doesn't exist, return -1; if
   old, return 0, if current, return 1. */
int current_menu(char *fn)
{
	char f[67], dat[9], ch;
	int count;

        while (inkey() && (!hangup));
	sprintf(f,"|1F030000%s|#\r\r", fn);
	comstr(f);
	ch = delaykey(1);
	if (ch == '1') {
	  getkey();                                     /* Eat "." */
	  while (delaykey(1) != '.' && (hangup == 0));  /* Eat file size */

	  count = 0;
	  while ((ch = delaykey(1)) != '.' && (hangup == 0))  /* Save up the date */
		dat[count++] = ch;

		 /* Eat file time */
	  while ((ch = getkey()) != 10 && ch != 13 && (hangup == 0)); 

	  if (hangup)
		return 0;

	  if ( (dat[7] > sysinfo.ripmenudate[7] || dat[6] < '9') || (
	   (strncmp(dat, sysinfo.ripmenudate, 8) >= 0) && 
	   (dat[7] == sysinfo.ripmenudate[7]) ) )
		return 1;
	  else
		return 0;
	} else
	  return -1;
}

/* Function: main RIP initialization routine.  Check whether user has a
   RIPEXT.MNU on his/her machine, and whether it is a current version.  If
   not, allow the user to choose and download the latest menus, or just to
   use them without downloading.
 */
void rip_menus(void)
{
        char ext[4], fn[13];

	if (rip_subset) {
	  rip_pause = -1;
	  return;
	}

	/* Make sure ripmenudate has proper size, delimiters, etc. */
	sysinfo.ripmenudate[8] = 0;
	sysinfo.ripmenudate[2] = '/';
	sysinfo.ripmenudate[5] = '/';

	switch (current_menu("RIPEXT.MNU")) {
	  case 1:
		comstr("|1R00000000RIPEXT.MNU\r\r");   // Get user's menu extension
		user_menus = delaykey(1);
		sprintf(fn,"menu0.mn%c", user_menus);

		if (current_menu(fn) > 0) {
//          pl(get_string(1615));
		  pl(get_string(1471));
		  setmenu("REM");                         /* Use remote files */
		  rip_pause = pausecheck(0, NULL);
		} else {
//          pl(get_string(1616));
		  pl(get_string(1472));
		  sprintf(ext,"MN%c", user_menus);
		  setmenu(ext);
		  rip_pause = pausecheck(1, ext);
		}
		return;
	  case 0:
		nl();
		pl(get_string(1450));
		break;
	  case -1:
		nl();
		outstr(get_string(1451));
		outstr(syscfg.systemname);
		pl(get_string(1452));    
		if (!rip_subset) {
		  pl(get_string(1453));  
		  set_name();
		  comstr(" ");
		  pausescr();
		}
		break;
	}
	choosemenu();
}

/* Function: check pause-status indicator.  Certain menus which fill the
   whole screen will obscure lists and other data, such as the subs list,
   before the user can read them.  If a MENU334 file of > 10 bytes exists,
   the pause-before-menu flag is set.  Also, MENU334 is then used as a
   frame in which the next command's results are displayed.  (This allows
   you to keep data within a certain border.)
 */
int pausecheck(int localflag, char *ext)
{
  char s[67], ch;
  struct ffblk ffblock;

  if (localflag == 0) {
	sprintf(s, "|1F020000MENU334.MN%c|#\r\r", user_menus);
	comstr(s);    /* Check for pause flag */
	ch = delaykey(1);
	delaykey(1);
	if (ch == '1') {                    /* File exists, as should be */
	  ch = delaykey(1);
	  if (ch != '0')                    /* Not zero-length */
		if (delaykey(1) == '\r')        /* but is it < 10 bytes? */
		  ch = '0';
		else
                  while ((delaykey(1) != '\r') && (!hangup));  /* Eat up to CR */
	  return (ch != '0');
	}
  } else {
	sprintf(s,"%sMENU334.%s", sysinfo.ripdir, ext);
	if (findfirst(s, &ffblock, 0) == 0)
	  if (ffblock.ff_fsize > 0) {
		return -1;
	  }
  }
  return 0;
}

/* Function: turn off RIP support.
 */
void end_rip(void)
{
	rip = 0;
}

/* Function: restore the proper RIP message-base menu.  If the user is
   a co-sysop or above, menu 13 is used; otherwise menu 1 is used.
 */
void restore_msg_menu(void)
{
  if (rip_ver >= 20000)
	return;
  if (menu_on()) {
	if (lcs())
	  printmenu(13);
	else
	  printmenu(1);
  }
}

/* Function: create a text window for the message header info to be
   displayed in.  The font size here is different from the regular text.
 */
void msgheader(int startat)
{
  unsigned char t; 
  char s[80];

  if (rip_ver >= 20000)
	return;
  if (rip) {
	t = (formery > '9') ? formery - 'A' + 10 : formery - '0';
	t = t * 8 / 14 + startat;
	t = (t < 9) ? t + '0' : t + 'A' - 10;
	sprintf(s,"\n!|e|w050%c2D0M03|e|#\r\r", t);
	comstr(s);
  }
}

/* Function: create the proper window size for the message text to be 
   displayed in.  The title and header info uses a different font, so
   we must compensate for that here.
 */
void setmsgview(int smallby)
{
  struct localrip_screeninfo r;
  
  if (rip_ver >= 20000)
	return;
  if (rip == 0) 
	return;

#ifdef RIPDRIVE
  if (rd_on()) {
	localrip_textinfo(&r);    
	formery = r.w_top;
	formery = smally = formery + '0';
  } else
#endif
  {
        comstr("\n!|1""0000$TWY0$ |#\r\r");
	formery = delaykey(1);          /* Get text window upper limit */
	if (hangup == 0)
	smally = delaykey(1);
	if (smally == ' ')
	  smally = formery;
	else {
	  formery = smally = (formery - '0') * 10 + smally;
	  if (hangup == 0)
		delaykey(1);
	}
  }

  smally += smallby;
  if (smally > '9')
	smally += 'A' - '0' - 10;
  if (formery > '9')
	formery += 'A' - '0' - 10;
}

/* Function: record a series of bytes, terminated when end_submenu is set to -1
   by the input routine (on an ^I).

   Note that control codes are designated by a % followed by a letter, much
   like the way they are ^ followed by a letter in RIP.  (We cannot use ^
   since RIP already takes it.)
*/
void submenu(void)
{
	char ch, macro[246];    /* Max of 245 chars */
	char s[255];
	int i = 0;
	end_submenu = 0;           /* Begin recording */

	while (!end_submenu && i < 245 && (hangup == 0)) {/* Record keystrokes */
		ch = getkey();
		if (ch == '%') {    /* Special marker for control code */
		  ch = toupper(getkey());
		  switch (ch) {
			case 'C':           /* Code = ^ */
			  macro[i++] = '^';
			  break;
			case 'B':           /* Code = | */
			  macro[i++] = '|';
			  break;
			case 'N':           /* Code = newline */
			  macro[i++] = '\n';
			  break;
			case 'R':           /* Code = CR */
			  macro[i++] = '\r';
			  break;
			case 'E':           /* Code = Exclamation */
			  macro[i++] = '!';
			  break;
			case '(':           /* Code = < */
			  macro[i++] = '<';
			  break;
			case ')':           /* Code = > */
			  macro[i++] = '>';
			  break;
			case '[':           /* Code = Escape */
			  macro[i++] = 27;
			  break;
			default:
			  macro[i++] = ch;
			  break;
		  }
		} else if (ch != submenu_code)
		  macro[i++] = ch;
	}
	macro[i] = 0;
	end_submenu = -1;

	sprintf(s,"|%s\r\r",macro);
	comstr (s);             /* Play it back now */
}

/* Function: read in a hypertext filename and then display the hypertext
   file.
 */
void hypertext(void)
{
	char f[67], ch, s[67];
	int i = 0;

        while ((ch = getkey()) != '\n' && ch != '\r' && (hangup == 0)) {
	  if (strchr(":\\/", ch) == NULL)       /* Don't allow path specifiers */
		f[i++] = ch;
          if (i>=(sizeof(f)-3))
            return;
        }

	if (hangup)
	  return;

	f[i] = 0;           /* Terminate the string */

        if (rd_on() == 0)
	  printf("[Hypertext image %s]\r", f);

	if (rip_subset)
	  ch = '0';
        else if (using_modem) {
	  sprintf(s, "|1F000000%s|#\r\r", f); /* Check for file on user's machine */
	  comstr(s);
	  ch = delaykey(2);
        } else
          ch=0;
        tmp_disable_pause(1);
	if (ch == '1') {                /* Exists on user's machine */
	  sprintf(s, "|1R00000000%s\r\r", f);
	  comstr(s);
	} else {                        /* Try to print locally */
	  rip_show(f);
	}
	ripcode = 0;
        tmp_disable_pause(0);

	cleared = HYPCLEAR;     /* Signify hypertext image which may need cls */
}

/* Function:  show all of the chains on the BBS.  If a file called 
			  DOORLST.RIP exists in the RIPMENUS directory, displays that;
			  otherwise, displays a pick list with all of the doors.
*/
#ifdef OLD
void rip_show_chains(int *mapp, int *map, int start)
{
  int abort,i;
  char s[81];

  abort=0;
  sprintf(s,"%s%s",sysinfo.ripdir,"doorlst.rip");
  if (exist(s))
	rip_show("doorlst.rip");
  else {
    if (rd_on() == 0)
      printf("\n%s%s\n", syscfg.systemname, get_string(1454));
    sprintf(s,"|1""0000((*%s%s::\\", syscfg.systemname,get_string(1454));
    comstr(s);
    if (start) {
        if (rd_on() == 0)
          printf("%s\n", get_string(1474));
        sprintf(s,"%s\\", get_string(1475));
        comstr(s);
    }
    for (i=start; (i<MIN(*mapp, start+18)) && (!abort) && (!hangup); i++) {
      if (rd_on() == 0)
        printf("%d> %s\n",i+1, chains[map[i]].description);
      if ((i+1)*10 < *mapp)       /* Add any necessary CR */
        sprintf(s,"%d^M@%s,\\",i+1, chains[map[i]].description);
      else
        sprintf(s,"%d@%s,\\",i+1, chains[map[i]].description);
      comstr(s);
    }
    if (start+18 < *mapp) {
        if (rd_on() == 0)
          printf("%s\n", get_string(1476));
                sprintf(s,"%s\\", get_string(1477));
		comstr(s);
	}
	comstr(get_string(1465));
	remstr("\r\r");
	if (rd_on() == 0)
	  printf("%s\n\n",get_string(1464));      
  }
}
#else
char *pos[14][2] = {
 {"0D2P7B37", "4G2X"},
 {"0D3J7B41", "4G3R"},
 {"0D4D7B4V", "4G4L"},
 {"0D577B5P", "4G5F"},
 {"0D617B6J", "4G69"},
 {"0D6V7B7D", "4G73"},
 {"0D7P7B87", "4G7X"},
 {"7T2PER37", "BW2X"},
 {"7T3JER41", "BW3R"},
 {"7T4DER4V", "BW4L"},
 {"7T57ER5P", "BW5F"},
 {"7T61ER6J", "BW69"},
 {"7T6VER7D", "BW73"},
 {"7T7PER87", "BW7X"}
};

void rip_show_chains(int *mapp, int *map, int start)
{
  int abort,i;
  char s[81];

  abort=0;
  sprintf(s,"%s%s",sysinfo.ripdir,"doorlst.rip");
  if (exist(s))
	rip_show("doorlst.rip");
  else {
	printmenu(312);
	sprintf(s,"|c09|@FO2MPage %d/%d|c0E", start / 14 + 1, (*mapp+13) / 14);
	comr(s);
	for (i=start; (i<MIN(*mapp, start+14)) && (!abort) && (!hangup); i++) {
	  if (rd_on() == 0)
		printf("%d> %s\n",i+1, chains[map[i]].description);
	  if ((i+1)*10 < *mapp)     // Add any necessary CR
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d^M", pos[i-start][0], 
		  i+1, chains[map[i]].description, i+1);
	  else
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d", pos[i-start][0], 
		  i+1, chains[map[i]].description, i+1);
	  comr(s);
	  /*
	  read_user(chains_reg[map[i]].regby[0],&u);
	  sprintf(s,"|c0E|@%s%s", pos[i-start][1], 
		(chains_reg[map[i]].regby[0]) ? u.name : get_string(315));
	  comr(s);
	  */
	}
  }
  if (rip_ver >= 20000)         // Fix window in RIP 2.0
	comr("|w0013271610");
}
#endif

/* Function: display the list of GFILES.  If there's a file called  
   GFILELST.RIP, this will be shown as the list.  Otherwise a pick-list
   is built up */
#ifdef OLD
void rip_list_gfiles(gfilerec *g, int nf, int start)
{
  int i;
  char s[81];

  sprintf(s,"%s%s",sysinfo.ripdir,"gfilelst.rip");
  if (exist(s))
	rip_show("gfilelst.rip");
  else {
	if (rd_on() == 0)
	  printf("\n%s\n", get_string(1455));
        sprintf(s,"|1""0000((*%s::\\", get_string(1455));
	comstr(s);
	if (start) {
		if (rd_on() == 0)
		  printf("%s\n", get_string(1474));
		sprintf(s,"%s\\", get_string(1475));
		comstr(s);
	}
	for (i=start; (i<MIN(nf,start+18)) && (!hangup); i++) {
	  if (rd_on() == 0)
		printf("%d> %s\n\\", i+1, g[i].description);
	  sprintf(s,"%d@%s,\\",i+1, g[i].description);
	  comstr(s);
	}
	if (start+18 < nf) {
		if (rd_on() == 0)
		  printf("%s\n", get_string(1476));
		sprintf(s,"%s\\", get_string(1477));
		comstr(s);
	}
	comstr(get_string(1465));
	remstr("\r\r");
	if (rd_on() == 0)
	  printf("%s\n", get_string(1464)); 
  }
}
#else
void rip_list_gfiles(gfilerec *g, int nf, int start)
{
  int i;
  char s[81];

  sprintf(s,"%s%s",sysinfo.ripdir,"gfilelst.rip");
  if (exist(s))
	rip_show("gfilelst.rip");
  else {
	if (rd_on() == 0)
	  printf("\n%s\n", get_string(1455));
	printmenu(315);
	sprintf(s,"|c09|@FO2MPage %d/%d|c0E", start / 14 + 1, (nf+13) / 14);
	comr(s);
	for (i=start; (i<MIN(nf,start+14)) && (!hangup); i++) {
	  if (rd_on() == 0)
		printf("%d> %s\n", i+1, g[i].description);
	  if ((i+1)*10 < nf)     // Add any necessary CR
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d^M", pos[i-start][0], 
		  i+1, g[i].description, i+1);
	  else
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d", pos[i-start][0], 
		  i+1, g[i].description, i+1);
	  comr(s);
	}
	if (rd_on() == 0)
	  printf("%s\n", get_string(1464)); 
  }
  if (rip_ver >= 20000)         // Fix window in RIP 2.0
	comr("|w0013271610");
}
#endif

#ifdef OLD
/* Function: list the G-Files sections.  If a file called GSECLST.RIP is
   available, will use this; otherwise creates a pick list */
void rip_list_sec(int *map, int nmap, int start)
{
  int i;
  char s[81];

  sprintf(s,"%s%s",sysinfo.ripdir,"gseclst.rip");
  if (exist(s))
	rip_show("gseclst.rip");
  else {
	if (rd_on() == 0)
	  printf("\n%s\n", get_string(1456));
        sprintf(s,"|1""0000((*%s::\\", get_string(1456));
	comstr(s);
	if (start) {
		if (rd_on() == 0)
		  printf("%s\n", get_string(1474));
		sprintf(s, "%s\\",get_string(1475));
		comstr(s);
	}
	for (i=start; (i<MIN(nmap,start+18)) && (!hangup); i++) {
	  if (rd_on() == 0)
		printf("%d> %s\n", i+1, gfilesec[map[i]].name);
	  sprintf(s,"%d@%s,\\",i+1, gfilesec[map[i]].name);
	  comstr(s);
	}
	if (cs()) {
	  if (rd_on() == 0) {
		printf("%s\n", get_string(50));
		printf("%s\n", get_string(51));
	  }
	  sprintf(s,"%s\\",get_string(1466));
	  comstr(s);
	}
	if (start+18 < nmap) {
		if (rd_on() == 0)
		  printf("%s\n", get_string(1476));
		sprintf(s, "%s\\",get_string(1477));
		comstr(s);
	}
	comstr(get_string(1465));
	remstr("\r\r");
	if (rd_on() == 0)
	  printf("%s\n", get_string(1464));       
  }
}
#else
void rip_list_sec(int *map, int nmap, int start)
{
  int i;
  char s[81];

  sprintf(s,"%s%s",sysinfo.ripdir,"gseclst.rip");
  if (exist(s))
	rip_show("gseclst.rip");
  else {
	printmenu(317);
	sprintf(s,"|c09|@FO2MPage %d/%d|c0E", start / 14 + 1, (nmap+13) / 14);
	comr(s);
	if (rd_on() == 0)
	  printf("\n%s\n", get_string(1456));
	for (i=start; (i<MIN(nmap,start+14)) && (!hangup); i++) {
	  if (rd_on() == 0)
		printf("%d> %s\n", i+1, gfilesec[map[i]].name);
	  if ((i+1)*10 < nmap)     // Add any necessary CR
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d^M", pos[i-start][0], 
		  i+1, gfilesec[map[i]].name, i+1);
	  else
		sprintf(s,"|1U%s0000<>[%d] %-36.36s<>%d", pos[i-start][0], 
		  i+1, gfilesec[map[i]].name, i+1);
	  comr(s);
	}
	if (rd_on() == 0)
	  printf("%s\n", get_string(1464));       
  }
  if (rip_ver >= 20000)         // Fix window in RIP 2.0
	comr("|w0013271610");
}
#endif

/* Function: display the timebank with a RIP screen, and update the screen
   to keep it current */
void rip_timebank(void)
{
  char s[80],c;
  int i,done=0;
  double nsln;

  do {
	printmenu(366);
	sprintf(s,"|@7N54%d\r\r",thisuser.banktime);
	comstr(s);
	c=onek("QDW");
	switch(c) {
	  case 'D':
		rip_con();
		input(s,3);
		rip_coff();
		i=atoi(s);
		if (i>0) {
		  nsln=nsl();
		  if ((i+thisuser.banktime)>syscfg.sl[thisuser.sl].time_per_logon)
			i=syscfg.sl[thisuser.sl].time_per_logon-thisuser.banktime;
		  if (i>(nsln/60.0))
			i=(nsln/60.0);
		  thisuser.banktime+=i;
		  thisuser.extratime-=i*60.0;
		  tleft(0);
		}
		break;
	  case 'W':
		if (!thisuser.banktime)
		  break;
		rip_con();
		input(s,3);
		rip_coff();
		i=atoi(s);
		if (i>0) {
		  nsln=nsl();
		  if (i>thisuser.banktime)
			i=thisuser.banktime;
		  thisuser.banktime-=i;
		  thisuser.extratime+=(i*60.0);
		  tleft(0);
		}
		break;
	  case 'Q':
		done=1;
		break;
	}
  } while (!done && !hangup);
  cleared = NEEDCLEAR;
}

/* Function: display Your Info in a RIP dialog screen */
void rip_yourinfo(void)
{
  printmenu(326);
  tmp_disable_pause(1);
  rip_show("MENU326.LCL");          // Show the local overlay with the data
  getkey();
  if (hangup)
	  return;
  tmp_disable_pause(0);
  cleared = NEEDCLEAR;
}

/* Function: display extended file info in a RIP dialog box */
int rip_printfileinfo(uploadsrec *u, int dn)
{
  char s[81];
  double d;
  int abort;

  d=XFER_TIME(u->numbytes);

  rip_saveall();
  printmenu(364);
  
  if (rd_on() == 0)
	printf("%s",get_string(746)); 
  comstr("|Y00000100|@5S2X\\");
#ifdef RIPDRIVE
  if (rd_on())
	comr(stripfn(u->filename));
  else
#endif
	pl(stripfn(u->filename));

  if (rd_on() == 0)
	printf("%s",get_string(748)); 
  comstr("|Y00000100|@CE39\\");
#ifdef RIPDRIVE
  if (rd_on()) {
	sprintf(s,"%ldk (%s)\r\n", bytes_to_k(u->numbytes), ctim(u->numbytes));
	comstr(s);
  } else
#endif
	npr("%ldk (%s)\r\n", bytes_to_k(u->numbytes), ctim(u->numbytes));

  if (rd_on() == 0)
	printf("%s",get_string(749)); 
  comstr("|@AO49\\");
#ifdef RIPDRIVE
  if (rd_on())
	comr(ctim(d));
  else
#endif
	pl(ctim(d));

  if (rd_on() == 0)
	printf("%s",get_string(750)); 
  comstr("|@6849\\");
#ifdef RIPDRIVE
  if (rd_on())
	comr(u->date);
  else
#endif
	pl(u->date);

  if (rd_on() == 0)
	printf("%s",get_string(751)); 
  comstr("|@5C3X\\");
#ifdef RIPDRIVE
  if (rd_on())
	comr(u->upby);
  else
#endif
	pl(u->upby);

  if (rd_on() == 0)
	printf("%s",get_string(752)); 
  comstr("|@CW3X\\");
#ifdef RIPDRIVE
  if (rd_on()) {
	sprintf(s, "%d", u->numdloads);
	comr(s);
  } else
#endif
	pln(u->numdloads);

  if (rd_on() == 0)
	sprintf(s,"%s%s",directories[dn].path,u->filename);
  comstr("|@5S39\\");
  if (!exist(s)) {
	comr(get_string(1457));
	if (rd_on() == 0)
	  puts(get_string(754));
  } else
	comr(get_string(1458));  

  if (rd_on() == 0)
	printf("%s",get_string(747)); 
  comstr("|Y02000400|@2X51\\");
#ifdef RIPDRIVE
  if (rd_on())
	comr(u->description);
  else
#endif
	pl(u->description);

  abort=0;
  if (u->mask & mask_extended) {
        comr("|1""0000$SAVE1$");
	if (rd_on() == 0)
	  puts(get_string(753));
	ex_line = 0;
	rip_print_extended(u->filename,&abort,sysinfo.max_extend_lines);
  }

  if (!exist(s))
	return(1);

  if (nsl()>=d)
	return(1);
  else
	return(0);
}

char *ex_p[10] = { "5N\\", "5Y\\", "69\\", "6K\\", "6V\\", "76\\", "7H\\", 
	 "7S\\", "83\\", "8E\\"};

/* Function: print the extended description in the dialog box, in the
   correct places */
void rip_print_extended(char *fn, int *abort, unsigned char numlist)
{
  char *ss;
  int next=0;
  unsigned char numl=0;
  int cpos=0;
  char ch;
  char s2[4];

  strcpy(s2,"x\\");
  comr("|1""0000$RESTORE1$");
  ss=read_extended_description(fn);
  if (ss) {
	while ((numl < ex_line)) {
	  while ((ss[cpos] != 10) && ss[cpos])
		cpos++;
	  if (ss[cpos] == 0)
		break;
	  else
		numl++;
	  cpos++;  
	}
	comstr("|Y02000400|@2Z5C\\");
	while ((ss[cpos] != 0) && (!(*abort)) && (numl < MIN(ex_line+10, 
         numlist)) && (!hangup)) {
	  ch=ss[cpos++];
	  s2[0] = ch;
	  if (cpos > strlen(ss))
		break;
	  if (rd_on() == 0)
		outstr(s2);
	  comstr(s2);
	  if (ch==10) {
		++numl;
		comr("");
		comstr("|@2Y\\");
		comstr(ex_p[numl-1-ex_line]);
	  } else
		if ((ch!=13) && (WhereX()>=78)) {
		  osan("\r",abort,&next);
		  ch=10;
		}
	}
	if (WhereX())              
	  comstr("\r");
	farfree(ss);
  }
  comstr("\r");
  if (ss[cpos]) {
	ex_line += 10;
  } else
	ex_line = 0;
}

void rip_detect(void)
{
	  strcpy(ripext, "RIP");
	  rip = rip_check();
	  if (rip) {
		rip_pause = 0;
		rip_popup = -1;
		ripcode = rip;
	  }
}

/* Interprets a code from the Ctrl-P Ctrl-P sequence, translating it
   into the correct string. */
char *interpret(char c)
{
  static char s[80];

  s[0]=0;
  if (g_flags & g_flag_disable_mci)
    return("");

  switch (c) {
	case '@':                   // Dir name
	  strcpy(s, directories[udir[curdir].subnum].name);
	  break;
	case '~':                   // Total mails/feedbacks sent
	  sprintf(s, "%u", thisuser.emailsent + thisuser.feedbacksent+thisuser.emailnet);
	  break;
	case '/':                   // Today's date
	  strcpy(s, date());
	  break;
	case '%':                   // Time left today
	  //sprintf(s, "%f", thisuser.extratime);
	  sprintf(s, "%d", (int)(nsl()/60));
	  break;
	case '#':                   // User's number
	  sprintf(s, "%u", usernum);
	  break;
	case '$':                   // File points
	  sprintf(s, "%lu", thisuser.filepoints);
	  break;
	case '*':                   // User reg num
	  sprintf(s, "%lu", thisuser.wwiv_regnum);
	  break;
	case '-':                   // Aggravation points
	  sprintf(s, "%u", thisuser.ass_pts);
	  break;
	case ':':                   // Sub number
	  strcpy(s, usub[cursub].keys);
	  break;
	case ';':                   // Directory number
	  strcpy(s, udir[curdir].keys);
	  break;
	case '!':                   // Built-in pause
	  pausescr();
	  break;
	case '&':                   // RIP/ANSI/ASCII mode
	  if (rip)
		strcpy(s, "RIPspeed");
	  else if (thisuser.sysstatus & sysstatus_ansi)
		strcpy(s, "ANSI");
	  else
		strcpy(s, "ASCII");
	  break;
	case 'A':                   // User's age
	  sprintf(s, "%d", thisuser.age);
	  break;
	case 'a':                   // User's language
	  strcpy(s, cur_lang_name);
	  break;
	case 'B':                   // User's birthday
	  sprintf(s, "%d/%d/%d", thisuser.month, thisuser.day, thisuser.year);
	  break;
	case 'b':                   // Minutes in bank
	  sprintf(s, "%u", thisuser.banktime);
	  break;
	case 'C':                   // User's city
	  strcpy(s, thisuser.city);
	  break;
	case 'c':                   // User's country
	  strcpy(s, thisuser.country);
	  break;
	case 'D':                   // Files downloaded
	  sprintf(s, "%u", thisuser.downloaded);
	  break;
	case 'd':                   // User's DSL
	  sprintf(s, "%d", thisuser.dsl);
	  break;
	case 'E':                   // E-mails sent
	  sprintf(s, "%u", thisuser.emailsent);
	  break;
	case 'e':                   // Net E-mails sent
	  sprintf(s, "%u", thisuser.emailnet);
	  break;
	case 'F':
	  sprintf(s, "%u", thisuser.feedbacksent);
	  break;
	case 'f':                   // First time user called
	  strcpy(s, thisuser.firston);
	  break;
	case 'G':                   // MessaGes read
	  sprintf(s, "%lu", thisuser.msgread);
	  break;
	case 'g':                   // Gold
	  sprintf(s, "%f", thisuser.gold);
	  break;
	case 'I':                   // User's call sIgn
	  strcpy(s, thisuser.callsign);
	  break;
	case 'i':                   // Illegal log-ons
	  sprintf(s, "%u", thisuser.illegal);
	  break;
	case 'J':                   // Message conference
	  strcpy(s, subconfs[uconfsub[curconfsub].confnum].name);
	  break;
	case 'j':                   // Transfer conference
	  strcpy(s, dirconfs[uconfdir[curconfdir].confnum].name);
	  break;
	case 'K':                   // Kb uploaded
	  sprintf(s, "%lu", thisuser.uk);
	  break;
	case 'k':                   // Kb downloaded
	  sprintf(s, "%lu", thisuser.dk);
	  break;
	case 'L':                   // Last call
	  strcpy(s, thisuser.laston);
	  break;
	case 'l':                   // Number of logons
	  sprintf(s, "%u", thisuser.logons);
	  break;
	case 'M':                   // Mail waiting
	  sprintf(s, "%d", thisuser.waiting);
	  break;
	case 'm':                   // Messages posted
	  sprintf(s, "%u", thisuser.msgpost);
	  break;
	case 'N':                   // User's name
	  strcpy(s, thisuser.name);
	  break;
	case 'n':                   // Sysop's note
	  strcpy(s, thisuser.note);
	  break;
	case 'O':                   // Times on today
	  sprintf(s, "%d", thisuser.ontoday);
	  break;
	case 'o':                   // Time on today
	  //sprintf(s, "%f", thisuser.timeontoday);
	  sprintf(s,"%ld",(long)((thisuser.timeon+timer()-timeon)/60.0));
	  break;
	case 'P':                   // BBS phone
	  strcpy(s, syscfg.systemphone);
	  break;
	case 'p':                   // User's phone
	  strcpy(s, thisuser.dataphone);
	  break;
	case 'R':                   // User's real name
	  strcpy(s, thisuser.realname);
	  break;
	case 'r':                   // Last baud rate
	  sprintf(s, "%d", thisuser.lastrate);
	  break;
	case 'S':                   // User's SL
	  sprintf(s, "%d", thisuser.sl);
	  break;
	case 's':                   // User's street address
	  strcpy(s, thisuser.street);
	  break;
	case 'T':                   // User's sTate
	  strcpy(s, thisuser.state);
	  break;
	case 't':                   // Current time
	  strcpy(s, times());
	  break;
	case 'U':                   // Files uploaded
	  sprintf(s, "%u", thisuser.uploaded);
	  break;
	case 'u':                   // Current sub
	  strcpy(s, subboards[usub[cursub].subnum].name);
	  break;
	case 'W':                   // Total # of messages in sub
	  sprintf(s, "%d", nummsgs);
	  break;
	case 'X':                   // User's sex
	  sprintf(s, "%c", thisuser.sex);
	  break;
	case 'Y':                   // Your BBS name
	  strcpy(s, syscfg.systemname);
	  break;
	case 'y':                   // Computer type
	  strcpy(s, ctypes[thisuser.comp_type]);
	  break;
	case 'Z':                   // User's zip code
	  strcpy(s, thisuser.zipcode);
	  break;
	default:
	  return "";
  }

  return s;
}

int todownload(char array[][13], int start, char *fspec, char *fpath)
{
  int f, qflag, count, pos=0;
  struct ffblk ffb;
  struct ftime tim;
  char fn[67];
  char s[80], s2[80], ch;

  count = start;
  strcpy(fn, fpath);
  strcat(fn, fspec);
  qflag = findfirst(fn, &ffb, FA_ARCH);

  npr("%s %s...\r\n", get_string(1637), fspec);
  while ((qflag==0) && (!hangup)) {
	strcpy(fn, fpath);
	strcat(fn, ffb.ff_name);
	f=sh_open1(fn,O_RDONLY | O_BINARY);
	getftime(f,&tim);
	sh_close(f);

	strcpy(fn, ffb.ff_name);
	if (strncmp(fn, "RIPEXT.MN", 9) == 0)
	  fn[strlen(fn) - 1] = 'U';

	/* Adjust RIPscrip 2.0 BMP filenames so they will work */
	if (strncmp(&fn[strlen(fn)-3], "BM", 2) == 0) {
	  fn[strlen(fn)-3] = 'I';
	  fn[strlen(fn)-2] = 'C';
	}

	sprintf(s2,"|1F030000%s\r\n",fn);
	comstr(s2);
	pos = 0;
	ch = getkey();
	while ((ch != 13) && (!hangup)) {
	  s[pos++] = ch;
	  ch = getkey();
	}
	s[pos] = 0;
	sprintf(s2,"1.%ld.%02d/%02d/%02d.%02d:%02d:%02d", ffb.ff_fsize,
	  tim.ft_month, tim.ft_day, tim.ft_year+80, tim.ft_hour,
	  tim.ft_min, (tim.ft_tsec*2));
	if (strcmp(s,s2))    // Files match
	  strncpy(array[count++], ffb.ff_name, 13);
	qflag = findnext(&ffb);
  }
  return count;
}

void menuxfer(char *fpath, char files[][13], int num, int *sent, double *percent)
{
  char bn,ch,s[81];
  int f,i,abort,terr,xx1,yy1;
  char fn[67];
  long cp,len;
  int ucrc;
  double tpb;

  int count = 0;

  abort=0;

  /* Trim the filename off, leaving the path intact */
  while ((count < num) && (!abort) && (!hangup)) {
        checkhangup();
	strcpy(fn, fpath);
	strcat(fn, files[count]);
	ucrc = 1;
	*percent = 0;
	*sent = 0;
	cp=0L;
	bn=1;
	terr=0;

	f=sh_open1(fn,O_RDONLY | O_BINARY);
	len=filelength(f);
	if (!len)
	  len=1;
	tpb=(12.656) / ((double) (modem_speed));

	xx1=WhereX();
	yy1=WhereY();
	movecsr(52,0);
	outs(get_stringx(1,67));
	movecsr(52,1);
	outs(get_stringx(1,68));
	movecsr(52,2);
	outs(get_stringx(1,69));
	movecsr(52,3);
	outs(get_stringx(1,70));
	movecsr(52,4);
	outs(get_stringx(1,71));
	movecsr(52,5);
	outs(get_stringx(1,72));
	movecsr(52,6);
	outs(get_stringx(1,73));
	movecsr(65,0);

	strcpy(fn, files[count]);
	/* Adjust the menu extension and Hot icon file names */
	/* Do this by looking at three of the last four characters */
	/* And then setting the last character based on a match */
	if (strncmp(fn, "RIPEXT.MN", 9) == 0)
	  fn[strlen(fn) - 1] = 'U';

	/* Adjust RIPscrip 2.0 BMP filenames so they will work */
	if (strncmp(&fn[strlen(fn)-3], "BM", 2) == 0) {
	  fn[strlen(fn)-3] = 'I';
	  fn[strlen(fn)-2] = 'C';
	}

	outs(stripfn(fn));

	sprintf(s,"%ld - %ldk", (len+127)/128, bytes_to_k(len));
	movecsr(65,2);
	outs(s);

	if (!okstart(&ucrc,&abort))
	  abort=1;
	if ((!abort) && (!hangup)) {
	  ch=send_b(f, len, 5, 0, &ucrc,fn,&terr,&abort);
	  if (ch==24)
		abort=1;
	  if (ch==21) {
		send_b(f,0L,3,0,&ucrc,fn,&terr,&abort);
		abort=1;
	  }
	}
	while ((!hangup) && (!abort) && (cp<len)) {
          checkhangup();
	  i=1;
	  if ((len-cp)<768L)
		i=0;
	  sprintf(s,"%ld - %ldk",
		cp/128+1,
		cp/1024+1);
	  movecsr(65,3);
	  outs(s);
	  movecsr(65,1);
	  outs(ctim(((double)(len-cp))*tpb));
	  movecsr(69,4);
	  outs("0");

	  ch=send_b(f,cp,i,bn,&ucrc,fn,&terr,&abort);
	  if (ch==24)
		abort=1;
	  else
		if (ch==21) {
		  wait1(18);
		  dump();
		  send_b(f,0L,3,0,&ucrc,fn,&terr,&abort);
		  abort=1;
		} else {
		  ++bn;
		  if (i)
			cp+=1024;
		  else
			cp+=128;
		}
	}
	if ((!hangup) && (!abort))
	  send_b(f,0L,2,0,&ucrc,fn,&terr,&abort);
	if (!abort) {
	  *sent=1;
	  *percent=1.0;
	} else {
	  *sent=0;
	  if (i)
		cp+=1024;
	  else
		cp+=128;
	  if (cp>=len)
		*percent=1.0;
	  else {
		if (i)
		  cp-=1024;
		else
		  cp-=128;
		*percent=((double)(cp))/((double)(len));
	  }
	}
	sh_close(f);
	movecsr(xx1,yy1);
	count++;
  }
}

int rip_file(char *fn, ripmsgrec *m, int maxary)
{
  int i,i1,i2,cnt;
  char *buf,s[81];
  long l,l1;

  for (i=0; i<maxary; i++) {
	m[i].stored_as=-1L;
	m[i].storage_type=255;
	m[i].menu_num=0;
  }
  sprintf(s,"%s%s",languagedir,fn);
  i=sh_open1(s,O_RDONLY | O_BINARY);
  if (i<0) {
	return(1);
  }
  l=filelength(i);
  buf=(char *) malloca(l);
  sh_lseek(i,0L,SEEK_SET);
  if (buf==NULL) {
	printf(get_string(1617));
	end_bbs(noklevel);
  }
  sh_read(i,(void *) buf,l);
  sh_close(i);
  i1=cnt=0;
  for (l1=0; l1<l; l1++) {
	if ((buf[l1]=='`') && ((l1==0) || (buf[l1-1]==10))) {
	  i1=1;
	  i2=0;
	} else {
	  if (i1) {
		if ((buf[l1]>='0') && (buf[l1]<='9')) {
		  i2*=10;
		  i2+=(buf[l1])-'0';
		} else {
		  while ((l1<l) && (buf[l1]!=10))
			++l1;
		  ++l1;
		  if ((i2>=0) && (cnt<maxary)) {
			m[cnt].stored_as=l1;
			m[cnt++].menu_num=i2;
		  }
		  i1=0;
		}
	  }
	}
  }
  bbsfree((void *) buf);

  return(0);
}

void rd_disable(void)
{
  localrip_deactivate();
  defscreenbottom=(int) peekb(0x0000,0x0484);
  if (defscreenbottom<24)
	defscreenbottom=24;
  if (defscreenbottom>63)
	defscreenbottom=24;
  if ((defscreenbottom!=42) && (defscreenbottom!=49))
	defscreenbottom=24;
  screenlinest=defscreenbottom+1;
  screenbottom=defscreenbottom;
  screenlen=160*(screenbottom+1);
}

void rip_list_qscan(int start)
{
  int i,abort;
  char s[81];
  abort=0;
  printmenu(323);
  for (i=0;(start<num_subs)&& (i<thisuser.screenlines-5) &&
   (usub[start].subnum!=-1)&&(!abort) && (!hangup);start++,i++) {
        sprintf(s,"2%c[1D[B",
	(qsc_q[usub[start].subnum/32]&
	(1L<<(usub[start].subnum%32)))?'*':' ');
	outstr(s);
  } 
}

void rip_config_qscan(void)
{
  char s[200], *k;
  int i,done,start=0;
  done=0;
  helpl=7;
  do {
	printmenu(323);
	sprintf(s,"|c09|@FO2MPage %d/%d|c0E", start / 14 + 1, (num_subs+13) / 14);
	comr(s);
        for (i=start; (i < MIN(num_subs, start+14)) && (usub[start].subnum != -1) &&
         (!hangup); i++) {
	  if ((i+1)*10 < num_subs)     // Add any necessary CR
		sprintf(s,"|1U%s0000<>%c[%s] %-35.35s<>%s^M", pos[i-start][0], 
		 (qsc_q[usub[i].subnum / 32] & (1L << (usub[i].subnum % 32))) ? 
		 '*': ' ', usub[i].keys, subboards[usub[i].subnum].name, 
		 usub[i].keys);
	  else
		sprintf(s,"|1U%s0000<>%c[%s] %-35.35s<>%s", pos[i-start][0], 
		 (qsc_q[usub[i].subnum / 32] & (1L << (usub[i].subnum % 32))) ? 
		 '*': ' ', usub[i].keys, subboards[usub[i].subnum].name, 
		 usub[i].keys);
	  comr(s);
	} 

	if (rip_ver >= 20000)         // Fix window in RIP 2.0
	  comr("|w0013271610");

	nl();
	outstr(get_string(1618));
	k=mmkey(0);
	switch (k[0]) {
	  case 'Q':
		rip_cls();
		printmenu(320);
		return;
	  case 'N':
		if (start+14 < num_subs)
		  start += 14;
		break;
	  case 'P':
		if (start >= 14)
		  start -= 14;
		break;
	  default:
                for (i=0; (i < num_subs) && (usub[i].subnum != -1) && (!hangup); i++)
		  if (strcmp(usub[i].keys, k) == 0)
			qsc_q[usub[i].subnum/32] ^= (1L << (usub[i].subnum % 32));
		break;
	}
  } while ((!done) && (!hangup));
}

