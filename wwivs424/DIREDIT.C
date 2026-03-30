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


void dirdata(int n, char *s)
{
  char x,i;
  directoryrec r;

  r=directories[n];
  if (r.dar==0)
    x=32;
  else {
    for (i=0; i<16; i++)
      if ((1 << i) & r.dar)
        x='A'+i;
  }
  sprintf(s,"%4d %1c   %-39.39s %-8s %-3d %-3d %-3d %-9.9s",
            n,x,stripcolors(r.name),r.filename,r.dsl,r.age,r.maxfiles,
            r.path);
}

void showdirs(void)
{
  int abort,i;
  char s[180];

  outchr(12);
  abort=0;
  pla(get_string(133),
      &abort);
  pla(get_string(134),
      &abort);
  for (i=0; (i<num_dirs) && (!abort); i++) {
    dirdata(i,s);
    pla(s,&abort);
  }
}



void modify_dir(int n)
{
  directoryrec r;
  char s[81],ch,ch2;
  int i,done;

  r=directories[n];
  done=0;
  do {
    outchr(12);
    outstr(get_string(62)); pl(r.name);
    outstr(get_string(63)); pl(r.filename);
    outstr(get_string(135)); pl(r.path);
    outstr(get_string(136)); pln(r.dsl);
    outstr(get_string(137)); pln(r.age);
    outstr(get_string(138)); pln(r.maxfiles);
    strcpy(s,get_string(5));
    if (r.dar!=0) {
      for (i=0; i<16; i++)
        if ((1 << i) & r.dar)
          s[0]='A'+i;
      s[1]=0;
    }
    outstr(get_string(139)); pl(s);
    outstr(get_string(140)); pl((r.mask&mask_PD)?str_yes:str_no);
    outstr(get_string(141)); pln(r.type);
    outstr(get_string(142));
    pl((r.mask & mask_no_uploads) ? get_string(143) : get_string(115));
    outstr(get_string(144));
    pl((r.mask & mask_archive) ? str_yes : str_no);
    outstr(get_string(1153));
    if (r.mask & mask_cdrom)
      pl(get_string(1154));
    else
      pl(get_string(1155));
    if (r.mask & mask_cdrom) {
      outstr(get_string(1497));
      if (r.mask & mask_offline)
        pl(str_no);
      else
        pl(str_yes);
    }
    nl();
    prt(2,get_string(1498));
    ch=onek("QABCDEFGHIJKLM[]");
    switch(ch) {
      case 'Q':done=1; break;
      case '[':
        directories[n]=r;
        if (--n<0)
          n=num_dirs-1;
        r=directories[n];
        break;
      case ']':
        directories[n]=r;
        if (++n>=num_dirs)
          n=0;
        r=directories[n];
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
        prt(2,get_string(72));
        inputf(s,8);
        if ((s[0]!=0) && (strchr(s,'.')==0))
          strcpy(r.filename,s);
        break;
      case 'C':
        nl();
        pl(get_string(145));
        pl(get_string(146));
        prt(2,get_string(147));
        input(s,80);
        if (s[0]) {
          if (chdir(s)) {
            cd_to(cdir);
            if (mkdir(s)) {
              pl(get_string(148));
              pl(get_string(149));
              getkey();
              s[0]=0;
            }
          } else {
            cd_to(cdir);
          }
          if (s[0]) {
            strcat(s,"\\");
            strcpy(r.path,s);
            pl(get_string(150));
            pl(get_string(151));
            pl(get_string(152));
            pl(get_string(149));
            getkey();
          }
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(153));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<256) && (s[0]))
          r.dsl=i;
        break;
      case 'E':
        nl();
        prt(2,get_string(77));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<128) && (s[0]))
          r.age=i;
        break;
      case 'F':
        nl();
        prt(2,get_string(79));
        input(s,3);
        i=atoi(s);
        if ((i>0) && (i<1000) && (s[0]))
          r.maxfiles=i;
        break;
      case 'G':
        nl();
        prt(2,get_string(154));
        ch2=onek("ABCDEFGHIJKLMNOP ");
        if (ch2==32)
          r.dar=0;
        else
          r.dar=1 << (ch2-'A');
        break;
      case 'H':
        r.mask ^= mask_PD;
        break;
      case 'I':
        prt(2,get_string(156));
        input(s,4);
        i=atoi(s);
        if ((s[0]) && (i != r.type)) {
          r.type=i;
        }
        break;
      case 'J':
        r.mask ^= mask_no_uploads;
        break;
      case 'K':
        r.mask ^= mask_archive;
        break;
      case 'L':
        r.mask ^= mask_cdrom;
        if (r.mask & mask_cdrom) {
          r.mask |= mask_no_uploads;
        } else {
          r.mask &= ~mask_offline;
        }
        break;
      case 'M':
        if (r.mask & mask_cdrom) {
          r.mask ^= mask_offline;
        }
      break;
    }
  } while ((!done) && (!hangup));
  directories[n]=r;
}


