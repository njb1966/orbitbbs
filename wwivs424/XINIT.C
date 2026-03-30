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
#include <math.h>
#include "ini.h"

#ifndef __OS2__
#define EMS_XMS
#endif

/****************************************************************************/

#ifndef __OS2__
void far interrupt inlii();
void far interrupt checkai();
void far interrupt plai();
void far interrupt outchri();
void far interrupt outstri();
void far interrupt nli();
void far interrupt pli();
void far interrupt emptyi();
void far interrupt inkeyi();
void far interrupt getkeyi();
void far interrupt inputi();
void far interrupt inputli();
void far interrupt yni();
void far interrupt nyi();
void far interrupt ansici();
void far interrupt oneki();
void far interrupt prti();
void far interrupt mpli();
#endif

#define INI_STRFILE 7

/****************************************************************************/

void far *mallocx(unsigned long l, char *where)
{
  void *x;
  char huge *xx;

  if (!l)
    l=1;
  x=bbsmalloc(l);
  if (!x) {
    printf("Insufficient memory (%ld bytes) for %s.\n",l,where);
    end_bbs(noklevel);
  }

  xx=(char huge *)x;
  while (l>0) {
    if (l>32768) {
      memset((void *)xx,0,32768);
      l-=32768;
      xx += 32768;
    } else {
      memset((void *)xx,0,l);
      break;
    }
  }

  return(x);
}

/****************************************************************************/

/* Turns a string into a bitmapped unsigned short flag for use with
 * extern_prog calls.
 */

unsigned short str2spawnopt(unsigned char *s)
{
  unsigned short return_val;
  unsigned char ts[128];

  return_val=0;
  strcpy(ts,s);
  strupr(ts);

  if (strstr(ts, "ABORT")!=NULL)
    return_val |= EFLAG_ABORT;
  if (strstr(ts, "INTERNAL")!=NULL)
    return_val |= EFLAG_INTERNAL;
  if (strstr(ts, "NOHANGUP")!=NULL)
    return_val |= EFLAG_NOHUP;
  if (strstr(ts, "COMIO")!=NULL)
    return_val |= EFLAG_COMIO;
  if (strstr(ts, "SHRINK")!=NULL)
    return_val |= EFLAG_SHRINK;
  if (strstr(ts, "FILES")!=NULL)
    return_val |= EFLAG_FILES;
  if (strstr(ts, "NOPAUSE")!=NULL)
    return_val |= EFLAG_NOPAUSE;
  if (strstr(ts, "NETPROG")!=NULL)
    return_val |= EFLAG_NETPROG;
  if (strstr(ts, "TOPSCREEN")!=NULL)
    return_val |= EFLAG_TOPSCREEN;

  return(return_val);
}

/****************************************************************************/

/* Takes string s and creates restrict val
*/

unsigned short str2restrict(unsigned char *s)
{
  int i,r;
  char *rs=restrict_string;
  char s1[81];

  r=0;
  strcpy(s1,s);
  strupr(s1);

  for (i=strlen(rs)-1; i>=0; i--) {
    if (strchr(s1,rs[i])) {
      r |= (1<<i);
    }
  }

  return(r);
}

/****************************************************************************/

#ifdef __OS2__
#define OFFOF(x) (((long)&thisuser.x) - ((long)&thisuser))
#else
#define OFFOF(x) (FP_OFF(&(thisuser.x))-FP_OFF(&thisuser))
#endif

/****************************************************************************/

/* Reads WWIV.INI info from [WWIV] subsection, overrides some config.dat
 * settings (as appropriate), updates config.dat with those values. Also
 * tries to read settings from [WWIV-<instnum>] subsection - this overrides
 * those in [WWIV] subsection.
 */

static unsigned char nucol[]  ={7,  11, 14, 5,  31, 2,  12,  9,  6,  3};
static unsigned char nucolbw[]={7,  15, 15, 15, 112,15, 15,  7,  7,  7};

typedef struct {
  unsigned char *name;
  unsigned short eflags;
} eventinfo_t;

static eventinfo_t eventinfo[] = {
 {"TIMED",      EFLAG_SHRINK | EFLAG_FILES},
 {"NEWUSER",    0},
 {"BEGINDAY",   EFLAG_SHRINK},
 {"LOGON",      EFLAG_COMIO},
 {"ULCHK",      EFLAG_NOHUP | EFLAG_SHRINK},
 {"FSED",       EFLAG_COMIO},
 {"PROT_SINGLE",0},
 {"PROT_BATCH", EFLAG_SHRINK|EFLAG_TOPSCREEN},
 {"CHAT",       EFLAG_SHRINK|EFLAG_FILES},
 {"ARCH_E",     EFLAG_COMIO|EFLAG_INTERNAL},
 {"ARCH_V",     EFLAG_COMIO|EFLAG_INTERNAL|EFLAG_ABORT},
 {"ARCH_A",     EFLAG_COMIO|EFLAG_INTERNAL},
};

