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

/****************************************************************************/

/*
 * makewindow makes a "shadowized" window with the upper-left hand corner at
 * (x,y), and the lower-right corner at (x+xlen,y+ylen).
 */

void makewindow(int x, int y, int xlen, int ylen)
{
  int i,xx,yy;
  unsigned char s[81];

  if (xlen>80)
    xlen=80;
  if (ylen>(screenbottom+1-topline))
    ylen=(screenbottom+1-topline);
  if ((x+xlen)>80)
    x=80-xlen;
  if ((y+ylen)>screenbottom+1)
    y=screenbottom+1-ylen;

  xx=WhereX();
  yy=WhereY();
  for (i=1; i<xlen-1; i++)
    s[i]=196;
  s[0]=218;
  s[xlen-1]=191;
  s[xlen]=0;
  movecsr(x,y);
  outs(s);
  s[0]=192;
  s[xlen-1]=217;
  movecsr(x,y+ylen-1);
  outs(s);
  for (i=1; i<xlen-1; i++)
    s[i]=32;
  s[0]=179;
  s[xlen-1]=179;
  for (i=1; i<ylen-1; i++) {
    movecsr(x,i+y);
    outs(s);
  }
  for (i=0; i<xlen; i++)
    set_attr_xy(x+1+i, y+ylen, 0x08);
  for (i=0; i<ylen; i++)
    set_attr_xy(x+xlen,y+1+i, 0x08);
  movecsr(xx,yy);
}

/****************************************************************************/

/*
 * Edits a string, doing local screen I/O only.
 */

void editline(unsigned char *s, int len, int status, int *returncode, unsigned char *ss)
{
  int i,oldatr,cx,cy,pos,ch,done,insert,i1;

  oldatr=curatr;
  cx=WhereX();
  cy=WhereY();
  for (i=strlen(s); i<len; i++)
    s[i]=32;
  s[len]=0;
  curatr=sysinfo.editline_color;
  outs(s);
  movecsr(cx,cy);
  done=0;
  pos=0;
  insert=0;
  do {
    ch=getchd();
    if (ch==0) {
      ch=getchd();
      switch (ch) {
        case 59:
          done=1;
          *returncode=DONE;
          break;
        case 71: pos=0; movecsr(cx,cy); break;
        case 79: pos=len; movecsr(cx+pos,cy); break;
        case 77: if (pos<len) {
            pos++;
            movecsr(cx+pos,cy);
          }
          break;
        case 75: if (pos>0) {
            pos--;
            movecsr(cx+pos,cy);
          }
          break;
        case 72:
        case 15:
          done=1;
          *returncode=PREV;
          break;
        case 80:
          done=1;
          *returncode=NEXT;
          break;
        case 82:
          if (status!=SET) {
            if (insert)
              insert=0;
            else
              insert=1;
          }
          break;
        case 83:
          if (status!=SET) {
            for (i=pos; i<len; i++)
              s[i]=s[i+1];
            s[len-1]=32;
            movecsr(cx,cy);
            outs(s);
            movecsr(cx+pos,cy);
          }
          break;
      }
    } else {
      if (ch>31) {
        if (status==UPPER_ONLY)
          ch=upcase(ch);
        if (status==SET) {
          ch=upcase(ch);
          if (ch!=' ') {
            i1=1;
            for (i=0; i<len; i++)
              if ((ch==ss[i]) && (i1)) {
                i1=0;
                pos=i;
                movecsr(cx+pos,cy);
                if (s[pos]==' ')
                  ch=ss[pos];
                else
                  ch=' ';
              }
            if (i1)
              ch=ss[pos];
          }
        }
        if ((pos<len)&&((status==ALL) || (status==UPPER_ONLY) || (status==SET) ||
            ((status==NUM_ONLY) && (((ch>='0') && (ch<='9')) || (ch==' '))))) {
          if (insert) {
            for (i=len-1; i>pos; i--)
              s[i]=s[i-1];
            s[pos++]=ch;
            movecsr(cx,cy);
            outs(s);
            movecsr(cx+pos,cy);
          } else {
            s[pos++]=ch;
            out1ch(ch);
          }
        }
      } else {
        ch=ch;
        switch(ch) {
          case 13:
          case 9:
            done=1;
            *returncode=NEXT;
            break;
          case 27:
            done=1;
            *returncode=ABORTED;
            break;
          case 8:
            if (pos>0) {
              if (insert) {
                for (i=pos-1; i<len; i++)
                  s[i]=s[i+1];
                s[len-1]=32;
                pos--;
                movecsr(cx,cy);
                outs(s);
                movecsr(cx+pos,cy);
              } else {
                pos--;
                movecsr(cx+pos,cy);
              }
            }
            break;
        }
      }
    }
  } while (done==0);
  movecsr(cx,cy);
  curatr=oldatr;
  outs(s);
  movecsr(cx,cy);
}

