/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/


#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <dos.h>
#include <alloc.h>
#include <time.h>

#include "vardec.h"

/****************************************************************************/

#ifdef __OS2__
#define OFFOF(x) (((long)(&(u.x))) - ((long)&u))
#else
#define OFFOF(x) (FP_OFF(&(u.x))-FP_OFF(&u))
#endif
#define C_STR(a) strncpy(new.a,old.a,sizeof(new.a)-1)
#define C_NUM(a) new.a=old.a


/****************************************************************************/



/* DATA FOR EVERY USER */
typedef struct {
	char		name[31],		/* user's name */
			realname[21],		/* user's real name */
			callsign[7],		/* user's amateur callsign */
			phone[13],		/* user's phone number */
			pw[9],			/* user's password */
			laston[9],		/* last date on */
			firston[9],		/* first date on */
			note[41],		/* sysop's note about user */
			macros[3][81],		/* macro keys */
			sex;			/* user's sex */
	unsigned char	age,			/* user's age */
			inact,			/* if deleted or inactive */
			comp_type,		/* computer type */
			defprot,		/* deflt transfer protocol */
			defed,			/* default editor */
			screenchars,screenlines,/* screen size */
			sl,			/* security level */
			dsl,			/* transfer security level */
			exempt,			/* exempt from ratios, etc */
			colors[8],		/* user's colors */
			votes[20],		/* user's votes */
			illegal,		/* illegal logons */
			waiting,		/* number mail waiting */
			sysopsub,		/* sysop sub board number */
			ontoday;		/* num times on today */
	unsigned short	homeuser,homesys,	/* where user can be found */
			forwardusr,forwardsys,	/* where to forward mail */
			msgpost,		/* number messages posted */
			emailsent,		/* number of email sent */
			feedbacksent,		/* number of f-back sent */
			posttoday,		/* number posts today */
			etoday,			/* number emails today */
			ar,			/* board access */
			dar,			/* directory access */
			restrict,		/* restrictions on account */
			ass_pts,		/* bad things the user did */
			uploaded,		/* number files uploaded */
			downloaded,		/* number files downloaded */
			lastrate,		/* last baud rate on */
			logons;			/* total number of logons */
	unsigned long	msgread,		/* total num msgs read */
			uk,			/* number of k uploaded */
			dk,			/* number of k downloaded */
			qscn,			/* which subs to n-scan */
                        qscnptr[33],            /* q-scan pointers */
			nscn1,nscn2,		/* which dirs to n-scan */
			daten,			/* numerical time last on */
			sysstatus;		/* status/defaults */
	float		timeontoday,		/* time on today */
			extratime,		/* time left today */
			timeon,			/* total time on system */
			pos_account,		/* $ credit */
			neg_account,		/* $ debit */
			gold;			/* game money */
	unsigned char	bwcolors[8];		/* b&w colors */
	unsigned char	month,day,year;		/* user's birthday */
	unsigned int    emailnet,		/* email sent into net */
			postnet;		/* posts sent into net */
	unsigned short	fsenttoday1;		/* feedbacks today */
        unsigned char   num_extended;           /* num lines of ext desc */
        unsigned char   optional_val;           /* optional lines in msgs */
        unsigned long   wwiv_regnum;            /* users WWIV reg number */
        unsigned char   net_num;                /* net_num for forwarding */
        char            res[28];                /* reserved bytes */
        unsigned long   qscn2;                  /* additional qscan ptr */
        unsigned long   qscnptr2[32];           /* additional quickscan ptrs */
} olduserrec;

/****************************************************************************/

static int ouf, nuf, qf, qfl;
static char oun[128],nun[128],qfn[128],sun[128];
static unsigned long *qsc, *qsc_n, *qsc_q, *qsc_p;

#ifdef INIT
extern configrec syscfg;
#else
static configrec syscfg;
#endif


/****************************************************************************/

