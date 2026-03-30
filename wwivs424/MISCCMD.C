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

#define DOTS 5
#define FRAME 7

/* AutoMessage feature removed in OrbitBBS */
void read_automessage(void)  {}
void write_automessage1(void) {}
void write_automessage(void)  {}


void bbslist(void)
{
  int i,f,done,ok;
  char s[150],s1[150],ch,ch1,*ss;
  char phone[51], name[51], speed[5], type[5];
  long l,l1;

  done=0;
  do {
    helpl=0;
    nl();
    prt(2,get_string(489));
    ch=onek("QRNA");
    switch(ch) {
      case 'Q':
        done=1;
        break;
      case 'R':
        printfile("BBSLIST.MSG");
        break;
      case 'N':
        print_net_listing(0);
        break;
      case 'A':
        helpl=25;
        if ((actsl<=10)) {
          nln(2);
          pl(get_string(490));
          nl();
          break;
        }
        if (thisuser.restrict & restrict_automessage) {
          nln(2);
          pl(get_string(491));
          nl();
          break;
        }
        nl();
        pl("Enter telnet address (e.g., bbs.example.com:2323):");
        outstr(":");
        mpl(50);
        input(phone,50);
        if (strlen(phone)>0) {
          ok=1;
          sprintf(s1,"%sBBSLIST.MSG",syscfg.gfilesdir);
          f=sh_open1(s1,O_RDONLY | O_BINARY);
          if (f>0) {
            sh_lseek(f,0L,SEEK_SET);
            l=filelength(f);
            if ((ss=malloca(l+500L))==NULL) {
              sh_close(f);
              return;
            }
            sh_read(f,ss,(int)l);
            l1=0L;
            while ((l1<l) && (ok)) {
              i=0;
              do {
                ch=ss[l1++];
                s1[i]=ch;
                if (ch==13)
                  s1[i]=0;
                ++i;
              } while ((ch!=10) && (i<120) && (l1<l));
              if (strstr(s1,phone)!=NULL)
                ok=0;
            }
            bbsfree(ss);
            sh_close(f);
          }
          if (ok) {
            pl(get_string(494));
            nln(2);
            pl(get_string(495));
            outstr(":");
            mpl(50);
            inputl(name,50);
            nl();
            pl(get_string(496));
            pl(get_string(497));
            outstr(":");
            mpl(4);
            input(speed,4);
            nl();
            pl(get_string(498));
            outstr(":");
            mpl(4);
            input(type,4);
            sprintf(s,"%-50.50s  %-50s  [%4s] (%4s)\r\n",
              phone, name, speed, type);
            nln(2);
            pl(s);
            nl();
            prt(5,get_string(499));
            if (yn()) {
              sprintf(s1,"%sBBSLIST.MSG",syscfg.gfilesdir);
              f=sh_open(s1,O_RDWR | O_CREAT | O_BINARY, S_IREAD | S_IWRITE);
              if (filelength(f)) {
                sh_lseek(f,-1L,SEEK_END);
                sh_read(f,((void *)&ch1),1);
                if (ch1==26)
                  sh_lseek(f,-1L,SEEK_END);
              }
              sh_write(f,(void *)s,strlen(s));
              sh_close(f);
              nl();
              pl(get_string(500));
            }
          } else {
            pl(get_string(501));
            nln(2);
          }
        } else {
          nl();
          pl(get_string(502));
          nl();
        }
        break;
    }
  } while ((!done) && (!hangup));
}