/****************************************************************************/

/*
 * Allows local-only editing of some of the user data in a shadowized
 * window.
 */

#define utoa(s,v,r) ultoa((unsigned long)(s),v,r)
void val_cur_user(void)
{
  char sl[4],dsl[4],exempt[4],sysopsub[4],ar[17],dar[17],restrict[17],
       rst[17],tl[81],uk[8],dk[8],up[6],down[6],posts[6],banktime[6],
       gold[6],ass[6],logons[6];
  int cp,i,done,rc,wx,wy;

  pr_Wait(1);
  savescreen(&screensave);
#ifdef RIPDRIVE
  rd_coff();
#endif
  curatr=sysinfo.f1_color;
  wx=5;
  wy=3;
  makewindow(wx,wy,70,14);
  sprintf(tl,"Ã%s´", charstr(70-wx+3,'Ä'));
  movecsr(wx,wy+4);  outs(tl);
  movecsr(wx,wy+7);  outs(tl);
  movecsr(wx,wy+11); outs(tl);
  utoa(thisuser.sl,sl,10);
  utoa(thisuser.dsl,dsl,10);
  utoa(thisuser.exempt,exempt,10);
  if (*qsc>999)
    *qsc=999;
  utoa(*qsc,sysopsub,10);
  ultoa((unsigned long)thisuser.uk,uk,10);
  ultoa((unsigned long)thisuser.dk,dk,10);
  utoa(thisuser.uploaded,up,10);
  utoa(thisuser.downloaded,down,10);
  utoa(thisuser.msgpost,posts,10);
  utoa(thisuser.banktime,banktime,10);
  utoa(thisuser.logons,logons,10);
  utoa(thisuser.ass_pts,ass,10);
  gcvt((float)thisuser.gold,5,gold);
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
  cp=0;
  done=0;
  movecsr(wx+2,wy+1); sprintf(tl,get_stringx(1,77),sl); outs(tl);
  movecsr(wx+36,wy+1); sprintf(tl,get_stringx(1,78),ar); outs(tl);
  movecsr(wx+2,wy+2); sprintf(tl,get_stringx(1,79),dsl); outs(tl);
  movecsr(wx+36,wy+2); sprintf(tl,get_stringx(1,80),dar); outs(tl);
  movecsr(wx+2,wy+3); sprintf(tl,get_stringx(1,81),exempt); outs(tl);
  movecsr(wx+36,wy+3); sprintf(tl,get_stringx(1,82),restrict); outs(tl);
  movecsr(wx+2,wy+5); sprintf(tl,get_stringx(1,83),sysopsub); outs(tl);
  movecsr(wx+36,wy+5); sprintf(tl,get_stringx(1,103),banktime); outs(tl);
  movecsr(wx+2,wy+6); sprintf(tl,get_stringx(1,110),ass); outs(tl);
  movecsr(wx+36,wy+6); sprintf(tl,get_stringx(1,111),gold); outs(tl);
  movecsr(wx+2,wy+8); sprintf(tl,get_stringx(1,104),uk); outs(tl);
  movecsr(wx+36,wy+8); sprintf(tl,get_stringx(1,105),dk); outs(tl);
  movecsr(wx+2,wy+9); sprintf(tl,get_stringx(1,106),up); outs(tl);
  movecsr(wx+36,wy+9); sprintf(tl,get_stringx(1,107),down); outs(tl);
  movecsr(wx+2,wy+10); sprintf(tl,get_stringx(1,108),posts); outs(tl);
  movecsr(wx+36,wy+10); sprintf(tl,get_stringx(1,109),logons); outs(tl);
  movecsr(wx+2,wy+12); sprintf(tl,get_stringx(1,84),thisuser.note); outs(tl);
#ifdef RIPDRIVE
  rd_con();
#endif
  while (done==0) {
    switch(cp) {
      case 0:
        movecsr(wx+22,wy+1);
        editline(sl,3,NUM_ONLY,&rc,"");
        thisuser.sl=(char) atoi(sl);
        utoa(thisuser.sl,sl,10);
        sprintf(tl,"%-3s",sl); outs(tl);
        break;
      case 1:
        movecsr(wx+50,wy+1);
        editline(ar,16,SET,&rc,"ABCDEFGHIJKLMNOP ");
        thisuser.ar=0;
        for (i=0; i<=15; i++)
          if (ar[i]!=32)
            thisuser.ar |= (1 << i);
        break;
      case 2:
        movecsr(wx+22,wy+2);
        editline(dsl,3,NUM_ONLY,&rc,"");
        thisuser.dsl=(char) atoi(dsl);
        utoa(thisuser.dsl,dsl,10);
        sprintf(tl,"%-3s",dsl); outs(tl);
        break;
      case 3:
        movecsr(wx+50,wy+2);
        editline(dar,16,SET,&rc,"ABCDEFGHIJKLMNOP ");
        thisuser.dar=0;
        for (i=0; i<=15; i++)
          if (dar[i]!=32)
            thisuser.dar |= (1 << i);
        break;
      case 4:
        movecsr(wx+22,wy+3);
        editline(exempt,3,NUM_ONLY,&rc,"");
        thisuser.exempt=(char) atoi(exempt);
        utoa(thisuser.exempt,exempt,10);
        sprintf(tl,"%-3s",exempt); outs(tl);
        break;
      case 5:
        movecsr(wx+50,wy+3);
        editline(restrict,16,SET,&rc,rst);
        thisuser.restrict=0;
        for (i=0; i<=15; i++)
          if (restrict[i]!=32)
            thisuser.restrict |= (1 << i);
        break;
      case 6:
        movecsr(wx+14,wy+5);
        editline(sysopsub,3,NUM_ONLY,&rc,"");
        *qsc= atoi(sysopsub);
        utoa(*qsc,sysopsub,10);
        sprintf(tl,"%-3s",sysopsub); outs(tl);
        break;
      case 7:
        movecsr(wx+49,wy+5);
        editline(banktime,5,NUM_ONLY,&rc,"");
        thisuser.banktime=(unsigned int)atoi(banktime);
        utoa(thisuser.banktime,banktime,10);
        sprintf(tl,"%-5s",banktime); outs(tl);
        break;
      case 8:
        movecsr(wx+14,wy+6);
        editline(ass,5,NUM_ONLY,&rc,"");
        thisuser.ass_pts=(unsigned int)atoi(ass);
        utoa(thisuser.ass_pts,ass,10);
        sprintf(tl,"%-5s",ass); outs(tl);
        break;
      case 9:
        movecsr(wx+49,wy+6);
        editline(gold,5,NUM_ONLY,&rc,"");
        thisuser.gold=(float) atof(gold);
        gcvt((float)thisuser.gold,5,gold);
        sprintf(tl,"%-5s",gold); outs(tl);
        break;
      case 10:
        movecsr(wx+20,wy+8);
        editline(uk,7,NUM_ONLY,&rc,"");
        thisuser.uk=(unsigned long) atol(uk);
        ultoa((unsigned long)thisuser.uk,uk,10);
        sprintf(tl,"%-7s",uk); outs(tl);
        break;
      case 11:
        movecsr(wx+54,wy+8);
        editline(dk,7,NUM_ONLY,&rc,"");
        thisuser.dk=(unsigned long) atol(dk);
        ultoa((unsigned long)thisuser.dk,dk,10);
        sprintf(tl,"%-7s",dk); outs(tl);
        break;
      case 12:
        movecsr(wx+20,wy+9);
        editline(up,5,NUM_ONLY,&rc,"");
        thisuser.uploaded=(unsigned int) atoi(up);
        utoa(thisuser.uploaded,up,10);
        sprintf(tl,"%-5s",up); outs(tl);
        break;
      case 13:
        movecsr(wx+54,wy+9);
        editline(down,5,NUM_ONLY,&rc,"");
        thisuser.downloaded=(unsigned int) atoi(down);
        utoa(thisuser.downloaded,down,10);
        sprintf(tl,"%-5s",down); outs(tl);
        break;
      case 14:
        movecsr(wx+20,wy+10);
        editline(posts,5,NUM_ONLY,&rc,"");
        thisuser.msgpost=(unsigned int) atoi(posts);
        utoa(thisuser.msgpost,posts,10);
        sprintf(tl,"%-5s",posts); outs(tl);
        break;
      case 15:
        movecsr(wx+54,wy+10);
        editline(logons,5,NUM_ONLY,&rc,"");
        thisuser.logons=(unsigned int) atoi(logons);
        utoa(thisuser.logons,logons,10);
        sprintf(tl,"%-5s",logons); outs(tl);
        break;
      case 16:
        movecsr(wx+8,wy+12);
        editline(thisuser.note,sizeof(thisuser.note)-1,ALL,&rc,"");
        trimstr(thisuser.note);
        break;
    }
    switch(rc) {
      case ABORTED: done=1; break;
      case DONE: done=1; break;
      case NEXT: cp=(cp+1) % 17; break;
      case PREV: cp--; if (cp==-1) cp=16;  break;
    }
  }
  restorescreen(&screensave);
  reset_act_sl();
  changedsl();
  pr_Wait(0);
}

