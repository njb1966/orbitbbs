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

#include "ripint.h"
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
#if defined(BC_FAST_VIDEO) || defined(__OS2__)
#include <conio.h>
#endif
#ifdef __OS2__
#include "portio.h"
#else
#ifdef RIPDRIVE
//#define rd_on() (ripdrive && (sysinfo.flags & OP_FLAGS_RIPDRIVE_ON))
 char kbuf[768], *kb = NULL;
 char lastkey;
#endif
#endif

#define frequency 500

#define UPPER_CASE  0
#define MIXED_CASE  1
#define PROPER_CASE 2
#define FILE_NAME   3

/****************************************************************************/

#ifndef __OS2__
int check_comport(int pn)
{
  unsigned char new_iir, old_iir;

  old_iir = inportb(syscfgovr.com_base[pn]+2);
  outportb(syscfgovr.com_base[pn]+2,0x81);
  new_iir = inportb(syscfgovr.com_base[pn]+2);
  outportb(syscfgovr.com_base[pn]+2,old_iir);

  if (new_iir & 0x38) {
    /* no com port */
    return(0);
  }

  if (new_iir==0) {
    /* unbuffered */
    return(1);
  }

  outportb(syscfgovr.com_base[pn]+2,0xc1);
  new_iir = inportb(syscfgovr.com_base[pn]+2);
  outportb(syscfgovr.com_base[pn]+2,old_iir);

  switch ((new_iir >> 6) & 0x03) {
    case 0: /* no 16550 */
    case 1: /* huh? */
      return(1);

    case 2: /* 16550 */
    case 3: /* 16550A */
      return(2);
  }

  return(1);
}
#endif

/****************************************************************************/

void savel(char *cl, char *atr, char *xl, char *cc)
{
  int i, i1;

  *cc = curatr;
  strcpy(xl, endofline);
#ifdef __OS2__
  { /* add in these extra braces to define more local vars */
    char buf[1024];
    USHORT CurX, CurY, BufLen = sizeof(buf);

    VioGetCurPos(&CurY, &CurX, 0);
    VioReadCellStr(buf, &BufLen, CurY, 0, 0);
    i = 0;
    for (i1 = 0; i1 < WhereX(); i1++) {
      cl[i1] = buf[i1 * 2];
      atr[i1] = buf[(i1 * 2) + 1];
    }
  }
#else
  i = ((WhereY() + topline) * 80) * 2;
  for (i1 = 0; i1 < WhereX(); i1++) {
    cl[i1]  = scrn[i + (i1 * 2)];
    atr[i1] = scrn[i + (i1 * 2) + 1];
  }
#endif
  cl[WhereX()]  = 0;
  atr[WhereX()] = 0;
}

/****************************************************************************/

void restorel(char *cl, char *atr, char *xl, char *cc)
{
  int i;

  if (WhereX())
    nl();
  for (i = 0; cl[i] != 0; i++) {
    setc(atr[i]);
    outchr(cl[i]);
  }
  setc(*cc);
  strcpy(endofline, xl);
}

/****************************************************************************/

void ptime(void)
{
  char xl[81], cl[81], atr[81], cc, s[81];
  long l;

  savel(cl, atr, xl, &cc);

  ansic(0);
  nln(2);
  time(&l);
  strcpy(s, ctime(&l));
  s[strlen(s) - 1] = 0;
  pl(s);
  if (useron && !in_extern) {
    outstr(get_string(29)); npr("%s\r\n", ctim(timer() - timeon));
    outstr(get_string(921)); npr("%s\r\n", ctim(nsl()));
  }
  nl();

  restorel(cl, atr, xl, &cc);
}

/****************************************************************************/

void reprint(void)
{
  char xl[81], cl[81], atr[81], cc, ansistr_1[81];
  int ansiptr_1;

  ansiptr_1=ansiptr;
  ansiptr=0;
  ansistr[ansiptr_1]=0;
  strcpy(ansistr_1,ansistr);

  savel(cl, atr, xl, &cc);
  nl();
  restorel(cl, atr, xl, &cc);

  strcpy(ansistr,ansistr_1);
  ansiptr=ansiptr_1;
}

/****************************************************************************/

void print_help(int n)
{
  char xl[81], cl[81], atr[81], cc, s[81], ch;
  int next, ot;

  if (menu_on() && (helpl == 1)) {
    printmenu(340);
    do {
      ch=getkey();
    } while ((ch = getkey()) != '\r' && ch != '\n' && (hangup == 0));
    cleared = WASCLEARED;
  }

  if (rip_on())
    comstr("\x1|1\x1b""0000$COFF$$SAVE0$$SMF$\r");
  sprintf(s,"%sHELP.MSG",languagedir);
  next = 0;

  if (helps[n].stored_as) {
    ot=tagging; tagging=0;
    savel(cl, atr, xl, &cc);
    ansic(0);
    outstr("\f");

    read_message1(&helps[n], 0, 0, &next, s);
    if (rip_on()) {
      pausescr();
      comstr("\x1|1\x1b""0000$RESTORE0$$RMF$$CON$\r");
/*    if (menu_on())
	  if (rip_pause)
		printmenu(334);
	  else
		rip_cls();
*/
    }

    if ((ot) && !(thisuser.sysstatus & sysstatus_no_tag)) {
      pausescr();
      relist();
    }
    restorel(cl, atr, xl, &cc);
    tagging=ot;
  }
}

