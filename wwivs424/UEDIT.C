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

static unsigned long *u_qsc;


void deluser(int un)
{
  userrec u;
  int i,i1,f,n;
  mailrec m;
  char fn[81];
  votingrec v;
  voting_response vr;

  read_user(un,&u);
  if ((u.inact & inact_deleted)==0) {
    rsm(un,&u,0);
    dsr(u.name);
    u.inact |= inact_deleted;
    u.waiting=0;
    write_user(un,&u);
    f=open_email(1);
    if (f>0) {
      i1=filelength(f)/sizeof(mailrec);
      for (i=0; i<i1; i++) {
        sh_lseek(f,((long) i) * sizeof(mailrec), SEEK_SET);
        sh_read(f,(void *)(&m),sizeof(mailrec));
        if (((m.tosys==0) && (m.touser==un)) ||
            ((m.fromsys==0) && (m.fromuser==un))) {
          delmail(f,i);
        }
      }
      f=sh_close(f);
    }
    sprintf(fn,"%sVOTING.DAT",syscfg.datadir);
    f=sh_open(fn,O_RDWR | O_BINARY, S_IREAD | S_IWRITE);
    n=(int) (filelength(f) / sizeof(votingrec)) -1;
    for (i=0; i<20; i++)
      if (u.votes[i]) {
        if (i<=n) {
          sh_lseek(f,((long) i) * sizeof(votingrec), SEEK_SET);
          sh_read(f,(void *)&v,sizeof(votingrec));
          vr=v.responses[u.votes[i]-1];
          vr.numresponses--;
          v.responses[u.votes[i]-1]=vr;
          sh_lseek(f,((long) i) * sizeof(votingrec), SEEK_SET);
          sh_write(f,(void *)&v,sizeof(votingrec));
        }
        u.votes[i]=0;
      }
    f=sh_close(f);
    write_user(un,&u);
  }
}


void outstr_x2(char *s)
{
  ansic_x(2);
  outstr(s);
  ansic_x(1);
}


