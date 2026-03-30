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

#include <time.h>
#include <dir.h>


/* number of dots for searching */
#define DOTS 5


#define SETREC(f,i)  sh_lseek(f,((long) (i))*((long)sizeof(uploadsrec)),SEEK_SET);

/* color for frame */
#define FRAME 7

void move_file(void)
{
  char sx[81],s1[81],s2[81],ch,*ss;
  int i,i1,ok,d1,d2,done,cp,f;
  uploadsrec u,u1;

  ok=0;
  nln(2);
  prt(2,get_string(821));
  input(sx,12);
  if (strchr(sx,'.')==NULL)
    strcat(sx,".*");
  align(sx);
  dliscan();
  i=recno(sx);
  if (i<0) {
    nl();
    pl(get_string(89));
    return;
  }
  done=0;

  tmp_disable_conf(1);

  while ((!hangup) && (i>0) && (!done)) {
    cp=i;
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    f=sh_close(f);
    nl();
    printfileinfo(&u,udir[curdir].subnum);
    nl();
    prt(5,get_string(822));
    ch=ynq();
    if (ch=='Q')
      done=1;
    else if (ch=='Y') {
      sprintf(s1,"%s%s",directories[udir[curdir].subnum].path,u.filename);
      do {
        nln(2);
        prt(2,get_string(823));
        ss=mmkey(1);
        if (ss[0]=='?') {
          dirlist();
          dliscan();
        }
      } while ((!hangup) && (ss[0]=='?'));
      d1=-1;
      if (ss[0])
        for (i1=0; (i1<num_dirs) && (udir[i1].subnum!=-1); i1++)
          if (strcmp(udir[i1].keys,ss)==0)
            d1=i1;
      if (d1!=-1) {
        ok=1;
        d1=udir[d1].subnum;
        dliscan1(d1);
        if (recno(u.filename)>0) {
          ok=0;
          nl();
          pl(get_string(824));
        }
        if (numf>=directories[d1].maxfiles) {
          ok=0;
          nl();
          pl(get_string(825));
        }
        if (freek1(directories[d1].path)<((double)(u.numbytes/1024L)+3)) {
          ok=0;
          nl();
          pl(get_string(826));
        }
        dliscan();
      } else
        ok=0;
    } else
      ok=0;
    if (ok && !done) {
      prt(5,get_string(934));
      if (yn()) {
        time((long *)&u.daten);
      }

      --cp;
      f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      for (i1=i; i1<numf; i1++) {
        SETREC(f,i1+1);
        sh_read(f,(void *)&u1,sizeof(uploadsrec));
        SETREC(f,i1);
        sh_write(f,(void *)&u1,sizeof(uploadsrec));
      }
      --numf;
      SETREC(f,0);
      sh_read(f, &u1, sizeof(uploadsrec));
      u1.numbytes=numf;
      SETREC(f,0);
      sh_write(f,(void *)&u1,sizeof(uploadsrec));
      f=sh_close(f);
      ss=read_extended_description(u.filename);
      if (ss)
        delete_extended_description(u.filename);

      sprintf(s2,"%s%s",directories[d1].path,u.filename);
      dliscan1(d1);
      f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      for (i=numf; i>=1; i--) {
        SETREC(f,i);
        sh_read(f,(void *)&u1,sizeof(uploadsrec));
        SETREC(f,i+1);
        sh_write(f,(void *)&u1,sizeof(uploadsrec));
      }
      SETREC(f,1);
      sh_write(f,(void *)&u,sizeof(uploadsrec));
      ++numf;
      SETREC(f,0);
      sh_read(f, &u1, sizeof(uploadsrec));
      u1.numbytes=numf;
      if (u.daten>u1.daten) {
        u1.daten = u.daten;
        dir_dates[d1]=u.daten;
      }
      SETREC(f,0);
      sh_write(f,(void *)&u1,sizeof(uploadsrec));
      f=sh_close(f);
      if (ss) {
        add_extended_description(u.filename,ss);
        bbsfree(ss);
      }

      if ((strcmp(s1,s2)!=0) && (exist(s1))) {
        d2=0;
        if ((s1[1]!=':') && (s2[1]!=':'))
          d2=1;
        if ((s1[1]==':') && (s2[1]==':') && (s1[0]==s2[0]))
          d2=1;
        if (d2) {
          rename(s1,s2);
          unlink(s1);
        } else {
          copy_file(s1,s2);
          unlink(s1);
        }
      }
      nl();
      pl(get_string(827));
    }
    dliscan();
    i=nrecno(sx,cp);
  }

  tmp_disable_conf(0);
}

