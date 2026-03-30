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
#include <math.h>
#include <ctype.h>

#if defined(__OS2__) && !defined(KBDSTF_SCROLLLOCK_ON)
#define KBDSTF_SCROLLLOCK_ON 0x0010
#endif


/* functions for external programs to call */

#ifndef __OS2__
#pragma warn -par

void far interrupt inlii(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, char *s2, int i1, int i2)
{
  inli(s1,s2,i1,i2);
}

void far interrupt checkai(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int *i1, int *i2)
{
  checka(i1,i2);
}


void far interrupt plai(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int *i1)
{
  pla(s1,i1);
}


void far interrupt outchri(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char ch)
{
  outchr(ch);
}


void far interrupt outstri(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  outstr(s1);
}


void far interrupt nli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  nl();
}


void far interrupt pli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  pl(s1);
}


void far interrupt emptyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=empty();
}


void far interrupt inkeyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=(unsigned) empty();
}


void far interrupt getkeyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=(unsigned) getkey();
}


void far interrupt inputi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int i)
{
  input(s1,i);
}


void far interrupt inputli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int i)
{
  inputl(s1,i);
}


void far interrupt yni(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=yn();
}



void far interrupt nyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=ny();
}


void far interrupt ansici(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1)
{
  ansic(i1);
}


void far interrupt oneki(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  ax=(unsigned) onek(s1);
}


void far interrupt prti(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1, char *s1)
{
  prt(i1,s1);
}



void far interrupt mpli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1)
{
  mpl(i1);
}


#pragma warn +par
#endif

/* end of functions for external programs to call */


void reset_act_sl(void)
{
  actsl = thisuser.sl;
}

void show_files(char *fn, char *dir)
/* Displays list of files matching filespec fn in directory dir. */
{
  int f1,i;
  char s[120],s1[120],c;
  struct ffblk ff;
  unsigned char drive[MAXDRIVE], direc[MAXDIR], file[MAXFILE], ext[MAXEXT];

  c=(okansi())?205:'=';
  nl();

  fnsplit(dir,drive,direc,file,ext);
  sprintf(s,"%s%s",get_string(971),strupr(stripfn(fn)));
  strcat(s,get_string(975));
  strcat(s,drive); strcat(s,direc);
  strcat(s,get_string(976));
  i=(thisuser.screenchars-1)/2 - strlen(stripcolors(s))/2;
  npr("7%s",charstr(i,c));
  outstr(s);
  i=thisuser.screenchars-1-i-strlen(stripcolors(s));
  npr("7%s",charstr(i,c));

  sprintf(s1,"%s%s",dir,strupr(stripfn(fn)));
  f1=findfirst(s1,&ff,0);
  while (f1==0) {
    strcpy(s,ff.ff_name);
    align(s);
    sprintf(s1,"7[2%s7]1 ",s);
    if (WhereX()>(thisuser.screenchars-15))
      nl();
    outstr(s1);
    f1=findnext(&ff);
  }

  nl();
  ansic(7);
  pl(charstr(thisuser.screenchars-1,c));
  nl();
}

void remove_from_temp(char *fn, char *dir, int po)
{
  int f1,ok;
  char s[81],s1[81];
  struct ffblk ff;

  sprintf(s1,"%s%s",dir,stripfn(fn));
  f1=findfirst(s1,&ff,0);
  ok=1;
  nl();
  while ((f1==0) && (ok)) {
    sprintf(s,"%s%s",dir,ff.ff_name);
    if (po) {
      outstr(get_string(919));
      pl(ff.ff_name);
    }
    _chmod(s,1,0);
    unlink(s);
    f1=findnext(&ff);
  }
}

void check_event(void)
{
  double tl;

  if ((syscfg.executetime) && (instance==1)) {
    tl=time_event-timer();
    if (tl<0.0)
      tl += 24.0*3600.0;
    if ((tl-last_time)>30.0)
      do_event=1;
    last_time=tl;
  }
}


void run_event(void)
{
  if ((do_event) && (syscfg.executetime)) {
    do_event=0;
    nl();
    pl(get_string(920));
    nl();
    if (syscfg.executestr[0]) {
      if (instance==1) {
        holdphone(1);
        extern_prog(syscfg.executestr, sysinfo.spawn_opts[0]);
        holdphone(0);
      }
      read_status();
    } else
      end_bbs(oklevel);
  }
  clrscrb();
}


double freek(int dr)
{
  float d;
  struct dfree df;

  getdfree(dr,&df);
  d=(float) df.df_avail;
  d*=((float) df.df_bsec);
  d*=((float) df.df_sclus);
  d/=1024.0;
  if (df.df_sclus==0xffff)
    d=-1.0;
  return(d);
}

/* This should not be a problem 'till 2005 or so.                          */

unsigned char years_old(unsigned char m, unsigned char d, unsigned char y)
{
  struct date today;
  int a;

  getdate(&today);

  if (today.da_year-1900<y)
    return(0);
  if (today.da_year-1900==y) {
    if (today.da_mon<m)
      return(0);
    if (today.da_mon==m) {
      if (today.da_day<d)
        return(0);
    }
  }

  a=(int) (today.da_year-1900-y);
  if (today.da_mon<m)
    --a;
  else
    if ((today.da_mon==m) && (today.da_day<d))
      --a;
  return((unsigned char)a);
}



void itimer(void)
/* This function initializes the high-resolution timer */
{
#ifndef __OS2__
  outportb(0x43,0x34);
  outportb(0x40,0x00);
  outportb(0x40,0x00);
#endif

}

