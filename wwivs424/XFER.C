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



#define SETREC(f,i)  sh_lseek(f,((long) (i))*((long)sizeof(uploadsrec)),SEEK_SET);

/* number of dots for searching */
#define DOTS 5

/* color for frame */
#define FRAME 7

/* How far to indent extended descriptions */
#define INDENTION 24

/* the archive type to use */
#define ARC_NUMBER 0

int foundany;

typedef struct {
  char name[13];
  long offset; /* of ext_desc_type */
} ext_desc_rec;

static int ed_num, ed_got;
static ext_desc_rec *ed_info;

void zap_ed_info(void)
{
  if (ed_info) {
    bbsfree(ed_info);
    ed_info = NULL;
  }
  ed_num=0;
  ed_got=0;
}

void get_ed_info(void)
{
  int f;
  long l,l1;
  ext_desc_type ed;

  if (ed_got)
    return;

  zap_ed_info();
  ed_got=1;

  if (!numf)
    return;

  l=0;
  f=sh_open1(edlfn,O_RDONLY | O_BINARY);
  if (f>0) {
    l1=filelength(f);
    if (l1>0) {
      ed_info=malloca((long)numf*sizeof(ext_desc_rec));
      if (ed_info==NULL) {
        sh_close(f);
        return;
      }
      ed_num=0;
      while ((l<l1) && (ed_num<numf)) {
        sh_lseek(f,l,SEEK_SET);
        if (sh_read(f,&ed,sizeof(ext_desc_type)) == sizeof(ext_desc_type)) {
          strcpy(ed_info[ed_num].name,ed.name);
          ed_info[ed_num].offset=l;
          l += (long) ed.len + sizeof(ext_desc_type);
          ed_num++;
        }
      }
      if (l<l1)
        ed_got=2;
    }
    f=sh_close(f);
  }
}

unsigned long bytes_to_k(unsigned long b)
{
  return((unsigned long)((b+1023)/1024));
}


int check_batch_queue(char *fn)
{
  int i;

  for (i=0; i<numbatch; i++) {
    if (strcmp(fn,batch[i].filename)==0)
      if (batch[i].sending)
        return(1);
      else
        return(-1);
  }

  return(0);
}


int check_ul_event(int dn, uploadsrec *u)
{
  char s[161],s1[10];

  if (syscfg.upload_c[0]) {
    sprintf(s1,"%d",syscfgovr.primaryport);
    stuff_in(s,syscfg.upload_c,create_chain_file(),
             directories[dn].path,stripfn(u->filename),s1,"");
    extern_prog(s, sysinfo.spawn_opts[4]);
    sprintf(s,"%s%s",directories[dn].path, stripfn(u->filename));
    if (!exist(s)) {
      sprintf(s,get_stringx(1,40),
              u->filename, directories[dn].name);
      sysoplog(s);
      outstr(u->filename);
      pl(get_string(727));
      return(1);
    }
  }

  return(0);
}

char (*devices)[9];
int num_devices;

#ifdef __OS2__
#define NUM_OS2_DEVICES 17
static char *OS2DeviceNames[] =
  {
    "KBD$", "PRN", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6",
    "COM7", "COM8", "LPT1", "LPT2", "LPT3", "CLOCK$", "SCREEN$",
    "POINTER$", "MOUSE$"
  };
#endif

void finddevs(char (*devs)[9], int *count)
{
#ifdef __OS2__
  int x;

  *count = NUM_OS2_DEVICES;
  if (devs)
    for (x = 0; x < NUM_OS2_DEVICES; x++)
      strcpy(devs[x], OS2DeviceNames[x]);
#else
  char s[9];
  int i;
  char far *ss;
  long far *l;
  union REGS regs;
  struct SREGS sregs;

  *count=0;
  regs.x.ax=0x5200;
  int86x(INT_REAL_DOS, &regs, &regs, &sregs);
  ss=MK_FP(sregs.es, regs.x.bx);
  ss += 34;
  l=(long far *) ss;

  while (((unsigned short) l) != 0xffff) {
    ss=(char far *) l;
    ss += 10;
    if (*ss>32) {
      strncpy(s,ss,8);
      s[8]=0;
      for (i=7; i; i--)
        if (s[i]==' ')
          s[i]=0;
        else
          break;
      if (devs) {
        strcpy(devs[*count],s);
      }
      ++(*count);
    }
    l=(long far *) *l;
  }
#endif
}