int comparedl(uploadsrec *x, uploadsrec *y, int type)
{
  switch(type) {
    case 0:
      return(strcmp(x->filename,y->filename));
    case 1:
      if (x->daten < y->daten)
        return(-1);
      else
        if (x->daten > y->daten)
          return(1);
        else
          return(0);
    case 2:
      if (x->daten < y->daten)
        return(1);
      else
        if (x->daten > y->daten)
          return(-1);
        else
          return(0);
  }
  return(0);
}


void quicksort(int l,int r,int type)
{
  register int i,j,f;
  uploadsrec a,a2,x;

  i=l; j=r;
  f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  SETREC(f,((l+r)/2));
  sh_read(f, (void *)&x,sizeof(uploadsrec));
  do {
    SETREC(f,i);
    sh_read(f, (void *)&a,sizeof(uploadsrec));
    while (comparedl(&a,&x,type)<0) {
      SETREC(f,++i);
      sh_read(f, (void *)&a,sizeof(uploadsrec));
    }
    SETREC(f,j);
    sh_read(f, (void *)&a2,sizeof(uploadsrec));
    while (comparedl(&a2,&x,type)>0) {
      SETREC(f,--j);
      sh_read(f, (void *)&a2,sizeof(uploadsrec));
    }
    if (i<=j) {
      if (i!=j) {
        SETREC(f,i);
        sh_write(f,(void *)&a2,sizeof(uploadsrec));
        SETREC(f,j);
        sh_write(f,(void *)&a,sizeof(uploadsrec));
      }
      i++;
      j--;
    }
  } while (i<j);
  f=sh_close(f);
  if (l<j)
    quicksort(l,j,type);
  if (i<r)
    quicksort(i,r,type);
}


void sortdir(int dn, int type)
{
  dliscan1(dn);
  if (numf>1)
    quicksort(1,numf,type);
}


void sort_all(int type)
{
  int i;

  tmp_disable_conf(1);
  for (i=0; (i<num_dirs) && (udir[i].subnum!=-1) && (!kbhitb()); i++) {
    nl();
    ansic(1);
    outstr(get_string(828));
    pl(directories[udir[i].subnum].name);
    sortdir(i,type);
  }
  tmp_disable_conf(0);
}


void rename_file(void)
{
  char s[81],s1[81],s2[81],*ss,s3[81],ch;
  int i,cp,f;
  uploadsrec u;

  nln(2);
  prt(2,get_string(829));
  input(s,12);
  if (s[0]==0)
    return;
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  align(s);
  dliscan();
  nl();
  strcpy(s3,s);
  i=recno(s);
  while ((i>0) && (!hangup)) {
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    cp=i;
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    f=sh_close(f);
    nl();
    printfileinfo(&u,udir[curdir].subnum);
    nl();
    prt(5,get_string(830));
    ch=ynq();
    if (ch=='Q')
      break;
    else if (ch=='N') {
      i=nrecno(s3,cp);
      continue;
    }
    nl();
    prt(2,get_string(72));
    input(s,12);
    if (!okfn(s))
      s[0]=0;
    if (s[0]) {
      align(s);
      if (strcmp(s,get_string(1310))) {
        strcpy(s1,directories[udir[curdir].subnum].path);
        strcpy(s2,s1);
        strcat(s1,s);
        if (exist(s1))
          pl(get_string(831));
        else {
          strcat(s2,u.filename);
          rename(s2,s1);
          if (exist(s1)) {
            ss=read_extended_description(u.filename);
            if (ss) {
              delete_extended_description(u.filename);
              add_extended_description(s,ss);
              bbsfree(ss);
            }
            strcpy(u.filename,s);
          } else
            pl(get_string(832));
        }
      }
    }
    nl();
    pl(get_string(833));
    prt(2,": ");
    inputl(s,58);
    if (s[0]) {
      strcpy(u.description,s);
    }
    ss=read_extended_description(u.filename);
    nln(2);
    prt(5,get_string(834));
    if (yn()) {
      nl();
      if (ss) {
        prt(5,get_string(835));
        if (yn()) {
          bbsfree(ss);
          delete_extended_description(u.filename);
          u.mask &= ~mask_extended;
        } else {
          u.mask |= mask_extended;
          modify_extended_description(&ss,
            directories[udir[curdir].subnum].name,u.filename);
          if (ss) {
            delete_extended_description(u.filename);
            add_extended_description(u.filename,ss);
            bbsfree(ss);
          }
        }
      } else {
        modify_extended_description(&ss,
            directories[udir[curdir].subnum].name,u.filename);
        if (ss) {
          add_extended_description(u.filename,ss);
          bbsfree(ss);
          u.mask |= mask_extended;
        } else
          u.mask &= ~mask_extended;
      }
    } else
      if (ss) {
        bbsfree(ss);
        u.mask |= mask_extended;
      } else
        u.mask &= ~mask_extended;
    f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    SETREC(f,i);
    sh_write(f,(void *)&u,sizeof(uploadsrec));
    f=sh_close(f);
    i=nrecno(s3,cp);
  }
}


