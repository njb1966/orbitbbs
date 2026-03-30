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

/****************************************************************************/

/*
 * Shows listing of files currently in batch queue(s), both upload and
 * download. Shows estimated time, by item, for files in the d/l queue.
 */

void listbatch(void)
{
  char s[81];
  int abort,i;

  abort=0;
  nl();
  outstr(get_string(864));
  npr("%d  ",numbatch);
  if (numbatchdl) {
    outstr(get_string(865));
    pl(ctim(batchtime));
  } else
    nl();
  nl();
  for (i=0; (i<numbatch) && (!abort) && (!hangup); i++) {
    if (batch[i].sending)
      sprintf(s,"%d. %s %s   %s  %s",
      i+1,
      get_string(1621),
      batch[i].filename,
      ctim(batch[i].time),
      directories[batch[i].dir].name);
    else
      sprintf(s,"%d. %s %s             %s",
      i+1,
      get_string(1622),
      batch[i].filename,
      directories[batch[i].dir].name);

    pla(s,&abort);
  }
}

/****************************************************************************/

/*
 * Deletes one item (index i) from the d/l batch queue.
 */

void delbatch(int i)
{
  int i1;

  if (i<numbatch) {
    if (batch[i].sending) {
      batchtime -= batch[i].time;
      --numbatchdl;
    }
    --numbatch;
    for (i1=i; i1<=numbatch; i1++) {
      batch[i1]=batch[i1+1];
    }
  }
}

/****************************************************************************/

void downloaded(char *fn, long cps)
{
  int i,i1,f;
  uploadsrec u;
  char s[161];
  userrec ur;

  for (i1=0; i1<numbatch; i1++) {
    if ((strcmp(fn,batch[i1].filename)==0) && (batch[i1].sending)) {
      dliscan1(batch[i1].dir);
      i=recno((batch[i1].filename));
      if (i>0) {
        f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
        SETREC(f,i);
        sh_read(f,(void *)&u,sizeof(uploadsrec));
        ++thisuser.downloaded;
        thisuser.dk += (int) (bytes_to_k(u.numbytes));
        ++u.numdloads;
        SETREC(f,i);
        sh_write(f,(void *)&u,sizeof(uploadsrec));
        f=sh_close(f);
        if (cps)
          sprintf(s,get_stringx(1,50),u.filename, cps);
        else
          sprintf(s,get_stringx(1,43),u.filename);
        sysoplog(s);
        if (syscfg.sysconfig & sysconfig_log_dl) {
          read_user(u.ownerusr, &ur);
          if (!(ur.inact & inact_deleted)) {
            if (date_to_daten(ur.firston) < u.daten) {
              sprintf(s,get_stringx(1,51),
                nam(&thisuser,usernum), u.filename, date());
              ssm(u.ownerusr,0,s);
            }
          }
        }
      }
      delbatch(i1);

      return;
    }
  }
  sprintf(s,get_stringx(1,52),fn);
  sysoplog(s);
}

/****************************************************************************/

void didnt_upload(int ind)
{
  int i,i1,f;
  char s[161];
  uploadsrec u;

  if (batch[ind].sending)
    return;

  dliscan1(batch[ind].dir);
  i=recno(batch[ind].filename);
  if (i>0) {
    f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    do {
      SETREC(f,i);
      sh_read(f, &u, sizeof(uploadsrec));
      if (u.numbytes!=0) {
        f=sh_close(f);
        i=nrecno(batch[ind].filename, i);
        f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      }
    } while ((i!=-1) && (u.numbytes!=0));
    if ((i!=-1) && (u.numbytes==0)) {
      if (u.mask & mask_extended)
        delete_extended_description(u.filename);
      for (i1=i; i1<numf; i1++) {
        SETREC(f,i1+1);
        sh_read(f,(void *)&u,sizeof(uploadsrec));
        SETREC(f,i1);
        sh_write(f,(void *)&u,sizeof(uploadsrec));
      }
      --i;
      --numf;
      SETREC(f,0);
      sh_read(f, &u, sizeof(uploadsrec));
      u.numbytes=numf;
      SETREC(f,0);
      sh_write(f,(void *)&u,sizeof(uploadsrec));
      f=sh_close(f);
      return;
    }
    f=sh_close(f);
  }
  sprintf(s,get_stringx(1,53),batch[ind].filename);
  sysoplog(s);
}

