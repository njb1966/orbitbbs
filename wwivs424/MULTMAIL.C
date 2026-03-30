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



#define ALLOW_FULLSCREEN 1
#define EMAIL_STORAGE 2


void multimail(int *un, int numu)
{
  int i,i1,f,len,cv;
  mailrec m,m1;
  char s[81],t[81];
  userrec ur;
  slrec ss;

  if (freek1(syscfg.msgsdir)<10.0) {
    nl();
    pl(get_string(332));
    nl();
    return;
  }
  nl();
  ss=syscfg.sl[actsl];

  if (ss.ability & ability_email_anony)
    i=anony_enable_anony;
  else
    i=0;
  m.msg.storage_type=EMAIL_STORAGE;
  strcpy(irt,get_string(333));
  irt_name[0]=0;
  unlink("quotes.txt");
  inmsg(&m.msg,t,&i,1,"EMAIL",ALLOW_FULLSCREEN, get_string(333), 0);
  if (m.msg.stored_as==0xffffffff)
    return;
  strcpy(m.title,t);

  pl(get_string(334));
  sysoplog(get_stringx(1,20));

  for (cv=0; cv<numu; cv++) {
    if (un[cv]<0)
      continue;
    read_user(un[cv],&ur);
    if (((ur.sl==255) && (ur.waiting>(syscfg.maxwaiting * 5))) ||
        ((ur.sl!=255) && (ur.waiting>syscfg.maxwaiting)) ||
        (ur.waiting>200)) {
      outstr(nam(&ur,un[cv]));
      outstr(get_string(335));
      un[cv]=-1;
      continue;
    }
    if (ur.inact & inact_deleted) {
      pl(get_string(336));
      un[cv]=-1;
      continue;
    }

    strcpy(s,"   ");
    ++ur.waiting;
    write_user(un[cv],&ur);
    if (un[cv]==1)
      ++fwaiting;
    strcat(s,nam(&ur,un[cv]));
    lock_status();
    if (un[cv]==1)  {
      ++status.fbacktoday;
      ++thisuser.feedbacksent;
      ++thisuser.fsenttoday1;
      ++fsenttoday;
    } else {
      ++status.emailtoday;
      ++thisuser.etoday;
      ++thisuser.emailsent;
    }
    save_status();
    sysoplog(s);
    pl(s);
  }

  m.anony=i;
  m.fromsys=0;
  m.fromuser=usernum;
  m.tosys=0;
  m.touser=0;
  m.status=status_multimail;
  time((long *)&(m.daten));

  f=open_email(1);
  len=(int) filelength(f)/sizeof(mailrec);
  if (len==0)
    i=0;
  else {
    i=len-1;
    sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
    sh_read(f,(void *)&m1,sizeof(mailrec));
    while ((i>0) && (m1.tosys==0) && (m1.touser==0)) {
      --i;
      sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
      i1=sh_read(f,(void *)&m1,sizeof(mailrec));
      if (i1==-1)
        pl(get_string(1193));
    }
    if ((m1.tosys) || (m1.touser))
      ++i;
  }
  sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
  for (cv=0; cv<numu; cv++) {
    if (un[cv]>0) {
      m.touser=un[cv];
      sh_write(f,(void *)&m,sizeof(mailrec));
    }
  }
  sh_close(f);
  if (!wfc)
    topscreen();
}


char *mml_s;
int mml_started;


int oneuser(void)
{
  char s[81],*ss;
  unsigned short un,sn,i;
  userrec u;

  if (mml_s) {
    if (mml_started)
      ss=strtok(NULL,"\r\n");
    else
      ss=strtok(mml_s,"\r\n");
    mml_started=1;
    if (ss==NULL) {
      bbsfree(mml_s);
      mml_s=NULL;
      return(-1);
    }
    strcpy(s,ss);
    for (i=0; s[i]!=0; i++)
      s[i]=upcase(s[i]);
  } else {
    prt(2,">");
    input(s,40);
  }
  un=finduser1(s);
  if (un==65535)
    return(-1);
  if (s[0]==0)
    return(-1);
  if (un<=0) {
    nl();
    pl(get_string(8));
    nl();
    return(0);
  }
  sn=0;
  if (forwardm(&un,&sn)) {
    nl();
    pl(get_string(337));
    nl();
    if (sn) {
      pl(get_string(338));
      pl(get_string(339));
      nl();
      return(0);
    }
  }
  if (un==0) {
    nl();
    pl(get_string(8));
    nl();
    return(0);
  }
  read_user(un,&u);
  if (((u.sl==255) && (u.waiting>(syscfg.maxwaiting * 5))) ||
      ((u.sl!=255) && (u.waiting>syscfg.maxwaiting)) ||
      (u.waiting>200)) {
    nl();
    pl(get_string(340));
    nl();
    return(0);
  }
  if (u.inact & inact_deleted) {
    nl();
    pl(get_string(341));
    nl();
    return(0);
  }
  sprintf(s,"     -> %s",nam(&u,un));
  pl(s);
  return(un);
}


