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

/* the archive type to use */
#define ARC_NUMBER 0


/* strings not to allow in a .zip file to extract from */
char *bad_words[] = {
  "COMMAND",
  "..",
  NULL,
};

long bad_filename(char *fn)
{
  int i;

  strupr(fn);
  for (i=0; bad_words[i]; i++) {
    if (strstr(fn,bad_words[i])) {
      outstr(get_string(840));
      pl(fn);
      return(1);
    }
  }
  if (!okfn(fn)) {
    outstr(get_string(840));
    pl(fn);
    return(1);
  }
  return(0);
}

/****************************************************************************/

typedef struct {
  unsigned char type;
  char name[13];
  long len;
  int date,time,crc;
  long size;
} arch;

int check_for_files_arc(char *fn)
{
  int f;
  char s[20];
  arch a;
  long l1,l;

  f=sh_open1(fn,O_RDONLY | O_BINARY);
  if (f>0) {
    l1=filelength(f);
    l=1;
    sh_lseek(f,0L,SEEK_SET);
    sh_read(f,&a,1);
    if (a.type!=26) {
      sh_close(f);
      outstr(stripfn(fn));
      pl(get_string(841));
      return(1);
    }
    while (l<l1) {
      sh_lseek(f,l,SEEK_SET);
      if ((sh_read(f,&a,sizeof(arch)))==sizeof(arch)) {
	l += sizeof(arch);
	if (a.type==1) {
	  l-=4;
	  a.size=a.len;
	}
	if (a.type) {
	  l += a.len;
	  ++l;
	  strncpy(s,a.name,13);
	  s[13]=0;
	  if (bad_filename(s)) {
	    sh_close(f);
	    return(1);
	  }
	} else
	  l=l1;
      } else {
        sh_close(f);
        if (a.type!=0) {
          outstr(stripfn(fn));
          outstr(get_string(841));
          return(1);
        } else
          l=l1;
      }
    }

    sh_close(f);
    return(0);
  }
  outstr(get_string(842));
  pl(stripfn(fn));
  return(1);
}

/****************************************************************************/

/* .ZIP structures and defines */
#define ZIP_LOCAL_SIG 0x04034b50
#define ZIP_CENT_START_SIG 0x02014b50
#define ZIP_CENT_END_SIG 0x06054b50

typedef struct {
  unsigned long         signature;    /* 0x04034b50 */
  unsigned short        extract_ver;
  unsigned short        flags;
  unsigned short        comp_meth;
  unsigned short        mod_time;
  unsigned short        mod_date;
  unsigned long         crc_32;
  unsigned long         comp_size;
  unsigned long         uncomp_size;
  unsigned short        filename_len;
  unsigned short        extra_length;
} zip_local_header;

typedef struct {
  unsigned long         signature;    /* 0x02014b50 */
  unsigned short        made_ver;
  unsigned short        extract_ver;
  unsigned short        flags;
  unsigned short        comp_meth;
  unsigned short        mod_time;
  unsigned short        mod_date;
  unsigned long         crc_32;
  unsigned long         comp_size;
  unsigned long         uncomp_size;
  unsigned short        filename_len;
  unsigned short        extra_len;
  unsigned short        comment_len;
  unsigned short        disk_start;
  unsigned short        int_attr;
  unsigned long         ext_attr;
  unsigned long         rel_ofs_header;
} zip_central_dir;


typedef struct {
  unsigned long         signature;    /* 0x06054b50 */
  unsigned short        disk_num;
  unsigned short        cent_dir_disk_num;
  unsigned short        total_entries_this_disk;
  unsigned short        total_entries_total;
  unsigned long         central_dir_size;
  unsigned long         ofs_cent_dir;
  unsigned short        comment_len;
} zip_end_dir;