/****************************************************************************/

int try_to_ul(char *fn)
{

  if (hangup)
    return(1);

  if (!okfn(fn))
    return(1);

  /* future expansion here */

  /* yes, this should be return(1) until there is code above to prompt the */
  /* user for the file's info, and to handle uploading it */
  return(1);
}

/****************************************************************************/

void uploaded(char *fn, long cps)
{
  int i1, rn, f, f1;
  uploadsrec u;
  char s[161], s1[81], s2[81];

  for (i1=0; i1<numbatch; i1++) {
    if ((strcmp(fn,batch[i1].filename)==0) && (!batch[i1].sending)) {
      dliscan1(batch[i1].dir);
      rn=recno((batch[i1].filename));
      if (rn>0) {
        f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
        do {
          SETREC(f,rn);
          sh_read(f, &u, sizeof(uploadsrec));
          if (u.numbytes!=0)
            rn=nrecno(batch[i1].filename, rn);
        } while ((rn!=-1) && (u.numbytes!=0));
        f=sh_close(f);
        if ((rn!=-1) && (u.numbytes==0)) {

          sprintf(s1,"%s%s",syscfgovr.batchdir, fn);
          sprintf(s2,"%s%s", directories[batch[i1].dir].path, fn);

          if ((strcmp(s1,s2)!=0) && (exist(s1))) {
            f1=0;
            if ((s1[1]!=':') && (s2[1]!=':'))
              f1=1;
            if ((s1[1]==':') && (s2[1]==':') && (s1[0]==s2[0]))
              f1=1;
            if (f1) {
              rename(s1,s2);
              unlink(s1);
            } else {
              copy_file(s1,s2);
              unlink(s1);
            }
          }

          f1=sh_open1(s2,O_RDONLY | O_BINARY);
          if (f1>0) {
            if (syscfg.upload_c[0]) {
              sh_close(f1);
              if (check_ul_event(batch[i1].dir, &u)) {
                didnt_upload(i1);
                f1=-1;
              } else {
                f1=sh_open1(s2,O_RDONLY | O_BINARY);
              }
            }
            if (f1>=0) {
              u.numbytes = filelength(f1);
              sh_close(f1);
              get_file_idz(&u,batch[i1].dir);
              ++thisuser.uploaded;
              modify_database(u.filename,1);
              thisuser.uk += (int) (bytes_to_k(u.numbytes));
              lock_status();
              ++status.uptoday;
              ++status.filechange[filechange_upload];
              save_status();
              f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
              SETREC(f,rn);
              sh_write(f,(void *)&u,sizeof(uploadsrec));
              f=sh_close(f);
              sprintf(s,get_stringx(1,54),
                u.filename,
                directories[batch[i1].dir].name, cps);
              sysoplog(s);
              outstr(get_string(866));
              outstr(u.filename);
              outstr(get_string(867));
              pl(directories[batch[i1].dir].name);
            }
          }
          delbatch(i1);
          return;
        }
      }
      delbatch(i1);
      if (try_to_ul(fn)) {
        sprintf(s,get_stringx(1,55), fn);
        sysoplog(s);
        outstr(get_string(868));
        pl(fn);
      }

      return;
    }
  }
  if (try_to_ul(fn)) {
    sprintf(s,get_stringx(1,56),fn);
    sysoplog(s);
    outstr(get_string(869));
    pl(fn);
    sprintf(s,"%s%s",syscfgovr.batchdir, fn);
    unlink(s);
  }
}

/****************************************************************************/