double timer(void)
/* This function returns the time, in seconds since midnight. */
{
  double cputim;
#ifdef __OS2__
#define SECSINMINUTE 60
#define SECSINHOUR (60 * SECSINMINUTE)

  DATETIME dt;
  long l;

  DosGetDateTime(&dt);
  l = (dt.hours * SECSINHOUR) + (dt.minutes * SECSINMINUTE) + dt.seconds;
  cputim = (double)l + (((double) dt.hundredths) / 100);
#else
  unsigned short int h,m,l1,l2;

  disable();
  outportb(0x43,0x00);
  m=peek(0x0040,0x006c);
  h=peek(0x0040,0x006e);
  l1=inportb(0x40);
  l2=inportb(0x40);
  enable();
  l1=((l2*256)+l1) ^ 65535;
  cputim=((h*65536. + m)*65536. + l1)*8.380955e-7;
#endif
  return (cputim);
}


long timer1(void)
/* This function returns the time, in ticks since midnight. */
{
#ifdef __OS2__
  #define TICKS_PER_SECOND 18.2

  return (long) (timer() * TICKS_PER_SECOND);
#else
  unsigned short h,m;
  long l;

  m=peek(0x0040,0x006c);
  h=peek(0x0040,0x006e);
  l=((long)h)*65536 + ((long)m);
  return(l);
#endif
}


int sysop1(void)
/* This function returns the status of scoll lock.  If scroll lock is active
 * (ie, the user has hit scroll lock + the light is lit if there is a
 * scoll lock LED), the sysop is assumed to be available.
 */
{
#ifdef __OS2__
  KBDINFO ki;

  KbdGetStatus(&ki, 0);
  return (ki.fsState & KBDSTF_SCROLLLOCK_ON);
#else
  if ((peekb(0,1047) & 0x10)==0)
    return(0);
  else
    return(1);
#endif
}



int okansi(void)
/* This function checks the status of the current user's record to see if
 * the user has specified that he wants ANSI graphics displayed.
 */
{
  if (x_only)
    return(0);
  if (thisuser.sysstatus & sysstatus_ansi)
    return(1);

  return(0);
}

void tmp_disable_conf(int disable)
{
  static int ocs, oss, ocd, osd, cnt;

  if (disable && (disable!=-1)) {
    cnt++;
    if (okconf(&thisuser)) {
      g_flags |= g_flag_disable_conf;
      ocs=curconfsub;
      oss=usub[cursub].subnum;
      ocd=curconfdir;
      osd=udir[curdir].subnum;
      setuconf(CONF_SUBS, -1, oss);
      setuconf(CONF_DIRS, -1, osd);
    }
  } else {
    if (cnt) {
      cnt--;
      if (disable==-1)
        cnt=0;
      if (cnt==0) {
        if (g_flags & g_flag_disable_conf) {
          g_flags &= ~g_flag_disable_conf;
          setuconf(CONF_SUBS, ocs, oss);
          setuconf(CONF_DIRS, ocd, osd);
        }
      }
    }
  }
}

void tmp_disable_pause(int disable)
{
  if (disable) {
    if (thisuser.sysstatus & sysstatus_pause_on_page) {
      g_flags |= g_flag_disable_pause;
      thisuser.sysstatus &= ~sysstatus_pause_on_page;
    }
  } else {
    if (g_flags & g_flag_disable_pause) {
      g_flags &= ~g_flag_disable_pause;
      thisuser.sysstatus |= sysstatus_pause_on_page;
    }
  }
}

int okconf(userrec *u)
/* Checks status of given userrec to see if conferencing is turned on.
 */
{
  if (g_flags & g_flag_disable_conf)
    return(0);

  if (u->sysstatus & sysstatus_conference)
    return(1);
  else
    return(0);
}

void frequent_init(void)
/* This should be called after a user is logged off, and will initialize
 * screen-access variables.
 */
{
  setiia(90);
  g_flags=0;
  in_fsed=0;
  tagging=0;
  curlsub=-1;
  curldir=-1;
  curconfsub=0;
  curconfdir=0;
  ansiptr=0;
  curatr=0x07;
  outcom=0;
  incom=0;
  charbufferpointer=0;
  andwith=0xff;
  checkit=0;
  topline=0;
  screenlinest=defscreenbottom+1;
  if (!restoring_shrink)
    clrscrb();
  endofline[0]=0;
  hangup=0;
  hungup=0;
  chatcall=0;
  chatreason[0]=0;
  useron=0;
  change_color=0;
  chatting=0;
  echo=1;
  irt[0]=0;
  irt_name[0]=0;
  okskey=0;
  lines_listed=0;
  read_user(1,&thisuser);
  read_qscn(1,qsc,0);
  if (thisuser.inact & inact_deleted)
    fwaiting=0;
  else
    fwaiting=thisuser.waiting;
  okmacro=1;
  okskey=1;
  helpl=0;
  ihelp=0;
  mailcheck=0;
  smwcheck=0;
  in_extern=0;
  gatfn[0]=0;
  use_workspace=0;
  extratimecall=0.0;
  two_color=0;
  using_modem=0;
  set_global_handle(0);
  live_user=1;
  _chmod(dszlog,1,0);
  unlink(dszlog);
  ltime=0;
  set_x_only(0, NULL, 0);
  set_net_num(0);
  set_language(thisuser.language);
  tmp_disable_conf(-1);
}