int check_for_files_zip(char *fn)
{
  int f;
  long l,sig,len;
  zip_local_header zl;
  zip_central_dir zc;
  zip_end_dir ze;
  char s[161];

#define READ_FN(ln) {sh_read(f,s,ln); s[ln]=0;}

  f=sh_open1(fn,O_RDONLY | O_BINARY);
  if (f>0) {
    l=0;
    len=filelength(f);
    while (l<len) {
      sh_lseek(f,l,SEEK_SET);
      sh_read(f,&sig, 4);
      sh_lseek(f,l,SEEK_SET);
      switch(sig) {
        case ZIP_LOCAL_SIG:
          sh_read(f,&zl, sizeof(zl));
          READ_FN(zl.filename_len);
          if (bad_filename(s)) {
            sh_close(f);
            return(1);
          }
          l += sizeof(zl);
          l += zl.comp_size + zl.filename_len + zl.extra_length;
          break;
        case ZIP_CENT_START_SIG:
          sh_read(f,&zc, sizeof(zc));
          READ_FN(zc.filename_len);
          if (bad_filename(s)) {
            sh_close(f);
            return(1);
          }
          l += sizeof(zc);
          l += zc.filename_len + zc.extra_len;
          break;
        case ZIP_CENT_END_SIG:
          sh_read(f,&ze, sizeof(ze));
          sh_close(f);
          return(0);
        default:
          sh_close(f);
          pl(get_string(843));
          return(1);
      }
    }
    sh_close(f);
    return(0);
  }

  outstr(get_string(842));
  pl(stripfn(fn));
  return(1);
}

/****************************************************************************/

typedef struct {
  unsigned char   checksum;
  char            ctype[5];
  long            comp_size;
  long            uncomp_size;
  unsigned short  time;
  unsigned short  date;
  unsigned short  attr;
  unsigned char   fn_len;
} lharc_header;



int check_for_files_lzh(char *fn)
{
  long l1,l;
  int f,err=0;
  unsigned short crc;
  char s[256];
  char flag;
  lharc_header a;

  if ((f=sh_open1(fn,O_RDONLY|O_BINARY))<0) {
    outstr(get_string(842));
    pl(stripfn(fn));
    return(1);
  }
  l1=filelength(f);
  for(l=0;l<l1;l+=a.fn_len+a.comp_size+sizeof(lharc_header)+sh_read(f,&crc,sizeof(crc))+1) {
    sh_lseek(f,l,SEEK_SET);
    sh_read(f,&flag,1);
    if (!flag) {
      l=l1;
      break;
    }
    if ((sh_read(f,&a,sizeof(lharc_header)))!=sizeof(lharc_header)) {
      outstr(stripfn(fn));
      pl(get_string(844));
      err=1;
      break;
    }
    if (sh_read(f,s,a.fn_len)!=a.fn_len) {
      outstr(stripfn(fn));
      pl(get_string(844));
      err=1;
      break;
    }
    s[a.fn_len]=0;
    if (bad_filename(s)) {
      err=1;
      break;
    }
  }
  sh_close(f);
  return(err);
}
/****************************************************************************/

int check_for_files_arj(char *fn)
{
  int f,i;
  char s[256];
  long l1,l,l2;
  unsigned short sh;
  unsigned char s1;


  f=sh_open1(fn,O_RDONLY | O_BINARY);
  if (f>0) {
    l1=filelength(f);
    l=0;
    sh_lseek(f,0L,SEEK_SET);
    while (l<l1) {
      sh_lseek(f,l,SEEK_SET);
      i=sh_read(f,&sh,2);
      if ((i!=2) || (sh!=0xea60)) {
        sh_close(f);
        outstr(stripfn(fn));
        outstr(get_string(845));
        return(1);
      }
      l+=i+2;
      sh_read(f,&sh,2);
      sh_read(f,&s1,1);
      sh_lseek(f,l+12,SEEK_SET);
      sh_read(f,&l2,4);
      sh_lseek(f,l+(long)s1,SEEK_SET);
      sh_read(f,s,250);
      s[250]=0;
      if (strlen(s)>240) {
        sh_close(f);
        outstr(stripfn(fn));
        outstr(get_string(845));
        return(1);
      }
      l += 4+(long) sh;
      sh_lseek(f,l,SEEK_SET);
      sh_read(f,&sh,2);
      l += 2;
      while ((l<l1) && (sh)) {
        l += 6+(long) sh;
        sh_lseek(f,l-2,SEEK_SET);
        sh_read(f,&sh,2);
      }
      l += l2;
      if (bad_filename(s)) {
        sh_close(f);
        return(1);
      }
    }

    sh_close(f);
    return(0);
  }
  outstr(get_string(842));
  pl(stripfn(fn));
  return(1);
}



