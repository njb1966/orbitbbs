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


#define SETREC(f,i)  sh_lseek(f,((long) (i))*((long)sizeof(uploadsrec)),SEEK_SET);

/* number of dots for searching */
#define DOTS 5

/* color for frame */
#define FRAME 7

/* How far to indent extended descriptions */
#define INDENTION 24

/* the archive type to use */
#define ARC_NUMBER 0

extern int foundany;

/****************************************************************************/

void modify_extended_description(char **sss, char *dest, char *title)
{
  char s[161],s1[161];
  int f,ii,i,i1,i2,i3,i4;

  if (*sss)
    ii=1;
  else
    ii=0;
  i4=0;
  do {
    if (ii) {
      nl();
      if (okfsed() && (sysinfo.flags & OP_FLAGS_FSED_EXT_DESC))
        prt(5,get_string(734));
      else
        prt(5,get_string(735));
      if (!yn())
        return;
    } else {
      nl();
      prt(5,get_string(736));
      if (!yn())
        return;
    }
    if (okfsed() && (sysinfo.flags & OP_FLAGS_FSED_EXT_DESC)) {
      sprintf(s,"%sEXTENDED.DSC", syscfgovr.tempdir);
      if (*sss) {
        f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
        sh_write(f,*sss,strlen(*sss));
        sh_close(f);
        bbsfree(*sss);
        *sss=NULL;
      } else
        unlink(s);
      i=thisuser.screenchars;
      if (thisuser.screenchars>(76-INDENTION))
        thisuser.screenchars=76-INDENTION;
      i1=external_edit("extended.dsc",syscfgovr.tempdir,(int) thisuser.defed-1,
        sysinfo.max_extend_lines, dest, title, 1);
      thisuser.screenchars=i;
      if (i1) {
        if ((*sss=malloca(10240))==NULL)
          return;
        f=sh_open1(s,O_RDWR | O_BINARY);
        sh_read(f,*sss,(int) filelength(f));
        (*sss)[filelength(f)]=0;
        sh_close(f);
      }
      for (i3=strlen(*sss)-1; i3>=0; i3--)
        if ((*sss)[i3]==1)
          (*sss)[i3]=' ';
    } else {
      if (*sss)
        bbsfree(*sss);
      if ((*sss=malloca(10240))==NULL)
        return;
      *sss[0]=0;
      i=1;
      nl();
      sprintf(s,"%s %d %s, %d %s",
              get_string(737), sysinfo.max_extend_lines,
              get_string(738), 78-INDENTION, get_string(739));
      pl(s);
      nl();
      s[0]=0;
      i1=thisuser.screenchars;
      if (thisuser.screenchars>(76-INDENTION))
        thisuser.screenchars=76-INDENTION;
      do {
        ansic(2);
        npr("%d: ",i);
        ansic(0);
        s1[0]=0;
        while (inli2(s1,s,90,1,i4)) {
          if (i>1)
            --i;
          itoa(i,s1,10);
          strcat(s1,": ");
          prt(2,s1);
          i2=0;
          i4-=2;
          do {
            s[i2]=*(sss[0]+i4-1);
            ++i2;
            --i4;
          } while ((*(sss[0]+i4)!=10) && (i4!=0));
          if (i4)
            ++i4;
          *(sss[0]+i4)=0;
          s[i2]=0;
          strrev(s);
          if (strlen(s)>thisuser.screenchars-1)
            s[thisuser.screenchars-2]=0;
        }
        i2=strlen(s1);
        if (i2 && (s1[i2-1]==1))
          s1[i2-1]=0;
        if (s1[0]) {
          strcat(s1,"\r\n");
          strcat(*sss,s1);
          i4 += strlen(s1);
        }
      } while ((i++<sysinfo.max_extend_lines) && (s1[0]));
      thisuser.screenchars=i1;
      if (*sss[0]==0) {
        bbsfree(*sss);
        *sss=NULL;
      }
    }
    prt(5,get_string(740));
    i=!yn();
    if (i) {
      bbsfree(*sss);
      *sss=NULL;
    }
  } while (i);
}

/****************************************************************************/