/****************************************************************************/

/*
 * Reads a char and attribute at screen pos x,y. Returns char located at
 * that position in *c, and the attribute of that character in *a.
 */

void read_char_xy(int x, int y, int *c, int *a)
{
#ifdef __OS2__
  unsigned char s[3];
  USHORT BufLen = sizeof(s);

  VioReadCellStr(s, &BufLen, y + topline, x, 0);
  *c = (int) s[0];
  *a = (int) s[1];
#else
  union REGS r;
  int xx,yy;

  xx=WhereX(); yy=WhereY();
  movecsr(x,y);
  *c=0; *a=0;
  r.h.ah = 0x08;
  r.h.bh = 0;
  int86(0x10, &r, &r);
  *c=r.h.al;
  *a=r.h.ah;
  movecsr(xx,yy);
#endif
}

/****************************************************************************/

/*
 * Sets screen attribute at screen pos x,y to attribute contained in a.
 */

void set_attr_xy(int x, int y, int a)
{
#ifdef __OS2__
  BYTE Attrib = a;

  VioWrtNAttr(&Attrib, 1, y + topline, x, 0);
#else
  union REGS r;
  int xx,yy,oc,oa;

  xx=WhereX(); yy=WhereY();
  read_char_xy(x,y,&oc,&oa);
  movecsr(x,y);
  r.h.ah=0x09;
  r.h.al=(unsigned char)(oc);
  r.h.bh=0;
  r.h.bl=(unsigned char)(a);
  r.x.cx=1;
  int86(0x10, &r, &r);
  movecsr(xx,yy);
#endif
}

