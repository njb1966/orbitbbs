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

#include <ctype.h>
#include <math.h>
#include <string.h>


void input_phone(void)
{
  int ok;

  do {
    nl();
    ansic(3);
    pl(get_string(527));
    ansic(3);
    pl(get_string(493));
    prt(2,":");
    mpl(12);
    input(thisuser.phone,12);

    ok=valid_phone(thisuser.phone);
    if (!ok) {
      nl();
      ansic(6);
      pl(get_string(528));
      ansic(6);
      pl(get_string(529));
    }
  } while ((!ok) && (!hangup));
}


void input_dataphone(void)
{
  int ok;

  do {
    nl();
    ansic(3);
    pl(get_string(540));
    ansic(3);
    pl(get_string(493));
    prt(2,":");
    mpl(12);
    input(thisuser.dataphone,12);

    ok=valid_phone(thisuser.dataphone);

    if (!ok) {
      nl();
      ansic(6);
      pl(get_string(528));
      ansic(6);
      pl(get_string(529));
    }
  } while ((!ok) && (!hangup));
}

void input_language(void)
{
  int i;
  char onx[20],ch,*ss;

  if (num_languages>1) {

    thisuser.language=255;

    do {
      nln(2);
      for (i=0; i<num_languages; i++) {
        npr("%d. %s\r\n",i+1,languages[i].name);
        if (i<9)
          onx[i]='1'+i;
      }
      nl();
      prt(2,get_string(937));
      if (num_languages<10) {
        onx[num_languages]=0;
        ch=onek(onx);
        ch-='1';
      } else {
        for (i=1; i<=num_languages/10; i++)
          odc[i-1]='0'+i;
        odc[i-1]=0;
        ss=mmkey(2);
        ch=atoi(ss)-1;
      }
      if ((ch>=0) && (ch<num_languages)) {
        thisuser.language=languages[ch].num;
      }
    } while (thisuser.language==255);

    set_language(thisuser.language);
  }
}


int check_name(unsigned char *nn)
{
  int ok,f,i;
  unsigned char s[161],s1[161],s2[81];
  long p,l;

  ok=1;
  if (nn[strlen(nn)-1]==32)
    ok=0;
  if (nn[0]<65)
    ok=0;
  if (finduser(nn)!=0)
    ok=0;
  if (strchr(nn,'@')!=NULL)
    ok=0;
  if (strchr(nn,'#')!=NULL)
    ok=0;

  if (!ok)
    return(ok);

  sprintf(s,"%sTRASHCAN.TXT",syscfg.gfilesdir);
  f=sh_open1(s,O_RDONLY | O_BINARY);

  if (f<0)
    return(ok);

  sh_lseek(f,0L,SEEK_SET);
  l=filelength(f);
  p=0;
  sprintf(s2," %s ",nn);
  while ((p<l) && (ok)) {
    sh_lseek(f,p,SEEK_SET);
    sh_read(f,(void *)s,150);
    i=0;
    while ((i<150) && (s[i])) {
      if (s[i]==13)
        s[i]=0;
      else
        ++i;
    }
    s[150]=0;
    p += (long) (i+2);
    if (s[i-1]==1)
      s[i-1]=0;
    for (i=0; i<strlen(s); i++)
      s[i]=upcase(s[i]);
    sprintf(s1," %s ",s);
    if (strstr(s2,s1)!=NULL)
      ok=0;
  }
  sh_close(f);
  if (!ok) {
    hangup=1;
    hang_it_up();
  }
  return(ok);
}


void input_name(void)
{
  int ok,count;

  count=0;
  do {
    nl();
    ansic(3);
    if (syscfg.sysconfig & sysconfig_no_alias)
      pl(get_string(520));
    else
      pl(get_string(521));
    prt(2,":");
    mpl(30);
    input(thisuser.name,30);
    ok=check_name(thisuser.name);
    if (!ok) {
      nl();
      ansic(6);
      pl(get_string(522));
      ++count;
      if (count==3) {
        hangup=1;
        hang_it_up();
      }
    }
  } while ((!ok) && (!hangup));
}


