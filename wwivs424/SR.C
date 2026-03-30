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

#include <time.h>
#include <math.h>



void calc_CRC(unsigned char b)
{
  int i;

  checksum += b;

  crc ^= (((unsigned short) (b)) << 8);
  for (i=0; i<8; i++)
    if (crc & 0x8000) {
      crc=(crc << 1);
      crc ^= 0x1021;
    } else
      crc=(crc << 1);
}


char gettimeout(double d, int *abort)
{
  double d1;
  char ch;

  if (comhit())
    return(get1c());
  d1=timer();
  while ((fabs(timer()-d1)<d) && (!comhit()) && (!hangup) && (!*abort)) {
    if (kbhitb()) {
      ch=getchd();
      if (ch==0)
        getchd();
      else
        if (ch==27)
          *abort=1;
    }
    checkhangup();
  }
  if (comhit())
    return(get1c());
  else
    return(0);
}


int extern_prot(int pn, char *fn1, int sending)
{
  char s[255],s1[81],s2[81],fn[81],sx1[21],sx2[21],sx3[21];
  int i;

  if (sending) {
    nl();
    pl(get_string(886));
    if (pn<0)
      strcpy(s1,over_intern[(-pn)-1].sendfn);
    else
      strcpy(s1,(externs[pn].sendfn));
  } else {
    nl();
    pl(get_string(887));
    if (pn<0)
      strcpy(s1,over_intern[(-pn)-1].receivefn);
    else
      strcpy(s1,(externs[pn].receivefn));
  }
  strcpy(fn,fn1);
  stripfn1(fn);
  ultoa(com_speed,sx1,10);
  if ((com_speed==1) || (com_speed==49664))
    strcpy(sx1,"115200");
  ultoa(modem_speed,sx3,10);
  sx2[0]='0'+syscfgovr.primaryport;
  sx2[1]=0;
  stuff_in(s,s1,sx1,sx2,fn,sx3,"");
  if (s[0]) {
    set_protect(0);
    sprintf(s2,get_stringx(1,64),nam(&thisuser,usernum),modem_speed);
    outs(s2);
    outs("\r\n\r\n");
    outs(s);
    outs("\r\n");
    if (incom) {
      create_chain_file();
      i=extern_prog(s, sysinfo.spawn_opts[6]);
      topscreen();
      return(i);
    } else {
      topscreen();
      return(-5);
    }
  }
  return(-5);
}


int ok_prot(int pn, xfertype xt)
{
  int ok=0;
  int i;

  if (xt==xf_none)
    return(0);

  if ((pn>0) && (pn<(numextrn+6))) {

    switch(pn) {
      case 1:
        if ((xt==xf_down) || (xt==xf_down_temp))
          ok=1;
        break;
      case 2:
      case 3:
      case 4:
        if ((xt!=xf_up_batch) && (xt!=xf_down_batch) && (xt!=xf_bi))
          ok=1;
        if ((pn==4) && (xt==xf_down_batch))
          ok=1;
        break;
      case 5:
        if (xt==xf_up) {
          for (i=0; i<numextrn; i++)
            if (externs[i].receivebatchfn[0] || externs[i].bibatchfn)
              ok=1;
        } else if (xt==xf_down) {
          for (i=0; i<numextrn; i++)
            if (externs[i].sendbatchfn[0] || externs[i].bibatchfn)
              ok=1;
        }
        if ((xt==xf_up) || (xt==xf_down))
          ok=1;
        break;
      default:
        switch(xt) {
          case xf_up:
          case xf_up_temp:
            if (externs[pn-6].receivefn[0])
              ok=1;
            break;
          case xf_down:
          case xf_down_temp:
            if (externs[pn-6].sendfn[0])
              ok=1;
            break;
          case xf_up_batch:
            if (externs[pn-6].receivebatchfn[0])
              ok=1;
            break;
          case xf_down_batch:
            if (externs[pn-6].sendbatchfn[0])
              ok=1;
            break;
          case xf_bi:
            if (externs[pn-6].bibatchfn[0])
              ok=1;
            break;
        }
        if (externs[pn-6].othr & othr_error_correct)
          if (!(modem_flag & flag_ec))
            ok=0;
        break;
    }
  }

  return(ok);
}