void ymbatchdl(int had)
{
  int rr,i,ok,cur=0,f;
  char s[161],s1[81];
  uploadsrec u;
  double percent;

  if (!incom)
    return;
  sprintf(s,get_stringx(1,57),numbatchdl, ctim(batchtime));
  if (had)
    strcat(s,get_stringx(1,58));
  sysoplog(s);
  nl();
  pl(s);
  nl();

  rr=0;
  do {
    tleft(1);
    if ((syscfg.req_ratio>0.0001) && (ratio()<syscfg.req_ratio))
      rr=1;
    if (thisuser.exempt & exempt_ratio)
      rr=0;
    if (!batch[cur].sending) {
      rr=0;
      ++cur;
    }
    if ((nsl()>=batch[cur].time) && (!rr)) {
      dliscan1(batch[cur].dir);
      i=recno(batch[cur].filename);
      if (i<=0) {
        delbatch(cur);
      } else {
        sprintf(s,get_stringx(1,59),numbatchdl,ctim(batchtime));
        outs(s);
        f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
        SETREC(f,i);
        sh_read(f,(void *)&u,sizeof(uploadsrec));
        f=sh_close(f);
        sprintf(s,"%s%s",directories[batch[cur].dir].path,u.filename);
        if(directories[batch[cur].dir].mask & mask_cdrom) {
          sprintf(s1,"%s%s",
            directories[batch[cur].dir].path,u.filename);
          sprintf(s,"%s%s",
            syscfgovr.tempdir,u.filename);
          if (!exist(s))
            copy_file(s1,s);
        }
        write_inst(INST_LOC_DOWNLOAD,udir[curdir].subnum,INST_FLAGS_NONE);
        xymodem_send(s,&ok,&percent,u.filetype,1,1,1);
        if (ok) {
          downloaded(u.filename, 0);
        } else {
        }
      }
    } else
      delbatch(cur);
  } while ((ok) && (!hangup) && (numbatch>cur) && (!rr));
  if ((ok) && (!hangup))
    endbatch();
  if (rr) {
    nl();
    pl(get_string(870));
    nl();
  }
  if (had) {
    bihangup(0);
  }
}

/****************************************************************************/

void handle_dszline(char *l)
{
  char *ss;
  int i;
  long cps;
  char s[161];

  /* find the filename */
  ss=strtok(l," \t");
  for (i=0; (i<10) && (ss); i++) {
    switch(i) {
      case 4: cps=atol(ss); break;
    }
    ss=strtok(NULL," \t");
  }

  if (ss) {
    strcpy(s,stripfn(ss));
    align(s);

    switch(*l) {
      case 'Z':
      case 'r':
      case 'R':
      case 'B':
      case 'H':
        /* received a file */
        uploaded(s,cps);
        break;

      case 'z':
      case 's':
      case 'S':
      case 'b':
      case 'h':
      case 'Q':
        /* sent a file */
        downloaded(s,cps);
        break;

      case 'E':
      case 'e':
      case 'L':
      case 'l':
      case 'U':
        /* error */
        sprintf(s,get_stringx(1,60),ss);
        sysoplog(s);
        break;
    }
  }
}

/****************************************************************************/

double ratio1(long a)
{
  double r;

  if ((thisuser.dk==0) && (a==0))
    return(99.999);
  r=((float) thisuser.uk) / ((float) (thisuser.dk + a));
  if (r>99.998)
    r=99.998;
  return(r);
}

/****************************************************************************/

void make_ul_batch_list(char *listfn)
{
  int f,i;
  char s[255],s1[81];

  if(instance > 1)
    sprintf(listfn,"%s\\FILESUL.%3.3d",cdir,instance);
  else
    sprintf(listfn,"%s\\FILES.UL",cdir);

  _chmod(listfn,1,0);
  unlink(listfn);

  f=sh_open(listfn,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f<0) {
    listfn[0]=0;
    return;
  }

  for (i=0; i<numbatch; i++) {
    if (!batch[i].sending) {
      cd_to(directories[batch[i].dir].path);
      get_dir(s1,1);
      cd_to(cdir);
      sprintf(s,"%s%s\r\n",s1,stripfn(batch[i].filename));
      sh_write(f,s,strlen(s));
    }
  }
  sh_close(f);
}