/****************************************************************************/

/*
 * Allows selection of a name to "chat as". Returns selected string in *s.
 */

void select_chat_name(unsigned char *s)
{
  int un,rc;
  userrec tu;

  pr_Wait(1);
  savescreen(&screensave);
  strcpy(s,syscfg.sysopname);
  curatr=sysinfo.chat_select_color;
  makewindow(20,5,43,3);
  movecsr(22,6);
  outs(get_string(1009));
  movecsr(31,6);
  curatr=sysinfo.editline_color;
  outs(charstr(30,32));
  movecsr(31,6);
  editline(s,30,ALL,&rc,s);
  if (rc!=ABORTED) {
    trimstr(s);
    un=(unsigned short) atoi(s);
    if ((un>0) && (un<=syscfg.maxusers)) {
      read_user(un,&tu);
      strcpy(s,nam(&tu,un));
    } else {
      if (!s[0])
        strcpy(s,syscfg.sysopname);
    }
  } else
    strcpy(s,"");
  restorescreen(&screensave);
  pr_Wait(0);
}

/****************************************************************************/

int x1,y1,x2,y2,cp0,cp1;

/*
 * Allows two-way chatting until sysop aborts/exits chat.
 */

#define MAXLEN 160
#define MAXLINES_SIDE 13

