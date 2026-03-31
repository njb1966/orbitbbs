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

#include "ripint.h"
#include <ctype.h>



/*
 * orbit_menu -- display the OrbitBBS main menu.
 *
 * Looks for MAINMENU.ANS (ANSI users) or MAINMENU.MSG (plain) in gfilesdir.
 * Drop either file into C:\ORBIT\GFILES\ and it is used automatically;
 * no recompile needed.  Falls back to the hardcoded menu if neither exists.
 */
#pragma option -O-
static void orbit_menu(void)
{
  int color = okansi();

  if (existprint("MAINMENU"))
    return;

  /* Fallback: hardcoded menu (active until MAINMENU.ANS is installed) */
  nl();
  if (color) outstr("\x1b[1;36m");
  pl("==============================================================================");
  if (color) outstr("\x1b[0m");
  if (color) outstr("\x1b[1;33m");
  pl("  OrbitBBS  --  Main Menu");
  if (color) outstr("\x1b[0m");
  if (color) outstr("\x1b[1;36m");
  pl("==============================================================================");
  if (color) outstr("\x1b[0m");
  nl();

  if (color) outstr("\x1b[1;32m");
  pl("  MESSAGES");
  if (color) outstr("\x1b[0m");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("P");
  if (color) outstr("\x1b[0m");
  outstr("  Post a message          ");
  if (color) outstr("\x1b[1;33m");
  outstr("Q");
  if (color) outstr("\x1b[0m");
  pl("  Quick scan (this sub)");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("S");
  if (color) outstr("\x1b[0m");
  outstr("  Scan messages           ");
  if (color) outstr("\x1b[1;33m");
  outstr("N");
  if (color) outstr("\x1b[0m");
  pl("  New scan (all subs)");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("Z");
  if (color) outstr("\x1b[0m");
  outstr("  Express scan            ");
  if (color) outstr("\x1b[1;33m");
  outstr("R");
  if (color) outstr("\x1b[0m");
  pl("  Remove your post");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("E");
  if (color) outstr("\x1b[0m");
  outstr("  Send email              ");
  if (color) outstr("\x1b[1;33m");
  outstr("M");
  if (color) outstr("\x1b[0m");
  pl("  Read your mail");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("K");
  if (color) outstr("\x1b[0m");
  outstr("  Delete old email        ");
  if (color) outstr("\x1b[1;33m");
  outstr("F");
  if (color) outstr("\x1b[0m");
  pl("  Feedback to sysop");
  nl();

  if (color) outstr("\x1b[1;32m");
  pl("  COMMUNITY & ENTERTAINMENT");
  if (color) outstr("\x1b[0m");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("G");
  if (color) outstr("\x1b[0m");
  outstr("  Feed reader             ");
  if (color) outstr("\x1b[1;33m");
  outstr(".");
  if (color) outstr("\x1b[0m");
  pl("  Doors & games");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("U");
  if (color) outstr("\x1b[0m");
  outstr("  User list               ");
  if (color) outstr("\x1b[1;33m");
  outstr("V");
  if (color) outstr("\x1b[0m");
  pl("  Vote");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("B");
  if (color) outstr("\x1b[0m");
  outstr("  BBS list                ");
  if (color) outstr("\x1b[1;33m");
  outstr("$");
  if (color) outstr("\x1b[0m");
  pl("  Time bank");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("*");
  if (color) outstr("\x1b[0m");
  pl("  Sub-board list");
  nl();

  if (color) outstr("\x1b[1;32m");
  pl("  NAVIGATION");
  if (color) outstr("\x1b[0m");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("] or +");
  if (color) outstr("\x1b[0m");
  outstr("  Next sub             ");
  if (color) outstr("\x1b[1;33m");
  outstr("[ or -");
  if (color) outstr("\x1b[0m");
  pl("  Prev sub");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("}");
  if (color) outstr("\x1b[0m");
  outstr("  Next conf                 ");
  if (color) outstr("\x1b[1;33m");
  outstr("{");
  if (color) outstr("\x1b[0m");
  pl("  Prev conf");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("H");
  if (color) outstr("\x1b[0m");
  outstr("  Hop to sub                ");
  if (color) outstr("\x1b[1;33m");
  outstr("J");
  if (color) outstr("\x1b[0m");
  pl("  Jump to conference");
  nl();

  if (color) outstr("\x1b[1;32m");
  pl("  SETTINGS & INFO");
  if (color) outstr("\x1b[0m");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("D");
  if (color) outstr("\x1b[0m");
  outstr("  Defaults                ");
  if (color) outstr("\x1b[1;33m");
  outstr("Y");
  if (color) outstr("\x1b[0m");
  pl("  Your info");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("I");
  if (color) outstr("\x1b[0m");
  outstr("  System info             ");
  if (color) outstr("\x1b[1;33m");
  outstr("L");
  if (color) outstr("\x1b[0m");
  pl("  Last callers");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("X");
  if (color) outstr("\x1b[0m");
  pl("  Toggle expert mode");
  nl();

  if (color) outstr("\x1b[1;32m");
  pl("  OTHER");
  if (color) outstr("\x1b[0m");
  outstr("  ");
  if (color) outstr("\x1b[1;33m");
  outstr("O");
  if (color) outstr("\x1b[0m");
  outstr("  Logoff                  ");
  if (color) outstr("\x1b[1;33m");
  outstr("/O");
  if (color) outstr("\x1b[0m");
  pl("  Instant logoff");
  nl();

  if (color) outstr("\x1b[1;36m");
  pl("==============================================================================");
  if (color) outstr("\x1b[0m");
  nl();
}

