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
#include "subxtr.h"



#define utoa(s,v,r) ultoa((unsigned long)(s),v,r)


void rename_pend(char *dir, char *file)
{
  char s[81], s1[81], s2[81];
  int i;

  sprintf(s,"%s%s",dir, file);

  if (atoi(file+1))
    strcpy(s2,"P1-");
  else
    strcpy(s2,"P0-");

  for (i=0; i<1000; i++) {
    sprintf(s1,"%s%s%u.NET", dir, s2, i);
    if (!rename(s, s1) || (errno!=EACCES))
      break;
  }
}

/****************************************************************************/

void checkup(struct ftime *f1, struct ftime *f2, int *tf)
{
  if ((f1->ft_year)>(f2->ft_year))
    *tf=1;
  else
    if (f1->ft_year==f2->ft_year)
      if (f1->ft_month>f2->ft_month)
	*tf=1;
      else
	if (f1->ft_month==f2->ft_month)
	  if (f1->ft_day>f2->ft_day)
	    *tf=1;
	  else
	    if (f1->ft_day==f2->ft_day)
	      if (f1->ft_hour>f2->ft_hour)
		*tf=1;
	      else
		if (f1->ft_hour==f2->ft_hour)
		  if (f1->ft_min>f2->ft_min)
		    *tf=1;
		  else
		    if (f1->ft_min==f2->ft_min)
		      if (f1->ft_tsec>f2->ft_tsec)
			*tf=1;
}


int checkup2(struct ftime *f, char *x)
{
  int q;
  char s[81];
  struct ftime f2;

  sprintf(s,"%s%s",net_data,x);
  q=sh_open1(s,O_RDONLY);
  if (q>0) {
    getftime(q,&f2);
    sh_close(q);
    q=0;
    checkup(&f2,f,&q);
  } else
    q=1;
  return(q);
}


int check_bbsdata(void)
{
  char s[81];
  int ok,ok2;
  struct ftime ft;

  sprintf(s,"%sCONNECT.UPD",net_data);
  if ((ok=exist(s))==0) {
    sprintf(s,"%sBBSLIST.UPD",net_data);
    ok=exist(s);
  }
  if (ok  && status.net_edit_stuff) {
    holdphone(1);
    sprintf(s,"NETEDIT .%d /U", net_num);
    extern_prog(s, EFLAG_NETPROG|EFLAG_SHRINK);
  } else {
    sprintf(s,"%sBBSDATA.NET",net_data);
    ok=sh_open1(s,O_RDONLY);
    if (ok>0) {
      getftime(ok,&ft);
      sh_close(ok);
      ok=checkup2(&ft,"BBSLIST.NET")||checkup2(&ft,"CONNECT.NET");
      ok2=checkup2(&ft,"CALLOUT.NET");
    } else
      ok=ok2=1;
  }
  sprintf(s,"%sBBSLIST.NET",net_data);
  if (!exist(s))
    ok=ok2=0;
  sprintf(s,"%sCONNECT.NET",net_data);
  if (!exist(s))
    ok=ok2=0;
  sprintf(s,"%sCALLOUT.NET",net_data);
  if (!exist(s))
    ok=ok2=0;
  if (ok||ok2) {
    if (status.net_version>=33)
      sprintf(s,"NETWORK3%s .%d",(ok?" Y":""), net_num);
    else
      sprintf(s,"NETWORK3%s",(ok?" Y":""));
    holdphone(1);

    extern_prog(s, EFLAG_NETPROG|EFLAG_SHRINK);
    lock_status();
    ++status.filechange[filechange_net];
    save_status();

    zap_call_out_list();
    zap_contacts();
    zap_bbs_list();
    if (ok>=0)
      return(1);
  }
  return(0);
}