void kill_old_email(void)
{
  int cur,max,i,i1,f,done,done1,forward;
  char s1[81],ch;
  long l;
  mailrec m, m1;
  userrec u;
  slrec ss;

  if (rip_on()) {
    cleared = -1;
    setmsgview(0);
    smally = formery;
  }
  prt(5,get_string(503));
  forward=(yn());
  ss=syscfg.sl[actsl];
  f=open_email(0);
  if (f==-1) {
    nl();
    pl(get_string(504));
    return;
  }
  max=(int) (filelength(f) / sizeof(mailrec));
  if (forward)
    cur=max-1;
  else
    cur=0;
  done=0;
  do {
    sh_lseek(f,((long) cur) * sizeof(mailrec), SEEK_SET);
    sh_read(f,(void *)&m,sizeof(mailrec));
    while (((m.fromsys!=0) || (m.fromuser!=usernum) || (m.touser==0)) && (cur<max) && (cur>=0)) {
      if (forward)
        --cur;
      else
        ++cur;
      if ((cur<max) && (cur>=0)) {
        sh_lseek(f,((long) cur) * sizeof(mailrec), SEEK_SET);
        sh_read(f,(void *)&m,sizeof(mailrec));
      }
    }
    if ((m.fromsys!=0) || (m.fromuser!=usernum) || (m.touser==0) || (cur>=max) || (cur<0))
      done=1;
    else {
      f=sh_close(f);
      do {
        done1=0;
        nl();
        if (E_C) {
          outstr(get_string(325));
        } else {
          outstr(get_string(1371));
        }
        if (m.tosys==0) {
          read_user(m.touser,&u);
          strcpy(s1,nam(&u,m.touser));
          if ((m.anony & (anony_receiver | anony_receiver_pp | anony_receiver_da))
              && ((ss.ability & ability_read_email_anony)==0))
            strcpy(s1,get_string(482));
          pl(s1);
        } else {
          npr("#%u @%u\r\n",m.touser, m.tosys);
        }
        if (E_C) {
          outstr(get_string(326));
        } else {
          outstr(get_string(1372));
        }
        pl(m.title);
        time(&l);
        i=(int) ((l-m.daten)/24.0/3600.0);
        if (E_C) {
          outstr(get_string(505));
        } else {
          outstr(get_string(1373));
        }
        npr("%d ",i);
        pl(get_string(506));
        nl();
        prt(2,get_string(507));
        ch=onek("QRDN");
        switch(ch) {
          case 'Q':
            done1=1;
            done=1;
            break;
          case 'N':
            done1=1;
            if (forward)
              --cur;
            else
              ++cur;
            if ((cur>=max) || (cur<0))
              done=1;
            break;
          case 'D':
            done1=1;
            f=open_email(1);
            sh_lseek(f,((long) cur) * sizeof(mailrec), SEEK_SET);
            sh_read(f,(void *)&m1,sizeof(mailrec));
            if (memcmp(&m, &m1, sizeof(mailrec))==0) {
              delmail(f,cur);
              nl();
              pl(get_string(508));
              nl();
              sprintf(s1,get_stringx(1, 100),nam(&u,m1.touser));
              sysoplog(s1);
            } else {
              pl(get_string(1174));
            }
            f=sh_close(f);
            break;
          case 'R':
            nln(2);
            if (E_C) {
              outstr(get_string(326));
            } else {
              outstr(get_string(1372));
            }
            pl(m.title);
            setorigin(0,0);
            read_message1(&m.msg,(m.anony & 0x0f), 0, &i1,"EMAIL");
            break;
        }
      } while ((!hangup) && (!done1));
      f=open_email(0);
      if (f<0)
        break;
    }
  } while ((!done) && (!hangup));
  f=sh_close(f);
}