/****************************************************************************/

void make_dl_batch_list(char *listfn)
{
  char s[255],s1[81],s2[81];
  int i, f, ok;
  double at=0.0;
  long addk=0,thisk;

  if(instance > 1)
    sprintf(listfn,"%s\\FILESDL.%3.3d",cdir,instance);
  else
    sprintf(listfn,"%s\\FILES.DL",cdir);

  _chmod(listfn,1,0);
  unlink(listfn);

  f=sh_open(listfn,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f<0) {
    listfn[0]=0;
    return;
  }

  for (i=0; i<numbatch; i++) {
    if (batch[i].sending) {
      if(directories[batch[i].dir].mask & mask_cdrom) {
        cd_to(syscfgovr.tempdir);
        get_dir(s1,1);
        sprintf(s,"%s%s",s1,stripfn(batch[i].filename));
        if(!exist(s)) {
          cd_to(directories[batch[i].dir].path);
          get_dir(s2,1);
          strcat(s2,stripfn(batch[i].filename));
          copy_file(s2,s);
        }
        strcat(s,"\r\n");
      } else {
        cd_to(directories[batch[i].dir].path);
        get_dir(s1,1);
        sprintf(s,"%s%s\r\n",s1,stripfn(batch[i].filename));
      }
      ok=1;
      cd_to(cdir);
      if (nsl() < (batch[i].time + at)) {
        ok=0;
        npr("%s%s%s\r\n",
            get_string(1478),
            batch[i].filename,
            get_string(1479));
      }
      thisk=bytes_to_k(batch[i].len);
      if ((syscfg.req_ratio>0.0001) && (ratio1(addk+thisk)<syscfg.req_ratio) &&
          (!(thisuser.exempt & exempt_ratio))) {
        ok=0;
        npr("%s%s%s\r\n",
            get_string(1478),
            batch[i].filename,
            get_string(1480));
      }
      if (ok) {
        sh_write(f,s,strlen(s));
        at += batch[i].time;
        addk += thisk;
      }
    }
  }
  sh_close(f);
}

/****************************************************************************/

void run_cmd(char *cmdln, char *downlist, char *uplist, char *dl, int had)
{
  char sx1[21], sx2[21], sx3[21], s[161];

  ultoa(com_speed,sx1,10);
  if ((com_speed==1) || (com_speed==49664))
    strcpy(sx1,"115200");
  ultoa(modem_speed,sx3,10);
  sx2[0]='0'+syscfgovr.primaryport;
  sx2[1]=0;
  stuff_in(s,cmdln,sx1,sx2,downlist,sx3,uplist);
  if (s[0]) {
    make_abs_cmd(s);
    clrscrb();
    outs(dl);
    outs("\r\n");
    outs(s);
    outs("\r\n");
    if (incom) {
      _chmod(dszlog,1,0);
      unlink(dszlog);

      create_chain_file();

      cd_to(syscfgovr.batchdir);

      extern_prog(s, sysinfo.spawn_opts[7]);

      cd_to(cdir);

      if (had) {
        bihangup(1);
        if (!cdet()) {
          dtr(1);
          wait1(5);
          holdphone(1);
        }
      } else {
        nl();
        pl(get_string(26));
        nl();
      }

      process_dszlog();

      topscreen();
    }
  }

  if (downlist[0]) {
    _chmod(downlist,1,0);
    unlink(downlist);
  }
  if (uplist[0]) {
    _chmod(uplist,1,0);
    unlink(uplist);
  }
}

/****************************************************************************/

