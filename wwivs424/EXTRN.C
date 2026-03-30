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
#include <process.h>
#include <math.h>


static union REGS ca_r;
static int ca_pause,ca_ctrl_c;
static long ca_d1;
static union REGS ni_r;
static struct SREGS ni_s;
static unsigned ni_n;
static char ni_ch,ni_ch1;
static unsigned char far *ni_st;

#define ST_SIZE 500
static unsigned short ni_stack[ST_SIZE];

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

static unsigned char getkeyext(void)
{
  unsigned char ch;
  static int holding=0;
  static char held=0;

  if (holding) {
    holding=0;
    return(held);
  }
  ch=getkey();
  if (charbufferpointer==0) {
    if (ch==16) {
      ch=getkey();
      if ((ch==1) && (charbufferpointer==0)) {
        strcpy(charbuffer,&(thisuser.macros[2][0]));
        ch=charbuffer[0];
        if (ch) {
          charbufferpointer=1;
          return(ch);
        } else
          return(getkeyext());
      } else
        if ((ch==4) && (charbufferpointer==0)) {
          strcpy(charbuffer,&(thisuser.macros[0][0]));
          ch=charbuffer[0];
          if (ch) {
            charbufferpointer=1;
            return(ch);
          } else
            return(getkeyext());
        } else
          if ((ch==6) && (charbufferpointer==0)) {
            strcpy(charbuffer,&(thisuser.macros[1][0]));
            ch=charbuffer[0];
            if (ch) {
              charbufferpointer=1;
              return(ch);
            } else
              return(getkeyext());
          } else {
            holding=1;
            held=ch;
            return(16);
          }
    }
  }
  return(ch);
}

/****************************************************************************/

static void checka1(void)
{
  char ch;
  long d1;

  while ((!empty()) && (!(abortext)) && (!hangup)) {
    ch=inkey();
    switch(ch) {
      case 3:
      case 32:
      case 24:
        if (eflags & EFLAG_ABORT)
          abortext=1;
        break;
      case 'P':
      case 'p':
      case 19:
        d1=timer1();
        while ((inkey()==0) && (labs(timer1()-d1)<3276L) && (!hangup))
          checkhangup();
        lines_listed=0;
        break;
    }
  }
}

/****************************************************************************/

static void checka2(void)
{
  ca_pause=0;
  ca_ctrl_c=0;
  ca_r.h.ah=1;

  int86(0x16,&ca_r,&ca_r);
  if ((ca_r.x.flags & 64)==0) {
    if (ca_r.x.ax==11779)
      ca_ctrl_c=1;
    if (ca_r.x.ax==7955)
      ca_pause=1;
  }
  if (head!=tail) {
    if (buffer[tail]==3)
      ca_ctrl_c=1;
    if (buffer[tail]==19)
      ca_pause=1;
  }
  if (ca_pause) {
    while (inkey()!=0)
      ;
    ca_d1=timer1();
    while ((inkey()==0) && (labs(timer1()-ca_d1)<3276L) && (!hangup))
      checkhangup();
    lines_listed=0;
  }
  if ((ca_ctrl_c) && (eflags & EFLAG_ABORT)) {
    while (inkey()!=0)
      ;
    pl("^C");
    ca_r.x.ax=0x4c00;
    int86(save_dos,&ca_r,&ca_r);
  }
}

/****************************************************************************/

static void outdosstr(char *s)
/* This function outputs a string of characters to the screen (and remotely
 * if applicable).  The com port is also checked first to see if a remote
 * user has hung up
 */
{
  int i;

  checkhangup();
  if (hangup==0) {
    i=0;
    while ((s[i] !='$') && (i<1024)) {
      checka2();
      outchr(s[i++]);
    }
  }
}




#pragma warn -par

