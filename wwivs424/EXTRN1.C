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

#include <process.h>

#ifndef __OS2__
extern void far interrupt newintr1();
extern int shrink_out_1(char *cmdline, unsigned short flags);
#endif

#define READ(x) sh_read(f,&(x),sizeof(x))
#define WRITE(x) sh_write(f,&(x),sizeof(x))

#ifndef __OS2__
#ifdef RIPDRIVE
  extern char *kbptr, keybuffer[768];
#endif
#define SCROLL_UP(t,b,l) \
  _CH=t;\
  _DH=b;\
  _BH=curatr;\
  _AL=l;\
  _CL=0;\
  _DL=79;\
  _AH=6;\
  my_video_int();
#endif

/****************************************************************************/

static void initporte(int port_num)
/* This function initializes the com buffer, setting up the interrupt,
 * and com parameters
 */
{
#ifdef __OS2__
  (void) port_num;
#else
  int temp;

  if (!ok_modem_stuff)
    return;
  disable();
  temp=port_num;
  setvect(ISR_VECT(async_irq),async_isr);
  head=tail=0;
  outportb(base+3,0x03);
  temp=inportb(base+5);
  temp=inportb(base);
  temp=inportb(ISR_CTRLR(async_irq));
  temp=temp & ((1 << (async_irq%8)) ^ 0x00ff);
  outportb(ISR_CTRLR(async_irq),temp);
  outportb(base+1,0x01);
  temp=inportb(base+4);
  outportb(base+4,temp | 0x0A);
  enable();
  dtr(1);
  reset_colors();
  ansic(0);
#endif
}

/****************************************************************************/

static int do_it(char *cl)
{
  int i,i1,l;
  char s[256];
  char *ss[30];

  strcpy(s,cl);
  ss[0]=s;
  i=1;
  l=strlen(s);
  for (i1=1; i1<l; i1++)
    if (s[i1]==32) {
      s[i1]=0;
      ss[i++]=&(s[i1+1]);
    }
  ss[i]=NULL;
  funcs[20]=NULL;
  i=spawnvpe(P_WAIT,ss[0],ss,xenviron);
  funcs[20]=NULL;
  return(i);
}

/****************************************************************************/