char *prot_name(int pn)
{
  char *ss=get_string(888);

  switch(pn) {
    case 1:
      ss=get_string(889);
      break;
    case 2:
      ss=get_string(890);
      break;
    case 3:
      ss=get_string(891);
      break;
    case 4:
      ss=get_string(892);
      break;
    case 5:
      ss=get_string(893);
      break;
    default:
      if ((pn>5) && (pn<(numextrn+6)))
        ss=externs[pn-6].description;
      break;
  }

  return(ss);
}

#define BASE_CHAR '!'

int get_protocol(xfertype xt)
{
  char s[81],s1[81],oks[81],ch,oks1[81],*ss,ch1,fl[80];
  int i,i1,prot,maxprot,done,only,oks1p;
  char ripstr[80];

  if (ok_prot(thisuser.defprot, xt))
    prot=thisuser.defprot;
  else
    prot=0;

  oks1p=0;
  oks1[0]=0;
  strcpy(oks,"Q?0");
  i1=strlen(oks);
  only=0;
  maxprot=5+numextrn;
  for (i=1; i<=maxprot; i++) {
    fl[i]=0;
    if (ok_prot(i,xt)) {
      if (i<10)
        oks[i1++]='0'+i;
      else
        oks[i1++]=BASE_CHAR+i-10;
      if (only==0)
        only=i;
      else
        only=-1;
      if (i>=3) {
        ch1=upcase(*prot_name(i));
        if (strchr(oks1,ch1)==0) {
          oks1[oks1p++]=ch1;
          oks1[oks1p]=0;
          fl[i]=1;
        }
      }
    }
  }
  oks[i1]=0;
  strcat(oks,oks1);
  if (only>0)
    prot=only;

  if ((only==0) && (xt != xf_none)) {
    nl();
    pl(get_string(894));
    nl();
    return(-1);
  }

  done=0;
  if (prot) {
    ss=prot_name(prot);
    sprintf(s,"%s%s) : ", get_string(895),ss);
    strcpy(s1,oks);
    strcat(s1,"\r");
  } else {
    strcpy(s,get_string(896));
    strcpy(s1,oks);
  }
  do {
    if (menu_on()) {
      comstr("|10000((*Protocol::\\");
      for (i=1; i<=maxprot; i++) {
        if (ok_prot(i,xt)) {
          ch1=upcase(*prot_name(i));
          if (fl[i]==0)
            sprintf(ripstr,"%c@%s,\\",
                    (i<10)?(i+'0'):(i+BASE_CHAR-10),prot_name(i));
          else
            sprintf(ripstr,"%c@~%c~%s,\\",
                    (i<10)?(i+'0'):(i+BASE_CHAR-10),*prot_name(i), prot_name(i)+1);
          comstr(ripstr);
        }
      }
      comstr("Q@~C~ancel))\r");
    } else {
      nl();
      prt(2,s);
    }
    ch=onek(s1);
    if (ch=='?') {
      nl();
      pl(get_string(897));
      pl(get_string(898));
      for (i=1; i<=maxprot; i++) {
        if (ok_prot(i,xt)) {
          ch1=upcase(*prot_name(i));
          if (fl[i]==0)
            npr("%c. %s\r\n",(i<10)?(i+'0'):(i+BASE_CHAR-10),prot_name(i));
          else
            npr("%c. %c)%s\r\n",
                (i<10)?(i+'0'):(i+BASE_CHAR-10),*prot_name(i), prot_name(i)+1);
        }
      }
      nl();
    } else
      done=1;
  } while ((!done) && (!hangup));
  if (ch==13)
    return(prot);
  if ((ch>='0') && (ch<='9'))
    return(ch-'0');
  else {
    if (ch=='Q')
      return(-1);
    else {
      i1=ch-BASE_CHAR+10;
      if (i1<numextrn+6)
        return(ch-BASE_CHAR+10);
      for (i=3; i<numextrn+6; i++) {
        if (upcase(*prot_name(i))==ch)
          return(i);
      }
    }
  }
  return(-1);
}