void upload(int dn)
{
  directoryrec d;
  uploadsrec u,u1;
  int i,i1,i2,i3,i4,ok,xfer,f;
  char s[255],s1[81],*ss;
  long l;
  double ti;

  if ((numbatch!=0) && (numbatch-numbatchdl>0)) {
    npr(get_string(1529));
    if (ny()) {
      batchdl(2);
      return;
    }
  }
  dliscan1(dn);
  d=directories[dn];
  if (numf>=d.maxfiles) {
    nl();
    nl();
    pl(get_string(755));
    nl();
    return;
  }
  if ((d.mask & mask_no_uploads) && (!dcs())) {
    nl();
    nl();
    pl(get_string(756));
    nl();
    return;
  }
  nl();
  l=(long)freek1(d.path);
  sprintf(s,"%s - %ldk %s.",get_string(757), l, get_string(758));
  pl(s);
  nl();
  if (l<100) {
    pl(get_string(759));
    nl();
    return;
  }
  if (menu_on()) {
    rip_saveall();
    printmenu(361);
    input(s,12);
    rip_restoreall();
  } else {
    prt(2,get_string(44));
    input(s,12);
  }
  if (!okfn(s))
    s[0]=0;
  else {
    if (!is_uploadable(s)) {
      if (so()) {
        nl();
        prt(5,get_string(1322));
        if (!yn()) {
          s[0]=0;
        }
      } else {
        nl();
        pl(get_string(1323));
        s[0]=0;
      }
    }
  }
  if (!s[0])
    return;
  align(s);
  if (strchr(s,'?')) {
    return;
  }
  if (d.mask & mask_archive) {
    ok=0;
    s1[0]=0;
    for (i=0; i<4; i++) {
      if (syscfg.arcs[i].extension[0] && syscfg.arcs[i].extension[0]!=' ') {
        if (s1[0])
          strcat(s1,", ");
        strcat(s1,syscfg.arcs[i].extension);
        if (strcmp(s+9,syscfg.arcs[i].extension)==0)
          ok=1;
      }
    }
    if (!ok) {
      nl();
      pl(get_string(760));
      pl(get_string(761));
      pl(s1);
      nl();
      return;
    }
  }
  strcpy(u.filename,s);
  u.ownerusr=usernum;
  u.ownersys=0;
  u.numdloads=0;
  u.filetype=0;
  u.mask=0;
  strcpy(u.upby,nam(&thisuser,usernum));
  strcpy(u.date,date());
  nl();
  ok=1;
  xfer=1;
  if (check_batch_queue(u.filename)) {
    ok=0;
    nl();
    pl(get_string(762));
    nl();
  } else {
    sprintf(s1,"%s '%s' %s %s? ",get_string(757), s,get_string(763),d.name);
    if (strcmp(s,"        .   "))
      prt(5,s1);
    else
      ok=0;
  }
  if ((ok) && (yn())) {
    sprintf(s1,"%s%s",d.path,s);
    if (exist(s1)) {
      if (dcs()) {
        xfer=0;
        nl();
        nl();
        pl(get_string(764));
        prt(5,get_string(765));
        if (yn()==0)
          ok=0;
      } else {
        nl();
        nl();
        pl(get_string(766));
        nl();
        ok=0;
      }
    } else
      if (!incom) {
        nl();
        pl(get_string(767));
        pl(get_string(768));
        nl();
        ok=0;
      }
    if ((d.mask & mask_PD) && (ok)) {
      nl();
      prt(5,get_string(769));
      if (!yn()) {
        nl();
        pl(get_string(770));
        pl(get_string(771));
        pl(get_string(772));
        pl(get_string(773));
        pl(get_string(774));
        nl();
        sprintf(s,get_stringx(1,41),u.filename);
        add_ass(5,s);
        ok=0;
      } else
        u.mask=mask_PD;
    }
    if (ok && (!(sysinfo.flags & OP_FLAGS_FAST_SEARCH))) {
      nl();
      pl(get_string(775));
      nl();
      i2=0;
      for (i=0; (i<num_dirs) && (udir[i].subnum!=-1); i++) {
        strcpy(s,get_string(776));
        strcat(s,directories[udir[i].subnum].name);
        for (i3=i4=strlen(s); i3<i2; i3++) {
          s[i3]=' ';
          s[i3+1]=0;
        }
        i2=i4;
        npr("%s\r",s);

        dliscan1(udir[i].subnum);
        i1=recno(u.filename);
        if (i1>=0) {
          nl();
          outstr(get_string(777));
          pl(directories[udir[i].subnum].name);
          if (dcs()) {
            nl();
            prt(5,get_string(778));
            if (!yn()) {
              ok=0;
              break;
            }
            nl();
          } else {
            ok=0;
            break;
          }
        }
      }
      for (i1=0; i1<i2; i1++)
        s[i1]=' ';
      s[i1]=0;
      npr("%s\r",s);
      if (ok)
        dliscan1(dn);
      nl();
    }
    if (ok) {
      nl();
      pl(get_string(779));
      outstr(": ");
      inputl(u.description,58);
      nl();
      ss=NULL;
      modify_extended_description(&ss, directories[dn].name,u.filename);
      if (ss) {
        add_extended_description(u.filename,ss);
        u.mask |= mask_extended;
        bbsfree(ss);
      }
      nl();
      if (xfer) {
        write_inst(INST_LOC_UPLOAD,udir[curdir].subnum,INST_FLAGS_NONE);
        ti=timer();
        receive_file(s1,&ok,&u.filetype, u.filename, dn);
        ti=timer()-ti;
        if (ti<0)
          ti += 24.0*3600.0;
        thisuser.extratime += ti;
      }
      if (ok) {
        if (ok==1) {
          f=sh_open1(s1,O_RDONLY | O_BINARY);
          if (f<0) {
            ok=0;
            nl();
            nl();
            pl(get_string(780));
            nl();
            if (u.mask & mask_extended)
              delete_extended_description(u.filename);
          }
          if (ok && syscfg.upload_c[0]) {
            sh_close(f);
            pl(get_string(26));
            if (check_ul_event(dn,&u)) {
              if (u.mask & mask_extended)
                delete_extended_description(u.filename);
              ok=0;
            } else {
              f=sh_open1(s1,O_RDONLY | O_BINARY);
            }
          }
        }
        if (ok) {
          if (ok==1) {
            l=filelength(f);
            u.numbytes=l;
            sh_close(f);
            ++thisuser.uploaded;
            modify_database(u.filename,1);
            thisuser.uk += bytes_to_k(l);
            get_file_idz(&u,dn);
            if (sysinfo.flags & OP_FLAGS_PACKSCAN_FREQ)
              remotenotify(u.filename,u.description);
          } else
            u.numbytes=0;
          time(&l);
          u.daten=l;
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
          if (ok==1) {
            lock_status();
            ++status.uptoday;
            ++status.filechange[filechange_upload];
            save_status();
            sprintf(s,get_stringx(1,42),u.filename,directories[dn].name);
            sysoplog(s);
            nl();
            nl();
            pl(get_string(781));
            nl();
            outstr(get_string(782));
            npr("%-6.3f\r\n", ratio());
            nl();
            nl();
            if (useron)
              topscreen();
          }
        }
      } else {
        nl();
        nl();
        pl(get_string(783));
        nl();
        if (u.mask & mask_extended)
          delete_extended_description(u.filename);
      }
    }
  }
}


