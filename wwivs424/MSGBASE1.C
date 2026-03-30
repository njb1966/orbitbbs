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
#include "ripint.h"

#include <dir.h>




#define ALLOW_FULLSCREEN 1
#define EMAIL_STORAGE 2

/****************************************************************************/

void send_net_post(postrec *p, char *extra, int subnum)
{
  net_header_rec nh, nh1;
  char *b, *b1;
  long len1, len2;
  char s[81];
  int f,i,onn,nn,n,nn1;
  unsigned int *list;
  xtrasubsnetrec *xnp;

  b=readfile(&(p -> msg),extra,&len1);
  if (b==NULL)
    return;

  onn=net_num;
  if (p->status & status_post_new_net)
    nn=p->title[80];
  else if (xsubs[subnum].num_nets)
    nn=xsubs[subnum].nets[0].net_num;
  else
    nn=net_num;

  nn1=nn;
  if (p->ownersys==0)
    nn=-1;

  nh1.tosys=0;
  nh1.touser=0;
  nh1.fromsys=p->ownersys;
  nh1.fromuser=p->owneruser;
  nh1.list_len=0;
  nh1.daten=p->daten;
  nh1.length=len1+1+strlen(p -> title);
  nh1.method=0;

  if (nh1.length > 32755) {
    outstr(get_string(645));
    npr("%lu",nh1.length-32755L);
    outstr(get_string(646));
    nh1.length = 32755;
    len1=nh1.length-strlen(p->title)-1;
  }

  if ((b1=malloca(nh1.length+100))==NULL) {
    bbsfree(b);
    set_net_num(onn);
    return;
  }

  strcpy(b1,p -> title);
  memmove(&(b1[strlen(p -> title)+1]),b,(unsigned int) len1);
  bbsfree(b);

  for (n=0; n<xsubs[subnum].num_nets; n++) {
    xnp=&(xsubs[subnum].nets[n]);

    if ((xnp->net_num==nn) && (xnp->host))
      continue;

    set_net_num(xnp->net_num);

    nh=nh1;
    list=NULL;
    nh.minor_type=xnp->type;
    if (!nh.fromsys)
      nh.fromsys=net_sysnum;

    if (xnp->host) {
      nh.main_type=main_type_pre_post;
      nh.tosys=xnp->host;
    } else {
      nh.main_type=main_type_post;
      sprintf(s,"%sN%s.NET",net_data, xnp->stype);
      f=sh_open1(s,O_RDONLY|O_BINARY);
      if (f>0) {
        len1=filelength(f);
        list=(unsigned int *)malloca(len1*2+1);
        if (!list)
          continue;
        if ((b=malloca(len1+100L))==NULL) {
          bbsfree(list);
          continue;
        }
        sh_read(f,b,len1);
        sh_close(f);
        b[len1]=0;
        len2=0;
        while (len2<len1) {
          while ((len2<len1) && ((b[len2]<'0') || (b[len2]>'9')))
            ++len2;
          if ((b[len2]>='0') && (b[len2]<='9') && (len2<len1)) {
            i=atoi(&(b[len2]));
            if (((net_num!=nn) || (nh.fromsys!=i)) && (i!=net_sysnum))
              list[(nh.list_len)++]=i;
            while ((len2<len1) && (b[len2]>='0') && (b[len2]<='9'))
              ++len2;
          }
        }
        bbsfree(b);
      }
      if (!nh.list_len) {
        if (list)
          bbsfree(list);
        continue;
      }
    }
    if (!xnp->type)
      nh.main_type=main_type_new_post;
    if (nn1==net_num)
      send_net(&nh, list, b1, xnp->type?NULL:xnp->stype);
    else
      gate_msg(&nh, b1, xnp->net_num, xnp->stype, list, nn);
    if (list)
      bbsfree(list);
  }

  bbsfree(b1);
  set_net_num(onn);
}

/****************************************************************************/