/****************************************************************************/


typedef struct {
  char *arc_name;
  int (*func)(char *);
} arc_testers;

arc_testers arc_t[] = {
  {"ZIP", check_for_files_zip},
  {"ARC", check_for_files_arc},
  {"LZH", check_for_files_lzh},
  {"ARJ", check_for_files_arj},
  {NULL, NULL},
};



int check_for_files(char *fn)
{
  char *ss;
  int i;

  ss=strrchr(fn,'.');
  if (ss) {
    ss++;
    for (i=0; arc_t[i].arc_name; i++) {
      if (stricmp(ss,arc_t[i].arc_name)==0) {
        return(arc_t[i].func(fn));
      }
    }
  } else {
    /* no extension? */
    pl(get_string(846));
    return(1);
  }
  return(0);
}


void download_temp_arc(char *fn, int xfer)
{
  char s[81],s1[81];
  long numbytes;
  int f,sent,abort;
  double d;

  outstr(get_string(847));
  npr("%s.%s:\r\n\r\n", fn, syscfg.arcs[ARC_NUMBER].extension);
  if (xfer && !ratio_ok()) {
    pl(get_string(848));
    return;
  }
  sprintf(s1,"%s%s.%s",syscfgovr.tempdir,fn, syscfg.arcs[ARC_NUMBER].extension);
  f=sh_open1(s1,O_RDONLY | O_BINARY);
  if (f<0) {
    pl(get_string(849));
    nl();
    return;
  }
  numbytes=filelength(f);
  sh_close(f);
  if (numbytes==0L) {
    pl(get_string(850));
    nl();
    return;
  }
  d=XFER_TIME(numbytes);
  if (d<=nsl()) {
    outstr(get_string(851));
    pl(ctim(d));
    sent=0;
    abort=0;
    sprintf(s,"%s.%s",fn,syscfg.arcs[ARC_NUMBER].extension);
    send_file(s1,&sent,&abort,0,s,-1,numbytes);
    if (sent) {
      if (xfer) {
        ++thisuser.downloaded;
        thisuser.dk += bytes_to_k(numbytes);
        nln(2);
        outstr(get_string(782)); npr("%-6.3f\r\n",ratio());
      }
      sprintf(s1,get_stringx(1,45),bytes_to_k(numbytes), s);
      sysoplog(s1);
      if (useron)
        topscreen();
    }
  } else {
    nln(2);
    pl(get_string(787));
    nl();
  }
}

void add_arc(char *arc, char *fn, int dos)
{
  char s[255], s1[81];

  sprintf(s1,"%s.%s",arc, syscfg.arcs[ARC_NUMBER].extension);
  get_arc_cmd(s,s1,2,fn);
  if (s[0]) {
    cd_to(syscfgovr.tempdir);
    outs(s);
    outs("\r\n");
    if (dos) {
      extern_prog(s, sysinfo.spawn_opts[11]);
    } else {
      extern_prog(s, 0);
      topscreen();
    }
    cd_to(cdir);
    sprintf(s,get_stringx(1,46),fn, s1);
    sysoplog(s);
  } else
    pl(get_string(852));
}