int upload_file(char *fn, int dn, char *desc)
{
  directoryrec d;
  uploadsrec u,u1;
  int i,f;
  char s[81],ff[81];
  long l;

  d=directories[dn];
  strcpy(s,fn);
  align(s);
  strcpy(u.filename,s);
  u.ownerusr=usernum;
  u.ownersys=0;
  u.numdloads=0;
  u.filetype=0;
  u.mask=0;
  if ((!(d.mask & mask_cdrom)) && (check_ul_event(dn,&u))) {
    outstr(fn);
    pl(get_string(836));
  } else {
    sprintf(ff,"%s%s",d.path,s);
    f=sh_open1(ff,O_RDONLY | O_BINARY);
    if (f<=0) {
      if (desc && (*desc)) {
        outstr(get_string(837));
        outstr(fn);
        outstr(": ");
        pl(desc);
      } else {
        outstr(fn);
        pl(get_string(838));
      }
      return(1);
    }
    l=filelength(f);
    u.numbytes=l;
    sh_close(f);
    strcpy(u.upby, nam(&thisuser,usernum));
    strcpy(u.date,date());
    if (d.mask & mask_PD)
      d.mask=mask_PD;
    npr("%s: %4ldk :",u.filename,bytes_to_k(u.numbytes));
    if ((desc) && (*desc)) {
      strncpy(u.description,desc,58);
      u.description[58]=0;
      pl(u.description);
    } else
      inputl(u.description,58);
    if (u.description[0]==0)
      return(0);
    get_file_idz(&u,dn);
    ++thisuser.uploaded;
    if (!(d.mask & mask_cdrom))
      modify_database(u.filename,1);
    thisuser.uk += bytes_to_k(l);
    time(&l);
    u.daten=l;
    if (sysinfo.flags & OP_FLAGS_PACKSCAN_FREQ) {
      if (!(d.mask & mask_cdrom))
        remotenotify(u.filename,u.description);
    }
    f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    for (i=numf; i>=1; i--) {
      SETREC(f,i);
      sh_read(f,(void *)&u1,sizeof(uploadsrec));
      SETREC(f,i+1);
      sh_write(f,(void *)&u1,sizeof(uploadsrec));
    }
    SETREC(f,1);
    sh_write(f,(void *)&u,sizeof(uploadsrec));
    ++numf;
    SETREC(f,0);
    sh_read(f, &u1, sizeof(uploadsrec));
    u1.numbytes=numf;
    u1.daten=l;
    dir_dates[dn]=l;
    SETREC(f,0);
    sh_write(f,(void *)&u1,sizeof(uploadsrec));
    f=sh_close(f);
    lock_status();
    ++status.uptoday;
    ++status.filechange[filechange_upload];
    save_status();
    sprintf(s,get_stringx(1,42),u.filename,d.name);
    sysoplog(s);
    topscreen();
  }
  return(1);
}


int maybe_upload(char *fn, int dn, char *desc)
{
  char s[81], ch, s1[81];
  int i,i1=0,ok=1,ocd,f;
  uploadsrec u;

  strcpy(s,fn);
  align(s);
  i=recno(s);

  if (i==-1) {
    if ((sysinfo.flags & OP_FLAGS_FAST_SEARCH) &&
        ((!is_uploadable(fn)) && (dcs()))) {
      npr("2%-12s: ", fn);
      prt(5,get_string(1322));
      ch=ynq();
      if (ch==*str_quit)
        return(0);
      else {
        if (ch==*str_no) {
          prt(5,get_string(835));
          if (yn()) {
            sprintf(s1,"%s%s", directories[dn].path, fn);
            unlink(s1);
            nl();
            return(1);
          } else {
            return(1);
          }
        }
      }
    }
    if (!upload_file(s,udir[dn].subnum,desc))
      ok=0;
  } else {
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    SETREC(f,i);
    sh_read(f,(void *)&u, sizeof(uploadsrec));
    f=sh_close(f);
    ocd=curdir;
    curdir=dn;
    printinfo(&u,&i1);
    curdir=ocd;
    if (i1)
      ok=0;
  }
  return(ok);
}