/****************************************************************************/

/****************************************************************************/

void get_file_idz(uploadsrec *u, int dn)
{
  char *b,*ss,cmd[161],s[81];
  int f,i,ok=0;

  if (sysinfo.flags & OP_FLAGS_READ_CD_IDZ) {
    if (directories[dn].mask & mask_cdrom)
      return;
  }
  ss=strchr(stripfn(u->filename),'.');
  if (ss==NULL)
    return;
  ++ss;
  for (i=0; i<4; i++)
    if (!ok)
      ok=(stricmp(ss,syscfg.arcs[i].extension)==0);
  if (!ok)
    return;

  sprintf(s,"%sFILE_ID.DIZ",syscfgovr.tempdir);
  unlink(s);
  sprintf(s,"%sDESC.SDI",syscfgovr.tempdir);
  unlink(s);

  cd_to(directories[dn].path);
  get_dir(s,1);
  strcat(s,stripfn(u->filename));
  cd_to(cdir);
  get_arc_cmd(cmd,s,1,"FILE_ID.DIZ DESC.SDI");
  cd_to(syscfgovr.tempdir);
  extern_prog(cmd, (sysinfo.spawn_opts[9]&EFLAG_SHRINK) |EFLAG_NOHUP);
  cd_to(cdir);
  sprintf(s,"%sFILE_ID.DIZ",syscfgovr.tempdir);
  if (!exist(s))
    sprintf(s,"%sDESC.SDI",syscfgovr.tempdir);
  if (exist(s)) {
    nl();
    ansic(9); npr(get_string(995));
    ansic(2); outstr(stripfn(s));
    ansic(9); npr(get_string(996));
    ss=read_extended_description(u->filename);
    if (ss) {
      bbsfree(ss);
      delete_extended_description(u->filename);
    }
    if ((b=malloca((long)sysinfo.max_extend_lines*256+1))==NULL)
      return;
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if (filelength(f)<(sysinfo.max_extend_lines*256)) {
      sh_read(f,b,(int) filelength(f));
      b[filelength(f)]=0;
    } else {
      sh_read(f,b,(int)sysinfo.max_extend_lines*256);
      b[(int)sysinfo.max_extend_lines*256]=0;
    }
    sh_close(f);
    if (sysinfo.flags & OP_FLAGS_IDZ_DESC) {
      ss=strtok(b,"\n");
      sprintf(u->description,"   %.56s",ss);
      ss=strtok(NULL,"");
    } else {
      ss=b;
    }
    if (ss) {
      for (i=strlen(ss)-1; i>0; i--) {
        if ((ss[i]==26) || (ss[i]==12))
          ss[i]=32;
      }
      add_extended_description(u->filename,ss);
      u->mask |= mask_extended;
    }
    bbsfree(b);
    pl(get_string(997));
  }
  sprintf(s,"%sFILE_ID.DIZ",syscfgovr.tempdir);
  unlink(s);
  sprintf(s,"%sDESC.SDI",syscfgovr.tempdir);
  unlink(s);
}