void add_temp_arc(void)
{
  char s[81],s2[81];
  int i;

  nl();
  pl(get_string(853));
  pl(get_string(854));
  prt(2,": ");
  input(s,12);
  if (!okfn(s))
    return;
  if (s[0]==0)
    return;
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  strcpy(s2,stripfn(s));
  for (i=0; i<strlen(s2); i++)
    if ((s2[i]=='|') || (s2[i]=='>') || (s2[i]=='<') || (s2[i]==';') || (s2[i]==' ') ||
        (s2[i]==':') || (s2[i]=='/') || (s2[i]=='\\'))
      return;
  add_arc("TEMP", s2, 1);
}

void del_temp(void)
{
  char s1[81];
  nl();
  prt(2,get_string(855));
  input(s1,12);
  if (!okfn(s1))
    return;
  if (s1[0]) {
    if (strchr(s1,'.')==NULL)
      strcat(s1,".*");
    remove_from_temp(s1,syscfgovr.tempdir, 1);
  }
}

void list_temp_dir(void)
{
  int i1,f1,abort;
  char s[81],s1[81];
  struct ffblk ff;

  sprintf(s1,"%s*.*",syscfgovr.tempdir);
  f1=findfirst(s1,&ff,0);
  nl();
  pl(get_string(856));
  nl();
  i1=0;
  abort=0;
  while ((f1==0) && (!hangup) && (!abort)) {
    strcpy(s,(ff.ff_name));
    align(s);
    sprintf(s1,"%12s  %-8ld",s,ff.ff_fsize);
    pla(s1, &abort);
    f1=findnext(&ff);
    i1=1;
  }
  if (!i1)
    pl(get_string(5));
  nl();
  if ((!abort) && (!hangup)) {
    npr("%s: %ldk.\r\n", get_string(857),(long) freek1(syscfgovr.tempdir));
    nl();
  }
}


void temp_extract(void)
{
  int i,i1,i2,ok,abort,ok1,f, ot;
  char s[255],s1[255],s2[81],s3[255],s4[129];
  uploadsrec u;

  dliscan();
  nl();
  pl(get_string(858));
  nl();
  prt(2,get_string(44));
  input(s,12);
  if ((!okfn(s)) || (s[0]==0)) {
    return;
  }
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  align(s);
  i=recno(s);
  ok=1;
  while ((i>0) && (ok) && (!hangup)) {
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    sh_close(f);
    sprintf(s2,"%s%s",directories[udir[curdir].subnum].path,u.filename);
    if(directories[udir[curdir].subnum].mask & mask_cdrom) {
      sprintf(s1,"%s%s",directories[udir[curdir].subnum].path,u.filename);
      sprintf(s2,"%s%s",syscfgovr.tempdir,u.filename);
      if(!exist(s2))
        copy_file(s1,s2);
    }
    get_arc_cmd(s1,s2,1,"");
    if ((s1[0]) && (exist(s2))) {
      nln(2);
      abort=0;
      ot=tagging; tagging=2;
      printinfo(&u,&abort);
      tagging=ot;
      nl();
      if(directories[udir[curdir].subnum].mask & mask_cdrom) {
        cd_to(syscfgovr.tempdir);
      } else {
        cd_to(directories[udir[curdir].subnum].path);
      }
      get_dir(s4,1);
      strcat(s4,stripfn(u.filename));
      cd_to(cdir);
      if (!check_for_files(s4)) {
        do {
          prt(2,get_string(859));
          input(s1,12);
          if (s1[0]==0)
            ok1=0;
          else
            ok1=1;
          if (!okfn(s1))
            ok1=0;
          if (strcmp(s1,"?")==0) {
            list_arc_out(stripfn(u.filename),directories[udir[curdir].subnum].path);
            s1[0]=0;
          }
          if (strcmp(s1,"Q")==0) {
            ok=0;
            s1[0]=0;
          }
          i2=0;
          for (i1=0; i1<strlen(s1); i1++)
            if ((s1[i1]=='|') || (s1[i1]=='>') || (s1[i1]=='<') || (s1[i1]==';') || (s1[i1]==' '))
              i2=1;
          if (i2)
            s1[0]=0;
          if (s1[0]) {
            if (strchr(s1,'.')==NULL)
              strcat(s1,".*");
            get_arc_cmd(s3,s4,1,stripfn(s1));
            cd_to(syscfgovr.tempdir);
            if (!okfn(s1))
              s3[0]=0;
            if (s3[0]) {
              extern_prog(s3, sysinfo.spawn_opts[9]);
              sprintf(s2,get_stringx(1,47),s1,u.filename);
            } else
              s2[0]=0;
            cd_to(cdir);
            if (s2[0])
              sysoplog(s2);
          }
        } while ((!hangup) && (ok) && (ok1));
      }
    } else
      if (s1[0]) {
        nl();
        pl(get_string(860));
        nl();
      }
    if (ok)
      i=nrecno(s,i);
  }
}