void c_setup(void)
{
  int f;

#ifndef INIT
  f=open("CONFIG.DAT",O_RDONLY|O_BINARY);
  if (f>0) {
    read(f,&syscfg,sizeof(syscfg));
    close(f);

#endif
    sprintf(oun,"%sUSER.LST",syscfg.datadir);
    sprintf(nun,"%sUSER.NEW",syscfg.datadir);
    sprintf(qfn,"%sUSER.QSC",syscfg.datadir);
    sprintf(sun,"%sUSER.OLD",syscfg.datadir);
#ifndef INIT
  } else {
    printf("CONFIG.DAT not found; run in main BBS dir.\n");
  }
#endif
  if (!syscfg.max_subs)
    syscfg.max_subs=64;
  if (!syscfg.max_dirs)
    syscfg.max_dirs=64;

  if (!syscfg.userreclen)
    syscfg.userreclen=sizeof(olduserrec);

  qfl=4*(1+syscfg.max_subs+((syscfg.max_subs+31)/32)+((syscfg.max_dirs+31)/32));
  syscfg.qscn_len=qfl;

  qsc=(unsigned long *)bbsmalloc(qfl);

  qsc_n=qsc+1;
  qsc_q=qsc_n+((syscfg.max_dirs+31)/32);
  qsc_p=qsc_q+((syscfg.max_subs+31)/32);
}

/****************************************************************************/