void print_data(int un, userrec *u, int lng, int cls)
{
  char s[81],s1[81],s2[81],s3[81];
  int i;

  if (cls)
    outchr(12);
  if ((u->inact) & inact_deleted) {
   ansic_x(6);
    pl(get_string(236));
    nl();
  }
  outstr_x2(get_string(1005)); pl(nam(u,un));
  outstr_x2(get_string(238)); pl(u->realname);
  if (lng) {
    if (u->street[0]) {
      outstr_x2(get_string(239)); pl(u->street);
    }
    if (u->city[0] || u->state[0] || u->country[0] || u->zipcode[0]) {
      outstr_x2(get_string(240)); npr("%s, %s  %s (%s)\r\n",
                            u->city, u->state, u->zipcode, u->country);
    }
  }
  if (u->registered) {
    outstr_x2(get_string(1421));
    npr("%s    ",daten_to_date(u->registered));
    npr("%s%s", get_string(1422),daten_to_date(u->expires));
    nl();
  }
  if ((u->callsign[0])!=0) {
    outstr_x2(get_string(241)); pl(u->callsign);
  }
  outstr_x2(get_string(242)); pl(u->phone);
  if (u->dataphone[0]) {
    outstr_x2(get_string(243)); pl(u->dataphone);
  }
  outstr_x2(get_string(244));
    npr("%d %c  (%02d/%02d/%02d)\r\n",
    u->age, u->sex, u->month, u->day, u->year);
  outstr_x2(get_string(245)); pl(ctypes[u->comp_type]);
  if (u->forwardusr) {
    outstr_x2(get_string(246));
    if (u->forwardsys) {
      if (net_num_max>1) {
        npr("%s #%d @%d\r\n", net_networks[u->net_num].name,
          u->forwardusr, u->forwardsys);
      } else {
        npr("#%d @%d\r\n",u->forwardusr, u->forwardsys);
      }
    } else {
      npr("#%d\r\n",u->forwardusr);
    }
  }
  if (lng) {
    outstr_x2(get_string(247));
    if (lecho)
      outs(u->pw);
    if ((incom) && (thisuser.sl==255))
      pr1(u->pw);
    else
      pr1(get_string(1527));
    nl();
    outstr_x2(get_string(248));
    npr("%s   %s\r\n",(u->laston),(u->firston));
    outstr_x2(get_string(249));
    if (u->emailnet)
      npr("P=%u E=%u F=%u W=%u O=%u D=%u\r\n",u->msgpost, u->emailsent,
          u->feedbacksent, u->waiting, u->emailnet, u->deletedposts);
    else
      npr("P=%u E=%u F=%u W=%u D=%u\r\n",u->msgpost, u->emailsent,
          u->feedbacksent, u->waiting, u->deletedposts);
    outstr_x2(get_string(250));
      npr("%d  %d  I=%d\r\n",
        u->logons,(strcmp(u->laston,date()))?0:u->ontoday,u->illegal);
    outstr_x2(get_string(251));
      npr("U=%d-%ldk   D=%d-%ldk\r\n",
        u->uploaded, u->uk, u->downloaded, u->dk);
    outstr_x2(get_string(252)); pln(u->lastrate);
  }
  if (u->note[0]) {
    outstr_x2(get_string(253)); pl(u->note);
  }
  if (u->ass_pts) {
    outstr_x2(get_string(254)); pln(u->ass_pts);
  }
  outstr_x2(get_string(255)); pln(u->sl);
  outstr_x2(get_string(318)); pln(u->dsl);
  if (u_qsc) {
    if ((*u_qsc)!=999) {
      outstr_x2(get_string(256)); pln(*u_qsc);
    }
  }
  if (u->exempt) {
    outstr_x2(get_string(257)); pln(u->exempt);
  }
  strcpy(s3,restrict_string);
  for (i=0; i<=15; i++) {
    if (u->ar & (1 << i))
      s[i]='A'+i;
    else
      s[i]=32;
    if (u->dar & (1 << i))
      s1[i]='A'+i;
    else
      s1[i]=32;
    if (u->restrict & (1 << i))
      s2[i]=s3[i];
    else
      s2[i]=32;
  }
  s[16]=0;
  s1[16]=0;
  s2[16]=0;
  if ((u->ar)!=0) {
    outstr_x2(get_string(258)); pl(s);
  }
  if ((u->dar)!=0) {
    outstr_x2(get_string(259)); pl(s1);
  }
  if ((u->restrict)!=0) {
    outstr_x2(get_string(260)); pl(s2);
  }
  if (u->wwiv_regnum) {
    outstr_x2(get_string(261));
      npr("%ld\r\n", u->wwiv_regnum);
  }
  if (lng)
    print_affil(u);
}


/****************************************************************************/



int matchuser(int un)
{
  userrec u;

  read_user(un,&u);
  sp=search_pattern;
  return(match_user(&u));
}