void list_users(int mode)
{
  subboardrec s;
  directoryrec d;
  userrec u;
  int i,abort,ok,num, count, color, next=0, ncnm=0;
  char s1[121],s2[121];

  if ((usub[cursub].subnum==-1) && (mode==0)) {
    nl();
    pl(get_string(510));
    nl();
    return;
  }
  if ((udir[curdir].subnum==-1) && (mode==1)) {
    nl();
    pl(get_string(32));
    nl();
    return;
  }
  abort=0;
  num=0;
  strcpy(s1,get_string(1175));
  if (mode==0) {
    s=subboards[usub[cursub].subnum];
    npr("\r\n\x1b[1;32m%s \x1b[1;33m%s\x1b[1;32m %s.\r\n", s1, subboards[usub[cursub].subnum].name,
        get_string(1628));
  }
  if (mode==1) {
    d=directories[udir[curdir].subnum];
    npr("\r\n\x1b[1;32m%s \x1b[1;33m%s\x1b[1;32m %s.\r\n", s1, directories[udir[curdir].subnum].name,
        get_string(1629));
  }
  nl();
  if (okansi()) {
    if (syscfg.sysconfig & sysconfig_extended_info) {
      pla("\x1b[1;33m #    Name                           City/State\x1b[0m",&abort);
      sprintf(s1,"\x1b[37m%s\x1b[0m", charstr(66, '\xcd'));
      pla(s1,&abort);
    } else {
      pla("\x1b[1;33m #    Name\x1b[0m",&abort);
      sprintf(s1,"\x1b[37m%s\x1b[0m", charstr(36, '\xcd'));
      pla(s1,&abort);
    }
  } else {
    if (syscfg.sysconfig & sysconfig_extended_info) {
      pla(get_string(1180),&abort);
      pla(get_string(1181),&abort);
    } else {
      pla(get_string(1182),&abort);
      pla(get_string(1183),&abort);
    }
  }
  count=0;
  color=3;
  outstr("\r");
  outstr(get_string(1184));
  read_status();
  for (i=0; (i<status.users) && (!abort) && (!hangup); i++) {
    if (ncnm>5) {
      count++;
      npr("%d.",color);
      if (count==DOTS) {
        osan("\r", &abort, &next);
        osan(get_string(1184), &abort, &next);
        color++;
        count=0;
        if (color==4)
          color++;
        if (color==7)
          color=0;
      }
    }
    read_user(smallist[i].number,&u);
    ok=1;
    if (mode==0) {
      if (u.sl<s.readsl)
        ok=0;
      if (u.age < (s.age&0x7f))
        ok=0;
      if ((s.ar!=0) && ((u.ar & s.ar)==0))
        ok=0;
    }
    if (mode==1) {
      if (u.dsl<d.dsl)
        ok=0;
      if (u.age<d.age)
        ok=0;
      if ((d.dar!=0) && ((u.dar & d.dar)==0))
        ok=0;
    }
    if (ok) {
      ncnm=0;
      sprintf(s2,"%s, %s",u.city,u.state);
      if (okansi()) {
        if (syscfg.sysconfig & sysconfig_extended_info) {
          sprintf(s1,"\r\x1b[37m\xb3\x1b[1;33m%-4d\x1b[37m\xb3\x1b[1;32m%-30.30s\x1b[37m\xb3\x1b[1;32m%-30.30s\x1b[37m\xb3\x1b[0m",
            smallist[i].number, u.name, s2);
        } else {
          sprintf(s1,"\r\x1b[37m\xb3\x1b[1;33m%-4d\x1b[37m\xb3\x1b[1;32m%-30.30s\x1b[37m\xb3\x1b[0m",
            smallist[i].number, u.name);
        }
      } else {
        if (syscfg.sysconfig & sysconfig_extended_info) {
          sprintf(s1,"\r:%-4d:%-30s:%-30s:",
            smallist[i].number,u.name,s2);
        } else {
          sprintf(s1,"\r:%-4d:%-30s:",
            smallist[i].number,u.name);
        }
      }
      pla(s1,&abort);
      ++num;
    } else
      ncnm++;
  }
  if (okansi()) {
    if (syscfg.sysconfig & sysconfig_extended_info) {
      sprintf(s1,"\r\x1b[37m%s\x1b[0m", charstr(66, '\xcd'));
    } else {
      sprintf(s1,"\r\x1b[37m%s\x1b[0m", charstr(36, '\xcd'));
    }
  } else {
    if (syscfg.sysconfig & sysconfig_extended_info) {
      sprintf(s1,"\r%s",get_string(1187));
    } else {
      sprintf(s1,"\r%s",get_string(1188));
    }
  }
  pla(s1,&abort);
  if (abort) {
    if (num>0)
      num--;
    nln(2);
  }
  npr("\x1b[1;33m%d\x1b[0m %s.\r\n",num,(num==1) ? get_string(1630) : get_string(1631));
}


void print_quest(int mapp, int map[21])
{
  char s[100];
  int i,abort,f;
  votingrec v;

  outchr(12);
  abort=0;
  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return;

  for (i=1; (i<=mapp) && (abort==0); i++) {
    sh_lseek(f,((long) (map[i])) * sizeof(votingrec), SEEK_SET);
    sh_read(f,(void *)&v,sizeof(votingrec));
    sprintf(s,"%c %2d: %s",
      thisuser.votes[map[i]]?' ':'*',i, v.question);
    pla(s,&abort);
  }
  f=sh_close(f);
  nl();
  if (abort)
    nl();
}


