/*****************************************************************************

                                WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/



#include <process.h>
#include <math.h>
#include <dir.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <dos.h>
#include <alloc.h>
#include <time.h>


#include "vardec.h"
#include "share.h"


void far *funcs[25];
/****************************************************************************/
char *interpret(char c);
void my_video_int(void);
int okansi(void);
void wait1(long l);
void setbeep(int i);
void pla(char *s, int *abort);
void checka(int *abort, int *next);
void inli(char *s, char *rollover, int maxlen, int crend);
long timer1(void);
void movecsr(int x,int y);
int WhereX(void);
int WhereY(void);
void lf(void);
void cr(void);
void clrscrb(void);
void bs(void);
void out1chx(unsigned char ch);
void out1ch(unsigned char ch);
void far interrupt async_isr(void);
void outcomch(char ch);
char get1c(void);
int comhit(void);
void dump(void);
void set_baud(unsigned int rate);
void initport(int portnum);
void closeport(void);
int cdet(void);
void checkhangup(void);
void addto(char *s, int i);
void makeansi(unsigned char attr, char *s, int forceit);
void setfgc(int i);
void setbgc(int i);
void execute_ansi(void);
void outchr(char c);
void outstr(char *s);
void nl(void);
void backspace(void);
void setc(unsigned char ch);
void pausescr(void);
void pl(char *s);
int kbhitb(void);
int empty(void);
void skey1(char *ch);
char getchd(void);
char getchd1(void);
char inkey(void);
void mpl(int i);
char upcase(char ch);
unsigned char getkey(void);
void input1(char *s, int maxlen, int lc, int crend);
void input(char *s, int len);
void inputl(char *s, int len);
int yn(void);
int ny(void);
void ansic(int n);
char onek(char *s);
void prt(int i, char *s);
unsigned char getkeyext(void);
int do_it(char *cl);
int do_remote(char *s, int ccc);
void checka2(void);
void outdosstr(char *s);
int full_external(char *s, int ccc);
int init_r(void);
void get_dir(char *s, int be);
void cd_to(char *s);
void setup_stuff(void);
void main(int argc, char *argv[]);

/****************************************************************************/
char *xenviron[50],newprompt[161];
char ver_no1[51], ver_no2[51];


int topline=0,screenbottom=24,tempio,curatr=0x07,lines_listed=0,outcom=0;
int hangup=0,hungup=0,echo=1,lecho=1,input_extern=0,ctc=0,ccc=0;
int defscreenbottom=24,debuglevel=0,instance=1;

char charbuffer[161]="",endofline[81]="";
int charbufferpointer=0;
int ansiptr=0;

int oldx,oldy,flow_control,save_dos;
char ansistr[81];
int change_color;
userrec thisuser;

unsigned int baud_rate;

long timelastchar1,hanguptime1;

unsigned char andwith=255;

char cdir[81];


volatile int head,tail;
volatile char buffer[MAX_BUF];
int base,async_irq,useron,in_extern,ok_modem_stuff;
int outcom,incom,using_modem,chatting,screenlinest;

extern char *wwiv_version;

#pragma warn -par
void sysoplog(char *s) {}
#pragma warn +par

/****************************************************************************/

void my_video_int(void)
{
#if __TURBOC__ >= 0x0200
  /* TC 2.0 or TC++ here */
  static unsigned short sav_bp;

  __emit__(0x56, 0x57); /* push si, push di */
  sav_bp = _BP;
  geninterrupt(0x10);
  _BP = sav_bp;
  __emit__(0x5f, 0x5e); /* pop di, pop si */
#else
  /* TC 1.5 here */
  _VideoInt();
#endif
}


/****************************************************************************/

int okansi(void)
/* This function checks the status of the current user's record to see if
 * the user has specified that he wants ANSI graphics displayed.
 */
{
  if (thisuser.sysstatus & sysstatus_ansi)
    return(1);
  else
    return(0);
}

void giveup_timeslice(void) {}

void wait1(long l)
{
  long l1;

  l1 = timer1()+l;

  enable();
  while (timer1()<l1)
    ;
}




#define frequency 500

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

void pla(char *s, int *abort)
{
  int i,next;

  i=0;
  checkhangup();
  if (hangup)
    *abort=1;
  checka(abort,&next);
  while ((s[i]) && (!(*abort))) {
    outchr(s[i++]);
    checka(abort,&next);
  }
  if (!(*abort))
    nl();
}


void checka(int *abort, int *next)
{
  char ch;

  while ((!empty()) && (!(*abort)) && (!hangup)) {
    checkhangup();
    ch=inkey();
    switch(ch) {
      case 14:
        *next=1;
      case 3:
      case 32:
      case 24:
        *abort=1;
        break;
      case 'P':
      case 'p':
      case 19:
        ch=getkey();
        break;
    }
  }
}