int extern_prog(char *cmdline, unsigned short flags)
{
  char cmdline1[255], cmdline2[255], save_net[255], *ss;
  int rc=-1, sb, ookskey, ogh, cx, cy, xxx, dotop=0, i,i1;
  unsigned long osysstatus;
  unsigned short osysconfig;
  char restorefn[81], statfn[81];
  void interrupt far (*saveint)();
#define SHRINK_ENVIRON
#ifdef SHRINK_ENVIRON
  unsigned short envseg;
  char *env, *env1;
  unsigned short far *envptr;
  int count;
#endif
#define SAVE_ENV
#ifdef SAVE_ENV
  unsigned short es_seg;
  long es_len;
  char *es_ptr;
  char far *es_rptr;
#endif

  rip_popup=0;

  /* store flags */
  eflags = flags;

  /* forget it if the user has hung up */
  if (!(eflags & EFLAG_NOHUP)) {
    checkhangup();
    if (hangup)
      return(-1);
  }

  /* get local copy of commandline */
  strcpy(cmdline1, cmdline);

  if (eflags & EFLAG_NETPROG) {
    strcpy(save_net, cmdline1);
    sprintf(cmdline1, "%s%s", net_data, save_net);
  } else {
    save_net[0]=0;
  }

net_retry:

  /* make it into absolute pathname, if not already done so */
  make_abs_cmd(cmdline1);

  /* use command.com if a .BAT file */
  strcpy(cmdline2, cmdline1);
  strtok(cmdline2, " \t");
  if (strlen(cmdline2)>4) {
    ss=cmdline2+strlen(cmdline2)-4;
    if (stricmp(ss,".exe") && stricmp(ss,".com")) {
      if (stricmp(ss,".bat")) {
        if (save_net[0]) {
          strcpy(cmdline1, save_net);
          save_net[0]=0;
          goto net_retry;
        }
        return(-1);
      } else {
        sprintf(cmdline2, "%s /C %s", getenv("COMSPEC"), cmdline1);
        strcpy(cmdline1, cmdline2);
      }
    }
  } else {
    if (save_net[0]) {
      strcpy(cmdline1, save_net);
      save_net[0]=0;
      goto net_retry;
    }
    return(-1);
  }

  /* get ready to run it */
  sb=base;
  sl1(1,"");
  if (useron) {
    write_user(usernum,&thisuser);
    write_qscn(usernum,qsc,0);
  }
  close_strfiles();
  ookskey=okskey;
  okskey=0;
  in_extern=1;
  hanguptime1=-1L;
  abortext=0;
  ogh=global_handle;
  set_global_handle(0);


  /* ensure return.exe is there for shrinking with COMIO */
  if ((eflags & EFLAG_SHRINK) && (eflags & EFLAG_COMIO)) {
    sprintf(cmdline2, "%s\\RETURN.EXE", cdir);
    if (!exist(cmdline2)) {
      nl();
      pl(get_string(885));
      nl();
      eflags &= ~EFLAG_SHRINK;
    }
  }

  /* need files for return.exe */
  if ((eflags & EFLAG_SHRINK) && (eflags & EFLAG_COMIO)) {
    eflags |= EFLAG_FILES;
  }

  /* make shrink files, if needed */
  if (eflags & EFLAG_FILES) {
    if (eflags & EFLAG_NOPAUSE) {
      osysstatus=thisuser.sysstatus;
      thisuser.sysstatus &= ~sysstatus_pause_on_page;
      save_state(0, eflags&EFLAG_ABORT, restorefn, statfn);
      thisuser.sysstatus=osysstatus;
    } else {
      save_state(0, eflags&EFLAG_ABORT, restorefn, statfn);
    }
  }

  /* extra processing for net programs */
  if (eflags & EFLAG_NETPROG) {
    write_inst(INST_LOC_NET, net_num+1, INST_FLAGS_NONE);

    /* create ORBIT_NET.DAT, if needed */
    if ((eflags & EFLAG_SHRINK) && (status.net_version<33)) {
      sprintf(cmdline2,"%d\r\n",net_num);
      i=sh_open("ORBIT_NET.DAT",O_RDWR|O_BINARY|O_TRUNC|O_CREAT, S_IREAD|S_IWRITE);
      if (i>0) {
        sh_write(i,cmdline2,strlen(cmdline2));
        sh_close(i);
      }
    }
  }

#ifdef SAVE_ENV
  es_seg = *((unsigned short far *) MK_FP(_psp, 0x2c));
  es_rptr=MK_FP(es_seg, 0);
  es_len = ((long) (*((unsigned short far *) MK_FP(es_seg-1, 3)))) * 16L;
  es_ptr=bbsmalloc(es_len);
  if (es_ptr) {
    memcpy(es_ptr, es_rptr, es_len);
  }
#endif

  /* run it */
  if (eflags & EFLAG_SHRINK) {
    /* shrink */
    if (!(eflags & EFLAG_TOPSCREEN)) {
      set_protect(0);
      dotop=1;
    }

    if (eflags & EFLAG_COMIO) {
      sprintf(cmdline2, "%s\\RETURN.EXE 1 0 %s", cdir, cmdline1);
      strcpy(cmdline1, cmdline2);
    }

    if (ok_modem_stuff)
      closeport();

    saveint=getvect(save_dos);
    setvect(save_dos,NULL);
#ifdef SHRINK_ENVIRON
    envptr=MK_FP(_psp, 0x2c);
    envseg=*envptr;
    count=20;
    for (i=0; xenviron[i]; i++)
      count += strlen(xenviron[i])+1;
    env=(char *)malloca(count);
    if (env) {
      count=(((long)env)%16);
      if (count)
        env1= (env+16-count);
      else
        env1= env;
      count=0;
      for (i=0; xenviron[i]; i++) {
        if (strncmp(xenviron[i],"ORBIT_FP=",9)) {
          strcpy((char *) (env1+count), xenviron[i]);
          count += strlen(xenviron[i])+1;
        }
      }
      env1[count]=0;
      *envptr=FP_SEG(env1) + (FP_OFF(env1)/16);
    }
#endif
    rc=shrink_out_1(cmdline1, 0);
#ifdef SHRINK_ENVIRON
    if (env) {
      *envptr=envseg;
      bbsfree(env);
    }
#endif
    setvect(save_dos,saveint);

  } else {
    /* don't shrink */

    /* set up for comio redirect, if needed */
    if (eflags & EFLAG_COMIO) {
      if (!(eflags & EFLAG_INTERNAL) && !(eflags & EFLAG_TOPSCREEN)) {
        if (screenlinest>defscreenbottom-topline) {
          set_protect(0);
          dotop=1;
        }

        if ((screenlinest<=defscreenbottom) && (screenlinest>20)) {
          screenbottom=screenlinest-1+topline;
          cy=WhereY();
          cx=WhereX();
          xxx=cy-screenbottom+topline;
          if (xxx>0) {
            SCROLL_UP(topline,defscreenbottom,xxx);
            movecsr(cx,screenbottom);
          }
        }

      }

      setvect(save_dos, getvect(INT_REAL_DOS));
      setvect(INT_REAL_DOS, newintr1);
      osysconfig=syscfg.sysconfig;
      syscfg.sysconfig |= sysconfig_no_local;
    } else {
      if (!(eflags & EFLAG_TOPSCREEN)) {
        set_protect(0);
        dotop=1;
      }
    }

    /* disable pause, if needed */
    if (eflags & EFLAG_NOPAUSE) {
      osysstatus=thisuser.sysstatus;
      thisuser.sysstatus &= ~sysstatus_pause_on_page;
    }

    /* do it */
    rc=do_it(cmdline1);

    /* re-enable pause, if needed */
    if (eflags & EFLAG_NOPAUSE) {
      thisuser.sysstatus=osysstatus;
    }

    /* undo comio redirect, if needed */
    if (eflags & EFLAG_COMIO) {
      setvect(INT_REAL_DOS, getvect(save_dos));
      if (abortext) {
        nln(2);
        if (!rc)
          rc=1;
      }
      syscfg.sysconfig=osysconfig;
      if (!(eflags & EFLAG_INTERNAL)) {
        screenbottom=defscreenbottom;
      }
    }
  }

#ifdef SAVE_ENV
  if (es_ptr) {
    memcpy(es_rptr, es_ptr, es_len);
    bbsfree(es_ptr);
    es_ptr=NULL;
  }
#endif

  /* remove shrink files, if needed */
  if (eflags & EFLAG_FILES) {
    _chmod(restorefn, 1, 0);
    _chmod(statfn, 1, 0);
    unlink(restorefn);
    unlink(statfn);
  }

  /* kill no-longer-required ORBIT_NET.DAT file, if needed */
  if ((eflags & EFLAG_NETPROG) && (eflags & EFLAG_SHRINK)) {
    unlink("ORBIT_NET.DAT");
  }

  /* clean up */
  cd_to(cdir);
  base=sb;
  initporte(syscfgovr.primaryport);
  if (useron) {
    i=useron;
    i1=wfc;
    useron=0;
    wfc=0;
    read_user(usernum,&thisuser);
    read_qscn(usernum,qsc,0);
    useron=i;
    wfc=i1;
  }
  okskey=ookskey;
  if (in_extern==2)
    getkey();
  in_extern=0;
  if (dotop && useron)
    topscreen();
  if (ogh)
    set_global_handle(1);

  rip_popup = -1;
#ifdef RIPDRIVE
  if (rd_on()) {
	delaykey(1);
  }
  kbptr = NULL;
  *keybuffer = 0;
#endif

  /* return to caller */
  return(rc);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

void create_filename(int which, char *name)
{
  switch(which) {
    case CHAINFILE_CHAIN:
      if (instance>1)
        sprintf(name,"%s\\CHAIN.%3.3d", cdir, instance);
      else
        sprintf(name,"%s\\CHAIN.TXT", cdir);
      break;
    case CHAINFILE_DORINFO:
      if (instance < 10)
        sprintf(name,"%s\\DORINFO%d.DEF",cdir, instance);
      else if (instance==10)
        sprintf(name,"%s\\DORINFO0.DEF", cdir);
      else if (instance < 37)
        sprintf(name,"%s\\DORINFO%c.DEF",cdir, instance-10+'@');
      else
        sprintf(name,"%s\\DORINFO.%3.3d",cdir, instance);
      break;
    case CHAINFILE_PCBOARD:
      if (instance>1)
        sprintf(name,"%s\\PCBOARD.%3.3d", cdir, instance);
      else
        sprintf(name,"%s\\PCBOARD.SYS", cdir);
      break;
    case CHAINFILE_CALLINFO:
      if (instance>1)
        sprintf(name,"%s\\CALLINFO.%3.3d", cdir, instance);
      else
        sprintf(name,"%s\\CALLINFO.BBS", cdir);
      break;
    case CHAINFILE_DOOR:
      if (instance>1)
        sprintf(name,"%s\\DOOR.%3.3d", cdir, instance);
      else
        sprintf(name,"%s\\DOOR.SYS", cdir);
      break;

    default:
      /* hmm? */
      strcpy(name,"NUL:");
      break;
  }
}

/****************************************************************************/

static void getname(int wn, unsigned char *s)
{
  unsigned char *ss;

  if (!wn) {
    ss=strchr(s,' ');
    if (ss)
      s[strlen(s)-strlen(ss)]=0;
  } else {
    ss=strrchr(s,' ');
    sprintf(s,"%s",(ss)?++ss:"");
  }
}

/****************************************************************************/

static void create_drop_files(char *cspeed)
{
  char s[150],s1[81],s2[81],*ss;
  int fl=-1,h1,h2,m1,m2;
  double d;
  pcboard_sys_rec pcb;
  FILE *f;
  long l;

  /* minutes left */
  l=(long) (nsl()/60);
  l-=1L;
  if (l<0L)
    l=0L;

  /* make DORINFO1.DEF (RBBS and many others) */
  create_filename(CHAINFILE_DORINFO, s);
  unlink(s);
  f=fsh_open(s,"wt");
  if (f) {
    fprintf(f,"%s\n%s\n\nCOM%d\n",syscfg.systemname,syscfg.sysopname,
      incom?syscfgovr.primaryport:0);
    if (using_modem)
      fprintf(f,"%u ",com_speed);
    else
      /* local speed */
      fprintf(f,"0 ");
    fprintf(f,"BAUD,%s\n%c\n",(andwith==0x7f)?"E,7,1":"N,8,1",
            (multitasker&01?'4':'0'));
    if (syscfg.sysconfig & sysconfig_no_alias) {
      strcpy(s,thisuser.realname);
      getname(0,s);
      fprintf(f,"%s\n",s);
      strcpy(s,thisuser.realname);
      getname(1,s);
      fprintf(f,"%s\n",s);
    } else
      fprintf(f,"%s\n\n",thisuser.name);
    if (syscfg.sysconfig & sysconfig_extended_info)
      fprintf(f,"%s, %s\n",thisuser.city,thisuser.state);
    else
      fprintf(f,"\n");
    fprintf(f,"%c\n%d\n%ld\n",(thisuser.sysstatus & sysstatus_ansi)?'1':'0',
      thisuser.sl,(long)(l));
    fsh_close(f);
  }

  /* make PCBOARD.SYS (PC Board) */
  create_filename(CHAINFILE_PCBOARD, s);
  unlink(s);
  fl=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD|S_IWRITE);
  if (fl) {
    memset(&pcb,0,sizeof(pcb));
    strcpy(pcb.display,  "-1");
    strcpy(pcb.printer, (syscfg.sysconfig & sysconfig_printer)?"-1":" 0");
    strcpy(pcb.page_bell," 0");
    strcpy(pcb.alarm, (sysop_alert)?"-1":" 0");
    strcpy(pcb.errcheck, (modem_flag & flag_ec)?"-1":" 0");
    if (okansi()) {
      pcb.graphics = 'Y';
      pcb.ansi = '1';
    } else {
      pcb.graphics = 'N';
      pcb.ansi = '0';
    }
    pcb.nodechat=32;
    sprintf(pcb.openbps, "%-5.5s",cspeed);
    if (!incom)
      strcpy(pcb.connectbps,"Local");
    else
      sprintf(pcb.connectbps,"%-5.5u",modem_speed);
    pcb.usernum=usernum;
    sprintf(s,"%-25.25s",thisuser.name);
    ss=strtok(s," \t");
    sprintf(pcb.firstname,"%-15.15s",ss);
    strcpy(pcb.password,"XXX"); /* Don't write password - security */
    pcb.time_on=thisuser.timeon/60;
    pcb.prev_used = 0;
    d  = thisuser.timeon/60;
    h1 = (d/60)/10;
    h2 = (d/60)-(h1*10);
    m1 = (d-((h1*10+h2)*60))/10;
    m2 = (d-((h1*10+h2)*60))-(m1*10);
    pcb.time_logged[0] = h1+'0';
    pcb.time_logged[1] = h2+'0';
    pcb.time_logged[2] = ':';
    pcb.time_logged[3] = m1+'0';
    pcb.time_logged[4] = m2+'0';
    pcb.time_limit = nsl();
    pcb.down_limit = 1024;
    pcb.curconf = curconfsub;
    strcpy(pcb.slanguage,cur_lang_name);
    strcpy(pcb.name,thisuser.name);
    pcb.sminsleft = pcb.time_limit;
    if (num_instances()>1)
      pcb.snodenum = instance;
    else
      pcb.snodenum = 0;
    strcpy(pcb.seventtime, "01:00");
    strcpy(pcb.seventactive, (syscfg.executetime && syscfg.executestr[0])?
      "-1":" 0");
    strcpy(pcb.sslide," 0");
    pcb.scomport=syscfgovr.primaryport+'0';
    pcb.packflag = 27;
    pcb.bpsflag = 32;
    /* Added for PCB 14.5 Revision */
    strcpy(pcb.lastevent, status.date1);
    pcb.exittodos = '0';
    pcb.eventupcoming = '0';
    pcb.lastconfarea = curconfsub;
    /* End Additions */

    sh_write(fl, &pcb, sizeof(pcb));
    fl=sh_close(fl);
  }

  /* make CALLINFO.BBS (WildCat!) */
  create_filename(CHAINFILE_CALLINFO, s);
  unlink(s);
  f=fsh_open(s,"wt");
  if (f) {
    fprintf(f,"%s\n",thisuser.realname);
    switch (modem_speed) {
      case 300    : fprintf(f,"1\n");
      case 1200   : fprintf(f,"2\n");
      case 2400   : fprintf(f,"0\n");
      case 19200  : fprintf(f,"4\n");
      default: fprintf(f,"3\n");
    }
    fprintf(f," \n%d\n%ld\n%s\n%s\n%d\n%ld\n%.5s\n0\nABCD\n0\n0\n0\n0\n",
      thisuser.sl, l,
      (thisuser.sysstatus & sysstatus_ansi)?"COLOR":"MONO",
      "X" /* thisuser.pw */, usernum, ((long)(timeon/60)), times());
    fprintf(f,"%s\n%s 00:01\nEXPERT\nN\n%s\n%d\n%d\n1\n%d\n%d\n%s\n%s\n%d\n",
      thisuser.phone, thisuser.laston, thisuser.laston, thisuser.logons,
      thisuser.screenlines, thisuser.uploaded, thisuser.downloaded,
      (andwith==0x7f)?"7E1":"8N1",
      (incom)?"REMOTE":"LOCAL",
      (incom)?0:syscfgovr.primaryport);
    strcpy(s1,"00/00/00");
    sprintf(s2,"%d",thisuser.month);
    memmove(&(s1[2-strlen(s2)]),&(s2[0]),strlen(s2));
    sprintf(s2,"%d",thisuser.day);
    memmove(&(s1[5-strlen(s2)]),&(s2[0]),strlen(s2));
    sprintf(s2,"%d",thisuser.year);
    memmove(&(s1[8-strlen(s2)]),&(s2[0]),strlen(s2));
    fprintf(f,"%s\n",s1);
    fprintf(f,"%s\n",(incom)?cspeed:"14400");
    fsh_close(f);
  }

  /* make DOOR.SYS (Generic) */
  create_filename(CHAINFILE_DOOR, s);
  unlink(s);
  f=fsh_open(s,"wt");
  if (f) {
    fprintf(f,"COM%d\n%s\n%c\n%u\n%u\n%c\n%c\n%c\n%c\n%s\n%s, %s\n",
      (using_modem)?syscfgovr.primaryport:0,
      cspeed,
      (andwith==0x7f)?'7':'8',
      instance,  /* node */
      (using_modem)?modem_speed:14400,
      'Y', /* screen display */
      (syscfg.sysconfig & sysconfig_printer)?'Y':'N',
      'N',  /* page bell */
      'N',  /* caller alarm */
      thisuser.realname,
      thisuser.city,
      thisuser.state);
    fprintf(f,"%s\n%s\n%s\n%d\n%u\n%s\n%ld\n%ld\n",
      thisuser.phone,
      thisuser.dataphone,
      "X", /* thisuser.pw */
      thisuser.sl,
      thisuser.logons,
      thisuser.laston,
      (unsigned long)(60L*l),
      l);
    sprintf(s1,"%s",okansi()?"GR":"NG");
    if (andwith==0x7f)
      strcpy(s1,"7E");
    fprintf(f,"%s\n%u\n%c\n%s\n%u\n%s\n%u\n%c\n%u\n%u\n%u\n%u\n",
      s1,
      thisuser.screenlines,
      (thisuser.sysstatus & sysstatus_expert)?'Y':'N',
      "1,2,3", /* conferences */
      cursub,  /* current 'conference' */
      "12/31/99", /* expiration date */
      usernum,
      'Y', /* default protocol */
      thisuser.uploaded,
      thisuser.downloaded,
      0,  /* kb dl today */
      0); /* kb dl/day max */
    strcpy(s1,"00/00/00");
    sprintf(s2,"%d",thisuser.month);
    memmove(&(s1[2-strlen(s2)]),&(s2[0]),strlen(s2));
    sprintf(s2,"%d",thisuser.day);
    memmove(&(s1[5-strlen(s2)]),&(s2[0]),strlen(s2));
    sprintf(s2,"%d",thisuser.year);
    memmove(&(s1[8-strlen(s2)]),&(s2[0]),strlen(s2));
    fprintf(f,"%s\n%s\n%s\n%s\n%s\n%s\n%c\n%c\n%c\n%u\n%u\n%s\n%-.5s\n%s\n",
      s1,
      syscfg.datadir,
      syscfg.gfilesdir,
      syscfg.sysopname,
      thisuser.name,
      "00:01", /* event time */
      (modem_flag & flag_ec)?'Y':'N',
      (okansi())?'N':'Y',  /* ansi ok but graphics turned off? */
      'N', /* record-locking */
      thisuser.colors[0],
      thisuser.banktime,
      thisuser.laston, /* last n-scan date */
      times(),
      "00:01"); /* time last call */
    fprintf(f,"%u\n%u\n%ld\n%ld\n%s\n%u\n%ld\n",
      99, /* max files dl/day */
      0,  /* files dl today so far */
      thisuser.uk,
      thisuser.dk,
      thisuser.note,
      thisuser.chainsrun,
      thisuser.msgpost);
    fsh_close(f);
  }
}

