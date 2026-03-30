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



static void maybe_netmail(xtrasubsnetrec *ni, int add)
{
  prt(5,get_string(217));
  if (yn()) {
    strcpy(irt,get_string(218));
    strcat(irt,ni->stype);
    if (add)
      strcat(irt,get_string(219));
    else
      strcat(irt,get_string(220));
    set_net_num(ni->net_num);
    email(1, ni->host, 0, 0);
  }
}

/****************************************************************************/


void sub_req(int main_type, int minor_type, int tosys, char *extra)
{
  net_header_rec nh;

  nh.tosys=tosys;
  nh.touser=1;
  nh.fromsys=net_sysnum;
  nh.fromuser=1;
  nh.main_type=main_type;
  nh.minor_type=minor_type;
  nh.list_len=0;
  time((long *)&nh.daten);
  if (!minor_type)
    nh.length=strlen(extra)+1;
  else {
    nh.length=1;
    extra="";
  }
  nh.method=0;

  send_net(&nh, NULL, extra, NULL);

  nl();
  if (main_type == main_type_sub_add_req)
    outstr(get_string(221));
  else
    outstr(get_string(222));
  pln(tosys);
  pausescr();
}

/****************************************************************************/

#define OPTION_AUTO   0x0001
#define OPTION_NO_TAG 0x0002
#define OPTION_GATED  0x0004
#define OPTION_NETVAL 0x0008
#define OPTION_ANSI   0x0010


int find_hostfor(char *type, unsigned short *ui, char *desc, unsigned short *opt)
{
  int i,i1,done=0;
  char s[161],*ss;
  FILE *f;
  unsigned short h, o,rc=0;

  if (desc)
    *desc=0;
  *opt=0;

  for (i=0; (i<256) && (!done); i++) {
    if (i)
      sprintf(s,"%sSUBS.%d",net_data,i);
    else
      sprintf(s,"%sSUBS.LST",net_data);
    f=fsh_open(s,"r");
    if (f) {
      while ((!done) && fgets(s,160,f)) {
        if (s[0]>' ') {
          ss=strtok(s," \r\n\t");
          if (ss) {
            if (stricmp(ss,type)==0) {
              ss=strtok(NULL," \r\n\t");
              if (ss) {
                h=(unsigned short) atol(ss);
                o=0;
                ss=strtok(NULL,"\r\n");
                if (ss) {
                  i1=0;
                  while (*ss && ((*ss==' ') || (*ss=='\t'))) {
                    ++ss;
                    ++i1;
                  }
                  if (i1<4) {
                    while (*ss && (*ss!=' ') && (*ss!='\t')) {
                      switch(*ss) {
                        case 'T': o |= OPTION_NO_TAG; break;
                        case 'R': o |= OPTION_AUTO; break;
                        case 'G': o |= OPTION_GATED; break;
                        case 'N': o |= OPTION_NETVAL; break;
                        case 'A': o |= OPTION_ANSI; break;
                      }
                      ++ss;
                    }
                    while (*ss && ((*ss==' ') || (*ss=='\t')))
                      ++ss;
                  }
                  if (*ui) {
                    if (*ui==h) {
                      done=1;
                      *opt=o;
                      rc=h;
                      if (desc)
                        strcpy(desc,ss);
                    }
                  } else {
                    nl();
                    outstr(get_string(223)); pl(type);
                    outstr(get_string(224)); npr("%u\r\n",h);
                    outstr(get_string(225)); npr("%s\r\n",ss);
                    nl();
                    prt(5,get_string(226));
                    if (yn()) {
                      done=1;
                      *ui=h;
                      *opt=o;
                      rc=h;
                      if (desc)
                        strcpy(desc,ss);
                    }
                  }
                }
              }
            }
          }
        }
      }
      fsh_close(f);
    } else {
      done=1;
    }
  }

  return(rc);
}

/****************************************************************************/

void sub_xtr_del(int n, int nn, int f)
{
  xtrasubsnetrec xn;
  int i,ok;
  unsigned short opt;

  xn=xsubs[n].nets[nn];

  if (f) {
    xsubs[n].num_nets--;

    for (i=nn; i<xsubs[n].num_nets; i++)
      xsubs[n].nets[i]=xsubs[n].nets[i+1];
  }

  set_net_num(xn.net_num);

  if ((xn.host) && (valid_system(xn.host))) {
    ok=find_hostfor(xn.stype, (unsigned short *)&xn.host, NULL, &opt);
    if (ok) {
      if (opt&OPTION_AUTO) {
        if (status.net_version>=29) {
          prt(5,get_string(227));
          if (yn())
            sub_req(main_type_sub_drop_req, xn.type, xn.host, xn.stype);
        } else
          maybe_netmail(&xn, 0);
      } else {
        maybe_netmail(&xn, 0);
      }
    } else {
      prt(5,get_string(227));
      if (yn())
        sub_req(main_type_sub_drop_req, xn.type, xn.host, xn.stype);
      else
        maybe_netmail(&xn, 0);
    }
  }
}

/****************************************************************************/