void inli(char *s, char *rollover, int maxlen, int crend)
{
  int cp,i,i1,done,cm,begx;
  char s1[255];
  unsigned char ch;

  cm=chatting;

  begx=WhereX();
  if (rollover[0]!=0) {
    if (charbufferpointer) {
      strcpy(s1,rollover);
      strcat(s1,&charbuffer[charbufferpointer]);
      strcpy(&charbuffer[1],s1);
      charbufferpointer=1;
    } else {
      strcpy(&charbuffer[1],rollover);
      charbufferpointer=1;
    }
    rollover[0]=0;
  }
  cp=0;
  done=0;
  do {
    ch=getkey();
    if (cm)
      if (chatting==0)
        ch=13;
    if ((ch>=32)) {
      if ((WhereX()<(thisuser.screenchars-1)) && (cp<maxlen)) {
        s[cp++]=ch;
        outchr(ch);
        if (WhereX()==(thisuser.screenchars-1))
          done=1;
      } else {
        if (WhereX()>=(thisuser.screenchars-1))
          done=1;
      }
    } else
        switch(ch) {
	  case 7:
	    if ((chatting) && (outcom))
	      outcomch(7);
	    break;
          case 13: /* C/R */
            s[cp]=0;
            done=1;
            break;
          case 8:  /* Backspace */
            if (cp) {
              if (s[cp-2]==3) {
                cp-=2;
                ansic(0);
              } else if (s[cp-2]==15) {
                for (i=strlen(interpret(s[cp-1])); i>0; i++)
                  backspace();
                cp-=2;
                if (s[cp-1]==15)
                  cp--;
              } else {
                if (s[cp-1]==8) {
                  cp--;
                  outchr(32);
                } else {
                  cp--;
                  backspace();
                }
              }
            }
            break;
          case 24: /* Ctrl-X */
            while (WhereX()>begx) {
              backspace();
              cp=0;
            }
            ansic(0);
            break;
          case 23: /* Ctrl-W */
            if (cp) {
              do {
                if (s[cp-2]==3) {
                  cp-=2;
                  ansic(0);
                } else
                  if (s[cp-1]==8) {
                    cp--;
                    outchr(32);
                  } else {
                    cp--;
                    backspace();
                  }
              } while ((cp) && (s[cp-1]!=32) && (s[cp-1]!=8) && (s[cp-2]!=3));
            }
            break;
          case 14: /* Ctrl-N */
            if ((WhereX()) && (cp<maxlen)) {
              outchr(8);
              s[cp++]=8;
            }
            break;
          case 16: /* Ctrl-P */
            if (cp<maxlen-1) {
              ch=getkey();
              if ((ch>='0') && (ch<='9')) {
                s[cp++]=3;
                s[cp++]=ch;
                ansic(ch-'0');
              } else if ((ch==16) && (cp<maxlen-2)) {
                ch=getkey();

                if (ch != 16) {     // RIP font style code
                  s[cp++]=15;
                  s[cp++]=15;
                  s[cp++]=ch;
                  outchr('\xf');
                  outchr('\xf');
                  outchr(ch);
                }
              }
            }
            break;
          case 9:  /* Tab */
            i=5-(cp % 5);
            if (((cp+i)<maxlen) && ((WhereX()+i)<thisuser.screenchars)) {
              i=5-((WhereX()+1) % 5);
              for (i1=0; i1<i; i1++) {
                s[cp++]=32;
                outchr(32);
              }
            }
            break;
        }
  } while ((done==0) && (hangup==0));
  if (ch!=13) {
    i=cp-1;
    while ((i>0) && (s[i]!=32) && (s[i]!=8) || (s[i-1]==3))
      i--;
    if ((i>(WhereX()/2)) && (i!=(cp-1))) {
      i1=cp-i-1;
      for (i=0; i<i1; i++)
        outchr(8);
      for (i=0; i<i1; i++)
        outchr(32);
      for (i=0; i<i1; i++)
        rollover[i]=s[cp-i1+i];
      rollover[i1]=0;
      cp -= i1;
    }
    s[cp++]=1;
    s[cp]=0;
  }
  if (crend)
    nl();

}

/****************************************************************************/

long timer1(void)
/* This function returns the time, in seconds since midnight. */
{
  unsigned short h,m;
  long l;

  m=peek(0x0040,0x006c);
  h=peek(0x0040,0x006e);
  l=((long)h)*65536 + ((long)m);
  return(l);
}

#define SCROLL_UP(t,b,l) \
  _CH=t;\
  _DH=b;\
  _BH=curatr;\
  _AL=l;\
  _CL=0;\
  _DL=79;\
  _AH=6;\
  my_video_int();

void movecsr(int x,int y)
/* This, obviously, moves the cursor to the location specified, offset from
 * the protected dispaly at the top of the screen
 */
{
  if (x<0)
    x=0;
  if (x>79)
    x=79;
  if (y<0)
    y=0;
  y+=topline;
  if (y>screenbottom)
    y=screenbottom;

  _BH=0x00;
  _DH=y;
  _DL=x;
  _AH=0x02;
  my_video_int();
}



