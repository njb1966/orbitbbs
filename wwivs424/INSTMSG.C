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
#include <errno.h>

/****************************************************************************/

void send_inst_msg(inst_msg_header *ih, unsigned char *msg)
{
  int f,i;
  unsigned char s[81], tfn[81];

  sprintf(tfn,"%sTMSG%3.3d.%3.3d",syscfg.datadir, instance, ih->dest_inst);
  f=sh_open1(tfn,O_RDWR | O_BINARY | O_CREAT);
  if (f>=0) {
    sh_lseek(f,0L,SEEK_END);
    if (ih->msg_size>0)
      if (!msg)
        ih->msg_size=0;
    sh_write(f,(void *)ih,sizeof(inst_msg_header));
    if (ih->msg_size>0)
      sh_write(f, (void *)msg,ih->msg_size);
    f=sh_close(f);

    for (i=0; i<1000; i++) {
      sprintf(s,"%sMSG%5.5d.%3.3d",syscfg.datadir,i,ih->dest_inst);
      if (!rename(tfn, s) || (errno!=EACCES)) {
        break;
      }
    }
  }
}

/****************************************************************************/

void send_inst_str1(unsigned short m, int whichinst, unsigned char *sendstr)
{
  inst_msg_header ih;

  ih.main=m;
  ih.minor=0;
  ih.from_inst=instance;
  ih.from_user=usernum;
  ih.msg_size=strlen(sendstr)+1;
  ih.dest_inst=whichinst;
  time((long *)&ih.daten);

  send_inst_msg(&ih,sendstr);
}

/****************************************************************************/

void send_inst_str(int whichinst, unsigned char *sendstr)
{
  send_inst_str1(INST_MSG_STRING, whichinst, sendstr);
}

/****************************************************************************/

void send_inst_sysstr(int whichinst, unsigned char *sendstr)
{
  send_inst_str1(INST_MSG_SYSMSG, whichinst, sendstr);
}

/****************************************************************************/

void send_inst_shutdown(int whichinst)
{
  inst_msg_header ih;

  ih.main=INST_MSG_SHUTDOWN;
  ih.minor=0;
  ih.from_inst=instance;
  ih.from_user=usernum;
  ih.msg_size=0;
  ih.dest_inst=whichinst;
  time((long *)&ih.daten);

  send_inst_msg(&ih,NULL);
}

/****************************************************************************/

/*
 * "Broadcasts" a message to all online instances.
 */

void broadcast(unsigned char *sendstr)
{
  int i,ni;
  instancerec ir;

  ni=num_instances();
  for (i=1;i<=ni;i++) {
    if (i==instance)
      continue;
    if (get_inst_info(i, &ir)) {
      if (inst_available(&ir))
        send_inst_str(i,sendstr);
    }
  }
}

/****************************************************************************/

/*
 * Handles one inter-instance message, based on type, returns inter-instance
 * main type of the "packet".
 */

int handle_inst_msg(inst_msg_header *ih, unsigned char *msg)
{
  unsigned short i;
  char xl[81], cl[81], atr[81], cc;
  userrec u;

  if ((!ih) || ((ih->msg_size>0) && (msg==NULL)))
    return(-1);

  switch (ih->main) {
    case INST_MSG_STRING:
    case INST_MSG_SYSMSG:
      if ((ih->msg_size>0) && useron && (!hangup)) {
        savel(cl, atr, xl, &cc);
        nln(2);
        read_user(ih->from_user,&u);
        if (ih->main==INST_MSG_STRING)
          npr("1%.12s (%d)0> 2",nam(&u,ih->from_user), ih->from_inst);
        else
          outstr(get_string(1503));
        i=0;
        while (i<ih->msg_size)
          outchr(msg[i++]);
        nln(2);
        restorel(cl, atr, xl, &cc);
      }
      break;
    /* Handle this one in process_inst_msgs */
    case INST_MSG_SHUTDOWN:
    default:
      break;
  }
  return(ih->main);
}

/****************************************************************************/

void process_inst_msgs(void)
{
  int f1,done=0,hi;
  unsigned char s1[81],*m;
  inst_msg_header ih;
  unsigned long l,l1=0L;
  int oiia;
  struct ffblk ff;

  if ((x_only) || (!inst_msg_waiting()))
    return;

  last_iia=timer1();

  oiia=iia;
  setiia(0);
  m=NULL;
  sprintf(s1,"%sMSG*.%3.3d",syscfg.datadir,instance);
  done = findfirst(s1,&ff,0);
  while ((done==0) && (!hangup)) {
    sprintf(s1,"%s%s",syscfg.datadir,ff.ff_name);
    f1=sh_open1(s1,O_RDONLY | O_BINARY);
    if (f1==-1)
      continue;
    l=filelength(f1);
    l1=0;
    while (l1<l) {
      m=NULL;
      sh_read(f1,&ih,sizeof(inst_msg_header));
      l1+=sizeof(inst_msg_header);
      if (ih.msg_size>0) {
        m=malloca(ih.msg_size);
        if (m==NULL) {
          break;
        }
        sh_read(f1,m,ih.msg_size);
        l1+=ih.msg_size;
      }
      hi=handle_inst_msg(&ih,m);
      if (m) {
        bbsfree(m);
        m=NULL;
      }
      if (hi==INST_MSG_SHUTDOWN) {
        if (useron) {
          tmp_disable_pause(1);
          nln(2);
          existprint("OFFLINE");
          if (useron>0) {
            if (usernum==1)
              fwaiting=thisuser.waiting;
            write_user(usernum,&thisuser);
            write_qscn(usernum,qsc,0);
          }
          tmp_disable_pause(0);
        }
        sh_close(f1);
        unlink(s1);
        topline=0;
        clrscrb();
        hangup=1;
        hang_it_up();
        holdphone(0);
        wait1(18);
        end_bbs(QUIT_LEVEL);
      }
    }
    sh_close(f1);
    unlink(s1);
    done=findnext(&ff);
  }
  setiia(oiia);
}

