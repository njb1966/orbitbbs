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



int same_email(tmpmailrec *tm, mailrec *m)
{
  if ((tm->fromsys!=m->fromsys) ||
      (tm->fromuser!=m->fromuser) ||
      (m->tosys!=0) ||
      (m->touser!=usernum) ||
      (tm->daten!=m->daten) ||
      (tm->index==-1) ||
      (memcmp(&tm->msg, &m->msg, sizeof(messagerec)!=0)))
    return(0);
  else
    return(1);
}


void purgemail(tmpmailrec *mloc,int mw,int *curmail, mailrec *m1, slrec *ss)
{
  int i, f;
  mailrec m;

  if (m1->fromuser==65535)
    return;

  ansic(5);
  if ((m1->anony & anony_sender) && ((ss->ability & ability_read_email_anony)==0)) {
    outstr(get_string(699));
  } else {
    outstr(get_string(700));
    if (m1->fromsys) {
      npr("#%u @%u? ",m1->fromuser, m1->fromsys);
    } else {
      npr("#%u? ",m1->fromuser);
    }
  }
  if (yn()) {
    f=open_email(1);
    if (f<0)
      return;
    for (i=0; i<mw; i++) {
      if (mloc[i].index>=0) {
        sh_lseek(f,((long) mloc[i].index) * sizeof(mailrec),SEEK_SET);
        sh_read(f,(void *) (&m),sizeof(mailrec));
        if (same_email(mloc+i, &m)) {
          if ((m.fromuser==m1->fromuser) && (m.fromsys==m1->fromsys)) {
            outstr(get_string(701));
            npr("%d\r\n",i+1);
            delmail(f,mloc[i].index);
            mloc[i].index=-1;
            if (*curmail==i)
              ++(*curmail);
          }
        }
      } else {
        if (*curmail==i)
          ++(*curmail);
      }
    }
    f=sh_close(f);
  }
}


#define MAXMAIL 255


void resynch_email(tmpmailrec *mloc, int mw, int rec, mailrec *m, int del, unsigned short stat)
{
  int f, i, i1, mp, mfl;
  mailrec m1;

  f=open_email(del || stat);
  if (f>0) {
    mfl=filelength(f)/sizeof(mailrec);

    for (i=0; i<mw; i++) {
      if (mloc[i].index>=0)
        mloc[i].index=-2;
    }

    mp=0;

    for (i=0; i<mfl; i++) {
      sh_lseek(f,((long) i) * sizeof(mailrec),SEEK_SET);
      sh_read(f,(void *) (&m1),sizeof(mailrec));

      if ((m1.tosys==0) && (m1.touser==usernum)) {
        for (i1=mp; i1<mw; i1++) {
          if (same_email(mloc+i1, &m1)) {
            mloc[i1].index=i;
            mp=i1+1;
            if (i1==rec)
              *m=m1;
            break;
          }
        }
      }
    }

    for (i=0; i<mw; i++) {
      if (mloc[i].index==-2)
        mloc[i].index=-1;
    }

    if (stat && !del && (mloc[rec].index>=0)) {
      m->status |= stat;
      sh_lseek(f,((long) mloc[rec].index) * sizeof(mailrec),SEEK_SET);
      sh_write(f,(void *) (m),sizeof(mailrec));
    }
    if (del && (mloc[rec].index>=0)) {
      if (del==2) {
        m->touser=0;
        m->tosys=0;
        m->daten=0xffffffff;
        m->msg.storage_type=0;
        m->msg.stored_as=0xffffffff;
        sh_lseek(f,((long) mloc[rec].index) * sizeof(mailrec),SEEK_SET);
        sh_write(f,(void *) (m),sizeof(mailrec));
      } else {
        delmail(f, mloc[rec].index);
      }
      mloc[rec].index=-1;
    }

    f=sh_close(f);

  } else {
    mloc[rec].index=-1;
  }
}