int WhereX(void)
/* This function returns the current X cursor position, as the number of
 * characters from the left hand side of the screen.  An X position of zero
 * means the cursor is at the left-most position
 */
{
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DL;
  return(tempio);
}



int WhereY(void)
/* This function returns the Y cursor position, as the line number from
 * the top of the logical window.  The offset due to the protected top
 * of the screen display is taken into account.  A WhereY() of zero means
 * the cursor is at the top-most position it can be at.
 */
{
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DH;
  return(tempio-topline);
}



void lf(void)
/* This function performs a linefeed to the screen (but not remotely) by
 * either moving the cursor down one line, or scrolling the logical screen
 * up one line.
 */
{
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DL;
  if (_DH==screenbottom) {
    SCROLL_UP(topline,screenbottom,1);
    _DL=tempio;
    _DH=screenbottom;
    _BH=0;
    _AH=0x02;
    my_video_int();
  } else {
    tempio=_DH+1;
    _DH=tempio;
    _AH=0x02;
    my_video_int();
  }
}



void cr(void)
/* This short function returns the local cursor to the left-most position
 * on the screen.
 */
{
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  _DL=0x00;
  _AH=2;
  my_video_int();
}

void clrscrb(void)
/* This clears the local logical screen */
{
  SCROLL_UP(topline,screenbottom,0);
  movecsr(0,0);
  lines_listed=0;
}



void bs(void)
/* This function moves the cursor one position to the left, or if the cursor
 * is currently at its left-most position, the cursor is moved to the end of
 * the previous line, except if it is on the top line, in which case nothing
 * happens.
 */
{
  _BH=0;
  _AH=3;
  my_video_int();
  if (_DL==0) {
    if (_DH != topline) {
      _DL=79;
      tempio=_DH-1;
      _DH=tempio;
      _AH=2;
      my_video_int();
    }
  } else {
    _DL--;
    _AH=2;
    my_video_int();
  }
}



void out1chx(unsigned char ch)
/* This function outputs one character to the screen, then updates the
 * cursor position accordingly, scolling the screen if necessary.  Not that
 * this function performs no commands such as a C/R or L/F.  If a value of
 * 8, 7, 13, 10, 12 (backspace, beep, C/R, L/F, TOF), or any other command-
 * type characters are passed, the appropriate corresponding "graphics"
 * symbol will be output to the screen as a normal character.
 */
{
  _BL=curatr;
  _BH=0x00;
  _CX=0x01;
  _AL=ch;
  _AH=0x09;
  my_video_int();
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  ++_DL;
  if (_DL==80) {
    _DL=0;
    if (_DH==screenbottom) {
      SCROLL_UP(topline,screenbottom,1);
      _DH=screenbottom;
      _DL=0;
      _BH=0;
      _AH=0x02;
      my_video_int();
    } else {
      tempio=_DH+1;
      _DH=tempio;
      _AH=0x02;
      my_video_int();
    }
  } else {
    _AH=0x02;
    my_video_int();
  }
}




void out1ch(unsigned char ch)
/* This function outputs one character to the local screen.  C/R, L/F, TOF,
 * BS, and BELL are interpreted as commands instead of characters.
 */
{
  if (ch>31)
    out1chx(ch);
  else
    if (ch==13)
      cr();
    else
      if (ch==10)
        lf();
      else
        if (ch==12)
          clrscrb();
        else
          if (ch==8)
            bs();
          else
            if (ch==7)
              if (outcom==0) {
                setbeep(1);
                wait1(4);
                setbeep(0);
              }
}



/****************************************************************************/
void far interrupt async_isr(void)
/* This function is called every time a char is received on the com port.
 * The character is stored in the buffer[] array, and the head pointer is
 * updated.
 */
{
  buffer[head++] = inportb(base);
  if (head == MAX_BUF)
    head = 0;
  ISR_RESET(async_irq);
}



void outcomch(char ch)
/* This function outputs one character to the com port */
{
  while (!(inportb(base + 5) & 0x20))
    ;
  if (flow_control)
    while (!(inportb(base + 6) & 0x10))
      ;
  outportb(base, ch);
}




char get1c(void)
/* This function returns one character from the com port, or a zero if
 * no character is waiting
 */
{
  char c1;

  if (head != tail) {
    disable();
    c1 = buffer[tail++];
    if (tail == MAX_BUF)
      tail = 0;
    enable();
    return(c1);
  } else
    return(0);
}



int comhit(void)
/* This returns a value telling if there is a character waiting in the com
 * buffer.
 */
{
  return(head != tail);
}



void dump(void)
/* This function clears the com buffer */
{
  disable();
  head = tail = 0;
  enable();
}

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