void read_idz(void)
{
  char s[81],s1[161];
  int i,f,abort=0,next=0;
  uploadsrec u;

  tmp_disable_pause(1);
  set_protect(0);
  dliscan();
  file_mask(s);
  sprintf(s1,"9%s2%s #%s...",get_string(998),
     directories[udir[curdir].subnum].name,
     udir[curdir].keys);
  pl(s1);
  f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  for (i=1; (i<=numf) && (!hangup) && !abort; i++) {
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    if (compare(s,u.filename))
      get_file_idz(&u,udir[curdir].subnum);
    SETREC(f,i);
    sh_write(f,(void *)&u,sizeof(uploadsrec));
    checka(&abort, &next);
  }
  f=sh_close(f);
  topscreen();
  tmp_disable_pause(0);
}


void tag_it(void)
{
  int i,i2,i3,i4,bad,fp;
  char s[161],s1[161],s2[81],s3[400];
  double t;
  long fs;

  if (numbatch>=sysinfo.max_batch) {
    prt(6,get_string(1325));
    getkey();
    return;
  }
  outstr(get_string(1326));
  npr("%d",tagptr);
  outstr(get_string(1327));
  mpl(30);
  input(s3,30);
  if (s3[0]=='*') {
    s3[0]=0;
    for(i2=0;i2<tagptr && i2<78;i2++) {
      sprintf(s2,"%d ",i2+1);
      strcat(s3,s2);
      if (strlen(s3)>sizeof(s3)-10)
        break;
    }
    nl();
    outstr(get_string(1328));
    pl(s3);
  }
  for(i2=0;i2<strlen(s3);i2++) {
    sprintf(s1,"%s",s3+i2);
    i4=0;
    bad=0;
    for(i3=0;i3<strlen(s1);i3++) {
      if ((s1[i3]==' ') || (s1[i3]==',') || (s1[i3]==';')) {
        s1[i3]=0;
        i4=1;
      } else {
        if (i4==0)
          i2++;
      }
    }
    i=atoi(s1);
    if (i==0)
      break;
    i--;
    if ((s1[0]) && (i>=0) && (i<tagptr)) {
      if (check_batch_queue(filelist[i].u.filename)) {
        ansic(6);
        outstr(filelist[i].u.filename);
        pl(get_string(1329));
        bad=1;
      }
      if (numbatch>=sysinfo.max_batch) {
        ansic(6);
        outstr(get_string(1330));
        npr("%d",sysinfo.max_batch);
        pl(get_string(1331));
        bad=1;
      }
      if ((syscfg.req_ratio>0.0001)
        && (ratio()<syscfg.req_ratio)
        && (!thisuser.exempt & exempt_ratio)
        && (bad==0)) {
        ansic(2);
        outstr(get_string(730));
        sprintf(s," %-5.3f.  ",ratio());
        outstr(s);
        outstr(get_string(731));
        sprintf(s," %-5.3f ",syscfg.req_ratio);
        outstr(s);
        outstr(get_string(732));
        nl();
        bad=1;
      }
      if (bad==0) {
        sprintf(s,"%s%s",directories[filelist[i].directory].path,
          stripfn(filelist[i].u.filename));
        if (filelist[i].dir_mask & mask_cdrom) {
          sprintf(s2,"%s%s",
            directories[filelist[i].directory].path,
            stripfn(filelist[i].u.filename));
          sprintf(s,"%s%s",
            syscfgovr.tempdir,stripfn(filelist[i].u.filename));
         if (!exist(s))
           copy_file(s2,s);
        }
        fp=sh_open1(s,O_RDONLY | O_BINARY);
        if (fp<0) {
          ansic(6);
          outstr(get_string(1332));
          outstr(stripfn(filelist[i].u.filename));
          pl(get_string(1333));
          bad=1;
        } else {
          fs=filelength(fp);
          fp=sh_close(fp);
        }
      }
      if (bad==0) {
        t=(12.656) / ((double) (modem_speed)) * ((double)(fs));
        if (nsl()<=(batchtime + t)) {
          ansic(6);
          outstr(get_string(1334));
          outstr(filelist[i].u.filename);
          pl(".");
          bad=1;
        }
      }
      if (bad==0) {
        batchtime += t;
        strcpy(batch[numbatch].filename,filelist[i].u.filename);
        batch[numbatch].dir=filelist[i].directory;
        batch[numbatch].time=t;
        batch[numbatch].sending=1;
        batch[numbatch].len=fs;
        numbatch++;
        ++numbatchdl;
        ansic(1);
        outstr(filelist[i].u.filename);
        pl(get_string(1335));
      }
    } else {
      ansic(6);
      outstr(get_string(1336));
      npr("%d.",i+1);
      nl();
    }
    lines_listed=0;
  }
}


