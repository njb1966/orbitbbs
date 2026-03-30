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



void print_quests(void)
{
  char s[100];
  int i,abort,f;
  votingrec v;

  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  abort=0;
  for (i=1; (i<=20) && (abort==0); i++) {
    sh_lseek(f,((long) (i-1)) * sizeof(votingrec), SEEK_SET);
    sh_read(f,(void *)&v,sizeof(votingrec));
    sprintf(s,"%2d: %s",i,
      v.numanswers?v.question:get_string(99));
    pla(s,&abort);
  }
  f=sh_close(f);
  nl();
  if (abort)
    nl();
}


void set_question(int ii)
{
  votingrec v;
  int i,i1,f;
  char s[81];
  userrec u;
  voting_response vr;

  pl(get_string(100));
  outstr(": ");
  inputl(s,75);
  strcpy(v.question,s);
  v.numanswers=0;
  vr.numresponses=0;
  vr.response[0]='X';
  vr.response[1]=0;
  for (i=0; i<20; i++)
    v.responses[i]=vr;
  while ((v.numanswers<19) && (s[0])) {
    npr("%d: ",v.numanswers+1);
    inputl(s,63);
    strcpy(vr.response,s);
    vr.numresponses=0;
    v.responses[v.numanswers]=vr;
    if (s[0])
      ++v.numanswers;
  }

  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);

  sh_lseek(f,((long) (ii)) * sizeof(votingrec), SEEK_SET);
  sh_write(f,(void *)(&v),sizeof(votingrec));
  f=sh_close(f);

  if (v.numanswers)
    questused[ii]=1;
  else
    questused[ii]=0;

  read_user(1,&u);
  i1=number_userrecs();
  for (i=1; i<=i1; i++) {
    read_user(i,&u);
    u.votes[ii]=0;
    write_user(i,&u);
  }
}


void ivotes(void)
{
  int i,f,n,done;
  char s[81];
  votingrec v;

  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  n=(int) (filelength(f) / sizeof(votingrec)) -1;
  if (n<20) {
    v.question[0]=0;
    v.numanswers=0;
    for (i=n; i<20; i++)
      sh_write(f,(void *)&v,sizeof(votingrec));
  }
  f=sh_close(f);
  done=0;
  do {
    print_quests();
    nl();
    prt(2,get_string(101));
    input(s,2);
    if (strcmp(s,"Q")==0)
      done=1;
    i=atoi(s);
    if ((i>0) && (i<21)) {
      set_question(i-1);
    }
  } while ((!done) && (!hangup));
}


void voteprint(void)
{
  int f,f1,i,i1,i2,nu;
  char s[81], fn[81];
  votingrec v;
  char *x;
  userrec u;

  nu=number_userrecs();
  if ((x=malloca(20*(2+nu)))==NULL)
    return;
  for (i=0; i<=nu; i++) {
    read_user(i,&u);
    for (i1=0; i1<20; i1++)
      x[i1+i*20]=u.votes[i1];
  }
  sprintf(s,"%sVOTING.TXT",syscfg.gfilesdir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  sprintf(s,get_stringx(1,16),date());
  sh_write(f,(void *)s,strlen(s));

  sprintf(fn,"%sVOTING.DAT",syscfg.datadir);

  read_status();

  for (i=0; i<20; i++) {
    f1=sh_open1(fn,O_RDONLY | O_BINARY);
    sh_lseek(f1,((long)i)*sizeof(votingrec),SEEK_SET);
    sh_read(f1,(void *)&v,sizeof(votingrec));
    f1=sh_close(f1);
    if (v.numanswers) {
      pl(v.question);
      sprintf(s,"\r\n%s\r\n",v.question);
      sh_write(f,s,strlen(s));
      for (i1=0; i1<v.numanswers; i1++) {
        sprintf(s,"     %s\r\n",v.responses[i1].response);
        sh_write(f,s,strlen(s));
        for (i2=0; i2<status.users; i2++)
          if (x[i+20*smallist[i2].number]==i1+1) {
            sprintf(s,"          %s #%d\r\n",smallist[i2].name,smallist[i2].number);
            sh_write(f,s,strlen(s));
          }
      }
    }
  }
  sh_close(f);
  bbsfree(x);
}