void list_temp_text(void)
{
  int f1,ok,sent;
  char s[81],s1[81];
  struct ffblk ff;
  double percent;

  nl();
  prt(2,get_string(861));
  input(s,12);
  if (!okfn(s))
    return;
  if (s[0]) {
    if (strchr(s,'.')==NULL)
      strcat(s,".*");
    sprintf(s1,"%s%s",syscfgovr.tempdir,stripfn(s));
    f1=findfirst(s1,&ff,0);
    ok=1;
    nl();
    while ((f1==0) && (ok)) {
      sprintf(s,"%s%s",syscfgovr.tempdir,ff.ff_name);
      nl();
      outstr(get_string(862));
      pl(ff.ff_name);
      nl();
      ascii_send(s,&sent,&percent);
      if (sent) {
        sprintf(s,get_stringx(1,48),ff.ff_name);
        sysoplog(s);
      } else {
        sprintf(s,get_stringx(1,49),ff.ff_name,percent*100.0);
        sysoplog(s);
        ok=0;
      }
      f1=findnext(&ff);
    }
  }
}


void list_temp_arc(void)
{
  char s1[81];

  sprintf(s1,"TEMP.%s",syscfg.arcs[ARC_NUMBER].extension);
  list_arc_out(s1,syscfgovr.tempdir);
  nl();
}




void temporary_stuff(void)
{
  char ch;
  int done;

  done=0;
  do {
    nl();
    if (menu_on()) {
      rip_saveall();
      printmenu(14);
    } else
      prt(2,get_string(863));
    ch=onek("Q?DRAVLT");
    if (menu_on())
      rip_restoreall();
    switch(ch) {
      case 'Q':
        done=1;
        break;
      case 'D':
        download_temp_arc("TEMP", 1);
        break;
      case 'V':
        list_temp_arc();
        break;
      case 'A':
        add_temp_arc();
        break;
      case 'R':
        del_temp();
        break;
      case 'L':
        list_temp_dir();
        break;
      case 'T':
        list_temp_text();
        break;
      case '?':
        printmenu(14);
        break;
    }
  } while ((!hangup) && (!done));
}



