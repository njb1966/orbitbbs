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

#include <mem.h>
#include <conio.h>

#include "ripint.h"

#ifndef __OS2__
#define SCROLL_UP(t,b,l) \
  _CH=t;\
  _DH=b;\
  _BH=curatr;\
  _AL=l;\
  _CL=0;\
  _DL=79;\
  _AH=6;\
  my_video_int();
#endif


#define GLOBAL_SIZE 4096

static char *global_buf;
static int global_ptr;
static int wx=0;


#ifdef __OS2__
static unsigned char CellStr[3] = { ' ', 0, 0 };
#endif


void my_video_int(void)
{
#ifndef __OS2__
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
#endif
}


void set_global_handle(int i)
{
  char s[81];

  if (x_only)
    return;

  if (i) {
    if (!global_handle) {
      sprintf(s,"%sGLOBAL.TXT",syscfg.gfilesdir);
      global_handle=sh_open(s,O_RDWR | O_APPEND | O_BINARY | O_CREAT,
                           S_IREAD | S_IWRITE);
      global_ptr=0;
      global_buf=malloca(GLOBAL_SIZE);
      if ((global_handle<0) || (!global_buf)) {
        global_handle=0;
        if (global_buf) {
          bbsfree(global_buf);
          global_buf=NULL;
        }
      }

    }
  } else {
    if (global_handle) {
      sh_write(global_handle,global_buf,global_ptr);
      sh_close(global_handle);
      global_handle=0;
      if (global_buf) {
        bbsfree(global_buf);
        global_buf=NULL;
      }
    }
  }
}


void global_char(char ch)
{

  if (global_buf && global_handle) {
    global_buf[global_ptr++]=ch;
    if (global_ptr==GLOBAL_SIZE) {
      sh_write(global_handle,global_buf,global_ptr);
      global_ptr=0;
    }
  }
}

void set_x_only(int tf, char *fn, int ovwr)
{
  static int gh;
  char s[81];

  if (x_only) {
    if (!tf) {
      if (global_handle) {
        sh_write(global_handle,global_buf,global_ptr);
        sh_close(global_handle);
        global_handle=0;
        if (global_buf) {
          bbsfree(global_buf);
          global_buf=NULL;
        }
      }
      x_only=0;
      set_global_handle(gh);
      gh=0;
      express=expressabort=0;
    }
  } else {
    if (tf) {
      gh=global_handle;
      set_global_handle(0);
      x_only=1;
      wx=0;
      sprintf(s,"%s%s",syscfgovr.tempdir,fn);
      if (ovwr)
        global_handle=sh_open(s,O_RDWR | O_TRUNC | O_BINARY | O_CREAT,
                             S_IREAD | S_IWRITE);
      else
        global_handle=sh_open(s,O_RDWR | O_APPEND | O_BINARY | O_CREAT,
                             S_IREAD | S_IWRITE);
      global_ptr=0;
      express=1;
      expressabort=0;
      global_buf=malloca(GLOBAL_SIZE);
      if ((global_handle<0) || (!global_buf)) {
        global_handle=0;
        if (global_buf) {
          bbsfree(global_buf);
          global_buf=NULL;
        }
        set_x_only(0, NULL, 0);
      }
    }
  }
  timelastchar1=timer1();
}


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

  if (x_only) {
    wx=x;
    return;
  }

#ifdef __OS2__
  VioSetCurPos((USHORT)y, (USHORT)x, 0);
#else
  _BH=0x00;
  _DH=y;
  _DL=x;
  _AH=0x02;
  my_video_int();
#endif
}



int WhereX(void)
/* This function returns the current X cursor position, as the number of
 * characters from the left hand side of the screen.  An X position of zero
 * means the cursor is at the left-most position
 */
{
#ifdef __OS2__
  USHORT CurX, CurY;
#endif

  if (x_only)
    return(wx);


#ifdef __OS2__
  VioGetCurPos(&CurY, &CurX, 0);
  return CurX;
#else
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DL;
  return(tempio);
#endif
}