void initport(int port_num)
/* This function initializes the com buffer, setting up the interrupt,
 * and com parameters
 */
{
  int temp;

  temp=port_num;

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
  outportb(base + 4, temp | 0x0B);
  enable();
  set_baud(baud_rate);
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

int cdet(void)
/* This returns the status of the carrier detect lead from the modem */
{
  return((inportb(base + 6) & 0x80) ? 1 : 0);
}



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
    }
  }
}




void addto(char *s, int i)
{
  char temp[20];

  if (s[0])
    strcat(s, ";");
  else
    strcpy(s, "\x1B[");
  itoa(i, temp, 10);
  strcat(s, temp);
}



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
    if ((catr & 0x80) ^ (attr & 0x80))
      addto(s, 5);
  }
  if (s[0])
    strcat(s, "m");
  if (!okansi() && !forceit)
    s[0]=0;
}



void setfgc(int i)
/* This sets the foreground color to that passed.  It is called only from
 * execute_ansi
 */
{
  curatr = (curatr & 0xf8) | i;
}



void setbgc(int i)
/* This sets the background color to that passed.  It is called only from
 * execute_ansi
 */
{
  curatr = (curatr & 0x8f) | (i << 4);
}


void execute_ansi(void)
/* This function executes an ANSI string to change color, position the
 * cursor, etc.
 */
{
  int args[10], argptr, count, ptr, tempptr, ox, oy;
  char cmd, temp[10], *clrlst = "04261537";

  if (ansistr[1] != '[') {

    /* do nothing if invalid ANSI string. */

  } else {
    argptr = tempptr = 0;
    ptr = 2;
    for (count = 0; count < 10; count++)
      args[count] = temp[count] = 0;
    cmd = ansistr[ansiptr - 1];
    ansistr[ansiptr - 1] = 0;
    while (ansistr[ptr]) {
      if (ansistr[ptr] == ';') {
        temp[tempptr] = 0;
        tempptr = 0;
        args[argptr++] = atoi(temp);
      } else
        temp[tempptr++] = ansistr[ptr];
      ++ptr;
    }
    if (tempptr) {
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
          if (args[0] == 2)
            clrscrb();
          break;
        case 'k':
        case 'K':
          ox = WhereX();
          oy = WhereY();
          _CX = 80 - ox;
          _AH = 0x09;
          _BH = 0x00;
          _AL = 32;
          _BL = curatr;
          my_video_int();
          movecsr(ox, oy);
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



void outchr(char c)
/* This function outputs one character to the screen, and if output to the
 * com port is enabled, the character is output there too.  ANSI graphics
 * are also trapped here, and the ansi function is called to execute the
 * ANSI codes
 */
{
  int i, i1;

  if (change_color == 1) {
    change_color = 0;
    if ((c >= '0') && (c <= '9'))
      ansic(c - '0');
    return;
  } else if (change_color == 2) {
    // This byte is reserved for RIPscrip 2 font style commands & codes
    if (c == 15)
      change_color = 3;
    else
      change_color = 0;
    return;
  } else if (change_color == 3) {
    // Control or substitution codes
    change_color = 0;
    outstr(interpret(c));
    return;
  }

  if (c == 3) {
    change_color = 1;
    return;
  } else if (c == 15) {
    change_color = 2;
    return;
  }

  if ((c == 10) && endofline[0]) {
    if (!in_extern)
      outstr(endofline);
    endofline[0] = 0;
  }
  if (outcom && (c != 9))
    outcomch(echo ? c : 'X');
  if (ansiptr) {
    ansistr[ansiptr++] = c;
    ansistr[ansiptr]   = 0;
    if (((c > '@') && (c != '[')) || (ansistr[1] != '['))
      execute_ansi();
  } else if (c == 27) {
    ansistr[0] = 27;
    ansiptr = 1;
  } else if (c == 9) {
    i1 = WhereX();
    for (i = i1; i< (((i1 / 8) + 1) * 8); i++)
      outchr(32);
  } else if (echo || lecho) {
    out1ch(c);
    if (c == 10) {
      ++lines_listed;
      if ((sysstatus_pause_on_page & thisuser.sysstatus) &&
          (lines_listed >= screenlinest - 1)) {
        pausescr();
        lines_listed = 0;
      }
    }
  } else
    out1ch('X');
}



void outstr(char *s)
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
  outstr("\r\n");
}



void backspace(void)
/* This function executes a backspace, space, backspace sequence. */
{
  int i;

  i = echo;
  echo = 1;
  outstr("\b \b");
  echo = i;
}



void setc(unsigned char ch)
/* This sets the current color (both locally and remotely) to that
 * specified (in IBM format).
 */
{
  char s[30];

  makeansi(ch, s, 0);
  outstr(s);
}



void pausescr(void)
/* This will pause output, displaying the [PAUSE] message, and wait for
 * a key to be hit.
 */
{
  int i;

  if (okansi()) {
    i = curatr;
    setc((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[3] :
          thisuser.bwcolors[3]);
    outstr("[PAUSE]\x1b[7D");
    setc(i);
    getkey();
    outstr("       \x1b[7D");
  } else {
    outstr("[PAUSE]");
    getkey();
    for (i = 0; i < 7; i++)
      backspace();
  }
}



void pl(char *s)
{
  outstr(s);
  nl();
}


int kbhitb(void)
{
  union REGS r;

  r.h.ah = 1;
  int86(0x16, &r, &r);
  return((r.x.flags & 64) == 0);
}


int empty(void)
{
  if (kbhitb() || (incom && (head != tail)) ||
      (charbufferpointer && charbuffer[charbufferpointer]) ||
      (in_extern == 2))
    return(0);
  return(1);
}



void skey1(char *ch)
{
  char c;

  c = *ch;
  if (c == 127)
    c = 8;
    switch(c) {
      case 1:
      case 4:
      case 6:
        if (!charbufferpointer) {
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
    }
  *ch = c;
}

char getchd(void)
{
  union REGS r;

  r.h.ah = 0x07;
  int86(save_dos, &r, &r);
  return(r.h.al);
}


char getchd1(void)
{
  union REGS r;

  r.h.ah = 0x06;
  r.h.dl = 0xFF;
  int86(save_dos, &r, &r);
  return((r.x.flags & 0x40) ? 255 : r.h.al);
}


char inkey(void)
/* This function checks both the local keyboard, and the remote terminal
 * (if any) for input.  If there is input, the key is returned.  If there
 * is no input, a zero is returned.  Function keys hit are interpreted as
 * such within the routine and not returned.
 */
{
  char ch=0;

  if (charbufferpointer) {
    if (!charbuffer[charbufferpointer])
      charbufferpointer = charbuffer[0] = 0;
    else
      return(charbuffer[charbufferpointer++]);
  }
  if (kbhitb() || (in_extern == 2)) {
    ch = getchd1();
    if (!ch) {
      if (in_extern)
        in_extern = 2;
      else {
        ch = getchd1();
        ch=0;
      }
    } else if (in_extern)
      in_extern = 1;
    timelastchar1=timer1();
  } else if (incom && comhit()) {
    ch = (get1c() & andwith);
  }
  skey1(&ch);
  return(ch);
}



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



char upcase(char ch)
/* This converts a character to uppercase */
{
  if ((ch > '`') && (ch < '{'))
    ch = ch - 32;
  return(ch);
}


unsigned char getkey(void)
/* This function returns one character from either the local keyboard or
 * remote com port (if applicable).  After 1.5 minutes of inactivity, a
 * beep is sounded.  After 3 minutes of inactivity, the user is hung up.
 */
{
  unsigned char ch;
  int beepyet;
  long dd;

  beepyet = 0;
  timelastchar1=timer1();

  lines_listed = 0;
  do {
    while (empty() && !hangup) {
      dd = timer1();
      if (labs(dd - timelastchar1) > 65536L)
        timelastchar1 -= 1572480L;
      if (((dd - timelastchar1) > 1638L) && (!beepyet)) {
        beepyet = 1;
        outchr(7);
      }
      if (labs(dd - timelastchar1) > 3276L) {
        nl();
        outstr("Call back later when you are there.");
        nl();
        hangup = 1;
      }
      checkhangup();
    }
    ch = inkey();
  } while (!ch && !in_extern && !hangup);
  return(ch);
}



void input1(char *s, int maxlen, int lc, int crend)
/* This will input a line of data, maximum maxlen characters long, terminated
 * by a C/R.  if (lc) is non-zero, lowercase is allowed, otherwise all
 * characters are converted to uppercase.
 */
{
  int curpos=0, done=0;
  unsigned char ch;

  while (!done && !hangup) {
    ch = getkey();
    if (ch > 31) {
      if (curpos < maxlen) {
        if (!lc)
          ch = upcase(ch);
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
        case 24:
          while (curpos) {
            curpos--;
            backspace();
            if (s[curpos] == 26)
              backspace();
          }
          break;
      }
  }
  if (hangup)
    s[0] = 0;
}



void input(char *s, int len)
/* This will input an upper-case string */
{
  input1(s, len, 0, 1);
}



void inputl(char *s, int len)
/* This will input an upper or lowercase string of characters */
{
  input1(s, len, 1, 1);
}



int yn(void)
/* The keyboard is checked for either a Y, N, or C/R to be hit.  C/R is
 * assumed to be the same as a N.  Yes or No is output, and yn is set to
 * zero if No was returned, and yn() is non-zero if Y was hit.
 */
{
  char ch=0;

  ansic(1);
  while (!hangup && ((ch = upcase(getkey())) != 'Y') && (ch != 'N') && (ch != 13))
    ;
  outstr((ch == 'Y') ? "Yes" : "No");
  nl();
  return(ch == 'Y');
}



int ny(void)
/* This is the same as yn(), except C/R is assumed to be "Y" */
{
  char ch=0;

  ansic(1);
  while (!hangup && ((ch = upcase(getkey())) != 'Y') && (ch != 'N') && (ch != 13))
    ;
  outstr((ch == 'N') ? "No" : "Yes");
  nl();
  return((ch == 'Y') || (ch==13));
}


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


void prt(int i, char *s)
{
  ansic(i);
  outstr(s);
  ansic(0);
}

/****************************************************************************/
#pragma warn -par

void far interrupt inlii(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, char *s2, int i1, int i2)
{
  inli(s1,s2,i1,i2);
}

void far interrupt checkai(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int *i1, int *i2)
{
  checka(i1,i2);
}


void far interrupt plai(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int *i1)
{
  pla(s1,i1);
}


void far interrupt outchri(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char ch)
{
  outchr(ch);
}


void far interrupt outstri(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  outstr(s1);
}


void far interrupt nli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  nl();
}


void far interrupt pli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  pl(s1);
}


void far interrupt emptyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=empty();
}


void far interrupt inkeyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=(unsigned) empty();
}


void far interrupt getkeyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=(unsigned) getkey();
}