int read_same_email(tmpmailrec *mloc, int mw, int rec, mailrec *m, int del, unsigned short stat)
{
  int f;

  if (mloc[rec].index<0)
    return(0);

  f=open_email(del || stat);
  sh_lseek(f,((long) mloc[rec].index) * sizeof(mailrec),SEEK_SET);
  sh_read(f,(void *) (m),sizeof(mailrec));

  if (!same_email(mloc+rec,m)) {
    f=sh_close(f);
    read_status();
    if (emchg) {
      resynch_email(mloc, mw, rec, m, del, stat);
    } else {
      mloc[rec].index=-1;
    }
  } else {

    if (stat && !del && (mloc[rec].index>=0)) {
      m->status |= stat;
      sh_lseek(f,((long) mloc[rec].index) * sizeof(mailrec),SEEK_SET);
      sh_write(f,(void *) (m),sizeof(mailrec));
    }
    if (del) {
      if (del==2) {
        m->touser=0;
        m->tosys=0;
        m->daten=0xffffffff;
        m->msg.storage_type=0;
        m->msg.stored_as=0xffffffff;
        sh_lseek(f,((long) mloc[rec].index) * sizeof(mailrec),SEEK_SET);
        sh_write(f,(void *) (m),sizeof(mailrec));
      } else {
        delmail(f, mloc[rec].index);
      }
      mloc[rec].index=-1;
    }

    f=sh_close(f);
  }

  if (mloc[rec].index!=-1)
    return(1);
  else
    return(0);
}

void add_netsubscriber(unsigned short sn)
{
  char s[10], s1[10],ns1[80];
  FILE *host;

  if (!valid_system(sn))
    sn=0;

  nl();
  prt(1,get_string(1265));
  nln(2);
  prt(2,get_string(1266));
  mpl(7);
  input(s,7);
  if (s[0]==0)
    return;
  strcpy(s1,s);
  sprintf(ns1,"%sN%s.NET",net_data,s);
  if (!exist(ns1)) {
    nl();
    ansic(6);
    npr("%s%s",get_string(1267),ns1);
    nl();
    return;
  }
  nl();
  if (sn) {
    outstr(get_string(1268));
    npr("%u.%s",sn,net_name);
    outstr(get_string(1269));
    npr("%s? ",s);
  }
  if (!sn || !ny()) {
    prt(2,get_string(1270));
    mpl(5);
    input(s,5);
    if (!s[0])
      return;
    sn=atoi(s);
    if (!valid_system(sn)) {
      npr("@%u%s%s.", sn, get_string(1271), net_name);
      nl();
      return;
    }
  }
  sprintf(s,"%u\n",sn);
  host=fsh_open(ns1,"a+t");
  if (host) {
    fprintf(host,s);
    fsh_close(host);

    if (exist("AUTOSEND.EXE")) {
      outstr(get_string(1525));
      if (yn()) {
        sprintf(ns1,"AUTOSEND.EXE %s %u .%d",s1,sn,net_num);
        extern_prog(ns1,EFLAG_SHRINK);
      }
    }
  }
}

