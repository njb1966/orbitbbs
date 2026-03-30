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

#include "subxtr.h"


void save_subs(void)
{
  char s[100],s1[100],s2[100];
  int f,i,i1,i2,onn;
  FILE *fp, *al, *su, *nn;
  xtrasubsnetrec *xnp;

  onn=net_num;

  for (i=0; i<num_subs; i++) {
    subboards[i].type=0;
    subboards[i].age &= 0x7f;
    if (status.net_version<32) {
      if (xsubs[i].num_nets) {
        subboards[i].type=xsubs[i].nets[0].type;
        subboards[i].age |= 0x80;
        subboards[i].name[40]=xsubs[i].nets[0].net_num;
        subboards[i].name[39]=0;
      }
    }
  }

  sprintf(s,"%sSUBS.DAT",syscfg.datadir);
  f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f<0) {
    pl(get_string(164));
    pausescr();
  } else {
    i=sh_write(f,(void *)&subboards[0], num_subs * sizeof(subboardrec));
    if (i!=(num_subs*sizeof(subboardrec))) {
      pl(get_string(165));
      pausescr();
    }
    sh_close(f);
  }

  sprintf(s,"%sSUBS.XTR",syscfg.datadir);
  unlink(s);
  fp=fsh_open(s,"w");
  if (fp) {
    for (i=0; i<num_subs; i++) {
      if (xsubs[i].num_nets) {
        fprintf(fp,"!%u\n@%s\n#%lu\n",i,
          xsubs[i].desc,(xsubs[i].flags&XTRA_MASK));
        for (i1=0; i1<xsubs[i].num_nets; i1++) {
          fprintf(fp,"$%s %s %lu %u %u\n",
            net_networks[xsubs[i].nets[i1].net_num].name,
            xsubs[i].nets[i1].stype,
            xsubs[i].nets[i1].flags,
            xsubs[i].nets[i1].host,
            xsubs[i].nets[i1].category);
        }
      }
    }
    fsh_close(fp);
  }

  for (i=0; i<net_num_max; i++) {
    set_net_num(i);

    sprintf(s,"%sALLOW.NET", net_data); unlink(s);
    sprintf(s1,"%sSUBS.PUB", net_data); unlink(s1);
    sprintf(s2,"%sNNALL.NET", net_data); unlink(s2);

    if (status.net_version<32) {
      al=su=nn=NULL;
      for (i1=0; i1<num_subs; i1++) {
        for (i2=0,xnp=xsubs[i1].nets; i2<xsubs[i1].num_nets; i2++,xnp++) {
          if (xnp->net_num==net_num) {
            if (xnp->flags & XTRA_NET_AUTO_ADDDROP) {
              if (!al)
                al=fsh_open(s,"w");
              if (al)
                fprintf(al,"%s\n",xnp->stype);
            }
            if (xnp->flags & XTRA_NET_AUTO_INFO) {
              if (!su)
                su=fsh_open(s1,"w");
              if (s)
                fprintf(su,"%s\n",xnp->stype);
            }
            if (xnp->host) {
              if (!nn)
                nn=fsh_open(s2,"w");
              if (nn)
                fprintf(nn,"%-7s %-6u          %s\n",xnp->stype, xnp->host,
                        xsubs[i1].desc?xsubs[i1].desc:subboards[i1].name);
            }
          }
        }
      }
      if (al) fsh_close(al);
      if (su) fsh_close(su);
      if (nn) fsh_close(nn);
    }
  }

  set_net_num(onn);
}

/****************************************************************************/

