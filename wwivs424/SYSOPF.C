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



void isr1(int un, unsigned char *name)
{
  int cp,i;
  smalrec sr;

  cp=0;
  while ((cp<status.users) && (strcmp(name,(smallist[cp].name))>0))
    ++cp;
  for (i=status.users; i>cp; i--)
    smallist[i]=smallist[i-1];
  strcpy(sr.name,name);
  sr.number=un;
  smallist[cp]=sr;
  ++status.users;
}


void reset_files(void)
{
  int i,i1,f;
  userrec u;
  char s[81];
  long pos;
  unsigned short users;

  read_status();
  status.users=users=0;
  nl();
  i1=number_userrecs();
  sprintf(s,"%sUSER.LST",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>=0) {
    for (i=1; i<=i1; i++) {
      pos=((long) syscfg.userreclen) * ((long) i);
      sh_lseek(f,pos,SEEK_SET);
      sh_read(f, (void *)&u, syscfg.userreclen);
      if ((u.inact & inact_deleted)==0) {
        fix_user_rec(&u);
        status.users=users;
        isr1(i,u.name);
        users=status.users;
      }
      if ((i % 10)==0) {
        f=sh_close(f);
        npr("%d\r",i);
        f=sh_open1(s,O_RDONLY | O_BINARY);
      }
    }
    f=sh_close(f);
  }
  outstr("\r\n\r\n");

  lock_status();

  sprintf(s,"%sNAMES.LST",syscfg.datadir);
  i=sh_open1(s,O_RDWR | O_BINARY | O_TRUNC);
  if (i<0) {
    printf(get_stringx(1,132),s);
    exit(noklevel);
  }
  sh_write(i,(void *) (smallist), (sizeof(smalrec) * status.users));
  i=sh_close(i);
  status.users=users;
  save_status();
}

void outstr_x1(char *s)
{
  ansic_x(1);
  outstr(s);
  ansic_x(2);
}


void prstatus(void)
{
  int i;
  long l;

  read_status();
  outchr(12);
  if (syscfg.newuserpw[0]!=0) {
    outstr_x1(get_string(298)); pl(syscfg.newuserpw);
  }
  outstr_x1(get_string(299)); pl(syscfg.closedsystem?get_string(300):get_string(301));
  outstr_x1(get_string(302)); pln(status.users);
  outstr_x1(get_string(303)); npr("%ld\r\n",status.callernum1);
  outstr_x1(get_string(304)); pl(status.date1);
  outstr_x1(get_string(305)); pl(times());
  outstr_x1(get_string(306)); pln(status.activetoday);
  outstr_x1(get_string(307)); pln(status.callstoday);
  outstr_x1(get_string(308)); pln(status.msgposttoday-status.localposts);
  outstr_x1(get_string(960)); pln(status.localposts);
  outstr_x1(get_string(309)); pln(status.emailtoday);
  outstr_x1(get_string(310)); pln(status.fbacktoday);
  outstr_x1(get_string(311)); pln(status.uptoday);
  outstr_x1(get_string(312)); pln(fwaiting);
#ifndef __OS2__
  outstr_x1(get_string(18));  pln(_stklen);
#endif
  outstr_x1(get_string(19));  npr("%dk\r\n", (int) (bbscoreleft()/1024));
  outstr_x1(get_string(20));  npr("%d%%\r\n", cachestat());
  outstr_x1(get_string(938)); npr("%lu\r\n",status.qscanptr);
  i=3;
  l=(long) freek(3);
  while ((l>0) && ((i+'@')<=cdir[0])) {
    ansic_x(1);
    npr("%c",i+'@');
    outstr(get_string(313));
    ansic_x(2);
    npr("%ldk\r\n",l);
    i++;
    if ((i+'@')<=cdir[0])
      l=(long) freek(i);
  }
  ansic_x(1);
  outstr(get_string(314));
  ansic_x(2);
  if (!sysop2())
    outstr(get_string(112));
  pl(get_string(315));
  if (num_instances() > 1)
    multi_instance();
}