void swap_dirs(int dir1, int dir2)
{
  int i,i1,i2,nu;
  unsigned long *qsc, *qsc_n, tl;
  directoryrec drt;
  SUBCONF_TYPE dir1conv, dir2conv;

  dir1conv=(SUBCONF_TYPE) dir1;
  dir2conv=(SUBCONF_TYPE) dir2;

  if ((dir1<0) || (dir1>=num_dirs) || (dir2<0) || (dir2>=num_dirs))
    return;

  update_conf(CONF_DIRS, &dir1conv, &dir2conv, CONF_UPDATE_SWAP);

  dir1=(int) dir1conv;
  dir2=(int) dir2conv;

  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len);
  if (qsc) {
    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc,1);
      qsc_n=qsc+1;

      if (qsc_n[dir1/32] & (1L<<(dir1%32)))
        i1=1;
      else
        i1=0;
      if (qsc_n[dir2/32] & (1L<<(dir2%32)))
        i2=1;
      else
        i2=0;
      if (i1+i2==1) {
        qsc_n[dir1/32] ^= (1L<<(dir1%32));
        qsc_n[dir2/32] ^= (1L<<(dir2%32));
      }

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  } 

  drt=directories[dir1];
  directories[dir1]=directories[dir2];
  directories[dir2]=drt;

  tl=dir_dates[dir1];
  dir_dates[dir1]=dir_dates[dir2];
  dir_dates[dir2]=tl;
}


void insert_dir(int n)
{
  directoryrec r;
  int i,i1,nu;
  unsigned long *qsc, *qsc_n, m1, m2, m3;
  SUBCONF_TYPE nconv;

  nconv=(SUBCONF_TYPE) n;

  if ((n<0) || (n>num_dirs))
    return;

  update_conf(CONF_DIRS, &nconv, NULL, CONF_UPDATE_INSERT);

  n=(int) nconv;

  for (i=num_dirs-1; i>=n; i--) {
    directories[i+1]=directories[i];
    dir_dates[i+1]=dir_dates[i];
  }
  strcpy(r.name,get_string(159));
  strcpy(r.filename,get_string(82));
  strcpy(r.path,syscfg.dloadsdir);
  r.dsl=10;
  r.age=0;
  r.maxfiles=50;
  r.dar=0;
  r.type=0;
  r.mask=0;
  directories[n]=r;
  ++num_dirs;

  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len);
  if (qsc) {
    qsc_n=qsc+1;

    m1=1L<<(n%32);
    m2=0xffffffff<<((n%32)+1);
    m3=0xffffffff>>(32-(n%32));

    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc, 1);

      for (i1=num_dirs/32; i1>n/32; i1--) {
        qsc_n[i1]=(qsc_n[i1]<<1) | (qsc_n[i1-1]>>31);
      }
      qsc_n[i1]=m1 | (m2 & (qsc_n[i1]<<1)) | (m3 & qsc_n[i1]);

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  }
}