void input_realname(void)
{
  if (!(syscfg.sysconfig & sysconfig_no_alias)) {
    do {
      nl();
      ansic(3);
      pl(get_string(523));
      prt(2,":");
      mpl(20);
      inputp(thisuser.realname,20);

      if (thisuser.realname[0]==0) {
        nl();
        ansic(6);
        pl(get_string(524));
      }
    } while ((thisuser.realname[0]==0) && (!hangup));
  } else {
    strncpy(thisuser.realname, thisuser.name, sizeof(thisuser.realname)-1);
  }
}

void input_callsign(void)
{
  nl();
  ansic(3);
  pl(get_string(525));
  ansic(3);
  pl(get_string(526));
  prt(2,":");
  mpl(6);
  input(thisuser.callsign,6);
}


int valid_phone(char *phone)
{
  int i;

  if (syscfg.sysconfig & sysconfig_free_phone)
    return(1);

  if (syscfg.sysconfig & sysconfig_extended_info) {
    if ((!usa_phone_convention(&thisuser)) && thisuser.country[0])
      return(1);
  }

  if (strlen(phone)!=12)
    return(0);
  if ((phone[3]!='-') || (phone[7]!='-'))
    return(0);
  for (i=0; i<12; i++) {
    if ((i!=3) && (i!=7)) {
      if ((phone[i]<'0') || (phone[i]>'9')) {
        return(0);
      }
    }
  }
  return(1);
}


void input_street(void)
{
  do {
    nl();
    ansic(3);
    pl(get_string(530));
    prt(2,":");
    mpl(30);
    inputp(thisuser.street,30);

    if (thisuser.street[0]==0) {
      nl();
      ansic(6);
      pl(get_string(531));
    }
  } while ((thisuser.street[0]==0) && (!hangup));
}

void input_city(void)
{
  do {
    nl();
    ansic(3);
    pl(get_string(532));
    prt(2,":");
    mpl(30);
    inputp(thisuser.city,30);

    if (thisuser.city[0]==0) {
      nl();
      ansic(6);
      pl(get_string(533));
    }
  } while ((thisuser.city[0]==0) && (!hangup));
}

void input_state(void)
{
  do {
    nl();
    ansic(3);
    if (strcmp(thisuser.country,"CAN")==0)
      pl(get_string(1521));
    else
      pl(get_string(534));
    prt(2,":");
    mpl(2);
    input(thisuser.state,2);

    if (thisuser.state[0]==0) {
      nl();
      ansic(6);
      pl(get_string(535));
    }
  } while ((thisuser.state[0]==0) && (!hangup));
}

void input_country(void)
{
  do {
    nl();
    ansic(3);
    pl(get_string(536));
    prt(2,":");
    mpl(3);
    input(thisuser.country,3);

    if (thisuser.country[0]==0) {
      nl();
      ansic(6);
      pl(get_string(537));
    }
  } while ((thisuser.country[0]==0) && (!hangup));
}


void input_zipcode(void)
{
  int len;

  do {
    nl();
    ansic(3);
    if (strcmp(thisuser.country,"CAN")==0) {
      pl(get_string(1522));
      len=7;
    } else {
      pl(get_string(538));
      len=10;
    }
    prt(2,":");
    mpl(len);
    input(thisuser.zipcode,len);

    if (thisuser.zipcode[0]==0) {
      nl();
      ansic(6);
      pl(get_string(539));
    }
  } while ((thisuser.zipcode[0]==0) && (!hangup));
}

void input_sex(void)
{
  nl();
  prt(2,get_string(541));
  thisuser.sex=onek("MF");
}