/****************************************************************************/

#ifdef __OS2__
#define inportb inp
#define outportb outp
#endif

void setbeep(int i)
{
  int i1,i2;

  if (i) {
    i1 = 0x34DD / frequency;
    i2 = inportb(0x61);
    if (!(i2 & 0x03)) {
      outportb(0x61, i2 | 0x03);
      outportb(0x43, 0xB6);
    }
    outportb(0x42, i1 & 0x0F);
    outportb(0x42, i1 >> 4);
  } else
    outportb(0x61, inportb(0x61) & 0xFC);
}

/****************************************************************************/

void far interrupt async_isr(void)
/* This function is called every time a char is received on the com port.
 * The character is stored in the buffer[] array, and the head pointer is
 * updated.
 */
{
#ifndef __OS2__
  buffer[head++] = inportb(base);
  if (head == MAX_BUF)
    head = 0;
  ISR_RESET(async_irq);
#endif
}

/****************************************************************************/

#ifndef __OS2__
void outcomch(char ch)
/* This function outputs one character to the com port */
{
  if (base) {
    while (!(inportb(base + 5) & 0x20))
      ;
    if (flow_control)
      while (!(inportb(base + 6) & 0x10))
        ;
    outportb(base, ch);
  }
}
#endif

/****************************************************************************/

char peek1c(void)
{
  if (head!=tail) {
    return(buffer[tail]);
  } else
    return(0);
}

/****************************************************************************/

char get1c(void)
/* This function returns one character from the com port, or a zero if
 * no character is waiting
 */
{
  char c1;

  if (head != tail) {
#ifndef __OS2__
    disable();
#endif
    c1 = buffer[tail++];
    if (tail == MAX_BUF)
      tail = 0;
#ifndef __OS2__
    enable();
#endif
    return(c1);
  } else
    return(0);
}

/****************************************************************************/

int comhit(void)
/* This returns a value telling if there is a character waiting in the com
 * buffer.
 */
{
  return(head != tail);
}

/****************************************************************************/

void dump(void)
/* This function clears the com buffer */
{
#ifndef __OS2__
  disable();
#endif
  head = tail = 0;
#ifndef __OS2__
  enable();
#endif
}

/****************************************************************************/

#ifndef __OS2__

/****************************************************************************/

void set_baud(unsigned int rate)
/* This function sets the com speed to that passed */
{
  float rl;

  if ((((rate > 49) && (rate < 57601)) || (rate==1)) && base) {
    if ((rate==1) || (rate==49664)) /* 49664 = 115200 % 65536 */
      rl = 1;
    else
      rl = 115200.0 / ((float) rate);
    rate = (int) rl;
    outportb(base + 3, inportb(base + 3) | 0x80);
    outportb(base,     (rate & 0x00FF));
    outportb(base + 1, ((rate >> 8) & 0x00FF));
    outportb(base + 3, inportb(base + 3) & 0x7F);
  }
}

/****************************************************************************/

void initport(int port_num)
/* This function initializes the com buffer, setting up the interrupt,
 * and com parameters
 */
{
  int temp;

  base = syscfgovr.com_base[port_num];
  async_irq = syscfgovr.com_ISR[port_num];
  setvect(ISR_VECT(async_irq), async_isr);
  head = tail = 0;
  outportb(base + 3, 0x03);
  disable();
  temp = inportb(base + 5);
  temp = inportb(base);

  temp = inportb(ISR_CTRLR(async_irq));
  temp = temp & ((1 << (async_irq%8)) ^ 0x00FF);
  outportb(ISR_CTRLR(async_irq), temp);
  outportb(base + 1, 0x01);
  temp=inportb(base + 4);
  outportb(base + 4, temp | 0x0A);
  outportb(base+2,0x40);
  enable();
  dtr(1);
}

/****************************************************************************/

void closeport(void)
/* This function closes out the com port, removing the interrupt routine,
 * etc.
 */
{
  int temp;

  if (base) {
    disable();
    temp = inportb(ISR_CTRLR(async_irq));
    temp = temp | ((1 << (async_irq%8)));
    outportb(ISR_CTRLR(async_irq), temp);
    outportb(base + 2, 0);
    outportb(base + 4, 3);
    setvect(ISR_VECT(async_irq),getvect(8)); /* for desqview */
    enable();
    base=0;
  }
}

/****************************************************************************/

void dtr(int i)
/* This function sets the DTR pin to the status given */
{
  int i1;

  if (base) {
    i1 = inportb(base + 4) & 0x00FE;
    outportb(base + 4, (i || no_hangup) ? (i1 + 1) : i1);
  }
}

/****************************************************************************/

void rts(int i)
/* This function sets the RTS pin to the status given */
{
  int i1;

  if (base) {
    i1 = inportb(base + 4) & 0x00FD;
    outportb(base + 4, (i) ? (i1 + 2) : i1);
  }
}

/****************************************************************************/

int cdet(void)
/* This returns the status of the carrier detect lead from the modem */
{
  if (base)
    return((inportb(base + 6) & 0x80) ? 1 : 0);
  else
    return(0);
}

/****************************************************************************/
#endif
/****************************************************************************/