int match_user(userrec *u)
{
  int ok=1,not=0,done=0,less=0,cpf=0,cpp=0,and=1,gotfcn=0,evalit=0,tmp,tmp1,tmp2;
  char fcn[20],parm[80],ts[40];
  long l;

  do {
    if (*sp==0)
      done=1;
    else {
      if (strchr("()|&!<>",*sp)) {
        switch(*sp++) {
          case '(':
            evalit=2;
            break;
          case ')':
            done=1;
            break;
          case '|':
            and=0;
            break;
          case '&':
            and=1;
            break;
          case '!':
            not=1;
            break;
          case '<':
            less=1;
            break;
          case '>':
            less=0;
            break;
        }
      } else if (*sp=='[') {
        gotfcn=1;
        sp++;
      } else if (*sp==']') {
        evalit=1;
        ++sp;
      } else if ((*sp!=' ') || (gotfcn)) {
        if (gotfcn) {
          if (cpp<22)
            parm[cpp++]=*sp++;
          else
            sp++;
        } else {
          if (cpf<sizeof(fcn)-1)
            fcn[cpf++]=*sp++;
          else
            sp++;
        }
      } else
        ++sp;
      if (evalit) {
        if (evalit==1) {
          fcn[cpf]=0;
          parm[cpp]=0;
          tmp=1;
          tmp1=atoi(parm);

          if (!strcmp(fcn,"SL")) {
            if (less)
              tmp=(tmp1>u->sl);
            else
              tmp=(tmp1<u->sl);
          } else if (!strcmp(fcn,"DSL")) {
            if (less)
              tmp=(tmp1>u->dsl);
            else
              tmp=(tmp1<u->dsl);
          } else if (!strcmp(fcn,"AR")) {
            if ((parm[0]>='A') && (parm[0]<='P')) {
              tmp1=1 << (parm[0]-'A');
              if (u->ar & tmp1)
                tmp=1;
              else
                tmp=0;
            } else
              tmp=0;
          } else if (!strcmp(fcn,"DAR")) {
            if ((parm[0]>='A') && (parm[0]<='P')) {
              tmp1=1 << (parm[0]-'A');
              if (u->dar & tmp1)
                tmp=1;
              else
                tmp=0;
            } else
              tmp=0;
          } else if (!strcmp(fcn,"SEX")) {
            tmp=parm[0]==u->sex;
          } else if (!strcmp(fcn,"AGE")) {
            if (less)
              tmp=(tmp1>u->age);
            else
              tmp=(tmp1<u->age);
          } else if (!strcmp(fcn,"LASTON")) {
            time(&l);
            tmp2=(unsigned int)( (l-u->daten)/24.0/3600.0 );
            if (less)
              tmp=tmp2<tmp1;
            else
              tmp=tmp2>tmp1;
          } else if (!strcmp(fcn,"AREACODE")) {
            tmp=(!strncmp(parm,u->phone,3));
          } else if (!strcmp(fcn,"RESTRICT")) {
            ;
          } else if (!strcmp(fcn,"LOGONS")) {
            if (less)
              tmp=u->logons<tmp1;
            else
              tmp=u->logons>tmp1;
          } else if (!strcmp(fcn,"REALNAME")) {
            strcpy(ts,u->realname);
            strupr(ts);
            tmp=(strstr(ts,parm)!=NULL);
          } else if (!strcmp(fcn,"BAUD")) {
            if (less)
              tmp=u->lastrate<(unsigned short)tmp1;
            else
              tmp=u->lastrate>(unsigned short)tmp1;
          } else if (!strcmp(fcn,"COMP_TYPE")) {
            tmp=u->comp_type==tmp1;
          }

        } else
          tmp=match_user(u);

        if (not)
          tmp=!tmp;
        if (and)
          ok = ok && tmp;
        else
          ok = ok || tmp;

        not=less=cpf=cpp=gotfcn=evalit=0;
        and=1;
      }
    }
  } while (!done);
  return(ok);
}

/****************************************************************************/

void changeopt(void)
{
  helpl=42;
  outchr(12);
  pl(get_string(262));
  if (search_pattern[0])
    pl(search_pattern);
  else
    pl(get_string(263));
  nl();
  nl();
  prt(5,get_string(264));
  if (yn()) {
    pl(get_string(265));
    prt(2,":");
    input(search_pattern,75);
  }
  helpl=0;
}


/****************************************************************************/

void auto_val(int n, userrec *u)
{
  if (u->sl==255)
    return;
  u->sl=syscfg.autoval[n].sl;
  u->dsl=syscfg.autoval[n].dsl;
  u->ar=syscfg.autoval[n].ar;
  u->dar=syscfg.autoval[n].dar;
  u->restrict=syscfg.autoval[n].restrict;
}