/****************************************************************************/

char *create_chain_file(void)
{
  char s[81],gd[81],dd[81], cspeed[15];
  static char fpn[81];
  long l,l1;
  FILE *f;

  ultoa(com_speed,cspeed,10);
  if ((com_speed==1) || (com_speed==49664))
    strcpy(cspeed,"115200");

  create_drop_files(cspeed);
  cd_to(syscfg.gfilesdir);
  get_dir(gd,1);
  cd_to(cdir);
  cd_to(syscfg.datadir);
  get_dir(dd,1);
  cd_to(cdir);
  islname(s);
  l=(long) (timeon);
  if (l<0)
    l += 3600*24;
  l1=(long) (timer()-timeon);
  if (l1<0)
    l1 += 3600*24;

  create_filename(CHAINFILE_CHAIN, fpn);

  unlink(fpn);
  f=fsh_open(fpn,"wt");
  if (f) {
    fprintf(f,"%d\n%s\n%s\n%s\n%d\n%c\n%10.2f\n%s\n%d\n%d\n%u\n",
            usernum, thisuser.name, thisuser.realname, thisuser.callsign,
            thisuser.age, thisuser.sex, thisuser.gold, thisuser.laston,
            thisuser.screenchars, thisuser.screenlines, thisuser.sl);
    fprintf(f,"%d\n%d\n%d\n%d\n%10.2f\n%s\n%s\n%s\n",
            cs(),so(),okansi(), incom, nsl(), gd, dd, s);
    if (using_modem)
      fprintf(f,"%u\n",modem_speed);
    else
      fprintf(f,"KB\n");
    fprintf(f,"%d\n%s\n%s\n%ld\n%ld\n%lu\n%u\n%lu\n%u\n%s\n%s\n%u\n",
            syscfgovr.primaryport, syscfg.systemname, syscfg.sysopname, l, l1,
            thisuser.uk, thisuser.uploaded, thisuser.dk, thisuser.downloaded,
            (andwith==0x7f)?"7E1":"8N1", cspeed, net_sysnum);
    fprintf(f,"%c\n%c\n%c\n",(rip)?'Y':'N',(menu_on())?'Y':'N',
#ifdef RIPDRIVE
     (rd_on())?'Y':'N'
#else
     'N'
#endif
     );
    fprintf(f,"%u\n%u\n", thisuser.ar, thisuser.dar);
    fsh_close(f);
  }
  return(fpn);
}