void cleanup_net(void)
{
  char s[81],cl[81];
  int ok,ok2,abort,nn,any,f1,nl=0;
  struct ffblk ff;

  if ((net_networks[0].sysnum==0) && (net_num_max==1))
    return;

  any=1;

  while (any && (nl++ < 10)) {
    any=0;

    for (nn=0; nn<net_num_max; nn++) {

      set_net_num(nn);

      if (!net_sysnum)
        continue;

      set_protect(0);
      clrscrb();

      ok2=1;
      abort=0;
      while ((ok2) && (!abort)) {
        ok2=0;
        ok=0;

        sprintf(s,"%sP*.%3.3d",net_data,instance);
        f1=findfirst(s,&ff,0);
        while (f1==0) {
          ok=1;
          rename_pend(net_data, ff.ff_name);
          f1=findnext(&ff);
        }

        if (instance==1) {
          if (!ok) {
            sprintf(s,"%sP*.NET",net_data);
            ok=(findfirst(s,&ff,0)==0);
          }
          if (ok) {
            holdphone(1);
            hangup=0;
            using_modem=0;
            if (status.net_version>=33)
              sprintf(cl,"NETWORK1 .%d",net_num);
            else
              strcpy(cl,"NETWORK1");
            if (extern_prog(cl, EFLAG_NETPROG|EFLAG_SHRINK)<0)
              abort=1;
            else
              any=1;
            ok2=1;
          }
          sprintf(s,"%sLOCAL.NET",net_data);
          if (exist(s)) {
            holdphone(1);
            any=1;
            ok=1;
            hangup=0;
            using_modem=0;
            if (status.net_version>=33)
              sprintf(cl,"NETWORK2 .%d",net_num);
            else
              strcpy(cl,"NETWORK2");
#ifdef OLD
            sprintf(s,"%sNETUP.XXX",net_data);
            if (exist(s))
#endif
              if (extern_prog(cl, EFLAG_NETPROG|EFLAG_SHRINK)<0)
                abort=1;
              else
                any=1;
#ifdef OLD
            else {
              if (extern_prog(cl, EFLAG_NETPROG)<0)
                abort=1;
              else
                any=1;
            }
#endif
            ok2=1;
            read_status();
            curlsub=-1;
            if (wfc) {
              wfc=0;
              read_user(1,&thisuser);
              fwaiting=thisuser.waiting;
              wfc=1;
            }
          }
          if (check_bbsdata())
            ok2=1;
          if (ok2) {
            clrscrb();
            zap_contacts();
          }
        }
      }
    }
  }
  holdphone(0);
}

void do_callout(int sn)
{
  int i,i1,i2;
  char s[81],s1[81];
  long l;
  net_system_list_rec *csne;

  time(&l);
  i=-1;
  i2=-1;
  if (!net_networks[net_num].con)
    read_call_out_list();
  for (i1=0; i1<net_networks[net_num].num_con; i1++)
    if (net_networks[net_num].con[i1].sysnum==sn)
      i=i1;
  if (!net_networks[net_num].ncn)
    read_contacts();
  for (i1=0; i1<net_networks[net_num].num_ncn; i1++)
    if (net_networks[net_num].ncn[i1].systemnumber==sn)
      i2=i1;
  if (i!=-1) {
    csne=next_system(net_networks[net_num].con[i].sysnum);
    if (csne) {
      sprintf(s,"NETWORK /N%u /A%s /P%s /S%u /T%ld",
        sn,
        (net_networks[net_num].con[i].options & options_sendback)?"1":"0",
        csne->phone,
        modem_i->defl.com_speed,
        l);
      if (net_networks[net_num].con[i].macnum) {
        sprintf(s1," /M%d",(int) net_networks[net_num].con[i].macnum);
        strcat(s,s1);
      }
      if (status.net_version>=33) {
        sprintf(s1," .%d",net_num);
        strcat(s,s1);
      }
      if (strncmp(csne->phone,"000",3)) {
        npr("%s%s - ",get_string(910),csne->name);
        if (net_num_max>1)
          npr("%s ",net_name);
        npr("@%u",sn);
        nl();
        describe_area_code(atoi(csne->phone),s1);
        npr("%s%s",get_string(999),s1);
        nl();
        if ((i2!=-1) && (net_networks[net_num].ncn[i2].bytes_waiting)) {
          npr("%s%ldk\r\n", get_string(1520),
              bytes_to_k(net_networks[net_num].ncn[i2].bytes_waiting));
        }
        outstr(get_string(1000));
        pl(s);
        ansic(7);
        pl(charstr(80,205));
        holdphone(1);
        Wait(2.5);
        extern_prog(s, EFLAG_NETPROG|EFLAG_SHRINK);
        zap_contacts();
        read_status();
        last_time_c=l;
        global_xx=0;
        cleanup_net();
        holdphone(0);
        imodem(0);
      }
    }
  }
}