int print_question(int i, int ii)
{
  char s[81];
  int i1,t,t1,abort, f;
  votingrec v;
  voting_response vr;

  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return(0);
  sh_lseek(f,((long) ii)*sizeof(votingrec),SEEK_SET);
  sh_read(f,(void *)&v,sizeof(votingrec));
  f=sh_close(f);
  abort=0;

  if (!v.numanswers)
    return(0);

  outchr(12);
  sprintf(s,"%s%d",get_string(513),i);
  pla(s,&abort);
  pla(v.question,&abort);
  nl();
  t=0;
  for (i1=0; i1<v.numanswers; i1++) {
    vr=v.responses[i1];
    t+=vr.numresponses;
  }

  read_status();
  sprintf(s,"%s: %4.1f%%\r\n",get_string(514),
    ((double) t) / ((double) status.users) * 100.0);
  pla(s,&abort);
  nl();
  if (t)
    t1=t;
  else
    t1=1;
  pla(get_string(515),&abort);
  for (i1=0; i1<5; i1++)
    odc[i1]=0;
  for (i1=0; (i1<v.numanswers) && (!abort); i1++) {
    vr=v.responses[i1];
    if (((i1+1) % 10)==0)
      odc[((i1+1)/10)-1]='0'+((i1+1)/10);
    sprintf(s,"%2d: %-60s : %4d  %5.1f%%",
      i1+1, vr.response, vr.numresponses,
      ((float)vr.numresponses)/((float)t1)*100.0);
    pla(s,&abort);
  }
  nl();
  if (abort)
    nl();
  return(!abort);
}


void vote_question(int i, int ii)
{
  int ok,pqo,i1, f;
  char s[81],*ss, fn[81];
  votingrec v;

  sprintf(fn,"%sVOTING.DAT",syscfg.datadir);

  pqo=print_question(i,ii);
  ok=pqo;
  ok=1;
  if (restrict_vote & thisuser.restrict)
    ok=0;
  if (actsl<=10)
    ok=0;
  if (!ok)
    return;

  f=sh_open1(fn,O_RDONLY| O_BINARY);
  if (f<0)
    return;
  sh_lseek(f,((long) ii)*sizeof(votingrec),SEEK_SET);
  sh_read(f,(void *)&v,sizeof(votingrec));
  f=sh_close(f);

  if (!v.numanswers)
    return;

  strcpy(s,get_string(516));
  if (thisuser.votes[ii])
    strcat(s,v.responses[thisuser.votes[ii]-1].response);
  else
    strcat(s,get_string(517));
  pl(s);
  nl();
  prt(5,get_string(264));
  if (!yn())
    return;

  prt(2,get_string(297));
  ss=mmkey(2);
  i1=atoi(ss);
  if (i1>v.numanswers)
    i1=0;
  if ((i1==0) && (strcmp(ss,"0")))
    return;

  f=sh_open1(fn,O_RDWR | O_BINARY);
  if (f<0)
    return;
  sh_lseek(f,((long) ii)*sizeof(votingrec),SEEK_SET);
  sh_read(f,(void *)&v,sizeof(votingrec));

  if (!v.numanswers) {
    f=sh_close(f);
    return;
  }

  if (thisuser.votes[ii]) {
    v.responses[thisuser.votes[ii]-1].numresponses--;
  }
  thisuser.votes[ii]=i1;
  if (i1) {
    v.responses[thisuser.votes[ii]-1].numresponses++;
  }
  sh_lseek(f,((long) ii)*sizeof(votingrec),SEEK_SET);
  sh_write(f,(void *)&v,sizeof(votingrec));
  f=sh_close(f);
  nln(2);
}