void boarddata(int n, char *s)
{
  char x,y,k,i;
  subboardrec r;

  r=subboards[n];
  if (r.ar==0)
    x=32;
  else {
    for (i=0; i<16; i++)
      if ((1 << i) & r.ar)
        x='A'+i;
  }
  switch(r.anony & 0x0f) {
    case 0: y='N'; break;
    case anony_enable_anony: y='Y'; break;
    case anony_enable_dear_abby: y='D'; break;
    case anony_force_anony: y='F'; break;
    case anony_real_name: y='R'; break;
  }
  if (r.key==0)
    k=32;
  else
    k=r.key;
  sprintf(s,"%4d %1c  %-37.37s %-8s %-3d %-3d %-2d %-5d %7s",
            n,x,stripcolors(r.name),r.filename,r.readsl,r.postsl,r.age&0x7f,
            r.maxmsgs,xsubs[n].num_nets?xsubs[n].nets[0].stype:"");
  x=k; x=y; /* leave in old code but ignore warning */
}

void showsubs(void)
{
  int abort,i;
  char s[180];

  outchr(12);
  abort=0;
  pla(get_string(166),
      &abort);
  pla(get_string(167),
      &abort);
  for (i=0; (i<num_subs) && (!abort); i++) {
    subboards[i].anony &= ~anony_require_sv;
    boarddata(i,s);
    pla(s,&abort);
  }
}