static char (*side0)[MAXLEN], (*side1)[MAXLEN];

void two_way_chat(char *s, char *rollover, int maxlen, int crend, unsigned char *sysopname)
{
  char s2[100],temp1[100];
  int side,cnt,cntr;
  int i,i1,done,cm,begx;
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

  done=0;
  side = 0;
  do {
    ch=getkey();
      if (lastcon) {
        if (WhereY() == 11) {
          outstr("\x1b[12;1H");
          ansic(3);
          for (cnt=0;cnt<thisuser.screenchars;cnt++)
            s2[cnt]=205;
          sprintf(temp1,get_stringx(1,93),sysopname,nam(&thisuser,usernum));
          cnt=(((thisuser.screenchars-strlen(stripcolors(temp1)))/2));
          if (cnt)
            strncpy(&s2[cnt-1],temp1,(strlen(temp1)));
          else
            strcpy(s2,charstr(205,thisuser.screenchars-1));
          s2[thisuser.screenchars]=0;
          outstr(s2);
          s2[0]=0;
          temp1[0]=0;
          for (cntr=1;cntr<12;cntr++) {
            sprintf(s2,"\x1b[%d;%dH",cntr,1);
            outstr(s2);
            if ((cntr>=0) && (cntr<5)) {
              ansic(1);
              outstr(side0[cntr+6]);
            }
            outstr("\x1b[K");
            s2[0]=0;
          }
          sprintf(s2,"\x1b[%d;%dH",5,1);
          outstr(s2);
          s2[0]=0;
        } else
          if (WhereY()>11) {
            x2=(WhereX()+1);
            y2=(WhereY()+1);
            sprintf(s2,"\x1b[%d;%dH",y1,x1);
            outstr(s2);
            s2[0]=0;
          }
        side = 0;
        ansic(1);
      } else {
        if (WhereY()>=23) {
          for (cntr=13;cntr<25;cntr++) {
            sprintf(s2,"\x1b[%d;%dH",cntr,1);
            outstr(s2);
            if ((cntr>=13) && (cntr<17)) {
              ansic(5);
              outstr(side1[cntr-7]);
            }
            outstr("\x1b[K");
            s2[0]=0;
          }
          sprintf(s2,"\x1b[%d;%dH",17,1);
          outstr(s2);
          s2[0]=0;
        } else
          if ((WhereY()<12) && (side==0)) {
            x1=(WhereX()+1);
            y1=(WhereY()+1);
            sprintf(s2,"\x1b[%d;%dH",y2,x2);
            outstr(s2);
            s2[0]=0;
          }
        side=1;
        ansic(5);
      }
      if (cm)
        if (chatting==0)
          ch=13;
      if ((ch>=32)) {
        if (side==0) {
          if ((WhereX()<(thisuser.screenchars-1)) && (cp0<maxlen)) {
            if (WhereY()<11) {
              side0[WhereY()][cp0++]=ch;
              outchr(ch);
            } else {
              side0[WhereY()][cp0++]=ch;
              side0[WhereY()][cp0]=0;
              for (cntr=0;cntr<12;cntr++) {
                sprintf(s2,"\x1b[%d;%dH",cntr,1);
                outstr(s2);
                if ((cntr>=0) && (cntr<6)) {
                  ansic(1);
                  outstr(side0[cntr+6]);
                  y1=WhereY()+1;
                  x1=WhereX()+1;
                }
                outstr("\x1b[K");
                s2[0]=0;
              }
              sprintf(s2,"\x1b[%d;%dH",y1,x1);
              outstr(s2);
              s2[0]=0;
            }
            if (WhereX()==(thisuser.screenchars-1))
              done=1;
          } else {
            if (WhereX()>=(thisuser.screenchars-1))
              done=1;
          }
        } else {
          if ((WhereX()<(thisuser.screenchars-1)) && (cp1<maxlen)) {
            if (WhereY()<23) {
              side1[WhereY()-13][cp1++]=ch;
              outchr(ch);
            } else {
              side1[WhereY()-13][cp1++]=ch;
              side1[WhereY()-13][cp1]=0;
              for (cntr=13;cntr<25;cntr++) {
                sprintf(s2,"\x1b[%d;%dH",cntr,1);
                outstr(s2);
                if ((cntr>=13) && (cntr<18)) {
                  ansic(5);
                  outstr(side1[cntr-7]);
                  y2=WhereY()+1;
                  x2=WhereX()+1;
                }
                outstr("\x1b[K");
                s2[0]=0;
              }
              sprintf(s2,"\x1b[%d;%dH",y2,x2);
              outstr(s2);
              s2[0]=0;
            }
            if (WhereX()==(thisuser.screenchars-1))
              done=1;
          } else {
            if (WhereX()>=(thisuser.screenchars-1))
              done=1;
          }
        }
      } else
        switch(ch) {
          case 7:
            if ((chatting) && (outcom))
              outcomch(7);
            break;
          case 13: /* C/R */
            if (side == 0)
              side0[WhereY()][cp0]=0;
            else
              side1[WhereY()-13][cp1]=0;
            done=1;
            break;
          case 8:  /* Backspace */
            if (side==0) {
              if (cp0) {
                if (side0[WhereY()][cp0-2]==3) {
                  cp0-=2;
                  ansic(0);
                } else
                  if (side0[WhereY()][cp0-1]==8) {
                    cp0--;
                    outchr(32);
                  } else {
                    cp0--;
                    backspace();
                  }
              }
            } else
              if (cp1) {
                if (side1[WhereY()-13][cp1-2]==3) {
                  cp1-=2;
                  ansic(0);
                } else
                  if (side1[WhereY()-13][cp1-1]==8) {
                    cp1--;
                    outchr(32);
                  } else {
                    cp1--;
                    backspace();
                  }
              }
            break;
          case 24: /* Ctrl-X */
            while (WhereX()>begx) {
              backspace();
              if (side==0)
                cp0=0;
              else
                cp1=0;
            }
            ansic(0);
            break;
          case 23: /* Ctrl-W */
            if (side==0) {
              if (cp0) {
                do {
                  if (side0[WhereY()][cp0-2]==3) {
                    cp0-=2;
                    ansic(0);
                  } else
                    if (side0[WhereY()][cp0-1]==8) {
                      cp0--;
                      outchr(32);
                    } else {
                      cp0--;
                      backspace();
                    }
                } while ((cp0) && (side0[WhereY()][cp0-1]!=32) &&
                       (side0[WhereY()][cp0-1]!=8) &&
                       (side0[WhereY()][cp0-2]!=3));
              }
            } else {
              if (cp1) {
                do {
                  if (side1[WhereY()-13][cp1-2]==3) {
                    cp1-=2;
                    ansic(0);
                  } else
                    if (side1[WhereY()-13][cp1-1]==8) {
                      cp1--;
                      outchr(32);
                    } else {
                      cp1--;
                      backspace();
                    }
                } while ((cp1) && (side1[WhereY()-13][cp1-1]!=32) &&
                       (side1[WhereY()-13][cp1-1]!=8) &&
                       (side1[WhereY()-13][cp1-2]));
              }
            }
            break;
          case 14: /* Ctrl-N */
            if (side == 0) {
              if ((WhereX()) && (cp0<maxlen)) {
                outchr(8);
                side0[WhereY()][cp0++]=8;
              }
            } else
              if ((WhereX()) && (cp1<maxlen)) {
                outchr(8);
                side1[WhereY()-13][cp1++]=8;
              }
            break;
          case 16: /* Ctrl-P */
            if (side==0) {
              if (cp0<maxlen-1) {
                ch=getkey();
                if ((ch>='0') && (ch<='9')) {
                  side0[WhereY()][cp0++]=3;
                  side0[WhereY()][cp0++]=ch;
                  ansic(ch-'0');
                }
              }
            } else {
              if (cp1<maxlen-1) {
                ch=getkey();
                if ((ch>='0') && (ch<='9')) {
                  side1[WhereY()-13][cp1++]=3;
                  side1[WhereY()-13][cp1++]=ch;
                  ansic(ch-'0');
                }
              }
            }
            break;
          case 9:  /* Tab */
            if (side==0) {
              i=5-(cp0 % 5);
              if (((cp0+i)<maxlen) && ((WhereX()+i)<thisuser.screenchars)) {
                i=5-((WhereX()+1) % 5);
                for (i1=0; i1<i; i1++) {
                  side0[WhereY()][cp0++]=32;
                  outchr(32);
                }
              }
            } else {
              i=5-(cp1 % 5);
              if (((cp1+i)<maxlen) && ((WhereX()+i)<thisuser.screenchars)) {
                i=5-((WhereX()+1) % 5);
                for (i1=0; i1<i; i1++) {
                  side1[WhereY()-13][cp1++]=32;
                  outchr(32);
                }
              }
            }
            break;
        }
  } while ((done==0) && (hangup==0));

  if (ch!=13) {
    if (side==0) {
      i=cp0-1;
      while ((i>0) && (side0[WhereY()][i]!=32) &&
             (side0[WhereY()][i]!=8) || (side0[WhereY()][i-1]==3))
        i--;
      if ((i>(WhereX()/2)) && (i!=(cp0-1))) {
        i1=cp0-i-1;
        for (i=0; i<i1; i++)
          outchr(8);
        for (i=0; i<i1; i++)
          outchr(32);
        for (i=0; i<i1; i++)
          rollover[i]=side0[WhereY()][cp0-i1+i];
        rollover[i1]=0;
        cp0 -= i1;
      }
      side0[WhereY()][cp0]=0;
    } else {
      i=cp1-1;
      while ((i>0) && (side1[WhereY()-13][i]!=32) &&
             (side1[WhereY()-13][i]!=8) || (side1[WhereY()-13][i-1]==3))
        i--;
      if ((i>(WhereX()/2)) && (i!=(cp1-1))) {
        i1=cp1-i-1;
        for (i=0; i<i1; i++)
          outchr(8);
        for (i=0; i<i1; i++)
          outchr(32);
        for (i=0; i<i1; i++)
          rollover[i]=side1[WhereY()-13][cp1-i1+i];
        rollover[i1]=0;
        cp1 -= i1;
      }
      side1[WhereY()-13][cp1]=0;
    }
  }
  if ((crend) && (WhereY() != 11) && (WhereY()<23))
    nl();
  if (side==0)
    cp0=0;
  else
    cp1=0;
  s[0]=0;
}

