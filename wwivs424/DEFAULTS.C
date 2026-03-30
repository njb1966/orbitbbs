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


void select_editor(void)
{
  char *ss;
  int i;

  helpl=37;
  if (numed==0) {
    nl();
    pl(get_string(379));
    nl();
    return;
  }
  for (i=0; i<5; i++)
    odc[i]=0;
  pl(get_string(380));
  for (i=0; i<numed; i++) {
    npr("%d. %s\r\n",i+1,editors[i].description);
    if (((i+1) %10)==0)
      odc[(i+1)/10-1]=(i+1)/10;
  }
  nl();
  ansic(2);
  outstr(get_string(381));
  npr("%d",numed);
  outstr(get_string(382));
  ansic(0);
  ss=mmkey(2);
  i=atoi(ss);
  if ((i>=1) && (i<=numed))
    thisuser.defed=i;
  else
    if (strcmp(ss,"0")==0)
      thisuser.defed=0;
}


void print_cur_stat(void)
{
  userrec ur;

  outchr(12);

  ansic_x(1); outstr(get_string(383));
  ansic_x(2); npr("%d X %d\r\n", thisuser.screenchars,
    thisuser.screenlines);
  ansic_x(1); outstr(get_string(384));
  ansic_x(2); npr("%s\r\n", (thisuser.sysstatus & sysstatus_ansi)?
    ((thisuser.sysstatus & sysstatus_color)? get_string(385):get_string(386)):
    get_string(387));
  ansic_x(1); outstr(get_string(388));
  ansic_x(2); npr("%s\r\n",(thisuser.sysstatus & sysstatus_pause_on_page)?
    get_string(389):get_string(390));
  ansic_x(1); outstr(get_string(391));
  ansic_x(2);
  if ((thisuser.forwardsys==0) && (thisuser.forwardusr==0))
    pl(get_string(392));
  else {
    if (thisuser.forwardsys) {
      outstr(get_string(393));
      npr("#%u @%u.%s.\r\n",
        thisuser.forwardusr,thisuser.forwardsys,
        net_networks[thisuser.net_num].name);
    } else {
      if (thisuser.forwardusr==65535) {
        pl(get_string(300));
      } else {
        read_user(thisuser.forwardusr,&ur);
        if (ur.inact & inact_deleted) {
          thisuser.forwardusr=0;
          pl(get_string(392));
        } else {
          outstr(get_string(393));
          npr("%s\r\n",nam(&ur,thisuser.forwardusr));
        }
      }
    }
  }
  ansic_x(1); pl(get_string(394));
  ansic_x(1); pl(get_string(395));
  ansic_x(1); pl(get_string(396));
  if (okansi()) {
    ansic_x(1); pl(get_string(397));
    ansic_x(1); outstr(get_string(398));
    ansic_x(2); npr("%s\r\n",((thisuser.defed) && (thisuser.defed<=numed))?
      editors[thisuser.defed-1].description:get_string(5));
    ansic_x(1); outstr(get_string(399));
    ansic_x(2); pl((thisuser.sysstatus & sysstatus_extra_color)?str_yes:str_no);
  }
  ansic_x(1); outstr(get_string(400));
  ansic_x(2); npr("%d\r\n",thisuser.optional_val);
  ansic_x(1); outstr(get_string(1149));
  ansic_x(2); pl((thisuser.sysstatus & sysstatus_conference)?str_yes:str_no);
  /* wwiv_regnum display removed in OrbitBBS */
  if (num_languages>1) {
    ansic_x(1); outstr(get_string(936));
    ansic_x(2); pl(cur_lang_name);
  }
  if (num_instances()>1) {
    ansic_x(1); outstr(get_string(1493));
    ansic_x(2); pl((thisuser.sysstatus & sysstatus_no_msgs)?str_no:str_yes);
  }
  ansic_x(1); outstr(get_string(1494));
  ansic_x(2); pl((thisuser.sysstatus & sysstatus_disable_rip)?str_no:str_yes);
  ansic_x(1); pl(get_string(403));
}


