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

#include "share.h"

/****************************************************************************/

/* Max line length in conference files */
#define MAX_CONF_LINE 4096

/* To prevent heap fragmentation, allocate confrec.subs in multiples. */
#define CONF_MULTIPLE ((unsigned int)(max_subs/5))

/****************************************************************************/

/*
 * Returns info on conference types
 */

int get_conf_info(unsigned int conftype, unsigned int *num, confrec **cpp, char *fn, unsigned int *num_s, userconfrec **uc)
{
  switch(conftype) {
    case CONF_SUBS:
      if (cpp)
        *cpp=subconfs;
      if (num)
        *num=subconfnum;
      if (fn)
        sprintf(fn,"%sSUBS.CNF",syscfg.datadir);
      if (num_s)
        *num_s=num_subs;
      if (uc)
        *uc=uconfsub;
      return(0);
    case CONF_DIRS:
      if (cpp)
        *cpp=dirconfs;
      if (num)
        *num=dirconfnum;
      if (fn)
        sprintf(fn,"%sDIRS.CNF",syscfg.datadir);
      if (num_s)
        *num_s=num_dirs;
      if (uc)
        *uc=uconfdir;
      return(0);
  }
  return(1);
}

/****************************************************************************/

/*
 * Presents user with selection of conferences, gets selection, and changes
 * conference.
 */

void jump_conf(unsigned int conftype)
{
  int i, abort=0;
  unsigned char ch, s[81], s1[81];
  confrec *cp;
  userconfrec *uc;
  unsigned int nc;

  get_conf_info(conftype, &nc, &cp, NULL, NULL, &uc);
  abort=0;
  strcpy(s," ");
  nl();
  ansic(1);
  pl(get_string(1081));
  nl();
  for (i=0; (i<MAX_CONFERENCES) && (uc[i].confnum!=-1); i++) {
    sprintf(s1,"2%c7)1 %s",cp[uc[i].confnum].designator,
      stripcolors(cp[uc[i].confnum].name));
    pla(s1,&abort);
    s[i+1]=cp[uc[i].confnum].designator;
    s[i+2]=0;
  }

  nl();
  ansic(2);
  npr(get_string(1082));
  npr("%s",&s[1]);
  outstr(get_string(1083));
  ch=onek(s);
  if (ch!=' ') {
    for (i=0; (i<MAX_CONFERENCES) && (uc[i].confnum!=-1); i++) {
      if (ch==cp[uc[i].confnum].designator) {
        setuconf(conftype, i, -1);
        break;
      }
    }
  }
}

/****************************************************************************/

/*
 * Removes, inserts, or swaps subs/dirs in all conferences of a specified
 * type.
 */

void update_conf(unsigned int conftype, SUBCONF_TYPE *sub1, SUBCONF_TYPE *sub2, unsigned int action)
{
  confrec *cp;
  unsigned int nc, num_s;
  int pos1, pos2;
  int i,i1;
  confrec c;

  if (get_conf_info(conftype, &nc, &cp, NULL, &num_s, NULL))
    return;

  if ((!nc) || (!cp))
    return;

  switch (action) {
    case CONF_UPDATE_INSERT:
      if (!sub1)
        return;
      for (i=0; i<nc; i++) {
        c=cp[i];
        if ((c.num) && (c.subs)) {
          for (i1=0; i1<c.num; i1++) {
            if (c.subs[i1]>=*sub1)
              c.subs[i1]=c.subs[i1]+1;
          }
        }
        cp[i]=c;
      }
      save_confs(conftype, -1, NULL);
      break;
    case CONF_UPDATE_DELETE:
      if (!sub1)
        return;
      for (i=0; i<nc; i++) {
        c=cp[i];
        if ((c.num) && (c.subs)) {
          if (in_conference(*sub1,&c)!=-1)
            delsubconf(conftype, &c, sub1);
        }
        for (i1=0; i1<c.num; i1++) {
          if (c.subs[i1]>=*sub1)
            c.subs[i1]=c.subs[i1]-1;
        }
        cp[i]=c;
      }
      save_confs(conftype, -1, NULL);
      break;
    case CONF_UPDATE_SWAP:
      if ((!sub1) || (!sub2))
        return;
      for (i=0; i<nc; i++) {
        c=cp[i];
        pos1=in_conference(*sub1,&c);
        pos2=in_conference(*sub2,&c);
        if (pos1!=-1)
          c.subs[pos1]=*sub2;
        if (pos2!=-1)
          c.subs[pos2]=*sub1;
        cp[i]=c;
      }
      save_confs(conftype, -1, NULL);
      break;
  }
}

/****************************************************************************/

/*
 * Returns bitmapped word representing an AR or DAR string.
 */

unsigned int str_to_arword(unsigned char *arstr)
{
  int i;
  unsigned int rar=0;
  char s[81];

  strcpy(s,arstr);
  strupr(s);

  for (i=0; i<16; i++) {
    if (strchr(s,i+'A')!=NULL)
      rar |= (1 << i);
  }
  return(rar);
}