int ok_to_call(int i)
{
  int ok;
  struct time ti;
  char ch,h,l;
  net_call_out_rec *con;

  con=&(net_networks[net_num].con[i]);

  ok=((con->options & options_no_call)==0);
  if (con->options & options_receive_only)
    ok=0;
  ch=dow();
  gettime(&ti);
  if (con->options & options_ATT_night) {
    if ((ch!=0) && (ch!=6)) {
      if ((ti.ti_hour<23) && (ti.ti_hour>=7))
        ok=0;
    }
    if (ch==0) {
      if ((ti.ti_hour<23) && (ti.ti_hour>=16))
        ok=0;
    }
  }
  if (con->options & options_PCP_night) {
    if ((ch!=0) && (ch!=6)) {
      if ((ti.ti_hour<18) && (ti.ti_hour>=7))
        ok=0;
      if ((ti.ti_hour==6) && (ti.ti_min>30))
        ok=0;
    }
  }

  l=con->min_hr;
  h=con->max_hr;
  if ((l>-1) && (h>-1) && (h!=l)) {
    if ((h==0) || (h==24)) {
      if (ti.ti_hour<l)
        ok=0;
      else
        if ((ti.ti_hour==l) && (ti.ti_min<12))
          ok=0;
        else
          if ((ti.ti_hour==23) && (ti.ti_min>30))
            ok=0;
    } else
      if ((l==0) || (l==24)) {
        if (ti.ti_hour>=h)
          ok=0;
        else
          if ((ti.ti_hour==h-1) && (ti.ti_min>30))
            ok=0;
          else
            if ((ti.ti_hour==0) && (ti.ti_min<12))
              ok=0;
      } else
        if (h>l) {
          if ((ti.ti_hour<l) || (ti.ti_hour>=h))
            ok=0;
          else
            if ((ti.ti_hour==l) && (ti.ti_min<12))
              ok=0;
            else
              if ((ti.ti_hour==h-1) && (ti.ti_min>30))
                ok=0;
        } else {
          if ((ti.ti_hour>=h) && (ti.ti_hour<l))
            ok=0;
          else
            if ((ti.ti_hour==l) && (ti.ti_min<12))
              ok=0;
            else
              if ((ti.ti_hour==h-1) && (ti.ti_min>30))
                ok=0;
        }
  }

  return(ok);
}



#define WEIGHT 30.0

void fixup_long(long *f, long l)
{
  if (*f>l)
    *f=l;

  if (*f+(SECONDS_PER_DAY*30L)<l)
    *f=l-SECONDS_PER_DAY*30L;
}

void free_vars(float **weight, int **try)
{
  int nn;

  if (weight || try) {
    for (nn=0; nn<net_num_max; nn++) {
      if (try && try[nn])
        bbsfree(try[nn]);
      if (weight && weight[nn])
        bbsfree(weight[nn]);
    }
    if (try)
      bbsfree(try);
    if (weight)
      bbsfree(weight);
  }
}