void uedit(int usern, int other)
{
  char s[81],s1[81],s2[81],ch,ch1;
  int i,i1,un,done,nu,done1,full,temp_full,tempu,cls=1,done2;
  userrec u;
  int m,dd;

  u_qsc=(unsigned long *)malloca(syscfg.qscn_len);

  if (incom)
    full=0;
  else
    full=1;
  if (other&1)
    full=0;
  if (other&2)
    cls=0;
  un=usern;
  done=0;
  read_user(un,&u);
  nu=number_userrecs();
  do {
    read_user(un,&u);
    read_qscn(un,u_qsc,0);
    done1=0;
    temp_full=0;
    do {
      print_data(un,&u, ((full) || (temp_full)), cls);
      nl();
      prt(2,get_string(266));
      if ((thisuser.sl==255) || (wfc))
        ch=onek("ACDEGILMNOPQRSTUXYZ0123456789[]{}/,.?~%:");
      else
        ch=onek("ACDEGILMNOPQRSTUYZ0123456789[]{}/,.?%");
      switch(ch) {
        case 'A':
          nl();
          prt(2,get_string(280));
          ch1=onek("\rABCDEFGHIJKLMNOP");
          if (ch1!=13) {
            ch1-='A';
            if ((wfc) || (thisuser.ar & (1 << ch1))) {
              u.ar ^= (1 << ch1);
              write_user(un,&u);
            }
          }
          break;
        case 'C':
          nl();
          prt(2,get_string(270));
          input(s,sizeof(u.callsign)-1);
          if (s[0]) {
            strcpy(u.callsign,s);
            write_user(un,&u);
          } else {
            prt(5,get_string(271));
            if (yn()) {
              u.callsign[0]=0;
              write_user(un,&u);
            }
          }
          break;
        case 'D':
          if (((u.inact & inact_deleted)==0) && (actsl>u.sl)) {
            prt(5,get_string(267));
            if (yn()) {
              deluser(un);
              read_user(un,&u);
            }
          }
          break;
        case 'E':
          nl();
          prt(2,get_string(277));
          input(s,3);
          i=atoi(s);
          if ((i>=0) && (i<=255) && (s[0])) {
            u.exempt=i;
            write_user(un,&u);
          }
          break;
        case 'G':
          prt(5,get_string(1642));
          if (yn()) {
            nl();
            outstr(get_string(274));
            npr("%02d/%02d/%02d\r\n",(int) u.month, (int) u.day, (int) u.year);
            input_age(&u);
            write_user(un,&u);
          }
          break;
        case 'I':
          nl();
          prt(2,get_string(281));
          ch1=onek("\rABCDEFGHIJKLMNOP");
          if (ch1!=13) {
            ch1-='A';
            if ((wfc) || (thisuser.dar & (1 << ch1))) {
              u.dar ^= (1 << ch1);
              write_user(un,&u);
            }
          }
          break;
        case 'L':
          nl();
          prt(2,get_string(269));
          inputl(s,sizeof(u.realname)-1);
          if (s[0]) {
            strcpy(u.realname,s);
            write_user(un,&u);
          }
          break;
        case 'M':
          nl();
          pl(get_string(275));
          nl();
          for (i=0; ctypes[i]; i++)
            npr("%d. %s\r\n",i+1,ctypes[i]);
          nl();
          prt(2,get_string(276));
          input(s,2);
          i1=atoi(s);
          if ((i1>0) && (i1<=i)) {
            u.comp_type=i1-1;
            if (checkcomp("Ami"))
              u.colors[0]=4;
            else
              u.colors[0]=7;
            write_user(un,&u);
          }
          break;
        case 'N':
          nl();
          prt(2,get_string(69));
          input(s,sizeof(u.name)-1);
          if (s[0]) {
            if (finduser(s)<1) {
              dsr(u.name);
              strcpy(u.name,s);
              isr(un,u.name);
              write_user(un,&u);
            }
          }
          break;
        case 'O':
          nl();
          prt(2,get_string(273));
          inputl(s,sizeof(u.note)-1);
          strcpy(u.note,s);
          write_user(un,&u);
          break;
        case 'P':
          nl();
          prt(2,get_string(272));
          input(s,sizeof(u.phone)-1);
          if (s[0]) {
            strcpy(u.phone,s);
            write_user(un,&u);
          }
          prt(2,get_string(287));
          input(s,sizeof(u.dataphone)-1);
          if (s[0]) {
            if (s[0]=='0')
              u.dataphone[0]=0;
            else
              strcpy(u.dataphone,s);
            write_user(un,&u);
          }
          break;
        case 'Q':
          done=1;
          done1=1;
          break;
        case 'R':
          if (u.inact & inact_deleted) {
            u.inact ^= inact_deleted;
            isr(un,u.name);
            write_user(un,&u);
          }
          break;
        case 'S':
          if (u.sl==255)
            break;
          if (u.sl>=actsl)
            break;
          nl();
          prt(2,get_string(76));
          input(s,3);
          i=atoi(s);
          if ((!wfc) && (i>=actsl))
            i=-1;
          if ((i>=0) && (i<255) && (s[0])) {
            u.sl=i;
            write_user(un,&u);
            if (un==usernum)
              actsl=i;
          }
          break;
        case 'T':
          if (u.dsl==255)
            break;
          if (u.dsl>=thisuser.dsl)
            break;
          nl();
          prt(2,get_string(153));
          input(s,3);
          i=atoi(s);
          if ((!wfc) && (i>=thisuser.dsl))
            i=-1;
          if ((i>=0) && (i<255) && (s[0])) {
            u.dsl=i;
            write_user(un,&u);
          }
          break;
        case 'U':
          nl();
          prt(2,get_string(268));
          input(s,sizeof(u.name)-1);
          i=finduser1(s);
          if (i>0) {
            un=i;
            done1=1;
          }
          break;
        case 'X':
          if (!(sysinfo.flags & OP_FLAGS_USER_REGIST))
            break;
          nl();
          if (u.registered!=0) {
            strncpy(s1,daten_to_date(u.registered),sizeof(s1));
            strncpy(s2,daten_to_date(u.expires),sizeof(s2));
            npr("%s%s%s%s\r\n", get_string(1387), s1, get_string(1388), s2);
          } else
            pl(get_string(1389));
          do {
            nl();
            pl(get_string(1390));
            pl(get_string(792));
            outstr(":");
            input(s,8);
          } while ((strlen(s)!=8) && (s[0]!=0));
          if (s[0]==0)
            strcpy(s,date());

          m=atoi(s);
          dd=atoi(&(s[3]));
          if ((strlen(s)==8) && (m>0) && (m<=12)
            && (dd>0) && (dd<32))
            u.registered=date_to_daten(s);
          else
            nl();
          do {
            nl();
            pl(get_string(1392));
            pl(get_string(792));
            outstr(":");
            input(s,8);
          } while ((strlen(s)!=8) && (s[0]!=0));
          if (strlen(s)==8) {
            m=atoi(s);
            dd=atoi(&(s[3]));
          } else {
            u.registered=0L;
            u.expires=0L;
          }
          if ((strlen(s)==8) && (m>0) && (m<=12)
            && (dd>0) && (dd<32))
            u.expires=date_to_daten(s);
          write_user(un,&u);
          break;
        case 'Y':
          if (u_qsc) {
            nl();
            prt(2,get_string(278));
            input(s,3);
            i=atoi(s);
            if ((i>=0) && (i<=999) && (s[0])) {
              *u_qsc=i;
              write_qscn(un,u_qsc,0);
            }
          }
          break;
        case 'Z':
          nl();
          npr("%s%s\r\n",get_string(1309),restrict_string);
          do {
            prt(2,get_string(279));
            s[0]=13;
            s[1]='?';
            strcpy(&(s[2]),restrict_string);
            ch1=onek(s);
            if (ch1==32)
              ch1=13;
            if (ch1=='?')
              printmenu(10);
            if ((ch1!=13) && (ch1!='?')) {
              i=-1;
              for (i1=0; i1<16; i1++)
                if (ch1==s[i1+2])
                  i=i1;
              if (i>-1) {
                u.restrict ^= (1 << i);
                write_user(un,&u);
              }
            }
          } while ((!hangup) && (ch1=='?'));
          break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
          if (ch!='0')
            i=ch-'1';
          else
            i=9;
          if ((actsl>=syscfg.autoval[i].sl) &&
              (thisuser.dsl>=syscfg.autoval[i].dsl) &&
              (((~thisuser.ar) & syscfg.autoval[i].ar)==0) &&
              (((~thisuser.dar) & syscfg.autoval[i].dar)==0)) {
            auto_val(i,&u);
            write_user(un,&u);
          }
          break;
        case ']':
          ++un;
          if (un>nu)
            un=1;
          done1=1;
          break;
        case '[':
          --un;
          if (un==0)
            un=nu;
          done1=1;
          break;
        case '}':
          tempu=un;
          ++un;
          if (un>nu)
            un=1;
          while ((un!=tempu) && (!matchuser(un))) {
            ++un;
            if (un>nu)
              un=1;
          }
          done1=1;
          break;
        case '{':
          tempu=un;
          --un;
          if (un<1)
            un=nu;
          while ((un!=tempu) && (!matchuser(un))) {
            --un;
            if (un<1)
              un=nu;
          }
          done1=1;
          break;
        case '/':
          changeopt();
          break;
        case ',':
          temp_full=(!temp_full);
          break;
        case '.':
          full=(!full);
          temp_full=full;
          break;
        case '?':
          printmenu(6);
          getkey();
          break;
        case '~':
          u.ass_pts=0;
          write_user(un,&u);
          break;
        case '%':
          nl();
          prt(2,get_string(282));
          inputl(s1,sizeof(u.street)-1);
          if (s1[0]) {
            strcpy(u.street,s1);
            write_user(un,&u);
          }
          prt(2,get_string(283));
          inputl(s1,sizeof(u.city)-1);
          if (s1[0]) {
            strcpy(u.city,s1);
            write_user(un,&u);
          }
          prt(2,get_string(284));
          input(s1,sizeof(u.state)-1);
          if (s1[0]) {
            strcpy(u.state,s1);
            write_user(un,&u);
          }
          prt(2,get_string(285));
          input(s1,sizeof(u.country)-1);
          if (s1[0]) {
            strcpy(u.country,s1);
            write_user(un,&u);
          }
          prt(2,get_string(286));
          input(s1,sizeof(u.zipcode)-1);
          if (s1[0]) {
            strcpy(u.zipcode,s1);
            write_user(un,&u);
          }
          prt(2,get_string(287));
          input(s1,sizeof(u.dataphone)-1);
          if (s1[0]){
            strcpy(u.dataphone,s1);
            write_user(un,&u);
          }
          break;
        case ':':
          do {
            done2=0;
            nl();
            pl(get_string(288));
            pl(get_string(289));
            pl(get_string(290));
            pl(get_string(291));
            pl(get_string(292));
            pl(get_string(293));
            pl(get_string(294));
            pl(get_string(295));
            pl(get_string(296));
            pl(get_string(12));
            outstr(get_string(297));
            ch1=onek("Q12345678");
            switch(ch1) {
              case '1': u.realname[0]=0; break;
              case '2': u.year=0; break;
              case '3': u.street[0]=0; break;
              case '4': u.city[0]=0; break;
              case '5': u.state[0]=0; break;
              case '6': u.country[0]=0; break;
              case '7': u.zipcode[0]=0; break;
              case '8': u.dataphone[0]=0; break;
              case 'Q': done2=1; break;
            }
          } while (!done2);
          write_user(un,&u);
          break;
      }
    } while ((!done1) && (!hangup));
  } while ((!done) && (!hangup));
  if (u_qsc)
    bbsfree(u_qsc);
  u_qsc=NULL;
  if (!wfc)
    topscreen();
}

/***************************************************************************/

unsigned char *daten_to_date(long dt)
{
  struct date d;
  struct time t;
  static unsigned char s[9];

  unixtodos(dt,&d,&t);

  sprintf(s, "%02d/%02d/%02d", d.da_mon, d.da_day,
    d.da_year-2000 > 0 ? d.da_year-2000 : d.da_year-1900);

  return(s);
}

/***************************************************************************/

void print_affil(userrec *u)
{
  net_system_list_rec *csne;

  if ((u->net_num==0) || (u->homesys==0))
    return;
  set_net_num(u->net_num);
  csne=next_system(u->homesys);
  ansic_x(2);
  outstr(get_string(1431));
  ansic_x(1);
  npr("@%u, %s, on %s.",u->homesys,csne->name,net_name);
  nln(2);
}


