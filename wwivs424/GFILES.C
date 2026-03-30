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

char *get_file(char *fn, long *len)
{
  int i;
  char *s;

  i=sh_open1(fn,O_RDONLY | O_BINARY);
  if (i<0) {
    *len=0L;
    return(NULL);
  }
  if ((s=malloca(filelength(i)+50))==NULL) {
    *len=0L;
    sh_close(i);
    return(NULL);
  }
  *len=(long) sh_read(i,(void *)s, filelength(i));
  sh_close(i);
  return(s);
}


gfilerec *read_sec(int sn, int *nf)
{
  gfilerec *g;
  int f,i;
  char s[81];

  i=sizeof(gfilerec)*(gfilesec[sn].maxfiles);
  if ((g=malloca((long) i))==NULL) {
    *nf=0;
    return(NULL);
  }
  sprintf(s,"%s%s.GFL",syscfg.datadir,gfilesec[sn].filename);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    *nf=0;
  else {
    *nf=sh_read(f,(void *)g,i)/sizeof(gfilerec);
    sh_close(f);
  }
  return(g);
}


void list_sec(int *map, int nmap)
{
  int i,abort;
  char s[81];

  abort=0;
  pla("",&abort);
  pla("",&abort);
  pla(get_string(48),&abort);
  pla("",&abort);
  for (i=0; (i<nmap) && (!abort) && (!hangup); i++) {
    sprintf(s,"%2d. %s",i+1, gfilesec[map[i]].name);
    pla(s,&abort);
  }
  nl();
}


void list_gfiles(gfilerec *g, int nf)
{
  int i,abort;
  char s[81];

  abort=0;
  pla("",&abort);
  pla("",&abort);
  pla(get_string(49),&abort);
  pla("",&abort);
  if ((nf==0) && (abort==0)) {
    nl();
    pl(get_string(5));
    nl();
  }
  for (i=0; (i<nf) && (!abort) && (!hangup); i++) {
    sprintf(s,"%2d. %s",i+1, g[i].description);
    pla(s,&abort);
  }
  nl();
  if ((!abort) && (cs())) {
    pl(get_string(50));
    pl(get_string(51));
    nl();
  }
}


void gfile_sec(int sn)
{
  gfilerec *g;
  int i,done,nf,i1,start=0;
  char xdc[81],*ss,*ss1,s[81];

  g=read_sec(sn,&nf);
  if (g==NULL)
    return;
  strcpy(xdc,odc);
  for (i=0; i<20; i++)
    odc[i]=0;
  for (i=1; i<=nf/10; i++)
    odc[i-1]=i+'0';
  if ((menu_on() == 0) || rip_subset)
    list_gfiles(g,nf);
  done=0;
  while ((!done) && (!hangup)) {
    if (menu_on() && !rip_subset)
      rip_list_gfiles(g, nf, start);
    nl();
    tleft(1);
    prt(2,get_string(52));
    ss=mmkey(2);
    i=atoi(ss);
    if (strcmp(ss,"Q")==0)
      done=1;
    else
      if ((strcmp(ss,"A")==0) && (cs())) {
        bbsfree(g);
	fill_sec(sn);
        g=read_sec(sn,&nf);
        if (g==NULL)
          return;
        for (i=0; i<20; i++)
          odc[i]=0;
        for (i=1; i<=nf/10; i++)
          odc[i-1]=i+'0';
      } else if ((strcmp(ss,"D")==0) && (cs())) {
        nl();
        prt(2,get_string(53));
        ss1=mmkey(2);
        i=atoi(ss1);
        if ((i>0) && (i<=nf)) {
          sprintf(s,"%s %s? ",get_string(54), g[i-1].description);
          prt(5,s);
          if (yn()) {
            prt(5,get_string(55));
            if (yn()) {
              sprintf(s,"%s%s\\%s",syscfg.gfilesdir,
                    gfilesec[sn].filename,g[i-1].filename);
              unlink(s);
            }
            for (i1=i; i1<nf; i1++)
              g[i1-1]=g[i1];
            --nf;
            sprintf(s,"%s%s.GFL",syscfg.datadir,gfilesec[sn].filename);
            i=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
            sh_write(i,(void *)g,nf*sizeof(gfilerec));
            sh_close(i);
            nl();
            pl(get_string(56));
            nl();
          }

        } else if (strcmp(ss1,"P")==0) {
          if (start > 0)
            start -= 14;
          if (start < 0)
            start = 0;
        } else if (strcmp(ss1,"N")==0) {
          if (start+14 < nf)
            start += 14;
        }
      } else {
          if (strcmp(ss,"?")==0) {
	    list_gfiles(g,nf);
          } else {
            if (menu_on()) {
              rip_cls();
              printmenu(321);
            }
            if ((i>0) && (i<=nf)) {
              sprintf(s,"%s\\%s",gfilesec[sn].filename,g[i-1].filename);
	      i1=printfile(s);
              thisuser.gfilesread++;
	      if (i1==0) {
                sprintf(s,get_stringx(1,15),g[i-1].filename);
	        sysoplog(s);
	      }
            }
          }
      }
  }
  if (menu_on())
    printmenu(321);
  bbsfree(g);
  strcpy(odc,xdc);
}


