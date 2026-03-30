/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/

#include "vars.h"

#pragma hdrstop

#include <dir.h>

/****************************************************************************/

/*
 * Attempts to allocate nbytes (+1) bytes on the heap, returns ptr to memory
 * if successful.
 */

void far *malloca(unsigned long nbytes)
{
  void *buf;
  char s[160];

  buf=bbsmalloc(nbytes+1);
  if (buf==NULL) {
    nl();
    npr(get_stringx(1,31),nbytes);
    nl();
    sprintf(s,get_stringx(1,32),nbytes);
    sysoplog(s);
  }
  return(buf);
}

/****************************************************************************/

static void stuff_in_num(char *s, char *fmt, int num)
{
  char temp[81];

  sprintf(temp, fmt, num);
  strcat(s,temp);
}

/*
 * Replacable parameters:
 *
 *  Param     Description                       Example
 *  ---------------------------------------------------------------------
 *   %%       A single '%'                      "%"
 *   %1-%5    Specified passed-in parameter
 *   %B       Com port base address, in hex     "3f8"
 *   %I       Com port irq, in decimal          "4"
 *   %M       Modem baud rate                   "14400"
 *   %S       Com port baud rate                "38400"
 *   %P       Com port number                   "1"
 *   %N       Instance number                   "1"
 *   %T       Time remaining (min)              "30"
 *   %C       chain.txt full pathname           "c:\wwiv\chain.txt"
 *   %D       doorinfo full pathname            "c:\wwiv\dorinfo1.def"
 *   %O       pcboard full pathname             "c:\wwiv\pcboard.sys"
 *   %A       callinfo full pathname            "c:\wwiv\callinfo.bbs"
 *   %R       door full pathname                "c:\wwiv\door.sys"
 */
void stuff_in(char *s, char *s1, char *f1, char *f2, char *f3, char *f4, char *f5)
{
  int r=0,w=0;
  double d;

  while (s1[r]!=0) {
    if (s1[r]=='%') {
      ++r;
      s[w]=0;
      switch(upcase(s1[r])) {
      /* used: %12345ABCDIMNOPRST */

      /* fixed strings */
        case '%': strcat(s,"%s");                                       break;

      /* replacable parameters */
        case '1': strcat(s,f1);                                         break;
        case '2': strcat(s,f2);                                         break;
        case '3': strcat(s,f3);                                         break;
        case '4': strcat(s,f4);                                         break;
        case '5': strcat(s,f5);                                         break;

      /* call-specific numbers */
        case 'B': stuff_in_num(s, "%x", base);                          break;
        case 'I': stuff_in_num(s, "%u", async_irq);                     break;
        case 'M': stuff_in_num(s, "%u", modem_speed);                   break;
        case 'P': stuff_in_num(s, "%u", incom?syscfgovr.primaryport:0); break;
        case 'N': stuff_in_num(s, "%u", instance);                      break;
        case 'S':
          if ((com_speed==1) || (com_speed==49664))
            strcat(s,"115200");
          else
            stuff_in_num(s, "%u", com_speed);
          break;

        case 'T':
          d=nsl();
          if (d<0)
            d += 24.0*3600.0;
          stuff_in_num(s, "%u", (int)d/60);
          break;

      /* chain.txt type filenames */
        case 'C': create_filename(CHAINFILE_CHAIN, s+w);                break;
        case 'D': create_filename(CHAINFILE_DORINFO, s+w);              break;
        case 'O': create_filename(CHAINFILE_PCBOARD, s+w);              break;
        case 'A': create_filename(CHAINFILE_CALLINFO, s+w);             break;
        case 'R': create_filename(CHAINFILE_DOOR, s+w);                 break;

      }
      w=strlen(s);
      r++;
    } else
      s[w++]=s1[r++];
  }
  s[w]=0;
}

/****************************************************************************/

void copy_line(char *s, char *b, long *ptr, long len)
{
  int i;
  long l;

  if (*ptr>=len) {
    s[0]=0;
    return;
  }
  l=*ptr;
  i=0;
  while ((b[l]!='\r') && (b[l]!='\n') && (l<len)) {
    s[i++]=b[l++];
  }
  s[i]=0;
  if ((b[l]=='\r') && (l<len))
    ++l;
  if ((b[l]=='\n') && (l<len))
    ++l;
  *ptr=l;
}

/****************************************************************************/