#define INI_STR_SPAWNOPT          1
#define INI_STR_NUCOLOR           2
#define INI_STR_NUCOLORBW         3
#define INI_STR_TOPCOLOR          4
#define INI_STR_F1COLOR           5
#define INI_STR_EDITLINECOLOR     6
#define INI_STR_CHATSELCOLOR      7
#define INI_STR_TERMINAL_CMD      8
#define INI_STR_EXECUTE_CMD       9
#define INI_STR_UPLOAD_CMD        10
#define INI_STR_BEGINDAY_CMD      11
#define INI_STR_NEWUSER_CMD       12
#define INI_STR_LOGON_CMD         13
#define INI_STR_SYSTEMNAME        14
#define INI_STR_SYSTEMPHONE       15
#define INI_STR_SYSOPNAME         16
#define INI_STR_FORCE_FBACK       17
#define INI_STR_CHECK_DUP_PHONES  18
#define INI_STR_HANGUP_DUP_PHONES 19
#define INI_STR_POSTTIME_COMPENS  20
#define INI_STR_USE_SIMPLE_ASV    21
#define INI_STR_SIMPLE_ASV        22
#define INI_STR_SHOW_HIER         23
#define INI_STR_IDZ_DESC          24
#define INI_STR_SETLDATE          25
#define INI_STR_NEW_CHATSOUND     26
#define INI_STR_SLASH_SZ          27
#define INI_STR_READ_CD_IDZ       28
#define INI_STR_FSED_EXT_DESC     29
#define INI_STR_FAST_TAG_RELIST   30
#define INI_STR_MAIL_PROMPT       31
#define INI_STR_SHOW_CITY_ST      32
#define INI_STR_LOCAL_SYSOP       33
#define INI_STR_2WAY_CHAT         34
#define INI_STR_OFF_HOOK          35
#define INI_STR_PRINTER           36
#define INI_STR_LOG_DL            37
#define INI_STR_CLOSE_XFER        38
#define INI_STR_ALL_UL_SYSOP      39
#define INI_STR_NO_EASY_DL        40
#define INI_STR_NEW_EXTRACT       41
#define INI_STR_FAST_SEARCH       42
#define INI_STR_RATIO             43
#define INI_STR_NET_CALLOUT       44
#define INI_STR_WFC_SCREEN        45
#define INI_STR_FIDO_PROCESS      46
#define INI_STR_USER_REGIST       47
#define INI_STR_MSG_TAG           48
#define INI_STR_CHAIN_REG         49
#define INI_STR_CAN_SAVE_SSM      50
#define INI_STR_PACKSCAN_FREQ     51
#define INI_STR_EXTRA_COLOR       52
#define INI_STR_MAX_BATCH         53
#define INI_STR_MAX_CHAINS        54
#define INI_STR_MAX_GFILESEC      55
#define INI_STR_MSG_COLOR         56
#define INI_STR_MAIL_WHO_LEN      57
#define INI_STR_SCREEN_SAVER_TIME 58
#define INI_STR_RIP_SUPPORT       59
#define INI_STR_RIP_BBS_ID        60
#define INI_STR_RIP_MENU_DATE     61
#define INI_STR_RIP_ADJUST        62
#define INI_STR_BEEP_CHAT         63
#define INI_STR_TWO_COLOR_CHAT    64
#define INI_STR_ALLOW_ALIASES     65
#define INI_STR_UNUSED            66
#define INI_STR_USE_LIST          67
#define INI_STR_FREE_PHONE        68
#define INI_STR_EXTENDED_USERINFO 69
#define INI_STR_RIPDRIVE_ON       70
#define INI_STR_MAX_EXTEND_LINES  71
#define INI_STR_RIP_DIR           72

static char *get_key_str(int n)
{
  return(get_stringx(INI_STRFILE, n));
}

#define INI_INIT_A(n,i,f,s) \
    {if (((ss=ini_get(get_key_str(n), i, s))!=NULL) && (atoi(ss)>0)) \
      {sysinfo.f[i] = atoi(ss);}}
#define INI_INIT_A_S(n,i,f,s) \
    {if ((ss=ini_get(get_key_str(n), i, s))!=NULL) \
      {sysinfo.f[i] = str2spawnopt(ss);}}
#define INI_INIT(n,f) \
    {if (((ss=ini_get(get_key_str(n), -1, NULL))!=NULL) && (atoi(ss)>0)) \
      sysinfo.f = atoi(ss);}
#define INI_INIT_N(n,f) \
    {if (((ss=ini_get(get_key_str(n), -1, NULL))!=NULL)) \
      sysinfo.f = atoi(ss);}
#define INI_INIT_STR(n, f) \
    set_string(n, syscfg.f, sizeof(syscfg.f), 0)
#define INI_GET_ASV(s, f, func, d) \
    {if ((ss=ini_get(get_key_str(INI_STR_SIMPLE_ASV), -1, s))!=NULL) \
       sysinfo.asv.f = func (ss); \
     else \
       sysinfo.asv.f = d;}

#define NEL(s) (sizeof(s) / sizeof((s)[0]))


static ini_flags_rec sysinfo_flags[] = {
  {INI_STR_FORCE_FBACK,       0, OP_FLAGS_FORCE_NEWUSER_FEEDBACK},
  {INI_STR_CHECK_DUP_PHONES,  0, OP_FLAGS_CHECK_DUPE_PHONENUM},
  {INI_STR_HANGUP_DUP_PHONES, 0, OP_FLAGS_HANGUP_DUPE_PHONENUM},
  {INI_STR_USE_SIMPLE_ASV,    0, OP_FLAGS_SIMPLE_ASV},
  {INI_STR_POSTTIME_COMPENS,  0, OP_FLAGS_POSTTIME_COMPENSATE},
  {INI_STR_SHOW_HIER,         0, OP_FLAGS_SHOW_HIER},
  {INI_STR_IDZ_DESC,          0, OP_FLAGS_IDZ_DESC},
  {INI_STR_SETLDATE,          0, OP_FLAGS_SETLDATE},
  {INI_STR_NEW_CHATSOUND,     0, OP_FLAGS_NEW_CHATSOUND},
  {INI_STR_SLASH_SZ,          0, OP_FLAGS_SLASH_SZ},
  {INI_STR_READ_CD_IDZ,       0, OP_FLAGS_READ_CD_IDZ},
  {INI_STR_FSED_EXT_DESC,     0, OP_FLAGS_FSED_EXT_DESC},
  {INI_STR_FAST_TAG_RELIST,   0, OP_FLAGS_FAST_TAG_RELIST},
  {INI_STR_MAIL_PROMPT,       0, OP_FLAGS_MAIL_PROMPT},
  {INI_STR_SHOW_CITY_ST,      0, OP_FLAGS_SHOW_CITY_ST},
  {INI_STR_NO_EASY_DL,        0, OP_FLAGS_NO_EASY_DL},
  {INI_STR_NEW_EXTRACT,       0, OP_FLAGS_NEW_EXTRACT},
  {INI_STR_FAST_SEARCH,       0, OP_FLAGS_FAST_SEARCH},
  {INI_STR_NET_CALLOUT,       0, OP_FLAGS_NET_CALLOUT},
  {INI_STR_WFC_SCREEN,        0, OP_FLAGS_WFC_SCREEN},
  {INI_STR_FIDO_PROCESS,      0, OP_FLAGS_FIDO_PROCESS},
  {INI_STR_USER_REGIST,       0, OP_FLAGS_USER_REGIST},
  {INI_STR_MSG_TAG,           0, OP_FLAGS_MSG_TAG},
  {INI_STR_CHAIN_REG,         0, OP_FLAGS_CHAIN_REG},
  {INI_STR_CAN_SAVE_SSM,      0, OP_FLAGS_CAN_SAVE_SSM},
  {INI_STR_PACKSCAN_FREQ,     0, OP_FLAGS_PACKSCAN_FREQ},
  {INI_STR_EXTRA_COLOR,       0, OP_FLAGS_EXTRA_COLOR},
  {INI_STR_RIP_SUPPORT,       0, OP_FLAGS_RIP_SUPPORT},
  {INI_STR_RIPDRIVE_ON,       0, OP_FLAGS_RIPDRIVE_ON},
};