void attempt_callout(void)
{
  int **try,i,i1,i2,ok,net_only,nn, num_call_sys, num_ncn,any;
  float **weight,fl,fl1,fl2;
  long l,l1;
  net_call_out_rec *con;
  net_contact_rec *ncn;

  read_status();

  net_only=1;
  if (syscfg.netlowtime!= syscfg.nethightime) {
    if (syscfg.nethightime>syscfg.netlowtime) {
      if ((timer()<=(syscfg.netlowtime*60.0)) || (timer()>=(syscfg.nethightime*60.0)))
        net_only=0;
    } else {
      if ((timer()<=(syscfg.netlowtime*60.0)) && (timer()>=(syscfg.nethightime*60.0)))
        net_only=0;
    }
  } else
    net_only=0;
  time(&l);
  if (last_time_c>l)
    last_time_c=0L;
  if (labs(last_time_c-l)<120)
    return;
  if (last_time_c==0L) {
    last_time_c=l;
    return;
  }

  if ((try=(int **)malloca(sizeof(int *) * net_num_max))==NULL)
    return;
  if ((weight=(float **)malloca(sizeof(float *) * net_num_max))==NULL) {
    bbsfree(try);
    return;
  }
  memset(try, 0, sizeof(int *) * net_num_max);
  memset(weight, 0, sizeof(float *) * net_num_max);

  fl2=0.0;
  any=0;

  for (nn=0; nn<net_num_max; nn++) {
    set_net_num(nn);
    if (!net_sysnum)
      continue;

    /* if (!net_networks[net_num].con) */
    read_call_out_list();
    /* if (!net_networks[net_num].ncn) */
    read_contacts();

    con=net_networks[net_num].con;
    ncn=net_networks[net_num].ncn;
    num_call_sys=net_networks[net_num].num_con;
    num_ncn=net_networks[net_num].num_ncn;

    try[nn]=(int *)malloca(sizeof(int)*num_call_sys);
    if (!try[nn])
      break;
    weight[nn]=(float *)malloca(sizeof(float)*num_call_sys);
    if (!weight[nn])
      break;

    for (i=0; i<num_call_sys; i++) {
      try[nn][i]=0;
      ok=ok_to_call(i);
      i2=-1;
      for (i1=0; i1<num_ncn; i1++)
        if (ncn[i1].systemnumber==con[i].sysnum)
          i2=i1;
      if ((ok) && (i2!=-1)) {
        if (ncn[i2].bytes_waiting==0L)
          if (con[i].call_anyway) {
            l1=(l-ncn[i2].lastcontact+60*3)/SECONDS_PER_HOUR/24;
            if (((unsigned char)l1 < con[i].call_anyway) ||
               ((con[i].options & options_sendback)==0))
              if (!net_only)
                ok=0;
          } else
            ok=0;
        if (con[i].options & options_once_per_day) {
          if (labs(l-ncn[i2].lastcontactsent)<
             (20L*SECONDS_PER_HOUR/con[i].times_per_day))
            ok=0;
        }

        if (ok)
          if ((bytes_to_k(ncn[i2].bytes_waiting)<con[i].min_k)
             && (labs(l-ncn[i2].lastcontact)<SECONDS_PER_DAY))
            ok=0;

        fixup_long((long *)&(ncn[i2].lastcontactsent),l);
        fixup_long((long *)&(ncn[i2].lasttry),l);
        if (ok) {
          if (ncn[i2].bytes_waiting==0L)
            fl=5.0*WEIGHT;
          else
            fl=1024.0/((float)ncn[i2].bytes_waiting)*WEIGHT*60.0;
          fl1=(float) (l-ncn[i2].lasttry);
          if ((fl<fl1) || (net_only)) {
            try[nn][i]=1;
            fl1=fl1/fl;
            if (fl1<1.0)
              fl1=1.0;
            if (fl1>5.0)
              fl1=5.0;
            weight[nn][i]=fl1;
            fl2 += fl1;
            any++;
          }
        }
      }
    }
  }

  if (any) {
    fl=fl2*((float)rand())/32767.0;
    fl1=0.0;
    for (nn=0; nn<net_num_max; nn++) {
      set_net_num(nn);
      if (!net_sysnum)
        continue;

      i1=-1;
      for (i=0; (i<net_networks[net_num].num_con); i++) {
        if (try[nn][i]) {
          fl1 += weight[nn][i];
          if (fl1>=fl) {
            i1=i;
            break;
          }
        }
      }
      if (i1!=-1) {
        free_vars(weight, try);
        weight=NULL;
        try=NULL;
        do_callout(net_networks[net_num].con[i1].sysnum);
        time(&l);
        last_time_c=l;
        break;
      }
    }
  }

  free_vars(weight, try);

  set_net_num(0);
}