void post(void)
{
  messagerec m;
  postrec p;
  char s[121];
  int i,dm,a;
  slrec ss;
  time_t time1, time2;

  if (!iscan(cursub)) {
    nl();
    pl(get_string(1195));
    return;
  }
  if (curlsub<0) {
    nl();
    pl(get_string(668));
    nl();
    return;
  }

  ss=syscfg.sl[actsl];

  if (freek1(syscfg.msgsdir)<10.0) {
    nl();
    pl(get_string(332));
    nl();
    return;
  }

  if ((restrict_post & thisuser.restrict) || (thisuser.posttoday>=ss.posts)) {
    nl();
    pl(get_string(669));
    nl();
    return;
  }

  if (actsl<subboards[curlsub].postsl) {
    nl();
    pl(get_string(670));
    nl();
    return;
  }

  m.storage_type=subboards[curlsub].storage_type;
  a=subboards[curlsub].anony & 0x0f;
  if ((a==0) && (ss.ability & ability_post_anony))
    a=anony_enable_anony;
  if ((a==anony_enable_anony) && (thisuser.restrict & restrict_anony))
    a=0;
  if (xsubs[curlsub].num_nets) {
    a &= (anony_real_name);
    if (thisuser.restrict & restrict_net) {
      nl();
      pl(get_string(671));
      nl();
      return;
    }
    if (net_sysnum) {
      nl();
      outstr(get_string(672));
      for (i=0; i<xsubs[curlsub].num_nets; i++) {
        if (i)
          outstr(", ");
        outstr(net_networks[xsubs[curlsub].nets[i].net_num].name);
      }
      outstr(".\r\n");
      nl();
    }
  }

  time1=time(NULL);

  write_inst(INST_LOC_POST,curlsub,INST_FLAGS_NONE);

  inmsg(&m,p.title,&a,1,(subboards[curlsub].filename),ALLOW_FULLSCREEN,
    subboards[curlsub].name, (subboards[curlsub].anony&anony_no_tag)?1:0);
  if (m.stored_as!=0xffffffff) {

    p.anony=a;
    p.msg=m;
    p.ownersys=0;
    p.owneruser=usernum;
    lock_status();
    p.qscan=status.qscanptr++;
    save_status();
    time((long *)(&p.daten));
    if (thisuser.restrict & restrict_validate)
      p.status=status_unvalidated;
    else
      p.status=0;

    open_sub(1);

    if ((xsubs[curlsub].num_nets) &&
      (subboards[curlsub].anony & anony_val_net) && (!lcs() || irt[0])) {
      p.status |= status_pending_net;
      dm=1;
      for (i=nummsgs; (i>=1) && (i>(nummsgs-28)); i--) {
        if (get_post(i)->status & status_pending_net) {
          dm=0;
          break;
        }
      }
      if (dm) {
        sprintf(s,get_stringx(1,37),subboards[curlsub].name);
        ssm(1,0,s);
      }
    }


    if (nummsgs>=subboards[curlsub].maxmsgs) {
      i=1;
      dm=0;
      while ((dm==0) && (i<=nummsgs)) {
        if ((get_post(i)->status & status_no_delete)==0)
          dm=i;
        ++i;
      }
      if (dm==0)
        dm=1;
      delete(dm);
    }

    add_post(&p);
    ++thisuser.msgpost;
    ++thisuser.posttoday;
    lock_status();
    ++status.msgposttoday;
    ++status.localposts;

    if (sysinfo.flags & OP_FLAGS_POSTTIME_COMPENSATE) {
      time2=time(NULL);
      if (time1>time2)
        time2+=24*3600;
      time1=(float)(time2-time1);
      if ((time1/60.0)>syscfg.sl[actsl].time_per_logon)
        time1=(float)(syscfg.sl[actsl].time_per_logon*60.0);
                  thisuser.extratime+=(float)(time1);
    }

    save_status();
    close_sub();

    topscreen();
    sprintf(s,get_stringx(1,38),p.title,subboards[curlsub].name);
    sysoplog(s);
    outstr(get_string(673));
    pl(subboards[curlsub].name);
    if (xsubs[curlsub].num_nets) {
      ++thisuser.postnet;
      if (!(p.status & status_pending_net))
        send_net_post(&p, subboards[curlsub].filename, curlsub);
    }
  }
}


void grab_user_name(messagerec *m, char *fn)
{
  char *ss,*ss1,*ss2;
  long len;

  ss=readfile(m,fn,&len);
  if (ss) {
    ss1=strchr(ss,'\r');
    if (ss1) {
      *ss1=0;
      ss2=ss;
      if ((ss[0]=='`') && (ss[1]=='`')) {
        for (ss1=ss+2; *ss1; ss1++) {
          if ((ss1[0]=='`') && (ss1[1]=='`'))
            ss2=ss1+2;
        }
        while (*ss2==' ')
          ++ss2;
      }
      strcpy(net_email_name,ss2);
    } else
      net_email_name[0]=0;
    bbsfree(ss);
  } else
    net_email_name[0]=0;
}