void c_old_to_new(void)
{
  int nu,i,i1,f;
  olduserrec old;
  userrec new,u;

  ouf=open(oun,O_RDONLY|O_BINARY);
  if (ouf<0) {
    printf("Couldn't open '%s'\n",oun);
    return;
  }
  nuf=open(nun,O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
  if (nuf<0) {
    close(ouf);
    printf("Couldn't create '%s'\n",nun);
    return;
  }
  qf=open(qfn,O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);
  if (qf<0) {
    close(ouf);
    close(nuf);
    printf("Couldn't create '%s'\n",qfn);
    return;
  }

  nu=filelength(ouf)/syscfg.userreclen;

  for (i=0; i<nu; i++) {
    if (i%10 == 0)
      printf("%u/%u\r",i,nu);

    memset(&old,0,sizeof(old));
    memset(&new,0,sizeof(new));
    memset(qsc,0,qfl);

    if (i) {
      lseek(ouf,((long)i)*((long)syscfg.userreclen), SEEK_SET);
      read(ouf,&old,syscfg.userreclen);

      C_STR(name);
      C_STR(realname);
      C_STR(callsign);
      C_STR(phone);
      C_STR(pw);
      C_STR(laston);
      C_STR(firston);
      C_STR(note);
      for (i1=0; i1<3; i1++)
        C_STR(macros[i1]);
      C_NUM(sex);
      C_NUM(age);
      C_NUM(inact);
      C_NUM(comp_type);
      C_NUM(defprot);
      C_NUM(defed);
      C_NUM(screenchars);
      C_NUM(screenlines);
      C_NUM(sl);
      C_NUM(dsl);
      C_NUM(exempt);
      for (i1=0; i1<8; i1++) {
        C_NUM(colors[i1]);
        C_NUM(bwcolors[i1]);
      }
      new.colors[8]=1;
      new.colors[9]=3;
      new.bwcolors[8]=7;
      new.bwcolors[9]=7;
      for (i1=0; i1<20; i1++)
        C_NUM(votes[i1]);
      C_NUM(illegal);
      C_NUM(waiting);
      C_NUM(ontoday);
      C_NUM(homeuser);
      C_NUM(homesys);
      C_NUM(forwardusr);
      C_NUM(forwardsys);
      C_NUM(msgpost);
      C_NUM(emailsent);
      C_NUM(feedbacksent);
      C_NUM(posttoday);
      C_NUM(etoday);
      C_NUM(ar);
      C_NUM(dar);
      C_NUM(restrict);
      C_NUM(ass_pts);
      C_NUM(uploaded);
      C_NUM(downloaded);
      C_NUM(lastrate);
      C_NUM(logons);
      C_NUM(msgread);
      C_NUM(uk);
      C_NUM(dk);
      C_NUM(daten);
      C_NUM(sysstatus);
      C_NUM(timeontoday);
      C_NUM(extratime);
      C_NUM(timeon);
      C_NUM(pos_account);
      C_NUM(neg_account);
      C_NUM(gold);
      C_NUM(month);
      C_NUM(day);
      C_NUM(year);
      C_NUM(emailnet);
      C_NUM(postnet);
      C_NUM(fsenttoday1);
      C_NUM(num_extended);
      C_NUM(optional_val);
      C_NUM(wwiv_regnum);
      C_NUM(net_num);

      *qsc=old.sysopsub;
      if (*qsc==255)
        *qsc=999;
      qsc_n[0]=old.nscn1;
      qsc_n[1]=old.nscn2;
      qsc_q[0]=old.qscn;
      qsc_q[1]=old.qscn2;

      for (i1=0; i1<32; i1++) {
        qsc_p[i1]=old.qscnptr[i1];
        qsc_p[i1+32]=old.qscnptr2[i1];
      }
    }

    lseek(nuf,((long)i)*((long)sizeof(new)), SEEK_SET);
    write(nuf,&new,sizeof(new));

    lseek(qf,((long)i)*((long)qfl),SEEK_SET);
    write(qf,qsc,qfl);

  }
  close(ouf);
  close(nuf);
  close(qf);

  unlink(sun);
  rename(oun,sun);
  rename(nun,oun);

  syscfg.userreclen=sizeof(u);
  syscfg.waitingoffset=OFFOF(waiting);
  syscfg.inactoffset=OFFOF(inact);
  syscfg.sysstatusoffset=OFFOF(sysstatus);
  syscfg.fuoffset=OFFOF(forwardusr);
  syscfg.fsoffset=OFFOF(forwardsys);
  syscfg.fnoffset=OFFOF(net_num);

  f=open("CONFIG.DAT",O_RDWR|O_BINARY|O_CREAT,S_IREAD|S_IWRITE);
  if (f>0) {
    write(f,&syscfg,sizeof(syscfg));
    close(f);
  }
  printf("\nDone.\n");
}

/****************************************************************************/

int c_check(void)
{
  if (access(qfn,0)==0)
    /* new format */
    return(1);
  else
    /* old format */
    return(0);
}

/****************************************************************************/

int c_check_old_struct(void)
{
  olduserrec u;

  if ((syscfg.userreclen) && (syscfg.userreclen!=sizeof(u)) && (syscfg.userreclen!=700))
    return(1);
  if ((syscfg.waitingoffset) && (syscfg.waitingoffset != OFFOF(waiting)))
    return(1);
  if ((syscfg.inactoffset) && (syscfg.inactoffset != OFFOF(inact)))
    return(1);
  if ((syscfg.sysstatusoffset) && (syscfg.sysstatusoffset != OFFOF(sysstatus)))
    return(1);
  if ((syscfg.fuoffset) && (syscfg.fuoffset != OFFOF(forwardusr)))
    return(1);
  if ((syscfg.fsoffset) && (syscfg.fsoffset != OFFOF(forwardsys)))
    return(1);
  if ((syscfg.fnoffset) && (syscfg.fnoffset != OFFOF(net_num)))
    return(1);

  return(0);
}

/****************************************************************************/

#ifndef INIT
void main()
{
  c_setup();

  if (c_check()==1) {
    /* convert back to old format */
    printf("Your userrec is already in the new format.\n");
  } else {
    /* convert to new format */
    if (c_check_old_struct()) {
      printf("Your current userrec format is unknown.\n");
      printf("Please copy your old userrec format into convert.c,\n");
      printf("and compile and run it.\n");
    } else {
      c_old_to_new();
      printf("Userlist converted.\n");
    }
  }
}
#endif
