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

extern char cid_num[], cid_name[];

void random_screen(char *s1)
{
  char s[81];
  int i,ns=0;

  sprintf(s,"%s%s%s",languagedir,s1,".0");
  if (exist(s) && !rip_on()) {
    for (i=0;i<1000;i++) {
      sprintf(s,"%s%s.%d",languagedir,s1,i);
      if (exist(s))
        ns++;
      else
        break;
    }
    sprintf(s,"%s.%d",s1,random(ns));
  } else if (rip) {
    sprintf(s,"%s%s%s",languagedir,s1,".R0");
    if (exist(s)) {
      for (i=0;i<100;i++) {
        sprintf(s,"%s%s.R%d",languagedir,s1,i);
        if (exist(s))
          ns++;
        else
          break;
      }
      sprintf(s,"%s%s.R%d",languagedir, s1, random(ns));
    } else
      sprintf(s,"%s%s%s",languagedir, s1, ".RIP");
  } else
    sprintf(s,"%s%s",s1,".ANS");
  printfile(s);
}

int usa_phone_convention(userrec *u)
{
  if ((strcmp(u->country,"USA")==0) ||
      (strcmp(u->country,"CAN")==0) ||
      (strcmp(u->country,"MEX")==0))
    return(1);
  return(0);
}


void getuser(void)
{
  char s[161],s2[161],s3[161],s4[161],*ss;
  int ok,count,net_only,ans,i;
  long l;
  FILE *f;

  write_inst(INST_LOC_GETUSER, 0, INST_FLAGS_NONE);

  net_only=1;
  if (syscfg.netlowtime!=syscfg.nethightime) {
    if (syscfg.nethightime>syscfg.netlowtime) {
      if ((timer()<=(syscfg.netlowtime*60.0))
          || (timer()>=(syscfg.nethightime*60.0)))
        net_only=0;
    } else {
      if ((timer()<=(syscfg.netlowtime*60.0))
          && (timer()>=(syscfg.nethightime*60.0)))
        net_only=0;
    }
  } else
    net_only=0;
  count=0;
  ok=0;
  checkit=1;
  okmacro=0;
  curconfsub=curconfdir=0;
  actsl=syscfg.newusersl;
  if ((!net_only) && (incom)) {
#ifdef OLD
    outstr(get_string(1168));
    thisuser.sysstatus=0;
    ans=yn();
#else
    ans=check_ansi();
#endif
    ripcode = 0;
    if (sysinfo.flags & OP_FLAGS_RIP_SUPPORT)
      rip_detect();
    else
      end_rip();
    sprintf(s,"%sWELCOME.ANS",languagedir);
    if (exist(s) || rip) {
      nl();
      if (ans>0 || rip) {
        thisuser.sysstatus |= sysstatus_ansi;
        thisuser.sysstatus |= sysstatus_color;
        if (rip)
          ripcode = 1;
        random_screen("WELCOME");
        if (rip)
          ripcode = 0;
      } else if (ans==0)
        printfile("WELCOME.MSG");
    } else {
      if (ans) {
        sprintf(s,"%sWELCOME.0",languagedir);
        if (exist(s))
          random_screen("WELCOME");
        else
          printfile("WELCOME.MSG");
      } else
        printfile("WELCOME.MSG");
    }
  }
  if (curatr!=7)
    reset_colors();
  do {
      nl();
      if (net_only) {
        pl(get_string(353));
        pl(get_string(354));
      } else
        pl(get_string(355));

      outstr(get_string(356));
      if (rip) {
        if (rip_subset || (count>0)) {
          printmenu(353);
          pl("\x1b[30;47m");
        } else {
          sprintf(s,"\1|1\x1b""0000$%s_ID$^m|#\r ", sysinfo.ripbbsid);
          comstr(s);
#ifdef RIPDRIVE
          if (rd_on()) {
            sprintf(s,"%sMENU353.LCL", sysinfo.ripdir);
            rd_str("\x1b[30;47m");
            rd_print(s);
            rd_str("\x1b[30;47m\n");
          }
#endif
        }
      }

      input(s,30);
      if (rip && (rip_subset || (count>0))) {
        rip_rtwpcb();
        comstr("\x1b[37;40m");
        comnl(get_string(1435));
      }
#ifdef RIPDRIVE
      else if (rip && rd_on()) {
	rd_str("\1|1\x1b""0000$RTW$$PCB$\r");
	pl(s);
      }
#endif

	usernum=finduser(s);
        if (net_only && (usernum!=-2)) {
          if ((usernum!=-4) || (strcmp(s,"DNM")))
            usernum=0;
        }
	if (usernum>0) {
	  read_user(usernum,&thisuser);
	  read_qscn(usernum,qsc,0);
	  if (set_language(thisuser.language)) {
		thisuser.language=0;
		set_language(thisuser.language);
	  }
	  if ((thisuser.sl < 255) && (user_online(usernum, &i))) {
		nl();
		ansic(6);
		outstr(get_string(1400));
		npr("%d!",i);
		nl();
		continue;
	  }
	  ok=1;
	  actsl = syscfg.newusersl;
	  topscreen();
	  outstr(get_string(357));
          if (rip_on()) {            /* Password dialog is special */
		printmenu(355);
		if (!rip_subset) {
		  comstr("\1|1F000000PASS.RIP|#\r ");   /* Check for PASS.RIP */
		  if (getkey() == '1')
			comstr("\1|1R00000000PASS.RIP\r ");         /* and play if exists */
		}
		pl("\x1b[30;47m");
	  } 

	  echo=0;
	  input(s,8);
          if (rip_on()) {
		rip_rtwpcb();
		comstr("\x1b[37;40m");
		comnl(get_string(1436));     //ripstring(2));
	  }

	  if (strcmp(s,thisuser.pw)!=0)
		ok=0;
	  if (((syscfg.sysconfig & sysconfig_free_phone)==0)) {
		if (usa_phone_convention(&thisuser) || !thisuser.country[0]) {
		  outstr(get_string(358));
                  if (rip_on()) {
			if (rip_subset) {
			  printmenu(354);
			  pl("\x1b[30;47m");
			} else {
			  if (rip_ver >= 20000)
                            comstr("\1|1\x1b""0000$PHONE$^m|#\r ");
			  else
                            comstr("\1|1\x1b""0000$PHONE_LAST4$^m|#\r ");
#ifdef RIPDRIVE
                          if (rd_on()) {
                                sprintf(s,"%sMENU354.LCL", sysinfo.ripdir);
				rd_print(s);
				rd_str("\x1b[30;47m\n");
			  }
#endif
			}
		  } 
		  echo=0;
		  input(s2,4);
                  if (rip_on() && (rip_subset || (count>0))) {
			rip_rtwpcb();
                        comstr("\x1b[37;40m");
			comnl(get_string(1437));     //ripstring(3));
		  }
#ifdef RIPDRIVE
                  if (rip_on() && rd_on()) {
			rd_str("\x1b[37;40m");
			rd_str("\1|1\x1b""0000$RTW$$PCB$\r");
			pl(s2);
		  }
#endif

		  if (strcmp(s2,&thisuser.phone[8])!=0) {
			ok=0;
			if ((strlen(s2)==4) && (s2[3]=='-')) {
			  nl();
			  pl(get_string(359));
			  nl();
			}
		  }
		}
	  }
	  if ((thisuser.sl==255) && (incom) && (ok)) {
		outstr(get_string(616));
                if (rip_on()) {            /* Password dialog is special */
		  printmenu(356);
		  pl("\x1b[30;47m");
		} 
		echo=0;
		input(s4,20);
                if (rip_on()) {
		  rip_rtwpcb();
		  comstr("[37;40m");
		  comnl(get_string(1438));       //ripstring(4));
		}
		if (strcmp(s4,syscfg.systempw)!=0)
		  ok=0;
	  }
	  echo=1;
	  if (ok) {
		reset_act_sl();
		changedsl();
	  } else {
		++thisuser.illegal;
		write_user(usernum,&thisuser);
		nl();
		pl(get_string(360));
		nl();
		sprintf(s3,get_stringx(1,21),
		  nam(&thisuser,usernum),ctim(timer()),s);
		if (((syscfg.sysconfig & sysconfig_free_phone)==0) &&
		   (usa_phone_convention(&thisuser))) {
		  sprintf(s4,get_stringx(1,22),s2);
		  strcat(s3,s4);
		}
		sl1(0,"");
		sl1(0,s3);
		sl1(0,"");
		usernum=0;
	  }
	} else
	  if (usernum==-1) {
		write_inst(INST_LOC_NEWUSER, 0, INST_FLAGS_NONE);
		play_sdf("NEWUSER",0);
                checkit=0;
		newuser();
		ok=1;
	  } else
            if (usernum==0) {
              if (net_only)
                    nl();
              else
                    pl(get_string(8));
            } else
              if ((usernum==-2) || (usernum==-3) || (usernum==-4)) {
                if (incom) {
                  read_status();
                  time(&l);
                  s2[0]=0;
                  switch(usernum) {
                    case -2:
                      if ((instance==1) || (status.net_version>=34)) {
                            sprintf(s2,"NETWORK /B%u /T%ld /F%u",
                              modem_speed, l, modem_flag);
                            write_inst(INST_LOC_NET, 0, INST_FLAGS_NONE);
                            extern_prog(s2, EFLAG_SHRINK);
                            set_net_num(0);
                      }
                      break;
                    case -3:
                      sprintf(s2,"REMOTE /B%u /F%u",
                            modem_speed, modem_flag);
                      extern_prog(s2, EFLAG_SHRINK);
                      break;
                    case -4:
                      s[8]=0;
                      if (s[0]) {
                        sprintf(s2,"%s /B%u /F%u",
                          s, modem_speed, modem_flag);
                        sprintf(s3,"%sREMOTES.DAT",syscfg.datadir);
                        f=fsh_open(s3,"rt");
                        if (f) {
                          ok=0;
                          while ((!ok) && (fgets(s3,80,f))) {
                                ss=strchr(s3,'\n');
                                if (ss)
                                  *ss=0;
                                if (stricmp(s3,s)==0)
                                  ok=1;
                          }
                          fsh_close(f);
                          if (ok) {
                                extern_prog(s2, EFLAG_SHRINK);
                          }
                        }
                      }
                      break;
                  }
                  read_status();
                  hangup=1;
                  dtr(0);
                  global_xx=0;
                  Wait(1.0);
                  dtr(1);
                  Wait(0.1);
                  cleanup_net();
                  imodem(0);
                }
                hangup=1;
              }
  } while ((!hangup) && (!ok) && (++count<3));
  if (count==3)
	hangup=1;
  checkit=0;
  okmacro=1;
  if ((!hangup) && (usernum>0) && (thisuser.restrict & restrict_logon) &&
	(strcmp(date(),thisuser.laston)==0) && (thisuser.ontoday>0)) {
	nl();
	pl(get_string(361));
	nl();
	hangup=1;
  }
}


