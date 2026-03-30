/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/

#pragma hdrstop

#define _DEFINE_GLOBALS_
#include "vars.h"
#include "subxtr.h"
#include <math.h>

#ifdef __OS2__
#include <process.h>
#else
unsigned _stklen=15360;
#endif

int unx;

extern char cid_num[], cid_name[];

/****************************************************************************/

void getcaller(void)
{
  char s[81],ch,lokb,*ss;
  int any,fast=0,i;
  double d;
  userrec tu;
  c_sub=c_dir=0;

  cid_num[0]=0;
  cid_name[0]=0;
  frequent_init();
  sl1(1,"");
  imodem(0);
  usernum=0;
  wfc=0;
  write_inst(INST_LOC_WFC,0,INST_FLAGS_NONE);
  read_user(1,&thisuser);
  read_qscn(1,qsc,0);
  usernum=1;
  reset_act_sl();
  fwaiting=thisuser.waiting;
  if (thisuser.inact & inact_deleted) {
    thisuser.screenchars=80;
    thisuser.screenlines=25;
  }
  screenlinest=defscreenbottom+1;
  d=(1.0+timer()) / 102.723;
  d-=floor(d);
  d*=10000.0;
  srand((unsigned int)d);
  do {
    write_inst(INST_LOC_WFC,0,INST_FLAGS_NONE);
    set_net_num(0);
    any=0;
    wfc=1;
    if (date_changed())
      if (date_changed()) {
        printf("\n\nClock Corrupted.\n\n");
        printf("Should put BBS in a batch file like:\n\n");
        printf("copy con: orbit.bat\n");
        printf(":top\n");
        printf("setclock\n");
        printf("bbs\n");
        printf("if errorlevel 1 goto top\n");
        printf("^Z\n");
        end_bbs(noklevel);
      }
    if ((strcmp(date(), status.date1)!=0)) {
      if (instance==1) {
        holdphone(1);
        beginday();
        holdphone(0);
        clrscrb();
      }
    }
    check_event();
    if (do_event) {
      run_event();
      any=1;
    }
    lokb=0;
    strcpy(curspeed,get_string(1042));
    if ((!any) && ((rand() % 8000)==0) && (net_sysnum) &&
        (ok_modem_stuff) && (sysinfo.flags & OP_FLAGS_NET_CALLOUT) &&
        ((instance==1) || (status.net_version>=34)) &&
        ((syscfgovr.com_ISR[syscfgovr.primaryport]<8) || (status.net_version>=35))) {
      attempt_callout();
      any=1;
    }
    if (kbhitb()) {
      wfc=0;
      read_user(1,&thisuser);
      read_qscn(1,qsc,0);
      fwaiting=thisuser.waiting;
      wfc=1;
    }
    okskey=0;
    ch=upcase(inkey());
    if (ch) {
      wfc=2;
      any=1;
      switch(ch) {
        case '=':
          if (ok_local()) {
            holdphone(1);
            reset_files();
            holdphone(0);
          }
          break;
        case '?':
          if (ok_local()) {
            i=7;
            do {
              clrscrb();
              nl();
              printmenu(i);
              ch=upcase(getkey());
              if ((ch!=' ') && (ch!=27))
                if (i==7)
                  i=19;
                else
                  i=7;
            } while ((ch!=' ') && (ch!=27));
          }
          break;
        case ' ':
          outs(get_string(41));
          d=timer();
          while ((!kbhitb()) && (fabs(timer()-d)<60.0));
          if (kbhitb()) {
            ch=upcase(getchd1());
            if (ch==str_yes[0]) {
              outs(str_yes);
              outstr("\r\n");
              lokb=1;
              if ((syscfg.sysconfig & sysconfig_off_hook)==0)
                dtr(0);
            } else {
              if (!ok_local())
                break;
              ss=get_string(42);
              if (ch==upcase(*ss)) {
                unx=1;
                fast=1;
              } else {
                switch (ch) {
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                    fast=1;
                    unx=ch-48;
                    break;
                }
              }
              read_status();
              if ((!fast) || (unx>status.users))
                break;
              read_user(unx,&tu);
              if ((tu.sl!=255) || (tu.inact & inact_deleted))
                break;
              usernum=unx;
              i=wfc; wfc=0;
              read_user(usernum,&thisuser);
              read_qscn(usernum,qsc,0);
              wfc=i;
              outchr(ch);
              outs("\r\n\r\n\r\n\r\n\r\n\r\n");
              lokb=2;
              if ((syscfg.sysconfig & sysconfig_off_hook)==0)
                dtr(0);
              reset_act_sl();
              changedsl();
              if (set_language(thisuser.language)) {
                thisuser.language=0;
                set_language(thisuser.language);
              }
              break;
            }
            if (ch==0)
              getchd1();
          }
          if (!lokb)
            out1ch(12);
          break;
        case 'A':
          if (!ok_modem_stuff)
            break;
          answer_phone();
          break;
        case 'B':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_BOARDEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            boardedit();
            cleanup_net();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'C':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_CHAINEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            chainedit();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'D':
          okskey=1;
          if (ok_local()) {
            /* DIREDIT removed in OrbitBBS */
          }
          okskey=0;
          break;
        case 'W':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_TEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            text_edit();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'F':
          if (ok_local()) {
            write_inst(INST_LOC_DOS,0,INST_FLAGS_NONE);
            holdphone(1);
            nl();
            pl(get_string(43));
            nl();
            extern_prog(getenv("COMSPEC"), EFLAG_SHRINK);
            out1ch(12);
            cleanup_net();
            holdphone(0);
          }
          break;
        case 'G':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_GFILEEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            gfileedit();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'I':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_VOTEEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            ivotes();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'J':
          okskey=1;
          if (ok_local()) {
            holdphone(1);
            edit_confs();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'K':
          if (ok_local()) {
            usernum=1;
            holdphone(1);
            okskey=1;
            prt(2,get_string(44));
            input(s,50);
            load_workspace(s,0);
            send_email();
            okskey=0;
            write_user(1,&thisuser);
            cleanup_net();
            holdphone(0);
          }
          break;
        case 'L':
          if (ok_local()) {
            read_status();
            slname(date(), s);
            print_local_file(s,status.log1);
          }
          break;
        case 'M':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_MAILR,0,INST_FLAGS_NONE);
            holdphone(1);
            mailr();
            holdphone(0);
          }
          okskey=0;
          break;
        case 'N':
          if (ok_local())
            print_local_file("NET.LOG","NETDAT*.LOG");
          break;
        case 'P':
          if (ok_local())
            print_pending_list();
          break;
        case 'Q':
          end_bbs(QUIT_LEVEL);
          break;
        case 27:
          outstr(get_string(1043));
          if (yn())
            end_bbs(QUIT_LEVEL);
          clrscrb();
          break;
        case 'R':
          if (ok_local()) {
            usernum=1;
            holdphone(1);
            okskey=1;
            readmail();
            okskey=0;
            write_user(1,&thisuser);
            cleanup_net();
            holdphone(0);
          }
          break;
        case 'S':
          if (ok_local()) {
            prstatus();
            getkey();
          }
          break;
        case 'T':
          if ((ok_local()) && (syscfg.terminal[0])) {
            write_inst(INST_LOC_TERM,0,INST_FLAGS_NONE);
            if (syscfg.sysconfig & sysconfig_shrink_term)
              extern_prog(syscfg.terminal, EFLAG_SHRINK);
            else
              extern_prog(syscfg.terminal, 0);
            imodem(1);
            imodem(0);
          }
          break;
        case 'U':
          okskey=1;
          if (ok_local()) {
            write_inst(INST_LOC_UEDIT,0,INST_FLAGS_NONE);
            holdphone(1);
            uedit(1,0);
            holdphone(0);
          }
          okskey=0;
          break;
        case 'E':
          if (ok_local()) {
            usernum=1;
            holdphone(1);
            okskey=1;
            send_email();
            okskey=0;
            write_user(1,&thisuser);
            cleanup_net();
            holdphone(0);
          }
          break;
        case 'X':
          for(i=0;i<50;i++) {
            if(xenviron[i]!=NULL)
              npr("\r\nxenv %d=%s",i,xenviron[i]);
          }
#ifndef __OS2__
          npr("\r\nreal=%x, new=%x",INT_REAL_DOS,save_dos);
#endif
          nl();
          pausescr();
        break;
        case 'Y':
          if (ok_local()) {
            read_status();
            slname(date(), s);
            print_local_file(status.log1,s);
          }
          break;
        case 'Z':
          if (ok_local()) {
            zlog();
            nl();
            getkey();
          }
          break;
        case '/':
          if ((net_sysnum) && (ok_local()) && ok_modem_stuff &&
              ((instance==1) || (status.net_version>=34)) &&
               ((syscfgovr.com_ISR[syscfgovr.primaryport]<8) || (status.net_version>=35)))
            force_callout(0);
          break;
        case '.':
          if ((net_sysnum) && (ok_local()) && ok_modem_stuff &&
              ((instance==1) || (status.net_version>=34)) &&
               ((syscfgovr.com_ISR[syscfgovr.primaryport]<8) || (status.net_version>=35)))
            force_callout(1);
          break;
        case ',':
          if ((net_sysnum>0) || (net_num_max>1) && ok_local()) {
            nl();
            prt(2,get_string(23));
            ch=onek("Q012");
            switch (ch) {
              case '0':
                print_local_file(get_string(1027),"");
                break;
              case '1':
                print_local_file(get_string(1028),"");
                break;
              case '2':
                print_local_file(get_string(1029),"");
                break;
            }
          }
          break;
        case '`':
          if ((net_sysnum) && (ok_local())) {
            holdphone(1);
            print_net_listing(1);
            holdphone(0);
          }
          break;
        case 9:
          if (ok_local()) {
            holdphone(1);
            instance_edit();
            holdphone(0);
          }
          break;
      }
      if (!incom && !lokb) {
        frequent_init();
        read_user(1,&thisuser);
        read_qscn(1,qsc,0);
        fwaiting=thisuser.waiting;
        reset_act_sl();
        usernum=1;
      }
      okskey=0;
      catsl();
      write_inst(INST_LOC_WFC, 0, INST_FLAGS_NONE);
    }
    if ((comhit()) && (ok_modem_stuff) && (!lokb)) {
      any=1;
      if (peek1c()==10)
        get1c();
      else {
        outs("* ");
        if (mode_switch(1.0,0)==mode_ring)
          answer_phone();
        else if (modem_mode == mode_con) {
          incom=outcom=1;
          if (!(modem_flag & flag_ec))
            wait1(45);
          else
            wait1(2);
        }
      }
    }
    if (!any) {
      if (c_sub<num_subs) {
        if (!sub_dates[c_sub]) {
          any=1;
          iscan1(c_sub,1);
        }
        c_sub++;
      } else if (c_dir<num_dirs) {
        if (!dir_dates[c_dir]) {
          any=1;
          dliscan_hash(c_dir);
        }
        c_dir++;
      } else {
        if (labs(timer1()-mult_time) > 1000L) {
          cleanup_net();
          mult_time=timer1();;
          giveup_timeslice();
        } else {
          giveup_timeslice();
        }
      }
    }
  } while ((!incom) && (!lokb) && (!endday));
  if (lokb) {
    if (ok_modem_stuff)
      modem_speed = modem_i->defl.modem_speed;
    else
      modem_speed = 14400;
  }
  using_modem=incom;
  if (lokb==2)
    using_modem=-1;
  okskey=1;
  if (!endday) {
    clrscrb();
    if (modem_mode==mode_fax)
      outs(get_string(45));
    else
      outs(get_string(46));
    outs(curspeed);
    outs("...\r\n");
  }
  wfc=0;
}