void fix_user_rec(userrec *u)
{
  u->name[sizeof(u->name)-1]=0;
  u->realname[sizeof(u->realname)-1]=0;
  u->callsign[sizeof(u->callsign)-1]=0;
  u->phone[sizeof(u->phone)-1]=0;
  u->pw[sizeof(u->pw)-1]=0;
  u->laston[sizeof(u->laston)-1]=0;
  u->note[sizeof(u->note)-1]=0;
  u->macros[0][sizeof(u->macros[0])-1]=0;
  u->macros[1][sizeof(u->macros[1])-1]=0;
  u->macros[2][sizeof(u->macros[2])-1]=0;
}


int number_userrecs(void)
{
  char s[81];
  int f,i;

  sprintf(s,"%sUSER.LST",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    i=((int) (filelength(f)/syscfg.userreclen)-1);
    f=sh_close(f);
  } else
    i=0;
  return(i);
}


void read_user(unsigned int un, userrec *u)
{
  char s[81];
  long pos;
  int f, nu;

  if (((useron) && (un==usernum)) || ((wfc) && (un==1))) {
    *u=thisuser;
    fix_user_rec(u);
    return;
  }

  sprintf(s,"%sUSER.LST",syscfg.datadir);

  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0) {
    u->inact=inact_deleted;
    fix_user_rec(u);
    return;
  }

  nu=((int) (filelength(f)/syscfg.userreclen)-1);

  if (un>nu) {
    sh_close(f);
    u->inact=inact_deleted;
    fix_user_rec(u);
    return;
  }
  pos=((long) syscfg.userreclen) * ((long) un);
  sh_lseek(f,pos,SEEK_SET);
  sh_read(f, (void *)u, syscfg.userreclen);
  sh_close(f);
  fix_user_rec(u);
}