#pragma option -O
void mainmenu(void)
{
  char *s, s1[81], s2[81], ch;
  int i, useconf, ac,pos;
  static int mflag = 0, no_clear = 0;
  long oldstat;
  FILE *namefile;

  s2[0]=0;
  if (usub[cursub].subnum==-1) {
    cursub=0;
    if (usub[cursub].subnum==-1) {
      strcpy(s2,get_string(16));
    }
  }

  write_inst(INST_LOC_MAIN,usub[cursub].subnum,INST_FLAGS_NONE);

  tleft(1);
  if ((sysstatus_expert & thisuser.sysstatus)==0) {
    if (menu_on() && no_clear == 0 && cleared != HYPCLEAR) {

      if (cleared == NEEDCLEAR)
        rip_cls();

      if (autox)
        rmenu = 300;

      if (rmenu > 0) {
        nln(4);
        /* Compensate for menu scrolling; varies on some systems */
        sprintf(s1,"\x1b[%dA\r", 8 + sysinfo.rip_rm_adjust);
        outstr(s1);
#ifdef RIPDRIVE
        if (rd_on()) {     /* For RIPdrive local */
          if (sysinfo.rip_rm_adjust > 1)
            sprintf(s1,"\x1b[%dB\r", sysinfo.rip_rm_adjust-2);
          else if (sysinfo.rip_rm_adjust < 1)
            sprintf(s1,"\x1b[%dA\r", 2-sysinfo.rip_rm_adjust);
          rd_str(s1);
        }
#endif
      }
      printmenu(rmenu);
      topscreen();
      if (cleared == NEEDCLEAR || cleared == WASCLEARED) {
        cleared = 0;
        outstr("\x1b[20H");        /* Line 20 at start */
      } else if (rmenu > 0)
        mflag = -1;

      if (rip_on() && autox && (!rip_subset)) {
        sprintf(s1, "%sAUTOEXEC.RIP", sysinfo.ripdir);   /* Check for AUTOEXEC.RIP */
        if (exist(s1))                           /* locally */
          rip_show(s1);
        comstr("|1F000000AUTOEXEC.RIP|#\r ");   /* and on user's machine */
        if (delaykey(1) == '1')
          comstr("|1R00000000AUTOEXEC.RIP\r "); /* and play if exists */
      }
      autox = 0;
      rmenu = 0;
    } else
      orbit_menu();
  }

  nln(2);
  tleft(1);

  useconf=((uconfsub[1].confnum!=-1) && okconf(&thisuser));

  if (E_C) {
    npr("1T 0- 1%s\r\n",ctim(nsl()));
    sprintf(s1,"7[1%s %c7] [1%s7]", get_string(1627),
            subconfs[uconfsub[curconfsub].confnum].designator,
      stripcolors(subconfs[uconfsub[curconfsub].confnum].name));
    if (s2[0]==0)
      sprintf(s2,"7[1%s%s7] [1%s7]0 : 2",
        useconf?get_string(1398):"",
        usub[cursub].keys,
        stripcolors(subboards[usub[cursub].subnum].name));
  } else {
    npr("T - %s\r\n",ctim(nsl()));
    sprintf(s1,"[%s %c] [%s]", get_string(1627),
      subconfs[uconfsub[curconfsub].confnum].designator,
      stripcolors(subconfs[uconfsub[curconfsub].confnum].name));
    if (s2[0]==0)
      sprintf(s2,"[%s%s] [%s] :",
        useconf?get_string(1398):"",
        usub[cursub].keys,
        stripcolors(subboards[usub[cursub].subnum].name));
  }

  if (useconf) {
    ansic(2);
    pl(s1);
  }
#ifdef RIPDRIVE
  if (rd_on())
    rd_str("\x1b[0J");       /* Clear to EOS */
#endif
  if (menu_on())
    outstr("\x1b[0J");
  prt(2,s2);
  helpl=1;
  s=mmkey(0);
  helpl=0;
  if (s[0])
    for (i=0; (i<num_subs) && (usub[i].subnum!=-1); i++)
      if (strcmp(usub[i].keys,s)==0)
        cursub=i;
  if (strlen(s)>2) {
    sprintf(s1,"//%s",s);
    sysopchar(s1);
  } else
    sysopchar(s);

  no_clear = 0;
  if (menu_on() && cleared != HYPCLEAR) {
    comstr("!|1K\r ");     /* Disable menus */
    if (mflag) {
      rip_pcb();
      mflag = 0;
    }
    if (rip_pause)
      printmenu(334);
    if ( (strcmp(s,"/C") == 0) || (s[1] == 0 && (strchr("PEFJ", s[0])
	 != NULL)) ) {
      printmenu(321);
      cleared = WASCLEARED;
    } else if (isalpha(s[0]) || (strchr("$", s[0]) != NULL && s[1] == 0))
      printmenu(320);       /* Draw w/Quit & Help unless menu pulled down */
    else if (s[0] != '/' && s[0] != '.')
      no_clear = -1;
    if ((strchr("EFGMNPQS",s[0]) != NULL) || (strcmp(s,"/A") == 0) ||
      strcmp(s,"/E") == 0) {
      rmenu = 300;
      cleared = NEEDCLEAR;
    }
  }

/**************************************************/

  if (so()) {
    if ((strcmp(s,"ALLOWEDIT")==0) || (strcmp(s,"AE")==0))
      edit_database();
    if ((strcmp(s,"READINI")==0) || (strcmp(s,"RI")==0)) {
      if (!read_ini_info()) {
        printf(get_string(1504));
        end_bbs(noklevel);
      }
      if (!chains_reg)
        sysinfo.flags &= ~OP_FLAGS_CHAIN_REG;
      topscreen();
    }
    if ((strcmp(s,"INSTEDIT")==0) || (strcmp(s,"IE")==0))
      instance_edit();
    if ((strcmp(s,"CONFEDIT")==0) || (strcmp(s,"JE")==0)) {
      write_inst(INST_LOC_CONFEDIT,0,INST_FLAGS_NONE);
      edit_confs();
    }
    if ((strcmp(s,"BOARDEDIT")==0) || (strcmp(s,"BE")==0)) {
      write_inst(INST_LOC_BOARDEDIT,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,2));
      boardedit();
    }
    if ((strcmp(s,"CHAINEDIT")==0) || (strcmp(s,"CE")==0)) {
      write_inst(INST_LOC_CHAINEDIT,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,4));
      chainedit();
    }
    if (strcmp(s, "CHAT")==0) {
      nl();
      pl(((*(char far *)0x00000417L ^= 0x10) & 0x10) ?
                       get_string(21) :
                       get_string(22));
      sysoplog(get_stringx(1,9));
      topscreen();
    }
    if ((strcmp(s,"CHUSER")==0) || (strcmp(s,"CU")==0)) {
      write_inst(INST_LOC_CHUSER,0,INST_FLAGS_NONE);
      chuser();
    }
    if (strcmp(s,"CLOUT")==0) {
      npr("\r\nNot Yet.\r\n");
    /*  force_callout(2); */
    }
    if (strncmp(s,"DEBUG",5)==0) {
      if((s[5]) && (s[5]!=' '))
        debuglevel=s[5]-'0';
      else if(s[6])
        debuglevel=s[6]-'0';
    }
    /* DIREDIT removed in OrbitBBS */
    if (strcmp(s,"DOS")==0) {
      if (checkpw()) {
        write_inst(INST_LOC_DOS,0,INST_FLAGS_NONE);
        sysoplog(get_stringx(1,6));
#ifdef RIPDRIVE
        if (rd_on())
          localrip_deactivate();
#endif
        extern_prog(getenv("COMSPEC"), EFLAG_SHRINK|EFLAG_COMIO|EFLAG_ABORT);
#ifdef RIPDRIVE
        if (rd_on())
          localrip_deactivate();
#endif
        topscreen();
      }
    }
    if ((strcmp(s,"EDIT")==0)) {
      if (checkpw()) {
        write_inst(INST_LOC_TEDIT,0,INST_FLAGS_NONE);
        nl();
        prt(2,get_string(44));
        input(s1,50);
        if (s1[0]) {
          if ((okansi()) && (thisuser.defed))
            external_edit(s1,"",thisuser.defed-1,500,".",s1,1);
          else
            tedit(s1);
        }
      }
    }
    if ((strcmp(s,"GFILEEDIT")==0) || (strcmp(s,"GE")==0)) {
      write_inst(INST_LOC_GFILEEDIT,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,5));
      gfileedit();
    }
    if (strcmp(s,"LOAD")==0) {
      nl();
      prt(2,get_string(44));
      input(s1,50);
      if (s1[0]) {
        nl();
        prt(5,get_string(17));
        if (yn()) {
          nl();
          load_workspace(s1,0);
        } else {
          nl();
          load_workspace(s1,1);
        }
      }
    }
    if (strcmp(s,"MAILR")==0) {
      if (checkpw()) {
        write_inst(INST_LOC_MAILR,0,INST_FLAGS_NONE);
        sysoplog(get_stringx(1,8));
        mailr();
      }
    }
    if ((strcmp(s,"REBOOT")==0) && (checkpw())) {
      write_inst(INST_LOC_REBOOT,0,INST_FLAGS_NONE);
      dtr(0);
      sysoplog(get_stringx(1,7));
      logoff();
      sl1(1,"");
      if (ok_modem_stuff)
        closeport();
      close_strfiles();
      Wait(3.0);
#ifdef __OS2__
      sprintf(s2, "/IBD:%c", syscfg.ramdrive);
      execlp("setboot.exe", "setboot.exe", s2, NULL);
#else
      setvect(0xff,(void interrupt (*) ()) MK_FP(0xffff,0x0000));
      geninterrupt(0xff);
#endif
    }
    if (strcmp(s,"RELOAD")==0) {
      write_inst(INST_LOC_RELOAD,0,INST_FLAGS_NONE);
      read_new_stuff();
    }
    if (strcmp(s,"RESETF")==0) {
      write_inst(INST_LOC_RESETF,0,INST_FLAGS_NONE);
      reset_files();
    }
    if (strcmp(s, "RESETQSCAN")==0) {
      prt(5,get_string(970));
      if (yn()) {
        write_inst(INST_LOC_RESETQSCAN,0,INST_FLAGS_NONE);
        for (i=0; i<=number_userrecs(); i++) {
          read_qscn(i, qsc, 1);
          memset(qsc_p,
                 0,
                 syscfg.qscn_len - 4*(1+((max_dirs+31)/32)+((max_subs+31)/32)));
          write_qscn(i, qsc, 1);
        }
        read_qscn(usernum, qsc, 0);
        close_qscn();
      }
    }
    if (strcmp(s,"STAT")==0) {
      read_status();
      nl();
#ifndef __OS2__
      outstr(get_string(18));
      pln(_stklen);
#endif
      outstr(get_string(19));
      npr("%dk\r\n", (int) (bbscoreleft()/1024));
      outstr(get_string(20));
      npr("%d%%\r\n", cachestat());
      outstr(get_string(938));
      npr("%lu\r\n",status.qscanptr);
      nl();
    }
    if (strcmp(s,"PACK")==0) {
      nl();
      prt(5, get_string(1505));
      if (yn())
        pack_all_subs();
      else
        pack_sub(usub[cursub].subnum);
    }
  }
