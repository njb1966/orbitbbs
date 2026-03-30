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

/****************************************************************************/

/*
 * Returns 1 if local sysop functions accessible, else returns 0.
 */

int ok_local(void)
{
  if (syscfg.sysconfig & sysconfig_no_local)
    return(0);
  else
    return(1);
}

/****************************************************************************/

/*
 * Takes user name/handle as parameter, and returns user number, if found,
 * else returns 0.
 */

int finduser1(unsigned char *sx)
{
  int i,i1,i2;
  unsigned char s[81],s1[81],ch;
  userrec u;

  if (sx[0]==0)
    return(0);
  i=finduser(sx);
  if (i>0)
    return(i);
  strcpy(s,sx);
  for (i=0; s[i]!=0; i++)
    s[i]=upcase(s[i]);
  i2=0;
  read_status();
  for (i=0; (i<status.users) && (i2==0); i++) {
    if (strstr(smallist[i].name,s)!=NULL) {
      i1=smallist[i].number;
      read_user(i1,&u);
      sprintf(s1,"%s %s %s ? ",get_string(618), nam(&u,i1), get_string(619));
      prt(5,s1);
      ch=ynq();
      if (ch=='Y')
        i2=i1;
      if (ch=='Q')
        i=status.users;
    }
  }
  return(i2);
}

/****************************************************************************/

/*
 * Creates sysoplog filename in s, from datestring.
 */

void slname(char *d, char *s)
{
  sprintf(s,"%c%c%c%c%c%c.LOG",
    d[6],d[7],d[0],d[1],d[3],d[4]);
}

/****************************************************************************/

/*
 * Returns instance (temporary) sysoplog filename in s.
 */

void islname(char *s)
{
  sprintf(s,"INST-%3.3d.LOG",instance);
}

/****************************************************************************/

#define CAT_BUFSIZE 8192

/*
 * Copies temporary/instance sysoplog to primary sysoplog file.
 */