static ini_flags_rec sysconfig_flags[] = {
  {INI_STR_LOCAL_SYSOP,       1, sysconfig_no_local},
  {INI_STR_2WAY_CHAT,         0, sysconfig_2_way},
  {INI_STR_OFF_HOOK,          0, sysconfig_off_hook},
  {INI_STR_PRINTER,           0, sysconfig_printer},
  {INI_STR_LOG_DL,            0, sysconfig_log_dl},
  {INI_STR_CLOSE_XFER,        0, sysconfig_no_xfer},
  {INI_STR_ALL_UL_SYSOP,      0, sysconfig_all_sysop},
  {INI_STR_BEEP_CHAT,         1, sysconfig_no_beep},
  {INI_STR_TWO_COLOR_CHAT,    0, sysconfig_two_color},
  {INI_STR_ALLOW_ALIASES,     1, sysconfig_no_alias},
  {INI_STR_USE_LIST,          0, sysconfig_list},
  {INI_STR_EXTENDED_USERINFO, 1, sysconfig_no_beep},
  {INI_STR_FREE_PHONE,        0, sysconfig_free_phone},
};

/****************************************************************************/
static void set_string(int key_idx, char *ptr, int len, int force)
{
  char *ss;

  ss=ini_get(get_key_str(key_idx), -1, NULL);

  if (force && !ss)
    ss="";

  if (ss)
    strncpy(ptr, ss, len);
}

/****************************************************************************/

int read_ini_info(void)
{
  unsigned char s[81], *ss;
  int i;
  unsigned short omb, omc, omg;

  /* can't allow user to change these on-the-fly */
  omb=sysinfo.max_batch;
  omc=sysinfo.max_chains;
  omg=sysinfo.max_gfilesec;

  /* clear out the struct */
  memset(&sysinfo, 0, sizeof(system_operation_rec));

  /* Setup default sysinfo data */

  for (i=0; i<10; i++) {
    sysinfo.newuser_colors[i]=nucol[i];
    sysinfo.newuser_bwcolors[i]=nucolbw[i];
  }

  sysinfo.topscreen_color=31;
  sysinfo.f1_color=31;
  sysinfo.editline_color=112;
  sysinfo.chat_select_color=95;
  sysinfo.msg_color=2;
  sysinfo.max_batch=50;
  sysinfo.max_extend_lines=10;
  sysinfo.max_chains=50;
  sysinfo.max_gfilesec=32;
  sysinfo.mail_who_field_len=35;

  for (i=0; i<NEL(eventinfo); i++) {
    sysinfo.spawn_opts[i]=eventinfo[i].eflags;
  }

  /* put in default sysinfo.flags */
  sysinfo.flags = OP_FLAGS_FIDO_PROCESS | OP_FLAGS_RIP_SUPPORT |
                  OP_FLAGS_RIPDRIVE_ON;

  if (ok_modem_stuff)
    sysinfo.flags |= OP_FLAGS_NET_CALLOUT;
  else
    sysinfo.flags &= ~OP_FLAGS_NET_CALLOUT;

  /* open the string file for the .INI strings */
  if (set_strings_fn(-(INI_STRFILE), syscfg.gfilesdir, "INI.STR", 0)) {
    if (set_strings_fn(-(INI_STRFILE), languagedir, "INI.STR", 0)) {
      if (set_strings_fn(-(INI_STRFILE), ".", "INI.STR", 0)) {
        printf("WWIV.INI disabled; no INI.STR found\n");
        return(1);
      }
    }
  }

  /* initialize ini communication */
  sprintf(s,"WWIV-%d",instance);
  if (ini_init("WWIV.INI", s, "WWIV")==0) {
/*
 * DO NOT DO ANYTHING HERE WHICH ALLOCATES MEMORY
 * the ini_init has allocated a lot of memory, and it will be freed
 * by ini_done.  If you need to read anything here which will cause memory
 * to be allocated, only store the size of the array here, and allocate it
 * after ini_done.  Or, if necessary, call ini_done(), allocate the memory,
 * then call ini_init again, and continue processing.
 */

    /* found something */

    /* pull out event flags */
    for (i=0; i<NEL(sysinfo.spawn_opts); i++) {
      if (i<NEL(eventinfo)) {
        INI_INIT_A_S(INI_STR_SPAWNOPT, i, spawn_opts, eventinfo[i].name);
      } else {
        INI_INIT_A_S(INI_STR_SPAWNOPT, i, spawn_opts, NULL);
      }
    }

    /* pull out newuser colors */
    for (i=0; i<10; i++) {
      INI_INIT_A(INI_STR_NUCOLOR,   i, newuser_colors, NULL);
      INI_INIT_A(INI_STR_NUCOLORBW, i, newuser_bwcolors, NULL);
    }

    /* pull out sysop-side colors */
    INI_INIT(INI_STR_TOPCOLOR,      topscreen_color);
    INI_INIT(INI_STR_F1COLOR,       f1_color);
    INI_INIT(INI_STR_EDITLINECOLOR, editline_color);
    INI_INIT(INI_STR_CHATSELCOLOR,  chat_select_color);

    /* pull out sizing options */
    INI_INIT(INI_STR_MAX_BATCH,     max_batch);
    INI_INIT(INI_STR_MAX_EXTEND_LINES,max_extend_lines);
    INI_INIT(INI_STR_MAX_CHAINS,    max_chains);
    INI_INIT(INI_STR_MAX_GFILESEC,  max_gfilesec);

    /* pull out strings */
    INI_INIT_STR(INI_STR_TERMINAL_CMD,  terminal);
    INI_INIT_STR(INI_STR_SYSTEMNAME,    systemname);
    INI_INIT_STR(INI_STR_SYSTEMPHONE,   systemphone);
    INI_INIT_STR(INI_STR_SYSOPNAME,     sysopname);
    INI_INIT_STR(INI_STR_EXECUTE_CMD,   executestr);
    INI_INIT_STR(INI_STR_UPLOAD_CMD,    upload_c);
    INI_INIT_STR(INI_STR_BEGINDAY_CMD,  beginday_c);
    INI_INIT_STR(INI_STR_NEWUSER_CMD,   newuser_c);
    INI_INIT_STR(INI_STR_LOGON_CMD,     logon_c);


    set_string(INI_STR_RIP_BBS_ID, sysinfo.ripbbsid, sizeof(sysinfo.ripbbsid), 0);
    set_string(INI_STR_RIP_MENU_DATE, sysinfo.ripmenudate, sizeof(sysinfo.ripmenudate), 0);
    strcpy(sysinfo.ripdir, "ripmenus\\");
    set_string(INI_STR_RIP_DIR, sysinfo.ripdir, sizeof(sysinfo.ripdir), 0);
    if (sysinfo.ripdir[strlen(sysinfo.ripdir)-1] != '\\')
      strcat(sysinfo.ripdir, "\\");
    INI_INIT_N(INI_STR_RIP_ADJUST, rip_rm_adjust);

    /* pull out sysinfo flags */
    sysinfo.flags = ini_flags('Y', get_key_str, sysinfo_flags,
                              NEL(sysinfo_flags), sysinfo.flags);


    /* allow override of msg_color */
    INI_INIT(INI_STR_MSG_COLOR,     msg_color);

    /* get asv values */
    INI_GET_ASV("SL", sl, atoi, syscfg.autoval[9].sl);
    INI_GET_ASV("DSL", dsl, atoi, syscfg.autoval[9].dsl);
    INI_GET_ASV("EXEMPT", exempt, atoi, 0);
    INI_GET_ASV("AR", ar, str_to_arword, syscfg.autoval[9].ar);
    INI_GET_ASV("DAR", dar, str_to_arword, syscfg.autoval[9].dar);
    INI_GET_ASV("RESTRICT", restrict, str2restrict, 0);

    /* sysconfig flags */
    syscfg.sysconfig = ini_flags('Y', get_key_str, sysconfig_flags,
                                 NEL(sysconfig_flags), syscfg.sysconfig);

    /* misc stuff */
    if (((ss=ini_get(get_key_str(INI_STR_MAIL_WHO_LEN), -1, NULL))!=NULL) &&
        ((atoi(ss)>0) || (ss[0]=='0')))
      sysinfo.mail_who_field_len = atoi(ss);
    if ((ss=ini_get(get_key_str(INI_STR_RATIO), -1, NULL))!=NULL)
      syscfg.req_ratio = atof(ss);

    INI_INIT_N(INI_STR_SCREEN_SAVER_TIME, screen_saver_time);

    /* now done */
    ini_done();
  }

  if (sysinfo.max_extend_lines>99)
    sysinfo.max_extend_lines=99;
  if (sysinfo.max_batch>999)
    sysinfo.max_batch=999;
  if (sysinfo.max_chains>999)
    sysinfo.max_chains=999;
  if (sysinfo.max_gfilesec>999)
    sysinfo.max_gfilesec=999;

  if (omb)
    sysinfo.max_batch=omb;
  if (omc)
    sysinfo.max_chains=omc;
  if (omg)
    sysinfo.max_gfilesec=omg;


  set_strings_fn(INI_STRFILE, NULL, NULL, 0);

  /* Success if here */
  return(1);
}