void upload_files(char *fn, int dn, int type)
/* This assumes the file holds listings of files, one per line, to be
 * uploaded.  The first word (delimited by space/tab) must be the filename.
 * after the filename are optional tab/space separated words (such as file
 * size or date/time).  After the optional words is the description, which
 * is from that position to the end of the line.  the "type" parameter gives
 * the number of optional words between the filename and description.
 * the optional words (size, date/time) are ignored completely.
 */
{
  char s[255],*fn1,*desc,last_fn[81],*ext=NULL;
  FILE *f;
  int ok=1,abort=0,next=0,ok1,i,f1;
  uploadsrec u;

  last_fn[0]=0;
  dliscan1(udir[dn].subnum);

  f=fsh_open(fn,"r");
  if (!f) {
    sprintf(s,"%s%s",directories[udir[dn].subnum].path,fn);
    f=fsh_open(s,"r");
  }
  if (!f) {
    outstr(fn);
    pl(get_string(1528));
  } else {
    while (ok && fgets(s,250,f)) {
      if (s[0]<32)
        continue;
      else if (s[0]==32) {
        if (last_fn[0]) {
          if (!ext) {
            ext=malloca(4096L);
            *ext=0;
          }
          for (desc=s; (*desc==' ') || (*desc=='\t'); desc++)
            ;
          if (*desc=='|') {
            do {
              desc++;
            } while ((*desc==' ') || (*desc=='\t'));
          }
          fn1=strchr(desc,'\n');
          if (fn1)
            *fn1=0;
          strcat(ext, desc);
          strcat(ext,"\r\n");
        }
      } else {
        ok1=0;
        fn1=strtok(s," \t\n");
        if (fn1) {
          ok1=1;
          for (i=0; ok1 && (i<type); i++)
            if (strtok(NULL," \t\n")==NULL)
              ok1=0;
          if (ok1) {
            desc=strtok(NULL,"\n");
            if (!desc)
              ok1=0;
          }
        }
        if (ok1) {
          if (last_fn[0] && ext && *ext) {
            f1=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
            SETREC(f1,1);
            sh_read(f1,(void *)&u, sizeof(uploadsrec));
            if (strcmp(last_fn,u.filename)==0) {
              modify_database(u.filename,1);
              add_extended_description(last_fn, ext);
              u.mask |= mask_extended;
              SETREC(f1,1);
              sh_write(f1, (void *)&u, sizeof(uploadsrec));
            }
            f1=sh_close(f1);
            *ext=0;
          }
          while ((*desc==' ') || (*desc=='\t'))
            ++desc;
          ok=maybe_upload(fn1,dn,desc);
          checka(&abort,&next);
          if (abort)
            ok=0;
          if (ok) {
            strcpy(last_fn, fn1);
            align(last_fn);
            if (ext)
              *ext=0;
          }
        }
      }
    }
    fsh_close(f);
    if (ok && last_fn[0] && ext && *ext) {
      f1=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      SETREC(f1,1);
      sh_read(f1,(void *)&u, sizeof(uploadsrec));
      if (strcmp(last_fn,u.filename)==0) {
        modify_database(u.filename,1);
        add_extended_description(last_fn, ext);
        u.mask |= mask_extended;
        SETREC(f1,1);
        sh_write(f1, (void *)&u, sizeof(uploadsrec));
      }
      f1=sh_close(f1);
    }
  }

  if (ext)
    bbsfree(ext);
}

int uploadall(int dn)
{
  int i1,f1,maxf,ok,abort=0,next;
  char s[81],s1[81];
  struct ffblk ff;

  dliscan1(udir[dn].subnum);
  nln(2);
  strcpy(s,"*.*");
  strcpy(s1,(directories[udir[dn].subnum].path));
  maxf=directories[udir[dn].subnum].maxfiles;
  strcat(s1,s);
  f1=findfirst(s1,&ff,0);
  ok=1;
  i1=0;
  while ((f1==0) && (!hangup) && (numf<maxf) && (ok) && (!i1) && (!abort)) {
    ok=maybe_upload(ff.ff_name,dn,NULL);
    f1=findnext(&ff);
    checka(&abort,&next);
  }
  if ((!ok) || (abort))
    pl(get_string(14));
  if (numf>=maxf)
    pl(get_string(839));
  return(i1);
}