char *cn(char c)
{
  static char s[20];

  if ((checkcomp("Ami")) || (checkcomp("Mac"))) {
    strcpy(s,get_string(404));
    sprintf(s+strlen(s),"%d",c);
    return(s);
  }

  switch(c) {
    case 0:
      return(get_string(405));
    case 1:
      return(get_string(406));
    case 2:
      return(get_string(407));
    case 3:
      return(get_string(408));
    case 4:
      return(get_string(409));
    case 5:
      return(get_string(410));
    case 6:
      return(get_string(411));
    case 7:
      return(get_string(412));
  }
  return("");
}

char *describe(char col)
{
  static char s[81];

  if (thisuser.sysstatus & sysstatus_color) {
    strcpy(s,cn(col&0x07));
    strcat(s,get_string(413));
    strcat(s,cn((col>>4)&0x07));
  } else {
    if ((col & 0x07) == 0)
      strcpy(s,get_string(414));
    else
      strcpy(s,get_string(392));
  }
  if (col & 0x08) {
    if ((checkcomp("Ami")) || (checkcomp("Mac")))
      strcat(s,get_string(415));
    else
      strcat(s,get_string(416));
  }
  if (col & 0x80) {
    if (checkcomp("Ami"))
      strcat(s,get_string(417));
    else
      if (checkcomp("Mac"))
        strcat(s,get_string(418));
      else
        strcat(s,get_string(419));
  }
  return(s);
}

void color_list(void)
{
  int i;

  nln(2);
  for (i=0; i<8; i++) {
    if (i==0)
      setc(0x70);
    else
      setc(i);
    npr("%d. %s",i,cn(i));
    setc(0x07);
    nl();
  }
}



void change_colors(void)
{
  int i,done,i1;
  char s[81],ch,nc;

  done=0;
  helpl=36;
  nl();
  do {
    if ((thisuser.sysstatus & sysstatus_color)==0) {
      strcpy(s,get_string(420));
      if ((thisuser.bwcolors[1] & 0x70) == 0)
        strcat(s,cn(thisuser.bwcolors[1] & 0x07));
      else
        strcat(s,cn((thisuser.bwcolors[1] >> 4) & 0x07));
      pl(s);
      nl();
    }
    for (i=0; i<10; i++) {
      ansic(i);
      itoa(i,s,10);
      strcat(s,". ");
      switch(i) {
        case 0:
          strcat(s,get_string(421));
          break;
        case 1:
          strcat(s,get_string(422));
          break;
        case 2:
          strcat(s,get_string(423));
          break;
        case 3:
          strcat(s,get_string(424));
          break;
        case 4:
          strcat(s,get_string(425));
          break;
        case 5:
          strcat(s,get_string(426));
          break;
        case 6:
          strcat(s,get_string(427));
          break;
        case 7:
          strcat(s,get_string(428));
          break;
        case 8:
          strcat(s,get_string(958));
          break;
        case 9:
          strcat(s,get_string(959));
          break;
      }
      if (thisuser.sysstatus & sysstatus_color)
        strcat(s,describe(thisuser.colors[i]));
      else
        strcat(s,describe(thisuser.bwcolors[i]));
      pl(s);
    }
    nl();
    prt(2,get_string(429));
    ch=onek("Q0123456789");
    if (ch=='Q')
      done=1;
    else {
      i1=ch-'0';
      if (thisuser.sysstatus & sysstatus_color)  {
        color_list();
        ansic(0);
        nl();
        prt(2,get_string(430));
        ch=onek("01234567");
        nc=ch-'0';
        prt(2,get_string(431));
        ch=onek("01234567");
        nc=nc | ((ch-'0') << 4);
      } else {
         nl();
         prt(5,get_string(432));
           if (yn()) {
             if ((thisuser.bwcolors[1] & 0x70) == 0)
               nc=0 | ((thisuser.bwcolors[1] & 0x07) << 4);
             else
               nc=(thisuser.bwcolors[1] & 0x70);
           } else {
             if ((thisuser.bwcolors[1] & 0x70) == 0)
               nc=0 | (thisuser.bwcolors[1] & 0x07);
             else
               nc=((thisuser.bwcolors[1] & 0x70) >> 4);
           }
      }
      if ((checkcomp("Ami")) || (checkcomp("Mac")))
        prt(5,get_string(433));
      else
        prt(5,get_string(434));
      if (yn())
        nc |= 0x08;

      if (checkcomp("Ami"))
        prt(5,get_string(435));
      else
        if (checkcomp("Mac"))
          prt(5,get_string(436));
        else
          prt(5,get_string(437));
      if (yn())
        nc |= 0x80;

      nln(2);
      setc(nc);
      outstr(describe(nc));
      ansic(0);
      nln(2);
      prt(5,get_string(438));
      if (yn()) {
        nl();
        pl(get_string(439));
        nl();
        if (thisuser.sysstatus & sysstatus_color)
          thisuser.colors[i1]=nc;
        else
          thisuser.bwcolors[i1]=nc;
      } else {
        nl();
        pl(get_string(440));
        nl();
      }
    }
  } while ((!done) && (!hangup));
}



