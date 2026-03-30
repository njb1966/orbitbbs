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



#define utoa(s,v,r) ultoa((unsigned long)(s),v,r)


void gfiledata(int n, char *s)
{
  char x,i;
  gfiledirrec r;

  r=gfilesec[n];
  if (r.ar==0)
    x=32;
  else {
    for (i=0; i<16; i++)
      if ((1 << i) & r.ar)
        x='A'+i;
  }
  sprintf(s,"%2d %1c  %-40s  %-8s %-3d %-3d %-3d",
            n,x,stripcolors(r.name),r.filename,r.sl,r.age,r.maxfiles);
}

void showsec(void)
{
  int abort,i;
  char s[180];

  outchr(12);
  abort=0;
  pla(get_string(60),
      &abort);
  pla(get_string(61),
      &abort);
  for (i=0; (i<num_sec) && (!abort); i++) {
    gfiledata(i,s);
    pla(s,&abort);
  }
}


int exist_dir(char *s)
{
  int ok;

  cd_to(syscfg.gfilesdir);
  if (chdir(s))
    ok=0;
  else
    ok=1;
  cd_to(cdir);
  return(ok);
}


void modify_sec(int n)
{
  gfiledirrec r;
  char s[81],ch,ch2;
  int i,done;

  r=gfilesec[n];
  done=0;
  do {
    outchr(12);
    outstr(get_string(62)); pl(r.name);
    outstr(get_string(63)); pl(r.filename);
    outstr(get_string(64)); pln(r.sl);
    outstr(get_string(65)); pln(r.age);
    outstr(get_string(66)); pln(r.maxfiles);
    strcpy(s,get_string(5));
    if (r.ar!=0) {
      for (i=0; i<16; i++)
        if ((1 << i) & r.ar)
          s[0]='A'+i;
      s[1]=0;
    }
    outstr(get_string(67)); pl(s);
    nl();
    prt(2,get_string(68));
    ch=onek("QABCDEF[]");
    switch(ch) {
      case 'Q':done=1; break;
      case '[':
        gfilesec[n]=r;
        if (--n<0)
          n=num_sec-1;
        r=gfilesec[n];
        break;
      case ']':
        gfilesec[n]=r;
        if (++n>=num_sec)
          n=0;
        r=gfilesec[n];
        break;
      case 'A':
        nl();
        prt(2,get_string(69));
        inputl(s,40);
        if (s[0])
          strcpy(r.name,s);
        break;
      case 'B':
        nl();
        if (exist_dir(r.filename)) {
          nl();
          pl(get_string(70));
          pl(get_string(71));
          nl();
        }
        nl();
        prt(2,get_string(72));
        input(s,8);
        if ((s[0]!=0) && (strchr(s,'.')==0)) {
          strcpy(r.filename,s);
          if (!exist_dir(r.filename)) {
            nl();
            prt(5,get_string(73));
            if (yn()) {
              cd_to(syscfg.gfilesdir);
              mkdir(r.filename);
              cd_to(cdir);
            } else {
              nl();
              pl(get_string(74));
              nl();
            }
          } else {
            nl();
            pl(get_string(75));
            nl();
          }
          pausescr();
        }
        break;
      case 'C':
        nl();
        prt(2,get_string(76));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<256) && (s[0]))
          r.sl=i;
        break;
      case 'D':
        nl();
        prt(2,get_string(77));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<128) && (s[0]))
          r.age=i;
        break;
      case 'E':
        nl();
        pl(get_string(78));
        prt(2,get_string(79));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<99) && (s[0]))
          r.maxfiles=i;
        break;
      case 'F':
        nl();
        prt(2,get_string(80));
        ch2=onek("ABCDEFGHIJKLMNOP ");
        if (ch2==32)
          r.ar=0;
        else
          r.ar=1 << (ch2-'A');
        break;
    }
  } while ((!done) && (!hangup));
  gfilesec[n]=r;
}


void insert_sec(int n)
{
  gfiledirrec r;
  int i;

  for (i=num_sec-1; i>=n; i--)
    gfilesec[i+1]=gfilesec[i];
  strcpy(r.name,get_string(81));
  strcpy(r.filename,get_string(82));
  r.sl=10;
  r.age=0;
  r.maxfiles=99;
  r.ar=0;
  gfilesec[n]=r;
  ++num_sec;
  modify_sec(n);
}


void delete_sec(int n)
{
  int i;

  for (i=n; i<num_sec; i++)
    gfilesec[i]=gfilesec[i+1];
  --num_sec;
}