void far interrupt inputi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int i)
{
  input(s1,i);
}


void far interrupt inputli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1, int i)
{
  inputl(s1,i);
}


void far interrupt yni(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=yn();
}



void far interrupt nyi(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags)
{
  ax=ny();
}


void far interrupt ansici(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1)
{
  ansic(i1);
}


void far interrupt oneki(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           char *s1)
{
  ax=(unsigned) onek(s1);
}


void far interrupt prti(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1, char *s1)
{
  prt(i1,s1);
}



void far interrupt mpli(unsigned bp, unsigned di, unsigned si,
                           unsigned ds, unsigned es, unsigned dx,
                           unsigned cx, unsigned bx, unsigned ax,
                           unsigned ip, unsigned cs, unsigned flags,
                           int i1)
{
  mpl(i1);
}


#pragma warn +par

/****************************************************************************/

unsigned char getkeyext(void)
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


int do_it(char *cl)
{
  int i,i1,l;
  char s[160];
  char *ss[30];

  strcpy(s,cl);
  ss[0]=s;
  i=1;
  l=strlen(s);
  for (i1=1; i1<l; i1++)
    if (s[i1]==32) {
      s[i1]=0;
      ss[i++]=&(s[i1+1]);
    }
  ss[i]=NULL;
  funcs[20]=NULL;
  i=(spawnvpe(P_WAIT,ss[0],ss,xenviron) & 0x00ff);
  funcs[20]=NULL;
/*   spawnvp(P_WAIT,ss[0],ss); */
  return(i);
}