void delete_dir(int n)
{
  int i,i1,nu;
  unsigned long *qsc, *qsc_n, m2, m3;
  SUBCONF_TYPE nconv;

  nconv=(SUBCONF_TYPE) n;

  if ((n<0) || (n>=num_dirs))
    return;

  update_conf(CONF_DIRS, &nconv, NULL, CONF_UPDATE_DELETE);

  n=(int)nconv;

  for (i=n; i<num_dirs; i++) {
    directories[i]=directories[i+1];
    dir_dates[i]=dir_dates[i+1];
  }
  --num_dirs;

  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len);
  if (qsc) {
    qsc_n=qsc+1;

    m2=0xffffffff<<(n%32);
    m3=0xffffffff>>(32-(n%32));

    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc, 1);

      qsc_n[n/32]=(qsc_n[n/32]&m3) | ((qsc_n[n/32]>>1)&m2) |
                  (qsc_n[(n/32)+1]<<31);

      for (i1=(n/32)+1; i1<=(num_dirs/32); i1++) {
        qsc_n[i1]=(qsc_n[i1]>>1) | (qsc_n[i1+1]<<31);
      }

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  }
}


void dlboardedit(void)
{
  int i,i1,i2,done,f, confchg=0;
  char s[81],s1[81],ch;
  SUBCONF_TYPE iconv;

  if (!checkpw())
    return;
  showdirs();
  done=0;
  do {
    nl();
    prt(2,get_string(160));
    ch=onek("QSDIM?");
    switch(ch) {
      case '?':
        showdirs();
        break;
      case 'Q':
        done=1;
        break;
      case 'M':
        nl();
        prt(2,get_string(161));
        input(s,4);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_dirs))
          modify_dir(i);
        break;
      case 'S':
        if (num_dirs<max_dirs) {
          nl();
          prt(2,get_string(1156));
          input(s,4);
          i1=atoi(s);
          if ((!s[0]) || (i1<0) || (i1>=num_dirs))
            break;
          nl();
          prt(2,get_string(1157));
          input(s,4);
          i2=atoi(s);
          if ((!s[0]) || (i2<0) || (i2%32==0) || (i2>num_dirs) || (i1==i2))
            break;
          nl();
          if (i2<i1)
            i1++;
          write_qscn(usernum,qsc,1);
          prt(1,get_string(1158));
          insert_dir(i2);
          swap_dirs(i1,i2); 
          delete_dir(i1);
          confchg=1;
          showdirs();
        } else {
          nl();
          pl(get_string(1159));
        }
        break;
      case 'I':
        if (num_dirs<max_dirs) {
          nl();
          prt(2,get_string(162));
          input(s,4);
          i=atoi(s);
          if ((s[0]!=0) && (i>=0) && (i<=num_dirs)) {
            insert_dir(i);
            modify_dir(i);
            confchg=1;
            if (dirconfnum>1) {
              nl();
              list_confs(CONF_DIRS,0);
              i2=select_conf(get_string(1160),CONF_DIRS, 0);
              if (i2>=0)
                if (in_conference(i,&dirconfs[i2])<0) {
                  iconv=(SUBCONF_TYPE) i;
                  addsubconf(CONF_DIRS, &dirconfs[i2], &iconv);
                  i=(int) iconv;
                }
            } else {
              if (in_conference(i,&dirconfs[0])<0) {
                iconv=(SUBCONF_TYPE) i;
                addsubconf(CONF_DIRS, &dirconfs[0], &iconv);
                i=(int) iconv;
              }
            }
          }
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(163));
        input(s,4);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_dirs)) {
          nl();
          ansic(5);
          outstr(get_string(87));
          outstr(directories[i].name);
          outstr("? ");
          if (yn()) {
            strcpy(s,directories[i].filename);
            delete_dir(i);
            confchg=1;
            nl();
            prt(5,get_string(1380));
            if (yn()) {
              sprintf(s1,"%s%s.DIR",syscfg.datadir, s);
              unlink(s1);
              sprintf(s1,"%s%s.EXT",syscfg.datadir,s);
              unlink(s1);
            }
          }
        }
        break;
    }
  } while ((!done) && (!hangup));
  sprintf(s,"%sDIRS.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  huge_xfer(f, directories, sizeof(directoryrec), num_dirs, 1);
  sh_close(f);
  if (confchg)
    save_confs(CONF_DIRS, -1, NULL);
  if (!wfc)
    changedsl();
}