void find_devices(void)
{
  finddevs(NULL, &num_devices);
  devices=malloca((num_devices+2)*9);
  finddevs(devices,&num_devices);
}

int okfn(char *s)
{
  int i,l;
  unsigned char ch;

  l=strlen(s);
  if ((s[0]=='-') || (s[0]==' ') || (s[0]=='.') || (s[0]=='@'))
    return(0);
  for (i=0; i<l; i++) {
    ch=s[i];
    if ((ch==' ') || (ch=='/') || (ch=='\\') || (ch==':') ||
        (ch=='>') || (ch=='<') || (ch=='|')  || (ch=='+') ||
        (ch==',') || (ch==';') || (ch=='^')  || (ch=='\"') ||
        (ch=='\'') || (ch>126))
      return(0);
  }

  for (i=0; i<num_devices; i++) {
    l=strlen(devices[i]);
    if (strncmp(devices[i],s,l)==0) {
      if ((s[l]==0) || (s[l]=='.') || (l==8))
        return(0);
    }
  }
  return(1);
}

void print_devices(void)
{
  int i;
  for (i=0; i<num_devices; i++)
    pl(devices[i]);
}

char *exts[] = {"", ".COM", ".EXE", ".BAT", 0};

char *make_abs_cmd(char *out)
{
  char s[161],s1[161],s2[161],*ss,*ss1;
  int i;

  strcpy(s1,out);

  if (s1[1]==':') {
    if (s1[2]!='\\') {
      getcurdir(upcase(s1[0])-'A'+1, s);
      if (s[0])
        sprintf(s1,"%c:\\%s\\%s", s1[0], s, out+2);
      else
        sprintf(s1,"%c:\\%s",s1[0], out+2);
    }
  } else if (s1[0]=='\\') {
    sprintf(s1,"%c:%s", cdir[0], out);
  } else {
    strcpy(s2,s1);
    strtok(s2," \t");
    if (strchr(s2,'\\')) {
      sprintf(s1,"%s\\%s", cdir, out);
    }
  }

  ss=strchr(s1,' ');
  if (ss) {
    *ss=0;
    sprintf(s2," %s",ss+1);
  } else {
    s2[0]=0;
  }
  for (i=0; exts[i]; i++) {
    if (i==0) {
      ss1=strrchr(s1,'\\');
      if (!ss1)
        ss1=s1;
      if (strchr(ss1,'.')==0)
        continue;
    }
    sprintf(s,"%s%s",s1,exts[i]);
    if (s1[1]==':') {
      if (exist(s)) {
        sprintf(out,"%s%s",s,s2);
        goto got_cmd;
      }
    } else {
      if (exist(s)) {
        sprintf(out,"%s\\%s%s", cdir, s, s2);
        goto got_cmd;
      } else {
        ss1=searchpath(s);
        if (ss1) {
          sprintf(out, "%s%s", ss1, s2);
          goto got_cmd;
        }
      }
    }
  }

  sprintf(out, "%s\\%s%s", cdir, s1, s2);

got_cmd:
  return(out);
}

void get_arc_cmd(char *out, char *arcfn, int cmd, char *ofn)
{
  char *ss,s[161];
  int i;

  out[0]=0;
  ss=strrchr(arcfn,'.');
  if (ss==NULL)
    return;
  ++ss;
  for (i=0; i<4; i++)
    if (stricmp(ss,syscfg.arcs[i].extension)==0) {
      switch(cmd) {
        case 0: strcpy(s,syscfg.arcs[i].arcl); break;
        case 1: strcpy(s,syscfg.arcs[i].arce); break;
        case 2: strcpy(s,syscfg.arcs[i].arca); break;
      }
      if (s[0]==0)
        return;
      stuff_in(out,s,arcfn,ofn,"","","");
      make_abs_cmd(out);
      return;
    }

}