void process_dszlog(void)
{
  int f,i,i1;
  char *ss;
  char **lines;

  lines = (char **)malloca(sysinfo.max_batch * sizeof(char *) * 2);

  if (!lines)
    return;

  f=sh_open1(dszlog,O_RDONLY | O_BINARY);
  if (f>0) {
    i1=(int)filelength(f);
    ss=malloca(i1);
    if (ss) {
      i=sh_read(f,ss,i1);
      if (i>0) {
        ss[i]=0;
        lines[0]=strtok(ss,"\r\n");
        for (i=1; (i<sysinfo.max_batch*2-2) && (lines[i-1]); i++)
          lines[i]=strtok(NULL,"\n");

        lines[sysinfo.max_batch*2-2]=NULL;

        for (i1=0; lines[i1]; i1++) {
          handle_dszline(lines[i1]);
        }

      }
      bbsfree(ss);
    }
    sh_close(f);
  }

  bbsfree(lines);
}

/****************************************************************************/

void dszbatchdl(int had, char *cmdln, char *desc)
{
  char listfn[81],dl[161];

  sprintf(dl,get_stringx(1,61),
          desc, numbatchdl, ctim(batchtime));
  if (had)
    strcat(dl,get_stringx(1,58));
  sysoplog(dl);
  nl();
  pl(dl);
  nl();

  write_inst(INST_LOC_DOWNLOAD,udir[curdir].subnum,INST_FLAGS_NONE);
  make_dl_batch_list(listfn);
  run_cmd(cmdln, listfn, "", dl, had);
}

/****************************************************************************/

void dszbatchul(int had, char *cmdln, char *desc)
{
  char listfn[81],dl[161];
  double ti;

  sprintf(dl,get_stringx(1,62), desc, numbatch-numbatchdl);
  if (had)
    strcat(dl,get_stringx(1,58));
  sysoplog(dl);
  nl();
  pl(dl);
  nl();

  write_inst(INST_LOC_UPLOAD,udir[curdir].subnum,INST_FLAGS_NONE);
  make_ul_batch_list(listfn);

  ti=timer();
  run_cmd(cmdln, "", listfn, dl, had);
  ti=timer()-ti;
  if (ti<0)
    ti += 24.0*3600.0;
  thisuser.extratime += ti;
}

/****************************************************************************/

void bibatch(int had, char *cmdln, char *desc)
{
  char listfn[81], listfn1[81], dl[161];
  double ti;

  sprintf(dl,get_stringx(1,63),
    desc, numbatchdl, numbatch-numbatchdl);
  if (had)
    strcat(dl,get_stringx(1,58));
  sysoplog(dl);
  nl();
  pl(dl);
  nl();

  write_inst(INST_LOC_BIXFER,udir[curdir].subnum,INST_FLAGS_NONE);
  make_ul_batch_list(listfn);
  make_dl_batch_list(listfn1);

  ti=timer();
  run_cmd(cmdln, listfn1, listfn, dl, had);
  ti=timer()-ti;
  if (ti<0)
    ti += 24.0*3600.0;
  /* Hmm... */
  /* thisuser.extratime += ti; */
}

/****************************************************************************/