/****************************************************************************/

void save_state(int state,int ctc, char *restorefn, char *statfn)
{
  int f,i;
  char ldir[81];

  get_dir(ldir, 1);
  if (instance>1) {
    sprintf(restorefn, "%sRESTORE.%3.3d", ldir, instance);
    sprintf(statfn, "%sSTAT.%3.3d", ldir, instance);
  } else {
    sprintf(restorefn, "%sRESTORE.WWV", ldir);
    sprintf(statfn, "%sSTAT.WWV", ldir);
  }
  _chmod(restorefn, 1, 0);
  _chmod(statfn, 1, 0);
  unlink(restorefn);
  unlink(statfn);

  /* if (1) */ {
    f=sh_open(restorefn,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (f<0)
      return;

    WRITE(state);
    WRITE(oklevel);
    WRITE(noklevel);
    WRITE(ooneuser);
    WRITE(no_hangup);
    WRITE(ok_modem_stuff);
    WRITE(topdata);
    WRITE(last_time_c);
    WRITE(sysop_alert);
    WRITE(do_event);

    WRITE(andwith);
    WRITE(usernum);
    WRITE(chatcall);
    WRITE(chatreason);
    WRITE(timeon);
    WRITE(extratimecall);
    WRITE(curspeed);
    WRITE(modem_speed);
    WRITE(com_speed);
    WRITE(modem_flag);
    WRITE(cursub);
    WRITE(curdir);
    WRITE(curdloads);
    WRITE(msgreadlogon);
    WRITE(nscandate);
    WRITE(mailcheck);
    WRITE(smwcheck);
    WRITE(use_workspace);
    WRITE(using_modem);
    WRITE(last_time);
    WRITE(fsenttoday);
    WRITE(global_xx);
    WRITE(xtime);
    WRITE(xdate);
    WRITE(incom);
    WRITE(outcom);
    WRITE(global_handle);
    WRITE(actsl);
    WRITE(numbatch);
    WRITE(numbatchdl);

    sh_write(f,batch, numbatch*sizeof(batchrec));
    WRITE(batchtime);


    WRITE(last_time);
    WRITE(time_event);
    WRITE(syscfg.executetime);
    WRITE(syscfg.executestr);
    i=uconfsub[curconfsub].confnum;
    WRITE(i);
    i=uconfdir[curconfdir].confnum;
    WRITE(i);
    i=usub[cursub].subnum;
    WRITE(i);
    i=udir[curdir].subnum;
    WRITE(i);

    set_global_handle(0);

    sh_close(f);

    _chmod(restorefn, 1, FA_HIDDEN);
  }

  f=sh_open(statfn,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f<0)
    return;

  WRITE(incom);
  WRITE(outcom);
  WRITE(thisuser);
  WRITE(flow_control);
  WRITE(async_irq);
  WRITE(com_speed);
  WRITE(base);
  WRITE(andwith);
  WRITE(ctc);
  WRITE(defscreenbottom);
  WRITE(ok_modem_stuff);
  WRITE(save_dos);

  sh_close(f);

  _chmod(statfn, 1, FA_HIDDEN);

}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

#ifdef OLD_STUFF

int restore_data(char *s) /**/
{
  int f,stat,i;
  char s1[81];

  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    return(-1);

  READ(stat);
  READ(oklevel);
  READ(noklevel);
  READ(ooneuser);
  READ(no_hangup);
  READ(ok_modem_stuff);
  READ(topdata);
  READ(last_time_c);
  READ(sysop_alert);
  READ(do_event);

  if (stat) {
    READ(andwith);
    READ(usernum);
    READ(chatcall);
    READ(chatreason);
    READ(timeon);
    READ(extratimecall);
    READ(curspeed);
    READ(modem_speed);
    READ(com_speed);
    READ(modem_flag);
    READ(cursub);
    READ(curdir);
    READ(curdloads);
    READ(msgreadlogon);
    READ(nscandate);
    READ(mailcheck);
    READ(smwcheck);
    READ(use_workspace);
    READ(using_modem);
    READ(last_time);
    READ(fsenttoday);
    READ(global_xx);
    READ(xtime);
    READ(xdate);
    READ(incom);
    READ(outcom);
    READ(global_handle);
    READ(actsl);
    READ(numbatch);
    READ(numbatchdl);

    sh_read(f,batch, numbatch*sizeof(batchrec));
    READ(batchtime);

    READ(last_time);
    READ(time_event);
    READ(syscfg.executetime);
    READ(syscfg.executestr);
    READ(i);
    uconfsub[0].confnum=i;
    READ(i);
    uconfdir[0].confnum=i;
    curconfsub=curconfdir=0;
    READ(i);
    usub[0].subnum=i;
    READ(i);
    udir[0].subnum=i;
    cursub=curdir=0;

    if (global_handle) {
      global_handle=0;
      set_global_handle(1);
    }

    read_user(usernum,&thisuser);
    read_qscn(usernum,qsc,0);
    if (set_language(thisuser.language)) {
      thisuser.language=0;
      set_language(thisuser.language);
    }

    useron=1;
    changedsl();
    topscreen();
  }
  if (ok_modem_stuff) {
    initport(syscfgovr.primaryport);
    if (stat) {
      set_baud(com_speed);
      if (modem_flag & flag_fc)
        flow_control = 1;
    } else {
      do_result(&(modem_i->defl));
    }
  }

  sh_close(f);
  unlink(s);
  if(instance > 1)
    sprintf(s1,"STAT.%3.3d",instance);
  else
    sprintf(s1,"STAT.WWV");
  unlink(s1);
  return(stat);
}

#endif