char *mmkey(int dl)
{
  static unsigned char cmd1[10],cmd2[81],ch;
  int p,cp;

  do {
    do {
      ch=getkey();
    } while ((((ch<' ') && (ch!=13)) || (ch>126)) && (hangup==0));
    ch=upcase(ch);
    outchr(ch);
    if (ch==13)
      cmd1[0]=0;
    else
      cmd1[0]=ch;
    cmd1[1]=0;
    p=0;
    switch(dl) {

    case 1:
      if (strchr(dtc,ch)!=NULL)
        p=2;
      else if (strchr(dcd,ch)!=NULL)
        p=1;
      break;
    case 2:
      if (strchr(odc,ch)!=NULL)
        p=1;
      break;
    case 0:
      if (strchr(tc,ch)!=NULL)
        p=2;
      else if (strchr(dc,ch)!=NULL)
        p=1;
      break;
    }
    if (p) {
      cp=1;
      do {
        do {
          ch=getkey();
        } while ((((ch<' ') && (ch!=13) && (ch!=8)) || (ch>126)) && (hangup==0));
        ch=upcase(ch);
        if (ch==13) {
          nl();
          return(cmd1);
        } else {
          if (ch==8) {
            backspace();
            cmd1[--cp]=0;
          } else {
            cmd1[cp++]=ch;
            cmd1[cp]=0;
            outchr(ch);
            if ((ch=='/') && (cmd1[0]=='/')) {
              input(cmd2,50);
              return(cmd2);
            } else if (cp==p+1) {
              nl();
              return(cmd1);
            }
          }
        }
      } while (cp);
    } else {
      nl();
      return(cmd1);
    }
  } while (hangup==0);
  cmd1[0]=0;
  return(cmd1);
}

/****************************************************************************/

int inli2(char *s, char *rollover, int maxlen, int crend, int wb)
{
  int cp,i,i1,done,cm,begx;
  char s1[255],s2[255],*ss;
  unsigned char ch;

  cm=chatting;

  begx=WhereX();
  if (rollover[0]!=0) {
    ss=s2;
    for (i=0; rollover[i]; i++) {
      if ((rollover[i]==3) || (rollover[i]==15))
        *ss++='P'-'@';
      else
        *ss++=rollover[i];

      /* Add a second ^P if we are working on RIP codes */
      if (rollover[i]==15 && rollover[i+1]!=15 && rollover[i-1] !=15)
        *ss++='P'-'@';
    }
    *ss=0;
    if (charbufferpointer) {
      strcpy(s1,s2);
      strcat(s1,&charbuffer[charbufferpointer]);
      strcpy(&charbuffer[1],s1);
      charbufferpointer=1;
    } else {
      strcpy(&charbuffer[1],s2);
      charbufferpointer=1;
    }
    rollover[0]=0;
  }
  cp=0;
  done=0;
  do {
    ch=getkey();
    if (two_color)
      if (lastcon)
	ansic(1);
      else
	ansic(0);
    if (cm)
      if (chatting==0)
        ch=13;
    if ((ch>=32)) {
      if ((WhereX()<(thisuser.screenchars-1)) && (cp<maxlen)) {
        s[cp++]=ch;
        outchr(ch);
        if (WhereX()==(thisuser.screenchars-1))
          done=1;
      } else {
        if (WhereX()>=(thisuser.screenchars-1))
          done=1;
      }
    } else
        switch(ch) {
	  case 7:
	    if ((chatting) && (outcom))
	      outcomch(7);
	    break;
          case 13: /* C/R */
            s[cp]=0;
            done=1;
            break;
          case 8:  /* Backspace */
            if (cp) {
              if (s[cp-2]==3) {
                cp-=2;
                ansic(0);
              } else if (s[cp-2]==15) {
                for (i=strlen((char *)interpret(s[cp-1])); i>0; i--)
                  backspace();
                cp-=2;
                if (s[cp-1]==15)
                  cp--;
              } else {
                if (s[cp-1]==8) {
                  cp--;
                  outchr(32);
                } else {
                  cp--;
                  backspace();
                }
              }
            } else
              if (wb) {
                if (okansi()) {
                  if (curdloads==1)
                    outstr("\r\x1b[K");
                  outstr("\x1b[1A");
                } else
                  pl(get_string(1485));
                return(1);
              }
            break;
          case 24: /* Ctrl-X */
            while (WhereX()>begx) {
              backspace();
              cp=0;
            }
            ansic(0);
            break;
          case 23: /* Ctrl-W */
            if (cp) {
              do {
                if (s[cp-2]==3) {
                  cp-=2;
                  ansic(0);
                } else
                  if (s[cp-1]==8) {
                    cp--;
                    outchr(32);
                  } else {
                    cp--;
                    backspace();
                  }
              } while ((cp) && (s[cp-1]!=32) && (s[cp-1]!=8));
            }
            break;
          case 14: /* Ctrl-N */
            if ((WhereX()) && (cp<maxlen)) {
              outchr(8);
              s[cp++]=8;
            }
            break;
          case 16: /* Ctrl-P */
            if (cp<maxlen-1) {
              ch=getkey();
              if ((ch>='0') && (ch<='9')) {
                s[cp++]=3;
                s[cp++]=ch;
                ansic(ch-'0');
              } else if ((ch==16) && (cp<maxlen-2)) {
                ch=getkey();

                if (ch != 16) {     // RIP font style code
                  s[cp++]=15;
                  s[cp++]=15;
                  s[cp++]=ch;
                  outchr('\xf');
                  outchr('\xf');
                  outchr(ch);
                }
              }
            }
            break;
          case 9:  /* Tab */
            i=5-(cp % 5);
            if (((cp+i)<maxlen) && ((WhereX()+i)<thisuser.screenchars)) {
              i=5-((WhereX()+1) % 5);
              for (i1=0; i1<i; i1++) {
                s[cp++]=32;
                outchr(32);
              }
            }
            break;
        }
  } while ((done==0) && (hangup==0));
  if (ch!=13) {
    i=cp-1;
    while ((i>0) && (s[i]!=32) && (s[i]!=8))
      i--;
    if ((i>(WhereX()/2)) && (i!=(cp-1))) {
      i1=cp-i-1;
      for (i=0; i<i1; i++)
        outchr(8);
      for (i=0; i<i1; i++)
        outchr(32);
      for (i=0; i<i1; i++)
        rollover[i]=s[cp-i1+i];
      rollover[i1]=0;
      cp -= i1;
    }
    s[cp++]=1;
    s[cp]=0;
  }
  if (crend)
    nl();
  return(0);
}