void relist(void)
{
  char s[85],s1[40],s2[81];
  int i,i1,next,abort=0,fc;
  int tcd=-1, otag, tcdi;

  if (!filelist)
    return;
  outchr(12);
  lines_listed=0;
  otag=tagging;
  tagging=0;
  fc=thisuser.sysstatus & sysstatus_extra_color;
  if (sysinfo.flags & OP_FLAGS_FAST_TAG_RELIST) {
    ansic(fc ? FRAME : 0);
    if (okansi())
      pl(get_string(1317));
    else
      pl(get_string(1355));
  }
  for (i=0;i<tagptr;i++) {
    if (!(sysinfo.flags & OP_FLAGS_FAST_TAG_RELIST)) {
      if (tcd!=filelist[i].directory) {
        ansic(fc ? FRAME : 0);
        if (tcd!=-1) {
          if (okansi())
            npr("\r%s\r\n",get_string(1313));
          else
            npr("\r%s\r\n",get_string(1357));
        }
        tcd=filelist[i].directory;
        tcdi=-1;
        for (i1=0; i1<num_dirs; i1++) {
          if (udir[i1].subnum==tcd) {
            tcdi=i1;
            break;
          }
        }
        if (fc)
          ansic(2);
        if (tcdi==-1)
          npr("%s.\r\n", directories[tcd].name);
        else
          npr("%s - #%s.\r\n",directories[tcd].name,udir[tcdi].keys);
        ansic(fc ? FRAME : 0);
        if (okansi())
          npr("%s\r\n",get_string(1317));
        else
          npr("%s\r\n",get_string(1318));
      }
    }
    sprintf(s,"%d%2d%d%c",
      check_batch_queue(filelist[i].u.filename) ? 6 : 0,
      i+1, fc ? FRAME : 0, okansi() ? 'º' : '|' );
    osan(s,&abort,&next);
    if (fc)
      ansic(1);
    strncpy(s,filelist[i].u.filename,8);
    s[8]=0;
    osan(s,&abort,&next);
    strncpy(s,&((filelist[i].u.filename)[8]),4);
    s[4]=0;
    if (fc)
      ansic(1);
    osan(s,&abort,&next);
    ansic(fc ? FRAME : 0);
    osan((okansi() ? "º" : ":"),&abort,&next);

    ltoa(bytes_to_k(filelist[i].u.numbytes),s1,10);
    strcat(s1,"k");
    if (!(sysinfo.flags & OP_FLAGS_FAST_TAG_RELIST)) {
      if (!(directories[tcd].mask & mask_cdrom)) {
        strcpy(s2,directories[tcd].path);
        strcat(s2,filelist[i].u.filename);
        if (!exist(s2))
          strcpy(s1,get_string(741));
      }
    }
    for (i1=0; i1<5-(int)strlen(s1); i1++)
      s[i1]=32;
    s[i1]=0;
    strcat(s,s1);
    if (fc)
      ansic(2);
    osan(s,&abort,&next);

    ansic(fc ? FRAME : 0);
    osan((okansi() ? "º" : "|"),&abort,&next);
    sprintf(s1,"%d",filelist[i].u.numdloads);

    for (i1=0; i1<4-strlen(s1); i1++)
      s[i1]=32;
    s[i1]=0;
    strcat(s,s1);
    if (fc)
      ansic(2);
    osan(s,&abort,&next);

    ansic(fc ? FRAME : 0);
    osan((okansi() ? "º" : "|"),&abort,&next);
    sprintf(s,"%d%s",
      (filelist[i].u.mask & mask_extended) ? 1 : 2,
      filelist[i].u.description);
    plal(s,thisuser.screenchars-28,&abort);
  }
  ansic(fc ? FRAME : 0);
  if (okansi())
    npr("\r%s\r\n",get_string(1313));
  else
    npr("\r%s\r\n",get_string(1357));
  tagging=otag;
  lines_listed=0;
}

/****************************************************************************/