void add_list(int *un, int *numu, int maxu, int allowdup)
{
  int i,i1,done, mml;

  done=0;
  mml=(mml_s!=NULL);
  mml_started=0;
  while ((!done) && (!hangup) && (*numu<maxu)) {
    i=oneuser();
    if (mml && (!mml_s))
      done=1;
    if (i==-1)
      done=1;
    else
      if (i) {
        if (!allowdup)
          for (i1=0; i1<*numu; i1++)
            if (un[i1]==i) {
              nl();
              pl(get_string(342));
              nl();
              i=0;
            }
        if (i)
          un[(*numu)++]=i;
      }
  }
  if (*numu==maxu) {
    nl();
    pl(get_string(343));
    nl();
  }
}


#define MAX_LIST 40


void slash_e(void)
{
  int un[MAX_LIST],numu,done,i,i1,f1;
  char s[81],s1[81],ch,*sss;
  slrec ss;
  userrec u;
  struct ffblk ff;

  mml_s=NULL;
  mml_started=0;
  ss=syscfg.sl[actsl];
  if (freek1(syscfg.msgsdir)<10.0) {
    nl();
    pl(get_string(332));
    nl();
    return;
  }
  if (((fsenttoday>=5) || (thisuser.fsenttoday1>=10) ||
    (thisuser.etoday>=ss.emails)) && (!cs())) {
    pl(get_string(344));
    return;
  }
  if (restrict_email & thisuser.restrict) {
    pl(get_string(345));
    return;
  }
  done=0;
  numu=0;
  do {
    nln(2);
    if (menu_on()) {
      rip_saveall();
      printmenu(12);
    }
    prt(2,get_string(346));
    ch=onek("QAMDEL?");
    if (menu_on())
      rip_restoreall();
    switch(ch) {
      case '?':
        printmenu(12);
        break;
      case 'Q':
        done=1;
        break;
      case 'A':
        nl();
        pl(get_string(347));
        nl();
        helpl=5;
        mml_s=NULL;
        add_list(un,&numu,MAX_LIST,so());
        break;
      case 'M':
        sprintf(s,"%s*.MML",syscfg.datadir);
        f1=findfirst(s,&ff,0);
        if (f1) {
          nl();
          pl(get_string(348));
          nl();
          break;
        }
        nl();
        pl(get_string(349));
        nl();
        while (f1==0) {
          strcpy(s,ff.ff_name);
          sss=strchr(s,'.');
          if (sss)
            *sss=0;
          pl(s);

          f1=findnext(&ff);
        }

        nl();
        prt(2,get_string(297));
        input(s,8);

        sprintf(s1,"%s%s.MML",syscfg.datadir,s);
        i=sh_open1(s1,O_RDONLY | O_BINARY);
        if (i<0) {
          nl();
          pl(get_string(350));
          nl();
        } else {
          i1=filelength(i);
          mml_s=malloca(i1+10L);
          sh_read(i,mml_s,i1);
          mml_s[i1]='\n';
          mml_s[i1+1]=0;
          sh_close(i);
          mml_started=0;
          add_list(un,&numu,MAX_LIST,so());
          if (mml_s) {
            bbsfree(mml_s);
            mml_s=NULL;
          }
        }
        break;
      case 'E':
        if (!numu) {
          nl();
          pl(get_string(351));
          nl();
        } else {
          multimail(un,numu);
          done=1;
        }
        break;
      case 'D':
        if (numu) {
          nl();
          prt(2,get_string(352));
          input(s,2);
          i=atoi(s);
          if ((i>0) && (i<=numu)) {
            --numu;
            for (i1=i-1; i1<numu; i1++)
              un[i1]=un[i1+1];
          }
        }
        break;
      case 'L':
        for (i=0; i<numu; i++) {
          read_user(un[i],&u);
          npr("%d. %s\r\n",i+1,nam(&u,un[i]));
        }
        break;
    }
  } while ((!done) && (!hangup));
}