/****************************************************************************/

int read_config(void)
{
  int f,i;

  f=sh_open1("CONFIG.DAT",O_RDONLY | O_BINARY);
  if (f<0) {
    printf("CONFIG.DAT NOT FOUND.\n");
    return(1);
  }
  sh_read(f,(void *) (&syscfg), sizeof(configrec));
  sh_close(f);

  f=sh_open1("CONFIG.OVR", O_RDONLY|O_BINARY);
  if ((f>0) && (filelength(f) < (long)instance*(long)sizeof(configoverrec))) {
    f=sh_close(f);
    printf("Not enough instances configured.\n");
    end_bbs(noklevel);
  }
  if (f<0) {
    /* slap in the defaults */
    for (i=0; i<4; i++) {
      syscfgovr.com_ISR[i+1]=syscfg.com_ISR[i+1];
      syscfgovr.com_base[i+1]=syscfg.com_base[i+1];
      syscfgovr.com_ISR[i+5]=syscfg.com_ISR[i+1];
      syscfgovr.com_base[i+5]=syscfg.com_base[i+1];
    }
    syscfgovr.primaryport=syscfg.primaryport;
    strcpy(syscfgovr.modem_type, syscfg.modem_type);
    strcpy(syscfgovr.tempdir, syscfg.tempdir);
    strcpy(syscfgovr.batchdir, syscfg.batchdir);
    if (syscfg.sysconfig & sysconfig_high_speed)
      syscfgovr.comflags |= comflags_buffered_uart;
  } else {
    lseek(f, (instance-1)*sizeof(configoverrec), SEEK_SET);
    read(f, &syscfgovr, sizeof(configoverrec));
    sh_close(f);
  }
  return(0);
}

/****************************************************************************/

int save_config(void)
{
  int f;

  f=sh_open1("CONFIG.DAT",O_RDWR | O_BINARY);
  if (f>0) {
    sh_write(f,(void *) (&syscfg), sizeof(configrec));
    sh_close(f);
    return(0);
  }
  return(1);
}

/****************************************************************************/

void read_nextern(void)
{
  int f;
  char s[81];
  long l;

  if (externs)
    bbsfree(externs);
  externs=NULL;

  sprintf(s,"%sNEXTERN.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    if (l>15*sizeof(newexternalrec))
      l=15*sizeof(newexternalrec);
    externs=mallocx(l+10, "external protocols");
    numextrn=(sh_read(f,(void *)externs, (unsigned) l))/sizeof(newexternalrec);
    sh_close(f);
  } else
    numextrn=0;
}