void batchdl(int mode)
{
  int i,done,had,otag;
  char s[81],ch;

  done=0;
  if (numbatch==0) {
    nl();
    pl(get_string(871));
    nl();
    return;
  }
  otag=tagging;
  tagging=0;
  do {
    switch(mode) {
      case 0:
        nl();
        if (menu_on()) {
          rip_saveall();
          printmenu(9);
        } else
          prt(2,get_string(872));
        ch=onek("Q?CLRDUB");
        if (menu_on())
          rip_restoreall();
      break;
      case 1:
        listbatch();
        ch='D';
      break;
      case 2:
        listbatch();
        ch='U';
      break;
    }
    switch(ch) {
      case '?':
        printmenu(9);
        break;
      case 'Q':
        done=1;
        break;
      case 'L':
        listbatch();
        break;
      case 'R':
        nl();
        prt(2,get_string(724));
        input(s,4);
        i=atoi(s);
        if ((i>0) && (i<=numbatch)) {
          didnt_upload(i-1);
          delbatch(i-1);
        }
        if (numbatch==0) {
          nl();
          pl(get_string(873));
          nl();
          done=1;
        }
        break;
      case 'C':
        prt(5,get_string(874));
        if (yn()) {
          for (i=0; i<numbatch; i++)
            didnt_upload(i);
          numbatch=0;
          numbatchdl=0;
          batchtime=0.0;
          done=1;
          pl(get_string(875));
        }
        done=1;
        break;
      case 'U':
        if (numbatchdl==numbatch) {
          nl();
          pl(get_string(876));
          nl();
          break;
        }
        nl();
        prt(5,get_string(877));
        had=yn();
        nl();

        i=get_protocol(xf_up_batch);
        if (i>0) {

          dszbatchul(had, externs[i-6].receivebatchfn, externs[i-6].description);

          if (!had) {
            nl();
            outstr(get_string(782));
            npr("%-6.3f\r\n",ratio());
          }
        }
        done=1;
        break;
      case 'B':
        if (modem_flag & flag_as) {
          nl();
          pl(get_string(878));
          pl(get_string(879));
          pl(get_string(880));
          nl();
          prt(5,get_string(881));
          if (!yn()) {
            done=1;
            break;
          }
        }
        nl();
        prt(5,get_string(877));
        had=yn();
        nl();

        i=get_protocol(xf_bi);
        if (i>0) {

          bibatch(had, externs[i-6].bibatchfn, externs[i-6].description);

          if (!had) {
            nl();
            outstr(get_string(782));
            npr("%-6.3f\r\n",ratio());
          }
        }
        done=1;
        break;
      case 'D':
        if (numbatchdl==0) {
          nl();
          pl(get_string(882));
          nl();
          done=1;
          break;
        }

        nl();
        if (!ratio_ok()) {
          nl();
          pl(get_string(883));
          nl();
          done=1;
          break;
        }
        nl();
        prt(5,get_string(877));
        had=yn();
        nl();

        i=get_protocol(xf_down_batch);
        if (i>0) {

          if (i==4) {
            if (over_intern && (over_intern[2].othr & othr_override_internal) &&
                (over_intern[2].sendbatchfn[0]))
              dszbatchdl(had, over_intern[2].sendbatchfn, prot_name(4));
            else
              ymbatchdl(had);
          } else
            dszbatchdl(had,externs[i-6].sendbatchfn, externs[i-6].description);

          if (!had) {
            nl();
            outstr(get_string(782));
            npr("%-6.3f\r\n",ratio());
          }
        }
        done=1;
        break;
    }
  } while ((!done) && (!hangup));
  tagging=otag;
}

/****************************************************************************/


void bihangup(int up)
/* This function returns one character from either the local keyboard or
 * remote com port (if applicable).  Every second of inactivity, a
 * beep is sounded.  After 10 seconds of inactivity, the user is hung up.
 */
{
  unsigned char ch;
  long nextbeep;
  long dd;
  int color=5;

  timelastchar1=timer1();
  nextbeep=18L;
  nl();
  outstr(get_string(1481));
  nl();
  outstr(get_string(1482));
  npr("%d%d  %c",color,(int)(182L/nextbeep),7);
  do {
    while (empty() && !hangup) {
      dd = timer1();
      if (labs(dd - timelastchar1) > 65536L) {
        nextbeep -= 1572480L;
        timelastchar1 -= 1572480L;
      }
      if ((dd - timelastchar1) > nextbeep) {
        npr("\r%d%d  %c",color,(182L-nextbeep)/18L,7);
        nextbeep += 18L;
        if ((182L-nextbeep)/18L <= 6)
          color=2;
        if ((182L-nextbeep)/18L <= 3)
          color=6;
      }
      if (labs(dd - timelastchar1) > 182L) {
	nl();
	outstr(get_string(1483));
	nl();
        dtr(0);
	hangup = 1;
        if(up) {
          wait1(2);
          if (!cdet()) {
            dtr(1);
            wait1(2);
            holdphone(1);
          }
        }
      }
      giveup_timeslice();
      checkhangup();
    }
    ch = inkey();
    if ((ch=='h') || (ch=='H'))
      hangup=1;
  } while (!ch && !hangup);
}