void tag_files(void)
{
  int i, i1, i2, done=0, abort, had, oh, fc, ohl;
  char s[161],s1[161],s2[81],ch;
  double d;

  fc=thisuser.sysstatus & sysstatus_extra_color;
  if ((lines_listed==0) || (tagging==0) || (num_listed==0))
    return;
  abort=0;
  if ((x_only) || (tagging==2)) {
    tagptr=0;
    return;
  }
  tleft(1);
  if (hangup)
    return;
  if (thisuser.sysstatus & sysstatus_no_tag) {
    if (thisuser.sysstatus & sysstatus_pause_on_page)
      pausescr();
    ansic(fc ? FRAME : 0);
    if (okansi())
      npr("\r%s\r\n",get_string(1315));
    else
      npr("\r%s\r\n",get_string(1338));
    tagptr=0;
    return;
  }
  if (menu_on())
    printmenu(17);

  lines_listed=0;
  ansic(fc ? FRAME : 0);
  if (okansi())
    npr("\r%s\r\n",get_string(1313));
  else
    npr("\r%s\r\n",get_string(1340));

  oh=helpl;
  helpl=43;
  done=0;
  while ((!done) && (!hangup)) {
    ohl=helpl;
    helpl=43;
    lines_listed=0;
    ch=fancy_prompt(get_string(1341),"CDEMNQRTV?");
    lines_listed=0;
    helpl=ohl;
    switch (ch) {
      case '?':
        i=tagging;
        tagging=0;
        printmenu(17);
        pausescr();
        tagging=i;
        relist();
      break;
      case 'C':
      case ' ':
      case '\r':
        lines_listed=0;
        tagptr=0;
        titled=2;
        outchr(12);
        done=1;
      break;
      case 'D':
        batchdl(1);
        tagging=0;
        if (!had) {
          nl();
          pausescr();
          outchr(12);
        }
        done=1;
      break;
      case 'E':
        lines_listed=0;
        i1=tagging;
        tagging=0;
        prt(2,get_string(1342));
        npr("2%d)? ",tagptr);
        mpl(2);
        input(s,2);
        i=atoi(s)-1;
        if ((s[0]) && (i>=0) && (i<tagptr)) {
          d=XFER_TIME(filelist[i].u.numbytes);
          nl();
          if (menu_on()) {
            rip_printfileinfo(&filelist[i].u,filelist[i].directory);
            getkey();
            rip_restoreall();
          } else {
	    for (i2=0; i2<num_dirs; i2++) {
	      if (udir[i2].subnum==filelist[i].directory)
		break;
	    }
	    if (i2<num_dirs) {
	      npr("%s%s, %s\r\n",
		  get_string(1343),
		  udir[i2].keys,
		  directories[filelist[i].directory].name);
	    } else {
	      npr("%s%s, %s\r\n",
		  get_string(1343),
		  "??",
		  directories[filelist[i].directory].name);
	    }
            ansic(1);
            outstr(get_string(746));
            ansic(2);
            pl(filelist[i].u.filename);
            ansic(1);
            outstr(get_string(747));
            ansic(2);
            pl(filelist[i].u.description);
            if (filelist[i].u.mask & mask_extended) {
              strcpy(s1,edlfn);
              sprintf(edlfn,"%s%s.EXT",syscfg.datadir,
                directories[filelist[i].directory].filename);
              zap_ed_info();
              npr(get_string(1344));
              print_extended(filelist[i].u.filename,&abort,sysinfo.max_extend_lines,2);
              zap_ed_info();
              strcpy(edlfn, s1);
            }
            ansic(1);
            outstr(get_string(748));
            ansic(2);
            npr("%dk\r\n", bytes_to_k(filelist[i].u.numbytes));
            ansic(1);
            outstr(get_string(749));
            ansic(2);
            pl(ctim(d));
            ansic(1);
            outstr(get_string(750));
            ansic(2);
            pl(filelist[i].u.date);
            ansic(1);
            outstr(get_string(751));
            ansic(2);
            pl(filelist[i].u.upby);
            ansic(1);
            outstr(get_string(752));
            ansic(2);
            pln(filelist[i].u.numdloads);
            if (directories[filelist[i].directory].mask & mask_cdrom) {
              nl();
              pl(get_string(1345));
            } else {
              sprintf(s,"%s%s",directories[filelist[i].directory].path,
                filelist[i].u.filename);
              if (!exist(s)) {
                nl();
                pl(get_string(754));
              }
            }
            nl();
            pausescr();
            relist();
          }
        }
        tagging=i1;
      break;
      case 'N':
        tagging=2;
        done=1;
      break;
      case 'M':
        if (dcs()) {
          i=tagging;
          tagging=0;
            move_file_t();
          tagging=i;
          if (num_listed==0) {
            done=1;
            return;
          }
          relist();
        }
      break;
      case 'Q':
        tagging=0;
        titled=0;
        tagptr=0;
        lines_listed=0;
        done=1;
        return;
      case 'R':
        relist();
      break;
      case 'T':
        tag_it();
      break;
      case 'V':
        prt(2,get_string(1342));
        npr("2%d)? ",tagptr);
        mpl(2);
        input(s,2);
        i=atoi(s)-1;
        if ((s[0]) && (i>=0) && (i<tagptr)) {
          sprintf(s1,"%s%s",directories[filelist[i].directory].path,
            stripfn(filelist[i].u.filename));
          if (directories[filelist[i].directory].mask & mask_cdrom) {
            sprintf(s2,"%s%s",directories[filelist[i].directory].path,
              stripfn(filelist[i].u.filename));
            sprintf(s1,"%s%s",syscfgovr.tempdir,
               stripfn(filelist[i].u.filename));
            if (!exist(s1))
              copy_file(s2,s1);
          }
          if (!exist(s1)) {
            prt(6,get_string(1346));
            nl();
            pausescr();
            break;
          }
          get_arc_cmd(s,s1,0,"");
          if (!okfn(stripfn(filelist[i].u.filename)))
            s[0]=0;
          if (s[0]!=0) {
            nl();
            tagging=0;
            extern_prog(s, sysinfo.spawn_opts[10]);
            nl();
            pausescr();
            tagging=1;
            topscreen();
            outchr(12);
            relist();
          } else {
            prt(6,get_string(1347));
            nl();
            pausescr();
            break;
          }
        }
      break;
      default:
        outchr(12);
        done=1;
      break;
    }
  }
  helpl=oh;
  tagptr=0;
  lines_listed=0;
}