void valuser(int un)
{
  userrec u;
  char s[81],s1[81],s2[81],s3[81],ar1[20],dar1[20],ch1;
  int i,i1,ar2,dar2;

  read_user(un,&u);
  if ((u.inact & inact_deleted)==0) {
    nl();
    outstr(get_string(1005)); pl(nam(&u,un));
    outstr(get_string(238)); pl(u.realname);
    outstr(get_string(242)); pl(u.phone);
    outstr(get_string(244)); npr("%d %c\r\n", u.age, u.sex);
    outstr(get_string(245)); pl(ctypes[u.comp_type]);
    if (u.note[0]) {
      outstr(get_string(253));
      pl(u.note);
    }
    nl();
    outstr(get_string(255)); pln(u.sl);
    if ((u.sl!=255) && (u.sl<actsl)) {
      outstr(get_string(316));
      input(s,3);
      if (s[0]) {
        i=atoi(s);
        if ((!wfc) && (i>=actsl))
          i=-2;
        if ((i>=0) && (i<255))
          u.sl=i;
	if (i==-1) {
	  nl();
	  prt(5,get_string(267));
	  if (yn()) {
            deluser(un);
	    nl();
	    pl(get_string(56));
	    nl();
	  } else {
	    nl();
	    pl(get_string(317));
	  }
	  return;
	}
      }
    }
    nl();
    outstr(get_string(318)); pln(u.dsl);
    if ((u.dsl!=255) && (u.dsl<thisuser.dsl)) {
      outstr(get_string(316));
      input(s,3);
      if (s[0]) {
        i=atoi(s);
        if ((!wfc) && (i>=thisuser.dsl))
          i=-1;
        if ((i>=0) && (i<255))
          u.dsl=i;
      }
    }
    strcpy(s3,restrict_string);
    ar2=1;
    dar2=1;
    ar1[0]=13;
    dar1[0]=13;
    for (i=0; i<=15; i++) {
      if (u.ar & (1 << i))
        s[i]='A'+i;
      else
        s[i]=32;
      if (thisuser.ar & (1 << i))
        ar1[ar2++]='A'+i;
      if (u.dar & (1 << i))
        s1[i]='A'+i;
      else
        s1[i]=32;
      if (thisuser.dar & (1 << i))
        dar1[dar2++]='A'+i;
      if (u.restrict & (1 << i))
        s2[i]=s3[i];
      else
        s2[i]=32;
    }
    s[16]=0;
    s1[16]=0;
    s2[16]=0;
    ar1[ar2]=0;
    dar1[dar2]=0;
    nl();
    ch1=0;
    if (ar2>1)
      do {
        outstr(get_string(258)); pl(s);
        prt(2,get_string(319));
        ch1=onek(ar1);
        if (ch1!=13) {
          ch1-='A';
          if (s[ch1]==32)
            s[ch1]=ch1+'A';
          else
            s[ch1]=32;
          u.ar ^= (1 << ch1);
          ch1=0;
        }
      } while ((!hangup) && (ch1!=13));
    nl();
    ch1=0;
    if (dar2>1)
      do {
        outstr(get_string(259)); pl(s1);
        prt(2,get_string(319));
        ch1=onek(dar1);
        if (ch1!=13) {
          ch1-='A';
          if (s1[ch1]==32)
            s1[ch1]=ch1+'A';
          else
            s1[ch1]=32;
          u.dar ^= (1 << ch1);
          ch1=0;
        }
      } while ((!hangup) && (ch1!=13));
    nl();
    ch1=0;
    s[0]=13;
    s[1]='?';
    strcpy(&(s[2]),restrict_string);
    do {
      npr("      %s\r\n",s3);
      outstr(get_string(320)); pl(s2);
      prt(2,get_string(319));
      ch1=onek(s);
      if ((ch1!=13) && (ch1!=32) && (ch1!='?')) {
        i=-1;
        for (i1=0; i1<16; i1++)
          if (ch1==s[i1+2])
            i=i1;
        if (i>-1) {
          u.restrict ^= (1 << i);
          if (s2[i]==32)
            s2[i]=s3[i];
          else
            s2[i]=32;
        }
        ch1=0;
      }
      if (ch1=='?') {
        ch1=0;
        printmenu(10);
      }
    } while ((!hangup) && (ch1==0));
    write_user(un,&u);
    nl();
  } else {
    nl();
    pl(get_string(321));
    nl();
  }
}

