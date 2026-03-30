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

#define FRAME 7

/****************************************************************************/

/*
 * Makes various (local-only) sounds based upon input params. The params are:
 *     sf = starting frequency, in hertz
 *     ef = ending frequency, in hertz
 *     uf = frequency change, in hertz, for each step
 *   dly1 = delay, in milliseconds, between each step, when going from sf
 *             to ef
 *   dly2 = delay, in milliseconds, between each repetition of the sound
 *             sequence
 *     rp = number of times to play the whole sound sequence
 */

#ifdef __OS2__
#define sound(freq) setbeep(freq)
#define nosound() setbeep(0)
#endif

void chatsound(int sf, int ef, int uf, int dly1, int dly2, int rp)
{
  int i,i1;

  for (i1=0; i1<rp; i1++) {
    if (sf < ef) {
      for (i=sf; i<ef; i+=uf) {
        sound(i);
        delay(dly1);
      }
    } else {
      for (i=ef; i>sf; i-=uf) {
        sound(i);
        delay(dly1);
      }
    }
    delay(dly2);
  }
  nosound();
}

/****************************************************************************/

/* Chat with Sysop removed in OrbitBBS */
void reqchat(void) {}

/****************************************************************************/

/*
 * Displays some basic user statistics for the current user.
 */

void yourinfo(void)
{

  if (menu_on()) {
    rip_yourinfo();
    return;
  }

  outchr(12);
  ansic_x(1); outstr(get_string(602));
  ansic_x(2); pl(nam(&thisuser,usernum));
  ansic_x(1); outstr(get_string(603));
  ansic_x(2); pl(thisuser.phone);
  if (thisuser.waiting) {
    ansic_x(1); outstr(get_string(1001));
    ansic_x(2); npr("%d\r\n",thisuser.waiting);
  }
  ansic_x(1); outstr(get_string(604));
  ansic_x(2); npr("%d",thisuser.sl);

  if (actsl!=thisuser.sl) {
    ansic_x(1); outstr(get_string(605));
    ansic_x(2); npr("%d",actsl);
    pl(")");
  } else
    nl();
  ansic_x(1); outstr(get_string(606));
  ansic_x(2); pln(thisuser.dsl);
  ansic_x(1); outstr(get_string(1004));
  ansic_x(2); pl(thisuser.laston);
  ansic_x(1); outstr(get_string(607));
  ansic_x(2); pln(thisuser.logons);
  ansic_x(1); outstr(get_string(608));
  ansic_x(2); pln(thisuser.ontoday);
  ansic_x(1); outstr(get_string(609));
  ansic_x(2); pln(thisuser.msgpost);
  ansic_x(1); outstr(get_string(610));
  ansic_x(2); pln((thisuser.emailsent+thisuser.feedbacksent+thisuser.emailnet));
  ansic_x(1); outstr(get_string(611));
  ansic_x(2); npr("%ld",(long)((thisuser.timeon+timer()-timeon)/60.0));
  pl(get_string(612));

}

/****************************************************************************/

/*
 * Allows user to upload a post.
 */

void upload_post(void)
{
  char s[81],ch;
  int i,maxli,f;
  long l,l1;

  if (actsl<=syscfg.newusersl)
    maxli=60;
  else if (cs())
    maxli=120;
  else
    maxli=80;

  sprintf(s,"%sINPUT.MSG",syscfgovr.tempdir);
  l1=250*(long)maxli;

  nl();
  outstr(get_string(613));
  npr("%ld\r\n",l1);
  nl();
  receive_file(s,&i,&ch, "INPUT.MSG", -1);
  f=sh_open1(s,O_RDWR | O_BINARY);
  if (f>0) {
    l=filelength(f);
    if (l>l1) {
      nl();
      pl(get_string(614));
      nl();
      sh_close(f);
      unlink(s);
    } else {
      sh_close(f);
      use_workspace=1;
      nl();
      pl(get_string(615));
      nl();
    }
  } else {
    nl();
    pl(get_string(452));
    nl();
  }
}

