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

#include <math.h>



void send_block(char *b, int type, int ucrc, char bn)
{
  int maxb,i;
  char ch;

  checkhangup();
  switch(type) {
    case 5:
      maxb=128;
      outcomch(1);
      break;
    case 4:
      outcomch(0x81);
      outcomch(bn);
      outcomch(bn ^ 0xff);
      break;
    case 3:
      outcomch(24);
      break;
    case 2:
      outcomch(4);
      break;
    case 1:
      maxb=1024;
      outcomch(2);
      break;
    case 0:
      maxb=128;
      outcomch(1);
  }
  if ((type>1) && (type<5))
    return;
  outcomch(bn);
  outcomch(bn ^ 0xff);
  crc=0;
  checksum=0;
  for (i=0; i<maxb; i++) {
    ch=b[i];
    outcomch(ch);
    calc_CRC(ch);
  }
  if (ucrc) {
     outcomch(crc >> 8);
     outcomch(crc & 0x00ff);
  } else {
    outcomch(checksum);
  }
  dump();
}


char send_b(int f, long pos, int type, char bn, int *ucrc, char *fn, int *terr, int *abort)
{
  char b[1025],ch,x[20],x1[20];
  int i,i1,nb,done,nerr;
  struct ftime ff;
  struct date d;
  struct time t;

  nb=0;
  if (type==0)
    nb=128;
  if (type==1)
    nb=1024;
  if (nb) {
    sh_lseek(f,pos,SEEK_SET);
    i=sh_read(f,(void *)b,nb);
    for (i1=i; i1<nb; i1++)
      b[i1]=0;
  } else
    if (type==5) {
      for (i1=0; i1<128; i1++)
        b[i1]=0;
      nb=128;
      strcpy(b,stripfn(fn));
      ltoa(pos,x,10);
      strcat(x," ");
      getftime(f,&ff);
      t.ti_min=ff.ft_min;
      t.ti_hour=ff.ft_hour;
      t.ti_hund=0;
      t.ti_sec=ff.ft_tsec*2;
      d.da_year=1980+ff.ft_year;
      d.da_day=ff.ft_day;
      d.da_mon=ff.ft_month;
      ltoa(dostounix(&d,&t)-timezone,x1,8);
      strcat(x,x1);
      strcpy(&(b[strlen(b)+1]),x);
      b[127]=(unsigned char) (((int) (pos+127)/128) >> 8);
      b[126]=(unsigned char) (((int) (pos+127)/128) & 0x00ff);
    }
  done=0;
  nerr=0;
  do {
    send_block(b,type,*ucrc,bn);
    ch=gettimeout(5.0,abort);
    if ((ch=='C') && (pos==0))
      *ucrc=1;
    if ((ch==6) || (ch==24))
      done=1;
    else {
      ++nerr;
      ++(*terr);
      if (nerr>=9)
        done=1;
      itoa(nerr,x,10);
      movecsr(69,4);
      outs(x);
      itoa(*terr,x,10);
      movecsr(69,5);
      outs(x);
    }
  } while ((!done) && (!hangup) && (!*abort));
  if (ch==6)
    return(6);
  if (ch==24)
    return(24);
  return(21);
}


int okstart(int *ucrc, int *abort)
{
  char ch;
  double d;
  int ok,done;

  d=timer();
  ok=0;
  done=0;
  while ((fabs(timer()-d)<90.0) && (!done) && (!hangup) && (!*abort)) {
    ch=gettimeout(91.0-d,abort);
    if (ch=='C') {
      *ucrc=1;
      ok=1;
      done=1;
    }
    if (ch==21) {
      *ucrc=0;
      ok=1;
      done=1;
    }
    if (ch==24) {
      ok=0;
      done=1;
    }
  }
  return(ok);
}



void xymodem_send(char *fn, int *sent, double *percent, char ft, int ucrc, int ym, int ymb)
{
  char bn,ch,s[81];
  int f,i,abort,terr,xx1,yy1;
  long cp,len;
  double tpb;

  cp=0L;
  bn=1;
  abort=0;
  terr=0;
  f=sh_open1(fn,O_RDONLY | O_BINARY);
  if (f<0) {
    if (!ymb) {
      nl();
      pl(get_string(89));
      nl();
    }
    *sent=0;
    *percent=0.0;
    return;
  }
  len=filelength(f);
  if (!len)
    len=1;
  tpb=(12.656) / ((double) (modem_speed));

  if (!ymb) {
    nl();
    pl(get_string(886));
  }
  xx1=WhereX();
  yy1=WhereY();
  movecsr(52,0);
  outs(get_stringx(1,67));
  movecsr(52,1);
  outs(get_stringx(1,68));
  movecsr(52,2);
  outs(get_stringx(1,69));
  movecsr(52,3);
  outs(get_stringx(1,70));
  movecsr(52,4);
  outs(get_stringx(1,71));
  movecsr(52,5);
  outs(get_stringx(1,72));
  movecsr(52,6);
  outs(get_stringx(1,73));
  movecsr(65,0);
  outs(stripfn(fn));
  sprintf(s,"%ld - %ldk",
    (len+127)/128,
    bytes_to_k(len));
  movecsr(65,2);
  outs(s);

  if (!okstart(&ucrc,&abort))
    abort=1;
  if ((ft) && (!abort) && (!hangup)) {
    ch=send_b(f, len, 4, ft, &ucrc,fn,&terr,&abort);
    if (ch==24)
      abort=1;
    if (ch==21) {
      send_b(f,0L,3,0,&ucrc,fn,&terr,&abort);
      abort=1;
    }
  }
  if ((ym) && (!abort) && (!hangup)) {
    ch=send_b(f, len, 5, 0, &ucrc,fn,&terr,&abort);
    if (ch==24)
      abort=1;
    if (ch==21) {
      send_b(f,0L,3,0,&ucrc,fn,&terr,&abort);
      abort=1;
    }
  }
  while ((!hangup) && (!abort) && (cp<len)) {
    if (ym)
      i=1;
    else
      i=0;
    if ((len-cp)<128L)
      i=0;
    sprintf(s,"%ld - %ldk",
      cp/128+1,
      cp/1024+1);
    movecsr(65,3);
    outs(s);
    movecsr(65,1);
    outs(ctim(((double)(len-cp))*tpb));
    movecsr(69,4);
    outs("0");

    ch=send_b(f,cp,i,bn,&ucrc,fn,&terr,&abort);
    if (ch==24)
      abort=1;
    else
      if (ch==21) {
        wait1(18);
        dump();
        send_b(f,0L,3,0,&ucrc,fn,&terr,&abort);
        abort=1;
      } else {
        ++bn;
        if (i)
          cp+=1024;
        else
          cp+=128;
      }
  }
  if ((!hangup) && (!abort))
    send_b(f,0L,2,0,&ucrc,fn,&terr,&abort);
  if (!abort) {
    *sent=1;
    *percent=1.0;
  } else {
    *sent=0;
    if (i)
      cp+=1024;
    else
      cp+=128;
    if (cp>=len)
      *percent=1.0;
    else {
      if (i)
        cp-=1024;
      else
        cp-=128;
      *percent=((double)(cp))/((double)(len));
    }
  }
  sh_close(f);
  movecsr(xx1,yy1);
  if ((*sent) && (!ymb)) {
    pl(get_string(909));
    nl();
  }
}