int do_remote(char *s, int ccc)
{
  int rc;
  char x[161];

  checkhangup();
  if (hangup)
    return(32767);
  strcpy(x,getenv("COMSPEC"));
  strcat(x," /C ");
  strcat(x,s);
  if (ccc)
    rc=do_it(x);
  else
    rc=do_it(s);
  chdir(cdir);
  setdisk(cdir[0]-'A');
  return(rc);
}


union REGS ca_r;
int ca_pause,ca_ctrl_c;
long ca_d1;


void checka2(void)
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
  if ((ca_ctrl_c) && (ctc)) {
    while (inkey()!=0)
      ;
    pl("^C");
    ca_r.x.ax=0x4c00;
    int86(save_dos,&ca_r,&ca_r);
  }
}




void outdosstr(char *s)
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



union REGS ni_r;
struct SREGS ni_s;
unsigned ni_n;
char ni_ch,ni_ch1,ni_ss[10],ni_ch2;
unsigned char *ni_st;

#define ST_SIZE 500
static unsigned short ni_stack[ST_SIZE];


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
        outstr("Exiting...");
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
    checkhangup();
    if (hangup) {
      if (hanguptime1<0L) {
        hanguptime1=timer1();
        if (funcs[20]) {
          outstr("Terminating...\r\n");
          ip=FP_OFF(funcs[20]);
          cs=FP_SEG(funcs[20]);
          funcs[20]=(void far *)36;
        } else {
          funcs[20]=(void far *)36;
          outstr("Aborting...\r\n");
          ni_r.x.ax=0x4c00;
          int86x(save_dos,&ni_r,&ni_r,&ni_s);
        }
      } else {
        if (labs(timer1()-hanguptime1)>36L) {
          hanguptime1=timer1();
          outstr("Aborting...\r\n");
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



int full_external(char *s, int ccc)
{
  int cy,cx,xxx;

  checkhangup();
  if (hangup)
    return(0);
  in_extern=1;
  hanguptime1=-1L;
  setvect(save_dos,getvect(INT_REAL_DOS));

  setvect(INT_REAL_DOS,newintr1);

  if ((screenlinest<=defscreenbottom) && (screenlinest>20)) {
    screenbottom=screenlinest-1;
    cy=WhereY();
    cx=WhereX();
    xxx=cy-screenbottom+topline;
    if (xxx>0) {
      SCROLL_UP(topline,defscreenbottom,xxx);
      movecsr(cx,screenbottom);
    }
  }

  do_remote(s,ccc);

  setvect(INT_REAL_DOS,getvect(save_dos));

  if (in_extern==2)
    getkey();
  in_extern=0;
  return(0);
}


/****************************************************************************/

#define READ(x) read(i,&(x),sizeof(x))

int init_r(void)
{
  int i;
  char s[81];

  for (i=0; i<25; i++)
    funcs[i]=NULL;
  funcs[0]=(void far *)inlii;
  funcs[1]=(void far *)checkai;
  funcs[2]=(void far *)plai;
  funcs[3]=(void far *)outchri;
  funcs[4]=(void far *)outstri;
  funcs[5]=(void far *)nli;
  funcs[8]=(void far *)pli;
  funcs[9]=(void far *)emptyi;
  funcs[10]=(void far *)inkeyi;
  funcs[11]=(void far *)getkeyi;
  funcs[12]=(void far *)inputi;
  funcs[13]=(void far *)inputli;
  funcs[14]=(void far *)yni;
  funcs[15]=(void far *)nyi;
  funcs[16]=(void far *)ansici;
  funcs[17]=(void far *)oneki;
  funcs[18]=(void far *)prti;
  funcs[19]=(void far *)mpli;

  if(instance > 1)
    sprintf(s,"STAT.%3.3d",instance);
  else
    sprintf(s,"STAT.WWV");

  i=sh_open1(s,O_RDONLY | O_BINARY);
  if (i<0)
    return(1);

  READ(incom);
  READ(outcom);
  using_modem=incom || outcom;
  READ(thisuser);
  READ(flow_control);
  READ(async_irq);
  READ(baud_rate);
  READ(base);
  READ(andwith);
  READ(ctc);
  READ(defscreenbottom);
  READ(ok_modem_stuff);
  READ(save_dos);

  sh_close(i);

  if (ok_modem_stuff)
    initport(0);
  return(0);
}

void get_dir(char *s, int be)
{
  strcpy(s,"X:\\");
  s[0]='A'+getdisk();
  getcurdir(0,&(s[3]));
  if (be) {
    if (s[strlen(s)-1]!='\\')
      strcat(s,"\\");
  }
}


void cd_to(char *s)
{
  char s1[81];
  int i,db;

  strcpy(s1,s);
  i=strlen(s1)-1;
  db=(s1[i]=='\\');
  if (i==0)
    db=0;
  if ((i==2) && (s1[1]==':'))
    db=0;
  if (db)
    s1[i]=0;
  chdir(s1);
  if (s[1]==':')
    setdisk(s[0]-'A');
}

void setup_stuff(void)
{
  char s[161], *ss;
  int i,i1;

  strcpy(ver_no1,"BBS=");
  strcat(ver_no1,wwiv_version);


  strcpy(s,getenv("PROMPT"));
  if (strncmp(s,"WWIV: ",5))
    strcpy(newprompt,"PROMPT=WWIV: ");
  else
    strcpy(newprompt,"PROMPT=");
  if (s[0])
    strcat(newprompt,s);
  else
    strcat(newprompt,"$P$G");

  ss=getenv("ORBIT_INSTANCE");
  if(ss) {
    instance=atoi(ss);
  } else {
    instance=1;
  }

  sprintf(ver_no2,"ORBIT_FP=%04.4X:%04.4X",FP_SEG(funcs), FP_OFF(funcs));


  i=i1=0;
  while (environ[i]!=NULL) {
    if (strncmp(environ[i],"PROMPT=",7)==0)
      xenviron[i1++]=newprompt;
    else {
      if (strncmp(environ[i],"BBS=",4) && (strncmp(environ[i],"ORBIT_FP=",9)))
        xenviron[i1++]=environ[i];
      }
    ++i;
  }
  if (!getenv("PROMPT"))
    xenviron[i1++]=newprompt;
  if (!getenv("PKNOFASTCHAR"))
    xenviron[i1++]="PKNOFASTCHAR=Y";
  xenviron[i1++]=ver_no1;
  xenviron[i1++]=ver_no2;
  xenviron[i1]=NULL;

}

/****************************************************************************/

void main(int argc, char *argv[])
{
  int i,i1;
  char s[250], s1[80], *ss;
  FILE *f;

  get_dir(cdir,0);

  setup_stuff();

  strcpy(s,argv[0]);
  ss=strrchr(s,'.');
  if (ss)
    *ss=0;
  sprintf(s1,"%s.%03.3d", s, instance);
  if ((f=fopen(s1,"r"))!=NULL) {
    strcpy(s, argv[0]);
    argv = (char **) malloc(100 * sizeof(char *));
    argv[0] = strdup(s);
    argc=1;
    fgets(s, sizeof(s)-1, f);
    fclose(f);
    ss=strtok(s," \t\r\n");
    while (ss) {
      argv[argc++] = strdup(ss);
      ss=strtok(NULL," \t\r\n");
    }
    argv[argc] = NULL;
  }

  i=atoi(argv[1]);
  ccc=atoi(argv[2]);


  if (argc>=3) {
    s[0]=0;
    for (i1=3; i1<argc; i1++) {
      strcat(s,argv[i1]);
      strcat(s," ");
    }
    if (i) {
      if (!init_r()) {
        if (incom || outcom)
          screenbottom=thisuser.screenlines-1;
        else
          screenbottom=defscreenbottom;
        screenlinest=screenbottom+1;
        full_external(s,ccc);
        if (ok_modem_stuff)
          closeport();
      } else
        pl("Couldn't find data");
    } else
      do_remote(s,ccc);
  }
  cd_to(cdir);
  if (save_dos)
    setvect(save_dos, NULL);
  exit(0);
}
         
/* Interprets a code from the Ctrl-P Ctrl-P sequence, translating it
   into the correct string. */
char *interpret(char c)
{
  static char s[80];

  switch (c) {
    case '%':                   // Time left today
      sprintf(s, "%f", thisuser.extratime);
      break;
    case '$':                   // File points
      sprintf(s, "%d", thisuser.filepoints);
      break;
    case '*':                   // User reg num
      sprintf(s, "%d", thisuser.wwiv_regnum);
      break;
    case '-':                   // A** points
      sprintf(s, "%d", thisuser.ass_pts);
      break;
    case '!':                   // Built-in pause
      pausescr();
      break;
    case 'A':                   // User's age
      sprintf(s, "%d", thisuser.age);
      break;
    case 'B':                   // User's birthday
      sprintf(s, "%02.2d/%02.2d/%02.2d", thisuser.month, thisuser.day, thisuser.year);
      break;
    case 'b':                   // Minutes in bank
      sprintf(s, "%u", thisuser.banktime);
      break;
    case 'C':                   // User's city
      strcpy(s, thisuser.city);
      break;
    case 'c':                   // User's country
      strcpy(s, thisuser.country);
      break;
    case 'D':                   // Files downloaded
      sprintf(s, "%u", thisuser.downloaded);
      break;
    case 'd':                   // User's DSL
      sprintf(s, "%d", thisuser.dsl);
      break;
    case 'E':                   // E-mails sent
      sprintf(s, "%u", thisuser.emailsent);
      break;
    case 'F':
      sprintf(s, "%u", thisuser.feedbacksent);
      break;
    case 'f':                   // First time user called
      strcpy(s, thisuser.firston);
      break;
    case 'G':                   // MessaGes read
      sprintf(s, "%lu", thisuser.msgread);
      break;
    case 'g':                   // Gold
      sprintf(s, "%f", thisuser.gold);
      break;
    case 'I':                   // User's call sIgn
      strcpy(s, thisuser.callsign);
      break;
    case 'i':                   // Illegal log-ons
      sprintf(s, "%u", thisuser.illegal);
      break;
    case 'K':                   // Kb uploaded
      sprintf(s, "%lu", thisuser.uk);
      break;
    case 'k':                   // Kb downloaded
      sprintf(s, "%lu", thisuser.dk);
      break;
    case 'L':                   // Last call
      strcpy(s, thisuser.laston);
      break;
    case 'l':                   // Number of logons
      sprintf(s, "%u", thisuser.logons);
      break;
    case 'M':                   // Mail waiting
      sprintf(s, "%d", thisuser.waiting);
      break;
    case 'm':                   // Messages posted
      sprintf(s, "%u", thisuser.msgpost);
      break;
    case 'N':                   // User's name
      strcpy(s, thisuser.name);
      break;
    case 'n':                   // Sysop's note
      strcpy(s, thisuser.note);
      break;
    case 'O':                   // Times on today
      sprintf(s, "%d", thisuser.ontoday);
      break;
    case 'o':                   // Time on today
      sprintf(s, "%f", thisuser.timeontoday);
      break;
    case 'p':                   // User's phone
      strcpy(s, thisuser.dataphone);
      break;
    case 'R':                   // User's real name
      strcpy(s, thisuser.realname);
      break;
    case 'r':                   // Last baud rate
      sprintf(s, "%d", thisuser.lastrate);
      break;
    case 'S':                   // User's SL
      sprintf(s, "%d", thisuser.sl);
      break;
    case 's':                   // User's street address
      strcpy(s, thisuser.street);
      break;
    case 'T':                   // User's sTate
      strcpy(s, thisuser.state);
      break;
    case 'U':                   // Files uploaded
      sprintf(s, "%u", thisuser.uploaded);
      break;
    case 'X':                   // User's sex
      sprintf(s, "%c", thisuser.sex);
      break;
    case 'Z':                   // User's zip code
      strcpy(s, thisuser.zipcode);
      break;
    default:
      return "";
  }
  return s;
}