void force_callout(int dw)
{
  int i,i1,i2,ok,sn,nn,onxi,odci,nv,nitu,abort=0;
  unsigned int nr=1,tc=0;
  long l;
  char ch,s[101],onx[20],*ss,*ss1,*ss2,*mmk;
  net_system_list_rec *csne;
  unsigned long lc, cc;

  if ((instance!=1) && (status.net_version <34))
    return;

  time(&l);
  nl();
  prt(2,get_string(911));
  mpl(5);
  input(s,5);
  sn=atoi(s);
  if (!sn)
    return;

  odc[0]=0;
  odci=0;
  onx[0]='Q';
  onx[1]=0;
  onxi=1;
  nv=0;
  ss=malloca(net_num_max*3);
  ss1=ss+net_num_max;
  ss2=ss1+net_num_max;

  for (nn=0; nn<net_num_max; nn++) {
    set_net_num(nn);
    if ((!net_sysnum) || (net_sysnum==sn))
      continue;

    if (!net_networks[net_num].con)
      read_call_out_list();

    i=-1;
    for (i1=0; i1<net_networks[net_num].num_con; i1++) {
      if (net_networks[net_num].con[i1].sysnum == sn) {
        i=i1;
        break;
      }
    }

    if (i!=-1) {
      if (!net_networks[net_num].ncn)
        read_contacts();

      i2=-1;
      for (i1=0; i1<net_networks[net_num].num_ncn; i1++) {
        if (net_networks[net_num].ncn[i1].systemnumber==sn) {
          i2=i1;
          break;
        }
      }

      if (i2!=-1) {
        ss[nv]=nn;
        ss1[nv]=i;
        ss2[nv++]=i2;
      }
    }
  }

  nitu=-1;
  if (nv) {
    if (nv==1)
      nitu=0;
    else {
      nl();
      for (i=0; i<nv; i++) {
        set_net_num(ss[i]);
        csne=next_system(sn);
        if (csne) {
          if (i<9) {
            onx[onxi++]=i+'1';
            onx[onxi]=0;
          } else {
            odci=(i+1)/10;
            odc[odci-1]=odci+'0';
            odc[odci]=0;
          }
          npr("%d. %s (%s)\r\n",i+1,net_name, csne->name);
        }
      }
      pl(get_string(12));
      nl();
      prt(2,get_string(13));
      itoa(i+1,s,10);
      mpl(strlen(s));
      if (nv<9) {
        ch=onek(onx);
        if (ch!='Q')
          nitu=ch-'1';
      } else {
        mmk=mmkey(2);
        if (*mmk!='Q')
          nitu=atoi(mmk)-1;
      }
    }
  }

  if (nitu!=-1) {
    set_net_num(ss[nitu]);
    ok=ok_to_call(ss1[nitu]);

    if (!ok) {
      nl();
      prt(5,get_string(465));
      if (yn())
        ok=1;
    }
    if (ok) {
      if (net_networks[net_num].ncn[ss2[nitu]].bytes_waiting==0L)
        if (!(net_networks[net_num].con[ss1[nitu]].options & options_sendback))
          ok=0;
      if (ok) {
        if (dw) {
          nl();
          prt(2,get_string(968));
          mpl(5);
          input(s,5);
          nr=atoi(s);
        }
        if(dw==2) {
          sl1(1,"");
          if(useron) {
            write_user(usernum,&thisuser);
            write_qscn(usernum,qsc,0);
            useron=0;
          }
          hang_it_up();
          wait1(90);
        }
        if ((!dw) || (nr<1))
          nr=1;

        read_contacts();
        lc=net_networks[net_num].ncn[ss2[nitu]].lastcontact;
        while ((tc<nr) && (!abort)) {
          if (kbhitb()) {
            while (kbhitb()) {
              ch=upcase(getchd1());
              if (!abort)
                abort=(ch==27);
            }
          }
          tc++;
          set_net_num(ss[nitu]);
          read_contacts();
          cc=net_networks[net_num].ncn[ss2[nitu]].lastcontact;
          if ((abort) || (cc!=lc)) {
            break;
          } else {
            clrscrb();
            sprintf(s,"%s%d%s%d%s%d%s",
              get_string(969),
              nr,
              get_string(977),
              tc,
              get_string(978),
              nr-tc,
              get_string(979));
            pl(s);
            if (nr==tc) {
              bbsfree(ss);
              ss=NULL;
            }
            do_callout(sn);
          }
        }
      }
    }
  }
  if (ss)
    bbsfree(ss);
}