int list_arc_out(char *fn, char *dir)
{
  char s[161],s1[81],s2[81], rmfn[81];
  int i=0;

  rmfn[0]=0;

  sprintf(s1,"%s%s",dir,fn);
  if (directories[udir[curdir].subnum].mask & mask_cdrom) {
    sprintf(s1,"%s%s",syscfgovr.tempdir,fn);
    if (!exist(s1)) {
      sprintf(s2,"%s%s",dir,fn);
      copy_file(s2,s1);
      strcpy(rmfn, s1);
    }
  }
  get_arc_cmd(s,s1,0,"");
  if (!okfn(fn))
    s[0]=0;
  if (exist(s1) && (s[0]!=0)) {
    nln(2);
    outstr(get_string(728));
    pl(fn);
    nl();
    i=extern_prog(s, sysinfo.spawn_opts[10]);
    nl();
  } else {
    nl();
    outs(get_string(729));
    pl(fn);
    nl();
    i=0;
  }

  if (rmfn[0])
    unlink(rmfn);

  return(i);
}


int ratio_ok(void)
{
  int ok=1;
  char s[250];

  if (!(thisuser.exempt & exempt_ratio))
    if ((syscfg.req_ratio>0.0001) && (ratio()<syscfg.req_ratio)) {
      ok=0;
      nln(2);
      sprintf(s,"%s %-5.3f.  %s %-5.3f %s.",
              get_string(730), ratio(),
              get_string(731), syscfg.req_ratio,
              get_string(732));
      pl(s);
      nl();
    }

  if (!(thisuser.exempt & exempt_post))
    if ((syscfg.post_call_ratio>0.0001) && (post_ratio()<syscfg.post_call_ratio)) {
      ok=0;
      nln(2);
      sprintf(s,"%s %-5.3f.  %s %-5.3f %s.",
              get_string(733), post_ratio(),
              get_string(731), syscfg.post_call_ratio,
              get_string(732));
      pl(s);
      nl();
    }


  return(ok);
}


int dcs(void)
{
  if (cs())
    return(1);
  if (thisuser.dsl>=100)
    return(1);
  else
    return(0);
}


void dliscan1(int dn)
{
  int i,f;
  uploadsrec u;
  long l;

  sprintf(dlfn,"%s%s.DIR",syscfg.datadir,directories[dn].filename);
  f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  i=filelength(f)/sizeof(uploadsrec);
  if (i==0) {
    memset(&u, 0, sizeof(uploadsrec));
    strcpy(u.filename,"|MARKER|");
    time(&l);
    u.daten=l;
    SETREC(f,0);
    sh_write(f,(void *)&u,sizeof(uploadsrec));
  } else {
    SETREC(f,0);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    if (strcmp(u.filename,"|MARKER|")) {
      numf=u.numbytes;
      memset(&u, 0, sizeof(uploadsrec));
      strcpy(u.filename,"|MARKER|");
      time(&l);
      u.daten=l;
      u.numbytes = numf;
      SETREC(f,0);
      sh_write(f, &u, sizeof(uploadsrec));
    }
  }
  f=sh_close(f);
  numf=u.numbytes;
  this_date = u.daten;
  dir_dates[dn]=this_date;

  sprintf(edlfn,"%s%s.EXT",syscfg.datadir,directories[dn].filename);
  zap_ed_info();
}

void dliscan_hash(int dn)
{
  char s[81];
  int i,f;
  uploadsrec u;

  if ((dn>=num_dirs) || (dir_dates[dn]))
    return;

  sprintf(s,"%s%s.DIR",syscfg.datadir,directories[dn].filename);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0) {
    time((long *)&(dir_dates[dn]));
    return;
  }
  i=filelength(f)/sizeof(uploadsrec);
  if (i<1) {
    time((long *)&(dir_dates[dn]));
  } else {
    SETREC(f,0);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    if (strcmp(u.filename,"|MARKER|")==0) {
      dir_dates[dn]=u.daten;
    } else {
      time((long *)&(dir_dates[dn]));
    }
  }
  sh_close(f);
}


void dliscan(void)
{
  dliscan1(udir[curdir].subnum);
}