/****************************************************************************/

/*
 * Converts an unsigned int to a string representing those ARs (DARs).
 */

unsigned char *word_to_arstr(unsigned int ar)
{
  static unsigned char arstr[17];
  unsigned int i, i1=0;

  if (ar) {
    for (i=0; i<16; i++) {
      if ((1 << i) & ar) {
        arstr[i1++]='A'+i;
      }
    }
  }
  arstr[i1]=0;
  if (!arstr[0])
    strcpy(arstr,"-");
  return(arstr);
}

/****************************************************************************/

/*
 * Returns first available conference designator, of a specified conference
 * type.
 */

unsigned char first_available_designator(unsigned int conftype)
{
  int i,i1;
  confrec *cp;
  unsigned int nc;

  if (get_conf_info(conftype, &nc, &cp, NULL, NULL, NULL))
    return(0);

  if (nc==MAX_CONFERENCES)
    return(0);
  for (i=0; i<MAX_CONFERENCES; i++) {
    for (i1=0; i1<nc; i1++) {
      if (cp[i1].designator==('A'+i)) {
        break;
      }
    }
    if (i1>=nc)
      return(i+'A');
  }
  return(0);
}

/****************************************************************************/

/*
 * Returns 1 if subnum is allocated to conference c, 0 otherwise.
 */

int in_conference(unsigned int subnum, confrec *c)
{
  int init=-1;
  unsigned int i;

  if ((!c) || (!c->subs) || (c->num==0))
    return(-1);
  for (i=0; i<c->num; i++)
    if (init==-1)
      if (c->subs[i]==subnum)
        init=i;
  return(init);
}

/****************************************************************************/

/*
 * Saves conferences of a specified conference-type.
 */

void save_confs(unsigned int conftype, int whichnum, confrec *c)
{
  unsigned char cf[81];
  unsigned int i,i2,num, num1;
  FILE *f;
  confrec *cp;

  if (get_conf_info(conftype, &num, &cp, cf, NULL, NULL))
    return;

  f=fsh_open(cf,"wt");
  if (!f) {
    ansic(6);
    nl();
    outstr(get_string(1084));
    pl(cf);
    return;
  }

  num1=num;
  if (whichnum==num)
    num1++;

  fprintf(f,"%s\n\n",get_string(1085));
  for (i=0; i<num1; i++) {
    if (whichnum==i) {
      if (c) {
        fprintf(f,"~%c %s\n!%d %d %d %d %d %d %d %u %d %s ",
          c->designator, c->name, c->status, c->minsl, c->maxsl, c->mindsl,
          c->maxdsl, c->minage, c->maxage, c->minbps, c->sex,
          word_to_arstr(c->ar));
        fprintf(f,"%s\n@", word_to_arstr(c->dar));
        if (((c->num)>0) && (c->subs!=NULL)) {
          for (i2=0; i2<c->num; i2++)
            fprintf(f,"%d ",c->subs[i2]);
        }
        fprintf(f,"\n\n");
      } else
        continue;
    }
    if (i<num) {
      fprintf(f,"~%c %s\n!%d %d %d %d %d %d %d %u %d %s ",
        cp[i].designator, cp[i].name, cp[i].status,
        cp[i].minsl, cp[i].maxsl, cp[i].mindsl,
        cp[i].maxdsl, cp[i].minage, cp[i].maxage,
        cp[i].minbps, cp[i].sex, word_to_arstr(cp[i].ar));
      fprintf(f,"%s\n@",word_to_arstr(cp[i].dar));
      if (cp[i].num>0) {
        for (i2=0; i2<cp[i].num; i2++)
          fprintf(f,"%d ",cp[i].subs[i2]);
      }
      fprintf(f,"\n\n");
    }
  }
  fsh_close(f);
}

/****************************************************************************/

/*
 * Lists subs/dirs/whatever allocated to a specific conference.
 */

void showsubconfs(unsigned int conftype, confrec *c)
{
  int abort,i,i1,test;
  unsigned int num;
  char s[120], indexstr[5], confstr[MAX_CONFERENCES+1], ts[2];
  confrec *cp;
  unsigned int nc;

  if (!c)
    return;

  if (get_conf_info(conftype, &nc, &cp, NULL, &num, NULL))
    return;

  abort=0;
  pla(get_string(1086),&abort);
  pla(get_string(1087),&abort);

  for (i=0; (i<num) && (!abort); i++) {
    test=in_conference(i,c);
    itoa(test,indexstr,10);

    /* build conf list string */
    strcpy(confstr,"");
    for (i1=0; i1<nc; i1++) {
      if (in_conference(i, &cp[i1])!=-1) {
        sprintf(ts, "%c", cp[i1].designator);
        strcat(confstr,ts);
      }
    }
    sort_conf_str(confstr);

    switch (conftype) {
      case CONF_SUBS:
        sprintf(s,"%3d %-39s %4.4s %s",i,stripcolors(subboards[i].name),
          (test>-1)?indexstr:charstr(4,'-'),confstr);
        break;
      case CONF_DIRS:
        sprintf(s,"%3d %-39s %4.4s %s",i,stripcolors(directories[i].name),
          (test>-1)?indexstr:charstr(4,'-'),confstr);
        break;
    }
    pla(s,&abort);
  }
}