void gfileedit(void)
{
  int i,done,f;
  char s[81],ch;

  if (!checkpw())
    return;
  showsec();
  done=0;
  do {
    nl();
    prt(2,get_string(83));
    ch=onek("QDIM?");
    switch(ch) {
      case '?':
        showsec();
        break;
      case 'Q':
        done=1;
        break;
      case 'M':
        nl();
        prt(2,get_string(84));
        input(s,2);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_sec))
          modify_sec(i);
        break;
      case 'I':
        if (num_sec<sysinfo.max_gfilesec) {
          nl();
          prt(2,get_string(85));
          input(s,2);
          i=atoi(s);
          if ((s[0]!=0) && (i>=0) && (i<=num_sec))
            insert_sec(i);
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(86));
        input(s,2);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_sec)) {
          nl();
          ansic(5);
          outstr(get_string(87));
          outstr(gfilesec[i].name);
          outstr("? ");
          if (yn())
            delete_sec(i);
        }
        break;
    }
  } while ((!done) && (!hangup));
  sprintf(s,"%sGFILE.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  sh_write(f,(void *)(&gfilesec[0]), num_sec * sizeof(gfiledirrec));
  sh_close(f);
}


int fill_sec(int sn)
{
  gfilerec *g,g1;
  int f1,nf,ok,i,i1,chd;
  char s[81],s1[81];
  struct ffblk ff;

  g=read_sec(sn,&nf);
  sprintf(s1,"%s%s\\*.*",syscfg.gfilesdir,gfilesec[sn].filename);
  f1=findfirst(s1,&ff,0);
  ok=1;
  chd=0;
  while ((f1==0) && (!hangup) && (nf<gfilesec[sn].maxfiles) && (ok)) {
    strcpy(s,(ff.ff_name));
    align(s);
    i=1;
    for (i1=0; i1<nf; i1++)
      if (compare(s,g[i1].filename))
        i=0;
    if (i) {
      ansic(2);
      npr("%s : ",s);
      inputl(s1,60);
      if (s1[0]) {
        chd=1;
        i=0;
        while ((strcmp(s1,g[i].description)>0) && (i<nf))
          ++i;
        for (i1=nf; i1>i; i1--)
          g[i1]=g[i1-1];
        ++nf;
        strcpy(g1.filename,s);
        strcpy(g1.description,s1);
        time(&(g1.daten));
        g[i]=g1;
      } else
        ok=0;
    }
    f1=findnext(&ff);
  }
  if (!ok)
    pl(get_string(14));
  if (nf>=gfilesec[sn].maxfiles)
    pl(get_string(88));
  if (chd) {
    sprintf(s,"%s%s.GFL",syscfg.datadir,gfilesec[sn].filename);
    i=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    sh_write(i,(void *)g,nf*sizeof(gfilerec));
    sh_close(i);
    lock_status();
    strcpy(status.gfiledate, date());
    save_status();
  }
  bbsfree(g);
  return(!ok);
}

/****************************************************************************/

void pack_all_subs(void)
{
  int i=0, abort=0, next;

  tmp_disable_pause(1);
  while ((!hangup) && (!abort) && (i<num_subs)) {
    pack_sub(i);
    checka(&abort,&next);
    i++;
  }
  if (abort)
    pl(get_string(14));
  tmp_disable_pause(0);
}

/****************************************************************************/

void pack_sub(int si)
{
  char *sfn, *nfn, *mt, fn1[81], fn2[81];
  int i;
  long l;
  postrec *p;

  if (iscan1(si, 0)) {
    if (open_sub(1)) {

      sfn=subboards[si].filename;
      nfn="PACKTMP$";

      sprintf(fn1, "%s%s.DAT", syscfg.msgsdir, sfn);
      sprintf(fn2, "%s%s.DAT", syscfg.msgsdir, nfn);

      npr("\r\n%s %s \r\n", get_string(1501), subboards[si].name);

      for (i=1; i<=nummsgs; i++) {
        if (i%10==0)
          npr("%u/%u\r", i, nummsgs);
        p=get_post(i);
        if (p) {
          mt=readfile(&(p->msg), sfn, &l);
          if (!mt) {
            mt=malloca(10);
            if (mt) {
              strcpy(mt,"??");
              l=3;
            }
          }
          if (mt) {
            savefile(mt, l, &(p->msg), nfn);
            write_post(i, p);
          }
        }
      }

      unlink(fn1);
      rename(fn2, fn1);

      close_sub();
      nln(2);
      pl(get_string(1502));
    }
  }
}