void input_age(userrec *u)
{
  int ok,y,m,d;
  char ag[10];

  do {
    nl();
    do {
      nl();
      prt(2,get_string(542));
      mpl(2);
      input(ag,2);
      m=atoi(ag);
    } while ((!hangup) && ((m>12) || (m<1)));
    do {
      nl();
      prt(2,get_string(543));
      mpl(2);
      input(ag,2);
      d=atoi(ag);
    } while ((!hangup) && ((d>31) || (d<1)));
    do {
      nl();
      prt(2,get_string(544));
      mpl(2);
      input(ag,2);
      y=atoi(ag)+1900;
      if (y==1919) {
        nl();
        prt(5,get_string(545));
        if (!yn()) {
          y=0;
        }
      }
    } while ((!hangup) && (y<1905));
    ok=1;
    if (((m==2) || (m==9) || (m==4) || (m==6) || (m==11)) && (d==31))
      ok=0;
    if ((m==2) && (((y%4!=0) && (d==29)) || (d==30)))
      ok=0;
    if (!ok) {
      nl();
      ansic(6);
      pl(get_string(546));
    }
    if (years_old((unsigned char)m, (unsigned char)d, (unsigned char)(y-1900))<5) {
      nl();
      pl(get_string(547));
      ok=0;
    }
  } while ((!ok) && (!hangup));
  u->month=(unsigned char) m;
  u->day=(unsigned char) d;
  u->year=(unsigned char) (y-1900);
  u->age=years_old(u->month,u->day,u->year);
  nl();
}

void input_comptype(void)
{
  int i,ok,ct;
  char c[5];

  do {
   nl();
   pl(get_string(275));
   nl();
   for (i=0; ctypes[i]; i++)
     npr("%d. %s\r\n",i+1,ctypes[i]);
   nl();
   ansic(3);
   pl(get_string(548));
   ansic(3);
   pl(get_string(549));
   prt(2,":");
   mpl(2);
   input(c,2);
   ct=atoi(c);

   ok=1;
   if ((ct<1) || (ct>i))
     ok=0;

  } while ((!ok) && (!hangup));
  thisuser.comp_type=ct-1;
  /* if (checkcomp("Ami"))  thisuser.colors[0]=4; Why is this here? */
  if (hangup)
    thisuser.comp_type=0;
}

void input_screensize(void)
{
  int ok,x,y;
  char s[5];

  do {
    nl();
    ansic(3);
    pl(get_string(550));
    prt(2,":");
    mpl(2);
    input(s,2);
    x=atoi(s);
    if (s[0]==0)
      x=80;

    if ((x<32) || (x>80))
      ok=0;
    else
      ok=1;
  } while ((!ok) && (!hangup));

  do {
    nl();
    ansic(3);
    pl(get_string(551));
    prt(2,":");
    mpl(2);
    input(s,2);
    y=atoi(s);
    if (s[0]==0)
      y=25;

    if ((y<8) || (y>60))
      ok=0;
    else
      ok=1;
  } while ((!ok) && (!hangup));

  nl();
  thisuser.screenchars=x;
  thisuser.screenlines=y;
  screenlinest=y;
}

void input_pw(void)
{
  int ok;
  char s[81];

  do {
    nl();
    ansic(3);
    pl(get_string(552));
    prt(2,":");
    mpl(8);
    input(s,8);

    ok=1;
    if (strlen(s)<3)
      ok=0;
  } while ((!ok) && (!hangup));
  if (ok)
    strncpy(thisuser.pw,s,sizeof(thisuser.pw));
  else
    pl(get_string(463));
}


void input_ansistat(void)
{
  int i,c,c2;
  char ch;

  thisuser.sysstatus &= ~(sysstatus_ansi | sysstatus_color);
  nl();
  if (check_ansi()==1) {
    outstr(get_string(553));
  } else {
    outstr(get_string(1244));
    outstr(get_string(1245));
    outstr(get_string(1246));
    outstr(get_string(1247));
    outstr(get_string(1248));
    pl(get_string(1249));
    pl(get_string(554));
    outstr(get_string(555));
  }
  if (yn()) {
    thisuser.sysstatus |= sysstatus_ansi;
    nl();
    prt(5,get_string(556));
    if (yn()) {
      thisuser.sysstatus |= sysstatus_color;
      if (E_C) {
        thisuser.sysstatus |= sysstatus_extra_color;
      }
    } else {
      color_list();
      nl();
      prt(2,get_string(557));
      ch=onek("\r123456789");
      if (ch=='\r')
        ch='7';
      c=ch-'0';
      c2=c << 4;
      for (i=0; i<10; i++) {
       if ((thisuser.bwcolors[i] & 0x70) == 0)
         thisuser.bwcolors[i]=(thisuser.bwcolors[i] & 0x88) | c;
       else
         thisuser.bwcolors[i]=(thisuser.bwcolors[i] & 0x88) | c2;
      }
    }
  }
}