void l_config_qscan(void)
{
  int i,abort;
  char s[81];

  abort=0;
  nl();
  pl(get_string(441));
  nl();
  for (i=0; (i<num_subs) && (usub[i].subnum!=-1) && (!abort); i++) {
    sprintf(s,"%c %s. %s",
      (qsc_q[usub[i].subnum/32]&(1L<<(usub[i].subnum%32)))?'*':' ',
      usub[i].keys,
      subboards[usub[i].subnum].name);
    pla(s,&abort);
  }
  nln(2);
}

void config_qscan(void)
{
  char *s, s1[MAX_CONFERENCES+2], s2[120], ch;
  int i,done,done1,oc,os,abort=0;

  if (menu_on()) {
    rip_config_qscan();
    return;
  }

  done=done1=0;
  oc=curconfsub;
  os=usub[cursub].subnum;

  do {
    if ((okconf(&thisuser)) && (uconfsub[1].confnum!=-1)) {
      abort=0;
      strcpy(s1," ");
      nl();
      pl(get_string(1081));
      nl();
      i=0;
      while ((i<subconfnum) && (uconfsub[i].confnum!=-1) && (!abort)) {
        sprintf(s2,"%c) %s",subconfs[uconfsub[i].confnum].designator,
          stripcolors(subconfs[uconfsub[i].confnum].name));
        pla(s2,&abort);
        s1[i+1]=subconfs[uconfsub[i].confnum].designator;
        s1[i+2]=0;
        i++;
      }
      nl();
      outstr(get_string(1082));
      outstr(&s1[1]);
      outstr(get_string(1083));
      ch=onek(s1);
    } else
      ch='-';
    switch (ch) {
      case ' ':
        done1=1;
        break;
      default:
        if ((okconf(&thisuser)) && (uconfsub[1].confnum!=-1)) {
          i=0;
          while ((ch!=subconfs[uconfsub[i].confnum].designator) && (i<subconfnum))
            i++;

          if (i>=subconfnum)
            break;

          setuconf(CONF_SUBS, i, -1);
        }
        l_config_qscan();
        done=0;
        do {
          nl();
          outstr(get_string(1151));
          s=mmkey(0);
          if (s[0]) {
            for (i=0; (i<num_subs) && (usub[i].subnum!=-1); i++) {
              if (strcmp(usub[i].keys,s)==0) {
                qsc_q[usub[i].subnum/32] ^= (1L<<(usub[i].subnum%32));
              }
              if (strcmp(s,"S")==0) {
                qsc_q[usub[i].subnum/32] |= (1L<<(usub[i].subnum%32));
              }
              if (strcmp(s,"C")==0) {
                qsc_q[usub[i].subnum/32] &= ~(1L<<(usub[i].subnum%32));
              }
            }
            if (strcmp(s,"Q")==0)
              done=1;
            if (strcmp(s,"?")==0)
              l_config_qscan();
          }
        } while ((!done) && (!hangup));
        break;
    }
    if ((!okconf(&thisuser)) || (uconfsub[1].confnum==-1))
      done1=1;

  } while ((!done1) && (!hangup));

  if (okconf(&thisuser))
    setuconf(CONF_SUBS, oc, os);
}


void list_macro(unsigned char *s)
{
  int i;

  i=0;
  outchr('\"');
  while ((i<80) && (s[i]!=0)) {
    if (s[i]>=32)
      outchr(s[i]);
    else {
      outchr('^');
      outchr(s[i]+64);
    }
    ++i;
  }
  outchr('"');
  nl();
}