#define NET_SEARCH_SUBSTR     0x0001
#define NET_SEARCH_AREACODE   0x0002
#define NET_SEARCH_GROUP      0x0004
#define NET_SEARCH_AC         0x0008
#define NET_SEARCH_GC         0x0010
#define NET_SEARCH_NC         0x0020
#define NET_SEARCH_PHSUBSTR   0x0040
#define NET_SEARCH_NOCONNECT  0x0080
#define NET_SEARCH_NEXT       0x0100
#define NET_SEARCH_HOPS       0x0200
#define NET_SEARCH_ALL        0xFFFF

void print_net_listing(unsigned int tp)
{
  int done=0, i, i1, gn=0, onxi, odci, abort, matched, f, useregion;
  net_system_list_rec csne;
  unsigned short slist,cmdbit=0;
  unsigned char substr[81], onx[20], acstr[4], phstr[13], *mmk;
  unsigned char cmd, ch, s[161], s1[101], s2[101], s3[101], s4[101], bbstype;
  unsigned long l;

  read_status();

  helpl=45;

  if (!net_num_max)
    return;

  write_inst(INST_LOC_NETLIST,0,INST_FLAGS_NONE);

  if (tp) {
    l=thisuser.sysstatus;
    if ((l & sysstatus_pause_on_page)==0)
      thisuser.sysstatus ^= sysstatus_pause_on_page;
  }

  while (!done && !hangup) {
    outchr(12);
    if (net_num_max>1) {
      odc[0]=0;
      odci=0;
      onx[0]='Q';
      onx[1]=0;
      onxi=1;
      nl();
      for (i=0; i<net_num_max; i++) {
        if (i<9) {
          onx[onxi++]=i+'1';
          onx[onxi]=0;
        } else {
          odci=(i+1)/10;
          odc[odci-1]=odci+'0';
          odc[odci]=0;
        }
        npr("2%d7]1 %s\r\n",i+1,net_networks[i].name);
      }
      pl(get_string(1279));
      nl();
      prt(9,get_string(1280));
      if (net_num_max<9) {
        ch=onek(onx);
        if (ch=='Q')
          done=1;
        else
          i=ch-'1';
      } else {
        mmk=mmkey(2);
        if (*mmk=='Q')
          done=1;
        else
          i=atoi(mmk)-1;
      }

      if (done)
        break;

      if ((i>=0) && (i<net_num_max)) {
        set_net_num(i);
      } else
        continue;
    }

    read_bbs_list_index();
    abort=0; cmdbit=0; slist=0;
    substr[0]=0; acstr[0]=0; phstr[0]=0;

    nl();
    pl(get_string(1281));
    pl(get_string(1282));
    pl(get_string(1283));
    pl(get_string(1284));
    pl(get_string(1285));
    pl(get_string(1286));
    pl(get_string(1287));
    pl(get_string(1288));
    pl(get_string(1289));
    pl(get_string(1290));
    nl();
    outstr(get_string(1291));
    cmd=onek("Q123456789");

    switch (cmd) {
      case 'Q':
        done=1;
        break;
      case '1':
        cmdbit=NET_SEARCH_ALL;
        break;
      case '2':
        cmdbit=NET_SEARCH_AREACODE;
        nl();
        outstr(get_string(1292));
        input(acstr,3);
        if (strlen(acstr)!=3)
          abort=1;
        if (!abort)
          for (i=0; i<3; i++)
            if ((acstr[i]<'0') || (acstr[i]>'9'))
              abort=1;
        if (abort) {
          pl(get_string(1293));
          pausescr();
          cmdbit=0;
        }
        break;
      case '3':
        cmdbit=NET_SEARCH_GROUP;
        nl();
        outstr(get_string(1294));
        input(s,2);
        if ((s[0]==0) || (atoi(s)<1)) {
          pl(get_string(1295));
          pausescr();
          cmdbit=0;
          break;
        }
        gn=atoi(s);
        break;
      case '4':
        cmdbit=NET_SEARCH_AC;
        break;
      case '5':
        cmdbit=NET_SEARCH_GC;
        break;
      case '6':
        cmdbit=NET_SEARCH_NC;
        break;
      case '7':
        cmdbit=NET_SEARCH_SUBSTR;
        nl();
        outstr(get_string(1296));
        input(substr,40);
        if (substr[0]==0) {
          pl(get_string(1297));
          pausescr();
          cmdbit=0;
        }
        break;
      case '8':
        cmdbit=NET_SEARCH_PHSUBSTR;
        nl();
        outstr(get_string(1298));
        input(phstr,12);
        if (phstr[0]==0) {
          pl(get_string(1299));
          pausescr();
          cmdbit=0;
        }
        break;
      case '9':
        cmdbit=NET_SEARCH_NOCONNECT;
        break;
    }

    if (!cmdbit)
      continue;

    nl();
    outstr(get_string(1300));
    useregion=yn();

    sprintf(s,"%sBBSDATA.NET",net_data);
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if (f < 0) {
      npr("%s%s!",get_string(1301),s);
      nl();
      pausescr();
      continue;
    }

    strcpy(s,get_string(1302));
    nln(2);

    for (i=0; (i<num_sys_list) && (!abort); i++) {
      matched=0;
      sh_read(f,&csne,sizeof(net_system_list_rec));
      if ((csne.forsys==65535) && (cmdbit!=NET_SEARCH_NOCONNECT))
        continue;
      strcpy(s1,csne.phone);

      if (csne.other & other_net_coord)
        bbstype='&';
      else if (csne.other & other_group_coord)
        bbstype='%';
      else if (csne.other & other_coordinator)
        bbstype='^';
      else
        bbstype=' ';

      strcpy(s2,csne.name);
      for (i1=0; i1<strlen(s2); i1++)
        s2[i1]=upcase(s2[i1]);

      switch (cmdbit) {
        case NET_SEARCH_ALL:
          matched=1;
          break;
        case NET_SEARCH_AREACODE:
          if (strncmp(acstr, csne.phone, 3)==0)
            matched=1;
          break;
        case NET_SEARCH_GROUP:
          if (gn==csne.group)
            matched=1;
          break;
        case NET_SEARCH_AC:
          if (csne.other & other_coordinator)
            matched=1;
          break;
        case NET_SEARCH_GC:
          if (csne.other & other_group_coord)
            matched=1;
          break;
        case NET_SEARCH_NC:
          if (csne.other & other_net_coord)
            matched=1;
          break;
        case NET_SEARCH_SUBSTR:
          if (strstr(s2,substr)!=NULL)
            matched=1;
          else {
            sprintf(s3,"@%u",csne.sysnum);
            if (strstr(s3,substr)!=NULL)
              matched=1;
          }
          break;
        case NET_SEARCH_PHSUBSTR:
          if (strstr(s1,phstr)!=NULL)
            matched=1;
          break;
        case NET_SEARCH_NOCONNECT:
          if (csne.forsys==65535)
            matched=1;
          break;
      }

      if (matched) {
        slist++;
        if ((!useregion) && (slist==1)) {
          pla(get_string(1303),&abort);
          pla(get_string(1304),&abort);
        } else {
          if ((useregion) && (strncmp(s,csne.phone,3)!=0)) {
            strcpy(s,csne.phone);
            describe_area_code(atoi(csne.phone),s3);
            sprintf(s4,"\r\n%s%s\r\n",get_string(1305),s3);
            pla(s4,&abort);
            pla(get_string(1303),&abort);
            pla(get_string(1304),&abort);
          }
        }
        if (cmdbit!=NET_SEARCH_NOCONNECT)
          sprintf(s3,"%5u%c %12s  %-40s %3d %5u %2d",
              csne.sysnum,bbstype,csne.phone,csne.name,
              csne.numhops,csne.forsys,csne.group);
        else
          sprintf(s3,"%5u%c %12s  %-40s%s%2d",
              csne.sysnum,bbstype,csne.phone,csne.name,get_string(1307),csne.group);
        pla(s3, &abort);
      }
    }
    f=sh_close(f);
    if ((!abort) && (slist)) {
      nl();
      npr("%s%d",get_string(1306),slist);
    }
    nln(2);
    pausescr();
  }
  if (tp)
    thisuser.sysstatus=l;
}


