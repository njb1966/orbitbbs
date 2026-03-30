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
#include <math.h>

/****************************************************************************/

void make_inst_str(short wi, unsigned char *ss)
/*
 * Builds a string (in ss) like:
 *
 * Instance   1: Offline
 *     LastUser: Sysop #1
 *
 * or
 *
 * Instance  22: Network transmission
 *     CurrUser: Sysop #1
 */

{
  instancerec ir;
  userrec ur;
  unsigned char s[161],s1[81],s2[81];

  get_inst_info(wi, &ir);
  strcpy(s,"1");
  strncat(s, get_string(1206),sizeof(s));
  sprintf(s1,"%-3d",wi);
  strncat(s,s1,sizeof(s));
  strncat(s,": 2",sizeof(s));

  switch (ir.loc) {
    case INST_LOC_DOWN:
      sprintf(s1,"%-25.25s",get_string(1207));
      break;
    case INST_LOC_INIT:
      sprintf(s1,"%-25.25s",get_string(1208));
      break;
    case INST_LOC_EMAIL:
      sprintf(s1,"%-25.25s",get_string(1209));
      if (so() && (ir.subloc>0) && (ir.subloc<syscfg.maxusers)) {
      }
      break;
    case INST_LOC_MAIN:
      sprintf(s1,"%-25.25s",get_string(1210));
      if ((so()) && (ir.subloc < num_subs)) {
        sprintf(s2,"%s: %s",get_string(1398),
          stripcolors(subboards[ir.subloc].name));
        strncat(s1,s2,sizeof(s1));
      }
      break;
    case INST_LOC_XFER:
      sprintf(s1,"%-25.25s",get_string(1211));
      if ((so()) && (ir.subloc < num_dirs)) {
        sprintf(s2,"%s: %s",get_string(1399),
          stripcolors(directories[ir.subloc].name));
        strncat(s1,s2,sizeof(s1));
      }
      break;
    case INST_LOC_CHAINS:
      sprintf(s1,"%-25.25s",get_string(1212));
      break;
    case INST_LOC_NET:
      sprintf(s1,"%-25.25s",get_string(1213));
      break;
    case INST_LOC_GFILES:
      sprintf(s1,"%-25.25s",get_string(1214));
      break;
    case INST_LOC_BEGINDAY:
      sprintf(s1,"%-25.25s",get_string(1215));
      break;
    case INST_LOC_EVENT:
      sprintf(s1,"%-25.25s",get_string(1216));
      break;
    case INST_LOC_CHAT:
      sprintf(s1,"%-25.25s",get_string(1218));
      break;
    case INST_LOC_CHAT2:
      sprintf(s1,"%-25.25s",get_string(1217));
      break;
    case INST_LOC_CHATROOM:
      sprintf(s1,"%-25.25s",get_string(1219));
      break;
    case INST_LOC_LOGON:
      sprintf(s1,"%-25.25s",get_string(1220));
      break;
    case INST_LOC_LOGOFF:
      sprintf(s1,"%-25.25s",get_string(1221));
      break;
    case INST_LOC_FSED:
      sprintf(s1,"%-25.25s",get_string(1222));
      break;
    case INST_LOC_UEDIT:
      sprintf(s1,"%-25.25s",get_string(1223));
      break;
    case INST_LOC_CHAINEDIT:
      sprintf(s1,"%-25.25s",get_string(1224));
      break;
    case INST_LOC_BOARDEDIT:
      sprintf(s1,"%-25.25s",get_string(1225));
      break;
    case INST_LOC_DIREDIT:
      sprintf(s1,"%-25.25s",get_string(1226));
      break;
    case INST_LOC_GFILEEDIT:
      sprintf(s1,"%-25.25s",get_string(1227));
      break;
    case INST_LOC_CONFEDIT:
      sprintf(s1,"%-25.25s",get_string(1228));
      break;
    case INST_LOC_DOS:
      sprintf(s1,"%-25.25s",get_string(1229));
      break;
    case INST_LOC_DEFAULTS:
      sprintf(s1,"%-25.25s",get_string(1230));
      break;
    case INST_LOC_REBOOT:
      sprintf(s1,"%-25.25s",get_string(1231));
      break;
    case INST_LOC_RELOAD:
      sprintf(s1,"%-25.25s",get_string(1232));
      break;
    case INST_LOC_VOTE:
      sprintf(s1,"%-25.25s",get_string(1233));
      break;
    case INST_LOC_BANK:
      sprintf(s1,"%-25.25s",get_string(1234));
      break;
    case INST_LOC_AMSG:
      sprintf(s1,"%-25.25s",get_string(1235));
      break;
    case INST_LOC_SUBS:
      sprintf(s1,"%-25.25s",get_string(1236));
      if ((so()) && (ir.subloc < num_subs)) {
        sprintf(s2,"%s: %s",get_string(1398),
          stripcolors(subboards[ir.subloc].name));
        strncat(s1,s2,sizeof(s1));
      }
      break;
    case INST_LOC_CHUSER:
      sprintf(s1,"%-25.25s",get_string(1237));
      break;
    case INST_LOC_TEDIT:
      sprintf(s1,"%-25.25s",get_string(1238));
      break;
    case INST_LOC_MAILR:
      sprintf(s1,"%-25.25s",get_string(1239));
      break;
    case INST_LOC_RESETQSCAN:
      sprintf(s1,"%-25.25s",get_string(1240));
      break;
    case INST_LOC_VOTEEDIT:
      sprintf(s1,"%-25.25s",get_string(1241));
      break;
    case INST_LOC_VOTEPRINT:
      sprintf(s1,"%-25.25s",get_string(1401));
      break;
    case INST_LOC_RESETF:
      sprintf(s1,"%-25.25s",get_string(1406));
      break;
    case INST_LOC_FEEDBACK:
      sprintf(s1,"%-25.25s",get_string(1407));
      break;
    case INST_LOC_KILLEMAIL:
      sprintf(s1,"%-25.25s",get_string(1408));
      break;
    case INST_LOC_POST:
      sprintf(s1,"%-25.25s",get_string(1409));
      if ((so()) && (ir.subloc < num_subs)) {
        sprintf(s2,"%s: %s",get_string(1398),
          stripcolors(subboards[ir.subloc].name));
        strncat(s1,s2,sizeof(s1));
      }
      break;
    case INST_LOC_NEWUSER:
      sprintf(s1,"%-25.25s",get_string(1423));
      break;
    case INST_LOC_RMAIL:
      sprintf(s1,"%-25.25s",get_string(1424));
      break;
    case INST_LOC_DOWNLOAD:
      sprintf(s1,"%-25.25s",get_string(1425));
      break;
    case INST_LOC_UPLOAD:
      sprintf(s1,"%-25.25s",get_string(1426));
      break;
    case INST_LOC_BIXFER:
      sprintf(s1,"%-25.25s",get_string(1427));
      break;
    case INST_LOC_NETLIST:
      sprintf(s1,"%-25.25s",get_string(1428));
      break;
    case INST_LOC_TERM:
      sprintf(s1,"%-25.25s",get_string(1519));
      break;
    case INST_LOC_QWK:
      sprintf(s1,"%-25.25s",get_string(1639));
      break;
    case INST_LOC_GETUSER:
      sprintf(s1,"%-25.25s",get_string(1640));
      break;
    case INST_LOC_WFC:
      sprintf(s1,"%-25.25s",get_string(1402));
      break;
    default:
      sprintf(s1,"%-25.25s",get_string(1403));
      break;
  }
  strncat(s,s1,sizeof(s));
  strncat(s,"\r\n1",sizeof(s));

  if (ir.flags & INST_FLAGS_ONLINE)
    strncat(s,get_string(1404),sizeof(s));
  else
    strncat(s,get_string(1405),sizeof(s));

  strncat(s,": 2",sizeof(s));

  if ((ir.user < syscfg.maxusers) && (ir.user > 0)) {
    read_user(ir.user,&ur);
    sprintf(s1,"%-30.30s",nam(&ur,ir.user));
    strncat(s,s1,sizeof(s));
  } else {
    sprintf(s1,"%-30.30s",get_string(1410));
    strncat(s,s1,sizeof(s));
  }

  strcpy(ss,s);
}

