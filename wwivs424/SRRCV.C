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


char modemkey(int *tout)
{
  double d1;
  char ch;

  if (comhit()) {
    ch=get1c();
    calc_CRC(ch);
    return(ch);
  }
  if (*tout)
    return(0);
  d1=timer();
  while ((fabs(timer()-d1)<0.5) && (!comhit()) && (!hangup)) {
    checkhangup();
  }
  if (comhit()) {
    ch=get1c();
    calc_CRC(ch);
    return(ch);
  }
  *tout=1;
  return(0);
}



int receive_block(char *b, unsigned char *bln, int ucrc)
{
  int i,abort,err,tout,cc1;
  unsigned char ch,bn,bn1,cs1;

  abort=0;
  ch=gettimeout(5.0,&abort);
  err=0;
  if (abort)
    return(6);
  tout=0;
  if (ch==0x81) {
    bn=modemkey(&tout);
    bn1=modemkey(&tout);
    if ((bn ^ bn1)==0xff) {
      b[0]=bn;
      *bln=bn;
      return(8);
    } else
      return(3);
  } else
    if (ch==1) {
      bn=modemkey(&tout);
      bn1=modemkey(&tout);
      if ((bn ^ bn1)!=0xff)
        err=3;
      *bln=bn;
      crc=0;
      checksum=0;
      for (i=0; (i<128) && (!hangup); i++)
        b[i]=modemkey(&tout);
      if ((!ucrc) && (!hangup)) {
        cs1=checksum;
        bn1=modemkey(&tout);
        if (bn1!=cs1) {
          err=2;
        }
      } else if (!hangup) {
        cc1=crc;
        bn=modemkey(&tout);
        bn1=modemkey(&tout);
        if ((bn!=(unsigned char)(cc1 >> 8)) || (bn1!=(unsigned char)(cc1 & 0x00ff)))
          err=2;
      }
      if (tout)
        return(7);
      return(err);
    } else
      if (ch==2) {
        bn=modemkey(&tout);
        bn1=modemkey(&tout);
        crc=0;
        checksum=0;
        if ((bn ^ bn1)!=0xff)
          err=3;
        *bln=bn;
        for (i=0; (i<1024) && (!hangup); i++)
          b[i]=modemkey(&tout);
        if ((!ucrc) && (!hangup)){
          cs1=checksum;
          bn1=modemkey(&tout);
          if (bn1!=cs1)
            err=2;
        } else if (!hangup) {
          cc1=crc;
          bn=modemkey(&tout);
          bn1=modemkey(&tout);
          if ((bn!=(unsigned char)(cc1 >> 8)) || (bn1!=(unsigned char)(cc1 & 0x00ff)))
            err=2;
        }
        if (tout)
          return(7);
        if (err==0)
          return(1);
        else
          return(err);
      } else
        if (ch==24) {
          return(4);
        } else
          if (ch==4) {
            return(5);
          } else
            if (ch==0)
              return(7);
             else
              return(9);
}