void edit_database(void)
/*
 * Allows user to add or remove ALLOW.DAT entries.
 */
{
  unsigned char ch,s[20];
  int done=0;

  if (!(sysinfo.flags & OP_FLAGS_FAST_SEARCH))
    return;

  do {
    nl();
    pl(get_string(1382));
    pl(get_string(1383));
    pl(get_string(1384));
    nl();
    prt(1,get_string(1385));
    ch=onek("QAR");
    switch (ch) {
      case 'A':
        nl();
        prt(2,get_string(44));
        mpl(12);
        input(s,12);
        if (s[0])
          modify_database(s,1);
        break;
      case 'R':
        nl();
        prt(2,get_string(44));
        mpl(12);
        input(s,12);
        if (s[0])
          modify_database(s,0);
        break;
      case 'Q':
        done=1;
        break;
    }
  } while ((!hangup) && (!done));
}


/****************************************************************************/
/*
 * Checks the database file to see if filename is there.  If so, the record
 * number plus one is returned.  If not, the negative of record number
 * that it would be minus one is returned.  The plus one is there so that there
 * is no record #0 ambiguity.
 */

long db_index(int af, char *fn)
{
  char cfn[18],tfn[81], tfn1[81];
  int i;
  long hirec, lorec, currec, ocurrec=-1;

  strcpy(tfn1, fn);
  align(tfn1);
  strcpy(tfn,stripfn(tfn1));

  hirec=filelength(af)/13;
  lorec=0;

  if (hirec==0)
    return(-1);

  while (1) {
    currec=((long)((hirec+lorec)/2));
    if (currec==ocurrec) {
      if (i<0)
        return(-(currec+2));
      else
        return(-(currec+1));
    }
    ocurrec=currec;
    sh_lseek(af,((long) (currec*13)),SEEK_SET);
    sh_read(af,(void *)&cfn,13);
    i=strcmp(cfn,tfn);

    /* found */
    if (i==0) {
      return(currec+1);
    } else {
      if (i<0) {
        lorec=currec;
      } else {
        hirec=currec;
      }
    }
  }
}

/****************************************************************************/
/*
 * Adds or deletes a single entry to/from filename database.
 */

#define ALLOW_BUFSIZE 61440

void modify_database(char *fn, int add)
{
  char dbfn[81], tfn[81], tfn1[81];
  int af;
  unsigned int nb;
  long rec, len, l, l1,cp;
  char *bfr;

  if (!(sysinfo.flags & OP_FLAGS_FAST_SEARCH))
    return;

  sprintf(dbfn, "%sALLOW.DAT", syscfg.datadir);
  af=sh_open1(dbfn, O_RDWR|O_BINARY|O_CREAT);

  if (af<0)
    return;

  rec=db_index(af, fn);

  if (((rec<0) && (!add)) || ((rec>0) && (add))) {
    sh_close(af);
    return;
  }

  bfr=(char *)malloca(ALLOW_BUFSIZE);
  if (!bfr) {
    sh_close(af);
    return;
  }

  len=filelength(af);

  if (add) {
    cp=(-1-rec)*13;
    l1=len;

    do {
      l=l1-cp;
      if (l<ALLOW_BUFSIZE)
        nb=(unsigned int)l;
      else
        nb=ALLOW_BUFSIZE;
      if (nb) {
        sh_lseek(af, l1-(long)nb, SEEK_SET);
        sh_read(af, bfr, nb);
        sh_lseek(af, l1-(long)nb + 13, SEEK_SET);
        sh_write(af, bfr, nb);
        l1 -= nb;
      }
    } while (nb==ALLOW_BUFSIZE);

    /* put in the new value */
    strcpy(tfn1, fn);
    align(tfn1);
    strncpy(tfn,stripfn(tfn1), 13);
    sh_lseek(af, cp, SEEK_SET);
    sh_write(af,(void *)tfn,13);
  } else {
    cp=rec*13;

    do {
      l=len-cp;
      if (l<ALLOW_BUFSIZE)
        nb=(unsigned int)l;
      else
        nb=ALLOW_BUFSIZE;
      if (nb) {
        sh_lseek(af, cp, SEEK_SET);
        sh_read(af, bfr, nb);
        sh_lseek(af, cp-13, SEEK_SET);
        sh_write(af, bfr, nb);
        cp += nb;
      }
    } while (nb==ALLOW_BUFSIZE);

    /* truncate the file */
    chsize(af, len-13);
  }

  bbsfree(bfr);
  sh_close(af);
}

/****************************************************************************/
/*
 * Returns 1 if file not found in filename database.
 */