void move_file_t(void)
{
  char sx[81],s1[81],s2[81],ch,*ss;
  int i,i1,i2,ok,d1,d2,done,cp,f;
  uploadsrec u,u1;

  tmp_disable_conf(1);

  nl();
  if (numbatch==0) {
    nl();
    pl(get_string(1363));
    pausescr();
  }
  for (i2=numbatch-1; i2>=0; i2--) {
    ok=0;
    strcpy(sx,batch[i2].filename);
    align(sx);
    dliscan1(batch[i2].dir);
    i=recno(sx);
    if (i<0) {
      pl(get_string(89));
      pausescr();
    }
    done=0;
    while ((!hangup) && (i>0) && (!done)) {
      cp=i;
      f=sh_open1(dlfn,O_RDONLY | O_BINARY);
      SETREC(f,i);
      sh_read(f,(void *)&u,sizeof(uploadsrec));
      f=sh_close(f);
      printfileinfo(&u,batch[i2].dir);
      prt(5,get_string(822));
      ch=ynq();
      if (ch=='Q') {
        done=1;
        tmp_disable_conf(0);
        dliscan();
        return;
      }
      else if (ch=='Y') {
        sprintf(s1,"%s%s",directories[batch[i2].dir].path,u.filename);
        do {
          prt(2,get_string(823));
          ss=mmkey(1);
          if (ss[0]=='?') {
            dirlist();
            dliscan1(batch[i2].dir);
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
            pl(get_string(824));
          }
          if (numf>=directories[d1].maxfiles) {
            ok=0;
            pl(get_string(825));
          }
          if (freek1(directories[d1].path)<((double)(u.numbytes/1024L)+3)) {
            ok=0;
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
            remlist(batch[i2].filename);
            didnt_upload(i2);
            delbatch(i2);
          } else {
            copy_file(s1,s2);
            unlink(s1);
            remlist(batch[i2].filename);
            didnt_upload(i2);
            delbatch(i2);
          }
        }
        pl(get_string(827));
      }
      dliscan();
      i=nrecno(sx,cp);
    }
  }

  tmp_disable_conf(0);
}

void removefile(void)
{
  int i,i1,rm,abort,rdlp,f;
  char ch,s[81],s1[81];
  uploadsrec u;
  userrec uu;

  dliscan();
  nl();
  pl(get_string(816));
  outstr(":");
  mpl(12);
  input(s,12);
  if (s[0]==0) {
    return;
  }
  if (strchr(s,'.')==NULL)
    strcat(s,".*");
  align(s);
  i=recno(s);
  abort=0;
  while ((!hangup) && (i>0) && (!abort)) {
    f=sh_open1(dlfn,O_RDONLY | O_BINARY);
    SETREC(f,i);
    sh_read(f,(void *)&u,sizeof(uploadsrec));
    f=sh_close(f);
    if ((dcs()) || ((u.ownersys==0) && (u.ownerusr==usernum))) {
      nl();
      if (check_batch_queue(u.filename)) {
        pl(get_string(817));
        nl();
      } else {
        printfileinfo(&u,udir[curdir].subnum);
        prt(2,get_string(818));
        ch=ynq();
        if (ch=='Q')
          abort=1;
        else if (ch=='Y') {
          rdlp=1;
          if (dcs()) {
            prt(5,get_string(819));
            rm=yn();
            if (rm && (u.ownersys==0)) {
              prt(5,get_string(820));
              rdlp=yn();
            }
            if (sysinfo.flags & OP_FLAGS_FAST_SEARCH) {
              nl();
              prt(5,get_string(1381));
              if (yn())
                modify_database(u.filename,0);
            }
          } else {
            rm=1;
            modify_database(u.filename,0);
          }
          if (rm) {
            sprintf(s1,"%s%s",directories[udir[curdir].subnum].path,u.filename);
            unlink(s1);
            if ((rdlp) && (u.ownersys==0)) {
              read_user(u.ownerusr,&uu);
              if ((uu.inact & inact_deleted)==0) {
                if (date_to_daten(uu.firston) < u.daten) {
                  --uu.uploaded;
                  uu.uk -= bytes_to_k(u.numbytes);
                  write_user(u.ownerusr,&uu);
                }
              }
            }
          }
          if (u.mask & mask_extended)
            delete_extended_description(u.filename);
          sprintf(s1,get_stringx(1,44),u.filename,
                            directories[udir[curdir].subnum].name);
          sysoplog(s1);
          f=sh_open(dlfn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
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
        }
      }
    }
    i=nrecno(s,i);
  }
}