void add_extended_description(char *fn, char *desc)
{
  ext_desc_type ed;
  int f;

  strcpy(ed.name,fn);
  ed.len=strlen(desc);
  f=sh_open(edlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  sh_lseek(f,0L,SEEK_END);
  sh_write(f,&ed,sizeof(ext_desc_type));
  sh_write(f,desc,ed.len);
  f=sh_close(f);
  zap_ed_info();
}


void delete_extended_description(char *fn)
{
  ext_desc_type ed;
  long r,w,l1,f;
  char *ss=NULL;

  r=w=0;
  if ((ss=malloca(10240L))==NULL)
    return;
  f=sh_open(edlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  l1=filelength(f);
  while (r<l1) {
    sh_lseek(f,r,SEEK_SET);
    sh_read(f,&ed,sizeof(ext_desc_type));
    if (ed.len<10000) {
      sh_read(f,ss,ed.len);
      if (strcmp(fn,ed.name)) {
        if (r!=w) {
          sh_lseek(f,w,SEEK_SET);
          sh_write(f,&ed,sizeof(ext_desc_type));
          sh_write(f,ss,ed.len);
        }
        w +=(sizeof(ext_desc_type) + ed.len);
      }
    }
    r += (sizeof(ext_desc_type) + ed.len);
  }
  chsize(f,w);
  f=sh_close(f);
  bbsfree(ss);
  zap_ed_info();
}


char *read_extended_description(char *fn)
{
  ext_desc_type ed;
  long l,l1;
  char *ss=NULL;
  int f,i;

  get_ed_info();

  if (ed_got && ed_info) {
    for (i=0; i<ed_num; i++) {
      if (strcmp(fn, ed_info[i].name)==0) {
        f=sh_open1(edlfn, O_RDONLY|O_BINARY);
        if (f<0) {
          return(NULL);
        }
        sh_lseek(f, ed_info[i].offset, SEEK_SET);
        l=sh_read(f, &ed, sizeof(ext_desc_type));
        if ((l==sizeof(ext_desc_type)) && (strcmp(fn, ed.name)==0)) {
          ss=malloca((long) ed.len+10);
          if (ss) {
            sh_read(f,ss,ed.len);
            ss[ed.len]=0;
          }
          f=sh_close(f);
          return(ss);
        } else {
          zap_ed_info();
          f=sh_close(f);
          break;
        }
      }
    }
  }

  if (ed_got!=1) {
    l=0;
    f=sh_open1(edlfn,O_RDONLY | O_BINARY);
    if (f>0) {
      l1=filelength(f);
      while (l<l1) {
        sh_lseek(f,l,SEEK_SET);
        l += (long) sh_read(f,&ed,sizeof(ext_desc_type));
        if (strcmp(fn,ed.name)==0) {
          ss=malloca((long) ed.len+10);
          if (ss) {
            sh_read(f,ss,ed.len);
            ss[ed.len]=0;
          }
          f=sh_close(f);
          return(ss);
        } else
          l += (long) ed.len;
      }
      f=sh_close(f);
    }
  }
  return(NULL);
}



void print_extended(char *fn, int *abort, unsigned char numlist, int indent)
{
  char *ss;
  int next=0;
  unsigned char numl=0;
  int cpos=0;
  char ch,s[81];
  int i;

  ss=read_extended_description(fn);
  if (ss) {
    if (indent!=2)
      ch=10;
    while ((ss[cpos]) && (!(*abort)) && (numl<numlist)) {
      if (ch==10) {
        if (indent==1) {
          for (i=0; i<INDENTION; i++) {
            if ((i==12) || (i==18))
              s[i]=(okansi() ? 'º' : '|');
            else
              s[i]=32;
          }
          s[INDENTION]=0;
          ansic((thisuser.sysstatus & sysstatus_extra_color)
            && (!(*abort)) ? FRAME : 0);
          osan(s,abort,&next);
          if ((thisuser.sysstatus & sysstatus_extra_color) && (!(*abort)))
            ansic(1);
        } else {
          if (indent==2) {
            for (i=0; i<13; i++)
              s[i]=32;
            s[13]=0;
            osan(s,abort,&next);
            if ((thisuser.sysstatus & sysstatus_extra_color) && (!(*abort)))
              ansic(2);
          }
        }
      }
      outchr(ch=ss[cpos++]);
      checka(abort,&next);
      if (ch==10)
        ++numl;
      else
        if ((ch!=13) && (WhereX()>=78)) {
          osan("\r\n",abort,&next);
          ch=10;
        }
    }
    if (WhereX())
      nl();
    bbsfree(ss);
  } else {
    if (WhereX())
      nl();
  }
}



void align(char *s)
{
  char f[40],e[40],s1[20],*s2;
  int i,i1,i2;

  i1=0;
  if (s[0]=='.')
    i1=1;
  for (i=0; i<strlen(s); i++)
    if ((s[i]=='\\') || (s[i]=='/') || (s[i]==':') || (s[i]=='<') ||
      (s[i]=='>') || (s[i]=='|'))
      i1=1;
  if (i1) {
    strcpy(s,"        .   ");
    return;
  }
  s2=strrchr(s,'.');
  if ((s2==NULL) || (strrchr(s,'\\')>s2)) {
    e[0]=0;
  } else {
    strcpy(e,&(s2[1]));
    e[3]=0;
    s2[0]=0;
  }
  strcpy(f,s);

  for (i=strlen(f); i<8; i++)
    f[i]=32;
  f[8]=0;
  i1=0;
  i2=0;
  for (i=0; i<8; i++) {
    if (f[i]=='*')
      i1=1;
    if (f[i]==' ')
      i2=1;
    if (i2)
      f[i]=' ';
    if (i1)
      f[i]='?';
  }

  for (i=strlen(e); i<3; i++)
    e[i]=32;
  e[3]=0;
  i1=0;
  for (i=0; i<3; i++) {
    if (e[i]=='*')
      i1=1;
    if (i1)
      e[i]='?';
  }

  for (i=0; i<12; i++)
    s1[i]=32;
  strcpy(s1,f);
  s1[8]='.';
  strcpy(&(s1[9]),e);
  strcpy(s,s1);
  for (i=0; i<12; i++)
    s[i]=upcase(s[i]);
}


int compare(char *s1, char *s2)
{
  int ok,i;

  ok=1;
  for (i=0; i<12; i++)
    if ((s1[i]!=s2[i]) && (s1[i]!='?') && (s2[i]!='?'))
      ok=0;
  return(ok);
}


void printinfo(uploadsrec *u, int *abort)
{
  char s[85],s1[40],s2[81];
  int i,next,fc;

  fc=thisuser.sysstatus & sysstatus_extra_color;
  if (titled!=0)
    printtitle(abort);
  if ((tagging==0) && (!x_only))
    return;
  if ((tagging == 1) && !(thisuser.sysstatus & sysstatus_no_tag)
    && (!x_only)) {
    if (!filelist) {
      filelist=(tagrec *)malloca(50 * sizeof(tagrec));
      tagptr=0;
    }
    if (filelist) {
      filelist[tagptr].u=*u;
      filelist[tagptr].directory= udir[curdir].subnum;
      filelist[tagptr].dir_mask=  directories[udir[curdir].subnum].mask;
      tagptr++;
      sprintf(s,"\r%d%2d%d%c",
        (check_batch_queue(filelist[tagptr-1].u.filename)) ? 6 : 0,
        tagptr,fc ? FRAME : 0, okansi() ? 'º' : '|' );
      osan(s,abort,&next);
    }
  } else
    if (!x_only)
      outstr("\r");
  if (fc)
    ansic(1);
  strncpy(s,u->filename,8);
  s[8]=0;
  osan(s,abort,&next);
  strncpy(s,&((u->filename)[8]),4);
  s[4]=0;
  if (fc)
    ansic(1);
  osan(s,abort,&next);
  ansic(fc ? FRAME : 0);
  osan((okansi() ? "º" : "|"),abort,&next);

  ltoa(bytes_to_k(u->numbytes),s1,10);
  strcat(s1,"k");

  if (!(directories[udir[curdir].subnum].mask & mask_cdrom)) {
    strcpy(s2,directories[udir[curdir].subnum].path);
    strcat(s2,u->filename);
    if (!exist(s2))
      strcpy(s1,get_string(741));
  }

  for (i=0; i<5-(int)strlen(s1); i++)
    s[i]=32;
  s[i]=0;
  strcat(s,s1);
  if (fc)
    ansic(2);
  osan(s,abort,&next);

  if ((tagging == 1) && !(thisuser.sysstatus & sysstatus_no_tag)
    && (!x_only)) {
    ansic(fc ? FRAME : 0);
    osan((okansi() ? "º" : "|"),abort,&next);
    sprintf(s1,"%d",u->numdloads);

    for (i=0; i<4-(int)strlen(s1); i++)
      s[i]=32;
    s[i]=0;
    strcat(s,s1);
    if (fc)
      ansic(2);
    osan(s,abort,&next);
  }

  ansic(fc ? FRAME : 0);
  osan((okansi() ? "º" : "|"),abort,&next);
  sprintf(s,"%d%s",
    (u->mask & mask_extended) ? 1 : 2, u->description);
  if ((tagging) && !(thisuser.sysstatus & sysstatus_no_tag)
    && (!x_only)) {
    plal(s,thisuser.screenchars-28,abort);
  } else {
    plal(s,strlen(s),abort);
    if ((!*abort) && (thisuser.num_extended) && (u->mask & mask_extended))
      print_extended(u->filename,abort,thisuser.num_extended,1);
  }
  if (!(*abort)) {
    ++num_listed;
  } else {
    tagptr=0;
    tagging=0;
  }
}


void printtitle(int *abort)
{
  char s[81], *ss;

  if (x_only) {
    nl();
    ss="";
  } else
    ss="\r";
  if ((lines_listed >= screenlinest - 7) && (!x_only)
    && !(thisuser.sysstatus & sysstatus_no_tag) && filelist && num_listed) {
    tag_files();
    if (tagging==0)
      return;
  }
  sprintf(s,"%s%s - #%s, %d %s.",ss,directories[udir[curdir].subnum].name,
                                udir[curdir].keys,numf, get_string(742));
  ansic(thisuser.sysstatus & sysstatus_extra_color ? FRAME : 0);
  if (((num_listed==0) && (tagptr==0)) || (tagging==0)
    || ((thisuser.sysstatus & sysstatus_no_tag) && (num_listed==0))) {
    if (okansi())
      npr("%s%s\r\n",ss,get_string(1311));
    else
      npr("%s%s\r\n",ss,get_string(1312));
  } else if (lines_listed) {
    if ((titled!=2) && (tagging == 1)
      && !(thisuser.sysstatus & sysstatus_no_tag)) {
      if (okansi())
        npr("%s%s\r\n",ss,get_string(1313));
      else
        npr("%s%s\r\n",ss,get_string(1314));
    } else {
      if (((thisuser.sysstatus & sysstatus_no_tag) || (tagging==2))
        && (num_listed!=0)) {
        ansic(thisuser.sysstatus & sysstatus_extra_color ? FRAME : 0);
        if (okansi())
          npr("%s%s\r\n",ss,get_string(1315));
        else
          npr("%s%s\r\n",ss,get_string(1316));
      }
    }
  }
  if (thisuser.sysstatus & sysstatus_extra_color)
    ansic(2);
  pla(s,abort);
  if ((tagging == 1) && !(thisuser.sysstatus & sysstatus_no_tag)
    && (!x_only)) {
    ansic(thisuser.sysstatus & sysstatus_extra_color ? FRAME : 0);
    if (okansi())
      npr("\r%s\r\n",get_string(1317));
    else
      npr("\r%s\r\n",get_string(1318));
  } else {
    ansic(thisuser.sysstatus & sysstatus_extra_color ? FRAME : 0);
    if (okansi())
      npr("\r%s\r\n",get_string(1319));
    else
      if (x_only)
        npr("%s\r\n",get_string(1320));
      else
        npr("\r%s\r\n",get_string(1321));
  }
  titled=0;
}


void file_mask(char *s)
{
  nl();
  helpl=9;
  prt(2,get_string(743));
  input(s,12);
  if (s[0]==0)
    strcpy(s,"*.*");
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  align(s);
  nl();
}


void listfiles(void)
{
  char s[81];
  int i,abort,next=0,f;
  uploadsrec u;

  dliscan();
  file_mask(s);
  abort=0;
  num_listed=0;
  titled=1;
  lines_listed=0;
  f=sh_open1(dlfn,O_RDONLY | O_BINARY);
  for (i=1; (i<=numf) && (!abort) && (!hangup) && (tagging!=0); i++) {
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    if (compare(s,u.filename)) {
      f=sh_close(f);
      printinfo(&u, &abort);
      f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    } else if (!empty())
      checka(&abort,&next);
  }
  f=sh_close(f);
  endlist(1);
}


void nscandir(int d, int *abort)
{
  int i,od,next=0,f;
  uploadsrec u;

  if ((dir_dates[udir[d].subnum]) && (dir_dates[udir[d].subnum]<nscandate))
    return;

  od=curdir;
  curdir=d;
  dliscan();
  if (this_date>=nscandate) {
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    for (i=1; (i<=numf) && (!(*abort)) && (!hangup) && (tagging!=0); i++) {
      checkhangup();
      SETREC(f,i);
      sh_read(f,(void *)&u,sizeof(uploadsrec));
      if (u.daten>=nscandate) {
        f=sh_close(f);
        printinfo(&u, abort);
        f=sh_open1(dlfn,O_RDONLY | O_BINARY);
      } else if (!empty())
        checka(abort,&next);

    }
    f=sh_close(f);
  }
  curdir=od;
}


void nscanall(void)
{
  int abort,i,i1,count,color,ac=0;

  if ((uconfdir[1].confnum!=-1) && (okconf(&thisuser))) {
    if (!x_only) {
      nl();
      prt(5,get_string(1379));
      ac=yn();
      nl();
    } else
      ac=1;
    if (ac)
      tmp_disable_conf(1);
  }
  abort=0;
  num_listed=0;
  count=0;
  color=3;
  if (!x_only) {
    outstr("\r");
    outstr(get_string(1184));
  }
  for (i=0; (i<num_dirs) && (!abort) && (udir[i].subnum!=-1) &&
       (tagging!=0); i++) {
    count++;
    if (!x_only) {
      npr("%d.",color);
      if (count>=DOTS) {
        outstr("\r");
        outstr(get_string(1184));
        color++;
        count=0;
        if (color==4)
          color++;
        if (color==10)
          color=0;
      }
    }
    i1=udir[i].subnum;
    if (qsc_n[i1/32]&(1L<<(i1%32))) {
      titled=1;
      nscandir(i,&abort);
    }
  }
  endlist(2);
  if (ac)
    tmp_disable_conf(0);
}


void searchall(void)
{
  int i,i1,pts,abort,ocd,next=0,f,count,color,ac=0;
  char s[81];
  uploadsrec u;

  if ((uconfdir[1].confnum!=-1) && (okconf(&thisuser))) {
    if (!x_only) {
      nl();
      prt(5,get_string(1379));
      ac=yn();
      nl();
    } else
      ac=1;
    if (ac)
      tmp_disable_conf(1);
  }

  abort=0;
  ocd=curdir;
  if (x_only) {
    strcpy(s,"*.*");
    align(s);
  } else {
    nln(2);
    pl(get_string(745));
    file_mask(s);
    if (!x_only) {
      nl();
      outstr(get_string(1184));
    }
  }
  num_listed=0;
  lines_listed=0;
  count=0;
  color=3;
  for (i=0; (i<num_dirs) && (!abort) && (!hangup) && (tagging || x_only)
    && (udir[i].subnum!=-1); i++) {
    i1=udir[i].subnum;
    pts=0;
    if (qsc_n[i1/32]&(1L<<(i1%32)))
      pts=1;
    pts=1;
    /* remove pts=1 to search only marked directories */
    if (pts) {
      if (!x_only) {
        count++;
        npr("%d.",color);
        if (count>=DOTS) {
          outstr("\r");
          outstr(get_string(1184));
          color++;
          count=0;
          if (color==4)
            color++;
          if (color==10)
            color=0;
        }
      }
      curdir=i;
      dliscan();
      titled=1;
      f=sh_open1(dlfn,O_RDONLY | O_BINARY);
      for (i1=1; (i1<=numf) && (!abort) && (!hangup) && (tagging || x_only); i1++) {
        SETREC(f,i1);
        sh_read(f,(void *)&u,sizeof(uploadsrec));
        if (compare(s,u.filename)) {
          f=sh_close(f);
          printinfo(&u, &abort);
          f=sh_open1(dlfn,O_RDONLY | O_BINARY);
        } else if (!empty())
          checka(&abort,&next);
      }
      f=sh_close(f);
    }
  }
  curdir=ocd;
  endlist(1);
  if (ac)
    tmp_disable_conf(0);
}


int recno(char *s)
{
  int i,f;
  uploadsrec u;

  i=1;
  if (numf<1)
    return(-1);
  f=sh_open1(dlfn,O_RDONLY | O_BINARY);
  SETREC(f,i);
  sh_read(f,(void *)&u,sizeof(uploadsrec));
  while ((i<numf) && (compare(s,u.filename)==0)) {
    ++i;
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
  }
  f=sh_close(f);
  if (compare(s,u.filename))
    return(i);
  else
    return(-1);
}


int nrecno(char *s,int i1)
{
  int i,f;
  uploadsrec u;

  i=i1+1;
  if ((numf<1) || (i1>=numf))
    return(-1);

  f=sh_open1(dlfn,O_RDONLY | O_BINARY);
  SETREC(f,i);
  sh_read(f,(void *)&u,sizeof(uploadsrec));
  while ((i<numf) && (compare(s,u.filename)==0)) {
    ++i;
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
  }
  f=sh_close(f);
  if (compare(s,u.filename))
    return(i);
  else
    return(-1);
}


int printfileinfo(uploadsrec *u, int dn)
{
  char s[81];
  double d;
  int abort;

  d=XFER_TIME(u->numbytes);
  outstr(get_string(746)); pl(stripfn(u->filename));
  outstr(get_string(747)); pl(u->description);
  outstr(get_string(748)); npr("%dk\r\n", bytes_to_k(u->numbytes));
  outstr(get_string(749)); pl(ctim(d));
  outstr(get_string(750)); pl(u->date);
  outstr(get_string(751)); pl(u->upby);
  outstr(get_string(752)); pln(u->numdloads);
  nl();
  abort=0;
  if (u->mask & mask_extended) {
    pl(get_string(753));
    print_extended(u->filename,&abort,255,0);
  }

  sprintf(s,"%s%s",directories[dn].path,u->filename);
  if (!exist(s)) {
    nl();
    pl(get_string(754));
    nl();
    return(-1);
  }

  if (nsl()>=d)
    return(1);
  else
    return(0);
}


/****************************************************************************/
/* This kludge will get us through 2019 and should not interfere anywhere   */
/* else.                                                                    */
/****************************************************************************/

long date_to_daten(char *datet)
{
  struct time t;
  struct date d;

  if (strlen(datet)!=8)
    return(0);

  memset(&d, 0, sizeof(struct date));
  memset(&t, 0, sizeof(struct time));
  d.da_mon=atoi(datet);
  d.da_day=atoi(datet+3);
  d.da_year=1900+atoi(datet+6);                  /* fixed for 1920-2019 */
  if (datet[6]<'2')
    d.da_year+=100;
  return(dostounix(&d, &t));
}


/****************************************************************************/



void remlist(char *fn)
{
  int i,i2;
  char s[81],s1[81];

  sprintf(s,"%s",fn);
  align(s);

  if (filelist) {
    for (i=0;i<tagptr;i++) {
      strcpy(s1,filelist[i].u.filename);
      align(s1);
      if (strcmp(s,s1)==0) {
        for (i2=i;i2<tagptr-1;i2++)
          filelist[i2]=filelist[i2+1];
        tagptr--;
        num_listed--;
      }
    }
  }
}