void print_pending_list(void)
{
  int i,i1,i2,nn,num_ncn, num_call_sys,abort=0;
  char s[100],ch,s1[100];
  long l,l2;
  unsigned long ss;
  net_call_out_rec *con;
  net_contact_rec *ncn;

  if ((net_networks[0].sysnum==0) && (net_num_max==1))
    return;

  read_status();

  time(&l);
  ss=thisuser.sysstatus;
  thisuser.sysstatus |= sysstatus_pause_on_page;

  nln(2);
  pla(get_string(912),&abort);
  if (net_num_max>1) {
    pla(get_string(913),&abort);
    pla(get_string(914),&abort);
    pla(get_string(961),&abort);
  } else {
    pla(get_string(915),&abort);
    pla(get_string(916),&abort);
    pla(get_string(962),&abort);
  }

  for (nn=0; nn<net_num_max; nn++) {

    set_net_num(nn);

    if (!net_sysnum)
      continue;

    if ((!net_networks[net_num].con) || (instance!=1))
      read_call_out_list();
    read_contacts();

    con=net_networks[net_num].con;
    ncn=net_networks[net_num].ncn;
    num_call_sys=net_networks[net_num].num_con;
    num_ncn=net_networks[net_num].num_ncn;

    for (i1=0; i1<num_call_sys; i1++) {
      i2=-1;
      for (i=0; i<num_ncn; i++) {
        if (con[i1].sysnum==ncn[i].systemnumber) {
          i2=i;
          break;
        }
      }
      if (i2!=-1) {
        if (ok_to_call(i1)) {
          if (ok_modem_stuff) {
            if ((bytes_to_k(ncn[i2].bytes_waiting)>=con[i1].min_k) ||
               (labs(l-ncn[i2].lastcontact)>=SECONDS_PER_DAY)) {
                ch='*';
            } else {
              ch='+';
            }
          } else {
            ch='-';
          }
        } else {
          ch=' ';
        }
        if (ncn[i2].lastcontactsent) {
          sprintf(s1,"%4ld:",(l-ncn[i2].lastcontactsent)/SECONDS_PER_HOUR);
          l2=(((l-ncn[i2].lastcontactsent)%SECONDS_PER_HOUR)/60);
          itoa(l2,s,10);
          if (l2<10) {
            strcat(s1,"0");
            strcat(s1,s);
          } else
            strcat(s1,s);
        } else
          strcpy(s1,get_string(917));
        if (net_num_max>1) {
          sprintf(s,"%s7³5 %c%s%-11.11s%s%-6u%s%6ldk%s%11s 7³",
            charstr(14,' '),
            ch,
            get_string(974),
            net_name,
            get_string(974),
            ncn[i2].systemnumber,
            get_string(974),
            bytes_to_k(ncn[i2].bytes_waiting),
            get_string(974),
            s1);
        } else {
          sprintf(s,"%s7³5 %c%s%-6u%s%6ldk%s%11s 7³",
            charstr(21,' '),
            ch,
            get_string(974),
            ncn[i2].systemnumber,
            get_string(974),
            bytes_to_k(ncn[i2].bytes_waiting),
            get_string(974),
            s1);
        }
        pla(s,&abort);
      }
    }
  }

  for (nn=0; nn<net_num_max; nn++) {

    set_net_num(nn);

    if (!net_sysnum)
      continue;

    sprintf(s,"%sDEAD.NET",net_data);
    i=sh_open1(s,O_RDONLY | O_BINARY);
    if (i>0) {
      l=filelength(i);
      sh_close(i);
      if (net_num_max>1) {
        sprintf(s,
          "%s%s%-11.11s%s%6ldk%s",
          charstr(14,' '),
          get_string(966),
          net_name,
          get_string(967),
          bytes_to_k(l),
          get_string(964));
      } else {
        sprintf(s,
          "%s%s%6ldk%s",
          charstr(21,' '),
          get_string(965),
          bytes_to_k(l),
          get_string(964));
      }
      pla(s,&abort);
    }
  }
  for (nn=0; nn<net_num_max; nn++) {

    set_net_num(nn);

    if (!net_sysnum)
      continue;

    sprintf(s,"%sCHECK.NET",net_data);
    i=sh_open1(s,O_RDONLY | O_BINARY);
    if (i>0) {
      l=filelength(i);
      sh_close(i);
      if (net_num_max>1) {
        sprintf(s,
          "%s%s%-11.11s%s%6ldk",
          charstr(14,' '),
          get_string(966),
          net_name,
          get_string(967),
          bytes_to_k(l));
        strcat(s,get_string(1242));
      } else {
        sprintf(s,
          "%s%s%6ldk",
          charstr(21,' '),
          get_string(965),
          bytes_to_k(l));
        strcat(s,get_string(1242));
      }
      pla(s,&abort);
    }
  }


  if (net_num_max>1)
    pla(get_string(918),&abort);
  else
    pla(get_string(963),&abort);
  nl();
  thisuser.sysstatus=ss;
  if((!useron) && (lines_listed))
    pausescr();
}