/****************************************************************************/

void inli(char *s, char *rollover, int maxlen, int crend)
{
  inli2(s,rollover,maxlen,crend,0);
}

/****************************************************************************/

/*
 * Returns 1 if current user has sysop access (permanent or temporary), else
 * returns 0.
 */

int so(void)
{
  if (actsl==255)
    return(1);
  else
    return(0);
}

/****************************************************************************/

/*
 * Returns 1 if current user has co-sysop access (or better), else returns 0.
 */

int cs(void)
{
  slrec ss;

  ss=syscfg.sl[actsl];
  if (so())
    return(1);
  if (ss.ability & ability_cosysop)
    return(1);
  else
    return(0);
}

/****************************************************************************/

/*
 * Returns 1 if current user has limited co-sysop access (or better), else
 * returns 0.
 */

int lcs(void)
{
  slrec ss;

  ss=syscfg.sl[actsl];
  if (cs())
    return(1);
  if (ss.ability & ability_limited_cosysop) {
    if (*qsc==999)
      return(1);
    if (*qsc==usub[cursub].subnum)
      return(1);
    else
      return(0);
  } else
    return(0);
}

/****************************************************************************/

/*
 * Checks to see if user aborted whatever he/she was doing. Returns 1 in
 * *next if control-N was hit, for zipping past messages quickly.
 */

void checka(int *abort, int *next)
{
  char ch;

  while ((!empty()) && (!(*abort)) && (!hangup)) {
    checkhangup();
    ch=inkey();
    if (!tagging || (thisuser.sysstatus & sysstatus_no_tag))
      lines_listed=0;
    switch(ch) {
      case 14:
        *next=1;
      case 3:
      case 32:
      case 24:
        *abort=1;
        break;
      case 'P':
      case 'p':
      case 19:
        ch=getkey();
        break;
    }
  }
}

/****************************************************************************/

/*
 * Prints an abortable string (contained in *s). Returns 1 in *abort if the
 * string was aborted, else *abort should be zero.
 */

void pla(char *s, int *abort)
{
  int i,next;

  i=0;
  checkhangup();
  if (hangup)
    *abort=1;
  checka(abort,&next);
  while ((s[i]) && (!(*abort))) {
    outchr(s[i++]);
    checka(abort,&next);
  }
  if (!(*abort))
    nl();
}