int is_uploadable(char *fn)
{
  char dbfn[81];
  int af;
  long rc;

  if (!(sysinfo.flags & OP_FLAGS_FAST_SEARCH))
    return(1);;

  sprintf(dbfn, "%sALLOW.DAT", syscfg.datadir);
  af=sh_open1(dbfn, O_RDONLY|O_BINARY);
  if (af<0)
    return(1);
  rc=db_index(af, fn);
  sh_close(af);
  if (rc<0)
    return(1);
  else
    return(0);
}
/****************************************************************************/


void l_config_nscan(void)
{
  int i,abort,i1;
  char s[81], s2[81];

  abort=0;
  nl();
  pl(get_string(802));
  nl();
  for (i=0; (i<num_dirs) && (udir[i].subnum!=-1) && (!abort); i++) {
    i1=udir[i].subnum;
    if (qsc_n[i1/32]&(1L<<(i1%32)))
      strcpy(s,"* ");
    else
      strcpy(s,"  ");
    sprintf(s2,"%s%s. %s",s,udir[i].keys, directories[i1].name);
    pla(s2,&abort);
  }
  nln(2);
}

void config_nscan(void)
{
  char *s,s1[MAX_CONFERENCES+2],s2[120],ch;
  int i,i1,done,done1,oc,os,abort=0;

  done=done1=0;
  oc=curconfdir;
  os=udir[curdir].subnum;

  do {
    if ((okconf(&thisuser)) && (uconfdir[1].confnum!=-1)) {
      abort=0;
      strcpy(s1," ");
      nl();
      pl(get_string(1081));
      nl();
      i=0;
      while ((i<dirconfnum) && (uconfdir[i].confnum!=-1) && (!abort)) {
        sprintf(s2,"%c) %s",dirconfs[uconfdir[i].confnum].designator,
          stripcolors(dirconfs[uconfdir[i].confnum].name));
        pla(s2,&abort);
        s1[i+1]=dirconfs[uconfdir[i].confnum].designator;
        s1[i+2]=0;
        i++;
      }
      nl();
      outstr(get_string(1082));
      outstr(&s1[1]);
      outstr(get_string(1083));
      ch=onek(s1);
    } else
      ch='-';
    switch (ch) {
      case ' ':
        done1=1;
        break;
      default:
        if ((okconf(&thisuser)) && (dirconfnum>1)) {
          i=0;
          while ((ch!=dirconfs[uconfdir[i].confnum].designator) && (i<dirconfnum))
            i++;
          if (i>=dirconfnum)
            break;

          setuconf(CONF_DIRS, i, -1);
        }

        l_config_nscan();
        done=0;
        do {
          nl();
          outstr(get_string(1359));
          s=mmkey(1);
          if (s[0]) {
            for (i=0; i<num_dirs; i++) {
              i1=udir[i].subnum;
              if (strcmp(udir[i].keys,s)==0) {
                qsc_n[i1/32] ^= 1L<<(i1%32);
              }
              if (strcmp(s,"S")==0) {
                qsc_n[i1/32] |= 1L<<(i1%32);
              }
              if (strcmp(s,"C")==0) {
                qsc_n[i1/32] &= ~(1L<<(i1%32));
              }
            }
            if (strcmp(s,"Q")==0)
              done=1;
            if (strcmp(s,"?")==0)
              l_config_nscan();
          }
        } while ((!done) && (!hangup));
        break;
    }
    if ((!okconf(&thisuser)) || (uconfdir[1].confnum==-1))
      done1=1;
  } while ((!done1) && (!hangup));

  if (okconf(&thisuser))
    setuconf(CONF_DIRS, oc, os);
}