/****************************************************************************/

/*
 * Takes a string like "100-150,201,333" and returns pointer to list of
 * numbers. Number of numbers in the list is returned in numinlist.
 */

SUBCONF_TYPE *str_to_numrange(unsigned char *s, int *numinlist)
{
  unsigned char ls[81], rs[81], workstr[81];
  int i,i1,cnt=0,lonum,hinum;
  SUBCONF_TYPE intarray[1024], *intlist;
  unsigned int wc1, wc2;

  /* init vars */
  memset(intarray, 0, sizeof(intarray));
  *numinlist=0;

  /* check for input string */
  if (!s)
    return(NULL);

  /* get num "words" in input string */
  wc1=wordcount(s, ",");

  for (i1=1; (i1<=wc1); i1++) {
    checkhangup();
    if (hangup) {
      *numinlist=0;
      return(NULL);
    }
    strncpy(workstr, extractword(i1, s, ","), sizeof(workstr));
    wc2=wordcount(workstr, " -\t\r\n");
    switch (wc2) {
      case 0:
        break;
      case 1:
        i=atoi(extractword(1,workstr," -\t\r\n"));
        if ((i<1024) && (i>=0)) {
          if (intarray[i]==0) {
            intarray[i]=1;
            cnt++;
          }
        }
        break;
      default:
        /* get left/right string */
        strncpy(ls, extractword(1,workstr," -\t\r\n,"), sizeof(ls));
        strncpy(rs, extractword(2,workstr," -\t\r\n,"), sizeof(rs));
        /* convert to numbers */
        lonum=atoi(ls);
        hinum=atoi(rs);
        /* Switch them around if they were reversed */
        if (lonum>hinum) {
          i=lonum;
          lonum=hinum;
          hinum=i;
        }
        if (lonum<0)
          lonum=0;
        if (hinum>1023)
          hinum=1023;
        for (i=lonum; i<=hinum; i++) {
          if (intarray[i]==0) {
            intarray[i]=1;
            cnt++;
          }
        }
        break;
    } /* end switch */
  }

  if (!cnt)
    return(NULL);

  /* allocate memory for list */
  intlist=(SUBCONF_TYPE *)malloca(cnt * sizeof(SUBCONF_TYPE));
  if (intlist) {
    for (i=i1=0; i<1024; i++) {
      if (intarray[i]==1)
        intlist[i1++]=i;
    }
    *numinlist=cnt;
    return(intlist);
  } else {
    *numinlist=0;
    return(NULL);
  }
}

/****************************************************************************/

/*
 * Function to add one subconf (sub/dir/whatever) to a conference.
 */

void addsubconf(unsigned int conftype, confrec *c, SUBCONF_TYPE *which)
{
  SUBCONF_TYPE *intlist;
  int i1,lnum,hnum,numinlist;
  unsigned int i, num;
  SUBCONF_TYPE *tptr;
  unsigned char s[81];

  if ((!c) || (!c->subs))
    return;

  if (get_conf_info(conftype, NULL, NULL, NULL, &num, NULL))
    return;

  if (num<1)
    return;

  if (c->num>=1023) {
    pl(get_string(1490));
    return;
  }

  if (which==NULL) {
    nl();
    prt(2,get_string(1088));
    mpl(60);
    input(s,60);
    if (!s[0])
      return;
    intlist=str_to_numrange(s, &numinlist);
    lnum=0; hnum=numinlist-1;
  } else {
    intlist=NULL;
    lnum=hnum=*which;
  }

  /* add the subconfs now */
  for (i=lnum; i<=hnum; i++) {
    if (which==NULL)
      i1=intlist[i];
    else
      i1=i;
    if (i1>=num)
      break;
    if (in_conference(i1,c)>-1)
      continue;
    if (c->num>=c->maxnum) {
      c->maxnum += CONF_MULTIPLE;
      tptr=malloca(c->maxnum * sizeof(SUBCONF_TYPE));
      if (tptr) {
        memmove(tptr, c->subs, (c->maxnum-CONF_MULTIPLE) * sizeof(SUBCONF_TYPE));
        bbsfree(c->subs);
        c->subs=tptr;
      } else {
        ansic(6);
        pl(get_string(98));
        if (intlist)
          bbsfree(intlist);
        return;
      }
    }
    c->subs[c->num]=i1;
    c->num++;
  }
  if (intlist)
    bbsfree(intlist);
}

/****************************************************************************/

/*
 * Function to delete one subconf (sub/dir/whatever) from a conference.
 */