/****************************************************************************/

/*
 * High-level chat function, calls two_way_chat() if appropriate, else
 * uses normal TTY chat.
 */

void chat1(char *chatline, int two_way)
{
  int tempdata, cnt, otag;
  char cl[81],xl[81],s[161],s1[161],atr[81],s2[81],cc,sn[81];
  int cf,oe;
  double tc;

  select_chat_name(sn);
  if (sn[0]==0)
    return;

  otag=tagging;
  tagging=0;

  chatcall=0;
  if (two_way) {
    write_inst(INST_LOC_CHAT2,0,INST_FLAGS_NONE);
    chatting=2;
  } else {
    write_inst(INST_LOC_CHAT,0,INST_FLAGS_NONE);
    chatting=1;
  }
  tc=timer();
  cf=0;

  savel(cl,atr,xl,&cc);
  s1[0]=0;

  oe=echo;
  echo=1;
  nln(2);
  tempdata=topdata;
  if (!okansi())
    two_way=0;
  if (modem_speed==300)
    two_way=0;

  if (syscfg.sysconfig & sysconfig_two_color)
    two_color=1;

  if (menu_on()) {
    printmenu(314);
    cleared = NEEDCLEAR;
  }

  if (two_way) {
    clrscrb();
    cp0=0;
    cp1=0;
    if (defscreenbottom==24) {
      topdata=0;
      topscreen();
    }
    outstr("\x1b[2J");
    x2=1;
    y2=13;
    outstr("\x1b[1;1H");
    x1=WhereX();
    y1=WhereY();
    outstr("\x1b[12;1H");
    ansic(3);
    for (cnt=0;cnt<thisuser.screenchars;cnt++)
      outchr(205);
    sprintf(s,get_stringx(1,93),sn,nam(&thisuser,usernum));
    cnt=((thisuser.screenchars-strlen(stripcolors(s)))/2);
    sprintf(s1,"\x1b[12;%dH",cnt);
    outstr(s1);
    ansic(4);
    outstr(s);
    outstr("\x1b[1;1H");
    s[0]=0;
    s1[0]=0;
    s2[0]=0;
  }

  ansic(7);
  outstr(sn);
  pl(get_string(927));
  nl();
  strcpy(s1,chatline);

  if (two_way) {
    side0 = malloca(MAXLEN*MAXLINES_SIDE);
    side1 = malloca(MAXLEN*MAXLINES_SIDE);
    if (!side0 || !side1)
      two_way=0;
  }

  do {
    if (two_way)
      two_way_chat(s,s1,MAXLEN,1,sn);
    else
      inli(s,s1,160,1);
    if ((chat_file) && (!two_way)) {
      if (cf==0) {
        if (!two_way)
          outs(get_stringx(1,94));
        sprintf(s2,"%sCHAT.TXT",syscfg.gfilesdir);
        cf=sh_open(s2,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
        sh_lseek(cf,0L,SEEK_END);
        sprintf(s2,get_stringx(1,95),date(),times());
        sh_write(cf,(void *)s2,strlen(s2));
        strcpy(s2,get_stringx(1,96));
        sh_write(cf,(void *)s2,strlen(s2));
      }
      strcat(s,"\r\n");
      sh_write(cf,(void *)s,strlen(s));
    } else
      if (cf) {
        sh_close(cf);
        cf=0;
        if (!two_way)
          outs(get_stringx(1,97));
      }
    if (hangup)
      chatting=0;
  } while (chatting);
  if (chat_file) {
    sh_close(cf);
    chat_file=0;
  }

  if (side0) {
    bbsfree(side0);
    side0=NULL;
  }
  if (side1) {
    bbsfree(side1);
    side1=NULL;
  }

  ansic(0);
  two_color=0;

  if (two_way)
    outstr("\x1b[2J");

  nl();
  ansic(7);
  pl(get_string(928));
  nl();
  chatting=0;
  tc=timer()-tc;
  if (tc<0)
    tc += 86400.0;
  extratimecall += tc;
  topdata=tempdata;
  if (useron)
    topscreen();
  echo=oe;
  restorel(cl,atr,xl,&cc);

  tagging=otag;
  if (rip_on())
    comstr("|10000//CLS^m\r ");
  if (okansi())
    outstr("\x1b[K");
}

/****************************************************************************/