/**************************************************/

  if (cs()) {
    if (strcmp(s,"IVOTES")==0) {
      write_inst(INST_LOC_VOTE,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,11));
      ivotes();
    }
    if (strcmp(s,"LOG")==0) {
      slname(date(), s1);
      print_local_file(s1,"");
    }
    if (strcmp(s,"NLOG")==0) {
      print_local_file("NET.LOG","");
    }
    if (strcmp(s,"PENDING")==0) {
      print_pending_list();
    }
    if (strcmp(s,"STATUS")==0) {
      prstatus();
    }
    if (strcmp(s,"TEDIT")==0) {
      write_inst(INST_LOC_TEDIT,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,12));
      text_edit();
    }
    if ((strcmp(s,"UEDIT")==0) || (strcmp(s,"UE")==0)) {
      write_inst(INST_LOC_UEDIT,0,INST_FLAGS_NONE);
      sysoplog(get_stringx(1,10));
      uedit(usernum,0);
    }
    if (strcmp(s,"VOTEPRINT")==0) {
      write_inst(INST_LOC_VOTEPRINT,0,INST_FLAGS_NONE);
      voteprint();
    }
    if (strcmp(s,"YLOG")==0) {
      read_status();
      print_local_file(status.log1,"");
    }
    if (strcmp(s,"ZLOG")==0) {
      zlog();
    }
    if ((strcmp(s,",")==0) && ((net_sysnum>0) || (net_num_max>1))) {
      nl();
      prt(2,get_string(23));
      ch=onek("Q012");
      switch(ch) {
        case '0': print_local_file(get_string(1027),""); break;
        case '1': print_local_file(get_string(1028),""); break;
        case '2': print_local_file(get_string(1029),""); break;
      }
    }
    if (strcmp(s,"/?")==0) {
      printmenu(5);
    }
    if (strcmp(s,"SYSOP")==0) {
      printmenu(5);
    }
  }