void far interrupt newintr1(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{

  unsigned short ni_SS, ni_SP;
#define NEW_STK() { _BX=FP_OFF(&ni_stack[ST_SIZE-2]); _SS=_DS; _SP=_BX; }
#define OLD_STK() { _AX=ni_SS; _BX=ni_SP; _SS=_AX; _SP=_BX; }



  ni_r.x.ax=ax;
  ni_r.x.bx=bx;
  ni_r.x.cx=cx;
  ni_r.x.dx=dx;
  ni_r.x.si=si;
  ni_r.x.di=di;
  ni_r.x.flags=flags;
  ni_s.ds=ds;
  ni_s.es=es;

  ni_SS=_SS;
  ni_SP=_SP;

  ni_ch=ni_r.h.ah;
  ni_ch1=0;

  if (ni_ch==0x0c) {
    dump();
    switch(ni_r.h.al) {
      case 0x01:
      case 0x06:
      case 0x07:
      case 0x08:
      case 0x0a:
        ni_r.h.al = ni_r.h.al;
        ni_ch = ni_r.h.al;
        break;
    }
  }
  switch(ni_ch) {
    case 0x01:
      NEW_STK();
      ni_ch=getkeyext();
      outchr(ni_ch);
      if (hangup)
        ni_ch=3;
      ni_r.h.al=ni_ch;
      ni_ch1=1;
      OLD_STK();
      break;
    case 0x02:
      NEW_STK();
      outchr(ni_r.h.dl);
      ni_ch1=1;
      checka2();
      OLD_STK();
      break;
    case 0x06:
      NEW_STK();
      if (ni_r.h.dl!=0xff) {
        outchr(ni_r.h.dl);
        ni_ch1=1;
      } else {
        if (empty()) {
          ni_r.x.flags |= 64;
        } else {
          ni_r.x.flags &= (0xffff ^ 64);
          ni_r.h.al=getkeyext();
        }
      }
      OLD_STK();
      break;
    case 0x07:
      NEW_STK();
      ni_ch1=1;
      ni_r.h.al=getkeyext();
      OLD_STK();
      break;
    case 0x08:
      NEW_STK();
      ni_ch1=1;
      ni_r.h.al=getkeyext();
      OLD_STK();
      break;
    case 0x09:
      NEW_STK();
      outdosstr((char *) MK_FP(ni_s.ds, ni_r.x.dx));
      ni_ch1=1;
      OLD_STK();
      break;
    case 0x0a:
      NEW_STK();
      ni_st=(char *) MK_FP(ni_s.ds,ni_r.x.dx);
      ni_n=(unsigned int)(ni_st[0]);
      if (in_extern==2)
        getkeyext();
      in_extern=0;
      input_extern=1;
      input1(&(ni_st[2]),ni_n-3,1,0);
      input_extern=0;
      in_extern=1;
      ni_st[1]=strlen(&(ni_st[2]));
      strcat(&(ni_st[2]),"\r");
      if ((hangup)) {
        strcpy(&(ni_st[2]),"EXIT\r");
        ni_st[1]=4;
        outs("Exiting...");
      }
      ni_ch1=1;
      OLD_STK();
      break;
    case 0x0b:
      NEW_STK();
      if (empty())
        ni_r.h.al=0x00;
      else
        ni_r.h.al=0xff;
      ni_ch1=1;
      OLD_STK();
      break;
    case 0x0c:
      break;
    case 0x3f:
      if (ni_r.x.bx==0x0000) {
        NEW_STK();
        ni_st=(char *)MK_FP(ni_s.ds,ni_r.x.dx);
        inputl(ni_st,ni_r.x.cx);
        strcat(ni_st,"\r\n");
        ni_r.x.ax=strlen(ni_st);
        if (hangup)
          ni_r.x.ax=0;
        ni_r.x.flags &=(0xffff ^ 1);
        ni_ch1=1;
        OLD_STK();
      } else
        int86x(save_dos,&ni_r,&ni_r,&ni_s);
      break;
    case 0x40:
      if ((ni_r.x.bx==0x0001) || (ni_r.x.bx==0x0002)) {
        NEW_STK();
        ni_st=(char *)MK_FP(ni_s.ds,ni_r.x.dx);
        for (ni_n=0; ni_n<ni_r.x.cx; ni_n++) {
          outchr(ni_st[ni_n]);
          checka2();
        }
        ni_r.x.ax=ni_r.x.cx;
        ni_r.x.flags &=(0xffff ^ 1);
        ni_ch1=1;
        OLD_STK();
      } else
        int86x(save_dos,&ni_r,&ni_r,&ni_s);
      break;
    default:
      int86x(save_dos,&ni_r,&ni_r,&ni_s);
      break;
  }

  if (ni_ch1) {
    if ((eflags & EFLAG_INTERNAL) && (!abortext)) {
      checka1();
      if (abortext) {
        ni_r.x.ax=0x4c00;
        int86x(save_dos,&ni_r,&ni_r,&ni_s);
      }
    }
    checkhangup();
    if (hangup) {
      if (hanguptime1<0L) {
        hanguptime1=timer1();
        if (funcs[20]) {
          outs("Terminating...\r\n");
          ip=FP_OFF(funcs[20]);
          cs=FP_SEG(funcs[20]);
          funcs[20]=(void far *)36;
        } else {
          funcs[20]=(void far *)36;
          outs("Aborting...\r\n");
          ni_r.x.ax=0x4c00;
          int86x(save_dos,&ni_r,&ni_r,&ni_s);
        }
      } else {
        if (((unsigned long) funcs[20])>500)
          funcs[20]=(void far *)36;
        if (labs(timer1()-hanguptime1)>(long)funcs[20]) {
          hanguptime1=timer1();
          outs("Aborting...\r\n");
          ni_r.x.ax=0x4c00;
          int86x(save_dos,&ni_r,&ni_r,&ni_s);
        }
      }
    }
  }

  ax=ni_r.x.ax;
  bx=ni_r.x.bx;
  cx=ni_r.x.cx;
  dx=ni_r.x.dx;
  si=ni_r.x.si;
  di=ni_r.x.di;
  flags=ni_r.x.flags;
  ds=ni_s.ds;
  es=ni_s.es;
}

#pragma warn +par


