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

#include <math.h>


char cid_num[50];
char cid_name[50];


#define modem_time 3.5


void pr1(unsigned char *s)
/* This function ouputs a string to the com port.  This is mainly used
 * for modem commands
 */
{
  int i;

  for (i = 0; s[i] > 0; i++)
    if (s[i] == 255)
      wait1(9);
    else
      outcomch(s[i]);
}



void get_modem_line(char *s, double d, int allowa)
{
  int i=0;
  char ch=0, ch1;
  double t;

  t = timer();
  do {
    ch = get1c();
    if (kbhitb() && allowa) {
      ch1 = getchd();
      if (upcase(ch1) == 'H') {
        ch = 13;
        s[0] = i = 1;
      }
    } else if (!ch) {
      giveup_timeslice();
    }
    if (ch >= 32)
      s[i++] = upcase(ch);
  } while ((ch != 13) && (fabs(timer() - t) < d) && (i<=40));
  s[i] = 0;
}




void do_result(result_info *ri)
{
  if (ri->description[0])
    if (ri->flag_value & flag_append)
      strcat(curspeed, ri->description);
    else
      strcpy(curspeed, ri->description);

  if (ri->main_mode)
    modem_mode = ri->main_mode;

  /* ignore the ringing mode, caller-id stuff */
  if ((modem_mode == mode_ringing) ||
      (modem_mode == mode_cid_num) ||
      (modem_mode == mode_cid_name)) {
    modem_mode=0;
  }

  modem_flag = (modem_flag & ri->flag_mask) | ri->flag_value;
  if (modem_flag & flag_fc)
    flow_control = 1;

  if (ri->com_speed) {
    com_speed = ri->com_speed;
    set_baud(com_speed);
  }

  if (ri->modem_speed)
    modem_speed = ri->modem_speed;

}


void process_full_result(char *s)
{
  char *ss;
  int i,i1;

  /* first, check for caller-id info */
  for (i=0; i<modem_i->num_resl; i++) {
    i1=strlen(modem_i->resl[i].result);
    if (strncmp(modem_i->resl[i].result, s, i1)==0) {
      switch(modem_i->resl[i].main_mode) {
        case mode_cid_num:
          strcpy(cid_num, s+i1);
          return;
        case mode_cid_name:
          strcpy(cid_name, s+i1);
          return;
      }
    }
  }

  ss=strtok(s,modem_i->sepr);

  while (ss) {
    for (i=0; i<modem_i->num_resl; i++) {
      if (strcmp(modem_i->resl[i].result,ss)==0) {
        do_result(&(modem_i->resl[i]));
        break;
      }
    }
    ss=strtok(NULL,modem_i->sepr);
  }
}

int mode_switch(double d, int allowa)
{
  double t;
  char s[81];
  int abort=0;

  t=timer();
  modem_mode = 0;

  while ((modem_mode==0) && (fabs(timer()-t)<d) && (!abort)) {
    get_modem_line(s,d+t-timer(), allowa);
    if (s[0]==1)
      abort=1;
    else if (s[0])
      process_full_result(s);
  }

  return(modem_mode);
}

void holdphone(int d)
{

  if (!ok_modem_stuff)
    return;
  if (no_hangup)
    return;
  if ((!(modem_i->pick)) || (!(modem_i->hang)))
    return;

  dtr(1);
  if (d) {
    if (!global_xx) {
      if (syscfg.sysconfig & sysconfig_off_hook) {
        do_result(&(modem_i->defl));
        pr1(modem_i->pick);
        xtime=timer();
        global_xx=1;
      }
    }
  } else {
    if (syscfg.sysconfig & sysconfig_off_hook) {
      if (global_xx) {
        dtr(1);
        if (fabs(xtime-timer())<modem_time)
          outs("\r\n\r\nWaiting for modem...");
        while (fabs(xtime-timer())<modem_time)
          ;
        pr1(modem_i->hang);
        imodem(0);
        global_xx=0;
      }
    }
  }
}



void imodem(int x)
{
  int i,done;
  char *is;
  char s[81];

  if (!ok_modem_stuff) {
    outs("\x0c");
    return;
  }

  if (x)
    is=modem_i->setu;
  else
    is=modem_i->init;

  if (!(*is))
    return;

  outs("\x0cWaiting...");
  /* begin_crit(); */
  rts(1);
  dtr(1);
  do_result(&(modem_i->defl));
  i=0;
  done=0;
  wait1(9);
  while (!done) {
    initport(syscfgovr.primaryport);
    pr1(is);
    dump();
    if (mode_switch(3.0,0)==mode_norm) {
      done=1;
    } else {
      ++i;
      switch(modem_mode) {
      case 0: strcpy(s, "(No Response)..."); break;
      case mode_ring: strcpy(s, "(Ring)..."); break;
      case mode_dis: strcpy(s, "(Disconnect)..."); break;
      case mode_err: strcpy(s, "(Error)..."); break;
      default: sprintf(s,"(%d)...",modem_mode); break;
      }
      outs(s);
    }
    if (i>5)
      done=1;
  }
  /* end_crit(); */
  outs("\x0c");
}


void answer_phone(void)
{
  double d;

  if (!ok_modem_stuff)
    return;
  cid_num[0]=0;
  cid_name[0]=0;
  outs("Answering phone, 'H' to abort.\r\n");
  do_result(&(modem_i->defl));
  pr1(modem_i->ansr);
  d=timer();
  rts(1);
  if ((mode_switch(45.0,1)!=mode_con) && (modem_mode!=mode_fax)) {
    if (modem_mode == 0) {
      outcomch(' ');
      outcomch('\r');
      wait1(18);
      if (fabs(timer()-d)<modem_time) {
        outs("\r\nWaiting for modem...");
        while (fabs(timer()-d)<modem_time)
          ;
      }
      imodem(0);
      imodem(0);
    } else {
      outs(curspeed);
      imodem(0);
      imodem(0);
    }
  } else {
    incom=outcom=1;
    if (!(modem_flag & flag_ec))
      wait1(72);
    else
      wait1(36);
  }
}