/****************************************************************************/

void plal(char *s, int limit, int *abort)
{
  int i,next;

  i=0;
  checkhangup();
  if (hangup)
    *abort=1;
  checka(abort,&next);
  limit+=strlen(s)-strlen(stripcolors(s));
  while ((s[i]) && (!(*abort)) && (i<limit)) {
    if((s[i]!='\r') && (s[i]!='\n'))
      outchr(s[i++]);
    else
      i++;
    checka(abort,&next);
  }
  if (!(*abort))
    nl();
}

/****************************************************************************/

/*
 * Returns current time as string formatted like HH:MM:SS (01:13:00).
 */

char *ctim(double d)
{
  static char ch[10];
  long h,m,s;

  if (d<0)
    d += 24.0*3600.0;
  h=(long) (d/3600.0);
  d-=(double) (h*3600);
  m=(long) (d/60.0);
  d-=(double) (m*60);
  s=(long) (d);
  sprintf(ch,"%02.2ld:%02.2ld:%02.2ld",h,m,s);

  return(ch);
}

/*
 * Returns 1 if sysop is "chattable", else returns 0. Takes into account
 * current user's chat restriction (if any) and sysop high and low times,
 * if any, as well as status of scroll-lock key.
 */

int sysop2(void)
{
  int ok;

  ok=sysop1();
  if (restrict_chat & thisuser.restrict)
    ok=0;
  if (syscfg.sysoplowtime != syscfg.sysophightime) {
    if (syscfg.sysophightime>syscfg.sysoplowtime) {
      if ((timer()<=(syscfg.sysoplowtime*60.0)) || (timer()>=(syscfg.sysophightime*60.0)))
        ok=0;
    } else {
      if ((timer()<=(syscfg.sysoplowtime*60.0)) && (timer()>=(syscfg.sysophightime*60.0)))
        ok=0;
    }
  }
  return(ok);
}

/****************************************************************************/

/*
 * Returns 1 if computer type string in *s matches the current user's
 * defined computer type, else returns 0.
 */

int checkcomp(char *s)
{
  if (strstr(ctypes[thisuser.comp_type],s))
    return(1);
  else
    return(0);
}

/****************************************************************************/

/*
 * Returns 1 if ANSI detected, or if local user, else returns 0. Uses the
 * cursor position interrogation ANSI sequence for remote detection.
 */

int check_ansi(void)
{
  long l;
  char ch;

  if (!incom)
    return(1);

  while (comhit())
    get1c();

  pr1("\x1b[6n");

  l=timer1()+36;
  if (modem_flag & flag_ec)
    l += 18;

  while ((timer1()<l) && (!hangup)) {
    checkhangup();
    ch=get1c();
    if (ch=='\x1b') {
      l=timer1()+18;
      while ((timer1()<l) && (!hangup)) {
        if ((timer1()+1820)<l)
          l=timer1()+18;
        checkhangup();
        ch=get1c();
        if (ch) {
          if (((ch<'0') || (ch>'9')) && (ch!=';') && (ch!='['))
            return(1);
        }
      }
      return(1);
    } else if (ch=='N')
      return(-1);
    if ((timer1()+1820)<l)
      l=timer1()+36;
  }
  return(0);
}

/****************************************************************************/

/*
 * Reads in menus and so forth in a messagerec array in memory, limited to
 * maxary items in the array. The filename read in is contained in *fn, and
 * the language dir for the current user is the dir used, if possible. The
 * BBS will abort here if it cannot read the file into memory.
 */

int read_in_file(char *fn, messagerec *m, int maxary)
{
  int i,i1,i2;
  char *buf,s[81];
  long l,l1;

  for (i=0; i<maxary; i++) {
    m[i].stored_as=0L;
    m[i].storage_type=255;
  }
  sprintf(s,"%s%s",languagedir,fn);
  i=sh_open1(s,O_RDONLY | O_BINARY);
  if (i<0) {
    sprintf(s,"%s%s",syscfg.gfilesdir,fn);
    i=sh_open1(s,O_RDONLY|O_BINARY);
      if (i<0)
        return(1);
  }
  l=filelength(i);
  buf=(char *) malloca(l);
  sh_lseek(i,0L,SEEK_SET);
  if (buf==NULL) {
    printf("NOT ENOUGH MEMORY.\n");
    end_bbs(noklevel);
  }
  sh_read(i,(void *) buf,l);
  sh_close(i);
  i1=0;
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
          if ((i2>=0) && (i2<maxary))
            m[i2].stored_as=l1;
          i1=0;
        }
      }
    }
  }
  bbsfree((void *) buf);

  return(0);
}