void catsl(void)
{
  char s[81], s1[81], instname[81], wholename[81];
  int fi, fo, i;
  char *buf;

  islname(s1);
  sprintf(instname,"%s%s",syscfg.gfilesdir, s1);

  if (exist(instname)) {
    slname(date(), s);
    sprintf(wholename, "%s%s",syscfg.gfilesdir, s);

    buf=malloca(CAT_BUFSIZE);
    if (buf) {
      fo=sh_open(wholename, O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
      if (fo>0) {
        sh_lseek(fo, 0L, SEEK_END);

        fi=sh_open1(instname, O_RDONLY|O_BINARY);
        if (fi>0) {
          do {
            i=sh_read(fi, buf, CAT_BUFSIZE);
            if (i>0)
              sh_write(fo, buf, i);
          } while (i==CAT_BUFSIZE);

          sh_close(fi);
          unlink(instname);
        }
        sh_close(fo);
      }
      bbsfree(buf);
    }
  }
}

/****************************************************************************/

/*
 * Writes a line to the sysoplog.
 */

void sl1(int cmd, char *s)
{
  static int midline=0;
  static char slfn[81];
  char l[180],ch1;
  int i,f=-1;

  if (!slfn[0]) {
    strcpy(slfn, syscfg.gfilesdir);
    islname(slfn+strlen(slfn));
  }

  switch(cmd) {
    case 0: /* Write line to sysop's log */
      f=sh_open(slfn,O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
      if (f<0) {
        /* sysop log ? */
        return;
      }
      if (filelength(f)) {
        sh_lseek(f,-1L,SEEK_END);
        sh_read(f,((void *)&ch1),1);
        if (ch1==26)
          sh_lseek(f,-1L,SEEK_END);
      }

      if (midline) {
        sprintf(l,"\r\n%s",s);
        midline = 0;
      } else {
        sprintf(l,"%s",s);
      }
      if (syscfg.sysconfig & sysconfig_printer)
        fprintf(stdprn,"%s\r\n",l);
      i=strlen(l);
      l[i++]='\r';
      l[i++]='\n';
      l[i]=0;
      sh_write(f,(void *)l,i);
      f=sh_close(f);
    break;
    case 4:
      f = sh_open(slfn, O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
      if (f<0) {
        /* sysop log ? */
        return;
      }
      if (filelength(f)) {
        sh_lseek(f, -1L, SEEK_END);
        sh_read(f, ((void *)&ch1), 1);
        if (ch1 == 26)
          sh_lseek(f, -1L, SEEK_END);
     }
     if (!midline || ((midline + 2 + strlen(s)) > 78)) {
       strcpy(l, midline ? "\r\n   " : "   ");
       strcat(l, s);
       midline = 3 + strlen(s);
     } else {
       strcpy(l, ", ");
       strcat(l, s);
       midline += (2 + strlen(s));
     }
     if (syscfg.sysconfig & sysconfig_printer)
       fprintf(stdprn, "%s", l);
     i = strlen(l);
     sh_write(f, (void *)l, i);
     f=sh_close(f);
     break;
  }
}

/****************************************************************************/

/*
 * Writes a string to the sysoplog, if user online and actsl < 255.
 */

void sysopchar(char *s)
{
  if ((incom || (actsl != 255)) && (s[0]))
    sl1(4, s);
}

/****************************************************************************/

/*
 * Writes a string to the sysoplog, if actsl < 255 and user online,
 * indented a few spaces.
 */

void sysoplog(char *s)
{
  char s1[180];

  if ((incom) || (actsl!=255)) {
    sprintf(s1,"   %s",s);
    sl1(0,s1);
  }
}

/****************************************************************************/

/*
 * Sends a short message to a user. Takes user number and system number
 * and short message as params. Network number should be set before calling
 * this function.
 */

void ssm(unsigned int un, unsigned int sy, char *s)
{
  int  f,i,i1;
  userrec u;
  char s1[161];
  shortmsgrec sm;
  net_header_rec nh;

  if ((un==65535) || (un==0))
    return;
  if (sy==0) {
    read_user(un,&u);
    if (!(u.inact & inact_deleted)) {
      sprintf(s1,"%sSMW.DAT",syscfg.datadir);
      f=sh_open(s1,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      if (f<0)
        return;
      i=(int) (filelength(f) / sizeof(shortmsgrec));
      i1=i-1;
      if (i1>=0) {
        sh_lseek(f,((long) (i1)) * sizeof(shortmsgrec), SEEK_SET);
        sh_read(f,(void *)&sm,sizeof(shortmsgrec));
        while ((sm.tosys==0) && (sm.touser==0) && (i1>0)) {
          --i1;
          sh_lseek(f,((long) (i1)) * sizeof(shortmsgrec), SEEK_SET);
          sh_read(f,(void *)&sm,sizeof(shortmsgrec));
        }
        if ((sm.tosys) || (sm.touser))
          ++i1;
      } else
        i1=0;
      sm.tosys=sy;
      sm.touser=un;
      strncpy(sm.message,s,80);
      sm.message[80]=0;
      sh_lseek(f,((long) (i1)) * sizeof(shortmsgrec), SEEK_SET);
      sh_write(f,(void *)&sm,sizeof(shortmsgrec));
      sh_close(f);
      u.sysstatus |= sysstatus_smw;
      write_user(un,&u);
    }
  } else if (net_sysnum && valid_system(sy)) {
    nh.tosys=sy;
    nh.touser=un;
    nh.fromsys=net_sysnum;
    nh.fromuser=usernum;
    nh.main_type = main_type_ssm;
    nh.minor_type=0;
    nh.list_len=0;
    time((long *)&nh.daten);
    if (strlen(s)>80)
      s[80]=0;
    nh.length=strlen(s);
    nh.method=0;
    sprintf(s1,"%sP0%s",net_data,nete);
    f=sh_open(s1,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    sh_lseek(f,0L,SEEK_END);
    sh_write(f,(void *)&nh,sizeof(net_header_rec));
    sh_write(f,(void *)s,nh.length);
    sh_close(f);
  }
}


/****************************************************************************/

/*
 * Writes BBS location data to instance.dat, so other instances can see
 * some info about this instance.
 */

void write_inst(unsigned short loc, unsigned short subloc, unsigned short flags)
{
  char s[81];
  int f,re_write;
  unsigned short cf, ms;
  static instancerec ti;
  instancerec ir;

  if (ti.user==0) {
    if (get_inst_info(instance,&ir))
      ti.user=ir.user;
    else
      ti.user=1;
  }

  re_write=0;
  curloc=loc;


  cf=ti.flags & (~(INST_FLAGS_ONLINE | INST_FLAGS_MSG_AVAIL));
  if (useron) {
    cf |= INST_FLAGS_ONLINE;
    if (!(thisuser.sysstatus & sysstatus_no_msgs)) {
      switch (loc) {
        case INST_LOC_MAIN:
        case INST_LOC_XFER:
        case INST_LOC_SUBS:
        case INST_LOC_EMAIL:
        case INST_LOC_CHATROOM:
        case INST_LOC_RMAIL:
          cf |= INST_FLAGS_MSG_AVAIL;
          break;
      }
    } else if (loc == INST_LOC_CHATROOM) {
      cf |= INST_FLAGS_MSG_AVAIL;
    }
    if (using_modem)
      ms=modem_speed;
    else
      ms=0;
    if (ti.modem_speed != ms) {
      ti.modem_speed = ms;
      re_write=1;
    }
  }

  if (flags != INST_FLAGS_NONE) {
    if (flags & 0x8000) {
      /* reset an option */
      ti.flags &= flags;
    } else {
      /* set an option */
      ti.flags |= flags;
    }
  }

  if (cf!=ti.flags) {
    re_write=1;
    ti.flags=cf;
  }

  if (ti.number!=instance) {
    re_write=1;
    ti.number=instance;
  }

  if (loc==INST_LOC_DOWN)
    re_write=1;
  else {
    if (useron) {
      if (ti.user!=usernum) {
        re_write=1;
        if ((usernum>0) && (usernum<=syscfg.maxusers))
          ti.user=usernum;
      }
    }
  }

  if (ti.subloc!=subloc) {
    re_write=1;
    ti.subloc=subloc;
  }

  if (ti.loc!=loc) {
    re_write=1;
    ti.loc=loc;
  }

  if (re_write) {
    time((long *)&ti.last_update);
    sprintf(s,"%sINSTANCE.DAT",syscfg.datadir);
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    if (f>0) {
      lseek(f,(long) (instance * sizeof(instancerec)),SEEK_SET);
      write(f,(void *)&ti,sizeof(instancerec));
      sh_close(f);
    }
  }
}
/****************************************************************************/

/*
 * Finds usernum and system number from char *s, sets network number as
 * appropriate.
 */

void parse_email_info(char *s, unsigned short *un1, unsigned short *sy1)
{
  char *ss,*ss1,onx[20],ch,*mmk;
  unsigned un, sy;
  int i,nv,on,xx,onxi,odci;
  net_system_list_rec *csne;

  *un1=0;
  *sy1=0;
  ss=strrchr(s,'@');
  if (ss==NULL) {
    un=finduser1(s);
    if (un>0)
      *un1=un;
    else
      pl(get_string(8));
  } else if (atoi(ss+1)==0) {
    for (i=0; i<net_num_max; i++) {
      set_net_num(i);
      if (strnicmp("internet",net_name,8)==0) {
        strcpy(net_email_name,s);
        *sy1=1;
      }
    }
    if (i>=net_num_max)
      pl(get_string(8));
  } else {
    ss[0]=0;
    ss=&(ss[1]);
    i=strlen(s);
    while ((i>0) && (s[i-1]==' '))
      --i;
    s[i]=0;
    un=atoi(s);
    if ((un==0) && (s[0]=='#'))
      un=atoi(s+1);
    if (strchr(s,'@'))
      un=0;
    sy=atoi(ss);
    ss1=strchr(ss,'.');
    if (ss1)
      ss1++;
    if (un==0) {
      strcpy(net_email_name,s);
      i=strlen(net_email_name);
      while ((i>0) && (net_email_name[i-1]==' '))
        --i;
      net_email_name[i]=0;
      if (net_email_name[0])
        *sy1=sy;
      else
        pl(get_string(8));
    } else {
      *un1=un;
      *sy1=sy;
    }
    if (*sy1 && ss1) {
      for (i=0; i<net_num_max; i++) {
        set_net_num(i);
        if (stricmp(ss1,net_name)==0) {
          if (!valid_system(*sy1)) {
            nl();
            outstr(get_string(9));
            npr("%s @%u.\r\n",ss1,*sy1);
            nl();
            *sy1=*un1=0;
          } else {
            if (*sy1==net_sysnum) {
              *sy1=0;
              if (*un1==0)
                *un1=finduser(net_email_name);
              if ((*un1==0) || (*un1>32767)) {
                *un1=0;
                pl(get_string(8));
              }
            }
          }
          break;
        }
      }
      if (i>=net_num_max) {
        nl();
        outstr(get_string(10));
        outstr(ss1);
        nl();
        *sy1=*un1=0;
      }
    } else if (*sy1 && (net_num_max>1)) {
      odc[0]=0;
      odci=0;
      onx[0]='Q';
      onx[1]=0;
      onxi=1;
      nv=0;
      on=net_num;
      ss=malloca(net_num_max);
      xx=-1;
      for (i=0; i<net_num_max; i++) {
        set_net_num(i);
        if (net_sysnum==*sy1) {
          xx=i;
        } else if (valid_system(*sy1)) {
          ss[nv++]=i;
        }
      }
      set_net_num(on);
      if (nv==0) {
        if (xx!=-1) {
          set_net_num(xx);
          *sy1=0;
          if (*un1==0) {
            *un1=finduser(net_email_name);
            if ((*un1==0) || (*un1>32767)) {
              *un1=0;
              pl(get_string(8));
            }
          }
        } else {
          nl();
          pl(get_string(11));
          *sy1=*un1=0;
        }
      } else if (nv==1) {
        set_net_num(ss[0]);
      } else {
        nl();
        for (i=0; i<nv; i++) {
          set_net_num(ss[i]);
          csne=next_system(*sy1);
          if (csne) {
            if (i<9) {
              onx[onxi++]=i+'1';
              onx[onxi]=0;
            } else {
              odci=(i+1)/10;
              odc[odci-1]=odci+'0';
              odc[odci]=0;
            }
            npr("%d. %s (%s)\r\n",i+1,net_name,csne->name);
          }
        }
        pl(get_string(12));
        nl();
        prt(2,get_string(13));
        if (nv<9) {
          ch=onek(onx);
          if (ch=='Q')
            i=-1;
          else
            i=ch-'1';
        } else {
          mmk=mmkey(2);
          if (*mmk=='Q')
            i=-1;
          else
            i=atoi(mmk)-1;
        }
        if ((i>=0) && (i<nv)) {
          set_net_num(ss[i]);
        } else {
          nl();
          pl(get_string(14));
          nl();
          *un1=*sy1=0;
        }
      }
      bbsfree(ss);
    } else {
      if (*sy1==net_sysnum) {
        *sy1=0;
        if (*un1==0)
          *un1=finduser(net_email_name);
        if ((*un1==0) || (*un1>32767)) {
          *un1=0;
          pl(get_string(8));
        }
      } else if (!valid_system(*sy1)) {
        nl();
        pl(get_string(11));
        *sy1=*un1=0;
      }
    }
  }
}

/****************************************************************************/

/*
 * Checks system password, returns 1 if pw entered is valid, else returns 0.
 */

int checkpw(void)
{
  char s[81];

  nl();
  prt(2,get_string(616));
  echo=0;
  input(s,20);
  echo=1;
  if (strcmp(s,(syscfg.systempw))==0)
    return(1);
  nl();
  pl("Incorrect password.");
  return(0);
}

/****************************************************************************/

/*
 * Ends BBS execution with errorlevel of lev set. Closes stringfiles and
 * restores all interrupts to previous state.
 */

void end_bbs(int lev)
{
  char s[81];

  sprintf(s,get_stringx(1,131), wwiv_version,
          instance, times(), date());
  sl1(0,"");
  sl1(0, s);
  sl1(0,"");
  sl1(1,"");
  catsl();
  close_strfiles();
#ifdef __OS2__
  if (stdprn != NULL)
    fclose(stdprn);
#endif
  write_inst(INST_LOC_DOWN,0,INST_FLAGS_NONE);
  if (ok_modem_stuff)
    closeport();
  outs(wwiv_version);
  outs(" run complete.\r\n\r\n");
#ifndef __OS2__
  setvect(save_dos,NULL);
#endif
  exit(lev);
}

/****************************************************************************/

/*
 * Returns 1 if date has changed, else returns 0.
 */

int date_changed(void)
{
  struct date today,today1;

  getdate(&today);
  getdate(&today1);
  if (today.da_day==today1.da_day)
    return(0);
  else
    return(1);
}

/****************************************************************************/

/*
 * Displays a file locally, using LIST util if so defined in INIT, else
 * uses normal TTY output.
 */

void print_local_file(char *ss, char *ss1)
{
  char s[81];
  char s1[81];
  char *bs;

  bs=strchr(ss,'\\');
  if ((syscfg.sysconfig & sysconfig_list) && (!incom)) {
    if (!bs) {
      sprintf(s,"%s %s%s",get_string(1041),syscfg.gfilesdir,ss);
      if (ss1[0]) {
        bs=strchr(ss1,'\\');
        if (!bs)
          sprintf(s1,"%s %s%s",s,syscfg.gfilesdir,ss1);
        else
          sprintf(s1,"%s %s",s,ss1);
        strcpy(s,s1);
      }
    } else {
      sprintf(s,"%s %s",get_string(1041),ss);
      if (ss1[0]) {
        bs=strchr(ss1,'\\');
        if (!bs)
          sprintf(s1,"%s %s%s",s,syscfg.gfilesdir,ss1);
        else
          sprintf(s1,"%s %s",s,ss1);
        strcpy(s,s1);
      }
    }
    extern_prog(s, 0);
    if (useron) {
      clrscrb();
      topscreen();
    }
  } else {
    printfile(ss);
    nln(2);
    pausescr();
  }
}

/****************************************************************************/

/*
 * Hangs up the modem if user online. Whether using modem or not, sets
 * hangup to 1.
 */

void hang_it_up(void)
{
  int i;

  hangup=1;

  if (!ok_modem_stuff)
    return;

  dtr(0);
  if (cdet()) {
    wait1(9);
    if (cdet()) {
      wait1(9);
      if (cdet()) {
        i=0;
        dtr(1);
        while ((i++<2) && (cdet())) {
          wait1(27);
          pr1("\x1\x1\x1");
          wait1(54);
          if (modem_i->hang[0])
            pr1(modem_i->hang);
          else
            pr1("ATH\r");
          wait1(6);
        }
      }
    }
  }
  dtr(1);
}

/****************************************************************************/

/*
 * Plays a sound definition file (*.SDF) through PC speaker. SDF files
 * should reside in the gfiles dir. The only params passed to function are
 * filename and 0 if playback is unabortable, 1 if it is abortable. If no
 * extension then .SDF is appended. A full path to file may be specified to
 * override gfiles dir. Format of file is:
 *
 * <freq> <duration in ms> [pause_delay in ms]
 * 1000 1000 50
 *
 * Returns 1 if sucessful, else returns 0. The pause_delay is optional and
 * is used to insert silences between tones.
 */

#ifdef __OS2__
#define sound(freq) setbeep(freq)
#define nosound() setbeep(0)
#endif


int play_sdf(unsigned char *sdfn, int abortable)
{
  FILE *f;
  unsigned char soundline[81], tempstring[81], fname[81];
  int freq, dur, pausedelay, nw;

  /* append gfilesdir if no path specified */
  if (strchr(sdfn,'\\')==NULL) {
    strncpy(fname, syscfg.gfilesdir, sizeof(fname));
    strncat(fname, sdfn, sizeof(fname));
  } else
    strncpy(fname, sdfn, sizeof(fname));

  /* append .SDF if no extension specified */
  if (strchr(fname,'.')==NULL)
    strncat(fname, ".SDF", sizeof(fname));

  /* must exist */
  if (!exist(fname))
    return(0);

  /* must be able to open read-only */
  f=fsh_open(fname,"rt");
  if (!f)
    return(0);

  /* scan each line, ignore lines with words<2 */
  while (fgets(soundline, sizeof(soundline), f) != NULL) {
    if ((abortable) && (!empty()))
      break;
    freq=dur=pausedelay=0;
    nw=wordcount(soundline, DELIMS_WHITE);
    if (nw>=2) {
      strncpy(tempstring,extractword(1,soundline,DELIMS_WHITE),sizeof(tempstring));
      freq=atoi(tempstring);
      strncpy(tempstring,extractword(2,soundline,DELIMS_WHITE),sizeof(tempstring));
      dur=atoi(tempstring);

      /* only play if freq and duration > 0 */
      if ((freq>0) && (dur>0)) {
        if (nw>2) {
          strncpy(tempstring,extractword(3,soundline,DELIMS_WHITE),sizeof(tempstring));
          pausedelay=atoi(tempstring);
        }
        sound(freq);
        delay(dur);
        nosound();
        if (pausedelay>0)
          delay(pausedelay);
      }
    }
  }

  /* close and return success */
  fsh_close(f);
  return(1);
}

/****************************************************************************/