void write_user(unsigned int un, userrec *u)
{
  char s[81];
  long pos;
  int f;

  if ((un<1) || (un>syscfg.maxusers))
    return;

  if (((useron) && (un==usernum)) || ((wfc) && (un==1))) {
    if (u != &thisuser)
      thisuser=*u;
  }

  sprintf(s,"%sUSER.LST",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  if (f>=0) {
    pos=((long) syscfg.userreclen) * ((long) un);
    sh_lseek(f,pos,SEEK_SET);
    sh_write(f, (void *)u, syscfg.userreclen);
    sh_close(f);
  }
}

/****************************************************************************/

int qscn_file=-1;

int open_qscn(void)
{
  char s[81];

  if (qscn_file==-1) {
    sprintf(s,"%sUSER.QSC",syscfg.datadir);
    qscn_file=sh_open(s,O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    if (qscn_file<0) {
      qscn_file=-1;
      return(0);
    }
  }
  return(1);
}

void close_qscn(void)
{
  if (qscn_file!=-1) {
    sh_close(qscn_file);
    qscn_file=-1;
  }
}

void read_qscn(unsigned int un, unsigned long *qscn, int stayopen)
{
  long pos;
  int i;

  if (((useron) && (un==usernum)) || ((wfc) && (un==1))) {
    if (qscn != qsc) {
      for (i=(syscfg.qscn_len/4)-1; i>=0; i--)
        qscn[i]=qsc[i];
    }
    return;
  }

  if (open_qscn()) {
    pos=((long)syscfg.qscn_len)*((long)un);
    if (pos + (long)syscfg.qscn_len <= filelength(qscn_file)) {
      sh_lseek(qscn_file,pos,SEEK_SET);
      sh_read(qscn_file,qscn,syscfg.qscn_len);
      if (!stayopen)
        close_qscn();
      return;
    }
  }

  if (!stayopen)
    close_qscn();

  memset(qsc, 0, syscfg.qscn_len);
  *qsc=999;
  memset(qsc+1,0xff,((syscfg.max_dirs+31)/32)*4);
  memset(qsc+1+(syscfg.max_dirs+31)/32,0xff,((syscfg.max_subs+31)/32)*4);
}

void write_qscn(unsigned int un, unsigned long *qscn, int stayopen)
{
  long pos;
  int i;

  if ((un<1) || (un>syscfg.maxusers))
    return;

  if (((useron) && (un==usernum)) || ((wfc) && (un==1))) {
    if (qsc != qscn) {
      for (i=(syscfg.qscn_len/4)-1; i>=0; i--)
        qsc[i]=qscn[i];
    }
  }

  if (open_qscn()) {
    pos=((long)syscfg.qscn_len)*((long)un);
    sh_lseek(qscn_file,pos,SEEK_SET);
    sh_write(qscn_file,qscn,syscfg.qscn_len);
    if (!stayopen)
      close_qscn();
  }
}

/****************************************************************************/


double ratio(void)
{
  double r;

  if (thisuser.dk==0)
    return(99.999);
  r=((float) thisuser.uk) / ((float) thisuser.dk);
  if (r>99.998)
    r=99.998;
  return(r);
}


double post_ratio(void)
{
  double r;

  if (thisuser.logons==0)
    return(99.999);
  r=((float) thisuser.msgpost) / ((float) thisuser.logons);
  if (r>99.998)
    r=99.998;
  return(r);
}


unsigned char *nam(userrec *u1, unsigned int un)
{
  unsigned char *ss;
  static unsigned char o[81];
  int f,p;
  userrec u;

  u=*u1;
  f=1;
  for (p=0; p<strlen(u.name); p++) {
    if (f) {
      ss=strchr(translate_letters[1],u.name[p]);
      if (ss)
        f=0;
      o[p]=u.name[p];
    } else {
      ss=strchr(translate_letters[1],u.name[p]);
      if (ss)
        o[p]=locase(u.name[p]);
      else {
        if (((u.name[p]>=' ') && (u.name[p]<='/')) && (u.name[p]!=39))
          f=1;
        o[p]=u.name[p];
      }
    }
  }
  o[p++]=32;
  o[p++]='#';
  itoa(un,&o[p],10);
  return(o);
}


unsigned char *nam1(userrec *u1, unsigned int un, unsigned int sy)
{
  unsigned static char o[81];
  char s[10];

  strcpy(o,nam(u1,un));
  if (sy) {
    sprintf(s," @%u",sy);
    strcat(o,s);
  }
  return(o);
}

double nsl(void)
{
  double tlt,tlc,tot,tpl,tpd,dd,rtn;
  slrec xx;

  dd=timer();
  if (useron) {
    if (timeon>(dd+60.0))
      timeon -= 24.0*3600.0;
    tot=(dd-timeon);
    xx=syscfg.sl[actsl];
    tpl=((double) xx.time_per_logon) * 60.0;
    tpd=((double) xx.time_per_day) * 60.0;
    tlc = tpl - tot + (thisuser.extratime) + extratimecall;
    tlt = tpd - tot - ((double) thisuser.timeontoday) + (thisuser.extratime);

    tlt=(((tlc)<(tlt)) ? (tlc) : (tlt));
    if (tlt<0.0)
      tlt=0.0;
    if (tlt>32767.0)
      tlt=32767.0;
    rtn=tlt;
  } else {
    rtn=1.00;
  }
  ltime=0;
  if (syscfg.executetime) {
    tlt=time_event-dd;
    if (tlt<0.0)
      tlt += 24.0*3600.0;
    if (rtn>tlt) {
      rtn=tlt;
      ltime=1;
    }
    check_event();
    if (do_event)
      rtn=0.0;
  }
  if (rtn<0.0)
    rtn=0.0;
  if (rtn>32767.0)
    rtn=32767.0;
  return(rtn);
}


char *date(void)
{
  static char ds[9];
  struct date today;

  getdate(&today);
  sprintf(ds,"%02d/%02d/%02d",today.da_mon,today.da_day,today.da_year%100);
  return(ds);
}

char *times(void)
{
  static char ti[9];
  int h,m,s;
  double t;

  t=timer();
  h=(int) (t/3600.0);
  t-=((double) (h)) * 3600.0;
  m=(int) (t/60.0);
  t-=((double) (m)) * 60.0;
  s=(int) (t);
  sprintf(ti,"%02d:%02d:%02d",h,m,s);
  return(ti);
}


unsigned int finduser(unsigned char *s)
{
  int un;
  smalrec *sr;
  userrec u;
  unsigned char *ss;

  if (strcmp(s,"NEW")==0)
    return(-1);
  if (strcmp(s,"!-@NETWORK@-!")==0)
    return(-2);
  if (strcmp(s,"!-@REMOTE@-!")==0)
    return(-3);
  if (strncmp(s,"!=@",3)==0) {
    ss=s+strlen(s)-3;
    if (strcmp(ss,"@=!")==0) {
      strcpy(s,s+3);
      s[strlen(s)-3]=0;
      return(-4);
    }
  }

  if ((un=atoi(s))>0) {
    read_user(un,&u);
    if (u.inact & inact_deleted)
      return(0);
    return(un);
  }

  read_status();
  sr=(smalrec *) bsearch((void *)s,
			 (void *)smallist,
			 (size_t)status.users,
			 (size_t)sizeof(smalrec),
			 (int _Cdecl (*) (const void *, const void *))strcmp);
  if (sr==0L)
    return(0);
  else {
    read_user(sr->number,&u);
    if (u.inact & inact_deleted)
      return(0);
    else
      return(sr -> number);
  }
}


int access_conf(userrec *u, int sl, confrec *c)
{
  int ok=1;

  if (c->num<1)
    return(0);
  switch (c->sex) {
    case 0: if (u->sex!='M')
              return(0);
            break;
    case 1: if (u->sex!='F')
              return(0);
            break;
  }
  if ((sl<c->minsl) || (sl>c->maxsl))
    return(0);
  if ((u->dsl<c->mindsl) || (u->dsl>c->maxdsl))
    return(0);
  if ((u->age<c->minage) || (u->age>c->maxage))
    return(0);
  if (incom && (modem_speed<c->minbps))
    return(0);
  if ((u->ar & c->ar) != c->ar)
    return(0);
  if ((u->dar & c->dar) != c->dar)
    return(0);
  if ((c->status & conf_status_ansi) &&
      (!(u->sysstatus & sysstatus_ansi)))
    return(0);
  if ((c->status & conf_status_wwivreg) &&
      (u->wwiv_regnum<1))
    return(0);
  if ((c->status & conf_status_offline) &&
      (sl < 100))
    return(0);

  return(ok);
}


int access_sub(userrec *u, int sl, subboardrec *s)
{
  int ok=1;

  if (sl<s->readsl)
    ok=0;
  else if (u->age<(s->age&0x7f))
    ok=0;
  else if ((s->ar!=0) && (((u->ar) & (s->ar))==0))
    ok=0;
  else if ((s->anony & anony_ansi_only) && (!(u->sysstatus & sysstatus_ansi)))
    ok=0;

  return(ok);
}

int access_dir(userrec *u, int sl, directoryrec *d)
{
  int ok;

  ok=sl;
  ok=1;

  if (u->dsl<d->dsl)
    ok=0;
  else if (u->age<d->age)
    ok=0;
  else if ((d->dar) && ((d->dar & u->dar)==0))
    ok=0;
  else if ((d->mask & mask_offline) && (u->dsl < 100))
    ok=0;

  return(ok);
}

void addusub(usersubrec *ss1, int ns, int sub, char key)
{
  int last_num, last, i;

  last_num=0;
  for (last=0; last<ns; last++) {
    if (ss1[last].subnum==-1)
      break;
    if (ss1[last].subnum==sub)
      return;
    if (ss1[last].keys[0]==0)
      last_num=last+1;
  }

  if (last==ns)
    return;

  if (key) {
    ss1[last].subnum=sub;
    ss1[last].keys[0]=key;
  } else {
    for (i=last; i>last_num; i--)
      ss1[i]=ss1[i-1];
    ss1[last_num].subnum=sub;
    ss1[last_num].keys[0]=0;
  }
}

int setconf(unsigned int conftype, int which, int oldsub)
{
  int i,i1, ns, osub, dp, tp;
  confrec *c;
  usersubrec *ss1, s1;
  char *xdc, *xtc;

  dp=1;
  tp=0;

  switch(conftype) {
    case CONF_SUBS:
      ss1=usub;
      ns=num_subs;
      if (oldsub==-1)
        osub=usub[cursub].subnum;
      else
        osub=oldsub;
      xdc=dc;
      xtc=tc;
      xdc[0]='/';
      if (which==-1) {
        c=NULL;
      } else {
        if ((which<0) || (which>=subconfnum))
          return(1);
        c=&(subconfs[which]);
        if (!access_conf(&thisuser, actsl, c))
          return(1);
      }
      break;
    case CONF_DIRS:
      ss1=udir;
      ns=num_dirs;
      if (oldsub==-1)
        osub=udir[curdir].subnum;
      else
        osub=oldsub;
      xdc=dcd;
      xtc=dtc;
      xdc[0]='/';
      if (which==-1) {
        c=NULL;
      } else {
        if ((which<0) || (which>=dirconfnum))
          return(1);
        c=&(dirconfs[which]);
        if (!access_conf(&thisuser, actsl, c))
          return(1);
      }
      break;
    default:
      return(1);
  }

  memset(&s1, 0, sizeof(s1));
  s1.subnum=-1;

  for (i=0; i<ns; i++)
    ss1[i]=s1;

  if (c) {
    for (i=0; i<c->num; i++) {
      switch(conftype) {
        case CONF_SUBS:
          if (access_sub(&thisuser, actsl, (subboardrec *)&subboards[c->subs[i]])) {
            addusub(ss1, ns, c->subs[i], subboards[c->subs[i]].key);
          }
          break;
        case CONF_DIRS:
          if (access_dir(&thisuser, actsl, (directoryrec *)&directories[c->subs[i]])) {
            addusub(ss1, ns, c->subs[i], 0);
          }
          break;
      }
    }
  } else {
    switch(conftype) {
      case CONF_SUBS:
        for (i=0; i<subconfnum; i++) {
          if (access_conf(&thisuser, actsl, &(subconfs[i]))) {
            for (i1=0; i1<subconfs[i].num; i1++) {
              if (access_sub(&thisuser, actsl,
                             (subboardrec *)&subboards[subconfs[i].subs[i1]])) {
                addusub(ss1, ns, subconfs[i].subs[i1],
                        subboards[subconfs[i].subs[i1]].key);
              }
            }
          }
        }
        break;
      case CONF_DIRS:
        for (i=0; i<dirconfnum; i++) {
          if (access_conf(&thisuser, actsl, &(dirconfs[i]))) {
            for (i1=0; i1<dirconfs[i].num; i1++) {
              if (access_dir(&thisuser, actsl,
                             (directoryrec *)&directories[dirconfs[i].subs[i1]])) {
                addusub(ss1, ns, dirconfs[i].subs[i1], 0);
              }
            }
          }
        }
        break;
    }
  }

  if ((conftype==CONF_DIRS) && (ss1[0].subnum==0))
    i1=0;
  else
    i1=1;

  for (i=0; (i<ns) && (ss1[i].keys[0]==0) && (ss1[i].subnum!=-1); i++) {
    if (i1<100) {
      if (((i1 % 10)==0) && i1)
        xdc[dp++]=('0'+(i1/10));
    } else {
      if ((i1 % 100)==0)
        xtc[tp++]=('0'+(i1/100));
    }
    itoa(i1++,ss1[i].keys,10);
  }


  xdc[dp]=0;
  xtc[tp]=0;

  for (i1=0; (i1<ns) && (ss1[i1].subnum!=-1); i1++) {
    if (ss1[i1].subnum==osub)
      break;
  }
  if ((i1>=ns) || (ss1[i1].subnum==-1))
    i1=0;

  switch(conftype) {
    case CONF_SUBS: cursub=i1; break;
    case CONF_DIRS: curdir=i1; break;
  }

  return(0);
}

void setuconf(int conftype, int num, int oldsub)
{
  switch(conftype) {
    case CONF_SUBS:
      if ((num>=0) && (num<MAX_CONFERENCES) && (uconfsub[num].confnum!=-1)) {
        curconfsub=num;
        setconf(conftype, uconfsub[curconfsub].confnum, oldsub);
        return;
      }
      break;
    case CONF_DIRS:
      if ((num>=0) && (num<MAX_CONFERENCES) && (uconfdir[num].confnum!=-1)) {
        curconfdir=num;
        setconf(conftype, uconfdir[curconfdir].confnum, oldsub);
        return;
      }
      break;
  }
  setconf(conftype, -1, oldsub);
}

void changedsl(void)
{
  int i,i2;
  int ocurconfsub, ocurconfdir;
  userconfrec c1;

  ocurconfsub=uconfsub[curconfsub].confnum;
  ocurconfdir=uconfdir[curconfdir].confnum;
  topscreen();

  c1.confnum=-1;

  for (i=0; i<MAX_CONFERENCES; i++) {
    uconfsub[i]=c1;
    uconfdir[i]=c1;
  }

  i2=0;
  for (i=0; i<subconfnum; i++) {
    if (access_conf(&thisuser, actsl, &(subconfs[i]))) {
      c1.confnum=i;
      uconfsub[i2++]=c1;
    }
  }

  i2=0;
  for (i=0; i<dirconfnum; i++) {
    if (access_conf(&thisuser, actsl, &(dirconfs[i]))) {
      c1.confnum=i;
      uconfdir[i2++]=c1;
    }
  }

  for (curconfsub=0; (curconfsub<MAX_CONFERENCES) && (uconfsub[curconfsub].confnum!=-1); curconfsub++)
    if (uconfsub[curconfsub].confnum==ocurconfsub)
      break;
  if ((curconfsub>=MAX_CONFERENCES) || (uconfsub[curconfsub].confnum==-1))
    curconfsub=0;

  for (curconfdir=0; (curconfdir<MAX_CONFERENCES) && (uconfdir[curconfdir].confnum!=-1); curconfdir++)
    if (uconfdir[curconfdir].confnum==ocurconfdir)
      break;
  if ((curconfdir>=MAX_CONFERENCES) || (uconfdir[curconfdir].confnum==-1))
    curconfdir=0;

  if (okconf(&thisuser)) {
    setuconf(CONF_SUBS, curconfsub, -1);
    setuconf(CONF_DIRS, curconfdir, -1);
  } else {
    setconf(CONF_SUBS, -1, -1);
    setconf(CONF_DIRS, -1, -1);
  }

}


void isr(int un, unsigned char *name)
{
  int cp,i;
  unsigned char s[81];
  smalrec sr;

  cp=0;
  lock_status();
  while ((cp<status.users) && (strcmp(name,(smallist[cp].name))>0))
    ++cp;
  for (i=status.users; i>cp; i--)
    smallist[i]=smallist[i-1];
  strcpy(sr.name,name);
  sr.number=un;
  smallist[cp]=sr;
  sprintf(s,"%sNAMES.LST",syscfg.datadir);
  i=sh_open1(s,O_RDWR | O_BINARY | O_TRUNC);
  if (i<0) {
    printf(get_stringx(1,132),s);
    end_bbs(noklevel);
  }
  ++status.users;
  ++status.filechange[filechange_names];
  huge_xfer(i, smallist, sizeof(smalrec), status.users, 1);
  sh_close(i);
  save_status();
}


void dsr(unsigned char *name)
{
  int cp,i;
  unsigned char s[81];

  cp=0;
  lock_status();
  while ((cp<status.users) && (strcmp(name,(smallist[cp].name))!=0))
    ++cp;
  if (strcmp(name,(smallist[cp].name))) {
    save_status();
    sprintf(s,get_stringx(1,74),name);
    sl1(0,s);
    sl1(0,get_stringx(1,75));
    return;
  }
  for (i=cp; i<status.users-1; i++)
    smallist[i]=smallist[i+1];
  sprintf(s,"%sNAMES.LST",syscfg.datadir);
  i=sh_open(s,O_RDWR | O_BINARY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
  if (i<0) {
    printf(get_stringx(1,133),s);
    end_bbs(noklevel);
  }
  --status.users;
  ++status.filechange[filechange_names];
  huge_xfer(i, smallist, sizeof(smalrec), status.users, 1);
  sh_close(i);
  save_status();
}


void wait1(long l)
{
  long l1;

  l1 = timer1();

#ifndef __OS2__
  enable();
#endif
  while (labs(timer1()-l1)<l)
    giveup_timeslice();
}


void Wait(double d)
{
  wait1((long) (18.2*d));
}

double freek1(char *s)
{
  int d;

  d=cdir[0];
  if (s[1]==':')
    d=s[0];
  d=toupper(d)-'A'+1;
  return(freek(d));
}


int exist(char *s)
{
  int i;
  struct ffblk ff;

  i=findfirst(s,&ff,FA_HIDDEN);
  if (i)
    return(0);
  else
    return(1);
}


void add_ass(int i, char *ss)
{
  char s[81],s1[10];

  strcpy(s,"*** ");
  sysoplog(s);
  strcat(s,ss);
  sysoplog(s);
  itoa(i,s1,10);
  strcpy(s,get_stringx(1,76));
  strcat(s,s1);
  sysoplog(s);
  thisuser.ass_pts += i;
}


int find_interrupt(void)
{
#ifndef __OS2__
  long far *l;
  int i;

  for (i=0x60; i<0xf0; i++) {
    if ((i<0x70) || (i>0x77)) {
      l=MK_FP(0,(i*4));
      if (*l==0)
        return i;
    }
  }
#endif
  return(0);
}



/****************************************************************************/

void send_net(net_header_rec *nh, unsigned int *list, char *text, char *byname)
{
  int f;
  char s[100];
  long l1;

  sprintf(s,"%sP1%s",net_data,nete);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  sh_lseek(f,0L,SEEK_END);
  if (!list)
    nh->list_len=0;
  if (!text)
    nh->length=0;
  if (nh->list_len)
    nh->tosys=0;
  l1=nh->length;
  if (byname && *byname)
    nh->length += strlen(byname)+1;
  sh_write(f,(void *)nh,sizeof(net_header_rec));
  if (nh->list_len)
    sh_write(f,(void *)list,2*(nh->list_len));
  if (byname && *byname)
    sh_write(f,byname, strlen(byname)+1);
  if (nh->length)
    sh_write(f,(void *)text,l1);
  sh_close(f);
}

/****************************************************************************/

unsigned char dow(void)
/* returns day of week, 0=Sun, 6=Sat */
{
#ifdef __OS2__
  DATETIME dt;

  DosGetDateTime(&dt);
  return dt.weekday;
#else
  unsigned char ch;

  _AH=0x2a;
  geninterrupt(0x21);
  ch=_AL;

  return(ch);
#endif
}

/****************************************************************************/

#define MT_DESQVIEW 0x01
#define MT_WINDOWS  0x02
#define MT_OS2      0x04


void detect_multitask(void)
{
#ifdef __OS2__
  multitasker |= MT_OS2;
#else
  get_dos_version();
  get_win_version();
  get_dv_version();
#endif
}


int get_dos_version(void)
{
#ifdef __OS2__
  return(0);
#else
  _AX=0x3000;
  geninterrupt(0x21);
  if(_AX%256 >=10) {
    multitasker |= MT_OS2;
  }
  return(_AX);
#endif
}


int get_dv_version(void)
{
#ifdef __OS2__
  return 0;
#else
  int v;

  _AX=0x2b01;
  _CX=0x4445;
  _DX=0x5351;
  geninterrupt(0x21);
  if (_AL == 0xff) {
    return 0;
  } else {
    v=_BX;
    multitasker |= MT_DESQVIEW;
    return v;
  }
#endif
}


int get_win_version(void)
{
#ifdef __OS2__
  return 0;
#else
  int v=0;

/*
 *  push  bp
 *  push  es
 *  push  bx
 */
  __emit__(0x55, 0x06, 0x53);

  _AX=0x352f;
  geninterrupt(0x21);
  _AX=_ES;

  if (_AX | _BX) {

    _AX=0x1600;
    geninterrupt(0x2f);

    v=_AX;
    if (v%256<=1)
      v=0;
  }

/*
 * pop bx
 * pop es
 * pop bp
 */
  __emit__(0x5b, 0x07, 0x5d);

  if (v!=0)
    multitasker |= MT_WINDOWS;
  return(v);
#endif
}

void begin_crit(void)
{
#ifndef __OS2__
  if (multitasker & MT_DESQVIEW) {
    _AX=0x101a;
    geninterrupt(0x15);
    _AX=0x101b;
    geninterrupt(0x15);
    _AX=0x1025;
    geninterrupt(0x15);
  }
#endif
}


void end_crit(void)
{
#ifndef __OS2__
  if (multitasker & MT_DESQVIEW) {
    _AX=0x101a;
    geninterrupt(0x15);
    _AX=0x101c;
    geninterrupt(0x15);
    _AX=0x1025;
    geninterrupt(0x15);
  }
#endif
}


void giveup_timeslice(void)
{
#ifdef __OS2__
  DosSleep(0);
#else
  if (multitasker) {
    switch (multitasker) {
      case 1 :/* outs("DESQView pause"); */
        dv_pause();
      break;
      case 2 :/* outs("Windows pause"); */
        win_pause();
      break;
      case 3 :/* outs("Win or DV pause"); */
      /* dv_pause();
         win_pause(); */
      /* Enable one of the above.  I believe the logical choice
         would be the pause for the multitasker loaded last.
         At this point neither is enabled when both Windows and
         DESQView are found.  I believe that pausing both would
         be a bad idea.  Let me know if you run using both
         multitaskers. */
      break;
      case 4:  /* outs("OS/2"); */
        _AX=0x1680;
        geninterrupt(0x2f);
        break;
      case 5:
      case 6:
      case 7:
        /* delay(17); */
        win_pause();
      break;
     default: /* pl("Unrecognized multitasker"); */
       break;
    }
  } else {
    /* delay(17); */
  }
#endif
  if (inst_msg_waiting())
    process_inst_msgs();
}


void dv_pause(void)
{
#ifndef __OS2__
  _AX=0x101a;
  geninterrupt(0x15);
  _AX=0x1000;
  geninterrupt(0x15);
  _AX=0x1025;
  geninterrupt(0x15);
#endif
}


void win_pause(void)
{
#ifndef __OS2__
  __emit__(0x55, 0xb8, 0x80, 0x16, 0xcd, 0x2f, 0x5d);
/*
 * push bp
 * mov ax,0x1680
 * int 0x2f
 * pop bp
 */
#endif
}


/****************************************************************************/


static int statusfile=-1;

void get_status(int mode, int lock)
{
  char s[81];
  char fc[7];
  int i,i1,f, onn;
  long l;

  if (statusfile<0) {
    sprintf(s,"%sSTATUS.DAT",syscfg.datadir);
    if (lock)
      statusfile=sh_open1(s,O_RDWR | O_BINARY);
    else
      statusfile=sh_open1(s,O_RDONLY | O_BINARY);
  } else {
    lseek(statusfile, 0L, SEEK_SET);
  }
  if (statusfile<0) {
    if(!mode) {
      printf(get_stringx(1,132),s);
      end_bbs(noklevel);
    } else {
      sysoplog(get_stringx(1,134));
    }
  } else {
    l=status.qscanptr;
    for (i=0; i<7; i++)
      fc[i]=status.filechange[i];

    read(statusfile,(void *)(&status), sizeof(statusrec));
    if (!lock)
      statusfile=sh_close(statusfile);

    if (l != status.qscanptr) {
      if (sub_dates) {
        /* kill subs cache */
        for (i1=0; i1<num_subs; i1++) {
          sub_dates[i1]=0L;
        }
      }
      c_sub=0;
      subchg=1;
      gatfn[0]=0;
    }
    for (i=0; i<7; i++) {
      if (fc[i]!=status.filechange[i]) {
        switch(i) {
          case filechange_names: /* re-read names.lst */
            if (smallist) {
              sprintf(s,"%sNAMES.LST",syscfg.datadir);
              f=sh_open1(s,O_RDONLY | O_BINARY);
              if (f>=0) {
                huge_xfer(f, smallist, sizeof(smalrec), status.users, 0);
                sh_close(f);
              }
            }
            break;
          case filechange_upload: /* kill dirs cache */
            if (dir_dates) {
              for (i1=0; i1<num_dirs; i1++) {
                dir_dates[i1]=0L;
              }
            }
            c_dir=0;
            break;
          case filechange_posts:
            subchg=1;
            gatfn[0]=0;
            break;
          case filechange_email:
            emchg=1;
            mailcheck=0;
            break;
          case filechange_net:
            onn=net_num;
            zap_bbs_list();
            for (i1=0; i1<net_num_max; i1++) {
              set_net_num(i1);
              zap_call_out_list();
              zap_contacts();
            }
            set_net_num(onn);
            break;
        }
      }
    }
  }
}

void lock_status(void)
{
  get_status(1, 1);
}

void read_status(void)
{
  get_status(1, 0);
}

void save_status(void)
{
  char s[81];

  if (statusfile<0) {
    sprintf(s,"%sSTATUS.DAT",syscfg.datadir);
    statusfile=sh_open1(s,O_RDWR | O_BINARY);
  } else {
    lseek(statusfile, 0L, SEEK_SET);
  }

  if (statusfile<0) {
    sysoplog(get_stringx(1,135));
  } else {
    sh_write(statusfile, (void *)(&status), sizeof(statusrec));
    statusfile=sh_close(statusfile);
  }
}


/****************************************************************************/

/*
 * Returns 1 if a message waiting for this instance, 0 otherwise.
 */

int inst_msg_waiting(void)
{
  unsigned char s[81];
  int i;
  long l;

  if (in_extern || in_fsed || !iia || !echo)
    return(0);

  l=timer1();
  if (labs(l-last_iia)<iia)
    return(0);

  sprintf(s,"%sMSG*.%3.3d",syscfg.datadir,instance);
  i=exist(s);
  if (!i)
    last_iia=l;

  return(i);
}

/****************************************************************************/

/*
 * Sets inter-instance availability on/off, for inter-instance messaging.
 */

void setiia(int poll_ticks)
{
  iia=poll_ticks;
}

/****************************************************************************/

/* Don't ask me WHY this needs to be significantly less than 65534, but it
 * does.  65530 didn't work correctly.  I suspect it has something to do
 * with the alignment of huge pointers within the 64k segment.
 */
#define MAX_XFER 61440 /* 60 * 1024 */

long huge_xfer(int fd, void huge *buf, unsigned sz, unsigned nel, int wr)
{
  long nxfr=0, len=((long)sz) * ((long)nel);
  char huge *xbuf = (char huge *)buf;
  unsigned cur,cur1;

  while (len>0) {

    if (len>MAX_XFER)
      cur=MAX_XFER;
    else
      cur=len;

    if (wr)
      cur1=sh_write(fd,(char *)xbuf,cur);
    else
      cur1=sh_read(fd,(char *)xbuf,cur);

    if (cur1!=65535) {
      len  -= cur1;
      nxfr += cur1;
      xbuf  = ((char huge *)buf) + nxfr;
      /* not xbuf += cur1 due to bug in certain version of compiler */
    }

    if (cur1!=cur)
      break;
  }

  return(nxfr);
}
/****************************************************************************/


char *stripfn(char *fn)
{
  static char ofn[15];
  int i,i1;
  char s[81];

  i1=-1;
  for (i=0; i<strlen(fn); i++)
    if ((fn[i]=='\\') || (fn[i]==':') || (fn[i]=='/'))
      i1=i;
  if (i1!=-1)
    strcpy(s,&(fn[i1+1]));
  else
    strcpy(s,fn);
  for (i=0; i<strlen(s); i++)
    if ((s[i]>='A') && (s[i]<='Z'))
      s[i]=s[i]-'A'+'a';
  i=0;
  while (s[i]!=0) {
    if (s[i]==32)
      strcpy(&s[i],&s[i+1]);
    else
      ++i;
  }
  strcpy(ofn,s);
  return(ofn);
}


void stripfn1(char *fn)
{
  int i,i1;
  char s[81],s1[81];

  i1=0;
  for (i=0; i<strlen(fn); i++)
    if ((fn[i]=='\\') || (fn[i]==':') || (fn[i]=='/'))
      i1=i;
  strcpy(s1,fn);
  if (i1) {
    strcpy(s,&(fn[i1+1]));
    s1[i1+1]=0;
  } else {
    strcpy(s,fn);
    s1[0]=0;
  }

  for (i=0; i<strlen(s); i++)
    if ((s[i]>='A') && (s[i]<='Z'))
      s[i]=s[i]-'A'+'a';
  i=0;
  while (s[i]!=0) {
    if (s[i]==32)
      strcpy(&s[i],&s[i+1]);
    else
      ++i;
  }
  strcat(s1,s);
  strcpy(fn,s1);
}