/****************************************************************************/

/*
 * Sets current language to language index n, reads in menus and help files,
 * and initializes stringfiles for that language. Returns 1 if problem,
 * else returns 0.
 */

int set_language_1(int n)
{
  int fail=0, idx;

  for (idx=0; idx<num_languages; idx++)
    if (languages[idx].num==n)
      break;

  if ((idx>=num_languages) && (n==0))
    idx=0;

  if (idx>=num_languages)
    return(1);

  cur_lang=n;
  cur_lang_idx=idx;
  cur_lang_name=languages[idx].name;
  languagedir=languages[idx].dir;

  if (set_strings_fn(0, languagedir, "BBS.STR", 0)) {
    printf("Couldn't open %sBBS.STR for language #%d\n",
            languagedir, n);
    if (n==0) {
      printf("Copy ENGLISH.STR from the distribution archive to %sBBS.STR\n",
             languagedir);
    }
    fail=1;
  }

  if (n==0) {
    if (set_strings_fn(1, languagedir, "SYSOPLOG.STR", 0)) {
      printf("Couldn't open %sSYSOPLOG.STR for language #%d\n",
             languagedir, n);
      printf("Copy SYSOPLOG.STR from the distribution archive to %sSYSOPLOG.STR\n",
              languagedir);
      fail=1;
    }
  }

  if (fail)
    return(fail);

  set_strings_fn(2, languagedir, "YES.STR", 0);
  set_strings_fn(3, languagedir, "NO.STR", 0);

  strncpy(str_yes,get_string(1), sizeof(str_yes)-1);
  strncpy(str_no,get_string(2), sizeof(str_no)-1);
  strncpy(str_quit,get_string(929), sizeof(str_quit)-1);
  strncpy(str_pause,get_string(923), sizeof(str_pause)-1);
  str_yes[0]=upcase(str_yes[0]);
  str_no[0]=upcase(str_no[0]);
  str_quit[0]=upcase(str_quit[0]);

  if (read_in_file("MENUS.MSG",menus,sizeof(menus)/sizeof(menus[0]))) {
    printf("Couldn't open %sMENUS.MSG for language #%d\n",languagedir,n);
    return(1);
  }
  if (read_in_file("HELP.MSG",helps,sizeof(helps)/sizeof(helps[0]))) {
    printf("Couldn't open %sHELP.MSG for language #%d\n",languagedir,n);
    return(1);
  }
  read_in_file("MENUSANS.MSG",menus1,sizeof(menus1)/sizeof(menus1[0]));
  read_in_file("MENUS40.MSG",menus2,sizeof(menus2)/sizeof(menus2[0]));
  rip_file("MENUSSOF.MSG",menus3,sizeof(menus3)/sizeof(menus3[0]));
  rip_file("MENUSLCL.MSG",menusl,sizeof(menusl)/sizeof(menusl[0]));

  return(0);
}

/****************************************************************************/

/*
 * Sets language to language #n, returns 1 if a problem, else returns 0.
 */

int set_language(int n)
{
  int pl;

  if (cur_lang==n)
    return(0);

  pl=cur_lang;

  if (set_language_1(n)) {
    if (pl>=0) {
      if (set_language_1(pl))
        set_language_1(0);
    }
    return(1);
  }

  return(0);
}

/****************************************************************************/

void cd_to(char *s)
{
  char s1[81];
  int i,db;

  strcpy(s1,s);
  i=strlen(s1)-1;
  db=(s1[i]=='\\');
  if (i==0)
    db=0;
  if ((i==2) && (s1[1]==':'))
    db=0;
  if (db)
    s1[i]=0;
  chdir(s1);
  if (s[1]==':') {
    setdisk(s[0]-'A');
    if (s[2]==0)
      chdir("\\");
  }
}

void get_dir(char *s, int be)
{
  strcpy(s,"X:\\");
  s[0]='A'+getdisk();
  getcurdir(0,&(s[3]));
  if (be) {
    if (s[strlen(s)-1]!='\\')
      strcat(s,"\\");
  }
}