int find_new_usernum(userrec *u, unsigned long *qsc)
{
  char s[81];
  int f, nu, un,n;
  userrec tu;

  sprintf(s,"%sUSER.LST",syscfg.datadir);

  f=-1;
  for (n=0; (f<0) && (n<20); n++) {
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    if (f<0) {
      wait1(2);
    }
  }
  if (f<0)
    return(-1);

  nu=((int) (filelength(f)/syscfg.userreclen)-1);
  sh_lseek(f, syscfg.userreclen, SEEK_SET);
  un=1;

  read_status();
  if (nu == status.users) {
    un=nu+1;
  } else {

    while (un<=nu) {
      if (un%25 == 0) {
        f=sh_close(f);
        for (n=0; (f<0) && (n<20); n++) {
          f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
          if (f<0) {
            wait1(2);
          }
        }
        if (f<0)
          return(-1);
        sh_lseek(f, ((long) syscfg.userreclen) * ((long) un), SEEK_SET);
        nu=((int) (filelength(f)/syscfg.userreclen)-1);
      }

      sh_read(f, (void *)&tu, syscfg.userreclen);

      if ((tu.inact & inact_deleted) && (tu.sl != 255)) {
        sh_lseek(f, ((long) syscfg.userreclen) * ((long) un), SEEK_SET);
        sh_write(f, (void *)u, syscfg.userreclen);
        sh_close(f);
        write_qscn(un, qsc, 0);
        isr(un, u->name);
        return(un);
      } else
        un++;
    }
  }

  if (un <= syscfg.maxusers) {
    sh_lseek(f, ((long) syscfg.userreclen) * ((long) un), SEEK_SET);
    sh_write(f, (void *)u, syscfg.userreclen);
    sh_close(f);
    write_qscn(un, qsc, 0);
    isr(un, u->name);
    return(un);
  } else {
    sh_close(f);
    return(-1);
  }
}