void gfiles(void)
{
  int i,i1,done,*map,nmap,ok,start=0;
  char *ss;

  map=(int *)malloca(sysinfo.max_gfilesec * sizeof(int));
  if (!map)
    return;

  done=0;
  nmap=0;
  for (i=0; i<20; i++)
    odc[i]=0;
  for (i=0; i<num_sec; i++) {
    ok=1;
    if (thisuser.age<gfilesec[i].age)
      ok=0;
    if (actsl<gfilesec[i].sl)
      ok=0;
    if (((thisuser.ar & gfilesec[i].ar)==0) && (gfilesec[i].ar))
      ok=0;
    if (ok) {
      map[nmap++]=i;
      if ((nmap % 10)==0)
        odc[nmap/10-1]='0'+(nmap/10);
    }
  }
  if (nmap==0) {
    nl();
    pl(get_string(57));
    nl();
    bbsfree(map);
    return;
  }
  if ((menu_on() == 0) || rip_subset)
    list_sec(map,nmap);
  while ((!done) && (!hangup)) {
    if (menu_on() && !rip_subset)
      rip_list_sec(map,nmap,start);
    nl();
    tleft(1);
    prt(2,get_string(58));
    ss=mmkey(2);
    if (strcmp(ss,"Q")==0)
      done=1;
    else
      if (strcmp(ss,"?")==0)
        list_sec(map,nmap);
      else
    if ((strcmp(ss,"A")==0) && (cs())) {
      i1=0;
      for (i=0; (i<nmap) && (!i1); i++) {
        nl();
        outstr(get_string(59));
        pl(gfilesec[map[i]].name);
  	    nl();
        i1=fill_sec(map[i]);
      }
    } else if (strcmp(ss,"P")==0) {
      if (start > 0)
            start -= 14;
      if (start < 0)
            start = 0;
    } else if (strcmp(ss,"N")==0) {
      if (start+14 < nmap)
            start += 14;
    } else {
      i=atoi(ss);
      if ((i>0) && (i<=nmap)) {
        gfile_sec(map[i-1]);
      }
    }
  }

  if (menu_on()) {
    rip_cls();
    printmenu(321);
  }
  bbsfree(map);
}


/****************************************************************************/
/****************************************************************************/


void hop_sub(void)
{
  unsigned char s1[81], s2[81], ch;
  int c,i,i2,nc,abort=0,oc,os;

  nc=0;
  while (uconfsub[nc].confnum!=-1)
    nc++;

  nl();
  prt(2,get_string(1161));
  input(s1,40);
  if (!s1[0])
    return;
  nl();

  c=0; oc=curconfsub; os=usub[cursub].subnum;

  while ((c<nc) && (!abort)) {
    if (okconf(&thisuser))
      setuconf(CONF_SUBS,c,-1);
    i=0;
    while ((i<num_subs) && (usub[i].subnum!=-1) && (!abort)) {
      strcpy(s2,subboards[usub[i].subnum].name);
      for (i2=0;(s2[i2]=upcase(s2[i2]))!=0;i2++)
        ;
      if (strstr(s2,s1)!=NULL) {
        ansic(5);
	      npr("%s\"%s\"",get_string(1162),subboards[usub[i].subnum].name);
        npr(get_string(1378));
        ch=ynq();
        if (ch=='Y') {
          abort=1;
          cursub=i;
          break;
        } else if (ch=='Q') {
          abort=1;
          if (okconf(&thisuser))
            setuconf(CONF_SUBS, oc, os);
          break;
        }
      }
      ++i;
    }
    c++;
    if (!okconf(&thisuser))
      break;
  }
  if ((okconf(&thisuser)) && (!abort))
    setuconf(CONF_SUBS, oc, os);
}