/****************************************************************************/


int try_to_download(char *s, int dn,int title)
{
  int i,ok,sent,abort=0,next=0,i1,f;
  uploadsrec u;
  char s1[81],s2[81];
  userrec ur;

  dliscan1(dn);
  i=recno(s);
  if (i<=0) {
    abort=next=0;
    checka(&abort,&next);
    if (abort)
      return(-1);
    else
      return(0);
  }
  ok=1;
  foundany=1;
  while ((i>0) && (ok) && (!hangup)) {
    tleft(1);
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    f=sh_close(f);
    nl();
    if (title) {
      outstr(get_string(784));
      pl(directories[dn].name);
    }
    if (menu_on()) {
      i1 = rip_printfileinfo(&u,dn);
    } else
      i1=printfileinfo(&u,dn);
    if ((strncmp(u.filename,"WWIV4",5)==0) &&
        (!(sysinfo.flags & OP_FLAGS_NO_EASY_DL)))
      i1=1;
    else {
      if (!ratio_ok()) {
        if (menu_on())
          rip_restoreall();
        return(-1);
      }
    }
    if (i1) {
      write_inst(INST_LOC_DOWNLOAD,udir[curdir].subnum,INST_FLAGS_NONE);
      sprintf(s1,"%s%s",directories[dn].path,u.filename);
      if (directories[dn].mask & mask_cdrom) {
        sprintf(s2,"%s%s",directories[dn].path,u.filename);
        sprintf(s1,"%s%s",syscfgovr.tempdir,u.filename);
        if (!exist(s1))
          copy_file(s2,s1);
      }
      sent=0;
      abort=0;
      if (i1==-1)
        send_file(s1,&sent,&abort,u.filetype,u.filename,dn, -2L);
      else
        send_file(s1,&sent,&abort,u.filetype,u.filename,dn, u.numbytes);
      if (sent) {
        ++thisuser.downloaded;
        thisuser.dk += (int) (bytes_to_k(u.numbytes));
        ++u.numdloads;
        f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
        SETREC(f,i);
        sh_write(f,(void *)&u,sizeof(uploadsrec));
        f=sh_close(f);
        sprintf(s1,get_stringx(1,43),u.filename);
        sysoplog(s1);
        nl();
        nl();
        outstr(get_string(782)); npr("%-6.3f\r\n", ratio());
        if (syscfg.sysconfig & sysconfig_log_dl) {
          read_user(u.ownerusr, &ur);
          if (!(ur.inact & inact_deleted)) {
            if (date_to_daten(ur.firston) < u.daten) {
              sprintf(s1,"%s %s '%s' %s %s",
                nam(&thisuser,usernum), get_string(785), u.filename, get_string(786),date());
              ssm(u.ownerusr,0,s1);
            }
          }
        }
        if (useron)
          topscreen();
      }
    } else {
      nl();
      nl();
      pl(get_string(787));
      nl();
    }
    if (abort)
      ok=0;
    else
      i=nrecno(s,i);
  }
  if (menu_on())
    rip_restoreall();
  if (abort)
    return(-1);
  else
    return(1);
}
/****************************************************************************/