void checkhangup(void)
/* This function checks to see if the user logged on to the com port has
 * hung up.  Obviously, if no user is logged on remotely, this does nothing.
 * If carrier detect is detected to be low, it is checked 100 times
 * sequentially to make sure it stays down, and is not just a quirk.
 */
{
  int i, ok;

  if (!hangup && using_modem && !cdet()) {
    ok = 0;
    for (i = 0; (i < 500) && !ok; i++)
      if (cdet())
        ok = 1;
    if (!ok) {
      hangup = hungup = 1;
      if (useron && !in_extern)
        sysoplog(get_stringx(1,99));
    }
  }
}

/****************************************************************************/

void addto(char *s, int i)
{
  char temp[20];

  if (s[0])
    strcat(s, ";");
  else
    strcpy(s, "\x1b[");
  itoa(i, temp, 10);
  strcat(s, temp);
}

/****************************************************************************/

void makeansi(unsigned char attr, char *s, int forceit)
/* Passed to this function is a one-byte attribute as defined for IBM type
 * screens.  Returned is a string which, when printed, will change the
 * display to the color desired, from the current function.
 */
{
  unsigned char catr;
  char *temp = "04261537";

  catr = curatr;
  s[0] = 0;
  if (attr != catr) {
    if ((catr & 0x88) ^ (attr & 0x88)) {
      addto(s, 0);
      addto(s, 30 + temp[attr & 0x07] - '0');
      addto(s, 40 + temp[(attr & 0x70) >> 4] - '0');
      catr = attr & 0x77;
    }
    if ((catr & 0x07) != (attr & 0x07))
      addto(s, 30 + temp[attr & 0x07] - '0');
    if ((catr & 0x70) != (attr & 0x70))
      addto(s, 40 + temp[(attr & 0x70) >> 4] - '0');
    if ((catr & 0x08) ^ (attr & 0x08))
      addto(s, 1);
    if ((catr & 0x80) ^ (attr & 0x80)) {
      if (checkcomp("Mac"))     /*This is the code for Mac's underline*/
        addto(s, 4);            /*They don't have Blinking or Italics*/
      else {
        if (checkcomp("Ami"))   /*Some Amiga terminals use 3 instead of*/
          addto(s, 3);          /*5 for italics.  Using both won't hurt*/
        addto(s, 5);            /*anything, only italics will be generated*/
      }
    }
  }
  if (s[0])
    strcat(s, "m");
  if (!okansi() && !forceit)
    s[0]=0;
}

/****************************************************************************/

void setfgc(int i)
/* This sets the foreground color to that passed.  It is called only from
 * execute_ansi
 */
{
  curatr = (curatr & 0xf8) | i;
}

/****************************************************************************/

void setbgc(int i)
/* This sets the background color to that passed.  It is called only from
 * execute_ansi
 */
{
  curatr = (curatr & 0x8f) | (i << 4);
}

/****************************************************************************/

void execute_ansi(void)
/* This function executes an ANSI string to change color, position the
 * cursor, etc.
 */
{
  int args[11],argptr,count,ptr,tempptr,ox,oy;
  char cmd,temp[11],*clrlst="04261537";

  if (ansistr[1] != '[') {

    /* do nothing if invalid ANSI string. */

  } else {
    argptr = tempptr = 0;
    ptr = 2;
    for (count = 0; count < 10; count++)
      args[count] = temp[count] = 0;
    cmd = ansistr[ansiptr - 1];
    ansistr[ansiptr - 1] = 0;
    while ((ansistr[ptr]) && (argptr<10) && (tempptr<10)) {
      if (ansistr[ptr] == ';') {
        temp[tempptr] = 0;
        tempptr = 0;
        args[argptr++] = atoi(temp);
      } else
        temp[tempptr++] = ansistr[ptr];
      ++ptr;
    }
    if (tempptr && (argptr<10)) {
      temp[tempptr]  = 0;
      args[argptr++] = atoi(temp);
    }
    if ((cmd >= 'A') && (cmd <= 'D') && !args[0])
      args[0] = 1;
    switch (cmd) {
        case 'f':
        case 'H':
          movecsr(args[1] - 1, args[0] - 1);
          break;
        case 'A':
          movecsr(WhereX(), WhereY() - args[0]);
          break;
        case 'B':
          movecsr(WhereX(), WhereY() + args[0]);
          break;
        case 'C':
          movecsr(WhereX() + args[0], WhereY());
          break;
        case 'D':
          movecsr(WhereX() - args[0], WhereY());
          break;
        case 's':
          oldx = WhereX();
          oldy = WhereY();
          break;
        case 'u':
          movecsr(oldx, oldy);
          break;
        case 'J':
          if (args[0] == 2) {
            if (x_only)
              movecsr(0,0);
            else
              clrscrb();
          }
          break;
        case 'k':
        case 'K':
          if (!x_only) {
#if defined(BC_FAST_VIDEO) || defined(__OS2__)
          char EOLBuffer[160];
          int Temp;

          for (Temp = 0; Temp < sizeof(EOLBuffer); Temp++)
            if (Temp % 2)
              EOLBuffer[Temp] = (char) curatr;
            else
              EOLBuffer[Temp] = ' ';
          puttext(wherex(), wherey(), 80, wherey(), (void *)&EOLBuffer);
#else
            ox = WhereX();
            oy = WhereY();
            _CX = 80 - ox;
            _AH = 0x09;
            _BH = 0x00;
            _AL = 32;
            _BL = curatr;
            my_video_int();
            movecsr(ox, oy);
#endif
          }
          break;
        case 'm':
          if (!argptr) {
            argptr = 1;
            args[0] = 0;
          }
          for (count = 0; count < argptr; count++)
            switch (args[count]) {
              case 0: curatr = 0x07; break;
              case 1: curatr = curatr | 0x08; break;
              case 4: break;
              case 5: curatr = curatr | 0x80; break;
              case 7:
                ptr = curatr & 0x77;
                curatr = (curatr & 0x88) | (ptr << 4) | (ptr >> 4);
                break;
              case 8: curatr = 0; break;
              default:
                if ((args[count] >= 30) && (args[count] <= 37))
                  setfgc(clrlst[args[count] - 30] - '0');
                else if ((args[count] >= 40) && (args[count] <= 47))
                  setbgc(clrlst[args[count] - 40] - '0');
            }
          break;
      }
    }
  ansiptr = 0;
}