void xfer_defaults(void)
{
  char s[81],ch;
  int i,done;

  done=0;
  do {
    if (menu_on()) {
      printmenu(318);
      if ((thisuser.sysstatus & sysstatus_no_tag) == 0)
            comstr("|@7O3BX\r ");
      if (thisuser.sysstatus & sysstatus_nscan_file_system)
            comstr("|@7O4XX\r ");
    } else {
      outchr(12);
      pl(get_string(804));
      pl(get_string(805));
      outstr(get_string(806));
      npr(" (%s).\r\n", thisuser.sysstatus & sysstatus_nscan_file_system?str_yes:str_no);
      outstr(get_string(807));
      npr(" (%d %s%s).\r\n", thisuser.num_extended, get_string(808),
          thisuser.num_extended==1?"":"s");
      outstr(get_string(1360));
      if (thisuser.sysstatus & sysstatus_no_tag)
        outstr(get_string(1361));
      else
        outstr(get_string(1362));
      pl(")");
      pl(get_string(809));
      nl();
      prt(2,get_string(297));
    }
    helpl=32;
    ch=onek("Q12345");
    if (menu_on() && ch != '3' && ch != '5') {
      printmenu(319);
    }
    switch(ch) {
      case 'Q':
        done=1;
        break;
      case '1':
        helpl=24;
        config_nscan();
        break;
      case '2':
        nln(2);
        pl(get_string(810));
        nl();
        helpl=40;
        i=get_protocol(xf_down);
        if (i>=0)
          thisuser.defprot=i;
        break;
      case '3':
        thisuser.sysstatus ^= sysstatus_nscan_file_system;
        break;
      case '4':
        nln(2);
        pl(get_string(813));
        sprintf(s,"%s%d7)",get_string(814),sysinfo.max_extend_lines);
        pl(s);
        outstr(get_string(815));
        pln(thisuser.num_extended);
        prt(5,"? ");
        helpl=41;
        input(s,3);
        if (s[0]) {
          i=atoi(s);
          if ((i>=0) && (i<=sysinfo.max_extend_lines))
            thisuser.num_extended=i;
        }
        break;
      case '5':
          thisuser.sysstatus ^= sysstatus_no_tag;
        break;
    }
  } while ((!done) && (!hangup));
}



void finddescription(void)
{
  uploadsrec u;
  int i,i1,i2,abort,ocd,pts,next=0,f,count,color,ac=0;
  char s[81],s1[81];

  nl();
  if ((uconfdir[1].confnum!=-1) && (okconf(&thisuser))) {
    if (!x_only) {
      prt(5,get_string(1379));
      ac=yn();
    } else
      ac=1;
    if (ac)
      tmp_disable_conf(1);
  }
  nl();
  pl(get_string(793));
  nl();
  pl(get_string(794));
  outstr(":");
  input(s1,58);
  if (s1[0]==0) {
    tmp_disable_conf(0);
    return;
  }

  ocd=curdir;
  abort=0;
  num_listed=0;
  count=0;
  color=3;
  if (!x_only) {
    outstr("\r");
    outstr(get_string(1184));
  }
  lines_listed=0;
  for (i=0; (i<num_dirs) && (!abort) && (!hangup) && (tagging!=0)
    && (udir[i].subnum!=-1); i++) {
    i1=udir[i].subnum;
    pts=0;
    titled=1;
    if (qsc_n[i1/32]&(1L<<(i1%32)))
      pts=1;
    pts=1;
    /* remove pts=1 to search only marked directories */
    if ((pts) && (!abort) && (tagging!=0)) {
      if (!x_only) {
        count++;
        npr("%d.",color);
        if (count==DOTS) {
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
      f=sh_open1(dlfn,O_RDONLY | O_BINARY);
      for (i1=1; (i1<=numf) && (!abort) && (!hangup) && (tagging !=0); i1++) {
        SETREC(f,i1);
        sh_read(f,(void *)&u,sizeof(uploadsrec));
        strcpy(s,u.description);
        for (i2=0; i2<strlen(s); i2++)
          s[i2]=upcase(s[i2]);
        if (strstr(s,s1)!=NULL) {
          f=sh_close(f);
          printinfo(&u, &abort);
          f=sh_open1(dlfn,O_RDONLY | O_BINARY);
        } else if (!empty())
          checka(&abort,&next);
      }
      f=sh_close(f);
    }
  }
  if (ac)
    tmp_disable_conf(0);
  curdir=ocd;
  endlist(1);
}


void arc_l(void)
{
  char s[81];
  int i,abort,next,i1,f;
  uploadsrec u;

  nl();
  prt(2,get_string(795));
  input(s,12);
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  if (!okfn(s))
    s[0]=0;
  align(s);
  dliscan();
  abort=0;
  next=0;
  i=recno(s);
  do {
    if (i>0) {
      f=sh_open1(dlfn,O_RDONLY | O_BINARY);
      SETREC(f,i);
      sh_read(f,(void *)&u,sizeof(uploadsrec));
      f=sh_close(f);
      i1=list_arc_out(stripfn(u.filename),directories[udir[curdir].subnum].path);
      if (i1)
        abort=1;
      checka(&abort,&next);
      i=nrecno(s,i);
    }
  } while ((i>0) && (!hangup) && (!abort));
}