void sub_xtr_add(int n, int nn)
{
  unsigned short opt, ok, i,i1,f;
  char desc[100],s[100],onx[20],*mmk,ch;
  int onxi, odci,ii,gc;
  xtrasubsnetrec *xnp;
  FILE *ff;

  if ((nn<0) || (nn>=xsubs[n].num_nets)) {

    if (xsubs[n].num_nets_max<=xsubs[n].num_nets) {
      i1=xsubs[n].num_nets+4;
      xnp=(xtrasubsnetrec *)malloca(i1*sizeof(xtrasubsnetrec));
      if (!xnp)
        return;
      for (i=0; i<xsubs[n].num_nets; i++)
        xnp[i]=xsubs[n].nets[i];

      if (xsubs[n].flags&XTRA_MALLOCED)
        bbsfree(xsubs[n].nets);

      xsubs[n].nets=xnp;
      xsubs[n].num_nets_max=i1;
      xsubs[n].flags|=XTRA_MALLOCED;
    }
    xnp=&(xsubs[n].nets[xsubs[n].num_nets]);
  } else
    xnp=&(xsubs[n].nets[nn]);

  memset(xnp, 0, sizeof(xtrasubsnetrec));

  if (net_num_max>1) {
    odc[0]=0;
    odci=0;
    onx[0]='Q';
    onx[1]=0;
    onxi=1;
    nl();
    for (ii=0; ii<net_num_max; ii++) {
      if (ii<9) {
        onx[onxi++]=ii+'1';
        onx[onxi]=0;
      } else {
        odci=(ii+1)/10;
        odc[odci-1]=odci+'0';
        odc[odci]=0;
      }
      npr("%d. %s\r\n",ii+1,net_networks[ii].name);
    }
    pl(get_string(12));
    nl();
    prt(2,get_string(13));
    if (net_num_max<9) {
      ch=onek(onx);
      if (ch=='Q')
        ii=-1;
      else
        ii=ch-'1';
    } else {
      mmk=mmkey(2);
      if (*mmk=='Q')
        ii=-1;
      else
        ii=atoi(mmk)-1;
    }
    if ((ii>=0) && (ii<net_num_max)) {
      set_net_num(ii);
    } else
      return;
  }
  xnp->net_num=net_num;

  nl();
  prt(2,get_string(228));
  input(xnp->stype,7);
  if (xnp->stype[0]==0)
    return;

  xnp->type=atoi(xnp->stype);

  if ((!xnp->type) && (status.net_version<32)) {
    nl();
    pl(get_string(229));
    nl();
    pausescr();
    return;
  }

  if (xnp->type)
    sprintf(xnp->stype,"%u",xnp->type);

  prt(5,get_string(230));
  if (yn()) {

    sprintf(s,"%sN%s.NET",net_data,xnp->stype);
    f=sh_open(s,O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    if (f>0)
      sh_close(f);

    prt(5,get_string(231));
    if (ny())
      xnp->flags |= XTRA_NET_AUTO_ADDDROP;

    prt(5,get_string(232));
    if (ny()) {
      xnp->flags |= XTRA_NET_AUTO_INFO;
      if (display_sub_categories()) {
        gc=0;
        while (!gc) {
          nl();
          prt(2,get_string(931));
          input(s,3);
          i=atoi(s);
          if (i || (strcmp(s,"0")==0)) {
            sprintf(s,"%sCATEG.NET",net_data);
            ff=fsh_open(s,"r");
            while (fgets(s,100,ff)) {
              i1=atoi(s);
              if (i1==i) {
                fsh_close(ff);
                gc=1;
                xnp->category=i;
                break;
              }
            }
            fsh_close(ff);
            if (strcmp(s,"0")==0)
              gc=1;
            else if (!xnp->category)
              pl(get_string(932));
          } else {
            if ((strlen(s)==1) && (s[0]=='?')) {
              display_sub_categories();
              continue;
            }
          }
        }
      }
    }

  } else {
    ok=find_hostfor(xnp->stype, (unsigned short *) &(xnp->host), desc, &opt);

    if (!ok) {
      nl();
      prt(2,get_string(233));
      input(desc,6);
      xnp->host=(unsigned short) atol(desc);
      desc[0]=0;
    }

    if (!xsubs[n].desc[0])
      strcpy(xsubs[n].desc, desc);

    if (xnp->host==net_sysnum)
      xnp->host=0;

    if (xnp->host) {
      if (valid_system(xnp->host)) {
        if (ok) {
          if (opt & OPTION_NO_TAG) {
            subboards[n].anony |= anony_no_tag;
          }
          nl();
          if ((opt & OPTION_AUTO) && (status.net_version>=29)) {
            prt(5,get_string(234));
            if (yn())
              sub_req(main_type_sub_add_req, xnp->type, xnp->host, xnp->stype);
          } else {
            maybe_netmail(xnp, 1);
          }
        } else {
          if (status.net_version>=29) {
            nl();
            prt(5,get_string(234));
            ok=yn();
          } else {
            ok=0;
          }
          if (ok) {
            sub_req(main_type_sub_add_req, xnp->type, xnp->host, xnp->stype);
          } else {
            maybe_netmail(xnp, 1);
          }
        }
      } else {
        nl();
        pl(get_string(235));
        pausescr();
      }
    }
  }
  if ((nn<0) || (nn>=xsubs[n].num_nets)) {
    xsubs[n].num_nets++;
  }
}

/****************************************************************************/

int display_sub_categories(void)
{
  FILE *ff;
  char s[81], *ss;
  int i;

  if (!net_sysnum)
    return(0);

  sprintf(s,"%sCATEG.NET",net_data);
  ff=fsh_open(s,"r");
  if (ff) {
    nl();
    pl(get_string(930));
    i=0;
    while (!i && fgets(s,100,ff)) {
      ss=strchr(s,'\n');
      if (ss)
        *ss=0;
      pla(s,&i);
    }
    fsh_close(ff);
    return(1);
  }
  return(0);
}

/****************************************************************************/