void read_new_stuff(void)
{
  int i;

  zap_bbs_list();
  for (i=0; i<net_num_max; i++) {
    set_net_num(i);
    zap_call_out_list();
    zap_contacts();
  }
  set_language_1(cur_lang);
}


void mailr(void)
{
  int i,f,next,tp,nn;
  mailrec m, m1;
  char c;
  userrec u;

  f=open_email(0);
  if (f!=-1) {
    i=filelength(f)/sizeof(mailrec)-1;
    c=' ';
    while ((i>=0) && (c!='Q') && (!hangup)) {
      sh_lseek(f,((long) (i)) * ((long) sizeof(mailrec)),SEEK_SET);
      sh_read(f,(void *)&m,sizeof(mailrec));
      if (m.touser!=0) {
        f=sh_close(f);
        do {
          read_user(m.touser,&u);
          outstr(get_string(325));
          pl(nam(&u,m.touser));
          tp=80;
          if (m.status & status_source_verified)
            tp -= 2;
          if (m.status & status_new_net) {
            tp -= 1;
            if (strlen(m.title)<=tp) {
              nn=m.title[tp+1];
            } else
              nn=0;
          } else
            nn=0;
          set_net_num(nn);
          outstr(get_string(326)); pl(m.title);
          setorigin(m.fromsys, m.fromuser);
          read_message1(&(m.msg),m.anony & 0x0f,1,&next,"EMAIL");
          prt(2,get_string(327));
          if (next)
            c=' ';
          else
            c=onek("QRD ");
          if (c=='D') {
            f=open_email(1);
            sh_lseek(f,((long) (i)) * ((long) sizeof(mailrec)),SEEK_SET);
            sh_read(f,(void *)&m1,sizeof(mailrec));
            if (memcmp(&m, &m1, sizeof(mailrec))==0)
              delmail(f,i);
            else
              pl(get_string(1308));
            f=sh_close(f);
            if ((!useron) && (m.touser==1) && (m.tosys==0))
              --thisuser.waiting;
          }
          nln(2);
        } while ((c=='R') && (!hangup));

        f=open_email(0);
        if (f<0)
          break;
      }
      i-=1;
    }
    sh_close(f);
  }
}


