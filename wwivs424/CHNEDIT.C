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



void chaindata(int n, char *s)
{
  char w,x,y,z,i;
  chainfilerec c;

  c=chains[n];
  if (c.ar==0)
    x=32;
  else {
    for (i=0; i<16; i++)
      if ((1 << i) & c.ar)
        x='A'+i;
  }
  if (c.ansir & ansir_ansi)
    y='Y';
  else
    y='N';
  if (c.ansir & ansir_no_DOS)
    z='N';
  else
    z='Y';
  if (c.ansir & ansir_shrink)
    w='Y';
  else
    w='N';
  sprintf(s,"%2d %-28.28s  %-27.27s %-3d    %1c  %1c  %1c  %1c",
            n,stripcolors(c.description),c.filename,c.sl,y,x,z,w);
}

void showchains(void)
{
  int abort,i;
  char s[180];

  outchr(12);
  abort=0;
  pla(get_string(102),
      &abort);
  pla(get_string(103),
      &abort);
  for (i=0; (i<numchain) && (!abort); i++) {
    chaindata(i,s);
    pla(s,&abort);
  }
}



void modify_chain(int n)
{
  chainfilerec c;
  chainregrec r;
  char s[81],s1[81],s2[81],ch,ch2;
  int i,done,un;
  userrec u;

  c=chains[n];
  if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
    r=chains_reg[n];
  done=0;
  do {
    outchr(12);
    outstr(get_string(104)); pl(c.description);
    outstr(get_string(105)); pl(c.filename);
    outstr(get_string(106)); pln(c.sl);
    strcpy(s,get_string(5));
    if (c.ar!=0) {
      for (i=0; i<16; i++)
        if ((1 << i) & c.ar)
          s[0]='A'+i;
      s[1]=0;
    }
    outstr(get_string(107)); pl(s);
    outstr(get_string(108));
    pl((c.ansir & ansir_ansi)?get_string(109):get_string(110));
    outstr(get_string(111));
    if (c.ansir & ansir_no_DOS)
      outstr(get_string(112));
    pl(get_string(113));
    outstr(get_string(114));
    if (c.ansir & ansir_no_300)
      outstr(get_string(112));
    pl(get_string(115));
    outstr(get_string(116));
    pl((c.ansir & ansir_shrink)?str_yes:str_no);
    outstr(get_string(117));
    pl((c.ansir & ansir_no_pause)?str_yes:str_no);
    outstr(get_string(118));
    pl((c.ansir & ansir_local_only)?str_yes:str_no);
    outstr(get_string(1486));
    pl((c.ansir & ansir_multi_user)?str_yes:str_no);
    if (sysinfo.flags & OP_FLAGS_CHAIN_REG) {
      if(r.regby[0])
        read_user(r.regby[0],&u);
      outstr(get_string(1069));
      pl((r.regby[0]) ? u.name : get_string(1070));
      for(i=1;i<5;i++) {
        if (r.regby[i]!=0) {
          read_user(r.regby[i],&u);
          outstr(charstr(18,' '));
          pl(u.name);
        }
      }
      outstr(get_string(1071));
      npr("%d\r\n",r.usage);
      outstr(get_string(1487));
      if ((r.maxage==0) && (r.minage==0))
        r.maxage=255;
      npr("%d - %d\r\n", r.minage, r.maxage);
      outstr(get_string(1467));
      pl((c.ansir & ansir_rip_only) ? get_string(109):get_string(110));
      nl();
      prt(2,get_string(1072));
      ch=onek("QABCDEFGHIJKLMNR[]");
    } else {
      outstr(get_string(1467));
      pl((c.ansir & ansir_rip_only) ? get_string(109):get_string(110));
      nl();
      prt(2,get_string(119));
      ch=onek("QABCDEFGHIJKR[]");
    }
    switch(ch) {
      case 'Q':done=1; break;
      case '[':
        chains[n]=c;
        if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
          chains_reg[n]=r;
        if (--n<0)
          n=numchain-1;
        c=chains[n];
        if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
          r=chains_reg[n];
        break;
      case ']':
        chains[n]=c;
        if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
          chains_reg[n]=r;
        if (++n>=numchain)
          n=0;
        c=chains[n];
        if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
          r=chains_reg[n];
        break;
      case 'A':
        nl();
        prt(2,get_string(120));
        inputl(s,40);
        if (s[0])
          strcpy(c.description,s);
        break;
      case 'B':
        nl();
        prt(2,get_string(72));
        input(s,40);
        if (s[0]!=0)
          strcpy(c.filename,s);
        break;
      case 'C':
        nl();
        prt(2,get_string(76));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<256) && (s[0]))
          c.sl=i;
        break;
      case 'D':
        nl();
        prt(2,get_string(80));
        ch2=onek(" ABCDEFGHIJKLMNOP");
        if (ch2==32)
          c.ar=0;
        else
          c.ar=1 << (ch2-'A');
        break;
      case 'E':
        nl();
        prt(5,get_string(121));
        if (yn())
          c.ansir |= ansir_ansi;
        else
          c.ansir &= ~ansir_ansi;
        break;
      case 'F':
        nl();
        prt(5,get_string(122));
        if (ny())
          c.ansir &= ~ansir_no_DOS;
        else
          c.ansir |= ansir_no_DOS;
        break;
      case 'G':
        nl();
        prt(5,get_string(123));
        if (ny())
	  c.ansir &= ~ansir_no_300;
	else
	  c.ansir |= ansir_no_300;
        break;
      case 'H':
        nl();
        prt(5,get_string(124));
        if (yn())
          c.ansir |= ansir_shrink;
        else
          c.ansir &= ~ansir_shrink;
        break;
      case 'I':
        nl();
        prt(5,get_string(125));
        if (yn())
          c.ansir |= ansir_no_pause;
        else
          c.ansir &= ~ansir_no_pause;
        break;
      case 'J':
        nl();
        prt(5,get_string(126));
        if (yn())
          c.ansir |= ansir_local_only;
        else
          c.ansir &= ~ansir_local_only;
        break;
      case 'K':
        nl();
        prt(5,get_string(1076));
        if (yn())
          c.ansir |= ansir_multi_user;
        else
          c.ansir &= ~ansir_multi_user;
        break;
      case 'L':
        if (!(sysinfo.flags & OP_FLAGS_CHAIN_REG))
          break;
        for (i=0;i<5;i++) {
          nl();
          sprintf(s1,"%s",get_string(268));
          sprintf(s2,"%s",get_string(929));
          npr("2%s%s%s)0 ",s1,get_string(1077),s2);
          mpl(30);
          input(s1,30);
          if ((s1[0]!='Q') && (s1[0]!='q')) {
            if (s1[0]=='0') {
              r.regby[i]=0;
            } else {
              un=finduser1(s1);
              if (un>0) {
                read_user(un,&u);
                r.regby[i]=un;
                nl();
                npr("1%-20.20s2%d %s",get_string(1078),un,
                  (r.regby[i]) ? u.name : get_string(1070));
              }
            }
          } else
            break;
        }
      break;
      case 'M':
        r.usage=0;
      break;
      case 'N':
        nl();
        prt(5, get_string(1488));
        input(s,3);
        if (s[0])
          if((atoi(s) > 255) || (atoi(s) < 0))
            s[0]=0;
          else
            r.minage=atoi(s);
        prt(5, get_string(1489));
        input(s,3);
        if (s[0]) {
          if(atoi(s) < r.minage)
            break;
          if(atoi(s) > 255)
            r.maxage=255;
          else
            r.maxage=atoi(s);
        }
        break;
      case 'R':
        nl();
        prt(5,get_string(1468));
        if (yn())
          c.ansir |= ansir_rip_only;
        else
          c.ansir &= ~ansir_rip_only;
        break;
    }
  } while ((!done) && (!hangup));
  chains[n]=c;
  if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
    chains_reg[n]=r;
}