void newuser(void)
{
  int i, ok;
  char s[255],s1[81],ch;

  memset(&thisuser, 0, sizeof(userrec));
  memset(qsc, 0, syscfg.qscn_len);

  strncpy(thisuser.firston,date(),sizeof(thisuser.firston));
  strncpy(thisuser.laston,get_string(564),sizeof(thisuser.laston));
  strncpy(&thisuser.macros[0][0],get_string(565),sizeof(thisuser.macros[0]));
  strncpy(&thisuser.macros[1][0],get_string(566),sizeof(thisuser.macros[1]));
  strncpy(&thisuser.macros[2][0],get_string(567),sizeof(thisuser.macros[2]));

  thisuser.screenlines=25;
  thisuser.screenchars=80;
  screenlinest=25;

  thisuser.sl=syscfg.newusersl;
  thisuser.dsl=syscfg.newuserdsl;

  thisuser.ontoday=1;

  thisuser.restrict=syscfg.newuser_restrict;

  *qsc=999;
  memset(qsc_n,0xff,((max_dirs+31)/32)*4);
  memset(qsc_q,0xff,((max_subs+31)/32)*4);

  thisuser.sysstatus=sysstatus_pause_on_page |
                     sysstatus_nscan_file_system |
                     sysstatus_conference |
                     sysstatus_expert;

  thisuser.gold=syscfg.newusergold;

  thisuser.colors[0]=sysinfo.newuser_colors[0];
  thisuser.colors[1]=sysinfo.newuser_colors[1];
  thisuser.colors[2]=sysinfo.newuser_colors[2];
  thisuser.colors[3]=sysinfo.newuser_colors[3];
  thisuser.colors[4]=sysinfo.newuser_colors[4];
  thisuser.colors[5]=sysinfo.newuser_colors[5];
  thisuser.colors[6]=sysinfo.newuser_colors[6];
  thisuser.colors[7]=sysinfo.newuser_colors[7];
  thisuser.colors[8]=sysinfo.newuser_colors[8];
  thisuser.colors[9]=sysinfo.newuser_colors[9];

  thisuser.bwcolors[0]=sysinfo.newuser_bwcolors[0];
  thisuser.bwcolors[1]=sysinfo.newuser_bwcolors[1];
  thisuser.bwcolors[2]=sysinfo.newuser_bwcolors[2];
  thisuser.bwcolors[3]=sysinfo.newuser_bwcolors[3];
  thisuser.bwcolors[4]=sysinfo.newuser_bwcolors[4];
  thisuser.bwcolors[5]=sysinfo.newuser_bwcolors[5];
  thisuser.bwcolors[6]=sysinfo.newuser_bwcolors[6];
  thisuser.bwcolors[7]=sysinfo.newuser_bwcolors[7];
  thisuser.bwcolors[8]=sysinfo.newuser_bwcolors[8];
  thisuser.bwcolors[9]=sysinfo.newuser_bwcolors[9];

  reset_act_sl();

  for (i=0; i<6; i++) {
    ch=rand() % 36;
    if (ch<10)
      ch+='0';
    else
      ch+='A'-10;
    thisuser.pw[i]=ch;
  }
  thisuser.pw[6]=0;

  if (rip_on())
    random_screen("NEWUSER");

  input_language();

  sprintf(s,get_stringx(1,25),date(),times(),curspeed, instance);
  sl1(0,"");
  sl1(0,s);
  read_status();
  if (status.users>=syscfg.maxusers) {
    nln(2);
    pl(get_string(558));
    pl(get_string(559));
    pl(get_string(560));
    nl();
    hangup=1;
  }
  if (syscfg.closedsystem) {
    nln(2);
    pl(get_string(561));
    pl(get_string(562));
    nl();
    hangup=1;
  }
  if ((syscfg.newuserpw[0]!=0) && (incom)) {
    nln(2);
    ok=0;
    i=0;
    do {
      outstr(get_string(563));
      input(s,20);
      if (strcmp(s,syscfg.newuserpw)==0)
        ok=1;
      else {
        sprintf(s1,get_stringx(1,26),s);
        sl1(0,s1);
      }
    } while ((!ok) && (!hangup) && (i++<4));
    if (!ok)
      hangup=1;
  }

  if (!hangup) {
    input_ansistat();
    if (incom) {
      if (printfile("SYSTEM"))
        sl1(0,get_stringx(1,27));
      pausescr();
      if (printfile("NEWUSER"))
        sl1(0,get_stringx(1,28));
    }
    if (syscfg.sysconfig & sysconfig_extended_info)
      input_country();
    input_name();
    input_realname();
    input_phone();
    if (sysinfo.flags & OP_FLAGS_CHECK_DUPE_PHONENUM) {
      if (check_dupes(thisuser.phone)) {
        if (sysinfo.flags & OP_FLAGS_HANGUP_DUPE_PHONENUM) {
          hangup=1;
          hang_it_up();
          return;
        }
      }
    }
    if (syscfg.sysconfig & sysconfig_extended_info) {
      input_street();
      sprintf(s,"%sZIP-CITY\\ZIP1.DAT",syscfg.datadir);
      if (exist(s)) {
        input_zipcode();
        if ((check_zip(thisuser.zipcode,1))==0) {
          thisuser.city[0]=0;
          thisuser.state[0]=0;
          thisuser.zipcode[0]=0;
        }
      }
      if (thisuser.city[0]==0)
        input_city();
      if (thisuser.state[0]==0)
        input_state();
      if (thisuser.zipcode[0]==0)
        input_zipcode();
      input_dataphone();
      if (sysinfo.flags & OP_FLAGS_CHECK_DUPE_PHONENUM) {
        if (check_dupes(thisuser.dataphone)) {
          if (sysinfo.flags & OP_FLAGS_HANGUP_DUPE_PHONENUM) {
            hangup=1;
            hang_it_up();
            return;
          }
        }
      }
    }
    input_callsign();
    input_sex();
    input_age(&thisuser);
    input_comptype();
    input_screensize();

    if (numed && (thisuser.sysstatus & sysstatus_ansi)) {
      nl();
      prt(5,get_string(568));
      if (yn())
        select_editor();
      nl();
    }

    prt(5,get_string(569));
    if (yn()) {
      nl();
      pl(get_string(570));
      nl();
      i=get_protocol(xf_down);
      if (i)
        thisuser.defprot=i;
    }
    nl();
    outstr(get_string(571)); pl(thisuser.pw);
    nl();
    prt(5,get_string(572));
    if (yn())
      input_pw();

    if (sysinfo.flags & OP_FLAGS_SIMPLE_ASV) {
      if ((sysinfo.asv.sl>syscfg.newusersl) &&
          (sysinfo.asv.sl<100)) {
        nl();
        prt(5,get_string(1250));
        if (yn()) {
           nl();
           prt(5,get_string(1251));
           nl();
           mpl(60);
           inputl(thisuser.note,60);
           thisuser.sl=sysinfo.asv.sl;
           thisuser.dsl=sysinfo.asv.dsl;
           thisuser.ar=sysinfo.asv.ar;
           thisuser.dar=sysinfo.asv.dar;
           thisuser.exempt=sysinfo.asv.exempt;
           thisuser.restrict=sysinfo.asv.restrict;
           nl();
           existprint("ASV");
           nl();
           pausescr();
         }
       }
     }
  }

  if (!hangup)
    do {
      nln(2);
      outstr(get_string(573)); pl(thisuser.name);
      if (!(syscfg.sysconfig & sysconfig_no_alias)) {
        outstr(get_string(574)); pl(thisuser.realname);
      }
      outstr(get_string(575)); pl(thisuser.callsign);
      outstr(get_string(576)); pl(thisuser.phone);
      outstr(get_string(577)); npr("%c\r\n",thisuser.sex);
      outstr(get_string(578)); npr("%02d/%02d/%02d\r\n",
        (int) thisuser.month, (int) thisuser.day, (int) thisuser.year);
      outstr(get_string(579)); pl(ctypes[thisuser.comp_type]);
      outstr(get_string(580)); npr("%d X %d\r\n",
        thisuser.screenchars, thisuser.screenlines);
      outstr(get_string(581)); pl(thisuser.pw);
      if (syscfg.sysconfig & sysconfig_extended_info) {
        outstr(get_string(582)); pl(thisuser.street);
        outstr(get_string(583)); pl(thisuser.city);
        outstr(get_string(584)); pl(thisuser.state);
        outstr(get_string(585)); pl(thisuser.country);
        outstr(get_string(586)); pl(thisuser.zipcode);
        outstr(get_string(587)); pl(thisuser.dataphone);
      }
      nl();
      pl(get_string(588));
      nln(2);
      if (syscfg.sysconfig & sysconfig_extended_info) {
        prt(2,get_string(589));
        ch=onek("Q123456789ABCDEF");
      } else {
        prt(2,get_string(590));
        ch=onek("Q123456789");
      }
      ok=0;
      switch(ch) {
        case 'Q': ok=1; break;
        case '1': input_name(); break;
        case '2':
          if (!(syscfg.sysconfig & sysconfig_no_alias))
            input_realname();
          break;
        case '3': input_callsign(); break;
        case '4':
          input_phone();
        break;
        case '5': input_sex(); break;
        case '6': input_age(&thisuser); break;
        case '7': input_comptype(); break;
        case '8': input_screensize(); break;
        case '9': input_pw(); break;
        case 'A': input_street(); break;
        case 'B': input_city(); break;
        case 'C': input_state(); break;
        case 'D': input_country(); break;
        case 'E': input_zipcode(); break;
        case 'F': input_dataphone(); break;
      }
    } while ((!ok) && (!hangup));

  if (!hangup) {

    nl();
    pl(get_string(26));
    nl();
    usernum = find_new_usernum(&thisuser, qsc);
    if (usernum<=0) {
      nl();
      pl(get_string(1523));
      nl();
      hangup=1;
      return;
    }
    ok=0;
    topscreen();
    do {
      nln(2);
      outstr(get_string(591)); pln(usernum);
      outstr(get_string(592)); pl(thisuser.pw);
      nl();
      pl(get_string(593));
      pl(get_string(594));
      pl(get_string(595));
      pl(get_string(596));
      nl();
      outstr(get_string(357));
      echo=0;
      input(s,8);
      if (strcmp(s,thisuser.pw)==0)
        ok=1;
    } while ((!ok) && (!hangup));
    checkit=0;
    if (incom) {
      if (printfile("FEEDBACK"))
        sl1(0,get_stringx(1,29));
      feedback(1);
    }
    if (sysinfo.flags & OP_FLAGS_FORCE_NEWUSER_FEEDBACK) {
      if ((incom) && (!thisuser.emailsent) && (!thisuser.feedbacksent)) {
        existprint("NOFBACK");
        deluser(usernum);
        hangup=1;
        hang_it_up();
        return;
      }
    }
    if ((!hangup) && (syscfg.newuser_c[0])) {
      stuff_in(s,syscfg.newuser_c,create_chain_file(),"","","","");
      write_user(usernum,&thisuser);
      extern_prog(s, sysinfo.spawn_opts[1]);
      read_user(usernum,&thisuser);
    }
    reset_act_sl();
    changedsl();
  }
}