void scan(int msgnum, int optype, int *nextsub)
{
  char s[161],s1[81],*b,*ss1;

  char *fstr, ch, s2[81];
  int fnd, sdir=1, lmt;
  static char findstr[21];
  int i,i1,i2,done,quit,abort,next,val,realexpress;
  int title_lines;
  slrec ss;
  long len;
  postrec p1, p2, *p3, *p4;
  userrec tu;

  if (menu_on()) {
    if (lcs())
      printmenu(13);
    else
      printmenu(1);
    cleared = NEEDCLEAR; //-1;
  }
  if (rip_on())
    setmsgview(11);
  irt[0]=0;
  irt_name[0]=0;
  done=0;
  quit=0;
  val=0;
  realexpress=express;
  iscan(cursub);
  if (curlsub<0) {
    nl();
    pl(get_string(668));
    nl();
    return;
  }
  do {
    tleft(1);
    checkhangup();
    if (xsubs[curlsub].num_nets)
      set_net_num(xsubs[curlsub].nets[0].net_num);
    else
      set_net_num(0);
    if (optype!=0)
      resynch(cursub, &msgnum, NULL);
    write_inst(INST_LOC_SUBS, usub[cursub].subnum, INST_FLAGS_NONE);
    switch(optype) {
      case 0: /* Read Prompt */
        if (E_C) {
          sprintf(s,"1%s0:7(11-%u,^%u7)1,? 0: 2",get_string(678),
            nummsgs,msgnum);
        } else {
          sprintf(s,"%s:(1-%u,^%u),? :",get_string(678),
            nummsgs,msgnum);
        }
        nl();
        if (express) {
          s[0]=0;
          nln(2);
        } else {
          if (E_C) {
            sprintf(s1,"7[1%s7] [1%s7]",usub[cursub].keys,
              subboards[usub[cursub].subnum].name);
          } else {
            sprintf(s1,"[%s] [%s]",usub[cursub].keys,
              subboards[usub[cursub].subnum].name);
          }
          ansic(2); pl(s1);
          prt(2,s);
          helpl=16;
          input(s,5);
          resynch(cursub, &msgnum, NULL);
          while (s[0]==32) {
            strcpy(s1,&(s[1]));
            strcpy(s,s1);
          }
        }
        optype=0;
        i=atoi(s);
        if (s[0]==0) {
          i=msgnum+1;
          if (i>=nummsgs+1)
            done=1;
        }
        if ((i!=0) && (i<=nummsgs) && (i>=1)) {
          optype=2;
          msgnum=i;
        } else
          if (s[1]==0) {
            switch(s[0]) {

              /* Find addition */
              case 'F':
                abort=0;
                if (!(g_flags & g_flag_made_find_str)) {
                  fstr=strupr(stripcolors(get_post(msgnum)->title));
                  strncpy(findstr, fstr, sizeof(findstr)-1);
                  g_flags |= g_flag_made_find_str;
                } else
                  fstr=&findstr[0];
                while (strncmp(fstr,"RE:",3)==0 || *fstr==' ') {
                  if (*fstr==' ')
                    fstr++;
                  else
                    fstr+=3;
                }
                if (strlen(fstr)>=20)
                  fstr[20]=0;
                strncpy(findstr, fstr, sizeof(findstr)-1);
                nl();
                outstr(get_string(1619));
                outstr(fstr);
                outstr(get_string(1620));
                input(s1,20);
                if (!*s1)
                  strncpy(s1,fstr,sizeof(s1)-1);
                else
                  strncpy(findstr,s1,sizeof(findstr)-1);
                nl();
                prt(1,get_string(1511));
                s2[0]='Q';
                s2[1]=upcase(*get_string(1512));
                s2[2]=upcase(*get_string(1513));
                strcpy(&s2[3],"+-");
                ch=onek(s2);
                if (ch=='Q')
                  break;
                fnd=0;
                i=msgnum;
                nl();
                ansic(1);
                npr(get_string(1514));
                ansic(2);

                /* Store search direction and limit */
                if (ch=='-' || ch==upcase(*get_string(1512))) {
                  sdir=0;
                  lmt=1;
                } else {
                  sdir=1;
                  lmt=nummsgs;
                }

                while ((i!=lmt) && !abort && !hangup && !fnd) {
                  if (!sdir)
                    i--;
                  else
                    i++;
                  checka(&abort,&next);
                  if (!(i%5)) {
                    npr("%5.5d",i);
                    for (i1=0; i1<5; i1++)
                      outstr("\b");
                    if (!(i%100)) {
                      tleft(1);
                      checkhangup();
                    }
                  }
                  b=strupr(readfile(&(get_post(i)->msg),subboards[curlsub].filename,&len));
                  fnd=(strstr(strupr(stripcolors(get_post(i)->title)),s1) || strstr(b,s1));
                  bbsfree(b);
                }
                if (fnd) {
                  npr(get_string(1515));
                  nl();
                  msgnum=i;
                  optype=2;
                } else {
                  prt(6,get_string(1516));
                  nl();
                  optype=0;
                }
                break;
              /* End of find addition */

              case 'Q':
                quit=1;
                done=1;
                *nextsub=0;
                break;
              case 'B':
                if (*nextsub!=0) {
                  *nextsub=1;
                  done=1;
                  quit=1;
                }
                break;
              case 'T':
                optype=1;
                break;
              case 'R':
                optype=2;
                break;
              case 'A':
                if (rip_on()) {
                  sprintf(s,"\n!|w000%c271610|e|#\r ", formery);
                  comstr(s);
                }
                strcpy(irt_sub, subboards[usub[cursub].subnum].name);
                if ((get_post(msgnum)->ownersys) && (!get_post(msgnum)->owneruser))
                  grab_user_name(&(get_post(msgnum)->msg),subboards[curlsub].filename);
                grab_quotes(&(get_post(msgnum)->msg),subboards[curlsub].filename);
                ss=syscfg.sl[actsl];
                if (get_post(msgnum)->status & status_post_new_net) {
                  set_net_num(get_post(msgnum)->title[80]);
                  if (get_post(msgnum)->title[80]==-1) {
                    pl(get_string(679));
                    break;
                  }
                }
                if ((lcs()) || (ss.ability & ability_read_post_anony) || (get_post(msgnum)->anony==0))
                  email(get_post(msgnum)->owneruser,get_post(msgnum)->ownersys,0,0);
                else
                  email(get_post(msgnum)->owneruser,get_post(msgnum)->ownersys,0,get_post(msgnum)->anony);
                irt_sub[0]=0;
                grab_quotes(NULL, NULL);
                restore_msg_menu();
                break;
              case 'P':
                irt[0]=0;
                irt_name[0]=0;
              case 'W':
                if (rip_on()) {
                  sprintf(s,"\n!|w000%c271610|e|#\r ", formery);
                  comstr(s);
                }
                p2=*get_post(msgnum);
                grab_quotes(&(p2.msg),subboards[curlsub].filename);
                post();
                resynch(cursub, &msgnum, &p2);
                grab_quotes(NULL, NULL);
                restore_msg_menu();
                break;
              case '?':
                if (lcs())
                  printmenu(13);
                else
                  printmenu(1);
                break;
              case '-':
                if ((msgnum>1) && (msgnum-1<nummsgs)) {
                  --msgnum;
                  optype=2;
                }
                break;
              case 'C':
                express=1;
                break;
/*************/
              case 'V':
                if ((cs()) && (get_post(msgnum)->ownersys==0) && (msgnum>0) && (msgnum<=nummsgs))
                  valuser(get_post(msgnum)->owneruser);
                else
                  if ((cs()) && (msgnum>0) && (msgnum<=nummsgs)) {
                    nl();
                    pl(get_string(680));
                    nl();
                  }
                break;
              case 'N':
                if ((lcs()) && (msgnum>0) && (msgnum<=nummsgs)) {
                  open_sub(1);
                  resynch(cursub, &msgnum, NULL);
                  p3=get_post(msgnum);
                  p3->status ^= status_no_delete;
                  write_post(msgnum, p3);
                  close_sub();
                  nl();
                  if (p3->status & status_no_delete)
                    pl(get_string(681));
                  else
                    pl(get_string(682));
                  nl();
                }
                break;
              case 'X':
                if ((lcs()) && (msgnum>0) && (msgnum<=nummsgs) &&
                    (subboards[curlsub].anony & anony_val_net) &&
                    (xsubs[curlsub].num_nets)) {
                  open_sub(1);
                  resynch(cursub, &msgnum, NULL);
                  p3=get_post(msgnum);
                  p3->status ^= status_pending_net;
                  write_post(msgnum, p3);
                  close_sub();
                  nl();
                  if (p3->status & status_pending_net) {
                    val |= 2;
                    pl(get_string(683));
                  } else
                    pl(get_string(684));
                  nl();
                }
                break;
              case 'U':
                if ((lcs()) && (msgnum>0) && (msgnum<=nummsgs)) {
                  open_sub(1);
                  resynch(cursub, &msgnum, NULL);
                  p3=get_post(msgnum);
                  p3->anony=0;
                  write_post(msgnum, p3);
                  close_sub();
                  nl();
                  pl(get_string(685));
                }
                break;
              case 'D':
                if (lcs()) {
                  if (msgnum) {
                    open_sub(1);
                    resynch(cursub, &msgnum, NULL);
                    p2=*get_post(msgnum);
                    delete(msgnum);
                    close_sub();
                    if (p2.ownersys==0) {
                      read_user(p2.owneruser,&tu);
                      if ((tu.inact & inact_deleted)==0) {
                        if (date_to_daten(tu.firston) < p2.daten) {
                          nl();
                          prt(2,get_string(980));
                          mpl(3);
                          input(s1,3);
                          if (s1[0])
                            i1=(atoi(s1));
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
                    resynch(cursub, &msgnum, &p2);
                  }
                }
                break;
              case 'E':
                if (so()) {
                  if ((msgnum>0) && (msgnum<=nummsgs)) {
                    b=readfile(&(get_post(msgnum)->msg),(subboards[curlsub].filename),&len);
                    extract_out(b,len, get_post(msgnum)->title);
                  }
                }
                break;
              case 'M':
                if ((lcs()) && (msgnum>0) && (msgnum<=nummsgs)) {

                  tmp_disable_conf(1);
                  nl();
                  do {
                    prt(2,get_string(686));
                    ss1=mmkey(0);
                    if (ss1[0]=='?') {
                      sublist();
                    }
                  } while ((!hangup) && (ss1[0]=='?'));
                  i=-1;
                  if (ss1[0]==0) {
                    tmp_disable_conf(0);
                    break;
                  }
                  for (i1=0; (i1<num_subs) && (usub[i1].subnum!=-1); i1++)
                    if (strcmp(usub[i1].keys,ss1)==0)
                      i=i1;
                  if (i!=-1) {
                    if (actsl<subboards[usub[i].subnum].postsl) {
                      nl();
                      pl(get_string(1517));
                      nl();
                      i=-1;
                    }
                  }
                  if (i!=-1) {

                    open_sub(1);
                    resynch(cursub, &msgnum, NULL);
                    p2=*get_post(msgnum);
                    p1=p2;

                    b=readfile(&(p2.msg),(subboards[curlsub].filename),&len);
                    delete(msgnum);
                    if (msgnum>1)
                      msgnum--;
                    close_sub();


                    iscan(i);
                    open_sub(1);
                    p2.msg.storage_type=subboards[curlsub].storage_type;
                    savefile(b,len,&(p2.msg),(subboards[curlsub].filename));
                    lock_status();
                    p2.qscan=status.qscanptr++;
                    save_status();
                    if (nummsgs>=subboards[curlsub].maxmsgs) {
                      i1=1;
                      i2=0;
                      while ((i2==0) && (i1<=nummsgs)) {
                        if ((get_post(i1)->status & status_no_delete)==0)
                          i2=i1;
                        ++i1;
                      }
                      if (i2==0)
                        i2=1;
                      delete(i2);
                    }
                    if ((!(subboards[curlsub].anony & anony_val_net)) ||
                        (!xsubs[curlsub].num_nets))
                      p2.status &= ~status_pending_net;
                    add_post(&p2);
                    close_sub();
                    tmp_disable_conf(0);
                    iscan(cursub);
                    nl();
                    pl(get_string(687));
                    nl();
                    resynch(cursub, &msgnum, &p1);
                  } else
                    tmp_disable_conf(0);
                }
                break;
              case 'L':
                if (!so())
                  break;
                nl();
                prt(2,get_string(44));
                input(s,50);
                if (s[0]) {
                  nl();
                  prt(5,get_string(17));
                  if (yn()) {
                    nl();
                    load_workspace(s,0);
                  } else {
                    nl();
                    load_workspace(s,1);
                  }
                }
                break;
/*************/
            }
            restore_msg_menu();
        } else {
          if (strcmp(s,"CLS")==0)
            outchr('\x0c');
        }
        break;
      case 1: /* List Titles */
#ifdef RIPDRIVE
        rd_coff();
#endif
        i=0;
        abort=0;
        if (msgnum>=nummsgs)
          abort=1;
        else
          nl();
        title_lines=screenlinest-6;
        if (title_lines<1)
          title_lines=1;
        while ((!abort) && (!hangup) && (++i<=title_lines)) {
          ++msgnum;
          p3=get_post(msgnum);
          if ((p3->ownersys==0) && (p3->owneruser==usernum))
            sprintf(s1,"7[1%d7]",msgnum);
          else if (p3->ownersys!=0)
            sprintf(s1,"7<1%d7>",msgnum);
          else
            sprintf(s1,"7(1%d7)",msgnum);
          for (i1=0; i1<7; i1++)
            s[i1]=32;
          if (p3->qscan>qsc_p[curlsub])
            s[0]='*';
          if (p3->status & (status_pending_net | status_unvalidated))
            s[0]='+';
          strcpy(&s[9-strlen(stripcolors(s1))],s1);
          strcat(s,"1 ");
          if ((get_post(msgnum)->status&(status_unvalidated|status_delete))&&(!lcs()))
            strcat(s,get_string(665));
          else
            strcat(s,stripcolors(get_post(msgnum)->title));

          if (thisuser.screenchars>=80) {
            if (strlen(stripcolors(s))>50)
              while (strlen(stripcolors(s))>50)
                s[strlen(s)-1]=0;
            strcat(s,charstr(51-strlen(stripcolors(s)),' '));
            if (okansi())
              strcat(s,"7³1");
            else
              strcat(s,"|");
            strcat(s," ");
            if ((p3->anony & 0x0f) &&
              ((syscfg.sl[actsl].ability & ability_read_post_anony)==0)) {
                strcat(s,get_string(482));
            } else {
              b=readfile(&(p3->msg),(subboards[curlsub].filename),
                         &len);
              if (b) {
                strncpy(s1,b,sizeof(s1)-1);
                s1[sizeof(s1)-1]=0;
                strcpy(b, stripcolors(s1));
                i1=0; strcpy(s1,"");
                while ((b[i1]!=13) && (b[i1]) && ((long)i1<len) &&
                  (i1<(thisuser.screenchars-54)))
                  s1[i1]=b[i1++];
                s1[i1]=0;
                strcat(s,s1);
                bbsfree(b);
              }
            }
          }

          ansic(2);
          pla(s,&abort);
          if (msgnum>=nummsgs)
            abort=1;
        }
#ifdef RIPDRIVE
        rd_con();
#endif
        optype=0;
        break;
      case 2: /* Read Message */
#ifdef RIPDRIVE
        rd_coff();
#endif
        if ((msgnum>0) && (msgnum<=nummsgs))
          read_message(msgnum,&next,&val);
        ansic(0);
        nl();
        if (next) {
          ++msgnum;
          if (msgnum>nummsgs)
            done=1;
          optype=2;
        } else
          optype=0;
        if (expressabort)
          if (realexpress) {
            done=1;
            quit=1;
            *nextsub=0;
          } else {
            expressabort=0;
            express=0;
            optype=0;
          }
#ifdef RIPDRIVE
        rd_con();
#endif
        break;
    }
  } while ((!done) && (!hangup));
  if (!realexpress) {
    express=0;
    expressabort=0;
  }
  if ((val & 1) && (lcs()) && (!express)) {
    nl();
    prt(5,get_string(688));
    if (yn()) {
      open_sub(1);
      for (i=1; i<=nummsgs; i++) {
        p3=get_post(i);
        if (p3->status & (status_unvalidated | status_delete))
          p3->status &= (~(status_unvalidated | status_delete));
          write_post(i, p3);
      }
      close_sub();
    }
  }
  if ((val & 2) && (lcs()) && (!express)) {
    nl();
    prt(5,get_string(689));
    if (yn()) {
      open_sub(1);
      i2=0;
      for (i=1; i<=nummsgs; i++) {
        if (get_post(i)->status & status_pending_net) {
          i2++;
        }
      }
      p3=(postrec *)malloca(i2*sizeof(postrec));
      if (p3) {
        i2=0;
        for (i=1; i<=nummsgs; i++) {
          p4=get_post(i);
          if (p4->status & status_pending_net) {
            p3[i2++]=*p4;
            p4->status &= ~status_pending_net;
            write_post(i, p4);
          }
        }

        close_sub();

        i1=0;
        for (i=0; i<i2; i++) {
          send_net_post(p3+i, subboards[curlsub].filename, curlsub);
          i1++;
        }

        bbsfree(p3);
      } else
        close_sub();

      nl();
      npr("%d",i1);
      pl(get_string(690));
      nl();
    }
  }
  if ((!quit) && (!express)) {
    nl();
    ss=syscfg.sl[actsl];
    if (
        ((restrict_post & thisuser.restrict)==0) &&
        (thisuser.posttoday<ss.posts) &&
        (actsl>=subboards[curlsub].postsl)) {
      sprintf(s,"%s %s? ",get_string(691), subboards[curlsub].name);
      prt(5,s);
      irt[0]=0;
      irt_name[0]=0;
      grab_quotes(NULL, NULL);
      if (yn()) {
        post();
      }
    }
  }
  nl();
}


void qscan(int bn, int *ns)
{
  int i,nextsub,os,sn;
  char s[81];
  unsigned long qscnptrx,sd;

  sn=usub[bn].subnum;
  g_flags &= ~g_flag_made_find_str;

  if ((hangup) || (sn<0))
    return;

  nl();

  qscnptrx=qsc_p[sn];

  if (!sub_dates[sn])
    iscan1(sn, 1);

  sd=sub_dates[sn];
  if ((!sd) || (sd>qscnptrx)) {
    nextsub=*ns;
    os=cursub;
    cursub=bn;

    if (!iscan(cursub)) {
      nl();
      pl(get_string(1195));
      return;
    }
    qscnptrx=qsc_p[sn];

    sprintf(s,"< %s %s %s - %u %s >",get_string(692), subboards[curlsub].name,
              usub[cursub].keys,nummsgs, get_string(1518));
    prt(1,s);
    nl();

    for (i=nummsgs; (i>1) && (get_post(i-1)->qscan>qscnptrx); i--)
      ;

    if ((nummsgs>0) && (i<=nummsgs) && (get_post(i)->qscan>qsc_p[curlsub])) {
      scan(i,2,&nextsub);
    } else {
      read_status();
      qsc_p[curlsub]=status.qscanptr-1;
    }

    cursub=os;
    *ns=nextsub;
    sprintf(s,"< %s %s >",subboards[curlsub].name, get_string(693));
    prt(1,s);
  } else {
    sprintf(s,"< %s %s %s >",get_string(694), subboards[sn].name, usub[bn].keys);
    prt(1,s);
  }
  nl();
}


void nscan(int ss)
{
  int i,nextsub,abort,next;

  nl();
  nextsub=1;

  prt(3,get_string(695));
  nl();
  for (i=ss; (usub[i].subnum!=-1) && (i<num_subs) && (nextsub) && (!hangup); i++) {
    if (qsc_q[usub[i].subnum/32]&(1L<<(usub[i].subnum%32)))
      qscan(i,&nextsub);
    abort=next=0;
    checka(&abort,&next);
    if (abort)
      nextsub=0;
  }
  nl();
  prt(3,get_string(696));
  nln(2);
  if ((nextsub) && (thisuser.sysstatus & sysstatus_nscan_file_system) &&
    ((syscfg.sysconfig & sysconfig_no_xfer)==0) &&
    (!(g_flags & g_flag_scanned_files))) {
    g_flags |= g_flag_scanned_files;
    abort=0;
    lines_listed=0;
    tagging=1;
    tmp_disable_conf(1);
    nscanall();
    tmp_disable_conf(0);
    tagging=0;
  }
}



void scan2(void)
{
  char s[81];
  int i,i1;

  if (!iscan(cursub)) {
    nl();
    pl(get_string(1195));
    return;
  }
  nl();
  if (curlsub<0) {
    pl(get_string(668));
    nl();
    return;
  }
  npr("%d",nummsgs);
  outstr(get_string(697));
  pl(subboards[curlsub].name);
  if (nummsgs==0)
    return;
  helpl=11;
  prt(2,get_string(698));
  input(s,5);
  i=atoi(s);
  if (i<1)
    i=0;
  else
    if (i>nummsgs)
      i=nummsgs;
    else
      i--;
  i1=0;
  if (strcmp(s,"S")==0)
    scan(0,0,&i1);
  else
    if (strcmp(s,"Q")) {
      if (strcmp(s,"N")==0) {
      } else
        scan(i,1,&i1);
    }
}


void printmenu(int i)
{
  char s[81],s1[81],s2[81], ch;
  int next, cnt;
  messagerec m;

  next=0;

  ripcode = 0;
  tmp_disable_pause(1);
  if (rip_on() == 0 || ((sysstatus_expert & thisuser.sysstatus) && useron))
    goto normalmenu;

  if (rip_subset) {
    for (cnt = 0; cnt < sizeof(menus3)/sizeof(menus3[0]); cnt++)
      if (menus3[cnt].menu_num == i) {
        sprintf(s,"%sMENUSSOF.MSG",languagedir);
        if (!exist(s))
          sprintf(s,"%sMENUSSOF.MSG", syscfg.gfilesdir);
        m.stored_as = menus3[cnt].stored_as;
        m.storage_type = 255; //menus3[cnt].storage_type;
        ripcode = 1;
        if (m.stored_as)
          read_message1(&m,0,0,&next,s);
        break;
      }
    if (ripcode == 0)       //cnt >= sizeof(menus3)/sizeof(menus3[0]))
      goto normalmenu;
  } else {
    if (strcmp(ripext, "REM") == 0) {
      if (incom) {
        while (inkey());            /* Eat any extra junk */
        /* Check for file on user's machine */
        sprintf(s, "|1F000000MENU%u.MN%c|#\r ", i, user_menus);
        comstr(s);
        do {
          ch = delaykey(1);
        } while ((ch != '1') && (ch != '0') && (!hangup));
      } else {
        sprintf(s, "%smenu%d.mn%c", sysinfo.ripdir, i, user_menus);
        ch = (exist(s)) ? '1' : '0';
      }
    }
    if (hangup)
      return;
    if ((strcmp(ripext, "REM") == 0) && (ch == '1')) {
      sprintf(s, "|1R00000000MENU%d.MN%c\r ", i, user_menus);
      remstr(s);
#ifdef RIPDRIVE
      if (rd_on()) {
        sprintf(s, "%sMENU%d.MN%c", sysinfo.ripdir, i, user_menus);
        rd_print(s);
      }
#endif
    } else if (strcmp(ripext, "RIP") != 0) {
      //sprintf(s1,"MENU%u.", i);
      //if (strcmp(ripext,"REM") == 0)
      //  strcat (s1, ripext);
      //else
            sprintf(s1,"MENU%u.MN%c", i, user_menus);
      sprintf(s,"%s%s",sysinfo.ripdir,s1);
      if (exist(s) || i >= 300) {
        ripcode = 1;
        maybeprint(s, 2);
      } else
        goto normalmenu;
    } else {
      for (cnt = 0; cnt < sizeof(menusl)/sizeof(menusl[0]); cnt++)
        if (menusl[cnt].menu_num == i) {
          sprintf(s,"%sMENUSLCL.MSG",languagedir);
          if (!exist(s))
            sprintf(s,"%sMENUSLCL.MSG", syscfg.gfilesdir);
          m.stored_as = menusl[cnt].stored_as;
          m.storage_type = 255; //menusl[cnt].storage_type;
          ripcode = 1;
          if (m.stored_as)
            read_message1(&m,0,0,&next,s);
          break;
        }
      if (ripcode == 0)
        goto normalmenu;
    }
  }
#ifdef RIPDRIVE
  if (rd_on() == 0)
#endif
	printf("[Menu %d]\r", i);
  ripcode = 0;
  tmp_disable_pause(0);
  return;
normalmenu:
  tmp_disable_pause(0);
#ifdef RIPDRIVE
  rd_coff();
#endif
  if ((thisuser.sysstatus & (sysstatus_color | sysstatus_ansi))
      == (sysstatus_color | sysstatus_ansi)) {
    sprintf(s1,"MENU%u.ANS", i);
    sprintf(s,"%s%s",languagedir,s1);
    sprintf(s2,"%s%s",syscfg.gfilesdir, s1);
    if (exist(s) || exist(s2)) {
      printfile(s1);
#ifdef RIPDRIVE
      rd_con();
#endif
      return;
    }
  }
  sprintf(s1,"MENU%u.MSG", i);
  sprintf(s,"%s%s",languagedir,s1);
  sprintf(s2,"%s%s",syscfg.gfilesdir,s1);
  if (exist(s) || exist(s2)) {
    printfile(s1);
#ifdef RIPDRIVE
    rd_con();
#endif
    return;
  }

#define VALIDM(m,n) ((n>=0) && (n<(sizeof(m)/sizeof(m[0]))) && (m[n].stored_as))

  if ((thisuser.screenchars==40) && VALIDM(menus2,i)) {
    sprintf(s,"%sMENUS40.MSG",languagedir);
    if (!exist(s))
      sprintf(s,"%sMENUS40.MSG", syscfg.gfilesdir);
    read_message1(&menus2[i],0,0,&next,s);
  } else
    if ((okansi()) && VALIDM(menus1,i)) {
      sprintf(s,"%sMENUSANS.MSG",languagedir);
      if (!exist(s))
        sprintf(s,"%sMENUSANS.MSG", syscfg.gfilesdir);
      read_message1(&menus1[i],0,0,&next,s);
    } else {
      sprintf(s,"%sMENUS.MSG",languagedir);
      if (!exist(s))
        sprintf(s,"%sMENUS.MSG",syscfg.gfilesdir);
      if (VALIDM(menus,i))
        read_message1(&menus[i],0,0,&next,s);
    }
#ifdef RIPDRIVE
  rd_con();
#endif
}


void delmail(int f, int loc)
{
  mailrec m,m1;
  userrec u;
  int rm,i,t,otf;

  sh_lseek(f,((long) loc) * ((long) sizeof(mailrec)), SEEK_SET);
  sh_read(f,(void *)&m,sizeof(mailrec));

  if ((m.touser==0) && (m.tosys==0))
    return;

  rm=1;
  if (m.status & status_multimail) {
    t=filelength(f)/sizeof(mailrec);
    otf=0;
    for (i=0; i<t; i++)
      if (i!=loc) {
        sh_lseek(f,((long)i)*((long)sizeof(mailrec)),SEEK_SET);
        sh_read(f,(void *)&m1,sizeof(mailrec));
        if ((m.msg.stored_as==m1.msg.stored_as) && (m.msg.storage_type==m1.msg.storage_type) && (m1.daten!=0xffffffff))
          otf=1;
      }
    if (otf)
      rm=0;
  }

  if (rm)
    remove_link(&m.msg,"EMAIL");

  if (m.tosys==0) {
    read_user(m.touser,&u);
    if (u.waiting) {
      --u.waiting;
      write_user(m.touser,&u);
    }
    if (m.touser==1)
      --fwaiting;
  }

  sh_lseek(f,((long) loc) * ((long) sizeof(mailrec)), SEEK_SET);
  m.touser=0;
  m.tosys=0;
  m.daten=0xffffffff;
  m.msg.storage_type=0;
  m.msg.stored_as=0xffffffff;
  sh_write(f,(void *)&m,sizeof(mailrec));
  mailcheck=1;
}

void remove_post(void)
{
  int i,any,abort;
  char s[161];
  userrec tu;

  if (!iscan(cursub)) {
    nl();
    pl(get_string(1195));
    return;
  }
  if (curlsub<0) {
    nl();
    pl(get_string(668));
    nl();
    return;
  }
#ifdef RIPDRIVE
  rd_coff();
#endif
  any=0;
  abort=0;
  nln(2);
  outstr(get_string(723));
  pl(subboards[curlsub].name);
  nl();
  for (i=1; (i<=nummsgs) && (!abort); i++) {
    if ((get_post(i)->ownersys==0) && (get_post(i)->owneruser==usernum)) {
      any=1;
      sprintf(s,"%u: %s",i,get_post(i)->title);
      pla(s,&abort);
    }
  }
#ifdef RIPDRIVE
  rd_con();
#endif
  if (!any) {
    pl(get_string(5));
    if (!cs())
      return;
  }
  nl();
  prt(2,get_string(724));
  input(s,5);
  i=atoi(s);
  open_sub(1);
  if ((i>0) && (i<=nummsgs)) {
    if (((get_post(i)->ownersys==0) && (get_post(i)->owneruser==usernum)) || (lcs())) {
      if ((get_post(i)->owneruser==usernum) && (get_post(i)->ownersys==0)) {
        read_user(get_post(i)->owneruser,&tu);
        if ((tu.inact & inact_deleted)==0) {
          if (date_to_daten(tu.firston) < get_post(i)->daten) {
            if (tu.msgpost) {
              tu.msgpost--;
              write_user(get_post(i)->owneruser,&tu);
            }
          }
        }
      }
      sprintf(s,get_stringx(1,39),get_post(i)->title,subboards[curlsub].name);
      sysoplog(s);
      delete(i);
      nl();
      pl(get_string(725));
      nl();
    }
  }
  close_sub();
}



int external_edit(char *fn1, char *direc, int ednum, int numlines, char *dest, char *title, int flags)
{
  char s[255],s1[161],fn[128],s3[81],sx1[21],sx2[21],sx3[21],cdir1[81];
  int i,filethere,mod,newtl;
  struct ftime ftimep,ftimep1;
  FILE *f;


  if ((ednum>=numed) || (!okansi())) {
    nl();
    pl(get_string(726));
    nl();
    return(0);
  }
  if (incom)
    strcpy(s1,(editors[ednum].filename));
  else
    strcpy(s1,(editors[ednum].filenamecon));
  if (s1[0]==0) {
    nl();
    pl(get_string(726));
    nl();
    return(0);
  }

  make_abs_cmd(s1);

  get_dir(cdir1,0);
  cd_to(direc);

  _chmod("editor.inf",1,0);
  unlink("editor.inf");
  _chmod("result.ed",1,0);
  unlink("result.ed");

  strcpy(s3,fn1);
  stripfn1(s3);
  if (direc[0]) {
    cd_to(direc);
    get_dir(fn,1);
    cd_to(direc);
  } else
    fn[0]=0;
  strcat(fn,s3);
  filethere=exist(fn);
  if (filethere) {
    i=sh_open1(fn,O_RDONLY | O_BINARY);
    getftime(i,&ftimep);
    sh_close(i);
  }
  itoa(thisuser.screenchars,sx1,10);
  if (screenlinest>defscreenbottom-topline)
    newtl=0;
  else
    newtl=topline;
  if (using_modem)
    itoa(thisuser.screenlines,sx2,10);
  else
    itoa(defscreenbottom+1-newtl,sx2,10);
  itoa(numlines,sx3,10);
  stuff_in(s,s1,fn,sx1,sx2,sx3,"");

  if ((f=fsh_open("EDITOR.INF","wt"))!=NULL) {
    if (irt_name[0])
      flags |= 2;
    if (irt[0])
      flags |= 4;
    fprintf(f,"%s\n%s\n%u\n%s\n%s\n%u\n%u\n%u\n%u\n",
      title,
      dest,
      usernum,
      thisuser.name,
      thisuser.realname,
      thisuser.sl,
      flags,
      topline,
      thisuser.language);
    fsh_close(f);
  }
  if (flags&1) {
    /* disable tag lines */
    f=fsh_open("disable.tag","w");
    if (f>0) fsh_close(f);
  } else {
    unlink("disable.tag");
  }
  if (!irt[0]) {
    unlink("quotes.txt");
    unlink("quotes.ind");
  }

  in_fsed=1;
#ifdef RIPDRIVE
  if (rd_on()) {
    localrip_deactivate();
    prt(2,get_string(1006));
    mpl(60);
    pl(title);
  }
#endif
  extern_prog(s, sysinfo.spawn_opts[5]);
#ifdef RIPDRIVE
  if (rd_on()) {
    localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
    sprintf(s,"%sEDIT%d.RIP",languagedir,thisuser.defed);
    if (exist(s)) {
      ripcode=1;
      maybeprint(s,2);
      ripcode=0;
    } else {
      rip_cls();
      printmenu(320);
    }
    prt(2,get_string(1006));
    mpl(60);
    pl(title);
  }
#endif
  cd_to(direc);
  in_fsed=0;
  unlink("editor.inf");
  unlink("disable.tag");
  if (!wfc)
    topscreen();
  mod=0;
  if (!filethere) {
    mod=exist(fn);
  } else {
    i=sh_open1(fn,O_RDONLY | O_BINARY);
    if (i>0) {
      getftime(i,&ftimep1);
      sh_close(i);
      if ((ftimep.ft_year!=ftimep1.ft_year) ||
          (ftimep.ft_month!=ftimep1.ft_month) ||
          (ftimep.ft_day!=ftimep1.ft_day) ||
          (ftimep.ft_hour!=ftimep1.ft_hour) ||
          (ftimep.ft_min!=ftimep1.ft_min) ||
          (ftimep.ft_tsec!=ftimep1.ft_tsec))
        mod=1;
    }
  }
  cd_to(cdir1);
  return(mod);
}

#define LINELEN 79

#define NL {if (!cp) fsh_write(pfx,1,pfxlen,f);fsh_write("\r\n",1,2,f); cp=ns=ctlc=0;}
#define FLSH {if (ss1) { if (cp && (l3+cp>=LINELEN)) NL else if (ns) cp+=fsh_write(" ",1,1,f);\
              if (!cp) {if (ctld) fprintf(f,"\x04%c",ctld);ctld=0;cp=fsh_write(pfx,1,pfxlen,f);}\
              fsh_write(ss1,1,l2,f); cp+=l3; ss1=NULL;l2=l3=0;ns=1;}}

void grab_quotes(messagerec *m, char *aux)
{
  char *ss,*ss1;
  long l,l1,l2,l3;
  FILE *f;
  char *pfx;
  int cp=0,ctla=0,ctlc=0,ns=0,ctld=0;
  int pfxlen;
  char q_txt[81],q_ind[81];

  sprintf(q_txt,"%squotes.txt",syscfgovr.tempdir);
  sprintf(q_ind,"%squotes.ind",syscfgovr.tempdir);

  _chmod(q_txt,1,0);
  unlink(q_txt);
  _chmod(q_ind,1,0);
  unlink(q_ind);
  if (quotes_nrm)
    bbsfree(quotes_nrm);
  if (quotes_ind)
    bbsfree(quotes_ind);

  quotes_nrm=quotes_ind=NULL;
  quotes_nrm_l=quotes_ind_l=0;

  if (m && aux) {

    pfx=get_stringx(1,102);
    pfxlen=strlen(pfx);

    ss=readfile(m, aux, &l);

    if (ss) {
      quotes_nrm=ss;
      quotes_nrm_l=l;

      f=fsh_open(q_txt, "wb");
      if (f) {
        fsh_write(ss, 1, l, f);
        fsh_close(f);
      }

      f=fsh_open(q_ind, "wb");
      if (f) {
        l3=l2=0;
        ss1=NULL;
        for (l1=0; l1<l; l1++) {
          if (ctld==-1)
            ctld=ss[l1];
          else switch(ss[l1]) {
            case 1:
              ctla=1;
              break;
            case 2:
              break;
            case 3:
              if (!ss1)
                ss1=ss+l1;
              l2++;
              ctlc=1;
              break;
            case 4:
              ctld=-1;
              break;
            case '\n':
              if (ctla) {
                ctla=0;
              } else {
                FLSH;
                NL;
              }
              break;
            case ' ':
            case '\r':
              if (ss1) {
                FLSH;
              } else {
                if (ss[l1]==' ') {
                  if (cp+1>=LINELEN)
                    NL;
                  if (!cp) {
                    if (ctld)
                      fprintf(f,"\x04%c",ctld);
                    ctld=0;
                    cp=fsh_write(pfx,1,pfxlen,f);
                  }
                  cp++;
                  fsh_write(" ",1,1,f);
                }
              }
              break;
            default:
              if (!ss1)
                ss1=ss+l1;
              l2++;
              if (ctlc)
                ctlc=0;
              else
                l3++;
              break;
          }
        }
        FLSH;
        if (cp)
          fsh_write("\r\n",1,2,f);
        fsh_close(f);
#ifdef SAVE_IN_MEM
        ff=sh_open1(q_ind,O_RDONLY|O_BINARY);
        if (ff>0) {
          quotes_ind_l=filelength(ff);
          quotes_ind=(char *)malloca(quotes_ind_l);
          if (quotes_ind) {
            sh_read(ff,quotes_ind, quotes_ind_l);
          } else
            quotes_ind_l=0;
          sh_close(ff);
        }
#else
        bbsfree(quotes_nrm);
        quotes_nrm=NULL;
        quotes_nrm_l=0;
#endif
      }
    }
  }
}