void ascii_send(char *fn, int *sent, double *percent)
{
  char b[2048];
  int i,i1,abort,i2,next;
  long pos,max;

  i=sh_open1(fn,O_RDONLY | O_BINARY);
  if (i>0) {
    max=filelength(i);
    if (!max)
      max=1;
    i1=sh_read(i,(void *)b,1024);
    pos=0L;
    abort=0;
    while ((i1) && (!hangup) && (!abort)) {
      i2=0;
      while ((!hangup) && (!abort) && (i2<i1)) {
        checkhangup();
        outchr(b[i2++]);
        checka(&abort,&next);
      }
      pos += (long) i2;
      checka(&abort,&next);
      i1=sh_read(i,(void *)b,1024);
    }
    sh_close(i);
    if (!abort)
      *sent=1;
    else {
      *sent=0;
      thisuser.dk += bytes_to_k(pos);
    }
    *percent=((double) pos)/((double)max);
  } else {
    nl();
    pl(get_string(89));
    nl();
    *sent=0;
    *percent=0.0;
  }
}


void maybe_internal(char *fn, int *xferred, double *percent, char ft, char *ftp, int send, int prot)
{
  if (over_intern && (over_intern[prot-2].othr & othr_override_internal) &&
      ((send && over_intern[prot-2].sendfn[0]) ||
       (!send && over_intern[prot-2].receivefn[0]))) {
    if (extern_prot(-(prot-1), fn, send)==over_intern[prot-2].ok1)
      *xferred=1;
  } else {
    if (incom) {
      if (send) {
        switch(prot) {
          case 2:
            xymodem_send(fn, xferred, percent, ft, 0, 0, 0);
            break;
          case 3:
            xymodem_send(fn, xferred, percent, ft, 1, 0, 0);
            break;
          case 4:
            xymodem_send(fn, xferred, percent, ft, 1, 1, 0);
            break;
        }
      } else {
        switch(prot) {
          case 2:
            xymodem_receive(fn, ftp, xferred, 0);
            break;
          case 3:
          case 4:
            xymodem_receive(fn, ftp, xferred, 1);
            break;
        }
      }
    } else {
      outstr(get_string(899));
      pl(prot_name(prot));
    }
  }
}