/****************************************************************************/

void main(int argc, char *argv[])
{
  char s[81],ch,frc[81],s1[81],*ss;
  int i,i1,num_min=0;
  unsigned int ui=0, us=0;
  double dt;
  unsigned short c_s,c_o;
  int this_usernum=0;

  ss=getenv("BBS");
  if (ss) {
    if (strncmp(ss,"ORBIT",5)==0) {
      printf("You are already in the BBS, type 'EXIT' instead.\n\n");
      exit(-1);
    }
  }
  ss=getenv("ORBIT_DIR");
  if (ss) {
    cd_to(ss);
  }
  ss=getenv("ORBIT_INSTANCE");
  if(ss) {
    instance=atoi(ss);
    if ((instance<=0) || (instance>999)) {
      printf("ORBIT_INSTANCE can only be 1..999\n");
      exit(-1);
    }
  } else {
    instance=1;
  }
  already_on=0;
  endday=0;
  oklevel=0;
  noklevel=0;
  ooneuser=0;
  no_hangup=0;
  ok_modem_stuff=1;
  debuglevel=0;
  oklevel=OK_LEVEL;
  noklevel=NOK_LEVEL;
#ifdef NOT_ANYMORE
  if (instance > 1)
    sprintf(rf,"RESTORE.%3.3d",instance);
  else
    sprintf(rf,"RESTORE.WWV");
  if (exist(rf))
    restoring_shrink=1;
  else
    restoring_shrink=0;
#else
  restoring_shrink=0;
#endif
  frc[0]=0;

  s1[0]=0;

  for (i=1; i<argc; i++) {
    strcpy(s,argv[i]);
    if ((s[0]=='-') || (s[0]=='/')) {
      ch=upcase(s[1]);
      switch(ch) {
        case 'C':
          flow_control=1;
          break;
        case 'B':
          ui=(unsigned int) atol(&(s[2]));
          if (ui==49664) {
            strcpy(curspeed, "115200");
            if (!us)
              us=ui;
            already_on=1;
          } else if ((ui%300)==0) {
            ultoa((unsigned long) ui,curspeed,10);
            if (!us)
              us=ui;
            already_on=1;
          } else {
            ui=us=0;
            if (stricmp(&(s[2]),get_string(1042))==0) {
              strcpy(curspeed,get_string(1042));
              already_on=1;
            }
          }
          rip=0;
          break;
        case 'D':
          debuglevel=atoi(&(s[2]));
          break;
        case 'F':
          strcpy(frc,s+2);
          strupr(frc);
          already_on=1;
          break;
        case 'S':
          us=(unsigned int) atol(&(s[2]));
          if ((us%300) && (us != 49664)) {
            us=ui;
          }
          break;
        case 'N':
          oklevel=atoi(&(s[2]));
          break;
        case 'A':
          noklevel=atoi(&(s[2]));
          break;
        case 'O':
          ooneuser=1;
          break;
        case 'H':
          no_hangup=1;
          break;
        case 'P':
          strcpy(s1, s+2);
          strupr(s1);
          break;
        case 'I':
          i1=atoi(&(s[2]));
          if ((i1!=instance) || (!getenv("ORBIT_INSTANCE"))) {
            printf("\r\nEnvironment variable %s and /I%d (instance) must match.\r\n",ss,i1);
            exit(noklevel);
          }
          share_installed();
          break;
        case 'M':
          ok_modem_stuff=0;
          break;
#ifndef __OS2__
        case 'X':
          c_s=strtol(&(s[2]),NULL,16);
          c_o=strtol(&(s[7]),NULL,16);
          if (c_s && c_o) {
            point_shrink=MK_FP(c_s, c_o);
          }
          break;
#endif
        case 'R':
          num_min=atoi(&(s[2]));
          break;
        case 'V':
          this_usernum=atoi(&(s[2]));
          if (!already_on)
            strcpy(curspeed,"KB");
          already_on=1;
          break;
        case '?':
          printf("OrbitBBS v1.0\n\n");
          printf("BBS /B<rate> /S<rate> /N<level> /A<level> /R<min> /O /H /M /F /C /V<num>/?\n\n");
          printf("  /B - someone already logged on at rate (modem speed)\n");
          printf("  /S - used only with /B, indicates com port speed\n");
          printf("  /N - normal exit level\n");
          printf("  /A - abnormal exit level\n");
          printf("  /R - specify max # minutes until event\n");
          printf("  /O - quit after one user done\n");
          printf("  /H - don't hang up on user when he logs off\n");
          printf("  /M - don't access modem at all\n");
          printf("  /F - pass full result code (\"CONNECT 9600/ARQ/HST/HST/V.42BIS\")\n");
          printf("  /C - enable CTS/RTS flow control\n");
          printf("  /V - pass usernumber online\n");
          printf("  /? - display command line options\n\n");
          exit(0);
      }
    }
  }

  init();

  if (s1[0])
    strcpy(syscfg.systempw,s1);

  if (syscfg.sysconfig & sysconfig_no_local) {
    this_usernum=0;
    already_on=0;
  }

  if (frc[0])
    process_full_result(frc);

  if (num_min) {
    syscfg.executetime=(timer()+num_min*60)/60;
    if (syscfg.executetime>1440)
      syscfg.executetime-=1440;
    syscfg.executestr[0]=0;
    time_event=((double)syscfg.executetime)*60.0;
    last_time=time_event-timer();
    if (last_time<0.0)
      last_time+=24.0*3600.0;
  }

#ifdef OLD_SHRINK
  if (restoring_shrink) {
    restoring_shrink=0;
    switch(restore_data(rf)) {
      case -1: /* hanging up */
        goto hanging_up;
      case 0: /* WFC */
        goto wfc_label;
      case 1: /* main menu */
      case 2:
        goto main_menu_label;
    }
  }
#endif

  do {

    if (this_usernum) {
      usernum=this_usernum;
      read_user(usernum, &thisuser);
      if ((thisuser.inact & inact_deleted)==0) {
        gotcaller(ui, us);
        usernum=this_usernum;
        read_user(usernum,&thisuser);
        read_qscn(usernum, qsc, 0);
        reset_act_sl();
        changedsl();
        checkit=0;
        okmacro=1;
        if ((!hangup) && (usernum>0) && (thisuser.restrict & restrict_logon) &&
          (strcmp(date(),thisuser.laston)==0) && (thisuser.ontoday>0)) {
          nl();
          pl(get_string(361));
          nl();
          hangup=1;
        }
      } else
        this_usernum=0;
    }
    if (!this_usernum) {
      wait1(9);
      if (already_on)
        gotcaller(ui, us);
      else
        getcaller();
    }
    if (modem_mode==mode_fax) {
      if (exist("WWIVFAX.*")) {
        stuff_in(s1, "WWIVFAX %S %P", "", "", "", "", "");
        extern_prog(s1, EFLAG_SHRINK | EFLAG_FILES);
      }
      goto hanging_up;
    }
    if (using_modem>-1) {
      if (!using_modem)
        holdphone(1);
      if (!this_usernum)
        getuser();
    } else {
      holdphone(1);
      using_modem=0;
      checkit=0;
      okmacro=1;
      usernum=unx;
      reset_act_sl();
      changedsl();
    }
    this_usernum=0;
    if (!hangup) {
      if ((strcmp(date(), status.date1)!=0))
        beginday();
      logon();

main_menu_label:

      setiia(90);
      set_net_num(0);
      while (!hangup) {
        if (filelist) {
          bbsfree(filelist);
          filelist=NULL;
        }
        zap_ed_info();
        switch(curdloads) {
          case 2:
            write_inst(INST_LOC_CHAINS,0,INST_FLAGS_NONE);
            do_chains();
          break;
          /* case 1: File Transfer removed in OrbitBBS */
          case 0:
          default:
            write_inst(INST_LOC_MAIN,usub[cursub].subnum,INST_FLAGS_NONE);
            mainmenu();
          break;
        }
      }
      logoff();
      end_rip();
    }

hanging_up:
#ifdef RIPDRIVE
  if (rd_on())  {
	rd_disable();
	strcpy(s,sysinfo.ripdir);           // Clean up files
	strcat(s,"ripclip.brd");
	unlink(s);
	strcpy(s,sysinfo.ripdir);
	strcat(s,"ripmouse.sav");
	unlink(s);
	strcpy(s,sysinfo.ripdir);
	strcat(s,"ripterm.sav");
	unlink(s);
	strcpy(s,sysinfo.ripdir);
	strcat(s,"ripterm0.sav");
	unlink(s);
  }
  ripdrive = 0;
  end_rip();
#endif

    if ((!no_hangup) && (using_modem) && ok_modem_stuff) {
      hang_it_up();
    }
    catsl();
    frequent_init();

wfc_label:

    cleanup_net();
    if (!using_modem)
      holdphone(0);
    if ((!no_hangup) && ok_modem_stuff)
      dtr(0);
    already_on=0;
    if (sysop_alert && (!kbhitb())) {
      dtr(1);
      wait1(2);
      holdphone(1);
      dt=timer();
      clrscrb();
      nl();
      pl(get_string(47));
      nl();
      while ((!kbhitb()) && (fabs(timer()-dt)<60.0)) {
        setbeep(1);
        wait1(9);
        setbeep(0);
        wait1(18);
      }
      clrscrb();
      holdphone(0);
    }
    sysop_alert=0;
  } while ((!endday) && (!ooneuser));

  outs("\f");
  end_bbs(oklevel);
}

/****************************************************************************/