void xymodem_receive(char *fn, char *ft, int *received, int ucrc)
{
  char b[1025],x[81],ch;
  unsigned char bln;
  unsigned int bn;
  int done,ok,lastcan,lasteot,terr,cerr;
  long pos,reallen,filedatetime,lx;
  int f,i,i1,i2,i3,ox,oy;
  double tpb;
  struct ftime ff;
  struct date d;
  struct time t;
  double d1;

  unlink(fn);
  f=sh_open(fn,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
  ok=1;
  lastcan=0;
  lasteot=0;
  terr=0;
  cerr=0;
  if (f<0) {
    nln(2);
    pl(get_string(908));
    nl();
    *received=0;
    return;
  }
  pos=0L;
  reallen=0L;
  filedatetime=0L;
  bn=1;
  done=0;
  tpb=(12.656) / ((double) (modem_speed));
  nl();
  pl(get_string(887));
  ox=WhereX();
  oy=WhereY();
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
  i=0;
  do {
    if (i++>9) {
      *received=0;
      sh_close(f);
      unlink(fn);
      return;
    }
    if (ucrc)
      outcomch('C');
    else
      outcomch(21);

    d1=timer();
    while ((fabs(timer()-d1)<10.0) && (!comhit()) && (!hangup)) {
      checkhangup();
      if (kbhitb()) {
        ch=getchd();
        if (ch==0)
          getchd();
        else
          if (ch==27) {
            done=1;
            ok=0;
          }
      }
    }
  } while(!comhit() && !hangup);

  do {
    bln=255;
    itoa(cerr,x,10);
    strcat(x,"  ");
    movecsr(69,4);
    outs(x);
    itoa(terr,x,10);
    movecsr(69,5);
    outs(x);
    sprintf(x,"%ld - %ldk",
      pos/128+1,
      pos/1024+1);
    movecsr(65,3);
    outs(x);
    if (reallen) {
      movecsr(65,1);
      outs(ctim(((double)(reallen-pos))*tpb));
    }

    i=receive_block(b,&bln,ucrc);
    if ((i==0) || (i==1)) {
      if ((bln==0) && (pos==0L)) {
        i1=strlen(b)+1;
        i3=i1;
        while ((b[i3]>='0') && (b[i3]<='9') && ((i3-i1)<15))
          x[i3-i1]=b[i3++];
        x[i3-i1]=0;
        reallen=atol(x);
        sprintf(x,"%ld - %ldk",
          (reallen+127)/128,
          bytes_to_k(reallen));
        movecsr(65,2);
        outs(x);
        while ((b[i1]!=32) && (i1<64))
          ++i1;
        if (b[i1]==32) {
          ++i1;
          while ((b[i1]>='0') && (b[i1]<='8')) {
            filedatetime=(filedatetime*8) + ((long) (b[i1]-'0'));
            ++i1;
          }
          i1+=timezone+5*60*60;
        }
        outcomch(6);
      } else
        if ((bn & 0x00ff)==(unsigned int)bln) {
          sh_lseek(f,pos, SEEK_SET);
          lx=reallen-pos;
          if (i==0)
            i2=128;
          else
            i2=1024;
          if ((((long) i2)>lx) && (reallen))
            i2=(int) lx;
          sh_write(f,(void *)b,i2);
          pos += (long)i2;
          ++bn;
          outcomch(6);
        } else
          if (((bn-1) & 0x00ff)==(unsigned int)bln) {
            outcomch(6);
          } else {
            outcomch(24);
            ok=0;
            done=1;
          }
      cerr=0;
    } else
      if ((i==2) || (i==7) || (i==3)) {
        if ((pos==0L) && (reallen==0L) && (ucrc))
          outcomch('C');
        else
          outcomch(21);
        ++cerr;
        ++terr;
        if (cerr>9) {
          outcomch(24);
          ok=0;
          done=1;
        }
      } else
        if (i==6) {
          ok=0;
          done=1;
          outcomch(24);
        } else
          if (i==4) {
            if (lastcan) {
              ok=0;
              done=1;
              outcomch(6);
            } else {
              lastcan=1;
              outcomch(21);
            }
          } else
            if (i==5) {
              lasteot=1;
              if (lasteot) {
                done=1;
                outcomch(6);
              } else {
                lasteot=1;
                outcomch(21);
              }
            } else
              if (i==8) {
                *ft=bln;
                outcomch(6);
                cerr=0;
              } else
                if (i==9)
                  dump();
    if (i!=4)
      lastcan=0;
    if (i!=5)
      lasteot=0;
  } while ((!hangup) && (!done));
  movecsr(ox,oy);
  if (ok) {
    if (filedatetime) {
      unixtodos(filedatetime,&d,&t);
      if (d.da_year>=1980) {
        ff.ft_min=t.ti_min;
        ff.ft_hour=t.ti_hour;
        ff.ft_tsec=t.ti_sec/2;
        ff.ft_year=d.da_year-1980;
        ff.ft_day=d.da_day;
        ff.ft_month=d.da_mon;
        setftime(f,&ff);
      }
    }
    sh_close(f);
    *received=1;
  } else {
    sh_close(f);
    unlink(fn);
    *received=0;
  }
}