void make_macros(void)
{
  unsigned char tempmac[81],s[81];
  unsigned char ch,ch1;
  int i,i1,done,done1;

  done=0;
  do {
    nl();
    prt(2,get_string(444));
    helpl=33;
    ch=onek("QLM");
    switch(ch) {
      case 'Q':
        done=1;
        break;
      case 'L':
        nln(2);
        pl(get_string(445));
        list_macro(&(thisuser.macros[0][0]));
        nl();
        pl(get_string(446));
        list_macro(&(thisuser.macros[1][0]));
        nl();
        pl(get_string(447));
        list_macro(&(thisuser.macros[2][0]));
        break;
      case 'M':
        nl();
        prt(2,get_string(448));
        ch1=onek("QADF");
        if (ch1!='Q') {
          switch(ch1) {
            case 'A': i1=2; break;
            case 'D': i1=0; break;
            case 'F': i1=1; break;
          }
          strcpy(s,&(thisuser.macros[i1][0]));
          thisuser.macros[i1][0]=0;
          done1=0;
          i=0;
          nl();
          pl(get_string(449));
          nl();
	  okskey=0;
          do {
            ch1=getkey();
            if (ch1==26)
              done1=1;
            else
              if (ch1==8) {
                if (i>0) {
                  i--;
                  backspace();
                  if (tempmac[i]<32)
                    backspace();
                }
              } else {
                if (ch1>=32) {
                  tempmac[i++]=ch1;
                  outchr(ch1);
                } else {
                  tempmac[i++]=ch1;
                  outchr('^');
                  outchr(ch1+64);
                }
              }
            if (i>=78)
              done1=1;
          } while ((!done1) && (!hangup));
	  okskey=1;
          tempmac[i]=0;
          nl();
          pl(get_string(450));
          nln(2);
          list_macro(tempmac);
          nl();
          prt(5,get_string(438));
          if (yn()) {
            strcpy(&(thisuser.macros[i1][0]),tempmac);
            nl();
            pl(get_string(451));
          } else {
            nl();
            strcpy(&(thisuser.macros[i1][0]),s);
            pl(get_string(452));
          }
        }
        break;
    }
  } while ((!done) && (!hangup));
}


void input_pw1(void)
{
  char s[81],s1[81];

  nl();
  prt(5,get_string(453));
  if (yn()) {
    nl();
    pl(get_string(454));
    outstr(": ");
    echo=0;
    input(s,8);
    if (strcmp(s,thisuser.pw)) {
      nl();
      pl(get_string(455));
      nl();
      return;
    }
    nln(2);
    pl(get_string(456));
    outstr(": ");
    echo=0;
    input(s,8);
    nln(2);
    pl(get_string(457));
    outstr(": ");
    echo=0;
    input(s1,8);
    if (strcmp(s,s1)==0) {
      if (strlen(s1)<3) {
        nl();
        pl(get_string(458));
        pl(get_string(459));
        nl();
      } else {
        strcpy(thisuser.pw,s);
        nl();
        pl(get_string(460));
        nl();
        sysoplog(get_stringx(1, 98));
      }
    } else {
      nl();
      pl(get_string(462));
      pl(get_string(463));
      nl();
    }
  }
}

void modify_mailbox(void)
{
  char s[81];

  nl();
  helpl=35;

  prt(5,get_string(464));
  if (yn()) {
    prt(5,get_string(465));
    if (yn()) {
      thisuser.forwardsys=0;
      thisuser.forwardusr=-1;
      return;
    }
  }
  prt(5,get_string(466));
  if (!yn()) {
    thisuser.forwardsys=0;
    thisuser.forwardusr=0;
    return;
  }
  nl();
  prt(2,get_string(467));
  input(s,40);
  parse_email_info(s, &thisuser.forwardusr, &thisuser.forwardsys);
  if (thisuser.forwardsys) {
    thisuser.net_num=net_num;
    if (!thisuser.forwardusr) {
      thisuser.forwardsys=thisuser.net_num=0;
      nl();
      pl(get_string(468));
      nl();
    }
 } else if (thisuser.forwardusr==usernum) {
    nl();
    pl(get_string(469));
    nl();
    thisuser.forwardusr=0;
  }
  if ((!thisuser.forwardusr) && (!thisuser.forwardsys)) {
    thisuser.net_num=0;
    nl();
    pl(get_string(470));
    nl();
  } else {
    nl();
    pl(get_string(471));
    nl();
  }
}