void modify_sub(int n)
{
  subboardrec r;
  char s[81],s1[81],s2[81],ch,ch2,ch3;
  int i,done;
  xtrasubsnetrec *xnp;

  r=subboards[n];
  done=0;
  do {
    outchr(12);
    outstr(get_string(62)); pl(r.name);
    outstr(get_string(63)); pl(r.filename);
    if (r.key==0)
      strcpy(s,get_string(5));
    else {
      s[0]=r.key;
      s[1]=0;
    }
    outstr(get_string(169)); pl(s);
    outstr(get_string(170)); pln(r.readsl);
    outstr(get_string(171)); pln(r.postsl);
    switch(r.anony & 0x0f) {
      case 0: strcpy(s,str_no); break;
      case anony_enable_anony: strcpy(s,str_yes); break;
      case anony_enable_dear_abby: strcpy(s,get_string(172)); break;
      case anony_force_anony: strcpy(s,get_string(173)); break;
      case anony_real_name: strcpy(s,get_string(174)); break;
      default: strcpy(s,get_string(175)); break;
    }
    outstr(get_string(176)); pl(s);
    outstr(get_string(177)); pln(r.age&0x7f);
    outstr(get_string(178)); pln(r.maxmsgs);
    strcpy(s,get_string(5));
    if (r.ar!=0) {
      for (i=0; i<16; i++)
        if ((1 << i) & r.ar)
          s[0]='A'+i;
      s[1]=0;
    }
    outstr(get_string(179)); pl(s);
    outstr(get_string(180));
    if (xsubs[n].num_nets) {
      npr("\r\n      %-12.12s %-7.7s %-6.6s  %s\r\n",
          get_string(181), get_string(182), get_string(183), get_string(184));
      for (i=0,xnp=xsubs[n].nets; i<xsubs[n].num_nets; i++,xnp++) {
        if (xnp->host==0)
          strcpy(s,get_string(185));
        else
          sprintf(s,"%u ",xnp->host);
        if (xnp->category)
          sprintf(s1,"%s(%d)",get_string(187),xnp->category);
        else
          strcpy(s1,get_string(187));
        npr("   %c) %-12.12s %-7.7s %-6.6s  %s%s\r\n",
            i+'a',
            net_networks[xnp->net_num].name,
            xnp->stype,
            s,
            (xnp->flags&XTRA_NET_AUTO_ADDDROP)?get_string(186):"",
            (xnp->flags&XTRA_NET_AUTO_INFO)?s1:"");
      }
    } else {
      pl(get_string(188));
    }

    outstr(get_string(189)); pln(r.storage_type);
    outstr(get_string(190)); pl((r.anony & anony_val_net)?str_yes:str_no);
    outstr(get_string(191)); pl((r.anony & anony_ansi_only)?str_yes:str_no);
    outstr(get_string(192)); pl((r.anony & anony_no_tag)?str_yes:str_no);
    outstr(get_string(193)); pl((xsubs[n].desc[0])?xsubs[n].desc:get_string(5));
    nl();
    prt(2,get_string(194));
    ch=onek("QABCDEFGHIJKLMNO[]");
    switch(ch) {
      case 'Q':done=1; break;
      case '[':
        subboards[n]=r;
        if (--n<0)
          n=num_subs-1;
        r=subboards[n];
        break;
      case ']':
        subboards[n]=r;
        if (++n>=num_subs)
          n=0;
        r=subboards[n];
        break;
      case 'A':
        nl();
        prt(2,get_string(69));
        if (status.net_version<32)
          inputl(s,39);
        else
          inputl(s,40);
        if (s[0])
          strcpy(r.name,s);
        break;
      case 'B':
        nl();
        prt(2,get_string(72));
        inputf(s,8);
        if ((s[0]!=0) && (strchr(s,'.')==0)) {
          sprintf(s2,"%s",r.filename);
          strcpy(r.filename,s);
          sprintf(s,"%s%s.SUB",syscfg.datadir,r.filename);
          sprintf(s1,"%s%s.DAT",syscfg.msgsdir,r.filename);
          if ((r.storage_type==2) && (!exist(s)) && (!exist(s1)) &&
            (strcmp(r.filename,"NONAME")!=0)) {
            prt(2,get_string(973));
            if (yn()) {
              sprintf(s,"%s%s.SUB",syscfg.datadir,s2);
              sprintf(s1,"%s%s.SUB",syscfg.datadir,r.filename);
              rename(s,s1);
              sprintf(s,"%s%s.DAT",syscfg.msgsdir,s2);
              sprintf(s1,"%s%s.DAT",syscfg.msgsdir,r.filename);
              rename(s,s1);
            }
          }
        }
        break;
      case 'C':
        nl();
        prt(2,get_string(195));
        ch2=onek("@%^&()_=\\|;:'\",` ");
        if (ch2==32)
          r.key=0;
        else
          r.key=ch2;
        break;
      case 'D':
        nl();
        prt(2,get_string(196));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<256) && (s[0]))
          r.readsl=i;
        break;
      case 'E':
        nl();
        prt(2,get_string(197));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<256) && (s[0]))
          r.postsl=i;
        break;
      case 'F':
        nl();
        prt(2,get_string(198));
        strcpy(s,"NYDFR");
        s[0]=str_no[0];
        s[1]=str_yes[0];
        ch2=onek(s);
        if (ch2==str_no[0])
          ch2=0;
        else if (ch2==str_yes[0])
          ch2=1;
        r.anony &= 0xf0;
        switch(ch2) {
          case 0: break;
          case 1: r.anony |= anony_enable_anony; break;
          case 'D': r.anony |= anony_enable_dear_abby; break;
          case 'F': r.anony |= anony_force_anony; break;
          case 'R': r.anony |= anony_real_name; break;
        }
        break;
      case 'G':
        nl();
        prt(2,get_string(77));
        input(s,3);
        i=atoi(s);
        if ((i>=0) && (i<128) && (s[0]))
          r.age=(r.age&0x80)|(i&0x7f);
        break;
      case 'H':
        nl();
        prt(2,get_string(199));
        input(s,5);
        i=atoi(s);

        if (i>254) {
          if ((status.net_version<34) && (xsubs[n].num_nets)) {
            pl(get_string(1433));
            pausescr();
            break;
          }
        }

        if ((i>0) && (i<16384) && (s[0])) {
          r.maxmsgs=i;
        }
        break;
      case 'I':
        nl();
        prt(2,get_string(80));
        ch2=onek("ABCDEFGHIJKLMNOP ");
        if (ch2==32)
          r.ar=0;
        else
          r.ar=1 << (ch2-'A');
        break;
      case 'J':
        subboards[n]=r;
        if (xsubs[n].num_nets) {
          nl();
          prt(2,get_string(200));
          ch2=onek("QAMD");
        } else
          ch2='A';

        if (ch2=='A') {
          if ((status.net_version<32) && (xsubs[n].num_nets)) {
            nl();
            pl(get_string(201));
            nl();
            pausescr();
          } else if ((status.net_version<34) && (r.maxmsgs>254)) {
            nl();
            pl(get_string(1433));
            pl(get_string(1434));
            nl();
            pausescr();
          } else
            sub_xtr_add(n, -1);
        } else if ((ch2=='D') || (ch2=='M')) {
          nl();
          ansic(2);
          if (ch2=='D')
            outstr(get_string(202));
          else
            outstr(get_string(203));
          npr("%c",'a'+xsubs[n].num_nets-1);
          outstr(get_string(204));
          s[0]=' ';
          for (i=0; i<xsubs[n].num_nets; i++)
            s[i+1]='A'+i;
          s[i+1]=0;
          ansic(0);
          ch3=onek(s);
          if (ch3!=' ') {
            i=ch3-'A';
            if ((i>=0) && (i<xsubs[n].num_nets)) {
              if (ch2=='D') {
                sub_xtr_del(n, i, 1);
              } else {
                sub_xtr_del(n, i, 0);
                sub_xtr_add(n, i);
              }
            }
          }
        }
        r=subboards[n];
        break;
      case 'K':
        nl();
        prt(2,get_string(205));
        input(s,4);
        i=atoi(s);
        if ((s[0]) && (i>=0) && (i<=2))
          r.storage_type=i;
        break;
      case 'L':
        nl();
        prt(5,get_string(206));
        r.anony &= ~anony_val_net;
        if (yn())
          r.anony |= anony_val_net;
        break;
      case 'M':
        nl();
        prt(5,get_string(207));
        r.anony &= ~anony_ansi_only;
        if (yn())
          r.anony |= anony_ansi_only;
        break;
      case 'N':
        nl();
        prt(5,get_string(208));
        r.anony &= ~anony_no_tag;
        if (yn())
          r.anony |= anony_no_tag;
        break;
      case 'O':
        nl();
        prt(2,get_string(209));
        inputl(s,60);
        if (s[0])
          strcpy(xsubs[n].desc,s);
        else {
          nl();
          prt(2,get_string(994));
          if (yn())
            xsubs[n].desc[0]=0;
        }
        break;
    }
  } while ((!done) && (!hangup));
  subboards[n]=r;
}