/*************************************************/

  /* UPLOAD slash command removed in OrbitBBS */
  /* QWK offline reader removed from OrbitBBS */
  if (strcmp(s,"CLS")==0) {
    outstr("\x1b[2J\x1b[H");
  }
  if ((strcmp(s,"NET")==0) || (strncmp(s,"NET=",4)==0))
    print_net_listing(0);
  if (strcmp(s,"QSCAN")==0) {
    nl();
    prt(5,get_string(24));
    if (yn()) {
      read_status();
      for (i=0; i<max_subs; i++)
        qsc_p[i]=status.qscanptr-1L;
      nl();
      pl(get_string(25));
      nl();
    }
  }
  if (strcmp(s,"VER")==0) {
    nl();
    npr("%s   (%s)\r\n",wwiv_version, wwiv_date);
    nl();
    pl("OrbitBBS  --  The Small-Web BBS");
    nl();
    if (menu_on())
      pausescr();
  }
  if (strcmp(s,"WHO")==0) {
    multi_instance();
  }
  if (strlen(s) == 2 && s[0] == '/' && s[1] >= '0' && s[1] <= MAXMENU)
	rmenu = 301 + s[1] - '0';

  if (strcmp(s,"/A")==0) {
    write_inst(INST_LOC_SUBS,usub[cursub].subnum,INST_FLAGS_NONE);
    express=0;
    expressabort=0;
    if ((uconfsub[1].confnum!=-1) && (okconf(&thisuser))) {
      ac=1;
      tmp_disable_conf(1);
    }
    nscan(0);
    if (ac)
      tmp_disable_conf(0);
  }
  if (strcmp(s,"/E")==0)
    slash_e();
  if (strcmp(s,"/N")==0)
    nscan(cursub);
  if (strcmp(s,"/O")==0) {
    hangup=1;
      hangup=1;
  }
  if ((strcmp(s,"/R")==0) && (sysinfo.flags & OP_FLAGS_RIP_SUPPORT)) {
    thisuser.sysstatus &= ~sysstatus_disable_rip;
    if (using_modem) {
      rip = rip_check();
      if (rip_on()) {
        if (rip_ver == 15450) {
          rip_subset = -1;
          pl(get_string(1441));        //ripstring(7));
          pausescr();
        } else {
          cleared = WASCLEARED;
          rmenu = 300;
          choosemenu();
        }
      }
    } else {
      if (localrip_detect()) {          // RIPdrive is available
        if (((sysinfo.flags & OP_FLAGS_RIPDRIVE_ON) != 0)
            && !(thisuser.sysstatus & sysstatus_disable_rip)) {
          if (rip_on()==0)
            localrip_activate(sysinfo.ripdir, sysinfo.ripdir);
          rip = -1;
          ripdrive = -1;
          rip_popup = -1;
          setmenu("RIP");
          oldstat = thisuser.sysstatus;
          thisuser.sysstatus &= ~sysstatus_expert;
          do {
            rip_cls();
            printmenu(350);
            user_menus = getkey();
          } while ((user_menus < '0' || user_menus > '6'));
          thisuser.sysstatus = oldstat;
          rip_cls();
          if (user_menus != '0') {
            sprintf(s1, "%sNAME.MNU", sysinfo.ripdir);
            if ((namefile = fopen(s1, "w")) != NULL) {
              fprintf(namefile, "!|T%s\n", syscfg.systemname);
              fclose(namefile);
            }
            autox = 0;
            rmenu = 300;
            cleared = NEEDCLEAR;
            sprintf(s2,"MN%c", user_menus);
            rip_pause = pausecheck(1, s2);
            setmenu("REM");
            topdata = 0;
            if (thisuser.sysstatus & sysstatus_expert)
              pl (get_string(1506));
          } else {
            rip=0;
#ifdef RIPDRIVE
            ripdrive = 0;
            rd_disable();
#endif
          }
        } else
          pl(get_string(1507));
      } else
        pl(get_string(1508));
    }
  }
  if (strcmp(s, "/V")==0)
    valscan();
  /* /C chat command removed in OrbitBBS */
  if (strcmp(s,"/Z")==0) {
    if (sysinfo.flags & OP_FLAGS_SLASH_SZ) {
      prt(5,get_string(935));
      if (yn()) {
        pl(get_string(26));
        set_x_only(1, "POSTS.TXT", 0);
        nscan(0);
        set_x_only(0, NULL, 0);
        add_arc("OFFLINE", "POSTS.TXT", 0);
      /* download_temp_arc removed in OrbitBBS */
      }
    }
  }
  if ((s[1]==0) && (s[0]!=0)) {
    switch(s[0]) {
      case '}':
        if (okconf(&thisuser)) {
          if ((curconfsub<subconfnum-1) && (uconfsub[curconfsub+1].confnum>=0))
            ++curconfsub;
          else
            curconfsub=0;
          setuconf(CONF_SUBS, curconfsub, -1);
        }
        break;
      case '>':
      case '+':
      case ']':
        if ((cursub<num_subs-1) && (usub[cursub+1].subnum>=0))
          ++cursub;
        else
          cursub=0;
        break;
      case '{':
        if (okconf(&thisuser)) {
          if (curconfsub>0)
            --curconfsub;
          else {
            while ((uconfsub[curconfsub+1].confnum>=0) && (curconfsub<subconfnum-1))
              ++curconfsub;
          }
          setuconf(CONF_SUBS, curconfsub, -1);
        }
        break;
      case '<':
      case '-':
      case '[':
        if (cursub>0)
          --cursub;
        else {
          while ((usub[cursub+1].subnum>=0) && (cursub<num_subs-1))
            ++cursub;
        }
        break;
      case '!':
        helpl=14;
        if (!cs())
          return;
        nln(2);
        pl(get_string(15));
        outstr(":");
        input(s1,30);
        i=finduser1(s1);
        if (i>0) {
          sysoplog(get_stringx(1,13));
          valuser(i);
        } else
          pl(get_string(8));
        break;
      case '.':
        helpl=26;
        write_inst(INST_LOC_CHAINS,0,INST_FLAGS_NONE);
        play_sdf(get_string(1030),0);
        existprint(get_string(1030));
        do_chains();
        break;
      case '$':
        write_inst(INST_LOC_BANK,0,INST_FLAGS_NONE);
        time_bank();
        break;
      /* case 'A': AutoMessage removed in OrbitBBS */
      case 'B':
        bbslist();
        break;
      /* case 'C': Chat removed in OrbitBBS */
      case 'D':
        helpl=4;
        write_inst(INST_LOC_DEFAULTS,0,INST_FLAGS_NONE);
        if (existprint(get_string(1031)))
          pausescr();
        if (menu_on()) {
          comstr(" ");
          rmenu = 300;
        }
        defaults();
        break;
      case 'E':
        send_email();
        break;
      case 'F':
        write_inst(INST_LOC_FEEDBACK,0,INST_FLAGS_NONE);
        feedback(0);
        break;
      case 'G':
        helpl=28;
        write_inst(INST_LOC_GFILES,0,INST_FLAGS_NONE);
        existprint(get_string(1032));
        gfiles();
        rmenu = 300;
        break;
      case 'H':
        hop_sub();
        break;
      case 'I':
        nl();
        npr("%s   (%s)\r\n",wwiv_version, wwiv_date);
        nl();
        existprint(get_string(1033));
        existprint(get_string(1034));
        pausescr();
        break;
      case 'J':
        if (okconf(&thisuser))
          jump_conf(CONF_SUBS);
        break;
      case 'K':
        helpl=8;
        write_inst(INST_LOC_KILLEMAIL,0,INST_FLAGS_NONE);
        kill_old_email();
        break;
      case 'L':
        if (okansi()) outstr("\x1b[1;33m");
        if ((sysinfo.flags & OP_FLAGS_SHOW_CITY_ST) &&
            (syscfg.sysconfig & sysconfig_extended_info))
          pl("#      Name                      Time  Date  City            St  Speed   #T");
        else
          pl("#      Name                      Lang       Time  Date  Speed                #T");
        if (okansi()) outstr("\x1b[0m");
        i=okansi()?205:'=';
        npr(charstr(79,i));
#ifdef RIPDRIVE
        rd_coff();
#endif
        printfile("LASTON.TXT");
#ifdef RIPDRIVE
        rd_con();
#endif
        break;
      case 'M':
        readmail();
        break;
      case 'N':
        write_inst(INST_LOC_SUBS,65535,INST_FLAGS_NONE);
        express=0;
        expressabort=0;
        nscan(0);
        break;
      case 'O':
        nln(2);
        prt(5,get_string(28));
        helpl=12;
        if (yn()) {
          write_inst(INST_LOC_LOGOFF,0,INST_FLAGS_NONE);
          outchr(12);
          if (rip_on())
            random_screen("goodbye");
          outstr(get_string(29));
          pl(ctim(timer()-timeon));
#ifdef RIPDRIVE
          rd_coff();
#endif
          existprint(get_string(1036));
          pausescr();
          hangup=1;
        }
        break;
      case 'P':
        irt[0]=0;
        irt_name[0]=0;
        grab_quotes(NULL, NULL);
        if (usub[0].subnum!=-1)
          post();
        break;
      case 'Q':
        if (usub[0].subnum!=-1) {
          write_inst(INST_LOC_SUBS,usub[cursub].subnum,INST_FLAGS_NONE);
          i=0;
          express=0;
          expressabort=0;
          qscan(cursub,&i);
        }
        break;
      case 'R':
        if (usub[0].subnum!=-1) {
          write_inst(INST_LOC_SUBS,usub[cursub].subnum,INST_FLAGS_NONE);
          helpl=15;
          remove_post();
        }
        break;
      case 'S':
        if (usub[0].subnum!=-1) {
          write_inst(INST_LOC_SUBS,usub[cursub].subnum,INST_FLAGS_NONE);
          express=0;
          expressabort=0;
          scan2();
        }
        break;
      /* case 'T': File Transfer removed in OrbitBBS */
      case 'U':
      case '~':
#ifdef RIPDRIVE
        rd_coff();
#endif
        list_users(0);
#ifdef RIPDRIVE
        rd_con();
#endif
        break;
      case 'V':
        write_inst(INST_LOC_VOTE,0,INST_FLAGS_NONE);
        helpl=18;
        vote();
        break;
      case 'W':
        printmenu(15);
        break;
      case 'X':
        thisuser.sysstatus ^= sysstatus_expert;
        if (rip_on()) {
          comstr("!|*\r ");        /* Clear menu bar */
          cleared = WASCLEARED;
          if ((thisuser.sysstatus & sysstatus_expert) == 0)
            rmenu = 300;                  /* Menu bar stuff */
        }
        break;
      case 'Y':
        if (rip_on() && (rip_subset == 0)) {
          pl(get_string(1439));      //ripstring(5));
          if (ny()) {
            set_name();
            pausescr();
          }
        }
        yourinfo();
        pausescr();
        break;
      case 'Z':
        express=1;
        expressabort=0;
        tmp_disable_pause(1);
        nscan(0);
        express=0;
        expressabort=0;
        tmp_disable_pause(0);
        break;
      case '?':
        orbit_menu();
        break;
      case '#':
        nl();
        pl(get_string(939));
        nl();
      case '*':
#ifdef RIPDRIVE
        rd_coff();
#endif
        sublist();
#ifdef RIPDRIVE
        rd_con();
#endif
        break;
    }
  }
  if (rip_pause && menu_on())
    if ((s[1]==0) && (strchr("~!*YBILUW#", s[0]) != NULL))
      pausescr();
    else if ((strcmp(s, "VER") == 0) || (strcmp(s, "/?") == 0))
      pausescr();
  if (cleared == HYPCLEAR) {
    rmenu = 300;
    cleared = NEEDCLEAR;
  }
  helpl=0;
}

/****************************************************************************/

void dlmainmenu(void)
{
  /* File Transfer section removed in OrbitBBS */
  nl();
  pl("File transfers are not available.");
  nl();
}