/****************************************************************************/

/* Chat room removed in OrbitBBS */
void chat_room(void) {}

/****************************************************************************/

/*
 * Gets instancerec for specified instance, returns in ir.
 */

int get_inst_info(int wi, instancerec *ir)
{
  char s[81];
  int i, f;

  if (!ir)
    return(0);

  memset(ir, 0, sizeof(instancerec));

  sprintf(s,"%sINSTANCE.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return(0);
  i=(int)(filelength(f)/sizeof(instancerec));
  if (i<(wi+1)) {
    f=sh_close(f);
    return(0);
  }
  sh_lseek(f,(long) (wi) * sizeof(instancerec),SEEK_SET);
  sh_read(f,(void *)ir,sizeof(instancerec));
  f=sh_close(f);
  return(1);
}

/****************************************************************************/

/*
 * Returns 1 if inst_no has user online that can receive msgs, else 0
 */

int inst_available(instancerec *ir)
{
  if (!ir)
    return(0);

  if ((ir->flags & INST_FLAGS_ONLINE) && (ir->flags & INST_FLAGS_MSG_AVAIL))
    return(1);
  else
    return(0);
}

/****************************************************************************/

/*
 * Returns 1 if inst_no has user online in chat, else 0
 */

int inst_available_chat(instancerec *ir)
{
  if (!ir)
    return(0);

  if ((ir->flags & INST_FLAGS_ONLINE) &&
      (ir->flags & INST_FLAGS_MSG_AVAIL) &&
      (ir->loc == INST_LOC_CHATROOM))
    return(1);
  else
    return(0);
}

/****************************************************************************/

/*
 * Returns max instance number.
 */

int num_instances(void)
{
  char s[81];
  int i, f;

  sprintf(s,"%sINSTANCE.DAT",syscfg.datadir);
  f=sh_open(s,O_RDONLY | O_BINARY, S_IREAD | S_IWRITE);
  if (f<0)
    return(0);
  i=(int)(filelength(f)/sizeof(instancerec))-1;
  f=sh_close(f);
  return(i);
}

/****************************************************************************/

/*
 * Returns 1 if usernum un is online, and returns instance user is on in
 * wi, else returns 0.
 */

int user_online(int un, int *wi)
{
  int i, ni;
  instancerec ir;

  ni=num_instances();
  for (i=1;i<=ni;i++) {
    if (i==instance)
      continue;
    get_inst_info(i, &ir);
    if ((ir.user==un) && (ir.flags & INST_FLAGS_ONLINE)) {
      if (wi)
        *wi=i;
      return(1);
    }
  }
  if (wi)
    *wi=-1;
  return(0);
}

/****************************************************************************/

/*
 * Allows sending some types of messages to other instances, or simply
 * viewing the status of the other instances.
 */

void instance_edit(void)
{
  int done=0,ni,i;
  unsigned char ch, s[81];
  instancerec ir;

  if (!checkpw())
    return;

  ni=num_instances();

  while ((!done) && (!hangup)) {
    checkhangup();
    nl();
    pl(get_string(1411));
    pl(get_string(1412));
    pl(get_string(1413));
    pl(get_string(1414));
    nl();
    ansic(1);
    outstr(get_string(1385));
    ch=onek("Q123");
    switch (ch) {
      case '1':
        nl();
        ansic(1);
        pl(get_string(1415));
        multi_instance();
        break;
      case '2':
        nl();
        ansic(2);
        outstr(get_string(1416));
        mpl(3);
        input(s,3);
        if (!s[0])
          break;
        i=atoi(s);
        if ((!i) || (i>ni)) {
          nl();
          ansic(6);
          pl(get_string(1417));
          break;
        }
        if (i==instance) {
          topline=0;
          clrscrb();
          hangup=1;
          hang_it_up();
          holdphone(0);
          wait1(18);
          end_bbs(QUIT_LEVEL);
          break;
        }
        if (get_inst_info(i,&ir)) {
          if (ir.loc!=INST_LOC_DOWN) {
            nl();
            ansic(2);
            npr("%s%d",get_string(1418),i);
            nl();
            send_inst_shutdown(i);
          } else {
            nl();
            ansic(6);
            pl(get_string(1419));
          }
        } else {
          nl();
          ansic(6);
          pl(get_string(1417));
        }
        break;
      case '3':
        nl();
        ansic(5);
        outstr(get_string(465));
        if (yn()) {
          nl();
          ansic(2);
          pl(get_string(1420));
          for (i=1;i<=ni;i++) {
            if (i!=instance) {
              if (get_inst_info(i, &ir))
                if (ir.loc != INST_LOC_DOWN)
                  send_inst_shutdown(i);
            }
          }
          topline=0;
          clrscrb();
          hangup=1;
          hang_it_up();
          holdphone(0);
          wait1(18);
          end_bbs(QUIT_LEVEL);
        }
        break;
      case 'Q':
      default:
        done=1;
        break;
    }
  }
}