void readmail(void)
{
  int i,i2,f,mw,mfl,curmail,done,abort=0,next,okmail,tp,nn;
  unsigned short xx;
  char s[201],s1[205],s2[81],*b,*ss1;
  mailrec m, m1;
  slrec ss;
  userrec u;
  char ch;
  long len,num_mail,num_mail1;
  unsigned short un, sy;
  net_system_list_rec *csne;
  tmpmailrec *mloc;

  emchg=0;

  mloc=malloca(MAXMAIL*sizeof(tmpmailrec));
  if (!mloc) {
    pl(get_string(1272));
    return;
  }

  if (menu_on())
    printmenu(4);

  if (rip_on()) {
    cleared = NEEDCLEAR;
    setmsgview(11);
  }

  write_inst(INST_LOC_RMAIL,0,INST_FLAGS_NONE);
  ss=syscfg.sl[actsl];
  f=open_email(0);
  if (f<0) {
    nln(2);
    pl(get_string(702));
    nl();
    bbsfree(mloc);
    return;
  }
  mfl=filelength(f)/sizeof(mailrec);
  mw=0;
  for (i=0; (i<mfl) && (mw<MAXMAIL); i++) {
    sh_lseek(f,((long) (i)) * (sizeof(mailrec)), SEEK_SET);
    sh_read(f,(void *)(&m),sizeof(mailrec));
    if ((m.tosys==0) && (m.touser==usernum)) {
      mloc[mw].index=i;
      mloc[mw].fromsys=m.fromsys;
      mloc[mw].fromuser=m.fromuser;
      mloc[mw].daten=m.daten;
      mloc[mw].msg=m.msg;
      mw++;
    }
  }
  f=sh_close(f);
  thisuser.waiting=mw;
  if (usernum==1)
    fwaiting=mw;
  if (mw==0) {
    nln(2);
    pl(get_string(27));
    nl();
    bbsfree(mloc);
    return;
  }
#ifdef RIPDRIVE
  rd_coff();
#endif
  if (mw==1)
    curmail=0;
  else {
    nln(2);
    ansic(0);
    pl(get_string(703));
    nl();

    if ((thisuser.screenchars>=80) && (sysinfo.mail_who_field_len)) {
      (okansi())?strcpy(s1,"ÁÂÄ"):strcpy(s1,"++-");
      sprintf(s,"7%s%c",charstr(4,s1[2]),s1[1]);
      strcat(s,charstr(sysinfo.mail_who_field_len-4,s1[2]));
      strcat(s,charstr(1,s1[1]));
      strcat(s,charstr(thisuser.screenchars-sysinfo.mail_who_field_len-3,s1[2]));
      pla(s,&abort);
    }

    for (i=0; ((i<mw) && (!abort)); i++) {
      if (!read_same_email(mloc, mw, i, &m, 0, 0))
        continue;

      tp=80;
      if (m.status & status_source_verified)
        tp -= 2;
      if (m.status & status_new_net) {
        tp -= 1;
        if (strlen(m.title)<=tp) {
          nn=(unsigned char) m.title[tp+1];
        } else
          nn=0;
      } else
        nn=0;
      sprintf(s,"2%3d%s7%c1 ",
              i+1,
              m.status&status_seen?" ":"3*",
              okansi()?'³':'|');

      if ((m.anony & anony_sender) && ((ss.ability & ability_read_email_anony)==0)) {
        strcat(s,get_string(482));
      } else {
        if (m.fromsys==0) {
          if (m.fromuser==65535) {
            if (nn!=255)
              strcat(s,net_networks[nn].name);
          } else {
            read_user(m.fromuser,&u);
            strcat(s,nam(&u,m.fromuser));
          }
        } else {
          set_net_num(nn);
          csne=next_system(m.fromsys);
          if (csne)
            ss1=csne->name;
          else
            ss1=get_string(622);
          if (nn==255) {
            sprintf(s1,"%s #%u @%u",get_string(704), m.fromuser,m.fromsys);
          } else {
            if (net_num_max>1)
              sprintf(s1,"#%u @%u.%s (%s)",
                m.fromuser,m.fromsys,net_networks[nn].name,ss1);
            else
              sprintf(s1,"%s %u @%u (%s)",get_string(658), m.fromuser,m.fromsys,ss1);
          }
          strcat(s,s1);
        }
      }

      if ((thisuser.screenchars >= 80) && (sysinfo.mail_who_field_len)) {
        if (strlen(stripcolors(s))>sysinfo.mail_who_field_len)
          while (strlen(stripcolors(s))>sysinfo.mail_who_field_len)
            s[strlen(s)-1]=0;
        strcat(s,charstr((sysinfo.mail_who_field_len+1)-strlen(stripcolors(s)),' '));
        if (okansi())
          strcat(s,"7³1");
        else
          strcat(s,"|");
        strcat(s," ");
        strcat(s,stripcolors(m.title));
        while (strlen(stripcolors(s))>thisuser.screenchars-1)
          s[strlen(s)-1]=0;
      }
      pla(s,&abort);
      if ((i==(mw-1)) && (thisuser.screenchars>=80) && (!abort) && (sysinfo.mail_who_field_len)) {
        (okansi())?strcpy(s1,"ÁÂÄ"):strcpy(s1,"++-");
        sprintf(s,"7%s%c",charstr(4,s1[2]),s1[0]);
        strcat(s,charstr(sysinfo.mail_who_field_len-4,s1[2]));
        strcat(s,charstr(1,s1[0]));
        strcat(s,charstr(thisuser.screenchars-sysinfo.mail_who_field_len-3,s1[2]));
        pla(s,&abort);
      }
    }
    nl();
#ifdef RIPDRIVE
    rd_con();
#endif
    helpl=10;
    pl(get_string(705));
#ifdef RIPDRIVE
    if (rd_on())
      printf("\n\x1b[A");
#endif
    outstr(":");
    input(s,3);
    if (strchr(s,'Q')!=NULL) {
      bbsfree(mloc);
      return;
    }
    i=atoi(s);
    if (i)
      if (i<=mw)
        curmail=i-1;
      else
        curmail=0;
    else
      curmail=0;
  }
  done=0;
  do {
    if (E_C) {
      sprintf(s,"1%u/%u0",curmail+1,mw);
      strcat(s,charstr(12-strlen(stripcolors(s)),'.'));
      strcat(s," 2");
    } else {
      sprintf(s,"(%u/%u): ",curmail+1,mw);
    }
    abort=0;
    nln(2);
    if (rip_on())
      msgheader(1);
    osan(s,&abort,&next);
    next=0;
    ansic_x(sysinfo.msg_color);
    s[0]=0;

    if (!read_same_email(mloc, mw, curmail, &m, 0, 0)) {
      strcat(s,get_string(706));
      okmail=0;
      pl(s);
      nln(2);
    } else {
      strcat(s,m.title);
      strcpy(irt,m.title);
      irt_name[0]=0;
      abort=0;
      i=((ability_read_email_anony & ss.ability)!=0);
      okmail=1;
      pla(s,&abort);
      if ((m.fromsys) && (!m.fromuser))
        grab_user_name(&(m.msg),"EMAIL");
      else
        net_email_name[0]=0;
      tp=80;
      if (m.status & status_source_verified) {
        tp -= 2;
        if (strlen(m.title)<=tp) {
          xx=*(short *) (m.title+tp+1);
          strcpy(s,get_string(707));
          sprintf(s+strlen(s),"%u",xx);
          if (xx==1) {
            strcat(s,get_string(708));
          } else if ((xx>256) && (xx<512)) {
            strcpy(s2,get_string(709));
            sprintf(s2+strlen(s2),"%d)",xx-256);
            strcat(s,s2);
          }
        } else {
          strcpy(s,get_string(710));
        }
        if (!abort) {
          ansic(4);
          pla(s,&abort);
        }
      }
      if (m.status & status_new_net) {
        tp -= 1;
        if (strlen(m.title)<=tp) {
          nn=m.title[tp+1];
        } else
          nn=0;
      } else
        nn=0;
      set_net_num(nn);
      if (nn==255)
        setorigin(0,0);
      else
        setorigin(m.fromsys, m.fromuser);
      if (!abort) {
        read_message1(&m.msg, (m.anony & 0x0f), i, &next, "EMAIL");
        if (!(m.status & status_seen)) {
          read_same_email(mloc, mw, curmail, &m, 0, status_seen);
        }
      }
#ifdef RIPDRIVE
      rd_con();
#endif
    }
    do {
      write_inst(INST_LOC_RMAIL,0,INST_FLAGS_NONE);
      set_net_num(nn);
      i2=1;
      irt_name[0]=0;
      if (!(sysinfo.flags & OP_FLAGS_MAIL_PROMPT))
        prt(2,get_string(711));
      helpl=34;
      if (so()) {
        if (sysinfo.flags & OP_FLAGS_MAIL_PROMPT)
          npr(get_string(1365));
        ch=onek("QSRIDAMF?-+GEZPVUOLCN");
      } else {
        if (cs()) {
          if (sysinfo.flags & OP_FLAGS_MAIL_PROMPT)
            npr(get_string(1366));
          ch=onek("QSRIDAMF?-+GZPVUOC");
        } else {
          if (!okmail) {
            if (sysinfo.flags & OP_FLAGS_MAIL_PROMPT)
              npr(get_string(1367));
            ch=onek("QI?-+G");
          } else {
            if (sysinfo.flags & OP_FLAGS_MAIL_PROMPT)
              npr(get_string(1368));
            ch=onek("QSRIDAMF?+-G");
          }
        }
      }
      if (okmail && !read_same_email(mloc, mw, curmail, &m, 0, 0)) {
        nl();
        pl(get_string(1273));
        nl();
        ch='R';
      }
      switch (ch) {
        case 'N':
          if (m.fromuser==1)
            add_netsubscriber(m.fromsys);
          else
            add_netsubscriber(0);
          break;
        case 'E':
          if ((so()) && (okmail)) {
            b=readfile(&(m.msg),"EMAIL",&len);
            extract_out(b,len,m.title);
          }
          i2=0;
          break;
        case 'Q':
          done=1;
          break;
        case 'O':
          if ((cs()) && (okmail) && (m.fromuser!=65535) && (nn!=255)) {
            show_files("*.FRM",syscfg.gfilesdir);
            prt(2,get_string(712));
            mpl(8);
            input(s,8);
            if (!s[0])
              break;
            sprintf(s1,"%s%s.FRM",syscfg.gfilesdir,s);
            if (!exist(s1))
              sprintf(s1,"%sFORM%s.MSG",syscfg.gfilesdir,s);
            if (exist(s1)) {
              load_workspace(s1,1);
              num_mail=((long) thisuser.feedbacksent) +
                       ((long) thisuser.emailsent) +
                       ((long) thisuser.emailnet);
              grab_quotes(NULL, NULL);
              if (m.fromuser!=65535)
                email(m.fromuser,m.fromsys,0,m.anony);
              num_mail1=((long) thisuser.feedbacksent) +
                        ((long) thisuser.emailsent) +
                        ((long) thisuser.emailnet);
              if (num_mail != num_mail1) {
                if (m.fromsys!=0)
                  sprintf(s,"%s: %s",net_name,nam1(&thisuser,usernum,net_sysnum));
                else
                  strcpy(s,nam1(&thisuser,usernum,net_sysnum));
                if (m.anony & anony_receiver)
                  strcpy(s,get_string(482));
                strcat(s,get_string(713));
                strcat(s,date());
                if (!(m.status & status_source_verified))
                  ssm(m.fromuser,m.fromsys,s);
                read_same_email(mloc, mw, curmail, &m, 1, 0);
                ++curmail;
                if (curmail>=mw)
                  done=1;
                if (!wfc)
                  topscreen();
              } else {
                sprintf(s,"%sINPUT.MSG",syscfgovr.tempdir);/* need instance */
                unlink(s);
              }
            } else {
              nl();
              pl(get_string(89));
              nl();
              i2=0;
            }
          }
          break;
        case 'G':
          ansic(2);
          outstr(get_string(714));
          npr("%u) ? ",mw);
          ansic(0);
          input(s,3);
          i2=atoi(s);
          if ((i2>0) && (i2<=mw)) {
            curmail=i2-1;
            i2=1;
          } else
            i2=0;
          break;
        case 'I':
        case '+':
          ++curmail;
          if (curmail>=mw)
            done=1;
          break;
        case '-':
          if (curmail)
            --curmail;
           break;
        case 'R':
          break;
        case '?':
          printmenu(4);
          i2=0;
          break;
        case 'D':
          if (!okmail)
            break;
          if (m.fromsys!=0)
            sprintf(s,"%s: %s",net_name,nam1(&thisuser,usernum,net_sysnum));
          else
            strcpy(s,nam1(&thisuser,usernum,net_sysnum));
          if (m.anony & anony_receiver)
            strcpy(s,get_string(482));
          strcat(s,get_string(713));
          strcat(s,date());
          if ((!(m.status & status_source_verified)) && (nn!=255))
            ssm(m.fromuser,m.fromsys,s);
        case 'Z':
          if (!okmail)
            break;
          read_same_email(mloc, mw, curmail, &m, 1, 0);
          ++curmail;
          if (curmail>=mw)
            done=1;
          if (!wfc)
            topscreen();
          break;
        case 'P':
          if (!okmail)
            break;
          purgemail(mloc,mw,&curmail,&m,&ss);
          if (curmail>=mw)
            done=1;
          if (!wfc)
            topscreen();
          break;
        case 'F':
          if (!okmail)
            break;
          if (m.status & status_multimail) {
            nl();
            pl(get_string(715));
            nl();
            break;
          }
          nln(2);
          prt(2,get_string(716));
          input(s,75);
          parse_email_info(s, &un, &sy);
          if (un || sy) {
            if (forwardm(&un, &sy)) {
              pl(get_string(656));
            }
            if ((un==usernum) && (sy==0) && (!cs())) {
              pl(get_string(469));
              un=0;
            }
            if (un || sy) {
              if (sy) {
                if (net_num_max>1) {
                  if (un) {
                    sprintf(s1,"#%u @%u.%s", un, sy, net_name);
                  } else {
                    sprintf(s1,"%s @%u.%s",net_email_name, sy, net_name);
                  }
                } else {
                  if (un) {
                    sprintf(s1,"#%u @%u", un, sy);
                  } else {
                    sprintf(s1,"%s @%u", net_email_name, sy);
                  }
                }
              } else {
                set_net_num(nn);
                read_user(un,&u);
                strcpy(s1,nam1(&u,un,net_sysnum));
              }
              if (ok_to_mail(un, sy, 0)) {
                ansic(5);
                outstr(get_string(393));
                npr("%s? ",s1);
                if (yn()) {
                  f=open_email(1);
                  if (f<0)
                    break;
                  sh_lseek(f,((long) mloc[curmail].index) * sizeof(mailrec),SEEK_SET);
                  sh_read(f,(void *) (&m),sizeof(mailrec));
                  if (!same_email(mloc+curmail, &m)) {
                    pl(get_string(1274));
                    break;
                  }

                  m1.touser=0;
                  m1.tosys=0;
                  m1.daten=0xffffffff;
                  m1.msg.storage_type=0;
                  m1.msg.stored_as=0xffffffff;
                  sh_lseek(f,((long) mloc[curmail].index) * sizeof(mailrec),SEEK_SET);
                  sh_write(f,(void *) (&m1),sizeof(mailrec));

                  f=sh_close(f);

                  i=net_num;
                  sprintf(s,"\r\n%s: %s", get_string(717),
                      nam1(&thisuser,usernum,net_sysnum));
                  set_net_num(nn);
                  lineadd(&m.msg,s,"EMAIL");
                  sprintf(s,"%s %s %s",
                      nam1(&thisuser,usernum,net_sysnum),
                      get_string(718), s1);
                  if (!(m.status & status_source_verified))
                    ssm(m.fromuser,m.fromsys,s);
                  set_net_num(i);
                  sprintf(s,get_stringx(1,101), s1);
                  --thisuser.waiting;
                  if (usernum==1)
                    --fwaiting;
                  outstr(get_string(720));
                  if ((nn!=255) && ((status.net_version>=32) || (nn==net_num))) {
                    sendout_email(m.title, &m.msg, m.anony, un, sy, 1,
                                  m.fromuser, m.fromsys?m.fromsys:net_networks[nn].sysnum, 1, nn);
                  } else {
                    sendout_email(m.title, &m.msg, m.anony, un, sy, 1,
                                  usernum, net_sysnum, 1, net_num);
                  }
                  ++curmail;
                  if (curmail>=mw)
                    done=1;
                  if (!wfc)
                    topscreen();
                  mailcheck=1;
                }
              }
            }
          }
          break;
        case 'A':
        case 'S':
        case 'M':
          if (rip_on()) {
            sprintf(s,"\n!|w000%c271610|e|#\r ", formery);
            comstr(s);
          }
          if (!okmail)
            break;
          if (menu_on())
            printmenu(321);
          num_mail=((long) thisuser.feedbacksent) +
                   ((long) thisuser.emailsent) +
                   ((long) thisuser.emailnet);
          if (nn==255) {
            pl(get_string(679));
            i2=0;
            break;
          } else if (m.fromuser!=65535) {
            grab_quotes(&(m.msg),"EMAIL");
            if (ch=='M') {
              nl();
              pl(get_string(15));
              outstr(":");
              input(s,75);
              parse_email_info(s, &un, &sy);
              if (un || sy) {
                email(un, sy, 0, 0);
              }
            } else {
              email(m.fromuser,m.fromsys,0,m.anony);
            }
            grab_quotes(NULL, NULL);
          }
          num_mail1=((long) thisuser.feedbacksent) +
                    ((long) thisuser.emailsent) +
                    ((long) thisuser.emailnet);
          if ((ch=='A') || (ch=='M')) {
            if (num_mail!=num_mail1) {
              if (m.fromsys!=0)
                sprintf(s,"%s: %s",net_name,nam1(&thisuser,usernum,net_sysnum));
              else
                strcpy(s,nam1(&thisuser,usernum,net_sysnum));
              if (m.anony & anony_receiver)
                strcpy(s,get_string(482));
              strcat(s,get_string(713));
              strcat(s,date());
              if (!(m.status & status_source_verified))
                ssm(m.fromuser,m.fromsys,s);
              read_same_email(mloc, mw, curmail, &m, 1, 0);
              ++curmail;
              if (curmail>=mw)
                done=1;
              if (!wfc)
                topscreen();
            } else {
              nl();
              pl(get_string(721));
              nl();
              i2=0;
            }
          } else {
            if (num_mail != num_mail1) {
              if (!(m.status & status_replied)) {
                read_same_email(mloc, mw, curmail, &m, 0, status_replied);
              }
              ++curmail;
              if (curmail>=mw)
                done=1;
              if (!wfc)
                topscreen();
            }
          }
          if (menu_on())
            printmenu(4);
          break;
        case 'U':
        case 'V':
        case 'C':
          if (!okmail)
            break;
          if ((m.fromsys==0) && (cs()) && (m.fromuser!=65535))
            if (ch=='V')
              valuser(m.fromuser);
            else if (ch=='U')
              uedit(m.fromuser,0);
            else
              uedit(m.fromuser,3);
          else if (cs()) {
            nl();
            pl(get_string(722));
            nl();
          }
          i2=0;
          break;
        case 'L':
          if (!so())
            break;
          nl();
          prt(2,get_string(44));
          input(s,50);
          if (s[0]) {
            nl();
            prt(5,get_string(17));
            if (yn()) {
              nl();
              load_workspace(s,0);
            } else {
              nl();
              load_workspace(s,1);
            }
          }
          break;
      }
    } while ((!i2) && (!hangup));
  } while ((!hangup) && (!done));

  bbsfree(mloc);
}