void vote(void)
{
  int i,f,map[21],mapp,n,done;
  char s[81],sodc[10],*ss;
  votingrec v;

  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  if (f<0)
    return;
  n=(int) (filelength(f) / sizeof(votingrec)) -1;
  if (n<20) {
    v.question[0]=0;
    v.numanswers=0;
    for (i=n; i<20; i++)
      sh_write(f,(void *)&v,sizeof(votingrec));
  }
  mapp=0;
  for (i=0; i<5; i++)
    odc[i]=0;
  for (i=0; i<20; i++) {
    sh_lseek(f,((long) i) * sizeof(votingrec),SEEK_SET);
    sh_read(f,(void *)&v,sizeof(votingrec));
    if (v.numanswers) {
      map[++mapp]=i;
      if ((mapp % 10)==0)
        odc[(mapp/10)-1]='0'+(mapp/10);
    }
  }
  f=sh_close(f);

  strcpy(sodc,odc);
  if (mapp==0) {
    nln(2);
    pl(get_string(518));
    nl();
    return;
  }
  print_quest(mapp,&map[0]);
  done=0;
  do {
    nln(2);
    prt(2,get_string(519));
    strcpy(odc,sodc);
    ss=mmkey(2);
    i=atoi(ss);
    if ((i>0) && (i<=mapp))
      vote_question(i,map[i]);
    else
      if (strcmp(ss,"Q")==0)
        done=1;
      else
        if (strcmp(ss,"?")==0)
          print_quest(mapp,&map[0]);
  } while ((!done) && (!hangup));
}