/****************************************************************************/

void multi_instance(void)
{
  char s[161];
  int i1,ni;

  nl();
  ni=num_instances();
  if (ni<1) {
    ansic(6);
    pl(get_string(1205));
    return;
  }
  for (i1=1;i1<=ni;i1++) {
    make_inst_str(i1,s);
    if (E_C) {
      pl(s);
    } else {
      pl(stripcolors(s));
    }
    nl();
  }
}

/****************************************************************************/

int inst_ok(unsigned short loc, unsigned short subloc)
{
  char s[81];
  int i, i1, f, ok;
  instancerec instance_temp;

  if (loc==INST_LOC_FSED)
    return(0);

  ok=0;
  sprintf(s,"%sINSTANCE.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return(0);
  i=(int)(filelength(f)/sizeof(instancerec));
  f=sh_close(f);
  for (i1=1;i1<i;i1++) {
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if (f>0) {
      sh_lseek(f,(long) i1 * sizeof(instancerec),SEEK_SET);
      sh_read(f,(void *)&instance_temp,sizeof(instancerec));
      f=sh_close(f);
      if ((instance_temp.loc == loc)
        && (instance_temp.subloc == subloc)
        && (instance_temp.number != instance)) {
        ok=instance_temp.number;
      }
    }
  }
  return(ok);
}