void swap_subs(int sub1, int sub2)
{
  int i,i1,i2,nu;
  unsigned long *qsc, *qsc_p, *qsc_n, *qsc_q, tl;
  subboardrec sbt;
  xtrasubsrec xst;
  SUBCONF_TYPE sub1conv,sub2conv;

  sub1conv=(SUBCONF_TYPE) sub1;
  sub2conv=(SUBCONF_TYPE) sub2;

  if ((sub1<0) || (sub1>=num_subs) || (sub2<0) || (sub2>=num_subs))
    return;

  update_conf(CONF_SUBS, &sub1conv, &sub2conv, CONF_UPDATE_SWAP);

  sub1=(int) sub1conv;
  sub2=(int) sub2conv;

  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len);
  if (qsc) {
    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc,1);
      qsc_n=qsc+1;
      qsc_q=qsc_n+(max_dirs+31)/32;
      qsc_p=qsc_q+(max_subs+31)/32;

      if (qsc_q[sub1/32] & (1L<<(sub1%32)))
        i1=1;
      else
        i1=0;
      if (qsc_q[sub2/32] & (1L<<(sub2%32)))
        i2=1;
      else
        i2=0;
      if (i1+i2==1) {
        qsc_q[sub1/32] ^= (1L<<(sub1%32));
        qsc_q[sub2/32] ^= (1L<<(sub2%32));
      }

      tl=qsc_p[sub1];
      qsc_p[sub1]=qsc_p[sub2];
      qsc_p[sub2]=tl;

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  }

  sbt=subboards[sub1];
  subboards[sub1]=subboards[sub2];
  subboards[sub2]=sbt;

  tl=sub_dates[sub1];
  sub_dates[sub1]=sub_dates[sub2];
  sub_dates[sub2]=tl;

  xst=xsubs[sub1];
  xsubs[sub1]=xsubs[sub2];
  xsubs[sub2]=xst;

  save_subs();
}