void time_bank(void)
{
  char s[81],bc[11],c;
  int i,done=0;
  double nsln;

  nl();
  if (thisuser.sl<=syscfg.newusersl) {
    ansic(6);
    pl(get_string(981));
    return;
  }

  if (thisuser.banktime>syscfg.sl[thisuser.sl].time_per_logon)
    thisuser.banktime=syscfg.sl[thisuser.sl].time_per_logon;

  if (menu_on()) {
    rip_timebank();
    return;
  }

  if (okansi())
    strcpy(bc,get_string(982));
  else
    strcpy(bc,get_string(983));

  do {
    outchr(12);
    sprintf(s,"%c%s%c",bc[0],charstr(25,bc[4]),bc[1]);
    prt(4,s); nl();
    sprintf(s,"%c%-25.25s%c",bc[5],get_string(984),bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%s%c",bc[7],charstr(25,bc[4]),bc[9]);
    prt(4,s); nl();
    sprintf(s,"%c%-25.25s%c",bc[5],get_string(985),bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%-25.25s%c",bc[5],get_string(986),bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%-25.25s%c",bc[5],get_string(987),bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%s%c",bc[7],charstr(25,bc[4]),bc[9]);
    prt(4,s); nl();
    sprintf(s,"%c%-17.17s%-6d  %c",bc[5],get_string(988),thisuser.banktime,bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%-17.17s%-6d  %c",bc[5],get_string(1430),(int)(nsl()/60),bc[5]);
    prt(4,s); nl();
    sprintf(s,"%c%s%c",bc[2],charstr(25,bc[4]),bc[3]);
    prt(4,s);
    nln(2);
    prt(1,get_string(989));
    ansic(2);
    c=onek("QDW");
    switch(c) {
      case 'D':
        nl();
        prt(1,get_string(990));
        mpl(3);
        input(s,3);
        i=atoi(s);
        if (i>0) {
          nsln=nsl();
          if ((i+thisuser.banktime)>syscfg.sl[thisuser.sl].time_per_logon)
            i=syscfg.sl[thisuser.sl].time_per_logon-thisuser.banktime;
          if (i>(nsln/60.0))
            i=(nsln/60.0);
          thisuser.banktime+=i;
          thisuser.extratime-=i*60.0;
          tleft(0);
        }
        break;
      case 'W':
        nl();
        if (!thisuser.banktime)
          break;
        prt(1,get_string(991));
        mpl(3);
        input(s,3);
        i=atoi(s);
        if (i>0) {
          nsln=nsl();
          if (i>thisuser.banktime)
            i=thisuser.banktime;
          thisuser.banktime-=i;
          thisuser.extratime+=(i*60.0);
          tleft(0);
        }
        break;
      case 'Q':
        done=1;
        break;
    }
  } while (!done && !hangup);
}


void remotenotify(char *name, char *desc)
{
  char s[161],s1[81],s2[161];
  int f,i,rnnet,rndest,rnuser;
  long p,l;

  sprintf(s1,"%s%s",syscfg.datadir,get_string(1060));
  f=sh_open1(s1,O_RDWR | O_BINARY);
  if (f<0) {
    return;
  }
  sh_lseek(f,0L,SEEK_SET);
  l=filelength(f);
  p=0;
  nl();
  while (p<l) {
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
    strncpy(s2,extractword(1,s,","),sizeof(s2));
    if(strlen(s2) < 17) {
      rnnet=getnetnum(s2);
      strncpy(s2,extractword(2,s,","),sizeof(s2));
      rnuser=atoi(s2);
      strncpy(s2,extractword(3,s,","),sizeof(s2));
      rndest=atoi(s2);
      sprintf(s,"2@%d UPL:1 %s 2DESC:1 %s",
        net_networks[rnnet].sysnum,name,desc);
      if ((rnuser==0) || (rndest==0))
        return;
      if (so()) {
        npr("1SSM:7%s %d@%d %s",net_networks[rnnet].name,rnuser,rndest,s);
        npr("2 (y/N) ");
        if (yn()) {
          set_net_num(rnnet);
          ssm(rnuser,rndest,s);
        }
      } else {
        set_net_num(rnnet);
        ssm(rnuser,rndest,s);
        nl();
      }
    }
  }
  f=sh_close(f);
}


int remoteupload(char *message)
{
  char s[81],s1[81],s2[81],s3[81],ch,msg[81];
  int i,done,brd,network;

  strncpy(msg,stripcolors(message),80);
  if ((strlen(msg)<20) || (msg[0]!='@'))
    return(1);
  strcpy(s1,&msg[1]);
  for (i=0;i<strlen(s1);i++) {
    if (s1[i]==' ') {
      break;
    }
  }
  s1[i]=0;
  itoa(net_networks[net_num].sysnum,s3,10);
  if (strcmp(s1,s3)==NULL)
    return(1);
  for (i=0;i<strlen(msg);i++) {
    if (msg[i]==':') {
      strcpy(s,&msg[++i]);
      break;
    }
  }
  for (i=0;i<strlen(s);i++) {
    if (s[i]==' ') {
      strcpy(s,&s[i+1]);
      break;
    }
  }
  for (i=0;i<strlen(s);i++) {
    if (s[i]=='.') {
      s[i+4]=0;
      break;
    }
  }
  strcpy(s2,stripfn(s));
  done=0;

  do {
    nl();
    pl(get_string(1061));
    pl(get_string(1062));
    pl(get_string(1063));
    pl(get_string(1064));
    nl();
    prt(2,get_string(1065));
    ch=onek(" ADRS\r");
    switch(ch) {
      case 'A':
        if (sysinfo.flags & OP_FLAGS_FAST_SEARCH) {
          modify_database(s2,1);
          done=1;
        }
        break;
      case 'R':
        nl();
        npr(get_string(1066),s2,s1);
        if (yn()) {
          nl();
          for (brd=0; brd<net_num_max; brd++) {
            if (net_networks[brd].name != 0) {
              npr(" 2%d7)1 %s\r\n",
                brd+1,net_networks[brd].name);
            }
          }
          nl();
          npr(get_string(1068));
          mpl(5);
          input(s3,5);
          network=atoi(s3);

          if ((network-1) < net_num_max) {
/* Packscan freq.  Others would also go here. */
            nl();
            sprintf(s3,"FILEREQ /F:%s /C:1 /A:%s /N:%d",s2,s1,network-1);
            extern_prog(s3, EFLAG_SHRINK);
            topscreen();
            nl();
            done=1;
          }
        } else
          done=2;
        break;
      case 'S':
        done=2;
        break;
      case 13:
      case ' ':
      case 'D':
        done=1;
        break;
    }
  } while (!done);
  return(done);
}


int getnetnum(char *netnam)
{
  int i,matched=0;

  for (i=0; i<net_num_max; i++) {
    if (stricmp(net_networks[i].name, netnam)==0) {
      matched=1;
      break;
    }
  }
  if (!matched)
    i=atoi(netnam);
  return(i);
}