int WhereY(void)
/* This function returns the Y cursor position, as the line number from
 * the top of the logical window.  The offset due to the protected top
 * of the screen display is taken into account.  A WhereY() of zero means
 * the cursor is at the top-most position it can be at.
 */
{
#ifdef __OS2__
  USHORT CurX, CurY;

  VioGetCurPos(&CurY, &CurX, 0);
  return CurY - topline;
#else
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DH;
  return(tempio-topline);
#endif
}



void lf(void)
/* This function performs a linefeed to the screen (but not remotely) by
 * either moving the cursor down one line, or scrolling the logical screen
 * up one line.
 */
{
#ifdef __OS2__
  USHORT CurX, CurY;

  VioGetCurPos(&CurY, &CurX, 0);
  if (CurY >= screenbottom)
    {
      CellStr[1] = (unsigned char) curatr;
      VioScrollUp(topline, 0, screenbottom, 79, 1, CellStr, 0);
    }
  else
    VioSetCurPos(CurY + 1, CurX, 0);
#else
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  tempio=_DL;
  if (_DH>=screenbottom) {
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
#endif
}



void cr(void)
/* This short function returns the local cursor to the left-most position
 * on the screen.
 */
{
#ifdef __OS2__
  USHORT CurX, CurY;

  VioGetCurPos(&CurY, &CurX, 0);
  VioSetCurPos(CurY, 0, 0);
#else
  _BH=0x00;
  _AH=0x03;
  my_video_int();
  _DL=0x00;
  _AH=2;
  my_video_int();
#endif
}

void clrscrb(void)
/* This clears the local logical screen */
{
#ifdef __OS2__
  CellStr[1] = (unsigned char) curatr;
  VioScrollUp(topline, 0, screenbottom, 79, 65535, CellStr, 0);
#else
  SCROLL_UP(topline,screenbottom,0);
#endif
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
#ifdef __OS2__
  USHORT CurX, CurY;

  VioGetCurPos(&CurY, &CurX, 0);
  if (CurX != 0)
    VioSetCurPos(CurY, CurX - 1, 0);
  else
    if (CurY != topline)
      VioSetCurPos(CurY - 1, 79, 0);
#else
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
#endif
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
#ifdef __OS2__
  USHORT CurX, CurY;
  BYTE CharAttribute = curatr;

  VioGetCurPos(&CurY, &CurX, 0);
  VioWrtCharStrAtt(&ch, 1, CurY, CurX, &CharAttribute, 0);
  if (CurX != 79)
    VioSetCurPos(CurY, CurX + 1, 0);
  else
    if (CurY != screenbottom)
      VioSetCurPos(CurY + 1, 0, 0);
    else
      {
      CellStr[1] = (unsigned char) curatr;
      VioScrollUp(topline, 0, screenbottom, 79, 1, CellStr, 0);
      VioSetCurPos(CurY, 0, 0);
      }
#else
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
#endif
}




void out1ch(unsigned char ch)
/* This function outputs one character to the local screen.  C/R, L/F, TOF,
 * BS, and BELL are interpreted as commands instead of characters.
 */
{
  if (x_only) {
    if (ch>31) {
      wx=(wx+1)%80;
    } else if ((ch==13) || (ch==12)) {
      wx=0;
    } else if (ch==8) {
      if (wx)
        wx--;
    }
    return;
  }

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


void outs(unsigned char *s)
/* This (obviously) outputs a string TO THE SCREEN ONLY */
{
#ifdef __OS2__
  /* more efficient, but will it work with DOS segmented pointers? */
  while (*s)
    {
      out1ch(*s);
      s++;
    }
#else
  int i;
  unsigned char ch;

  for (i=0; s[i]!=0; i++) {
    ch=s[i];
    out1ch(ch);
  }
#endif
}

void outfast(char *s)
/* This RAPIDLY outputs ONE LINE to the screen only*/
{
#ifdef __OS2__
  USHORT CurX, CurY;
  BYTE CharAttribute = curatr;

  VioGetCurPos(&CurY, &CurX, 0);
  VioWrtCharStrAtt(s, strlen(s), CurY, CurX, &CharAttribute, 0);
#else
  int i,i1;

  i1 = (WhereY() * 80 + WhereX()) *2 ;
  for (i=0;s[i]!=0;i++) {
    scrn[i*2+i1+1]=curatr;
    scrn[i*2+i1]=s[i];
  }
  movecsr(WhereX()+strlen(s),WhereY());
#endif
}



void pr_Wait(int i1)
{
  int i, i2, i3;
  char *ss;

  ss=get_string(925);
  i2=i3=strlen(ss);
  for (i=0; i<i3; i++)
    if ((ss[i]==3) && (i2>1))
      i2-=2;

  if (i1) {
    if (okansi()) {
      i=curatr;
      setc((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[3] :
            thisuser.bwcolors[3]);
      outstr(ss);
      npr("\x1b[%dD",i2);
      setc(i);
    } else {
      outstr(ss);
    }
  } else {
    if (okansi()) {
      for (i=0; i<i2; i++)
        outchr(' ');
      npr("\x1b[%dD",i2);
    } else {
      for (i=0; i<i2; i++)
        backspace();
    }
  }
}



void set_protect(int l)
/* set_protect sets the number of lines protected at the top of the screen. */
{
  int sa;

  if (l!=topline) {
    if (l>topline) {
      if ((WhereY()+topline-l) < 0) {
#ifdef __OS2__
        CellStr[1] = (unsigned char) curatr;
        VioScrollDn(topline, 0, screenbottom+1, 79, l - topline, CellStr, 0);
#else
        _CH=topline;
        _DH=screenbottom+1;
        _AL=l-topline;
        _CL=0;
        _DL=79;
        _BH=0x07;
        _AH=7;
        my_video_int();
#endif
        movecsr(WhereX(),WhereY()+l-topline);
      } else {
        oldy += (topline-l);
      }
    } else {
      sa=curatr;
      curatr= ((thisuser.sysstatus & sysstatus_color) ? thisuser.colors[0] :
                thisuser.bwcolors[0]);
#ifdef __OS2__
      CellStr[1] = (unsigned char) curatr;
      VioScrollUp(l, 0, topline - 1, 79, 65535, CellStr, 0);
#else
      SCROLL_UP(l,topline-1,0);
#endif
      curatr=sa;
      oldy += (topline-l);
    }
  }
  topline=l;
  if (using_modem)
    screenlinest=thisuser.screenlines;
  else
    screenlinest=defscreenbottom+1-topline;
}


void savescreen(screentype *s)
{
#ifdef RIPDRIVE
  if (rd_on())
    rd_str("\1|1\x1b""0000$SAVE0$\r");
#endif
  if (!s->scrn1)
    s->scrn1=(char *)malloca(screenlen);

  if (s->scrn1)
#ifdef __OS2__
    {
      USHORT ScreenBufferSize = screenlen;

      VioReadCellStr(s->scrn1, &ScreenBufferSize, 0, 0, 0);
    }
#else
    memmove(s->scrn1,scrn,screenlen);
#endif

  s->x1=WhereX();
  s->y1=WhereY();
  s->topline1=topline;
  s->curatr1=curatr;
}


void restorescreen(screentype far *s)
/* restorescreen restores a screen previously saved with savescreen */
{
#ifdef RIPDRIVE
  if (rd_on())
    rd_str("\1|1\x1b""0000$RESTORE0$\r");
#endif
  if (s->scrn1) {
#ifdef __OS2__
    VioWrtCellStr(s->scrn1, screenlen, 0, 0, 0);
#else
    memmove(scrn,s->scrn1,screenlen);
#endif
    bbsfree(s->scrn1);
    s->scrn1=NULL;
  }
  topline=s->topline1;
  curatr=s->curatr1;
  movecsr(s->x1,s->y1);
}


void temp_cmd(char *s)
{
  int i;

  pr_Wait(1);
  savescreen(&screensave);
#ifdef RIPDRIVE
  if (rd_on())
    localrip_deactivate();
#endif
  i=topline;
  topline=0;
  curatr=0x07;
  clrscrb();
  extern_prog(s, EFLAG_TOPSCREEN|EFLAG_SHRINK);
#ifdef RIPDRIVE
  if (rd_on())
    localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
#endif
  restorescreen(&screensave);
  topline=i;
  pr_Wait(0);
}


char xlate[] = {
  'Q','W','E','R','T','Y','U','I','O','P',0,0,0,0,
  'A','S','D','F','G','H','J','K','L',0,0,0,0,0,
  'Z','X','C','V','B','N','M',
};

char scan_to_char(unsigned char ch)
{
  if ((ch>=16) && (ch<=50))
    return(xlate[ch-16]);
  else
    return(0);
}

void alt_key(unsigned char ch)
{
  char ch1;
  char *ss, *ss1,s[81],cmd[128];
  int f,l;

  cmd[0]=0;
  ch1=scan_to_char(ch);
  if (ch1) {
    sprintf(s,"%sMACROS.TXT",syscfg.datadir);
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if (f>0) {
      l=filelength(f);
      ss=malloca(l+10);
      if (ss) {
        sh_read(f,ss,l);
        sh_close(f);

        ss[l]=0;
        ss1=strtok(ss,"\r\n");
        while (ss1) {
          if (upcase(*ss1)==ch1) {
            strtok(ss1," \t");
            ss1=strtok(NULL,"\r\n");
            if (ss1 && (strlen(ss1)<128))
              strcpy(cmd,ss1);
            ss1=NULL;
          } else
            ss1=strtok(NULL,"\r\n");
        }
        bbsfree(ss);
      } else
        sh_close(f);
      if (cmd[0]) {
        if (cmd[0]=='@') {
          if (okmacro && okskey && (!charbufferpointer) && (cmd[1])) {
            for (l=strlen(cmd)-1; l>=0; l--) {
              if (cmd[l]=='{')
                cmd[l]='\r';
              strcpy(charbuffer,cmd);
              charbufferpointer=1;
            }
          }
        } else {
          temp_cmd(cmd);
        }
      }
    }
  }
}

void skey(char ch)
/* skey handles all f-keys and the like hit FROM THE KEYBOARD ONLY */
{
  int i,i1;

  if (((syscfg.sysconfig & sysconfig_no_local)==0) && (!in_extern)) {
    if (okskey) {
      if ((ch>=104) && (ch<=113)) {
        set_autoval(ch-104);
      } else {
        switch ((unsigned char) ch) {
          case 59: /* F1 */
            val_cur_user();
            break;
          case 60: /* F2 */
            topdata+=1;
            if (topdata==3)
              topdata=0;
#ifdef RIPDRIVE
            rd_coff();
#endif
            topscreen();
#ifdef RIPDRIVE
            rd_con();
#endif
            break;
          case 61: /* F3 */
            if (using_modem) {
              incom=(!incom);
              dump();
              tleft(0);
            }
            break;
          case 62: /* F4 */
            chatcall=0;
            topscreen();
            break;
          case 63: /* F5 */
            hangup=1;
            dtr(0);
            break;
          case 64: /* F6 */
            sysop_alert=!sysop_alert;
            tleft(0);
            break;
          case 65: /* F7 */
            thisuser.extratime-=5.0*60.0;
            tleft(0);
            break;
          case 66: /* F8 */
            thisuser.extratime+=5.0*60.0;
            tleft(0);
            break;
          case 67: /* F9 */
            if (thisuser.sl!=255) {
              if (actsl!=255)
                actsl=255;
              else
                reset_act_sl();
              changedsl();
              tleft(0);
            }
            break;
          case 68: /* F10 */
            if (chatting==0) {
              if (syscfg.sysconfig & sysconfig_2_way)
                chat1("",1);
              else
                chat1("",0);
            } else
              chatting=0;
            break;
          case 71: /* HOME */
            if (chatting==1) {
              if (chat_file)
                chat_file=0;
              else
                chat_file=1;
            }
            break;
          case 88: /* Shift-F5 */
            i1=(rand() % 20) + 10;
            for (i=0; i<i1; i++)
              outchr(rand() % 256);
            hangup=1;
            dtr(0);
            break;
          case 98: /* Ctrl-F5 */
            nl();
            pl(get_string(924));
            nl();
            hangup=1;
            dtr(0);
            break;
          case 103: /* Ctrl-F10 */
            if (chatting==0)
              chat1("",0);
            else
              chatting=0;
            break;
          case 84: /* Shift-F1 */
            set_global_handle(!global_handle);
            topscreen();
            break;
          case 93: /* Shift-F10 */
            temp_cmd(getenv("COMSPEC"));
            break;
#ifdef RIPDRIVE
          case 85: /* Shift-F2 */
            if (sysinfo.flags & OP_FLAGS_RIPDRIVE_ON) {
              sysinfo.flags &= ~OP_FLAGS_RIPDRIVE_ON;
              rd_disable();
            } else if (ripdrive) {
              sysinfo.flags |= OP_FLAGS_RIPDRIVE_ON;
              localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
              cleared = NEEDCLEAR;
            } else if (localrip_detect()) {
              sysinfo.flags |= OP_FLAGS_RIPDRIVE_ON;
              nl();
              pl(get_string(1641));
            }
            break;
#endif
          default:
            alt_key((unsigned char) ch);
            break;
        }
      }
    } else {
      if (wfc==1)
        holdphone(1);
      alt_key((unsigned char) ch);
      if (wfc==1) {
        cleanup_net();
        holdphone(0);
      }
    }
  }
}


void tleft(int x)
{
  int cx,cy,ctl,cc,ln,i;
  double nsln;
  char tl[30];
  static char sbuf[200];
  static char *ss[8];

  if (!sbuf[0]) {
    ss[0]=sbuf;
    for (i=0; i<7; i++) {
      strcpy(ss[i],get_stringx(1,85+i));
      ss[i+1]=ss[i]+strlen(ss[i])+1;
    }
  }

  cx=WhereX();
  cy=WhereY();
  ctl=topline;
  cc=curatr;
  if (crttype==7)
    curatr=0x07;
  else
    curatr=sysinfo.topscreen_color;
  topline=0;
  nsln=nsl();
  if (chatcall && (topdata==2))
    ln=5;
  else
    ln=4;


  if (topdata) {

    if ((using_modem) && (!incom)) {
      movecsr(1,ln);
      outs(ss[0]);
      for (i=19; i<strlen(curspeed); i++)
        out1ch('Í');
    } else {
      movecsr(1,ln);
      outs(curspeed);
      for (i=WhereX(); i<23; i++)
        out1ch('Í');
    }

    if ((thisuser.sl!=255) && (actsl==255)) {
      movecsr(23,ln);
      outs(ss[1]);
    }

    if (global_handle) {
      movecsr(40,ln);
      outs(ss[2]);
    }

    movecsr(54,ln);
    if (sysop_alert) {
      outs(ss[3]);
    } else {
      outs(ss[4]);
    }

    movecsr(64,ln);
    if (sysop1()) {
      outs(ss[5]);
    } else {
      outs(ss[6]);
    }
  }

  switch (topdata)
    {
    case 1:
      if (useron) {
        movecsr(18,3);
        sprintf(tl,"T-%6.2f",nsln/60.0);
        outs(tl);
      }
      break;
    case 2:
      movecsr(64,3);
      if (useron)
        sprintf(tl,"T-%6.2f",nsln/60.0);
      else
        strcpy(tl,thisuser.pw);
      outs(tl);
      break;
  }
  topline=ctl;
  curatr=cc;
  movecsr(cx,cy);
  if ((x) && (useron))
    if (nsln==0.0) {
      nl();
      pl(get_string(926));
      nl();
      hangup=1;
    }
}


void topscreen(void)
{
  int cc,cx,cy,ctl,i;
  char sl[81],ar[17],dar[17],restrict[17],rst[17],lo[90],ol[190],calls[20];


  switch(topdata) {
    case 0:
      set_protect(0);
      break;
    case 1:
      set_protect(5);
      break;
    case 2:
      if (chatcall)
        set_protect(6);
      else {
        if (topline==6)
          set_protect(0);
        set_protect(5);
      }
      break;
  }
  cx=WhereX();
  cy=WhereY();
  ctl=topline;
  cc=curatr;
  if (crttype==7)
    curatr=0x07;
  else
    curatr=sysinfo.topscreen_color;
  topline=0;
  for (i=0; i<80; i++)
    sl[i]=205;
  sl[80]=0;

  switch (topdata) {
    case 0:
      break;
    case 1:
      read_status();
      movecsr(0,0);
      sprintf(ol,"%-50s  Activity for %8s:      ",
          syscfg.systemname,status.date1);
      outs(ol);

      movecsr(0,1);
      sprintf(ol,"Users: %4u       Total Calls: %5lu      Calls Today: %4u    Posted      :%3u ",
          status.users,status.callernum1,status.callstoday,status.localposts);
      outs(ol);

      movecsr(0,2);
      sprintf(ol,"%-36s      %-4u min   /  %2u%%    E-mail sent :%3u ",
          nam(&thisuser,usernum),status.activetoday,
          (int) (10*status.activetoday/144),status.emailtoday);
      outs(ol);

      movecsr(0,3);
      sprintf(ol,"SL=%3u   DL=%3u               FW=%3u      Uploaded:%2u files    Feedback    :%3u ",
          thisuser.sl,thisuser.dsl,fwaiting,status.uptoday,status.fbacktoday);
      outs(ol);
      break;

    case 2:

      strcpy(rst,restrict_string);
      for (i=0; i<=15; i++) {
        if (thisuser.ar & (1 << i))
          ar[i]='A'+i;
        else
          ar[i]=32;
        if (thisuser.dar & (1 << i))
          dar[i]='A'+i;
        else
          dar[i]=32;
        if (thisuser.restrict & (1 << i))
          restrict[i]=rst[i];
        else
          restrict[i]=32;
      }
      dar[16]=0;
      ar[16]=0;
      restrict[16]=0;
      if (strcmp(thisuser.laston,date()))
        strcpy(lo,thisuser.laston);
      else
        sprintf(lo,"Today:%2d",thisuser.ontoday);

      movecsr(0,0);
      sprintf(ol,"%-35s W=%3u UL=%4u/%6lu SL=%3u LO=%5u PO=%4u",
          nam(&thisuser,usernum),thisuser.waiting,thisuser.uploaded,
          thisuser.uk,thisuser.sl,thisuser.logons,thisuser.msgpost);
      outs(ol);

      movecsr(0,1);
      if (thisuser.wwiv_regnum)
        sprintf(calls,"%lu",thisuser.wwiv_regnum);
      else
        strcpy(calls,thisuser.callsign);
      sprintf(ol,"%-20s %12s  %-6s DL=%4u/%6lu DL=%3u TO=%5.0lu ES=%4u",
          thisuser.realname,thisuser.phone,calls,
          thisuser.downloaded,thisuser.dk,thisuser.dsl,
          ((long) ((thisuser.timeon+timer()-timeon)/60.0)),
          thisuser.emailsent+thisuser.emailnet);
      outs(ol);

      movecsr(0,2);
      sprintf(ol,"ARs=%-16s/%-16s R=%-16s EX=%3u %-8s FS=%4u",
          ar,dar,restrict,thisuser.exempt,lo,thisuser.feedbacksent);
      outs(ol);

      movecsr(0,3);
      sprintf(ol,"%-40.40s %c %2u %-17s          FW= %3u",
          thisuser.note,thisuser.sex,thisuser.age,
          ctypes[thisuser.comp_type],fwaiting);
      outs(ol);

      if (chatcall) {
        movecsr(0,4);
        outs(chatreason);
      }
      break;
  }
  if (ctl!=0) {
    movecsr(0,ctl-1);
    outs(sl);
  }
  topline=ctl;
  movecsr(cx,cy);
  curatr=cc;
  tleft(0);
}

void set_autoval(int n)
{
  auto_val(n, &thisuser);
  reset_act_sl();
  changedsl();
}