void send_file(char *fn, int *sent, int *abort, char ft, char *sfn, int dn, long fs)
{
  int i,i1,ok;
  double percent,t;
  char s[81];

  *sent=0;
  *abort=0;
  if (fs<0) {
    i=get_protocol(xf_none);
  } else {
    if (dn==-1)
      i=get_protocol(xf_down_temp);
    else
      i=get_protocol(xf_down);
  }
  ok=0;
  percent=0.0;
  if (check_batch_queue(sfn)) {
    *sent=0;
    if (i>0) {
      nl();
      pl(get_string(762));
      nl();
    } else if (i==-1)
      *abort=1;
  } else {
    switch(i) {
      case -1:
        *abort=1;
        ok=1;
        break;
      case 0:
        ok=1;
        break;
      case 1:
        ascii_send(fn,sent,&percent);
        break;
      case 2:
      case 3:
      case 4:
        maybe_internal(fn, sent, &percent, ft, NULL, 1, i);
        break;
      case 5:
        ok=1;
        if (numbatch>=sysinfo.max_batch) {
          nl();
          pl(get_string(900));
          nl();
          *sent=0;
          *abort=0;
        } else {
          if (modem_speed)
            t=(12.656) / ((double) (modem_speed)) * ((double)(fs));
          else
            t=0.0;
          if (nsl()<=(batchtime + t)) {
            nl();
            pl(get_string(901));
            nl();
            *sent=0;
            *abort=0;
          } else {
            if (dn==-1) {
              nl();
              pl(get_string(902));
              nl();
              *sent=0;
              *abort=0;
            } else {
              batchtime += t;
              strcpy(batch[numbatch].filename,sfn);
              batch[numbatch].dir=dn;
              batch[numbatch].time=t;
              batch[numbatch].sending=1;
              batch[numbatch].len=fs;

              numbatch++;
              ++numbatchdl;
              nl();
              pl(get_string(903));
              sprintf(s,"%s - %d  %s - %s",get_string(904),
                      numbatch, get_string(905),ctim(batchtime));
              nl();
              pl(s);
              nl();
              *sent=0;
              *abort=0;
            }
          }
        }
        break;
      default:
        i1=extern_prot(i-6,fn,1);
        *abort=0;
        if (i1==externs[i-6].ok1)
          *sent=1;
        break;
    }
  }
  if ((*sent==0) && (ok==0))
    if (percent==1.0) {
      *sent=1;
      add_ass(10,get_stringx(1,65));
    } else {
      sprintf(s,get_stringx(1,66),stripfn(fn),percent*100.0);
      sysoplog(s);
    }
}


void receive_file(char *fn, int *received, char *ft, char *sfn, int dn)
{
  int i;

  if (dn==-1)
    i=get_protocol(xf_up_temp);
  else
    i=get_protocol(xf_up);

  switch(i) {
    case -1:
      *received=0;
      break;
    case 0:
      *received=0;
      break;
    case 2:
    case 3:
    case 4:
      maybe_internal(fn, received, NULL, 0, ft, 0, i);
      break;
    case 5:
      if (dn!=-1) {
        if (numbatch>=sysinfo.max_batch) {
          nl();
          pl(get_string(900));
          nl();
          *received=0;
        } else {
          *received=2;
          strcpy(batch[numbatch].filename,sfn);
          batch[numbatch].dir=dn;
          batch[numbatch].time=0;
          batch[numbatch].sending=0;
          batch[numbatch].len=0;

          numbatch++;
          nl();
          pl(get_string(903));
          nl();
          outstr(get_string(906));
          pln(numbatch-numbatchdl);
          nl();
        }
      } else {
        nl();
        pl(get_string(907));
        nl();
      }
      break;
    default:
      if ((i>5) && (incom)) {
        extern_prot(i-6,fn,0);
        *received=exist(fn);
      }
      break;
  }
}



char end_batch1(void)
{
  char b[128],ch;
  int i,i1,done,nerr;

  for (i1=0; i1<128; i1++)
    b[i1]=0;
  done=0;
  nerr=0;
  i=0;
  do {
    send_block(b,5,1,0);
    ch=gettimeout(5.0,&i);
    if ((ch==6) || (ch==24))
      done=1;
    else {
      ++nerr;
      if (nerr>=9)
        done=1;
    }
  } while ((!done) && (!hangup) && (!i));
  if (ch==6)
    return(6);
  if (ch==24)
    return(24);
  return(21);
}


void endbatch(void)
{
  char ch;
  int abort,ucrc,terr,xx1,yy1;

  abort=0;
  terr=0;
  xx1=WhereX();
  yy1=WhereY();
  if (!okstart(&ucrc,&abort))
    abort=1;
  if ((!abort) && (!hangup)) {
    ch=end_batch1();
    if (ch==24)
      abort=1;
    if (ch==21) {
      send_b(0,0L,3,0,&ucrc,"",&terr,&abort);
      abort=1;
    }
/*
    if ((!hangup) && (!abort))
      send_b(0,0L,2,0,&ucrc,"",&terr,&abort);
*/
  }
  movecsr(xx1,yy1);
}