void logon(void)
{
  char s[255],s1[181],s2[81],*ss;
  char speed[40],ch;
  int i,i1,dv,win,f;
  long len,pos;
  long l;

  if (usernum<1) {
	hangup=1;
	return;
  }
  useron=1;
  write_inst(INST_LOC_LOGON,0,INST_FLAGS_NONE);
  if (live_user) {
	reset_colors();
	ansic(0);
	outchr(12);
  }
  if ((thisuser.num_extended) > sysinfo.max_extend_lines)
    thisuser.num_extended=sysinfo.max_extend_lines;
  if ((thisuser.colors[8]==0) || (thisuser.colors[9]==0)) {
	thisuser.colors[8]=6;
	thisuser.colors[9]=3;
  }
  if (incom && live_user) {
        if (rip_on()) {
	  if (rip_subset) {
		ripcode = 1;
		random_screen("hello");
	  } else {
		comstr("\1|1F000000HELLO.RIP|#\n");    /* Check for HELLO.RIP locally */
		ch = getkey();
		if (ch == '1') {                /* Local playback of HELLO if there */
		  comstr("\1|1R00000000HELLO.RIP\n");
		} else {
		  ripcode = 1;
		  random_screen("hello");
		}
	  }
	  ripcode = 0;
	}
	play_sdf("LOGON",0);
	i=printfile("LOGON");
	if ((!i) && (!(thisuser.sysstatus & sysstatus_pause_on_page)))
	  pausescr();
  }
  strcpy(xdate,date());
  if (strcmp(xdate,thisuser.laston)==0)
	++thisuser.ontoday;
  else {
	thisuser.ontoday=1;
	thisuser.timeontoday=0.0;
	thisuser.extratime=0.0;
	thisuser.posttoday=0;
	thisuser.etoday=0;
	thisuser.fsenttoday1=0;
  }
  ++thisuser.logons;
  cursub=0;
  msgreadlogon=0;
  if ((udir[0].subnum==0) && (udir[1].subnum>0))
	curdir=1;
  else
	curdir=0;
  curdloads=0;
  if (actsl!=255) {
	lock_status();
	++status.callernum1;
	++status.callstoday;
	save_status();
  }
  if (rip_on()) {
	strcpy (speed, curspeed);
	strcpy (curspeed, "RIP/");
	strcat (curspeed, speed);
  }
  sprintf(s,"%ld: %s %s %s   %s - %d (%d)",
	status.callernum1,
	nam(&thisuser,usernum),
	times(),
	date(),
	curspeed,
	thisuser.ontoday,
	instance);
  sprintf(s2,"%sLASTON.TXT",syscfg.gfilesdir);
  ss=get_file(s2,&len);
  pos=0;
  if (ss!=NULL) {
	if (!cs())
	  for (i=0; i<4; i++)
		copy_line(s1,ss,&pos,len);
	i=1;
	do {
	  copy_line(s1,ss,&pos,len);
	  if ((s1[0]) && live_user) {
		if (i) {
		  nln(2);
		  pl(get_string(362));
		  nl();
		  if (sysinfo.flags & OP_FLAGS_SHOW_CITY_ST) {
			if (syscfg.sysconfig & sysconfig_extended_info)
			  pl(get_string(1429));
			else
			  pl(get_string(972));
		  } else {
			pl(get_string(972));
		  }
		  ansic(7);
		  i=(okansi())?205:'=';
		  pl(charstr(79,i));
		  i=0;
		}
		pl(s1);
	  }
	} while (pos<len);
  }

  if ((actsl!=255) || (incom)) {
	sl1(0,"");
	sl1(0,stripcolors(s));
	sl1(1,"");
	if (cid_num[0]) {
	  sprintf(s,"CID NUM : %s", cid_num);
	  sysoplog(s);
	}
	if (cid_name[0]) {
	  sprintf(s,"CID NAME: %s", cid_name);
	  sysoplog(s);
	}

	if (sysinfo.flags & OP_FLAGS_SHOW_CITY_ST) {
	  if (syscfg.sysconfig & sysconfig_extended_info)
		sprintf(s,"1%-6ld %-25.25s %-5.5s %-5.5s %-15.15s %-2.2s %-3.3s %-8.8s %2d\r\n",
		  status.callernum1,
		  nam(&thisuser,usernum),
		  times(),
		  date(),
		  thisuser.city,
		  thisuser.state,
		  thisuser.country,
		  curspeed,
		  thisuser.ontoday);
	  else
		sprintf(s,"1%-6ld %-25.25s %-10.10s %-5.5s %-5.5s %-20.20s %2d\r\n",
		  status.callernum1,
		  nam(&thisuser,usernum),
		  cur_lang_name,
		  times(),
		  date(),
		  curspeed,
		  thisuser.ontoday);
	} else {
	  sprintf(s,"1%-6ld %-25.25s %-10.10s %-5.5s %-5.5s %-20.20s %2d\r\n",
		status.callernum1,
		nam(&thisuser,usernum),
		cur_lang_name,
		times(),
		date(),
		curspeed,
		thisuser.ontoday);
	}

	if (actsl!=255) {
	  sprintf(s1,"%sUSER.LOG",syscfg.gfilesdir);
	  f=sh_open(s1,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
	  if (f>-1) {
		sh_lseek(f,0L,SEEK_END);
		sh_write(f,(void *)s,strlen(s));
		sh_close(f);
	  }

	  f=sh_open(s2,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	  if (f>-1) {
		pos=0;
		copy_line(s1,ss,&pos,len);
		for (i=1; i<8; i++) {
		  copy_line(s1,ss,&pos,len);
		  strcat(s1,"\r\n");
		  sh_write(f,(void *)s1,strlen(s1));
		}
		sh_write(f,(void *)s,strlen(s));
		sh_close(f);
	  }
	}
  }

  if (ss!=NULL)
	bbsfree(ss);
  /* AutoMessage removed */
  timeon=timer();
  if (live_user) {
	topscreen();
        if (rip_on())
	  rip_menus();
  }
  if ((syscfg.logon_c[0]) && live_user) {
	nl();
	stuff_in(s,syscfg.logon_c,create_chain_file(),"","","","");
	extern_prog(s, sysinfo.spawn_opts[3]);
        nln(2);
  }
  if (live_user) {
	nl();

	outstr(get_string(237));
	pl(nam(&thisuser,usernum));
	outstr(get_string(363)); npr("%d\r\n", (int) ((nsl()+30)/60.0));
	if (thisuser.illegal) {
	  outstr(get_string(364)); npr("%d\r\n",thisuser.illegal);
	}
	if (thisuser.waiting) {
	  outstr(get_string(365)); npr("%d\r\n",thisuser.waiting);
	}
	if (thisuser.ontoday==1) {
	  outstr(get_string(366)); pl(thisuser.laston);
	} else {
	  outstr(get_string(367)); npr("%d\r\n",thisuser.ontoday);
	}
	outstr(get_string(368));
	if (!sysop2())
	  outstr(get_string(112));
	pl(get_string(315));

/****************************************************************************/
/*
 * Do NOT remove or modify this part of the code.  If the registration number
 * (or an indicator that it is unregistered) is not printed out, it will
 * be plainly obvious that this is a 'pirated' version of the source code.
 */
	if (syscfg.wwiv_reg_number)
	  sprintf(s,"(Reg #%ld)",syscfg.wwiv_reg_number);
	else
	  strcpy(s,"(Unregistered)");
	outstr(get_string(369));
	npr("%s   %s\r\n",wwiv_version,s);
/****************************************************************************/

	read_status();
	for (i=0; i<net_num_max; i++) {
	  if (net_networks[i].sysnum) {
		sprintf(s,"9%s %s0",net_networks[i].name, get_string(370));
		strcat(s,charstr(18-strlen(stripcolors(s)),'.'));
		strcat(s,"2");
		sprintf(s1,"%s @%u",s,net_networks[i].sysnum);
		if (i)
		  pl(s1);
		else {
		  for (i1=strlen(s1); i1<26; i1++)
			s1[i1]=' ';
		  s1[i1]=0;
		  npr("%s  (net%u)\r\n",s1,status.net_version);
		}
	  }
	}
	if (multitasker) {
	  outstr(get_string(1169));
	  switch (multitasker) {
		case 1 :
		  dv=get_dv_version();
		  npr("DESQView 1%d.%02d\r\n",dv/256,dv%256);
		break;
		case 2 :
		  win=get_win_version();
                  if (win==4)
                    npr("Windows 195\r\n");
                  else
                    npr("Windows 1%d.%02d\r\n",win%256,win/256);
		break;
		case 3 :
		  win=get_win_version();
		  dv=get_dv_version();
		  npr("Win 1%d.%02d 9and 2DV 1%d.%02d\r\n",
			win%256,win/256,dv/256,dv%256);
		break;
		case 4 :
                case 5 :
                case 6 :
                case 7 :
                  if ((_osmajor/10==2) && (_osminor==30))
                    npr("OS/2 3.0 1WARP\r\n");
                  else
                    npr("OS/2 1%d.%2.2d\r\n",_osmajor/10,_osminor);
		break;
		default:
		  pl(get_string(1170));
	  }
	}
	npr("%s%d\r\n",get_string(1171),instance);
	nl();
	if (thisuser.forwardusr) {
	  if (thisuser.forwardsys) {
		set_net_num(thisuser.net_num);
		if (!valid_system(thisuser.forwardsys)) {
		  thisuser.forwardusr=0;
		  thisuser.forwardsys=0;
		  strcpy(s1,get_string(371));
		} else {
		  strcpy(s1,get_string(372));
		  if (net_num_max>1)
                        sprintf(s2,"#%u @%u.%s.",
                            thisuser.forwardusr,thisuser.forwardsys, net_name);
		  else
			sprintf(s2,"#%u @%u.",
				thisuser.forwardusr,thisuser.forwardsys);
		  strcat(s1,s2);
		}
	  } else {
		if (thisuser.forwardusr==65535)
		  strcpy(s1,get_string(373));
		else {
		  strcpy(s1,get_string(372));
		  sprintf(s2,"#%u.",thisuser.forwardusr);
		  strcat(s1,s2);
		}
	  }
	  pl(s1);
	  nl();
	}
	if (ltime) {
	  nl();
	  pl(get_string(374));
	  nl();
	}
	fsenttoday=0;
	if (thisuser.year) {
	  s[0]=years_old(thisuser.month,thisuser.day,thisuser.year);
	  if (thisuser.age!=s[0]) {
		thisuser.age=s[0];
		topscreen();
	  }
	} else {
	  nl();
	  pl(get_string(375));
	  do {
		nl();
		input_age(&thisuser);
		sprintf(s,"%02d/%02d/%02d",
		  (int) thisuser.month,
		  (int) thisuser.day,
		  (int) thisuser.year);
		nl();
		outstr(s);
		outstr(get_string(376));
		if (!yn())
		  thisuser.year=0;
	  } while ((!hangup) && (thisuser.year==0));
	}
	if (!thisuser.realname[0])
	  input_realname();
	if (syscfg.sysconfig & sysconfig_extended_info) {
	  if (!thisuser.street[0])
		input_street();
	  if (!thisuser.city[0])
		input_city();
	  if (!thisuser.state[0])
		input_state();
	  if (!thisuser.country[0])
		input_country();
	  if (!thisuser.zipcode[0])
		input_zipcode();
	  if (!thisuser.dataphone[0]) {
		input_dataphone();
	  }
	  if (sysinfo.flags & OP_FLAGS_USER_REGIST) {
#define SECS_PER_DAY 86400L
		if (thisuser.registered != 0) {
		  time(&l);
		  if ((thisuser.expires < l+30*SECS_PER_DAY)
			&& (thisuser.expires > l+10*SECS_PER_DAY)) {
			npr("%s%d %s.\r\n\r\n", get_string(1394),
			  (int)((thisuser.expires-l)/SECS_PER_DAY),get_string(1395));
		  } else if ((thisuser.expires > l) &&
					 (thisuser.expires < l+10*SECS_PER_DAY)) {
			ansic(6);
			if ((int)((thisuser.expires-l)/SECS_PER_DAY) > 1) {
			  npr("%s%d %s.",get_string(1394),
				(int)((thisuser.expires-l)/SECS_PER_DAY),get_string(1395));
			} else {
			  npr("%s%d %s.",get_string(1394),
				(int)((thisuser.expires-l)/3600L),get_string(1396));
			}
                        nln(2);
			pausescr();
		  }
		  if (thisuser.expires < l) {
			if (!so()) {
			  if ((thisuser.sl > syscfg.newusersl) ||
				  (thisuser.dsl > syscfg.newuserdsl)) {
				thisuser.sl=syscfg.newusersl;
				thisuser.dsl=syscfg.newuserdsl;
				thisuser.exempt=0;
				sprintf(s1,"%s%s", nam(&thisuser,usernum),
					get_stringx(1,118));
				ssm(1,0,s1);
				write_user(usernum,&thisuser);
				reset_act_sl();
				changedsl();
			  }
			}
			ansic(6);
			pl(get_string(1397));
			nl();
			pausescr();
		  }
		}
	  }
	}
	topscreen();
	create_chain_file();
	rsm(usernum,&thisuser,1);
	if (thisuser.waiting) {
          if (menu_on()) {
		printmenu(324);
		rip_popup = 0;      /* Disable the Yes/No prompt */
	  } else
		prt(5,get_string(377));
	  if (yn()) {
                if (rip_on()) {
                  rip_cls();
		  //rmenu = 300;
		}
		readmail();
	  }
	}
  }
  if (rip_on()) {
        rip_popup = -1;      /* Re-enable */
	cleared = -1;
  }
  nscandate=thisuser.daten;
  batchtime=0.0;
  numbatchdl=numbatch=0;
  i1=0;
  for (i=0; i<20; i++) {
	if (questused[i]) {
	  if (thisuser.votes[i]==0) {
		i1=1;
	  }
	}
  }
  if (restrict_vote & thisuser.restrict)
	i1=0;
  if (actsl<=syscfg.newusersl)
	i1=0;

  if (i1 && live_user) {
	nl();
	prt(3,get_string(378));
	nl();
  }
  if ((incom) || (sysop1()))
	broadcast(get_string(1172));
  setiia(90);

  /* Handle case of first conf with no subs avail */
  if ((usub[0].subnum==-1) && (okconf(&thisuser))) {
	for (curconfsub=0;
		 (curconfsub<subconfnum) &&
		 (uconfsub[curconfsub].confnum!=-1);
		 curconfsub++) {
	  setuconf(CONF_SUBS, curconfsub, -1);
	  if (usub[0].subnum!=-1)
		break;
	}
	if (usub[0].subnum==-1) {
	  curconfsub=0;
	  setuconf(CONF_SUBS, curconfsub, -1);
	}
  }
  rip_cls();
  autox = -1;
}


void logoff(void)
{
  long l;
  int f,r,w,t,i, w1,i1;
  char s[81];
  mailrec m;
  shortmsgrec sm;
  double ton;

  if (incom)
	play_sdf("LOGOFF",0);

  if (usernum>0) {
	if ((incom) || (sysop1()))
	  broadcast(get_string(1173));
  }
  setiia(90);
  dtr(0);
  hangup=1;
  if (usernum<1)
	return;
  thisuser.lastrate=modem_speed;
  strcpy(thisuser.laston,xdate);
  thisuser.illegal=0;
  if ((timer()-timeon)<-30.0)
	timeon-=24.0*3600.0;
  ton=timer()-timeon;
  thisuser.timeon += ton;
  thisuser.timeontoday += (ton-extratimecall);
  lock_status();
  status.activetoday += (int) (ton/60.0);
  save_status();
  time(&l);
  thisuser.daten=l;
  sprintf(s,get_stringx(1,23),
	msgreadlogon,
	(int)((timer()-timeon)/60.0));
  if ((incom) || (actsl!=255))
	sl1(0,s);
  if (mailcheck) {
	f=open_email(1);
	if (f!=-1) {
	  thisuser.waiting=0;
	  t=(int) (filelength(f)/sizeof(mailrec));
	  r=0;
	  w=0;
	  while (r<t) {
		sh_lseek(f,(long)(sizeof(mailrec)) * (long)(r),SEEK_SET);
		sh_read(f,(void *)&m,sizeof(mailrec));
		if ((m.tosys!=0) || (m.touser!=0)) {
		  if ((m.tosys==0) && (m.touser==usernum)) {
			if (thisuser.waiting!=255)
			  thisuser.waiting++;
		  }
		  if (r!=w) {
			sh_lseek(f,(long)(sizeof(mailrec)) * (long)(w),SEEK_SET);
			sh_write(f,(void *)&m,sizeof(mailrec));
		  }
		  ++w;
		}
		++r;
	  }
	  if (r!=w) {
		m.tosys=0;
		m.touser=0;
		for (w1=w; w1<r; w1++) {
		  sh_lseek(f,(long)(sizeof(mailrec)) * (long)(w1),SEEK_SET);
		  sh_write(f,(void *)&m,sizeof(mailrec));
		}
	  }
	  chsize(f,(long)(sizeof(mailrec)) * (long)(w));
	  lock_status();
	  status.filechange[filechange_email]++;
	  save_status();
	  sh_close(f);
	}
  } else {
	/* re-calculate mail waiting */
	f=open_email(0);
	if (f>0) {
	  i1=filelength(f)/sizeof(mailrec);
	  thisuser.waiting=0;
	  for (i=0; (i<i1); i++) {
		read(f, &m, sizeof(mailrec));
		if ((m.tosys==0) && (m.touser==usernum)) {
		  if (thisuser.waiting!=255)
			thisuser.waiting++;
		}
	  }
	  f=sh_close(f);
	}
  }
  if (smwcheck) {
	sprintf(s,"%sSMW.DAT",syscfg.datadir);
	f=sh_open(s,O_RDWR|O_BINARY|O_CREAT,S_IREAD|S_IWRITE);
	if (f!=-1) {
	  t=(int) (filelength(f)/sizeof(shortmsgrec));
	  r=0;
	  w=0;
	  while (r<t) {
		sh_lseek(f,(long)(sizeof(shortmsgrec)) * (long)(r),SEEK_SET);
		sh_read(f,(void *)&sm,sizeof(shortmsgrec));
		if ((sm.tosys!=0) || (sm.touser!=0)) {
		  if ((sm.tosys==0) && (sm.touser==usernum))
			thisuser.sysstatus |= sysstatus_smw;
		  if (r!=w) {
			sh_lseek(f,(long)(sizeof(shortmsgrec)) * (long)(w),SEEK_SET);
			sh_write(f,(void *)&sm,sizeof(shortmsgrec));
		  }
		  ++w;
		}
		++r;
	  }
	  chsize(f,(long)(sizeof(shortmsgrec)) * (long)(w));
	  sh_close(f);
	}
  }
  if (usernum==1)
	fwaiting=thisuser.waiting;
  write_user(usernum,&thisuser);
  write_qscn(usernum,qsc,0);
  remove_from_temp("*.*", syscfgovr.tempdir, 0);
  remove_from_temp("*.*", syscfgovr.batchdir, 0);
  if (numbatch && (numbatch != numbatchdl)) {
	for (i=0; i<numbatch; i++) {
	  if (!batch[i].sending) {
		didnt_upload(i);
	  }
	}
  }
  numbatch=numbatchdl=0;
}