void chuser(void)
{
  char s[81];
  int i;

  if (!checkpw())
    return;
  if (!so())
    return;
  prt(2,get_string(328));
  input(s,30);
  i=finduser1(s);
  if (i>0) {
    write_user(usernum,&thisuser);
    write_qscn(usernum,qsc,0);
    read_user(i,&thisuser);
    read_qscn(i,qsc,0);
    usernum=i;
    actsl=255;
    sprintf(s,get_stringx(1,17),nam(&thisuser,usernum));
    sysoplog(s);
    changedsl();
    topscreen();
  } else
    pl(get_string(8));
}

void zlog(void)
{
  zlogrec z;
  char s[81];
  int abort,f,i,i1;

  sprintf(s,"%sZLOG.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return;
  i=0;
  abort=0;
  sh_read(f,(void *)&z,sizeof(zlogrec));
  pla(get_string(329),&abort);
  pla(get_string(330),&abort);
  while ((i<97) && (!abort) && (!hangup) && (z.date[0]!=0)) {
    if (z.calls)
      i1=z.active/z.calls;
    else
      i1=0;
    sprintf(s,"%s    %4d    %4d     %3d     %3d     %3d    %3d     %3d      %3d",
         z.date,z.calls,z.active,z.posts,z.email,z.fback,z.up,10*z.active/144,i1);
    pla(s,&abort);
    ++i;
    if (i<97) {
      sh_lseek(f,((long) i) * sizeof(zlogrec),SEEK_SET);
      sh_read(f,(void *)&z,sizeof(zlogrec));
    }
  }
  sh_close(f);
}


void beginday(void)
{
  char s[255];
  zlogrec z,z1;
  int f,i;

  double fk;
  int    nus;

  if (instance != 1) {
    read_status();
    return;
  }
  lock_status();

  if (strcmp(date(), status.date1)==0)  {
    save_status();
    return;
  }
  pl(get_string(331));

  strcpy(z.date,status.date1);
  z.active=status.activetoday;
  z.calls=status.callstoday;
  z.posts=status.localposts;
  z.email=status.emailtoday;
  z.fback=status.fbacktoday;
  z.up=status.uptoday;
  status.callstoday=0;
  status.msgposttoday=0;
  status.localposts=0;
  status.emailtoday=0;
  status.fbacktoday=0;
  status.uptoday=0;
  status.activetoday=0;
  status.days++;
  strcpy(status.date3,status.date2);
  strcpy(status.date2,status.date1);
  strcpy(status.date1,date());
  strcpy(status.log2,status.log1);
  slname(status.date2, status.log1);

  sprintf(s,"%s%s",syscfg.gfilesdir, status.log2);
  unlink(s);

  sprintf(s,"%sUSER.LOG",syscfg.gfilesdir);
  unlink(s);
  sprintf(s,"%sZLOG.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDWR | O_BINARY);
  if (f<0) {
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    z1.date[0]=0;
    z1.active=0;
    z1.calls=0;
    z1.posts=0;
    z1.email=0;
    z1.fback=0;
    z1.up=0;
    for (i=0; i<97; i++)
      sh_write(f,(void *)&z1,sizeof(zlogrec));
  } else {
    for (i=96; i>=1; i--) {
      sh_lseek(f,(long) ((i-1) * sizeof(zlogrec)),SEEK_SET);
      sh_read(f,(void *)&z1,sizeof(zlogrec));
      sh_lseek(f,(long) (i * sizeof(zlogrec)),SEEK_SET);
      sh_write(f,(void *)&z1,sizeof(zlogrec));
    }
  }
  sh_lseek(f,0L,SEEK_SET);
  sh_write(f,(void *)&z,sizeof(zlogrec));
  sh_close(f);

  save_status();

  fk=freek1(syscfg.datadir);
  nus=syscfg.maxusers-status.users;

  if (fk<512.0) {
    sprintf(s,get_stringx(1,18),(int) fk);
    ssm(1,0,s);
  }

  if ((!syscfg.closedsystem) && (nus<15)){
    sprintf(s,get_stringx(1,19),nus);
    ssm(1,0,s);
  }

  if (syscfg.beginday_c[0]) {
    stuff_in(s,syscfg.beginday_c,create_chain_file(),"","","","");
    extern_prog(s, sysinfo.spawn_opts[2]);
  }
}