void insert_chain(int n)
{
  chainfilerec c;
  chainregrec r;
  int i;

  for (i=numchain-1; i>=n; i--) {
    chains[i+1]=chains[i];
    if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
      chains_reg[i+1]=chains_reg[i];
  }
  strcpy(c.description,get_string(127));
  strcpy(c.filename,get_string(128));
  c.sl=10;
  c.ar=0;
  c.ansir=0;
  chains[n]=c;
  ++numchain;
  if (sysinfo.flags & OP_FLAGS_CHAIN_REG) {
    memset((char *)&r, 0, sizeof(r));
    r.maxage=255;
    chains_reg[n]=r;
  }
  modify_chain(n);
}


void delete_chain(int n)
{
  int i;

  for (i=n; i<numchain; i++) {
    chains[i]=chains[i+1];
    if (sysinfo.flags & OP_FLAGS_CHAIN_REG)
      chains_reg[i]=chains_reg[i+1];
  }
  --numchain;
}


void chainedit(void)
{
  int i,done,f;
  char s[81],ch;

  if (!checkpw())
    return;
  showchains();
  done=0;
  do {
    nl();
    prt(2,get_string(129));
    ch=onek("QDIM?");
    switch(ch) {
      case '?':
        showchains();
        break;
      case 'Q':
        done=1;
        break;
      case 'M':
        nl();
        prt(2,get_string(130));
        input(s,2);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<numchain))
          modify_chain(i);
        break;
      case 'I':
        if (numchain<sysinfo.max_chains) {
          nl();
          prt(2,get_string(131));
          input(s,2);
          i=atoi(s);
          if ((s[0]!=0) && (i>=0) && (i<=numchain))
            insert_chain(i);
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(132));
        input(s,2);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<numchain)) {
          nl();
          ansic(5);
          outstr(get_string(87));
          outstr(chains[i].description);
          outstr("? ");
          if (yn())
            delete_chain(i);
        }
        break;
    }
  } while ((!done) && (!hangup));
  sprintf(s,"%sCHAINS.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f>0) {
    sh_write(f,(void *)chains, numchain * sizeof(chainfilerec));
    sh_close(f);
  }
  if (sysinfo.flags & OP_FLAGS_CHAIN_REG) {
    sprintf(s,"%sCHAINS.REG",syscfg.datadir);
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (f>0) {
      sh_write(f,(void *)chains_reg, numchain * sizeof(chainregrec));
      sh_close(f);
    }
  }
}