/****************************************************************************/


void gate_msg(net_header_rec *nh, char *text, unsigned short nn, char *byname, unsigned int *list, unsigned short fnn)
{
  char newname[256], fn[100], qn[200], on[200];
  char *ti, nm[205], *ss;
  int f,i;
  unsigned short ntl;

  if (strlen(text)<80) {
    ti=text;
    text+=strlen(ti)+1;
    ntl=nh->length-strlen(ti)-1;
    ss=strchr(text,'\r');
    if (ss && (ss-text<200) && (ss-text<ntl)) {
      strncpy(nm,text,ss-text);
      nm[ss-text]=0;
      ss++;
      if (*ss=='\n')
        ss++;
      nh->length-=(ss-text);
      ntl-=(ss-text);
      text=ss;

      qn[0]=on[0]=0;

      if ((fnn==65535) || (nh->fromsys == net_networks[fnn].sysnum)) {

        strcpy(newname,nm);
        ss=strrchr(newname,'@');
        if (ss) {
          sprintf(ss+1,"%u",net_networks[nn].sysnum);
          ss=strrchr(nm,'@');
          if (ss) {
            ++ss;
            while ((*ss>='0') && (*ss<='9'))
              ++ss;
            strcat(newname,ss);
          }
          strcat(newname,"\r\n");
          nh->fromsys = net_networks[nn].sysnum;
        }
      } else {
        if ((nm[0]=='`') && (nm[1]=='`')) {
          for (i=strlen(nm)-2; i>0; i--) {
            if ((nm[i]=='`') && (nm[i+1]=='`')) {
              break;
            }
          }
          if (i>0) {
            i+=2;
            strncpy(qn,nm,i);
            qn[i]=' ';
            qn[i+1]=0;
          }
        } else
          i=0;
        if (qn[0]==0) {
          ss=strrchr(nm,'#');
          if (ss) {
            if ((ss[1]>='0') && (ss[1]<='9')) {
              *ss=0;
              ss--;
              while ((ss>nm) && (*ss==' ')) {
                *ss=0;
                ss--;
              }
            }
          }
          if (nm[0]) {
            if (nh->fromuser)
              sprintf(qn,"``%s`` ",nm);
            else
              strcpy(on,nm);
          }
        }
        if ((on[0]==0) && (nh->fromuser==0)) {
          strcpy(on,nm+i);
        }
        if (status.net_version>=35) {
          if (on[0])
            sprintf(newname, "%s%s@%u.%s\r\n", qn, on, nh->fromsys,
                    net_networks[fnn].name);
          else
            sprintf(newname, "%s#%u@%u.%s\r\n", qn, nh->fromuser, nh->fromsys,
                    net_networks[fnn].name);
        } else {
          if (on[0])
            sprintf(newname,"%s%s %s AT %u\r\n",qn, net_networks[fnn].name,
                    on, nh->fromsys);
          else
            sprintf(newname,"%s%s #%u AT %u\r\n",qn, net_networks[fnn].name,
                    nh->fromuser, nh->fromsys);
        }

        nh->fromsys=net_networks[nn].sysnum;
        nh->fromuser=0;
      }


      nh->length += strlen(newname);
      if ((nh->main_type == main_type_email_name) ||
          (nh->main_type == main_type_new_post))
        nh->length += strlen(byname)+1;
      sprintf(fn,"%sP1%s",net_networks[nn].dir,nete);
      f=sh_open(fn,O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
      if (f) {
        sh_lseek(f,0L,SEEK_END);
        if (!list)
          nh->list_len=0;
        if (nh->list_len)
          nh->tosys=0;
        sh_write(f,nh, sizeof(net_header_rec));
        if (nh->list_len)
          sh_write(f,list,2*(nh->list_len));
        if ((nh->main_type == main_type_email_name) ||
            (nh->main_type == main_type_new_post))
          sh_write(f,byname, strlen(byname)+1);
        sh_write(f,ti,strlen(ti)+1);
        sh_write(f,newname,strlen(newname));
        sh_write(f,text,ntl);
        sh_close(f);
      }
    }
  }
}