/****************************************************************************/

void read_editors(void)
{
  int f;
  char s[81];
  long l;

  if (editors)
    bbsfree(editors);
  editors=NULL;
  sprintf(s,"%sEDITORS.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    if (l>10*sizeof(editorrec))
      l=10*sizeof(editorrec);
    editors=mallocx(l+10, "external editors");
    numed=(sh_read(f,(void *)editors, (unsigned) l))/sizeof(editorrec);
    sh_close(f);
  }
}

/****************************************************************************/

void read_nintern(void)
{
  int f;
  char s[81];

  if (over_intern)
    bbsfree(over_intern);
  over_intern=NULL;
  sprintf(s,"%sNINTERN.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY|O_BINARY);
  if (f>0) {
    over_intern=mallocx(3*sizeof(newexternalrec),"internal protocol overrides");
    sh_read(f,over_intern, 3*sizeof(newexternalrec));
    sh_close(f);
  }
}

/****************************************************************************/

int read_subs(void)
{
  int f;
  char s[81];

  if (subboards)
    bbsfree(subboards);
  subboards=NULL;
  max_subs=syscfg.max_subs;
  subboards=(subboardrec *) mallocx(max_subs*sizeof(subboardrec), "subboards");
  sprintf(s,"%sSUBS.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0) {
    printf("%s NOT FOUND.\n",s);
    return(1);
  }
  num_subs=(sh_read(f,subboards, (max_subs*sizeof(subboardrec))))/
    sizeof(subboardrec);
  f=sh_close(f);
  return(read_subs_xtr(max_subs, num_subs, subboards));
}

/****************************************************************************/

void read_networks(void)
{
  int f,i;
  char s[81],*ss;

  if (net_networks)
    bbsfree(net_networks);
  net_networks=NULL;
  sprintf(s,"%sNETWORKS.DAT", syscfg.datadir);
  f=sh_open1(s,O_RDONLY|O_BINARY);
  if (f>0) {
    net_num_max=filelength(f)/sizeof(net_networks_rec);
    if (net_num_max) {
      net_networks=mallocx(net_num_max*sizeof(net_networks_rec),"networks.dat");
      sh_read(f, net_networks, net_num_max*sizeof(net_networks_rec));
    }
    sh_close(f);
    for (i=0; i<net_num_max; i++) {
      ss=strchr(net_networks[i].name, ' ');
      if (ss)
        *ss=0;
    }
  }
  if (!net_networks) {
    net_networks=mallocx(sizeof(net_networks_rec), "networks.dat");
    net_num_max=1;
    strcpy(net_networks->name,"WWIVnet");
    strcpy(net_networks->dir, syscfg.datadir);
    net_networks->sysnum=syscfg.systemnumber;
  }
}

/****************************************************************************/

int read_names(void)
{
  int f;
  char s[81];

  if (smallist)
    bbsfree((void *)smallist);
  smallist=NULL;
  smallist=(smalrec *) mallocx(((long)syscfg.maxusers) * ((long)sizeof(smalrec)),
                               "names.lst - try decreasing max users in INIT");
  sprintf(s,"%sNAMES.LST",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0) {
    printf("%s NOT FOUND.\n",s);
    return(1);
  }
  huge_xfer(f, smallist, sizeof(smalrec), status.users, 0);
  sh_close(f);
  read_status();
  return(0);
}

/****************************************************************************/

void read_voting(void)
{
  int f,n,i;
  char s[81];
  votingrec v;

  for (i=0; i<20; i++)
    questused[i]=0;
  sprintf(s,"%sVOTING.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    n=(int) (filelength(f) / sizeof(votingrec)) -1;
    for (i=0; i<n; i++) {
      sh_lseek(f,(long) i * sizeof(votingrec),SEEK_SET);
      sh_read(f,(void *)&v,sizeof(votingrec));
      if (v.numanswers)
        questused[i]=1;
    }
    f=sh_close(f);
  }
}

/****************************************************************************/

int read_dirs(void)
{
  int f;
  char s[81];

  if (directories)
    bbsfree((void *)directories);
  directories=NULL;
  max_dirs=syscfg.max_dirs;
  directories=(directoryrec huge *)mallocx(((long)max_dirs)*((long)sizeof(directoryrec)),
    "directories");
  sprintf(s,"%sDIRS.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0) {
    printf("%s NOT FOUND.\n",s);
    return(1);
  }
  num_dirs=huge_xfer(f, directories, sizeof(directoryrec), max_dirs, 0) /
    sizeof(directoryrec);
  sh_close(f);
  return(0);
}

/****************************************************************************/

void read_chains(void)
{
  int f,i,i1;
  char s[81];

  if (chains)
    bbsfree(chains);
  chains=NULL;
  chains=(chainfilerec *) mallocx(sysinfo.max_chains* sizeof(chainfilerec), "chains");
  sprintf(s,"%sCHAINS.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    numchain=(sh_read(f,(void *)chains, sysinfo.max_chains*sizeof(chainfilerec)))/
      sizeof(chainfilerec);
    numchain=numchain;
  }
  sh_close(f);
  if (sysinfo.flags & OP_FLAGS_CHAIN_REG) {
    if (chains_reg)
      bbsfree(chains_reg);
    chains_reg=NULL;
    chains_reg=(chainregrec *) mallocx(sysinfo.max_chains* sizeof(chainregrec),
      "chain registration");
    sprintf(s,"%sCHAINS.REG",syscfg.datadir);
    f=sh_open1(s,O_RDONLY | O_BINARY);
    if (f>0) {
      sh_read(f,(void *)chains_reg, sysinfo.max_chains*sizeof(chainregrec));
    } else {
      for(i=0;i<numchain;i++) {
        for (i1=0; i1<sizeof(chains_reg[i].regby)/sizeof(chains_reg[i].regby[0]); i1++)
          chains_reg[i].regby[i1]=0;
        chains_reg[i].usage=0;
        chains_reg[i].minage=0;
        chains_reg[i].maxage=255;
      }
      f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      sh_write(f,(void *) (chains_reg), (sizeof(chainregrec) * numchain));
    }
    sh_close(f);
  }
}

/****************************************************************************/

int read_language(void)
{
  int f;
  char s[81];

  if (languages)
    bbsfree(languages);
  languages=NULL;
  sprintf(s,"%sLANGUAGE.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY|O_BINARY);
  if (f>=0) {
    num_languages=filelength(f)/sizeof(languagerec);
    if (num_languages) {
      languages=mallocx(num_languages*sizeof(languagerec),"language.dat");
      sh_read(f,languages,num_languages*sizeof(languagerec));
    }
    sh_close(f);
  }
  if (!num_languages) {
    languages=mallocx(sizeof(languagerec),"language.dat");
    num_languages=1;
    strcpy(languages->name,"English");
    strncpy(languages->dir,syscfg.gfilesdir, sizeof(languages->dir)-1);
  }
  cur_lang=-1;
  if (set_language(0)) {
    printf("You need the default language fully installed to run the BBS.\n");
    return(1);
  }
  return(0);
}

/****************************************************************************/

int read_modem(void)
{
  int f;
  char s[81];
  long l;

  if (modem_i)
    bbsfree(modem_i);
  modem_i=NULL;
  if (!ok_modem_stuff)
    return(0);
  if (instance > 1)
    sprintf(s,"%sMODEM.%3.3d",syscfg.datadir,instance);
  else
    sprintf(s,"%sMODEM.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f>0) {
    l=filelength(f);
    modem_i = mallocx(l, "modem.dat");
    sh_read(f,modem_i, (unsigned) l);
    sh_close(f);
    return(0);
  } else {
    printf("\nRun INIT.EXE to convert modem data.\n\n");
    return(1);
  }
}

/****************************************************************************/

void read_gfile(void)
{
  int f;
  char s[81];

  if (gfilesec)
    bbsfree(gfilesec);
  gfilesec=NULL;
  gfilesec=(gfiledirrec *) mallocx((long) (sysinfo.max_gfilesec* sizeof(gfiledirrec)), "gfiles");
  sprintf(s,"%sGFILE.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<0)
    num_sec=0;
  else
    num_sec=sh_read(f,(void *)gfilesec,sysinfo.max_gfilesec*sizeof(gfiledirrec))/sizeof(gfiledirrec);
  sh_close(f);
}

/****************************************************************************/

int make_abs_path(unsigned char *checkdir)
/*
 * Makes a path into an absolute path, returns 1 if original path altered,
 * else returns 0.
 */
{
  unsigned char olddir[81];

  if ((strlen(checkdir)<3) || (checkdir[1]!=':') || (checkdir[2]!='\\')) {
    get_dir(olddir,1);
    cd_to(cdir);
    cd_to(checkdir);
    get_dir(checkdir, 1);
    cd_to(olddir);
    return(1);
  }
  return(0);
}

/****************************************************************************/

void fix_paths(void)
/*
 * Makes relative paths into absolute paths. Can solve some hard-to-find
 * problems.
 */
{
  int f=-1, update=0;

  /* config.dat paths */
  if (make_abs_path(syscfg.gfilesdir))
    update|=1;
  if (make_abs_path(syscfg.datadir))
    update|=1;
  if (make_abs_path(syscfg.msgsdir))
    update|=1;
  if (make_abs_path(syscfg.dloadsdir))
    update|=1;

  /* config.ovr paths */
  if (make_abs_path(syscfgovr.tempdir)) {
    if (instance==1) {
      strncpy(syscfg.tempdir,syscfgovr.tempdir,sizeof(syscfg.tempdir));
      update|=1;
    }
    update|=2;
  }
  if (make_abs_path(syscfgovr.batchdir)) {
    if (instance==1) {
      strncpy(syscfg.batchdir,syscfgovr.batchdir,sizeof(syscfg.batchdir));
      update|=1;
    }
    update|=2;
  }

  if (update&1) {
    save_config();
  }
  if (update&2) {
    f=sh_open1("CONFIG.OVR", O_RDWR|O_BINARY);
    if ((f<0) || (filelength(f) < (long)instance*(long)sizeof(configoverrec)))
      f=sh_close(f);
    else {
      lseek(f, (instance-1)*sizeof(configoverrec), SEEK_SET);
      write(f, &syscfgovr, sizeof(configoverrec));
      sh_close(f);
    }
  }
}

/****************************************************************************/

void init(void)
{
  char *ss,s[161];
  int i,i1,sm;
  int f1,f2,f3,f4,f5,f6,f7, pk;
#ifndef __OS2__
  union REGS r;
#endif
  struct date today;


#ifdef __OS2__
  VIOMODEINFO vmi;

  VioGetMode(&vmi, 0);
  if (vmi.fbType & 0x02)
    {
      printf("\n\nYou must be in text mode to run WWIV.\n\n");
      end_bbs(noklevel);
    }
  if (vmi.col != ((USHORT)80))
    {
      printf("%u\n\nYou must be in 80 column mode to run WWIV.\n\n", (unsigned int)vmi.col);
      end_bbs(noklevel);
    }
  if ((!(vmi.fbType & 0x01)) || (vmi.color < 255))
    crttype = 7;
  else
    crttype = 0;
  defscreenbottom = vmi.row - 1;
#else
  save_dos=find_interrupt();
  if (!save_dos) {
    printf("\nNo spare interrupt vector found to use.\n\n");
    end_bbs(noklevel);
  }
  crttype=peekb(0x0040,0x0049);
  if (crttype==7)
    scrn=MK_FP(0xb000,0x0000);
  else
    scrn=MK_FP(0xb800,0x0000);

  detect_multitask();

  r.h.ah=15;
  int86(0x10,&r,&r);
  sm=r.h.al;
  if (r.h.ah!=80) {
    printf("\n\nYou must be in 80 column mode to run WWIV.\n\n");
    end_bbs(noklevel);
  }
  if ((sm==4) || (sm==5) || (sm==6)) {
    printf("\n\nYou must be in text mode to run WWIV.\n\n");
    end_bbs(noklevel);
  }
  defscreenbottom=(int) peekb(0x0000,0x0484);
  if (defscreenbottom<24)
    defscreenbottom=24;
  if (defscreenbottom>63)
    defscreenbottom=24;
  if ((defscreenbottom!=42) && (defscreenbottom!=49))
    defscreenbottom=24;
#endif
  screenbottom=defscreenbottom;
  screenlen=160*(screenbottom+1);
  if (instance > 1)
    sprintf(s,"RESTORE.%3.3d",instance);
  else
    sprintf(s,"RESTORE.WWV");
  if (!exist(s)) {
    for (i=0; i<screenbottom; i++)
      printf("\n");
    strcpy(s,wwiv_version);
    strcat(s,", Copyright (c) 1988-1995 by Wayne Bell.\n\n");
    printf(s);
  }
  strcpy(cdir,"X:\\");
  cdir[0]='A'+getdisk();
  getcurdir(0,&(cdir[3]));
  if (cdir[3]==0)
    cdir[2]=0;

#ifdef EMS_XMS
  if (_OvrInitEms(0,0,16)!=0)
    _OvrInitExt(0L,0);
#endif

  curlsub=-1;
  curldir=-1;
#ifndef __OS2__
  setvect(save_dos, getvect(INT_REAL_DOS));
#endif
  oldx=0;
  oldy=0;
#ifndef __OS2__
  itimer();
  r.h.ah=0x33;
  r.h.al=0x01;
  r.h.dl=0x00;
  int86(INT_REAL_DOS,&r,&r);
#endif
  use_workspace=0;
  input_extern=0;
  chat_file=0;
  sysop_alert=0;
  global_handle=0;
  tagptr=0;

  for (i=0; i<25; i++)
    funcs[i]=NULL;
#ifndef __OS2__
  funcs[0]=(void far *)inlii;
  funcs[1]=(void far *)checkai;
  funcs[2]=(void far *)plai;
  funcs[3]=(void far *)outchri;
  funcs[4]=(void far *)outstri;
  funcs[5]=(void far *)nli;
  funcs[8]=(void far *)pli;
  funcs[9]=(void far *)emptyi;
  funcs[10]=(void far *)inkeyi;
  funcs[11]=(void far *)getkeyi;
  funcs[12]=(void far *)inputi;
  funcs[13]=(void far *)inputli;
  funcs[14]=(void far *)yni;
  funcs[15]=(void far *)nyi;
  funcs[16]=(void far *)ansici;
  funcs[17]=(void far *)oneki;
  funcs[18]=(void far *)prti;
  funcs[19]=(void far *)mpli;
  sprintf(ver_no2,"WWIV_FP=%04.4X:%04.4X",FP_SEG(funcs), FP_OFF(funcs));
#else
  strcpy(ver_no2, "WWIV_FP=0000:0000");
#endif

  strcpy(ver_no1,"BBS=");
  strcat(ver_no1,wwiv_version);

  getdate(&today);
  if (today.da_year<1988) {
    printf("You need to set the date & time before running the BBS.\n");
    end_bbs(noklevel);
  }

  if (read_config())
    end_bbs(noklevel);

  strcpy(s,syscfgovr.tempdir);
  i=strlen(s);
  if (s[0]==0)
    i1=1;
  else {
    if ((s[i-1]=='\\') && (s[i-2]!=':'))
      s[i-1]=0;
    i1=chdir(s);
  }
  if (i1) {
    printf("\nYour temp dir isn't valid.\n");
    printf("It is now set to: '%s'\n\n",syscfgovr.tempdir);
    end_bbs(noklevel);
  } else
    cd_to(cdir);

  strcpy(s,syscfgovr.batchdir);
  i=strlen(s);
  if (s[0]==0)
    i1=1;
  else {
    if ((s[i-1]=='\\') && (s[i-2]!=':'))
      s[i-1]=0;
    i1=chdir(s);
  }
  if (i1) {
    printf("\nYour batch dir isn't valid.\n");
    printf("It is now set to: '%s'\n\n",syscfgovr.batchdir);
    end_bbs(noklevel);
  } else
    cd_to(cdir);

#ifdef __OS2__
  stdprn = NULL;
  if (syscfg.sysconfig & sysconfig_printer)
    if ((stdprn = fopen("PRN", "wt")) == NULL)
      {
      printf("\nCannot open PRN device for output.\n");
      end_bbs(noklevel);
      }
#endif


  fix_paths();

  write_inst(INST_LOC_INIT,0,INST_FLAGS_NONE);

  /* make sure it is the new userrec structure */
  sprintf(s,"%sUSER.QSC",syscfg.datadir);
  if (!exist(s)) {
    printf("You must go into INIT and convert your userlist before running the BBS.\n");
    end_bbs(noklevel);
  }

  /* update user info data */
  f1=sizeof(userrec);
  f2=OFFOF(waiting);
  f3=OFFOF(inact);
  f4=OFFOF(sysstatus);
  f5=OFFOF(forwardusr);
  f6=OFFOF(forwardsys);
  f7=OFFOF(net_num);

  if ((f1!=syscfg.userreclen) ||
      (f2!=syscfg.waitingoffset) ||
      (f3!=syscfg.inactoffset) ||
      (f4!=syscfg.sysstatusoffset) ||
      (f5!=syscfg.fuoffset) ||
      (f6!=syscfg.fsoffset) ||
      (f7!=syscfg.fnoffset)) {

    syscfg.userreclen=f1;
    syscfg.waitingoffset=f2;
    syscfg.inactoffset=f3;
    syscfg.sysstatusoffset=f4;
    syscfg.fuoffset=f5;
    syscfg.fsoffset=f6;
    syscfg.fnoffset=f7;

    /* store the new config.dat file */
    save_config();
  }

  if (!syscfgovr.primaryport)
    ok_modem_stuff=0;

  languages=NULL;
  if (read_language())
    end_bbs(noklevel);

  if (!read_ini_info()) {
    printf("Insufficient memory for system info structure.\n");
    end_bbs(noklevel);
  }

  net_networks=NULL;
  net_num=0;
  read_networks();
  set_net_num(0);


  get_status(0, 1);

  status.wwiv_version=wwiv_num_version;
  if (status.callernum!=65535) {
    status.callernum1=(long)status.callernum;
    status.callernum=65535;
  }
  save_status();

  gat=(unsigned short *) mallocx(2048 * sizeof(short), "gat");

  gfilesec=NULL;
  read_gfile();

  smallist=NULL;

  if (read_names())
    end_bbs(noklevel);

  subboards=NULL;

  if (read_subs())
    end_bbs(noklevel);

  directories=NULL;

  if (read_dirs())
    end_bbs(noklevel);

  numchain=0;
  chains=NULL;
  read_chains();

  modem_i=NULL;
  if (read_modem())
    end_bbs(noklevel);

  numextrn=0;
  externs=NULL;
  read_nextern();

  over_intern=NULL;
  read_nintern();

  numed=0;
  editors=NULL;
  read_editors();

  /* allocate sub cache */
  iscan1(-1, 0);

  batch=mallocx(sysinfo.max_batch * sizeof(batchrec), "batch list");

  read_user(1,&thisuser);
  if (thisuser.inact & inact_deleted)
    fwaiting=0;
  else
    fwaiting=thisuser.waiting;

  read_status();
  if (ok_modem_stuff && !restoring_shrink) {
    initport(syscfgovr.primaryport);
    do_result(&(modem_i->defl));
  }
  if (syscfg.sysconfig & sysconfig_no_local)
    topdata=0;
  else
    topdata=2;
  ss=getenv("PROMPT");
  strcpy(newprompt,"PROMPT=OrbitBBS: ");
  if (ss)
    strcat(newprompt,ss);
  else
    strcat(newprompt,"$P$G");
  if (instance > 1)
    sprintf(dszlog,"%s\\WWIVDSZ.%3.3d",cdir,instance);
  else
    sprintf(dszlog,"%s\\WWIVDSZ.LOG",cdir);
  sprintf(s,"DSZLOG=%s",dszlog);
  pk=i=i1=0;
  while (environ[i]!=NULL) {
    if (strncmp(environ[i],"PKNOFASTCHAR=",13)==0)
      pk=1;
    if (strncmp(environ[i],"PROMPT=",7)==0)
      xenviron[i1++]=newprompt;
    else
      if (strncmp(environ[i],"DSZLOG=",7)==0)
        xenviron[i1++]=strdup(s);
      else {
        if ((strncmp(environ[i],"BBS=",4)) &&
            (strncmp(environ[i],"WWIV_FP=",8)) &&
            (strncmp(environ[i],"WWIV_NET=",8)))
          xenviron[i1++]=environ[i];
      }
    ++i;
  }
  if (!getenv("DSZLOG"))
    xenviron[i1++]=strdup(s);
  if (!ss)
    xenviron[i1++]=newprompt;
  xenviron[i1++]=ver_no1;
  xenviron[i1++]=ver_no2;
  xenviron[i1++]=wwiv_net_no;
  if (!pk)
    xenviron[i1++]="PKNOFASTCHAR=Y";
  xenviron[i1]=NULL;

  read_voting();

  if (syscfgovr.comflags & comflags_buffered_uart)
    high_speed=1;
  else
    high_speed=0;
  time_event=((double)syscfg.executetime)*60.0;
  last_time=time_event-timer();
  if (last_time<0.0)
    last_time+=24.0*3600.0;
  do_event=0;
  usub=(usersubrec *)mallocx(max_subs*sizeof(usersubrec), "usub");
  sub_dates=(unsigned long *) mallocx(max_subs*sizeof(long),"sub_dates");

  udir=(usersubrec *)mallocx(max_dirs*sizeof(usersubrec), "udir");
  dir_dates=(unsigned long *) mallocx(max_dirs*sizeof(long),"dir_dates");

  uconfsub=(userconfrec *)mallocx(MAX_CONFERENCES*sizeof(userconfrec), "uconfsub");
  uconfdir=(userconfrec *)mallocx(MAX_CONFERENCES*sizeof(userconfrec), "uconfdir");

  qsc=(unsigned long *)mallocx(syscfg.qscn_len, "quickscan");
  qsc_n=qsc+1;
  qsc_q=qsc_n+(max_dirs+31)/32;
  qsc_p=qsc_q+(max_subs+31)/32;

  ss=getenv("ORBIT_INSTANCE");
  if (ss) {
    i=atoi(ss);
    if (i>0)
      sprintf(nete,".%3.3d",i);
    else
      strcpy(nete,".NET");
  } else {
    strcpy(nete,".NET");
  }

  read_bbs_list_index();
  frequent_init();
  if (!restoring_shrink && !already_on) {
    tmp_disable_pause(1);
    remove_from_temp("*.*", syscfgovr.tempdir, 1);
    remove_from_temp("*.*", syscfgovr.batchdir, 1);
    tmp_disable_pause(0);
    imodem(1);
    cleanup_net();
  }

  lecho=ok_local();
  daylight=0;
  find_devices();

  subconfnum=dirconfnum=0;
  read_all_conferences();

  if (!restoring_shrink && !already_on) {
    sprintf(s,get_stringx(1,130),wwiv_version,
            instance, times(), date());
    sl1(0,"");
    sl1(0,s);
    sl1(0,"");
  }

  if (instance > 1)
    sprintf(s,"WWIV_NET.%3.3d",instance);
  else
    sprintf(s,"WWIV_NET.DAT");
  unlink(s);

  randomize();

  if (!restoring_shrink)
    catsl();

  write_inst(INST_LOC_WFC,0,INST_FLAGS_NONE);
}
/****************************************************************************/

void gotcaller(unsigned int ms, unsigned int cs)
{
  double d;

  frequent_init();
  com_speed = cs;
  modem_speed = ms;
  sl1(1,"");
  read_user(1,&thisuser);
  read_qscn(1,qsc,0);
  reset_act_sl();
  usernum=1;
  if (thisuser.inact & inact_deleted) {
    thisuser.screenchars=80;
    thisuser.screenlines=25;
  }
  screenlinest=25;
  clrscrb();
  outs(get_string(46));
  outs(curspeed);
  outs("...\r\n");
  if (ms) {
    set_baud(cs);
    incom=1;
    outcom=1;
    using_modem=1;
  } else {
    using_modem=incom=outcom=0;
  }
  d=(timer()) / 102.723;
  d-=floor(d);
  d*=10000.0;
  srand((unsigned int)d);
}

/****************************************************************************/
