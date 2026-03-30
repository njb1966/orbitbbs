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


void zap_call_out_list(void)
{
  if (net_networks[net_num].con) {
    bbsfree((void *)net_networks[net_num].con);
    net_networks[net_num].con=NULL;
    net_networks[net_num].num_con=0;
  }
}


void read_call_out_list(void)
{
  char s[161],*ss;
  int f,i,i1;
  long l,p;
  net_call_out_rec *con;

  zap_call_out_list();

  sprintf(s,"%sCALLOUT.NET",net_data);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    if ((ss=malloca(l+512))==NULL)
      exit(noklevel);
    sh_read(f,(void *)ss,(int) l);
    sh_close(f);
    for (p=0L; p<l; p++)
      if (ss[p]=='@')
        ++net_networks[net_num].num_con;
    bbsfree(ss);
    if ((net_networks[net_num].con =
      (net_call_out_rec *)malloca((long) ((net_networks[net_num].num_con+2) *
                      sizeof(net_call_out_rec)))) == NULL)
      exit(noklevel);
    con=net_networks[net_num].con;
    con--;
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if ((ss=malloca(l+512))==NULL)
      exit(noklevel);
    sh_read(f,(void *)ss,(int) l);
    sh_close(f);
    p=0L;
    while (p<l) {
      while ((p<l) && (strchr("@%/\"&-=+~!();^|#$",ss[p])==NULL))
        ++p;
      if (p<l) {
        switch(ss[p]) {
          case '@':
            ++p;
            con++;
            con->macnum=0;
            con->options=0;
            con->call_anyway=0;
            con->password[0]=0;
            con->sysnum=atoi(&(ss[p]));
            con->min_hr=-1;
            con->max_hr=-1;
            con->times_per_day=0;
            con->min_k=0;
            con->call_x_days=0;
            break;
          case '&':
            con->options |= options_sendback;
            ++p;
            break;
          case '-':
            con->options |= options_ATT_night;
            ++p;
            break;
          case '=':
            con->options |= options_PCP_night;
            ++p;
            break;
          case '+':
            con->options |= options_no_call;
            ++p;
            break;
          case '~':
            con->options |= options_receive_only;
            ++p;
            break;
          case '!':
            con->options |= options_once_per_day;
            ++p;
            con->times_per_day=atoi(&(ss[p]));
            if (!con->times_per_day)
              con->times_per_day=1;
            break;
          case '%':
            ++p;
            con->macnum=(unsigned char) atoi(&(ss[p]));
            break;
          case '/':
            ++p;
            con->call_anyway=(unsigned char) atoi(&(ss[p]));
            break;
          case '#':
            ++p;
            con->call_x_days=(unsigned char) atoi(&(ss[p]));
            break;
          case '(':
            ++p;
            con->min_hr = (unsigned char) atoi(&(ss[p]));
            break;
          case ')':
            ++p;
            con->max_hr = (unsigned char) atoi(&(ss[p]));
            break;
          case '|':
            ++p;
            con->min_k=atoi(&(ss[p]));
            if (!con->min_k)
              con->min_k=0;
            break;
          case ';':
            ++p;
            con->options |= options_compress;
            break;
          case '^':
            ++p;
            con->options |= options_hslink;
            break;
          case '$':
            ++p;
            con->options |= options_force_ac;
            break;
          case '\"':
            ++p;
            i=0;
            while ((i<19) && (ss[p+(long)i]!='\"'))
              ++i;
            for (i1=0; i1<i; i1++)
              con->password[i1]=ss[p+(long)i1];
            con->password[i]=0;
            p+=(long)(i+1);
            break;
        }
      }
    }
    bbsfree(ss);
  }
}


int bbs_list_net_no=-1;


void zap_bbs_list(void)
{
  if (csn) {
    bbsfree((void far *)csn);
    csn=NULL;
  }
  if (csn_index) {
    bbsfree(csn_index);
    csn_index=NULL;
  }
  num_sys_list=0;
  bbs_list_net_no=-1;
}

void read_bbs_list(void)
{
  char s[161];
  int f,i;
  long l;

  zap_bbs_list();

  if (net_sysnum==0)
    return;
  sprintf(s,"%sBBSDATA.NET",net_data);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    num_sys_list=(int) (l/sizeof(net_system_list_rec));
    if ((csn=malloca(l+512L))==NULL)
      exit(noklevel);
    for (i=0; i<num_sys_list; i+=256) {
      sh_read(f,(void *)&(csn[i]),256*sizeof(net_system_list_rec));
    }
    sh_close(f);
  }
  bbs_list_net_no=net_num;
}


void read_bbs_list_index(void)
{
  char s[161];
  int f;
  long l;

  zap_bbs_list();

  if (net_sysnum==0)
    return;
  sprintf(s,"%sBBSDATA.IND",net_data);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    num_sys_list=(int) (l/2);
    if ((csn_index=malloca(l))==NULL)
      exit(noklevel);
    sh_read(f,(void *)csn_index,l);
    sh_close(f);
  } else
    read_bbs_list();
  bbs_list_net_no=net_num;
}


int system_index(unsigned int ts)
{
  int i;

  if (bbs_list_net_no != net_num)
    read_bbs_list_index();

  if (csn) {
    for (i=0; i<num_sys_list; i++)
      if ((csn[i].sysnum==ts) && (csn[i].forsys!=65535))
        return(i);
  } else {
    for (i=0; i<num_sys_list; i++)
      if (csn_index[i]==ts)
        return(i);
  }
  return(-1);
}


int valid_system(unsigned int ts)
{
  if (system_index(ts)==-1)
    return(0);
  else
    return(1);
}

net_system_list_rec *next_system(unsigned int ts)
{
  int i,f;
  static net_system_list_rec csne;
  char s[81];

  i=system_index(ts);

  if (i==-1) {
    return(NULL);
  } else if (csn) {
    return((net_system_list_rec *) &(csn[i]));
  } else {
    sprintf(s,"%sBBSDATA.NET",net_data);
    f=sh_open1(s,O_RDONLY | O_BINARY);
    sh_lseek(f,sizeof(net_system_list_rec)*((long)i),SEEK_SET);
    sh_read(f,&csne,sizeof(net_system_list_rec));
    sh_close(f);
    if (csne.forsys==65535)
      return(NULL);
    else
      return(&csne);
  }
}



void zap_contacts(void)
{
  if (net_networks[net_num].ncn) {
    bbsfree(net_networks[net_num].ncn);
    net_networks[net_num].ncn=NULL;
    net_networks[net_num].num_ncn=0;
  }
}

void read_contacts(void)
{
  int f;
  char s[81];
  long l;

  zap_contacts();

  sprintf(s,"%sCONTACT.NET",net_data);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>=0) {
    l=filelength(f);
    net_networks[net_num].num_ncn=(int) (l/sizeof(net_contact_rec));
    if ((net_networks[net_num].ncn=
      malloca((net_networks[net_num].num_ncn+2)*sizeof(net_contact_rec)))==NULL)
      exit(noklevel);
    sh_lseek(f,0L,SEEK_SET);
    sh_read(f,(void *)net_networks[net_num].ncn,
         net_networks[net_num].num_ncn*sizeof(net_contact_rec));
    sh_close(f);
  }
}


void set_net_num(int n)
{
  if ((n>=0) && (n<net_num_max)) {
    net_num=n;
    net_name=net_networks[net_num].name;
    net_data=net_networks[net_num].dir;
    net_sysnum=net_networks[net_num].sysnum;
    net_type=net_networks[net_num].type;
    sprintf(wwiv_net_no,"ORBIT_NET=%d",net_num);
  }
}