int check_zip(char *zip, int mode)
{
  int ok;
  char s[81];
  userrec u;
  FILE *zip_file;
  char zip_buf[80];

  if (zip[0]==0)
    return(0);
  ok=1;
  sprintf(s,"%sZIP-CITY\\ZIP%c.DAT",syscfg.datadir,zip[0]);
  if ((zip_file=fsh_open(s,"r"))==NULL) {
    nl(); ansic(6);
    outstr(s);
    pl(get_string(1261));
    ok=0;
  }
  for (;;) {
    if (fgets(zip_buf,sizeof zip_buf-1,zip_file)==NULL) {
      nl(); ansic(6);
      outstr(get_string(1262));
      outstr(zip);
      pl(".");
      fsh_close(zip_file);
      ok=0;
      break;
    }
    if (strncmp(zip_buf,zip,5)==0) {
      fsh_close(zip_file);
      break;
    }
  }
  if (!mode) {
    (void)sscanf(zip_buf,"%s %s %[ -Z]",&u.zipcode,&u.state,&u.city);
    properize(u.city);
    npr("\r\n2%s is in %s, %s.",u.zipcode,u.city,u.state);
    return(0);
  }
  if ((zip_buf) && (ok)) {
    (void)sscanf(zip_buf,"%s %s %[ -Z]",&u.zipcode,&u.state,&u.city);
    properize(u.city);
    npr("\r\n2%s, %s  %s? (y/N): ",u.city,u.state,"USA");
    if (yn()) {
      thisuser.city[0]=0;
      thisuser.state[0]=0;
      thisuser.country[0]=0;
      strcpy(thisuser.city,u.city);
      strcpy(thisuser.state,u.state);
      strcpy(thisuser.country,"USA");
    } else {
      nl(); ansic(6);
      pl(get_string(1263));
      ok=0;
    }
  }
  return(ok);
}


void properize(char *s)
{
  int i;

  for (i=0;i<strlen(s);i++) {
    if ((i==0) || ((i>0) && ((s[i-1]==' ') || (s[i-1]=='.')))) {
      s[i]=toupper(s[i]);
    } else {
      s[i]=tolower(s[i]);
    }
  }
}

int check_dupes(char *phone)
{
  int i, dupe=0;
  char s[81];
  userrec u;

  if (phone[0]) {
    npr(get_string(1524));
    for (i=0;i<status.users;i++) {
      if ((i % 10)==0)
        outstr("3.0");
      read_user(i,&u);
      if (!(u.inact & inact_deleted)) {
        if ((strcmp(phone,u.phone)==0) || (strcmp(phone,u.dataphone)==0)) {
          if ((i!=usernum) && (strcmp(thisuser.name,u.name)!=0)
            && (u.inact & inact_deleted)) {
            sprintf(s,"    %s%s%s",thisuser.name,get_stringx(1,112),phone);
            sl1(0,s);
            ssm(1,0,s);
            sprintf(s,"%s%s",get_stringx(1,113),u.name);
            sl1(0,s);
            ssm(1,0,s);
            dupe=1;
          }
        }
      }
    }
  }
  return(dupe);
}