void download(void)
{
  char s[81];
  int dn,count,color;

  if (numbatchdl!=0) {
    nl();
    outstr(get_string(1324));
    if (ny()) {
      batchdl(1);
      return;
    }
  }
  if (menu_on()) {
    rip_saveall();
    printmenu(360);
    input(s,12);
    rip_restoreall();
  } else {
    nl();
    pl(get_string(788));
    nl();
    prt(2,get_string(7));
    input(s,12);
  }
  if (s[0]==0)
    return;
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  align(s);
  if (try_to_download(s,udir[curdir].subnum,0)==0) {
    nl();
    pl(get_string(789));
    nl();
    foundany=dn=0;
    count=0;
    color=3;
    if (!x_only) {
       outstr("\r");
       outstr(get_string(1184));
    }
    while ((dn<num_dirs) && (udir[dn].subnum!=-1)) {
      count++;
      if (!x_only) {
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
      if (try_to_download(s,udir[dn].subnum,1)<0)
        break;
      else
        dn++;
    }
    if (!foundany) {
      pl(get_string(89));
      nl();
    }
  }
}

/****************************************************************************/


char fancy_prompt(char *prompt,char *accept)
{
  int i,i1;
  char s1[81],s2[81],s3[81];
  char ch;

  tleft(1);
  sprintf(s1,"\r2%s (1%s2)? 0",prompt,accept);
  sprintf(s2,"%s (%s)? ",prompt,accept);
  i1=strlen(s2);
  sprintf(s3,"%s%s",accept," \r");
  tleft(1);
  if (okansi()) {
    outstr(s1);
    ch=onek1(s3);
    npr("\x1b[%dD",i1);
    for (i=0; i<i1; i++)
      outchr(' ');
    npr("\x1b[%dD",i1);
  } else {
    outstr(s2);
    ch=onek1(s3);
    for (i = 0; i<i1; i++)
      backspace();
  }
  return(ch);
}


void endlist(int mode)
{
  char s[81];
  int fc;

  fc=thisuser.sysstatus & sysstatus_extra_color;
  if (tagging!=0) {
    if (num_listed)  {
      if ((tagging==1) && (!(thisuser.sysstatus&sysstatus_no_tag)) && filelist) {
        tag_files();
        return;
      } else {
        ansic(fc ? FRAME : 0);
        if ((titled!=2) && (tagging==1)
          && !(thisuser.sysstatus & sysstatus_no_tag)) {
          if (okansi())
            npr("\r%s\r\n",get_string(1313));
          else
            npr("\r%s\r\n",get_string(1349));
        } else {
          if (okansi())
            npr("\r%s\r\n",get_string(1315));
          else
            npr("\r%s\r\n",get_string(1351));
        }
      }
      switch(mode) {
        case 1:
          sprintf(s,"\r%s %ld",get_string(744), num_listed);
        break;
        case 2:
          sprintf(s,"\r%s %ld",get_string(744), num_listed);
        break;
      }
      pl(s);
    } else {
      switch(mode) {
        case 1:
          outstr("\r");
          pl(get_string(1352));
          nl();
          break;
        case 2:
          outstr("\r");
          pl(get_string(1353));
          nl();
          break;
      }
    }
  }
}


void yourinfodl(void)
{
  nl();
  ansic_x(1); outstr(get_string(796));
  ansic_x(2); npr("%ldk",thisuser.uk);
  ansic_x(1); outstr(get_string(797));
  ansic_x(2); npr("%d",thisuser.uploaded);
  pl(get_string(798));
  ansic_x(1); outstr(get_string(799));
  ansic_x(2); npr("%ldk",thisuser.dk);
  ansic_x(1); outstr(get_string(797));
  ansic_x(2); npr("%d",thisuser.downloaded);
  pl(get_string(798));
  ansic_x(1); outstr(get_string(800));
  ansic_x(2); npr("%-6.3f\r\n",ratio());
  ansic_x(1); outstr(get_string(801));
  ansic_x(2); pln(thisuser.dsl);
}


void setldate(void)
{
  struct date d;
  struct time t;
  char s[81];
  int m,dd,y;

  nl();
  nl();
  unixtodos(nscandate,&d,&t);
  nl();
  outstr(get_string(790));
  npr("%02d/%02d/%02d\r\n",d.da_mon,d.da_day,
    (d.da_year-2000) > 0 ? (d.da_year-2000) : (d.da_year-1900));
  nl();
  pl(get_string(791));
  pl(get_string(792));
  outstr(":");
  input(s,8);
  m=atoi(s);
  dd=atoi(&(s[3]));
  y=atoi(&(s[6]))+1900;                          /* fixed for 1980-2099 */
  if (s[6]<'8')
    y+=100;
  if ((strlen(s)==8) && (m>0) && (m<=12) && (dd>0) && (dd<32) && (y>=1980)) {
    t.ti_min=0;
    t.ti_hour=0;
    t.ti_hund=0;
    t.ti_sec=0;
    d.da_year=y;
    d.da_day=dd;
    d.da_mon=m;
    nl();
    outstr(get_string(790));
    npr("%02d/%02d/%02d\r\n",m,dd,
      (d.da_year-2000) > 0 ? (d.da_year-2000) : (d.da_year-1900));
    nl();
    nscandate=dostounix(&d,&t);
  }
}

/****************************************************************************/