void optional_lines(void)
{
  char s[81];
  int i;

  pl(get_string(472));
  pl(get_string(473));
  prt(2,get_string(474));
  input(s,2);

  i=atoi(s);
  if ((s[0]) && (i>=0) && (i<11))
    thisuser.optional_val = i;

}


void enter_regnum(void)
{
  char s[81];
  long l;

  pl(get_string(475));
  pl(get_string(476));
  nl();
  prt(2,get_string(261));
  input(s,5);
  l=atol(s);
  if ((s[0]) && (l>=0)) {
    thisuser.wwiv_regnum = l;
    changedsl();
  }
}

void defaults(void)
{
  int done;
  char ch;

  done=0;
  if (menu_on() == 0)
    print_cur_stat();
  do {
    tleft(1);
    if (hangup)
      return;
    nl();
    helpl=4;
    if (menu_on()) {
      printmenu(316);
      if (thisuser.sysstatus & sysstatus_ansi)
            comstr("|@AQ33X\r\r");
      if (thisuser.sysstatus & sysstatus_pause_on_page)
            comstr("|@AQ3RX\r\r");
      if (thisuser.sysstatus & sysstatus_extra_color)
            comstr("|@AQ4FX\r\r");
      if (thisuser.sysstatus & sysstatus_conference)
            comstr("|@AQ53X\r\r");
      if ((thisuser.sysstatus & sysstatus_no_msgs)==0)
            comstr("|@AQ5RX\r\r");
      ch=onek("Q?123456789ABCLMR");
    } else if (okansi()) {
      prt(2,get_string(477));
      ch=onek("Q?123456789ABCLMR");
    } else {
      prt(2,get_string(478));
      ch=onek("Q?1234567BCLMR");
    }
    if (menu_on()) {
      comr("|*");
      printmenu(319);
    }
    switch(ch) {
      case 'Q':
        if (menu_on())
          cleared = WASCLEARED;
        done=1;
        break;
      case '?':
        print_cur_stat();
        break;
      case '1':
        input_screensize();
        break;
      case '2':
        if (menu_on()) {
          thisuser.sysstatus ^= sysstatus_ansi;
          thisuser.sysstatus |= sysstatus_color;
          print_cur_stat();
        } else
          input_ansistat();
        break;
      case '3':
        thisuser.sysstatus ^= sysstatus_pause_on_page;
        if (menu_on()==0)
          print_cur_stat();
        break;
      case '4':
        modify_mailbox();
        break;
      case '5':
        config_qscan();
        break;
      case '6':
        input_pw1();
        break;
      case '7':
        make_macros();
        break;
      case '8':
        change_colors();
        break;
      case '9':
       select_editor();
        break;
      case 'A':
        thisuser.sysstatus ^= sysstatus_extra_color;
        if (menu_on()==0)
          print_cur_stat();
        break;
      case 'B':
        optional_lines();
        break;
      case 'C':
        thisuser.sysstatus ^= sysstatus_conference;
        changedsl();
        if (menu_on()==0)
          print_cur_stat();
        break;
      /* case 'W': enter_regnum() removed in OrbitBBS */
      case 'L':
        if (num_languages>1)
          input_language();
        break;
      case 'M':
        if (num_instances()>1) {
          thisuser.sysstatus &= ~sysstatus_no_msgs;
          nl();
          prt(5, get_string(1495));
          if (!yn())
            thisuser.sysstatus |= sysstatus_no_msgs;
        }
        break;
      case 'R':
        thisuser.sysstatus &= ~sysstatus_disable_rip;
        nl();
        prt(5, get_string(1496));
        if (!yn()) {
          thisuser.sysstatus |= sysstatus_disable_rip;
          rip_cls();
        }
        break;
    }
  } while ((!done) && (!hangup));
}