void insert_sub(int n)
{
  subboardrec r;
  int i,i1,i2,nu;
  unsigned long *qsc, *qsc_n, *qsc_q, *qsc_p, m1, m2, m3;
  SUBCONF_TYPE nconv;

  nconv=(SUBCONF_TYPE) n;

  if ((n<0) || (n>num_subs))
    return;

  update_conf(CONF_SUBS, &nconv, NULL, CONF_UPDATE_INSERT);

  n=(int) nconv;

  for (i=num_subs-1; i>=n; i--) {
    subboards[i+1]=subboards[i];
    sub_dates[i+1]=sub_dates[i];
    xsubs[i+1]=xsubs[i];
  }
  strcpy(r.name,get_string(210));
  strcpy(r.filename,get_string(82));
  r.key=0;
  r.readsl=10;
  r.postsl=20;
  r.anony=0;
  r.age=0;
  r.maxmsgs=50;
  r.ar=0;
  r.type=0;
  r.storage_type=2;
  subboards[n]=r;
  memset((void *) &(xsubs[n]),0, sizeof(xtrasubsrec));
  ++num_subs;
  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len);
  if (qsc) {
    qsc_n=qsc+1;
    qsc_q=qsc_n+(max_dirs+31)/32;
    qsc_p=qsc_q+(max_subs+31)/32;

    m1=1L<<(n%32);
    m2=0xffffffff<<((n%32)+1);
    m3=0xffffffff>>(32-(n%32));

    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc, 1);

      if ((*qsc!=999) && (*qsc>=n))
        (*qsc)++;

      for (i1=num_subs-1; i1>n; i1--)
        qsc_p[i1]=qsc_p[i1-1];
      qsc_p[n]=0;

      for (i2=num_subs/32; i2>n/32; i2--) {
        qsc_q[i2]=(qsc_q[i2]<<1) | (qsc_q[i2-1]>>31);
      }
      qsc_q[i2]=m1 | (m2 & (qsc_q[i2]<<1)) | (m3 & qsc_q[i2]);

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  }

  save_subs();

  if (curlsub>=n)
    curlsub++;
}


void delete_sub(int n)
{
  int i,i1,i2,nu;
  unsigned long *qsc, *qsc_n, *qsc_q, *qsc_p, m2, m3;
  SUBCONF_TYPE nconv;

  nconv=(SUBCONF_TYPE) n;

  if ((n<0) || (n>=num_subs))
    return;

  update_conf(CONF_SUBS, &nconv, NULL, CONF_UPDATE_DELETE);

  n=(int) nconv;

  while (xsubs[n].num_nets)
    sub_xtr_del(n, 0, 1);
  if ((xsubs[n].nets) && (xsubs[n].flags & XTRA_MALLOCED))
    bbsfree(xsubs[n].nets);

  for (i=n; i<num_subs; i++) {
    subboards[i]=subboards[i+1];
    sub_dates[i]=sub_dates[i+1];
    xsubs[i]=xsubs[i+1];
  }
  --num_subs;
  nu=number_userrecs();

  qsc=(unsigned long *)malloca(syscfg.qscn_len+4);
  if (qsc) {
    qsc_n=qsc+1;
    qsc_q=qsc_n+(max_dirs+31)/32;
    qsc_p=qsc_q+(max_subs+31)/32;

    m2=0xffffffff<<(n%32);
    m3=0xffffffff>>(32-(n%32));

    for (i=1; i<=nu; i++) {
      read_qscn(i,qsc, 1);

      if (*qsc!=999) {
        if (*qsc==n)
          *qsc=999;
        else if (*qsc>n)
          (*qsc)--;
      }

      for (i1=n; i1<num_subs; i1++)
        qsc_p[i1]=qsc_p[i1+1];

      qsc_q[n/32]=(qsc_q[n/32]&m3) | ((qsc_q[n/32]>>1)&m2) |
                  (qsc_q[(n/32)+1]<<31);

      for (i2=(n/32)+1; i2<=(num_subs/32); i2++) {
        qsc_q[i2]=(qsc_q[i2]>>1) | (qsc_q[i2+1]<<31);
      }

      write_qscn(i,qsc,1);
    }
    close_qscn();
    bbsfree(qsc);
  }

  save_subs();

  if (curlsub==n)
    curlsub=-1;
  else if (curlsub>n)
    curlsub--;
}