void delsubconf(unsigned int conftype, confrec *c, SUBCONF_TYPE *which)
{
  int pos,lnum,hnum,numinlist;
  unsigned int num;
  SUBCONF_TYPE i,i1,i2,*intlist;
  unsigned char s[81];

  if ((!c) || (!c->subs) || (c->num<1))
    return;

  if (get_conf_info(conftype, NULL, NULL, NULL, &num, NULL))
    return;

  if (which==NULL) {
    nl();
    prt(2,get_string(1091));
    mpl(60);
    input(s,60);
    if (!s[0])
      return;
    intlist=str_to_numrange(s, &numinlist);
    lnum=0; hnum=numinlist-1;
  } else {
    intlist=NULL;
    lnum=hnum=*which;
  }

  for (i=lnum; i<=hnum; i++) {
    if (which==NULL)
      i1=intlist[i];
    else
      i1=i;
    if (i1>=num)
      break;
    pos=in_conference(i1,c);
    if (pos<0)
      continue;
    for (i2=pos; i2<c->num; i2++)
      c->subs[i2]=c->subs[i2+1];
    c->num--;
  }
  if (intlist)
    bbsfree(intlist);
}

/****************************************************************************/

/*
 * Function for editing the data for one conference.
 */

int modify_conf(unsigned int conftype, unsigned int n)
{
  int done=0, ok, changed=0;
  confrec c, *cp;
  unsigned int num, ns;
  SUBCONF_TYPE i;
  unsigned char ch, ch1, s[81];

  if (get_conf_info(conftype, &num, &cp, NULL, &ns, NULL) || (n>=num))
    return(0);

  c=cp[n];

  do {
    outchr(12);

    outstr(get_string(1093));
    npr("%c\r\n", c.designator);

    outstr(get_string(1094));
    npr("%.52s\r\n", c.name);

    outstr(get_string(1095));
    pln(c.minsl);

    outstr(get_string(1096));
    pln(c.maxsl);

    outstr(get_string(1097));
    pln(c.mindsl);

    outstr(get_string(1098));
    pln(c.maxdsl);

    outstr(get_string(1099));
    pln(c.minage);

    outstr(get_string(1100));
    pln(c.maxage);

    outstr(get_string(1101));
    pl(word_to_arstr(c.ar));

    outstr(get_string(1102));
    pl(word_to_arstr(c.dar));

    outstr(get_string(1103));
    pln(c.minbps);

    outstr(get_string(1104));
    switch (c.sex) {
      case 0:
        pl(get_string(1105));
        break;
      case 1:
        pl(get_string(1106));
        break;
      case 2:
        pl(get_string(1107));
        break;
    }

    outstr(get_string(1108));
    pl((c.status & conf_status_ansi)?str_yes:str_no);

    outstr(get_string(1109));
    pl((c.status & conf_status_wwivreg)?str_yes:str_no);

    outstr(get_string(1491));
    pl((c.status & conf_status_offline)?str_no:str_yes);

    pl(get_string(1110));
    pl(get_string(1111));

    nl();

    prt(2,get_string(1113));

    ch=onek("QSABCDEFGHIJKLMNO");

    switch (ch) {
      case 'A':
        nl();
        prt(2,get_string(1115));
        ch1=onek("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        ok=1;
        for (i=0; i<num; i++) {
          if (i==n)
            i++;
          if (ok)
            if (cp)
              if (cp[i].designator==ch1)
                ok=0;
        }
        if (!ok) {
          nl();
          prt(6,get_string(1117));
          nl();
          pausescr();
          break;
        }
        c.designator=ch1;
        changed=1;
        break;
      case 'B':
        nl();
        prt(2,get_string(1118));
        inputl(s,60);
        if (s[0]) {
          strcpy(c.name,s);
          changed=1;
        }
        break;
      case 'C':
        nl();
        prt(2,get_string(1119));
        input(s,3);
        if (s[0]) {
          if ((atoi(s)>=0) && (atoi(s)<=255)) {
            c.minsl=atoi(s);
            changed=1;
          }
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(1120));
        input(s,3);
        if (s[0]) {
          if ((atoi(s)>=0) && (atoi(s)<=255)) {
            c.maxsl=atoi(s);
            changed=1;
          }
        }
        break;
      case 'E':
        nl();
        prt(2,get_string(1121));
        input(s,3);
        if (s[0]) {
          if ((atoi(s)>=0) && (atoi(s)<=255)) {
            c.mindsl=atoi(s);
            changed=1;
          }
        }
        break;
      case 'F':
        nl();
        prt(2,get_string(1122));
        input(s,3);
        if (s[0]) {
          if ((atoi(s)>=0) && (atoi(s)<=255)) {
            c.maxdsl=atoi(s);
            changed=1;
          }
        }
        break;
      case 'G':
        nl();
        prt(2,get_string(1123));
        input(s,2);
        if (s[0]) {
          c.minage=atoi(s);
          changed=1;
        }
        break;
      case 'H':
        nl();
        prt(2,get_string(1124));
        input(s,3);
        if (s[0]) {
          if ((atoi(s)>=0) && (atoi(s)<=255)) {
            c.maxage=atoi(s);
            changed=1;
          }
        }
        break;
      case 'I':
        nl();
        prt(2,get_string(1125));
        ch1=onek("\rABCDEFGHIJKLMNOP ");
        switch (ch1) {
          case ' ':
          case '\r':
            break;
          default:
            c.ar ^= (1 << (ch1-'A'));
            changed=1;
            break;
        }
        break;
      case 'J':
        nl();
        prt(2,get_string(1126));
        ch1=onek("\rABCDEFGHIJKLMNOP ");
        switch (ch1) {
          case ' ':
          case '\r':
            break;
          default:
            c.dar ^= (1 << (ch1-'A'));
            changed=1;
            break;
        }
        break;
      case 'K':
        nl();
        prt(2,get_string(1127));
        input(s,5);
        if (s[0]) {
          c.minbps=(SUBCONF_TYPE)(atol(s));
          changed=1;
        }
        break;
      case 'L':
        nl();
        changed=1;
        prt(5, get_string(1128));
        ch1=onek("MFA");
        switch (ch1) {
          case 'M':
            c.sex=0;
            break;
          case 'F':
            c.sex=1;
            break;
          case 'A':
            c.sex=2;
            break;
        }
        break;
      case 'M':
        nl();
        changed=1;
        c.status &= ~conf_status_ansi;
        prt(5, get_string(1129));
        if (yn())
          c.status |= conf_status_ansi;
        break;
      case 'N':
        nl();
        changed=1;
        c.status &= ~conf_status_wwivreg;
        prt(5, get_string(1130));
        if (yn())
          c.status |= conf_status_wwivreg;
        break;
      case 'O':
        nl();
        changed=1;
        c.status &= ~conf_status_offline;
        prt(5, get_string(1492));
        if (!ny())
          c.status |= conf_status_offline;
        break;
      case 'S':
        ok=0;
        do {
          nl();
          prt(2,get_string(1131));
          ch1=onek("QARCFS");
          switch (ch1) {
            case 'A':
              addsubconf(conftype, &c, NULL);
              changed=1;
              break;
            case 'R':
              delsubconf(conftype, &c, NULL);
              changed=1;
              break;
            case 'C':
              for (i=0; i<ns; i++) {
                if (in_conference(i,&c)>-1) {
                  delsubconf(conftype, &c, &i);
                  changed=1;
                }
              }
              break;
            case 'F':
              for (i=0; i<ns; i++) {
                if (in_conference(i,&c)<0) {
                  addsubconf(conftype, &c, &i);
                  changed=1;
                }
              }
              break;
            case 'Q':
              ok=1;
              break;
            case 'S':
              nl();
              showsubconfs(conftype, &c);
              break;
          }
        } while (!ok);
        break;
      case 'Q':
        done=1;
        break;
    }
  } while ((!done) && (!hangup));

  cp[n]=c;
  return(changed);
}

/****************************************************************************/

/*
 * Function for inserting one conference.
 */

void insert_conf(unsigned int conftype, unsigned int n)
{
  confrec c;
  unsigned int num;

  if (get_conf_info(conftype, &num, NULL, NULL, NULL, NULL) || (n>num))
    return;

  c.designator=first_available_designator(conftype);

  if (c.designator==0)
    return;

  sprintf(c.name,"%s%c",get_string(1133),c.designator);
  c.minsl=0; c.maxsl=255; c.mindsl=0; c.maxdsl=255; c.minage=0;
  c.maxage=255; c.status=0; c.minbps=0; c.sex=2;  c.ar=0; c.dar=0; c.num=0;
  c.subs=NULL;

  save_confs(conftype, n, &c);

  read_in_conferences(conftype);

  if (modify_conf(conftype, n))
    save_confs(conftype, -1, NULL);
}

/****************************************************************************/

/*
 * Function for deleting one conference.
 */

void delete_conf(unsigned int conftype, unsigned int n)
{
  unsigned int num;

  if (get_conf_info(conftype, &num, NULL, NULL, NULL, NULL) || (n>=num))
    return;

  save_confs(conftype, n, NULL);
  read_in_conferences(conftype);
}

/****************************************************************************/

/*
 * Function for editing conferences.
 */

void conf_edit(unsigned int conftype)
{
  int done=0, ec;
  char ch;
  confrec *cp;
  unsigned int num;

  if (get_conf_info(conftype, &num, &cp, NULL, NULL, NULL))
    return;

  outchr(12);
  list_confs(conftype,1);

  do {
    nl();
    outstr(get_string(1134));
    ch=onek("QIDM?");
    get_conf_info(conftype, &num, &cp, NULL, NULL, NULL);
    switch (ch) {
      case 'D':
        if (num==1) {
          nl();
          pl(get_string(1135));
        } else {
          ec=select_conf(get_string(1136),conftype, 0);
          if (ec>=0)
            delete_conf(conftype, ec);
        }
        break;
      case 'I':
        if (num==MAX_CONFERENCES) {
          nl();
          pl(get_string(1137));
        } else {
          ec=select_conf(get_string(1138),conftype, 0);
          if (ec!=-1) {
            if (ec==-2)
              ec=num;
            insert_conf(conftype, ec);
          }
        }
        break;
      case 'M':
        ec=select_conf(get_string(1139),conftype, 0);
        if (ec>=0) {
          if (modify_conf(conftype, ec))
            save_confs(conftype,-1,NULL);
        }
        break;
      case 'Q':
        done=1;
        break;
      case '?':
        outchr(12);
        list_confs(conftype,1);
        break;
    }
  } while ((!done) && (!hangup));
  if (!wfc)
    changedsl();
}

/****************************************************************************/

/*
 * Lists conferences of a specified type. If OP_FLAGS_SHOW_HIER is set,
 * then the subs (dirs, whatever) in each conference are also shown.
 */

void list_confs(unsigned int conftype, int ssc)
{
  unsigned char s[121], s1[121];
  int abort=0;
  unsigned int i,i2, num, num_s;
  confrec *cp;

  if (get_conf_info(conftype, &num, &cp, NULL, &num_s, NULL))
    return;

  pla(get_string(1140),&abort);
  pla(get_string(1141),&abort);

  for (i=0; ((i<num) && (!abort)); i++) {
    ansic(7);
    sprintf(s,"%cÍ4 %c 1 %-23.23s %3d %3d %4d %4d %4d %4d %5u %-3.3s ",
        (i==(num-1))?'È':'Ì', cp[i].designator, cp[i].name, cp[i].minsl,
        cp[i].maxsl, cp[i].mindsl, cp[i].maxdsl, cp[i].minage, cp[i].maxage,
        cp[i].minbps, word_to_arstr(cp[i].ar));
    sprintf(s1,"%-3.3s %c %1.1s %1.1s",
        word_to_arstr(cp[i].dar),
        (cp[i].sex)?((cp[i].sex==2)?'A':'F'):'M',
        (cp[i].status & conf_status_ansi)?str_yes:str_no,
        (cp[i].status & conf_status_wwivreg)?str_yes:str_no);
    strcat(s,s1);
    pla(s,&abort);
    if (sysinfo.flags & OP_FLAGS_SHOW_HIER) {
      if ((cp[i].num>0) && (cp[i].subs!=NULL) && (ssc)) {
        for (i2=0; ((i2<cp[i].num) && (!abort)); i2++) {
          if (cp[i].subs[i2]<num_s) {
            ansic(7);
            sprintf(s,"%c  %cÄÄÄ 9",(i!=(num-1))?'º':' ',
              (i2==(cp[i].num-1))?'À':'Ã');
            switch(conftype) {
              case CONF_SUBS:
                sprintf(s1,"%s%-3d : %s",get_string(1142),subconfs[i].subs[i2],
                    stripcolors(subboards[cp[i].subs[i2]].name));
                break;
              case CONF_DIRS:
                sprintf(s1,"%s%-3d : %s",get_string(1143),dirconfs[i].subs[i2],
                    stripcolors(directories[cp[i].subs[i2]].name));
                break;
            }
            strcat(s,s1);
            pla(s,&abort);
          }
        }
      }
    }
  }
  nl();
}

/****************************************************************************/

/*
 * Allows user to select a conference. Returns the conference selected
 * (0-based), or -1 if the user aborted. Error message is printed for
 * invalid conference selections.
 */

int select_conf(unsigned char *s, unsigned int conftype, int listconfs)
{
  int i,i1,ok=0,sl=0;
  unsigned char *mmk;

  do {
    if ((listconfs) || (sl)) {
      nl();
      list_confs(conftype,0);
      sl=0;
    }
    if (s && s[0]) {
      nl();
      prt(1,s);
    }
    mmk=mmkey(0);
    if (!mmk[0]) {
      i=-1;
      ok=1;
    } else {
      switch (mmk[0]) {
        case '?':
          sl=1;
          break;
        default:
          switch (conftype) {
            case CONF_SUBS:
              for (i1=0; i1<subconfnum; i1++) {
                if (mmk[0]==subconfs[i1].designator) {
                  ok=1;
                  i=i1;
                  break;
                }
              }
              break;
            case CONF_DIRS:
              for (i1=0; i1<dirconfnum; i1++) {
                if (mmk[0]==dirconfs[i1].designator) {
                  ok=1;
                  i=i1;
                  break;
                }
              }
              break;
          }
          if (mmk[0]=='$') {
            i=-2;
            ok=1;
          }
          break;
      }
      if ((!ok) && (!sl)) {
        nl();
        pl(get_string(1144));
      }
    }
  } while (!ok && !hangup);
  return(i);
}

/****************************************************************************/

/*
 * Creates a default conference file. Should only be called if no conference
 * file for that conference type already exists.
 */

int create_conf_file(unsigned int conftype)
{
  unsigned char cf[81];
  unsigned int i,num;
  FILE *f;

  if (get_conf_info(conftype, NULL, NULL, cf, &num, NULL))
    return(0);

  f=fsh_open(cf,"wt");
  if (!f)
    return(0);

  fprintf(f,"%s\n\n",get_string(1085));
  fprintf(f,"~A General\n!0 0 255 0 255 0 255 0 2 - -\n@");
  for (i=0;i<num;i++)
    fprintf(f,"%d ",i);
  fprintf(f,"\n\n");

  fsh_close(f);
  return(1);
}

/****************************************************************************/

/*
 * Reads in conferences and returns pointer to conference data. Out-of-memory
 * messages are shown if applicable.
 */

confrec *read_conferences(unsigned char *cf, unsigned int *nc, int max)
{
  unsigned char ts[128], *ls, *bs, *ss;
  unsigned long l;
  int i, i1, i2, i3, cc=0, nw, ok=1;
  confrec *conferences;
  FILE *f;

  *nc=get_num_conferences(cf);
  if (*nc<1)
    return(NULL);

  if (!exist(cf))
    return(NULL);

  f=fsh_open(cf,"rt");
  if (!f)
    return(NULL);

  l=((long)*nc)*sizeof(confrec);
  conferences=(confrec *)malloca(l);
  if (!conferences) {
    printf("%s%s.",get_string(1145),cf);
    fsh_close(f);
    return(NULL);
  }

  memset(conferences, 0, l);
  ls=malloca(MAX_CONF_LINE);
  if (!ls) {
    fsh_close(f);
    return(NULL);
  }

  while ((fgets(ls, MAX_CONF_LINE, f) != NULL) && (cc<MAX_CONFERENCES)) {
    nw=wordcount(ls, DELIMS_WHITE);
    if (nw) {
      strncpy(ts,extractword(1,ls,DELIMS_WHITE),sizeof(ts));
      if (ts[0]) {
        switch (ts[0]) {
          case '~':
            if ((nw>1) && (strlen(ts)>1) && (strlen(ls)>3)) {
              bs=strtok(&ls[3],"\r\n");
              if (bs) {
                conferences[cc].designator=ts[1];
                strncpy(conferences[cc].name,bs,sizeof(conferences[cc].name));
              }
            }
            break;
          case '!':
            if ((!conferences[cc].designator) || (nw!=11))
              break;
            for (i=1;i<=nw;i++) {
              strncpy(ts,extractword(i,ls,DELIMS_WHITE),sizeof(ts));
              switch (i) {
                case 1:
                  if (strlen(ts)>=2)
                    conferences[cc].status=atoi(&ts[1]);
                  break;
                case 2:
                  conferences[cc].minsl=atoi(ts);
                  break;
                case 3:
                  conferences[cc].maxsl=atoi(ts);
                  break;
                case 4:
                  conferences[cc].mindsl=atoi(ts);
                  break;
                case 5:
                  conferences[cc].maxdsl=atoi(ts);
                  break;
                case 6:
                  conferences[cc].minage=atoi(ts);
                  break;
                case 7:
                  conferences[cc].maxage=atoi(ts);
                  break;
                case 8:
                  conferences[cc].minbps=atol(ts);
                  break;
                case 9:
                  conferences[cc].sex=atoi(ts);
                  break;
                case 10:
                  conferences[cc].ar=str_to_arword(ts);
                  break;
                case 11:
                  conferences[cc].dar=str_to_arword(ts);
                  break;
              }
            }
            break;
          case '@':
            if (!conferences[cc].designator)
              break;
            if (strlen(extractword(1,ls,DELIMS_WHITE))<2)
              conferences[cc].num=0;
            else
              conferences[cc].num=nw;
            conferences[cc].maxnum=conferences[cc].num;
            l=(long)(conferences[cc].num*sizeof(unsigned int)+1);
            conferences[cc].subs=(unsigned short *)malloca(l);
            ok=(conferences[cc].subs!=NULL);
            if (ok)
              memset(conferences[cc].subs,0,l);
            else {
              printf("%s%d, %s %s%s.",get_string(1146),get_string(1625),cc+1,
                syscfg.datadir,cf);
              for (i2=0;i2<cc;i2++)
                bbsfree(conferences[i2].subs);
              bbsfree(conferences);
              bbsfree(ls);
              fsh_close(f);
              return(NULL);
            }
            i=0;
            i1=0;
            for (ss=strtok(ls,DELIMS_WHITE); ((ss) && (i++<nw)); ss=strtok(NULL,DELIMS_WHITE)) {
              if (i==1) {
                if (strlen(ss)>=2)
                  i3=atoi(ss+1);
                else
                  i3=-1;
              } else {
                i3=atoi(ss);
              }
              if ((i3>=0) && (i3<max)) {
                conferences[cc].subs[i1++]=i3;
              }
            }
            conferences[cc].num=i1;
            cc++;
            break;
        }
      }
    }
  }
  fsh_close(f);
  bbsfree(ls);
  return(conferences);
}

/****************************************************************************/

/*
 * Reads in a conference type. Creates default conference file if it is
 * necessary. If conferences cannot be read, then BBS aborts.
 */

void read_in_conferences(int conftype)
{
  unsigned int i, *np, max;
  unsigned char s[81];
  confrec **cpp;

  if (get_conf_info(conftype, NULL, NULL, s, &max, NULL))
    return;

  switch(conftype) {
    case CONF_SUBS: cpp=&subconfs; np=&subconfnum; break;
    case CONF_DIRS: cpp=&dirconfs; np=&dirconfnum; break;
  }

  /* free up any old memory */
  if (*cpp) {
    for (i=0; i<*np; i++)
      if ((*cpp)[i].subs)
        bbsfree((*cpp)[i].subs);
    bbsfree(*cpp);
    *cpp=NULL;
  }

  if (!exist(s)) {
    if (!create_conf_file(conftype)) {
      printf("%s\n",get_string(1147));
      end_bbs(noklevel);
    }
  }
  *cpp=read_conferences(s, np, max);
  if (!(*cpp)) {
    printf("%s\n",get_string(1148));
    end_bbs(noklevel);
  }
}

/****************************************************************************/

/*
 * Reads all conferences into memory. Creates default conference files if
 * none exist. If called when conferences already in memory, then memory
 * for "old" conference data is deallocated first.
 */

void read_all_conferences(void)
{
  read_in_conferences(CONF_SUBS);
  read_in_conferences(CONF_DIRS);
}

/****************************************************************************/

/*
 * Returns number of conferences in a specified conference file.
 */

unsigned int get_num_conferences(unsigned char *cf)
{
#ifdef MALLOC_STR
  unsigned char *ls;
#else
  unsigned char ls[MAX_CONF_LINE];
#endif
  int i=0;
  FILE *f;

  if (!exist(cf))
    return(0);
  f=fsh_open(cf,"rt");
  if (!f)
    return(0);
#ifdef MALLOC_STR
  ls=malloca(MAX_CONF_LINE);
  if (!ls) {
    fsh_close(f);
    return(0);
  }
#endif

  while (fgets(ls, sizeof(ls), f) != NULL)
    if (ls[0]=='~')
      i++;

  fsh_close(f);
#ifdef MALLOC_STR
  bbsfree(ls);
#endif
  return(i);
}

/****************************************************************************/

/*
 * Returns number of "words" in a specified string, using a specified set
 * of characters as delimiters.
 */

unsigned int wordcount(unsigned char *instr, unsigned char *delimstr)
{
  unsigned char *s;
#ifdef MALLOC_STR
  unsigned char *ts;
#else
  unsigned char ts[MAX_CONF_LINE];
#endif
  int i=0;

#ifdef MALLOC_STR
  ts=malloca(MAX_CONF_LINE);
  if (!ts)
    return(0);
#endif
  strcpy(ts,instr);
  for (s=strtok(ts,delimstr); s; s=strtok(NULL,delimstr))
    i++;
#ifdef MALLOC_STR
  bbsfree(ts);
#endif
  return(i);
}

/****************************************************************************/

/*
 * Returns pointer to string representing the nth "word" of a string, using
 * a specified set of characters as delimiters.
 */

unsigned char *extractword(unsigned ww,unsigned char *instr,unsigned char *delimstr)
{
  unsigned char *s;
#ifdef MALLOC_STR
  unsigned char *ts;
#else
  unsigned char ts[MAX_CONF_LINE];
#endif
  static unsigned char rs[41];
  int i=0;

  if (!ww)
    return("");

#ifdef MALLOC_STR
  ts=malloca(MAX_CONF_LINE);
  if (!ts)
    return("");
#endif
  strcpy(ts,instr);
  for (s=strtok(ts,delimstr); ((s) && (i++<ww)); s=strtok(NULL,delimstr)) {
    if (i==ww) {
      strcpy(rs,s);
#ifdef MALLOC_STR
      bbsfree(ts);
#endif
      return(rs);
    }
  }
#ifdef MALLOC_STR
  bbsfree(ts);
#endif
  return("");
}

/****************************************************************************/

void sort_conf_str(unsigned char *s)
{
  unsigned char s1[81], s2[81];
  unsigned char i;

  strncpy(s1, s, sizeof(s1));
  strncpy(s2, charstr(MAX_CONFERENCES, 32), sizeof(s2));

  for (i='A'; i<='Z'; i++) {
    if (strchr(s1, i)!=NULL)
      s2[i-'A']=i;
  }
  trimstr(s2);
  strcpy(s, s2);
}

/****************************************************************************/