/****************************************************************************/

/*
 * Copies a file (filename=*input) to another destination (filename=*output).
 */

void copy_file(char *input, char *output)
{
  int f1, f2, i;
  char *b;
  struct ftime ft;

  if ((strcmp(input, output) != 0) && (exist(input)) && (!exist(output))) {
    if ((b = malloca(16400)) == NULL)
      return;
    f1 = sh_open1(input, O_RDONLY | O_BINARY);
    getftime(f1, &ft);
    f2 = sh_open(output, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    i = read(f1, (void *) b, 16384);
    while (i > 0) {
      write(f2, (void *) b, i);
      i = read(f1, (void *) b, 16384);
    }
    f1=sh_close(f1);
    setftime(f2, &ft);
    f2=sh_close(f2);
    bbsfree(b);
  }
#ifdef LAZY_WRITES
  wait1(LAZY_WRITES);
#endif
}

/****************************************************************************/

/*
 * Handles reading short messages. This is also where PackScan file requests
 * plug in, if such are used.
 */

void rsm(int un, userrec *u, int mode)
{
  shortmsgrec sm;
  int i,i1,f,handled,any, all;
  char s1[81];

  any=0;
  all=1;
  if ((u->sysstatus) & sysstatus_smw) {
    sprintf(s1,"%sSMW.DAT",syscfg.datadir);
    f=sh_open(s1,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    if (f<0)
      return;
    i=(int) (filelength(f) / sizeof(shortmsgrec));
    for (i1=0; i1<i; i1++) {
      sh_lseek(f,((long) (i1)) * sizeof(shortmsgrec),SEEK_SET);
      sh_read(f,(void *)&sm,sizeof(shortmsgrec));
      if ((sm.touser==un) && (sm.tosys==0)) {
        pl(sm.message);
        handled=0;
        any=1;
        if ((!so()) || (!mode)) {
          handled=1;
        } else {
          if (sysinfo.flags & OP_FLAGS_PACKSCAN_FREQ) {
            if ( (strstr(sm.message,"UPL"))
              || (strstr(sm.message,"upload "))
              || (strstr(sm.message,"U/L"))) {
              handled=mode;
              handled=remoteupload(sm.message);
            }
          } else {
            handled=mode;
            handled=0;
          }

          if (sysinfo.flags & OP_FLAGS_CAN_SAVE_SSM) {
            if ((handled==0) && (mode)) {
              npr(get_string(1044));
              handled=!yn();
            }
          } else {
            handled=1;
          }

        }
        if (handled==1) {
          sm.touser=0;
          sm.tosys=0;
          sm.message[0]=0;
          sh_lseek(f,((long) (i1)) * sizeof(shortmsgrec),SEEK_SET);
          sh_write(f,(void *)&sm,sizeof(shortmsgrec));
        } else {
          all=0;
        }
      }
    }
    sh_close(f);
    smwcheck=1;
  }
  if (any)
    nl();
  if (all)
    u->sysstatus &= ~sysstatus_smw;
}

/****************************************************************************/

void show_chains(int *mapp, int *map)
{
  int abort=0,i,i1;
  char s[121],s1[121];
  userrec u;

  if ((sysinfo.flags & OP_FLAGS_CHAIN_REG) && chains_reg) {
    abort=0;
    nl();
    strcpy(s,get_string(1045));
    sprintf(s1,"%-42.42s2",get_string(1046));
    strcat(s,s1);
    sprintf(s1,"%-22.22s1",get_string(1047));
    strcat(s,s1);
    sprintf(s1,"%-5.5s",get_string(1048));
    strcat(s,s1);
    pla(s,&abort);

    if (okansi())
      sprintf(s,"%d %s",FRAME,get_string(1049));
    else
      sprintf(s,get_string(1050));
    pla(s,&abort);
    for (i=0; (i<*mapp) && (!abort) && (!hangup); i++) {
      strcat(s,". ");
      if (okansi()) {
        read_user(chains_reg[map[i]].regby[0],&u);
        sprintf(s," %d�5%3d%d�1%-41s%d�%d%-21s%d�1%5d%d�",
          FRAME, i+1, FRAME, chains[map[i]].description,
          FRAME, (chains_reg[map[i]].regby[0])?2:3,
          (chains_reg[map[i]].regby[0]) ? u.name : get_string(315),
          FRAME, chains_reg[map[i]].usage, FRAME );
        pla(s,&abort);
        if (chains_reg[map[i]].regby[0]!=0) {
          for(i1=1;i1<5;i1++) {
            if (chains_reg[map[i]].regby[i1]!=0) {
              read_user(chains_reg[map[i]].regby[i1],&u);
              sprintf(s," %d�   �%-41s�2%-21s%d�%5.5s�",
                FRAME, " ", u.name, FRAME, " ");
              pla(s,&abort);
            }
          }
        }
      } else {
        read_user(chains_reg[map[i]].regby[0],&u);
        sprintf(s," |%3d|%-41.41s|%-21.21s|%5d|",
          i+1,chains[map[i]].description,
          (chains_reg[map[i]].regby[0]) ? u.name : get_string(315),
          chains_reg[map[i]].usage);
        pla(s,&abort);
        if (chains_reg[map[i]].regby[0]!=0) {
          for(i1=1;i1<5;i1++) {
            if (chains_reg[map[i]].regby[i1]!=0) {
              read_user(chains_reg[map[i]].regby[i1],&u);
              sprintf(s," |%3.3s|%-41.41s|%-21.21s|%5.5s|",
                " "," ",(chains_reg[map[i]].regby[i1]) ?
                u.name : get_string(315)," ");
              pla(s,&abort);
            }
          }
        }
      }
    }
    if (okansi())
      sprintf(s,"%d %s",FRAME,get_string(1051));
    else
      sprintf(s,get_string(1052));
    pla(s,&abort);

  } else {

    abort=0;
    nl();
    for (i=0; (i<*mapp) && (!abort) && (!hangup); i++) {
      sprintf(s,"%d. %s",i+1, chains[map[i]].description);
      pla(s,&abort);
    }
    nl();
  }
}

/****************************************************************************/

/*
 * Executes a "chain", index number cn.
 */

void run_chain(int cn)
{
  char s[255],s1[81],s2[81],s3[81];
  int oc,inst,f;
  unsigned short fl;

  inst=(inst_ok(INST_LOC_CHAINS, cn+1));
  if (inst!=0) {
    strcpy(s,get_string(1053));
    strcat(s,chains[cn].description);
    strcat(s,get_string(1054));
    sprintf(s1,"%d.  ",inst);
    strcat(s,s1);
    if (!(chains[cn].ansir & ansir_multi_user)) {
      strcat(s,get_string(1055));
      pl(s);
      return;
    } else {
      strcat(s,get_string(1056));
      outstr(s);
      if (!(yn()))
        return;
    }
  }

  write_inst(INST_LOC_CHAINS, cn+1, INST_FLAGS_NONE);
  if ((sysinfo.flags & OP_FLAGS_CHAIN_REG) && chains_reg) {
    chains_reg[cn].usage++;
    sprintf(s,"%s%s",syscfg.datadir,get_string(1057));
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if (f) {
      sh_write(f,(void *)chains_reg, numchain * sizeof(chainregrec));
      f=sh_close(f);
    }
  }
  itoa(com_speed, s1, 10);
  if ((com_speed==1) || (com_speed==49664))
    strcpy(s1,"115200");
  itoa(syscfgovr.primaryport,s2,10);
  itoa(modem_speed, s3, 10);
  stuff_in(s,chains[cn].filename,create_chain_file(),s1,s2,s3,"");
  sprintf(s2,get_stringx(1,35),chains[cn].description);
  sysoplog(s2);
  oc=chatcall;
  chatcall=0;
  thisuser.chainsrun++;
  fl=0;
  if (!(chains[cn].ansir & ansir_no_DOS))
    fl |= EFLAG_COMIO;
  if (chains[cn].ansir & ansir_shrink)
    fl |= EFLAG_SHRINK|EFLAG_FILES;
  if (chains[cn].ansir & ansir_no_pause)
    fl |= EFLAG_NOPAUSE;

  extern_prog(s, fl);

  chatcall=oc;
  topscreen();
}

/****************************************************************************/

/*
 * Main high-level function for chain access and execution.
 */

void do_chains(void)
{
  int *map,mapp,i,ok,done,start=0;
  char *ss,s[67];
  chainfilerec c;
  chainregrec r;

  if (menu_on()) {
    rip_cls();
  }

  map=(int *)malloca(sysinfo.max_chains * sizeof(int));
  if (!map)
    return;

  tleft(1);
  mapp=0;
  memset(odc, 0, sizeof(odc));
  /* memset(otc, 0, sizeof(otc)); */
  for (i=0; i<numchain; i++) {
    ok=1;
    c=chains[i];
    if ((c.ansir & ansir_ansi) && (!okansi()))
      ok=0;
    if ((c.ansir & ansir_rip_only) && (!rip_on()))
      ok=0;
    if ((c.ansir & ansir_no_300) && (modem_speed==300) && incom)
      ok=0;
    if ((c.ansir & ansir_local_only) && (using_modem))
      ok=0;
    if (c.sl>actsl)
      ok=0;
    if (c.ar)
      if ((c.ar & thisuser.ar)==0)
        ok=0;
    if ((sysinfo.flags & OP_FLAGS_CHAIN_REG) && chains_reg && (actsl<255)) {
      r=chains_reg[i];
      if (r.maxage) {
        if ((r.minage>thisuser.age) || (r.maxage < thisuser.age))
          ok=0;
      }
    }
    if (ok) {
      map[mapp++]=i;
      if (mapp<100) {
        if ((mapp % 10) ==0)
          odc[mapp/10 -1]='0'+(mapp/10);
      } else {
        /* if ((mapp%100)==0)
          otc[mapp/100 -1] = '0' + (mapp/100); */
      }
    }
  }
  if (mapp==0) {
    nln(2);
    pl(get_string(620));
    nl();
    bbsfree(map);
    return;
  }
#ifdef AUTO_START
  if (mapp==1) {
    run_chain(map[0]);
    bbsfree(map);
    return;
  }
#endif
  if ((menu_on() == 0) || rip_subset)
    show_chains(&mapp,map);

  done=0;
  curdloads=0;
  do {
    tleft(1);

    if (menu_on() && (rip_subset==0))
      rip_show_chains(&mapp,map,start);
    nl();
    outstr(get_string(1058));
    npr("%d",mapp);
    outstr(get_string(1059));

    if (mapp<100) {
      ss=mmkey(2);
      i=atoi(ss);
    } else {
      input(s,3);
      i=atoi(s);
    }
    if ((i>0) && (i<=mapp)) {
      done=1;
      curdloads=2;

      if (menu_on()) {
        rip_cls();
        sprintf(s,"%sdoor%d.rip",languagedir,map[i-1]+1);
        if (exist(s)) {
          ripcode = 1;
          maybeprint(s,2);
          ripcode=0;
        }
      }
#ifdef RIPDRIVE
      if (rd_on())
        localrip_deactivate();
#endif
      run_chain(map[i-1]);
#ifdef RIPDRIVE
      if (rd_on())
        localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
#endif
      rip_cls();
    } else
      if (strcmp(ss,"Q")==0)
        done=1;
      else if (strcmp(ss,"?")==0)
          show_chains(&mapp,map);
      else if (strcmp(ss,"P")==0) {
        if (start > 0)
          start -= 14;
        if (start < 0)
          start = 0;
      } else if (strcmp(ss,"N")==0) {
        if (start+14 < mapp)
          start += 14;
      }
  } while ((!hangup) && (!done));
  if (menu_on()) {
    nl();
    rip_cls();
    cleared = -1;
    rmenu = 300;                  /* Menu bar stuff */
  }

  bbsfree(map);
}

/****************************************************************************/

/*
 * Compresses file *fn to directory *dir.
 */

void compress_file(char *fn, char *dir)
{
  char s1[161],s2[81],s3[81],s4[81],s5[81], *ss;

  outstr(get_string(1196));
  pl(fn);
  if (strchr(fn,'.')==NULL)
    strcat(fn,".MSG");
  strcpy(s1,fn);
  strcpy(s2, fn);
  ss=strchr(s2,'.');
  if (ss) ss[1]=0;
  strcat(s2,syscfg.arcs[0].extension);
  strcpy(s5,syscfg.arcs[0].arca);
  strcpy(s3,dir);
  strcat(s3,s2);
  strcpy(s4,dir);
  strcat(s4,s1);
  stuff_in(s1,s5,s3,s4,"","","");
  extern_prog(s1, sysinfo.spawn_opts[11]);
  unlink(s4);
  topscreen();
}

/****************************************************************************/

/*
 * Allows extracting a message into a file area, directly.
 */

void extract_mod(char *b, long len)
{
  char s1[81],s2[81],s4[81],ch=26,ch1,*ss1;
  int i,i1,exists,quit,mod_dir;

  tmp_disable_conf(1);
  do {
    prt(2,get_string(1198));
    ss1=mmkey(1);
    if (ss1[0]=='?')
      dirlist();
  } while ((!hangup) && (ss1[0]=='?'));

  mod_dir=-1;
  for (i1=0; (i1<num_dirs) && (udir[i1].subnum!=-1); i1++)
    if (strcmp(udir[i1].keys,ss1)==0)
      mod_dir=i1;

  if (mod_dir==-1) {
    goto go_away;
  }

  strcpy(s1,directories[udir[mod_dir].subnum].path);
  do {
    exists=quit=0;
    prt(2,get_string(1199));
    input(s2,12);
    if (!s2[0]) {
      goto go_away;
    }
    if (strchr(s2,'.')==NULL)
      strcat(s2,".MOD");
    sprintf(s4,"%s%s",s1,s2);
    if (exist(s4)) {
      exists=1;
      sprintf(s4,"%s%s",s2,get_string(1200));
      nl();
      pl(s4);
      nl();
    }
    if (exists==1) {
      prt(2,get_string(1201));
      ch1=onek("QN");
      switch(ch1) {
        case 'Q':
          quit=1;
          break;
        case 'N':
          s2[0]=0;
          break;
      }
      nl();
    }
  } while ((!hangup) && (s2[0]==0) && (!quit));

  if ((quit!=1) && (!hangup)) {
    i=sh_open(s4,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    sh_lseek(i,0L,SEEK_END);
    sh_write(i,(void *)b,len);
    sh_write(i,&ch,1);
    sh_close(i);
    npr("%s%s\r\n",get_string(1202),s4);
    compress_file(s2,s1);
    nln(2);
    prt(2,get_string(1203));
    if (yn()) {
      prt(5,get_string(1204));
      quit=uploadall(mod_dir);
    }
  }
go_away:
  tmp_disable_conf(0);
}

void extract_out(char *b, long len, char *title)
{
  char s1[81],s2[81],s3[81],ch=26,ch1;
  int i,done;

  if (sysinfo.flags & OP_FLAGS_NEW_EXTRACT) {
    do {
      s1[0]=0; s2[0]=0; s3[0]=0;
      done=1;
      if (menu_on()) {
        rip_saveall();
        printmenu(18);
      } else
        prt(5,get_string(1197));
      ch1=onek("Q1234?");
      if (menu_on())
        rip_restoreall();
      switch(ch1) {
        case '1':
          extract_mod(b,len);
          break;
        case '2':
          strcpy(s2,syscfg.gfilesdir);
          break;
        case '3':
          strcpy(s2,syscfg.datadir);
          break;
        case '4':
          strcpy(s2,syscfgovr.tempdir);
          break;
        case '?':
          printmenu(18);
          done=0;
          break;
      }
    } while ((!done) && (!hangup));

    if (s2[0]) {
      do {
        prt(2,get_string(674));
        input(s1,50);
        if (s1[0]) {
          if (strchr(s1,':') || strchr(s1,'\\'))
            strcpy(s3,s1);
          else
            sprintf(s3,"%s%s",s2,s1);
          if (exist(s3)) {
            nl();
            pl(get_string(675));
            nl();
            outstr(get_string(676));
            ch1=onek("QOAN");
            switch(ch1) {
              case 'Q':
                s3[0]=1;
                s1[0]=0;
                break;
              case 'N':
                s3[0]=0;
                break;
              case 'A':
                break;
              case 'O':
                unlink(s3);
                break;
            }
            nl();
          }
        } else
          s3[0]=1;
      } while ((!hangup) && (s3[0]==0));
      if ((s3[0]) && (!hangup)) {
        if (s3[0]!=1) {
          i=sh_open(s3,O_RDWR | O_BINARY | O_CREAT , S_IREAD | S_IWRITE);
          if (i<=0) {
            pl(get_string(933));
          } else {
            if (filelength(i)) {
              sh_lseek(i, -1L, SEEK_END);
              sh_read(i, ((void *)&ch1), 1);
              if (ch1 == 26)
                sh_lseek(i, -1L, SEEK_END);
            }
            sh_write(i,title,strlen(title));
            sh_write(i,"\r\n",2);
            sh_write(i,(void *)b,len);
            sh_write(i,&ch,1);
            sh_close(i);
            outstr(get_string(677));
            pl(s3);
          }
        }
      }
    }
  } else {
    do {
      prt(2,get_string(674));
      input(s1,50);
      if (s1[0]) {
        if (strchr(s1,':') || strchr(s1,'\\'))
          strcpy(s2,s1);
        else
          sprintf(s2,"%s%s",syscfg.gfilesdir,s1);
        if (exist(s2)) {
          nl();
          pl(get_string(675));
          nl();
          outstr(get_string(676));
          ch1=onek("QOAN");
          switch(ch1) {
            case 'Q':
              s2[0]=0;
              s1[0]=0;
              break;
            case 'N':
              s1[0]=0;
              break;
            case 'A':
              break;
            case 'O':
              unlink(s2);
              break;
          }
          nl();
        }
      } else
        s2[0]=0;
    } while ((!hangup) && (s2[0]!=0) && (s1[0]==0));
    if ((s1[0]) && (!hangup)) {
      i=sh_open(s2,O_RDWR | O_BINARY | O_CREAT , S_IREAD | S_IWRITE);
      if (i<=0) {
        pl(get_string(933));
      } else {
        if (filelength(i)) {
          sh_lseek(i, -1L, SEEK_END);
          sh_read(i, ((void *)&ch1), 1);
          if (ch1 == 26)
            sh_lseek(i, -1L, SEEK_END);
        }
        sh_write(i,title,strlen(title));
        sh_write(i,"\r\n",2);
        sh_write(i,(void *)b,len);
        sh_write(i,&ch,1);
        sh_close(i);
        outstr(get_string(677));
        pl(s2);
      }
    }
  }
  bbsfree(b);
}

/****************************************************************************/

/*
 * High-level function for sending email.
 */

void send_email(void)
{
  char s1[81];
  unsigned short sy,un;

  write_inst(INST_LOC_EMAIL,0,INST_FLAGS_NONE);
  nln(2);
  pl(get_string(15));
  helpl=14;
  outstr(":");
  input(s1,75);
  helpl=0;
  irt[0]=0;
  irt_name[0]=0;
  parse_email_info(s1,&un,&sy);
  grab_quotes(NULL, NULL);
  if (un || sy)
    email(un,sy,0,0);
}

/****************************************************************************/

/*
 * High-level function for selecting conference type to edit.
 */

void edit_confs(void)
{
  char ch;

  if (!checkpw())
    return;

  nln(2);
  pl(get_string(1023));
  nl();
  pl(get_string(1024));
  pl(get_string(1025));
  nl();
  outstr(get_string(1026));
  ch=onek("Q12");
  switch (ch) {
    case '1':
      conf_edit(CONF_SUBS);
      break;
    case '2':
      conf_edit(CONF_DIRS);
      break;
    default:
      break;
  }
}

/****************************************************************************/

/*
 * If nuf>0 then this is newuser feedback, else it is "normal" feedback.
 * Shows users with usernum<10 who are sysops, so user can select which
 * sysop to leave feedback to.
 */

void feedback(int nuf)
{
  int i,i1,nu;
  char onek_str[20],ch;
  userrec u;

  irt_name[0]=0;
  grab_quotes(NULL, NULL);

  if (nuf) {
    read_status();
    sprintf(irt,get_stringx(1,30),syscfg.maxusers-status.users);
    email(1,0,1,0);
    return;
  }

  strcpy(irt,get_stringx(1,14));
  nu=number_userrecs();
  i1=0;

  for (i=2; (i<10) && (i<nu); i++) {
    read_user(i,&u);
    if (((u.sl==255) || (syscfg.sl[u.sl].ability & ability_cosysop)) &&
       ((u.inact & inact_deleted)==0)) {
      i1++;
    }
  }

  if (!i1) {
    i=1;
  } else {
    onek_str[0]=0;
    i1=0;
    nl();
    for (i=1;((i<10) && (i<nu));i++) {
      read_user(i,&u);
      if (((u.sl==255) || (syscfg.sl[u.sl].ability & ability_cosysop)) &&
         ((u.inact & inact_deleted)==0)) {
        ansic(2); npr("%d",i);
        ansic(7); npr(")");
        ansic(1); npr(" %s",nam(&u,i));
        nl();
        onek_str[i1++]='0'+i;
      }
    }
    onek_str[i1++]=*str_quit;
    onek_str[i1]=0;
    nl();
    ansic(1); npr("%s (%s): ",get_string(1003), onek_str);
    mpl(1);
    ch=onek(onek_str);
    if (ch==*str_quit)
      return;
    nl();
    i=ch-'0';
  }
  email(i,0,0,0);
}

/****************************************************************************/

/*
 * Lists the available subs for the current user.
 */

void sublist(void)
{
  int i,i1,abort,oc,os,sn,en;
  char s[80],s1[80],*ss,ch;

  oc=curconfsub; os=usub[cursub].subnum;

  abort=0;
  sn=0;
  en=subconfnum-1;
  if (okconf(&thisuser)) {
    if (uconfsub[1].confnum!=-1) {
      nl();
      prt(2,get_string(1019));
      ch=onek("Q A");
      nl();
      switch (ch) {
        case ' ':
          sn=curconfsub;
          en=curconfsub;
          break;
        case 'Q':
          return;
      }
    }
  } else
    oc=-1;

  nl();
  ansic_x(1);
  pla(get_string(617),&abort);
  nl();
  i=sn;
  while ((i<=en) && (uconfsub[i].confnum!=-1) && (!abort)) {
    if ((uconfsub[1].confnum!=-1) && (okconf(&thisuser))) {
      setuconf(CONF_SUBS, i, -1);
      sprintf(s,"1%s %c0:2 %s",get_string(1021),
        subconfs[uconfsub[i].confnum].designator,
        stripcolors(subconfs[uconfsub[i].confnum].name));
      if (E_C) {
        pla(s,&abort);
      } else {
        pla(stripcolors(s),&abort);
      }
    }
    i1=0;
    while ((i1<num_subs) && (usub[i1].subnum!=-1) && (!abort)) {
      sprintf(s,"  5%4.4s2",usub[i1].keys);
      if (qsc_q[usub[i1].subnum/32]&(1L<<(usub[i1].subnum%32)))
        strcat(s,get_string(3));
      else
        strcat(s,get_string(4));
      if (net_sysnum || (net_num_max>1)) {
        if (xsubs[usub[i1].subnum].num_nets) {
          if (xsubs[usub[i1].subnum].num_nets>1)
            ss=get_string(1022);
          else
            ss=stripcolors(net_networks[xsubs[usub[i1].subnum].nets[0].net_num].name);
          if (subboards[usub[i1].subnum].anony & anony_val_net)
            sprintf(s1,"4[%-8.8s]9 ",ss);
          else
            sprintf(s1,"4<%-8.8s>9 ",ss);
          strcat(s,s1);
        } else
          strcat(s,charstr(11,' '));
          strcat(s,"9");
      }
      strcat(s,stripcolors(subboards[usub[i1].subnum].name));
      if (E_C) {
        pla(s,&abort);
      } else {
        pla(stripcolors(s),&abort);
      }
      i1++;
    }
    i++;
    nl();
    if (!okconf(&thisuser))
      break;
  }
  if (i==0) {
    ansic(6);
    pla(get_string(5),&abort);
    nl();
  }

  if (okconf(&thisuser))
    setuconf(CONF_SUBS, oc, os);
}

/****************************************************************************/

/*
 * Displays the available file areas for the current user.
 */

void dirlist(void)
{
  int i,i1,oc,os,abort,sn,en;
  char s[80],ch;

  oc=curconfdir; os=udir[curdir].subnum;

  abort=0;
  sn=0;
  en=dirconfnum-1;
  if (okconf(&thisuser)) {
    if (uconfdir[1].confnum!=-1) {
      nl();
      prt(2,get_string(1019));
      ch=onek("Q A");
      nl();
      switch (ch) {
        case ' ':
          sn=curconfdir;
          en=curconfdir;
          break;
        case 'Q':
          return;
      }
    }
  } else
    oc=-1;


  nl();
  ansic_x(1);
  pla(get_string(6),&abort);
  nl();
  i=sn;
  while ((i<=en) && (uconfdir[i].confnum!=-1) && (!abort)) {
    if ((uconfdir[1].confnum!=-1) && (okconf(&thisuser))) {
      setuconf(CONF_DIRS, i, -1);
      sprintf(s,"1%s %c0:2 %s",get_string(1021),
        dirconfs[uconfdir[i].confnum].designator,
        stripcolors(dirconfs[uconfdir[i].confnum].name));
      if (E_C) {
        pla(s,&abort);
      } else {
        pla(stripcolors(s),&abort);
      }
    }
    i1=0;
    while ((i1<num_dirs) && (udir[i1].subnum!=-1) && (!abort)) {
      sprintf(s,"  5%4.4s2 - 9%s",udir[i1].keys,
        stripcolors(directories[udir[i1].subnum].name));
      if (E_C) {
        pla(s,&abort);
      } else {
        pla(stripcolors(s),&abort);
      }
      i1++;
    }
    nl();
    i++;
    if (!okconf(&thisuser))
      break;
  }
  if (i==0) {
    ansic(6);
    pla(get_string(5),&abort);
    nl();
  }
  setuconf(CONF_DIRS, oc, os);
}

/****************************************************************************/

/*
 * Allows editing of ASCII textfiles. Must have an external editor defined,
 * and toggled for use in defaults.
 */

void text_edit(void)
{
  char s[81],s1[81];

  nl();
  prt(2,get_string(44));
  mpl(12);
  input(s,12);
  if (strstr(s,".LOG")!=NULL)
    s[0]=0;
  if (!okfn(s))
    s[0]=0;
  if (s[0]) {
    sprintf(s1,get_stringx(1,1),s);
    sysoplog(s1);
    if (okfsed())
      external_edit(s,syscfg.gfilesdir,thisuser.defed-1,500,syscfg.gfilesdir,s,1);
    else {
      sprintf(s1,"%s%s",syscfg.gfilesdir, s);
      tedit(s1);
    }
  }
}

/****************************************************************************/