void boardedit(void)
{
  int i,i1,i2,done, confchg=0;
  char s[81],s1[81],ch;
  SUBCONF_TYPE iconv;

  if (!checkpw())
    return;
  showsubs();
  done=0;
  read_status();
  do {
    nl();
    prt(2,get_string(211));
    ch=onek("QSDIM?");
    switch(ch) {
      case '?':
        showsubs();
        break;
      case 'Q':
        done=1;
        break;
      case 'M':
        nl();
        prt(2,get_string(212));
        input(s,4);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_subs))
          modify_sub(i);
        break;
      case 'S':
        if (num_subs<max_subs) {
          nl();
          prt(2,get_string(1275));
          input(s,4);
          i1=atoi(s);
          if ((!s[0]) || (i1<0) || (i1>=num_subs))
            break;
          nl();
          prt(2,get_string(1276));
          input(s,4);
          i2=atoi(s);
          if ((!s[0]) || (i2<0) || (i2%32==0) || (i2>num_subs) ||
             (i1==i2) || (i1+1==i2))
            break;
          nl();
          if (i2<i1)
            i1++;
          write_qscn(usernum,qsc,1);
          prt(1,get_string(1277));
          insert_sub(i2);
          swap_subs(i1,i2);
          delete_sub(i1);
          confchg=1;
          showsubs();
        } else {
          nl();
          pl(get_string(1278));
        }
        break;
      case 'I':
        if (num_subs<max_subs) {
          nl();
          prt(2,get_string(213));
          input(s,4);
          i=atoi(s);
          if ((s[0]!=0) && (i>=0) && (i<=num_subs)) {
            insert_sub(i);
            modify_sub(i);
            confchg=1;
            if (subconfnum>1) {
              nl();
             list_confs(CONF_SUBS,0);
              i2=select_conf(get_string(1160),CONF_SUBS, 0);
              if (i2>=0)
                if (in_conference(i,&subconfs[i2])<0) {
                  iconv=(SUBCONF_TYPE) i;
                  addsubconf(CONF_SUBS, &subconfs[i2], &iconv);
                  i=(int) iconv;
                }
            } else {
              if (in_conference(i,&subconfs[0])<0) {
                iconv=(SUBCONF_TYPE) i;
                addsubconf(CONF_SUBS, &subconfs[0], &iconv);
                i=(int) iconv;
              }
            }
          }
        }
        break;
      case 'D':
        nl();
        prt(2,get_string(214));
        input(s,4);
        i=atoi(s);
        if ((s[0]!=0) && (i>=0) && (i<num_subs)) {
          nl();
          sprintf(s1,"%s %s? ",get_string(215), subboards[i].name);
          prt(5,s1);
          if (yn()) {
            strcpy(s,subboards[i].filename);
            delete_sub(i);
            confchg=1;
            nl();
            prt(5,get_string(216));
            if (yn()) {
              sprintf(s1,"%s%s.SUB",syscfg.datadir, s);
              unlink(s1);
              sprintf(s1,"%s%s.DAT",syscfg.msgsdir,s);
              unlink(s1);
            }
          }
        }
        break;
    }
  } while ((!done) && (!hangup));
  save_subs();
  if (!wfc)
    changedsl();
  subchg=1;
  gatfn[0]=0;
  if (confchg)
    save_confs(CONF_SUBS, -1, NULL);
}