/****************************************************************************/

void outchr(unsigned char c)
/* This function outputs one character to the screen, and if output to the
 * com port is enabled, the character is output there too.  ANSI graphics
 * are also trapped here, and the ansi function is called to execute the
 * ANSI codes
 */
{
  int i, i1, nc;
  char cc[20];
  static char pipe_color[3];

  if (change_color==5) {
    change_color=0;
    outstr((char *)interpret(c));
    return;
  } if (change_color==4) {
    if (c==15)
      change_color=5;
    else
      change_color=0;
    return;
  } else if (change_color==3) {
    change_color=0;
    pipe_color[1]=c;
    pipe_color[2]=0;

    if (isdigit(pipe_color[0])) {
      if (isdigit(pipe_color[1]) || (pipe_color[1]==' ')) {
        nc=atoi(pipe_color);
      } else {
        change_color=-1;
      }
    } else if ((pipe_color[0]==' ') && isdigit(pipe_color[1])) {
      nc=atoi(pipe_color+1);
    } else if ((pipe_color[0]=='b') || (pipe_color[0]=='B')) {
      nc=16+atoi(pipe_color+1);
    } else {
      change_color=-1;
    }
    if (nc>=32)
      change_color=-1;


    if (change_color==-1) {
      outchr('|');
      outstr(pipe_color);
    } else {
      if (nc<16) {
        makeansi((curatr&0xf0)|nc, cc, 0);
        /* buildfor(nc, cc); */
      } else {
        makeansi((curatr&0x0f)|(nc<<4), cc, 0);
        /* buildback(nc, cc); */
      }
      outstr(cc);
    }
    return;
  } else if (change_color==2) {
    pipe_color[0]=c;
    ++change_color;
    return;
  } else if (change_color==1) {
    change_color = 0;
    if ((c >= '0') && (c <= '9'))
      ansic(c - '0');
    return;
  }

  if (c == 3) {
    change_color = 1;
    return;
  } else if (c==15) {
    change_color=4;
    return;
  } else if ((c=='|') && (g_flags & g_flag_pipe_colors) && (change_color!=-1)) {
    change_color = 2;
    return;
  } else if ((c == 10) && endofline[0]) {
    if (!in_extern)
      outstr(endofline);
    endofline[0] = 0;
  } else if (change_color==-1) {
    change_color=0;
  }

  if (global_handle) {
    if (echo)
      global_char(c);
  }

  if (!(sysinfo.flags & OP_FLAGS_NEW_CHATSOUND)) {
    if (chatcall && !x_only && !(syscfg.sysconfig & sysconfig_no_beep))
      setbeep(1);
  }

  if (outcom && !x_only && (c != 9)) {
    if (!(!okansi() && (ansiptr || c==27)))
      outcomch(echo ? c : 'X');
  }
  if (ansiptr) {
    ansistr[ansiptr++] = c;
    ansistr[ansiptr]   = 0;
    if ((((c < '0') || (c > '9')) && (c!='[') && (c!=';')) ||
        (ansistr[1] != '[') || (ansiptr>75))
      execute_ansi();
  } else if (c == 27) {
    ansistr[0] = 27;
    ansiptr = 1;
    ansistr[ansiptr]=0;
  } else {
    if (c == 9) {
      i1 = WhereX();
      for (i = i1; i< (((i1 / 8) + 1) * 8); i++)
        outchr(32);
    } else if (echo || lecho) {
      out1ch(c);
      if (c == 10) {
        ++lines_listed;
        if (lines_listed >= screenlinest - 3) {
          if(!in_extern) {
            if ((tagging) && !(thisuser.sysstatus & sysstatus_no_tag) &&
                filelist && !chatting) {
              if(num_listed!=0)
                tag_files();
              lines_listed = 0;
            }
          }
        }
        if (lines_listed >= screenlinest - 1) {
          if (sysstatus_pause_on_page & thisuser.sysstatus)
            if (!x_only)
              pausescr();
          lines_listed = 0;
        }
      }
    } else
      out1ch('X');
  }
  if (chatcall)
    setbeep(0);
}

/****************************************************************************/

void outstr(unsigned char *s)
/* This function outputs a string of characters to the screen (and remotely
 * if applicable).  The com port is also checked first to see if a remote
 * user has hung up
 */
{
  int i=0;

  checkhangup();
  if (!hangup)
    while (s[i])
      outchr(s[i++]);
}

/****************************************************************************/