void hop_dir(void)
{
  unsigned char s1[81],s2[81],ch;
  int c,i,i2,nc,abort=0,oc,os;

  nc=0;
  while (uconfdir[nc].confnum!=-1)
    nc++;

  nl();
  prt(2,get_string(1163));
  input(s1,40);
  if (!s1[0])
    return;
  nl();

  c=0; oc=curconfdir; os=udir[curdir].subnum;

  while ((c<nc) && (!abort)) {
    if (okconf(&thisuser))
      setuconf(CONF_DIRS,c,-1);
    i=0;
    while ((i<num_dirs) && (udir[i].subnum!=-1) && (!abort)) {
      strcpy(s2,directories[udir[i].subnum].name);
      for (i2=0;(s2[i2]=upcase(s2[i2]))!=0;i2++)
        ;
      if (strstr(s2,s1)!=NULL) {
        ansic(5);
	      npr("%s\"%s\"",get_string(1162),directories[udir[i].subnum].name);
        npr(get_string(1378));
        ch=ynq();
        if (ch=='Y') {
          abort=1;
          curdir=i;
          break;
        } else if (ch=='Q') {
          abort=1;
          if (okconf(&thisuser))
            setuconf(CONF_DIRS, oc, os);
          break;
        }
      }
      ++i;
    }
    c++;
    if (!okconf(&thisuser))
      break;
  }
  if ((okconf(&thisuser)) && (!abort))
    setuconf(CONF_DIRS, oc, os);
}

/****************************************************************************/

void valscan(void)
{  /* kcc */
  int os, ac, sn, i, i1, done, val, next;
  userrec tu;
  postrec *p1, p2;
  char s[81], ch;
  unsigned long sq;

  /* Must be local cosysop or better */
  if (!lcs())
    return;

  ac=done=0; os=cursub;

  if ((uconfsub[1].confnum!=-1) && (okconf(&thisuser))) {
    ac=1;
    tmp_disable_conf(1);
  }

  done=0;
  for (sn=0; (sn<num_subs) && (!hangup) && (!done); sn++) {
    if (!iscan(sn))
      continue;
    if (curlsub<0)
      return;

    sq=qsc_p[sn];

    /* Must be sub with validation "on" */
    if ((!(xsubs[curlsub].num_nets)) || (!(subboards[curlsub].anony & anony_val_net)))
      continue;

    nl();
    ansic(2);
    npr("{{ %s %s }}",get_string(1623), subboards[curlsub].name);
    nl();

    for (i=0; (i<=nummsgs) && (!hangup) && (!done); i++) {
      if (get_post(i)->status & status_pending_net) {
        checkhangup();
        tleft(1);
        if ((i>0) && (i<=nummsgs)) {
          read_message(i,&next,&val);
          npr("4[4%s: %s1]",get_string(1624), subboards[curlsub].name);
          nl();
          outstr(get_string(1499));
          ch=onek("QDVMR");
          switch (ch) {
            case 'Q':
              done=1;
              break;
            case 'R':
              i--;
              continue;
            case 'V':
              open_sub(1);
              resynch(curlsub, &i, NULL);
              p1=get_post(i);
              p1->status &= ~status_pending_net;
              write_post(i, p1);
              close_sub();
              send_net_post(p1, subboards[curlsub].filename, curlsub);
              nl();
              pl(get_string(1500));
              nl();
              break;
            case 'M':
              if ((lcs()) && (i>0) && (i<=nummsgs) &&
                  (subboards[curlsub].anony & anony_val_net) &&
                  (xsubs[curlsub].num_nets)) {
                open_sub(1);
                resynch(curlsub, &i, NULL);
                p1=get_post(i);
                p1->status &= ~status_pending_net;
                write_post(i, p1);
                close_sub();
                nl();
                pl(get_string(684));
                nl();
              }
              break;
            case 'D':
              if (lcs()) {
                if (i) {
                  open_sub(1);
                  resynch(curlsub, &i, NULL);
                  p2=*get_post(i);
                  delete(i);
                  close_sub();
                  if (p2.ownersys==0) {
                    read_user(p2.owneruser,&tu);
                    if ((tu.inact & inact_deleted)==0) {
                      if (date_to_daten(tu.firston) < p2.daten) {
                        nl();
                        prt(2,get_string(980));
                        mpl(3);
                        input(s,3);
                        if (s[0])
                          i1=(atoi(s));
                        else
                          i1=1;
                        if (i1>tu.msgpost)
                          i1=tu.msgpost;
                        if (i1)
                          tu.msgpost-=i1;
                        nl();
                        ansic(3);
                        outstr(get_string(993));
                        pln(i1);
                        tu.deletedposts++;
                        write_user(p2.owneruser,&tu);
                        topscreen();
                      }
                    }
                  }
                  resynch(curlsub, &i, &p2);
                }
              }
              break;
          }
        }
      }
    }
    qsc_p[sn]=sq;
  }

  if (ac)
    tmp_disable_conf(0);

  cursub=os;
}