void nl(void)
/* This function performs a CR/LF sequence to move the cursor to the next
 * line.  If any end-of-line ANSI codes are set (such as changing back to
 * the default color) are specified, those are executed first.
 */
{
  if (endofline[0]) {
    outstr(endofline);
    endofline[0] = 0;
  }
  if (ripcode)
    comstr("\r");
  else
    outstr("\r\n");
  if (inst_msg_waiting())
    process_inst_msgs();
}

/****************************************************************************/

void nln(int n)
{
  int i;

  for (i=0; i<n; i++) {
    nl();
  }
}

/****************************************************************************/

void backspace(void)
/* This function executes a backspace, space, backspace sequence. */
{
  int i;

  i = echo;
  echo = 1;
  outstr("\b \b");
  echo = i;
}

/****************************************************************************/

void setc(unsigned char ch)
/* This sets the current color (both locally and remotely) to that
 * specified (in IBM format).
 */
{
  char s[30];

  makeansi(ch, s, 0);
  outstr(s);
}

/****************************************************************************/

void pausescr(void)
/* This will pause output, displaying the [PAUSE] message, and wait for
 * a key to be hit.
 */
{
  int i,i1,i2,oiia;
  char *ss;

  if (x_only)
    return;

  oiia=iia;
  setiia(0);

  ss=str_pause;
  i2=i1=strlen(ss);
  for (i=0; i<i2; i++)
    if ((ss[i]==3) && (i1>1))
      i1-=2;

  if (okansi()) {
    i = curatr;
    setc((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[3] :
          thisuser.bwcolors[3]);
    outstr(ss);
    npr("\x1b[%dD",i1);
    setc(i);
    if (in_extern)
      getkey();
    else
      getkeymouse();
    for (i=0; i<i1; i++)
      outchr(' ');
    npr("\x1b[%dD",i1);
  } else {
    outstr(ss);
    if (in_extern)
      getkey();
    else
      getkeymouse();
    for (i = 0; i<i1; i++)
      backspace();
  }

  setiia(oiia);
}

/****************************************************************************/

void npr(char *fmt, ...)
/* just like printf, only out to the com port */
{
  va_list ap;
  char s[512];

  va_start(ap, fmt);
  vsprintf(s, fmt, ap);
  va_end(ap);
  outstr(s);
}

/****************************************************************************/

void pl(char *s)
{
  outstr(s);
  nl();
}

/****************************************************************************/

void pln(int n)
{
  char s[81];

  sprintf(s,"%u",n);
  pl(s);
}

/****************************************************************************/

#if defined(BC_FAST_VIDEO) || defined(__OS2__)
/*
 * Define this to get around the bug where Borland's kbhit()
 * will return false if user pressed an extended key AND
 * WWIV has already read in the '0' that indicates an
 * extended key.
 */
static int ExtendedKeyWaiting = 0;
#endif

int kbhitb(void)
{
#if defined(BC_FAST_VIDEO) || defined(__OS2__)
  return (x_only ? 0 : (kbhit() || ExtendedKeyWaiting));
#else
  union REGS r;

  if (x_only)
    return(0);

  r.h.ah = 1;
  int86(0x16, &r, &r);
#ifdef RIPDRIVE
  if (rd_on()) {
    if (*kb == 0 || kb == NULL) {
      *kbuf = 0;
      kb = kbuf;
    }
    localrip_keyfilter(0, kbuf);
    if ((*kb != 0)) {
      return (*kb);
    }
  }
#endif
  return((r.x.flags & 64) == 0);
#endif
}

/****************************************************************************/

int empty(void)
{
  if (x_only)
    return(1);

  if (kbhitb() || (incom && (head != tail)) ||
      (charbufferpointer && charbuffer[charbufferpointer]) ||
      (in_extern == 2))
    return(0);
  return(1);
}

/****************************************************************************/

void skey1(unsigned char *ch)
{
  unsigned char c;

  c = *ch;
  if ((c == 127) && (!in_fsed))
    c = 8;
  if (okskey)
    switch(c) {
      case 1:
      case 4:
      case 6:
        if (okmacro && !charbufferpointer) {
          if (c == 1)
            c = 2;
          else if (c == 4)
            c = 0;
          else if (c == 6)
            c = 1;
          strcpy(charbuffer, &(thisuser.macros[c][0]));
          c = charbuffer[0];
          if (c)
            charbufferpointer = 1;
        }
        break;
      case 15:
        if (helpl && !ihelp && !chatting && echo) {
          if (menu_on())
             cleared = -1;
          ihelp = 1;
          print_help(helpl);
          ihelp = 0;
        }
        break;
      case submenu_code:
        if (rip_on()) {
          if (end_submenu)
            submenu();
          else
            end_submenu = -1;
        }
        break;
      case 5:
        if (rip_on())
          hypertext();
        break;
      case 20:
        if (echo)
          ptime();
        if (menu_on() && rip_pause) {
          pausescr();
        }
        break;
      case 18:
        if (echo)
          reprint();
        break;
      case 25:
        thisuser.sysstatus ^= sysstatus_pause_on_page;
        break;
    }
  *ch = c;
}

/****************************************************************************/
/*
 * returns the ASCII code of the next character waiting in the
 * keyboard buffer.  If there are no characters waiting in the
 * keyboard buffer, then it waits for one.
 *
 * A value of 0 is returned for all extended keys (such as F1,
 * Alt-X, etc.).  The function must be called again upon receiving
 * a value of 0 to obtain the value of the extended key pressed.
 */
unsigned char getchd(void)
{
#if defined(BC_FAST_VIDEO) || defined(__OS2__)
  unsigned char rc;

  if (ExtendedKeyWaiting)
    {
      ExtendedKeyWaiting = 0;
      return (unsigned char) getch();
    }
  rc = getch();
  if (rc == 0)
    ExtendedKeyWaiting = 1;
  return rc;
#else
  union REGS r;
#ifdef RIPDRIVE
  if ((kb == NULL) || (*kb == 0) || (rd_on() == 0)) {
#endif

	r.h.ah = 0x07;
	int86(save_dos, &r, &r);
  
#ifdef RIPDRIVE
	if (rd_on()) {
	  lastkey = r.h.al;
	  if (*kb == 0) {
		*kbuf = 0;
	  }
	  if (lastkey) {         // We don't want to process extd. keys
		localrip_keyfilter(r.h.al, kbuf);
		kb = kbuf;
	  }
	}
  }
  if (rd_on() && (kb != NULL) && (*kb != 0)) {
	return (*(kb++));
  }
#endif
  return(r.h.al);
#endif
}

/****************************************************************************/
/*
 * returns the ASCII code of the next character waiting in the
 * keyboard buffer.  If there are no characters waiting in the
 * keyboard buffer, then it returns immediately with a value
 * of 255.
 *
 * A value of 0 is returned for all extended keys (such as F1,
 * Alt-X, etc.).  The function must be called again upon receiving
 * a value of 0 to obtain the value of the extended key pressed.
 */
unsigned char getchd1(void)
{
#if defined(BC_FAST_VIDEO) || defined(__OS2__)
  unsigned char rc;

  if (!((kbhit()) || ExtendedKeyWaiting))
    return 255;
  if (ExtendedKeyWaiting)
    {
      ExtendedKeyWaiting = 0;
      return (unsigned char) getch();
    }
  rc = getch();
  if (rc == 0)
    ExtendedKeyWaiting = 1;
  return rc;
#else
  union REGS r;
#ifdef RIPDRIVE
  static char last;
  int ky;
  if ((kb == NULL) || (*kb == 0) || (rd_on() == 0)) {
#endif

	r.h.ah = 0x06;
	r.h.dl = 0xFF;
	int86(save_dos, &r, &r);
	
#ifdef RIPDRIVE
	if (rd_on()) {
	  if ((r.x.flags & 0x40) == 0) {
		ky = r.h.al;
		last = (char) ky;
	   } else {
		 ky = 0;
		 last = 255;
	   }
	   if (*kb == 0) {
		 *kbuf = 0;
	   }
	   if (last) {         // We don't want to process extd. keys
		 localrip_keyfilter(ky, kbuf);
		 kb = kbuf;
	   }
	}
  }
  if (rd_on() && (kb != NULL) && (*kb != 0)) {
	return (*(kb++));
  }
#endif
		 
  return((r.x.flags & 0x40) ? 255 : r.h.al);
#endif
}

/****************************************************************************/

unsigned char inkey(void)
/* This function checks both the local keyboard, and the remote terminal
 * (if any) for input.  If there is input, the key is returned.  If there
 * is no input, a zero is returned.  Function keys hit are interpreted as
 * such within the routine and not returned.
 */
{
  unsigned char ch=0;

  if (x_only)
    return(0);

  if (charbufferpointer) {
    if (!charbuffer[charbufferpointer]) {
      charbufferpointer = charbuffer[0] = 0;
    } else {
      if ((charbuffer[charbufferpointer])==3)
        charbuffer[charbufferpointer]=16;
      return(charbuffer[charbufferpointer++]);
    }
  }
  if (kbhitb() || (in_extern == 2)) {
    ch = getchd1();
    lastcon = 1;
    if (!(g_flags & g_flag_allow_extended)) {
      if (!ch) {
        if (in_extern)
          in_extern = 2;
        else {
          ch = getchd1();
          skey(ch);
          ch = (((ch == 68) || (ch==103)) ? 2 : 0);
        }
      } else if (in_extern)
        in_extern = 1;
    }
    timelastchar1=timer1();
  } else if (incom && comhit()) {
    ch = (get1c() & andwith);
    lastcon = 0;
  }
  if (!(g_flags & g_flag_allow_extended))
    skey1(&ch);

  return(ch);
}

/****************************************************************************/

void mpl(int i)
/* This will make a reverse-video prompt line i characters long, repositioning
 * the cursor at the beginning of the input prompt area.  Of course, if the
 * user does not want ansi, this routine does nothing.
 */
{
  int i1;
  char s[81];

  if (okansi()) {
    ansic(4);
    for (i1 = 0; i1 < i; i1++)
      outchr(' ');
    outstr("\x1b[");
    itoa(i,s,10);
    outstr(s);
    outstr("D");
  }
}

/****************************************************************************/

unsigned char upcase(unsigned char ch)
/* This converts a character to uppercase */
{
  unsigned char *ss;

  ss=strchr(translate_letters[0],ch);
  if (ss)
    ch=translate_letters[1][ss-translate_letters[0]];
  return(ch);
}

/****************************************************************************/

unsigned char locase(unsigned char ch)
/* This converts a character to lowercase */
{
  unsigned char *ss;

  ss=strchr(translate_letters[1],ch);
  if (ss)
    ch=translate_letters[0][ss-translate_letters[1]];
  return(ch);
}

/****************************************************************************/

unsigned char getkey(void)
/* This function returns one character from either the local keyboard or
 * remote com port (if applicable).  After 1.5 minutes of inactivity, a
 * beep is sounded.  After 3 minutes of inactivity, the user is hung up.
 */
{
  unsigned char ch;
  int beepyet;
  long dd,tv,tv1;

  beepyet = 0;
  timelastchar1=timer1();

  if (so())
    tv=10920L;
  else
    tv=3276L;

  tv1=tv/2;

  if (!tagging || (thisuser.sysstatus & sysstatus_no_tag))
    lines_listed = 0;
  do {
    while (empty() && !hangup) {
      /* if (!in_extern) */
        giveup_timeslice();
      dd = timer1();
      if ((dd<timelastchar1) && ((dd+1000)>timelastchar1))
        timelastchar1=dd;
      if (labs(dd - timelastchar1) > 65536L)
        timelastchar1 -= 1572480L;
      if (((dd - timelastchar1) > tv1) && (!beepyet)) {
        beepyet = 1;
        outchr(7);
      }
      if (labs(dd - timelastchar1) > tv) {
        nl();
        if (!in_extern)
          outstr(get_string(924));
        nl();
        hangup = 1;
      }
      checkhangup();
    }
    ch = inkey();
  } while (!ch && !in_extern && !hangup);
  if (checkit && (ch > 127)) {
    checkit = 0;
    ch = ch & (andwith = 0x7F);
  }
  return(ch);
}

/****************************************************************************/

void input1(unsigned char *s, int maxlen, int lc, int crend)
/* This will input a line of data, maximum maxlen characters long, terminated
 * by a C/R.  if (lc) is non-zero, lowercase is allowed, otherwise all
 * characters are converted to uppercase.
 */
{
  int curpos=0,done=0,in_ansi=0;
  unsigned char ch,*ss;

  while (!done && !hangup) {
    ch = getkey();
    if (in_ansi) {
      if ((in_ansi==1) && (ch!='['))
        in_ansi=0;
      else {
        if (in_ansi==1)
          in_ansi=2;
        else if (((ch<'0') || (ch>'9')) && (ch!=';'))
          in_ansi=3;
        else
          in_ansi=2;
      }
    }
    if (!in_ansi) {
      if (ch > 31) {
        switch (lc) {
          case UPPER_CASE:
            ch=upcase(ch);
            break;
          case MIXED_CASE:
            break;
          case PROPER_CASE:
            ch=upcase(ch);
            if (curpos) {
              ss=strchr(valid_letters,s[curpos-1]);
              if ((ss!=NULL) || (s[curpos-1]==39)) {
                if ((curpos<2) || (s[curpos-2]!=77) || (s[curpos-1]!=99))
                  ch=locase(ch);
              }
            }
            break;
          case FILE_NAME:
            if (strchr("/\\+ <>|*?.,=\";:[]", ch))
              ch=0;
            else
              ch=upcase(ch);
            break;
        }
        if ((curpos<maxlen) && ch) {
          s[curpos++] = ch;
          outchr(ch);
        }
      } else
        switch(ch) {
          case 14:
          case 13:
            s[curpos] = 0;
            done = echo = 1;
            if (crend)
              nl();
            break;
          case 23: /* Ctrl-W */
            if (curpos) {
              do {
                curpos--;
                backspace();
                if (s[curpos]==26)
                  backspace();
              } while ((curpos) && (s[curpos-1]!=32));
            }
            break;
          case 26:
            if (input_extern) {
              s[curpos++] = 26;
              outstr("^Z");
            }
            break;
          case 8:
            if (curpos) {
              curpos--;
              backspace();
              if (s[curpos] == 26)
                backspace();
            }
            break;
          case 21:
          case 24:
            while (curpos) {
              curpos--;
              backspace();
              if (s[curpos] == 26)
                backspace();
            }
            break;
          case 27:
            in_ansi=1;
            break;
        }
    }
    if (in_ansi==3)
      in_ansi=0;
  }
  if (hangup)
    s[0] = 0;
}

/****************************************************************************/

void input(unsigned char *s, int len)
/* This will input an upper-case string */
{
  input1(s, len, UPPER_CASE, 1);
}

/****************************************************************************/

void inputl(unsigned char *s, int len)
/* This will input an upper or lowercase string of characters */
{
  input1(s, len, MIXED_CASE, 1);
}

/****************************************************************************/

void inputp(unsigned char *s, int len)
/* This will input an upper or lowercase string of characters */
{
  input1(s, len, PROPER_CASE, 1);
}

/****************************************************************************/

void inputf(unsigned char *s, int len)
/* This will input an uppercase string of characters, suitable for a fielname */
{
  input1(s, len, FILE_NAME, 1);
}

/****************************************************************************/

static void print_yn(int i)
{
  if (num_strings(i))
    pl(getrandomstring(i));
  else switch(i) {
    case 2: pl(str_yes); break;
    case 3: pl(str_no); break;
  }
}

/****************************************************************************/

int yn(void)
/* The keyboard is checked for either a Y, N, or C/R to be hit.  C/R is
 * assumed to be the same as a N.  Yes or No is output, and yn is set to
 * zero if No was returned, and yn() is non-zero if Y was hit.
 */
{
  char ch=0;

  ansic(1);
  if (menu_on() && rip_popup && (!rip_subset)) {
	outstr("[s\r");
	printmenu(330);
  }
  while ((!hangup) &&
         ((ch = upcase(getkey())) != *str_yes) &&
         (ch != *str_no) &&
         (ch != 13))
    ;
  if (menu_on() && rip_popup && (!rip_subset)) {
	printmenu(335);
	outstr("[u");
  }
  if (ch==*str_yes)
    print_yn(2);
  else
    print_yn(3);
  return(ch == *str_yes);
}

/****************************************************************************/

int ny(void)
/* This is the same as yn(), except C/R is assumed to be "Y" */
{
  char ch=0;

  ansic(1);
  if (menu_on() && rip_popup && (!rip_subset)) {
	outstr("[s\r");
        printmenu(336);
  }
  while ((!hangup) &&
         ((ch = upcase(getkey())) != *str_yes) &&
         (ch != *str_no) &&
         (ch != 13))
    ;
  if (menu_on() && rip_popup && (!rip_subset)) {
	printmenu(335);
	outstr("[u");
  }
  if (ch==*str_no)
    print_yn(3);
  else
    print_yn(2);
  return((ch == *str_yes) || (ch==13));
}

/****************************************************************************/

char ynq(void)
{
  char ch=0;
  ansic(1);
  if (menu_on() && rip_popup && (!rip_subset)) {
	outstr("[s\r");
	printmenu(331);
  }
  while ((!hangup) &&
         ((ch = upcase(getkey())) != *str_yes) &&
         (ch != *str_no) &&
         (ch != *str_quit) &&
         (ch != 13))
    ;
  if (menu_on() && rip_popup && (!rip_subset)) {
	printmenu(335);
	outstr("[u");
  }
  if (ch==*str_yes) {
    ch='Y';
    print_yn(2);
  } else if (ch==*str_quit) {
    ch='Q';
    pl(str_quit);
  } else {
    ch='N';
    print_yn(3);
  }
  return(ch);
}

/****************************************************************************/

void ansic(int n)
{
  char c;

  c = ((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[n] :
        thisuser.bwcolors[n]);
  if (c == curatr)
    return;
  setc(c);
  makeansi((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[0] :
        thisuser.bwcolors[0],endofline, 0);
}

/****************************************************************************/

void ansic_x(int n)
{
  if (E_C) {
    ansic(n);
  }
}

/****************************************************************************/

char onek(char *s)
{
  char ch;

  while (!strchr(s, ch = upcase(getkey())) && !hangup)
    ;
  if (hangup)
    ch = s[0];
  outchr(ch);
  nl();
  return(ch);
}

/****************************************************************************/

void prt(int i, char *s)
{
  ansic(i);
  outstr(s);
  ansic(0);
}

/****************************************************************************/

void reset_colors(void)
{
  outstr("\x1b[0m");
}

/****************************************************************************/

void goxy(int x, int y)
{
  if (okansi())
    npr("\x1b[%d;%dH",y,x);
}

/****************************************************************************/

unsigned char *charstr(int len, unsigned char rc)
/* Returns string comprised of char rc, len characters in length */
{
  static unsigned char s[161];

  if ((rc==0) || (len<1))
    return("");

  if (len>160)
    len=160;

  memset(s,rc,len);
  s[len]=0;
  return(s);
}

/****************************************************************************/

unsigned char *stripcolors(unsigned char *instr)
/* Takes input string and returns same string stripped of color codes. */
{
  static unsigned char s[161];
  int i,i1;

  if (strlen(instr)==0)
    return("");

  i=0; i1=0;
  do {
    if (instr[i]==3)
      i++;
    else {
      s[i1]=instr[i];
      i1++;
    }
    i++;
  } while (i<strlen(instr));
  s[i1]=0;
  return(s);
}

/****************************************************************************/

void trimstr(unsigned char *s)
{
  int i;

  i=strlen(s);
  while ((i>0) && (s[i-1]==32))
    --i;
  s[i]=0;
}

/****************************************************************************/

char onek1(char *s)
{
  char ch;

  while (!strchr(s, ch = upcase(getkey())) && !hangup)
    ;
  if (hangup)
    ch = s[0];
  return(ch);
}

/****************************************************************************/

#ifdef DEBUG_DEBUG
/* These two functions were added to aid in the resolution of missing
 * string characters.  They can be used in place of outstr and npr in
 * those places where string characters are lost.  Seems 18-20 are the
 * magic numbers.
 */

void outstr1(unsigned char *s)
/* This function outputs a string of characters to the screen (and remotely
 * if applicable).  The com port is also checked first to see if a remote
 * user has hung up
 */
{
  int i=0;

  printf("outstr=%d, ",strlen(s)); /* debug */
  checkhangup();
  if (!hangup)
    while (s[i])
      outchr(s[i++]);
}

/****************************************************************************/

void npr1(char *fmt, ...)
/* just like printf, only out to the com port */
{
  va_list ap;
  char s[512];

  printf("\r\nFMT=%d, ",strlen(fmt)); /* debug */
  va_start(ap, fmt);
  printf("s=%d, ",strlen(s)); /* debug */
  vsprintf(s, fmt, ap);
  printf("s=%d, ",strlen(s)); /* debug */
  va_end(ap);
  printf("s=%d, ",strlen(s)); /* debug */
  outstr1(s);
}
#endif

