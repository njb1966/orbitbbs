/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/



/*
 * Possible future enhancements:
 *
 * duplicate users?
 * modem stuff ok
 * check for various .dat files existence
 * decrement mail waiting
 */



#pragma warn -par
void sysoplog(char *s) {}
void giveup_timeslice(void) {}
void read_status(void) {}
#pragma warn +par

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <sys\stat.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <alloc.h>

#ifdef __OS2__
#define huge
#endif /* __OS2__ */

#define EXTENDED
#include "vardec.h"
#include "share.h"
#include "share.c"

#define OK  "   "
#define NOK "## "
#define QOK "?? "

void fix_user(void);
int trashed_user(void);
int open_file(char *fn);
void remove_link(messagerec *m1, char *aux);

int userfile=-1,configfile=-1,statusfile=-1;
int force_yes=0,no_usercheck=0;
statusrec status;
configrec syscfg;
char *thisuser;
char *thisuser_inact;
char cdir[81];
int num_dirs=0,num_subs=0;
directoryrec *directories;
subboardrec *subboards;
unsigned long max_qscan;
int curlsub=-1,nummsgs;
short gat[2048];
int gat_section;
int url;
int debuglevel=0;
int incom=0;                                     /* keep share.c happy */
int subchg=0;
messagerec *emailm;
char *emailmmm;
int numemailm,num_type_0;

smalrec *smallist,*new_smallist;
int cur_users;


extern unsigned int wwiv_num_version;
#define NOT_BBS 0
#define malloca(x) bbsmalloc(x)
#define MAX_TO_CACHE 163
#include "subacc.c"


int allow_changes=1,force_changes=0;

/****************************************************************************/

void read_user(unsigned int un)
{
  long pos;
  char s[80];

  if (userfile==-1) {
    sprintf(s,"%sUSER.LST",syscfg.datadir);
    userfile=sh_open1(s,O_RDWR | O_BINARY);
    if (userfile<0) {
      *thisuser_inact=inact_deleted;
      return;
    }
  }

  pos=((long) url) * ((long) un);
  if (filelength(userfile)<(pos+syscfg.userreclen)) {
    *thisuser_inact=inact_deleted;
    return;
  }

  lseek(userfile,pos,SEEK_SET);
  read(userfile, thisuser, syscfg.userreclen);
}

/****************************************************************************/

void write_user(unsigned int un)
{
  long pos;
  char s[80];

  if (userfile==-1) {
    sprintf(s,"%sUSER.LST",syscfg.datadir);
    userfile=sh_open1(s,O_RDWR | O_BINARY);
    if (userfile<0) {
      return;
    }
  }

  pos=((long) url) * ((long) un);
  if (filelength(userfile)<(pos+syscfg.userreclen)) {
    return;
  }

  lseek(userfile,pos,SEEK_SET);
  write(userfile, thisuser, syscfg.userreclen);
}

/****************************************************************************/


void close_user(void)
{
  if (userfile!=-1) {
    sh_close(userfile);
    userfile=-1;
  }
}

/****************************************************************************/

void save_status(void)
{
  char s[80];

  sprintf(s,"%sSTATUS.DAT",syscfg.datadir);
  statusfile=sh_open(s,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  write(statusfile, (void *)(&status), sizeof(statusrec));
  sh_close(statusfile);
  statusfile=-1;
}

/****************************************************************************/

int exist(char *s)
{
  int f;

  f=sh_open1(s,O_RDONLY | O_BINARY);
  sh_close(f);
  if (f>0)
    return(1);
  else
    return(0);
}

/****************************************************************************/

void give_up(void)
{
  printf("\n%sGiving up.\n",NOK);
  exit(-1);
}


/****************************************************************************/

void oom(void *x, char *where, long l)
{
  if (!x) {
    printf("%sOut of memory for %s (%ld bytes).\n",NOK, where, l);
    give_up();
  }
}

/****************************************************************************/

int yn(void)
{
  char ch;

  if (force_yes) {
    printf("Yes\n");
    return(1);
  }

  while (1) {
    ch=getch();
    if ((ch=='Y') || (ch=='y')) {
      printf("Yes\n");
      return(1);
    }
    if ((ch=='N') || (ch=='n') || (ch==13)) {
      printf("No\n");
      return(0);
    }
  }
}


/****************************************************************************/

void maybe_give_up(void)
{
  printf("%sFuture expansion might try to fix this problem.\n",OK);
  give_up();
}

/****************************************************************************/

int mdto(char *s)
{
  char s1[81];
  int i,db,st;

  strcpy(s1,s);
  i=strlen(s1)-1;
  db=(s1[i]=='\\');
  if (i==0)
    db=0;
  if ((i==2) && (s1[1]==':'))
    db=0;
  if (db)
    s1[i]=0;
  st=mkdir(s1);
  if (s[1]==':') {
    i=setdisk(s[0]-'A');
    if (i<=(s[0]-'A'))
      st=1;
  }
  return(st);
}


int cdto(char *s)
{
  char s1[81];
  int i,db,st;

  strcpy(s1,s);
  i=strlen(s1)-1;
  db=(s1[i]=='\\');
  if (i==0)
    db=0;
  if ((i==2) && (s1[1]==':'))
    db=0;
  if (db)
    s1[i]=0;
  st=chdir(s1);
  if (s[1]==':') {
    i=setdisk(s[0]-'A');
    if (i<=(s[0]-'A'))
      st=1;
  }
  return(st);
}

/****************************************************************************/


void get_dir(char *s, int be)
{
  strcpy(s,"X:\\");
  s[0]='A'+getdisk();
  getcurdir(0,&(s[3]));
  if (be) {
    if (s[strlen(s)-1]!='\\')
      strcat(s,"\\");
  }
}


/****************************************************************************/

int check_dir(char *dir, char *descr)
{
  int st;

  st=cdto(dir);
  if (st) {
    printf("\n%sUnable to find dir '%s' \n   for %s dir, \n   CREATE it (y/N)? ",
      NOK,dir,descr);
    if(yn()) {
      st=mdto(dir);
      if (st)
        printf("%sUnable to create dir '%s' for %s dir.\n",NOK,dir,descr);
    } else {

    }
  }
  cdto(cdir);
  return(st);
}

/****************************************************************************/


void check_all_dirs(void)
{
  int st=0,st1;
  int i;

  st |= check_dir(syscfg.msgsdir,"Messages");
  st |= check_dir(syscfg.gfilesdir,"G-files");
  st |= check_dir(syscfg.datadir,"Data");
  st |= check_dir(syscfg.dloadsdir,"Default Dloads");
  st |= check_dir(syscfg.tempdir,"Temp");

  if (st) {
    printf("%sOne of the critical system directories is not present or not set correctly.\n",NOK);
    give_up();
  }

  for (i=0; i<num_dirs; i++) {
    if (!(directories[i].mask & mask_cdrom)) {
      st1 = check_dir(directories[i].path,directories[i].name);
      if (st1) {
        st=1;
        printf("%sEither create the dir, or use //DIREDIT to set it correctly.\n",NOK);
        printf("%sThis program cannot fix this problem.\n",NOK);
      }
    }
  }
}



/****************************************************************************/

int trashed_str(char *s, int ml, int lower)
{
  int i,i1,st=0;

  i1=strlen(s);
  if (i1>ml)
    st=1;
  else
    for (i=0; i<i1; i++) {
      if (s[i]<32)
        st=1;
      if ((!lower) && (islower(s[i])))
        st=1;
    }
  return(st);
}

/****************************************************************************/


int trashed_user()
{
  if (trashed_str(thisuser,30,0))
    return(1);

  return(0);
}

void fix_user()
{
  thisuser[30]=0;
}


/****************************************************************************/


void check_userlist(void)
{
  long l;
  int nu,check_trash=0,trashed_userlist=0,xxx;

  url=syscfg.userreclen;
  if (!url) {
    url=700;
    syscfg.userreclen=700;
  }

  read_user(1);
  if (userfile<0) {
    printf("%sNo userlist present.\n",NOK);
    give_up();
  }

  l=filelength(userfile);
  nu=(int) (l/((long)url)) - 1;

  printf("%sFound %d user records... ",OK,nu);

  if (nu>syscfg.maxusers) {
    printf("Might be too many.\n");
    check_trash=1;
  } else
    printf("Reasonable number.\n");

  if (check_trash) {
    read_user(nu);
    if ((xxx=trashed_user())!=0) {
      trashed_userlist=1;
      printf("%sUserrec #%u appeares trashed (%d).\n",NOK,nu,xxx);
    }

    if (trashed_userlist) {
      printf("%sUserlist appears screwed up.\n",NOK);
    } else
      printf("%sUserlist seems OK.\n",OK);
  }

  if (trashed_userlist) {
    if (!allow_changes)
      give_up();

    printf("%sScanning for invalid user records...\n",OK);

    while ((nu) && (trashed_user()))
      read_user(--nu);

    printf("%sAppears to be %u 'real' user records.\n",OK,nu);

    if (!force_changes) {
      printf("%sTruncate userlist to %u users? ",QOK,nu);
      if (!yn())
        trashed_userlist=0;
    }

    if (trashed_userlist) {
      l=((long) nu) * ((long) syscfg.userreclen);
      chsize(userfile,l);
    }
  }
}


/****************************************************************************/

void isr1(int un, char *name)
{
  int cp;
  smalrec sr;

  cp=0;
  while ((cp<cur_users) && (strcmp(name,(new_smallist[cp].name))>0))
    ++cp;
  memmove(&(new_smallist[cp+1]),&(new_smallist[cp]),sizeof(smalrec)*(cur_users-cp));
  strcpy(sr.name,name);
  sr.number=un;
  new_smallist[cp]=sr;
  ++cur_users;
}

/****************************************************************************/


void check_nameslist(void)
{
  char s[81];
  int st=0,i,i1,i2,xxx;
  unsigned int status_users,names_users;

  printf("%sScanning NAMES.LST file...\n",OK);
  sprintf(s,"%sNAMES.LST",syscfg.datadir);
  i=sh_open1(s,O_RDONLY | O_BINARY);
  if (i>0) {
    if (filelength(i)) {
      smallist=(smalrec *)bbsmalloc(filelength(i)+1);
      oom(smallist, "smallist", filelength(i));
      read(i,smallist,(unsigned) filelength(i));
      names_users=(unsigned int) (filelength(i)/sizeof(smalrec));
      status_users=status.users;
    } else
      names_users=0;
    sh_close(i);

    if (names_users!=status_users) {
      status.users=names_users;
      printf("%sstatus.users was wrong.  ",NOK);
      if (allow_changes) {
        printf("Fixed it.\n");
        save_status();
      } else {
        printf("Leaving it alone.\n");
      }
    }
  } else {
    names_users=0;
    status.users=status.users;
    printf("%s%s NOT FOUND.\n",NOK,s);
  }

  cur_users=0;

  i1=filelength(userfile)/syscfg.userreclen-1;

  new_smallist=(smalrec *)bbsmalloc(((long)i1+1) * ((long)sizeof(smalrec)));
  oom(new_smallist,"new_smallist",((long)i1) * ((long)sizeof(smalrec)));

  for (i=1; i<=i1; i++) {
    read_user(i);
    if ((*thisuser_inact & inact_deleted)==0) {
      if ((xxx=trashed_user())!=0) {
#ifdef NOT_WORTH_IT
        printf("\n%s User #%d had trashed info (%d); patching.\n",NOK,i,xxx);
        fix_user();
        write_user(i);
#else
        i2=xxx;
        xxx=i2;
#endif
      }
      isr1(i,thisuser);
    }
    if ((i % 10)==0)
      printf("%s%d/%d\r",OK,i,i1);
  }
  printf("%s%d/%d\n",OK,i1,i1);

  if (cur_users!=names_users) {
    st=1;
    printf("%sA different number of users was found (%d vs %d).\n",NOK,
        cur_users,names_users);

  } else {
    for (i=0; i<names_users; i++) {
      if (strcmp(smallist[i].name,new_smallist[i].name))
        st=1;
      if (smallist[i].number!=new_smallist[i].number)
        st=1;
    }
  }
  if (st) {
    printf("%sThe new NAMES.LST file is different than the old one.\n",NOK);
    if (allow_changes) {
      status.users=cur_users;
      i=sh_open(s,O_RDWR | O_BINARY | O_TRUNC | O_CREAT, S_IREAD | S_IWRITE);
      if (i<0) {
        printf("%sCouldn't create %s.\n",NOK,s);
        give_up();
      }
      write(i,(void *) (new_smallist), (sizeof(smalrec) * status.users));
      sh_close(i);

      save_status();
      printf("%sNAMES.LST file fixed.\n",OK);
    } else {
      printf("%sLeaving it alone, but it should be fixed.  There is nothing bad that can\n",NOK);
      printf("%shappen by fixing the NAMES.LST file.\n",NOK);
    }
  }
  if (smallist) {
    bbsfree(smallist);
    smallist=NULL;
  }
  if (new_smallist) {
    bbsfree(new_smallist);
    new_smallist=NULL;
  }
}

/****************************************************************************/

#define GATSECLEN (4096L+2048L*512L)
#define MSG_STARTING (((long)gat_section)*GATSECLEN + 4096)


void set_gat_section(int f, int section)
{
  long l,l1;
  int i;

  if (gat_section!=section) {
    l=filelength(f);
    l1=((long)section)*GATSECLEN;
    if (l<l1) {
      chsize(f,l1);
      l=l1;
    }
    lseek(f,l1,SEEK_SET);
    if (l<(l1+4096)) {
      for (i=0; i<2048; i++)
        gat[i]=0;
      write(f,(void *)gat, 4096);
    } else {
      read(f,(void *)gat, 4096);
    }
    gat_section=section;
  }
}

/****************************************************************************/


void save_gat(int f)
{
  long l;

  l=((long)gat_section)*GATSECLEN;
  lseek(f,l,SEEK_SET);
  write(f,(void *)gat,4096);
}

/****************************************************************************/



void remove_link(messagerec *m1, char *aux)
{
  messagerec m;
  char s[81],s1[81];
  int f;
  long csec,nsec;

  m=*m1;
  strcpy(s,syscfg.msgsdir);
  switch(m.storage_type) {
    case 0:
    case 1:
      ltoa(m.stored_as,s1,16);
      if (m.storage_type==1) {
        strcat(s,aux);
        strcat(s,"\\");
      }
      strcat(s,s1);
      unlink(s);
      break;
    case 2:
      f=open_file(aux);
      if (f>=0) {
        set_gat_section(f,(int) (m.stored_as/2048));
        csec=m.stored_as % 2048;
        while ((csec>0) && (csec<2048)) {
          nsec=(long) gat[csec];
          gat[csec]=0;
          csec=nsec;
        }
        save_gat(f);
        sh_close(f);
      }
      break;
    default:
      /* illegal storage type */
      break;
  }
}

/****************************************************************************/


void delmail(int f, int loc)
{
  mailrec m,m1;
  int rm,i,t,otf;
  char s[81];

  sprintf(s,"%sEMAIL.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDWR | O_BINARY);


  lseek(f,((long) loc) * ((long) sizeof(mailrec)), SEEK_SET);
  read(f,(void *)&m,sizeof(mailrec));

  rm=1;
  if (m.status & status_multimail) {
    t=filelength(f)/sizeof(mailrec);
    otf=0;
    for (i=0; i<t; i++)
      if (i!=loc) {
        lseek(f,((long)i)*((long)sizeof(mailrec)),SEEK_SET);
        read(f,(void *)&m1,sizeof(mailrec));
        if ((m.msg.stored_as==m1.msg.stored_as) && (m.msg.storage_type==m1.msg.storage_type) && (m1.daten!=0xffffffff))
          otf=1;
      }
    if (otf)
      rm=0;
  }

  sh_close(f);
  if (rm)
    remove_link(&m.msg,"EMAIL");

  if (m.tosys==0) {
#ifdef DEC_MAIL_WAITING
    read_user(m.touser,&u);
    if (u.waiting) {
      --u.waiting;
      write_user(m.touser,&u);
      close_user();
    }
#endif
  }

  f=sh_open1(s,O_RDWR | O_BINARY);
  lseek(f,((long) loc) * ((long) sizeof(mailrec)), SEEK_SET);
  m.touser=0;
  m.tosys=0;
  m.daten=0xffffffff;
  m.msg.storage_type=0;
  m.msg.stored_as=0xffffffff;
  write(f,(void *)&m,sizeof(mailrec));
  sh_close(f);
}

/****************************************************************************/

void find_max_qscan(void)
{
  int i,cs,r,t,f,any_screwed, here_screwed;
  unsigned long maxthissub;
  mailrec m;
  char s[81];
  postrec *p;

  max_qscan=0L;
  any_screwed=0;
  num_type_0=0;

  printf("%sScanning subboards...\n",OK);

  for (cs=0; cs<num_subs; cs++) {
    iscan1(cs,0);
    open_sub(0);
    maxthissub=0L;
    here_screwed=0;
    if (nummsgs) {
      for (i=1; i<=nummsgs; i++) {
        p=get_post(i);
        if (maxthissub>=p->qscan) {
          any_screwed=1;
          here_screwed=1;
        }
        if (maxthissub<p->qscan)
          maxthissub=p->qscan;
        if (max_qscan<p->qscan)
          max_qscan=p->qscan;
        if (p->msg.storage_type<2) {
          /* if (max_qscan<p->msg.stored_as)
            max_qscan=p->msg.stored_as; */
          if (p->msg.storage_type==0)
            ++num_type_0;
        }
      }
      if (here_screwed) {
        printf("%sSub '%s' has q-scan pointers screwed up.\n",NOK,subboards[cs].name);
      }
    }
    close_sub();
  }

  printf("%sScanning EMAIL.DAT file...\n",OK);

  sprintf(s,"%sEMAIL.DAT",syscfg.datadir);
  f=sh_open1(s,O_BINARY | O_RDWR);
  if (f!=-1) {
    t=(int) (filelength(f)/sizeof(mailrec));
    r=0;
    numemailm=t;
    emailm=(messagerec *)bbsmalloc((t+1)*sizeof(messagerec));
    emailmmm=(char *)bbsmalloc(t+1);
    if (t) {
      oom(emailm,"emailm",t*sizeof(messagerec));
      oom(emailmmm,"emailmmm",t);
    }
    while (r<t) {
      lseek(f,(long)(sizeof(mailrec)) * (long)(r),SEEK_SET);
      read(f,(void *)&m,sizeof(mailrec));
      emailm[r]=m.msg;
      emailmmm[r]=m.status;
      if ((m.tosys!=0) || (m.touser!=0)) {
        if (m.msg.storage_type<2)
          /* if (max_qscan<m.msg.stored_as)
            max_qscan=m.msg.stored_as; */
        if (m.msg.storage_type==0)
          ++num_type_0;
      }
      ++r;
    }
    sh_close(f);
  }
  max_qscan++;
  if (max_qscan>status.qscanptr) {
    printf("%sMax qscan pointer trashed (%lu vs %lu).\n",NOK,max_qscan,status.qscanptr);
    status.qscanptr=max_qscan;
    if (allow_changes) {
      save_status();
      printf("%sFixed.\n",OK);
    } else
      printf("%sLeaving as-is, but it should be fixed.\n",OK);
  }

  if (max_qscan>=0x80000000) {
    printf("%sMax qscan pointer indicates qscan troubles (too big).\n", NOK);
    any_screwed=1;
  }

  if ((allow_changes) && (any_screwed)) {
    if (!force_changes) {
      printf("%sReset all qscan pointers on BBS? ",QOK);
      if (!yn())
        any_screwed=0;
    }
    if (any_screwed) {
      printf("%sResetting qscan pointers... do //resetqscan from mainmenu also.\n",OK);
      max_qscan=1;
      status.qscanptr=max_qscan;
      for (cs=0; cs<num_subs; cs++) {
        if (iscan1(cs,0)) {
          open_sub(1);
          for (i=1; i<=nummsgs; i++) {
            p=get_post(i);
            if (p) {
              p->qscan=status.qscanptr++;
              write_post(i,p);
            }
          }
          close_sub();
        }
      }
      save_status();
    }
  }
}


/****************************************************************************/


typedef struct {
  long    stored_as;
  int     subnum,msgnum;
} type_0;


/****************************************************************************/


char *describem(char *s, int subnum, int msgnum)
{
  mailrec me;
  postrec *mp;
  char s1[81];
  int f;

  if (subnum==-1) { /* email */
    sprintf(s1,"%sEMAIL.DAT",syscfg.datadir);
    f=sh_open1(s1,O_BINARY | O_RDWR);
    lseek(f,(long)(sizeof(mailrec)) * (long)(msgnum),SEEK_SET);
    read(f,(void *)&me,sizeof(mailrec));
    sh_close(f);

    if (me.fromsys)
      sprintf(s1,"user #%d @%d",me.fromuser,me.fromsys);
    else
      sprintf(s1,"user #%d",me.fromuser);

    sprintf(s,"Mail from %s to ",s1);

    if (me.tosys)
      sprintf(s1,"user #%d @%d",me.touser,me.tosys);
    else
      sprintf(s1,"user #%d",me.touser);

    strcat(s,s1);

    sprintf(s1," '%.30s'",me.title);
    strcat(s,s1);

  } else {
    iscan1(subnum,0);
    mp=get_post(msgnum);
    if (mp) {
      if (mp->ownersys)
        sprintf(s,"Post #%d on '%s' by user #%d @%d",
            msgnum+1,
            subboards[subnum].name,
            mp->owneruser,
            mp->ownersys);
      else
        sprintf(s,"Post #%d on '%s' by user #%d",
            msgnum+1,
            subboards[subnum].name,
            mp->owneruser);
    } else {
      sprintf(s, "Unreadable (?) post #%d on '%s'",
            msgnum+1, subboards[subnum].name);
    }
  }
  return(s);
}


/****************************************************************************/


void check_type_1(int num, messagerec *list, char *extra, int todesc)
{
  char s[81],s1[81];
  int i,i1,dup,ex;
  char *already_done;

  if (!num)
    return;

  sprintf(s,"%s%s\\",syscfg.msgsdir,extra);
  already_done=(char *)bbsmalloc(num);
  sprintf(s1,"already_done(%s)",extra);
  oom(already_done,s1,num);

  for (i=0; i<num; i++)
    already_done[i]=0;

  for (i=0; i<num; i++)
    if ((list[i].storage_type==1) && (!already_done[i])) {
      sprintf(s1,"%s%lX",s,list[i].stored_as);
      ex=exist(s1);
      dup=0;
      if (!((todesc==-1) && (emailmmm[i] & status_multimail))) {
        for (i1=i+1; i1<num; i1++)
          if (list[i1].storage_type==1)
            if (list[i1].stored_as==list[i].stored_as)
              dup=1;
      }
      if (dup || (!ex)) {
        if (!ex)
          printf("%sFile not found: '%s'\n",NOK,s1);
        if (dup)
          printf("%sMessage file multiply referenced: '%s'\n",NOK,s1);
        printf("%s  for %s\n",NOK,describem(s1,todesc,i));

        for (i1=i+1; i1<num; i1++)
          if ((list[i1].storage_type==1) &&
              (list[i1].stored_as==list[i].stored_as)) {
            printf("%s  for %s\n",NOK,describem(s1,todesc,i1));
            already_done[i1]=1;
          }
        printf("%s  No action taken.\n\n",NOK);
      }
    }
  bbsfree(already_done);
}

/****************************************************************************/


int open_file(char *fn)
{
  int f;
  char s[81];

  sprintf(s,"%s%s.DAT",syscfg.msgsdir,fn);
  f=sh_open1(s,O_RDWR | O_BINARY);
  if (f<0)
    return(-1);

  lseek(f,0L,SEEK_SET);
  read(f,(void *)gat,4096);
  gat_section=0;
  return(f);
}


/****************************************************************************/


void check_type_2(int num, messagerec *list, char *extra, int todesc)
{
  int f,i,any,csec,csec1,any_dead,num_deadm,anything_done,numsec,sec;
  char s[81];
  int *deadm;
  short huge *gati, huge *gatint;
  long high;

  if (!num)
    return;

  f=open_file(extra);
  numsec=((int) (filelength(f)/GATSECLEN))+1;
  high=((long)numsec)*2048;

  gati=(short *)bbsmalloc(high*2+1);
  sprintf(s,"gati(%s)",extra);
  oom((void *)gati,s,high*2);
  gatint=(short *)bbsmalloc(high*2+1);
  sprintf(s,"gatint(%s)",extra);
  oom((void *)gatint,s,high*2);


  for (i=0; i<high; i++)
    gati[i]=0;

  for (i=0; i<numsec; i++) {
    set_gat_section(f,i);
    memcpy((char *) (gatint+2048*i), gat, 4096);
  }

  deadm=(int *)bbsmalloc(num*2);
  sprintf(s,"deadm(%s)",extra);
  oom(deadm,s,num*2);

  any=0;
  any_dead=0;
  num_deadm=0;
  anything_done=0;

  for (i=0; i<num; i++)
    if (list[i].storage_type==2) {
      if (f<0) {
        if (!any) {
          any=1;
          printf("%sType 2 data file not found: '%s%s.DAT'\n",
                NOK,syscfg.msgsdir,extra);
        }
        deadm[num_deadm++]=i;
        anything_done=1;
      } else {
        sec=(list[i].stored_as/2048)*2048;
        csec=list[i].stored_as % 2048;
        while ((csec>0) && (csec<2048)) {
          if (gati[csec+sec]) {
            if (!((todesc==-1) && (emailmmm[i] & status_multimail))) {
              if (gati[csec+sec]!=-2)
                gati[csec+sec]=-1;
              ++any_dead;
            }
            csec=-1;
          } else {
            if (!gatint[csec+sec]) {
              gati[csec+sec]=-2;
              ++any_dead;
              csec=-1;
            } else {
              gati[csec+sec]=i+1;
              csec=gatint[csec+sec];
            }
          }
        }
      }
    }

  if (f<0) {
    if (anything_done) {

      printf("%sDunno what to do about that.\n",NOK);

    }
    bbsfree((void *)gati);
    bbsfree((void *)deadm);
    bbsfree((void *)gatint);
    sh_close(f);
    return;
  }

  if (any_dead) {
    anything_done=1;
    printf("%sErrors in '%s%s.DAT':\n",NOK,syscfg.msgsdir,extra);
    for (i=0; i<num; i++)
      if (list[i].storage_type==2) {
        sec=(list[i].stored_as/2048)*2048;
        csec=list[i].stored_as % 2048;
        csec1=-1;
        while ((csec>0) && (csec<2048)) {
          if (gati[csec+sec]<0) {
            printf("%s  for %s\n",NOK,describem(s,todesc,i));
            if (gati[csec+sec]==-1) {
              printf("%s    Collided on cluster #%d\n",NOK,csec+sec);
              if (csec1==-1) {
                printf("%s    First cluster of message, removing.\n",NOK);
                deadm[num_deadm++]=i;
              } else {
                gatint[csec1+sec]=-1;
                printf("%s    Truncating message.\n",NOK);
              }
            } else if (gati[csec+sec]==-2) {
              printf("%s    Pointed to unallocated cluster #%d\n",NOK,csec+sec);
              if (csec1==-1) {
                printf("%s    First cluster of message, removing.\n",NOK);
                deadm[num_deadm++]=i;
              } else {
                gatint[csec1+sec]=-1;
                printf("%s    Truncating message.\n",NOK);
              }
            } else {
              printf("%s    Unknown error.\n",NOK);
            }
          }
          csec1=csec;
          csec=gatint[csec+sec];
        }
      }

  }

  any_dead=0;
  for (csec=0; csec<numsec; csec++)
    if ((gatint[csec]) && (gati[csec]<=0)) {
      gatint[csec]=0;
      ++any_dead;
    }
  if (any_dead) {
    printf("%sLost clusters recovered from '%s%s.DAT': %d\n",
          NOK,syscfg.msgsdir,extra,any_dead);
    anything_done=1;
  }

  if (anything_done) {
    i=force_changes;
    if ((allow_changes) && (!i)) {
      printf("%sWrite these changes to disk? ",QOK);
      i=yn();
    }


    if (i) {
      printf("%sUpdating '%s%s.DAT'.\n",OK,syscfg.msgsdir,extra);
      for (i=0; i<numsec; i++) {
        set_gat_section(f,i);
        memcpy(gat, (char *) (gatint+2048*i), 4096);
        save_gat(f);
      }
      if (num_deadm) {
        printf("%sRemoving messages with no text.\n",OK);
        if (todesc!=-1) {
          iscan1(todesc,0);

          for (i=0; i<num_deadm; i++)
            delete(deadm[i]-i+1);

        } else {
          for (i=0; i<num_deadm; i++)
            delmail(f,deadm[i]);

        }
      }
    }
  }
  sh_close(f);
  bbsfree((void *)deadm);
  bbsfree((void *)gati);
  bbsfree((void *)gatint);
}


/****************************************************************************/


void check_msg_consistency(void)
{
  type_0 *x0;
  int cs,i,i1,i2,cp,ex;
  char s[81],s1[81];
  postrec *p;
  messagerec *pm;

  /* check type 0 consistency */

  if (num_type_0) {
    printf("%sChecking type 0 message consistency.\n",OK);
    x0=(type_0 *)bbsmalloc(sizeof(type_0)*num_type_0);
    oom(x0,"x0",sizeof(type_0)*num_type_0);
    cp=0;
    for (cs=0; cs<num_subs; cs++) {
      iscan1(cs, 0);
      open_sub(0);
      for (i=1; i<=nummsgs; i++) {
        p=get_post(i);
        if ((p->msg.storage_type==0) && (p->msg.stored_as!=(unsigned long)-1)) {
          x0[cp].stored_as=p->msg.stored_as;
          x0[cp].subnum=cs;
          x0[cp].msgnum=i+1;
          ++cp;
        }
      }
      close_sub();
    }
    for (cs=0; cs<numemailm; cs++)
      if ((!emailm[cs].storage_type) && (emailm[cs].stored_as != (unsigned long)-1)) {
        x0[cp].stored_as=emailm[cs].stored_as;
        x0[cp].subnum=-1;
        x0[cp].msgnum=cs;
        ++cp;
      }

    for (i=0; i<cp; i++) {
      if (x0[i].subnum!=-2) {
        sprintf(s,"%s%lX",syscfg.msgsdir,x0[i].stored_as);
        ex=exist(s);

        i2=0;

        for (i1=i+1; i1<cp; i1++)
          if (x0[i].stored_as==x0[i1].stored_as)
            i2=1;

        if (i2 || (!ex)) {
          if (!ex)
            printf("%sFile not found: '%s'\n",NOK,s);
          if (i2)
            printf("%sMessage file multiply referenced: '%s'\n",NOK,s);
          printf("%s  for %s\n",NOK,describem(s1,x0[i].subnum,x0[i].msgnum));
          for (i1=i+1; i1<cp; i1++)
            if (x0[i].stored_as==x0[i1].stored_as) {
              printf("%s  for %s\n",NOK,describem(s1,x0[i1].subnum,x0[i1].msgnum));
              x0[i1].subnum=-2;
            }
          printf("%s  No action taken.\n\n",NOK);
        }
      }
    }

    bbsfree(x0);
  }

  /* check type 1&2 consistency */

  printf("%sChecking type 1&2 message consistency.\n",OK);
  check_type_1(numemailm,emailm,"EMAIL",-1);
  check_type_2(numemailm,emailm,"EMAIL",-1);
  for (cs=0; cs<num_subs; cs++) {
    iscan1(cs, 0);
    if (nummsgs) {
      pm=(messagerec *)bbsmalloc(nummsgs*sizeof(messagerec));
      open_sub(1);
      sprintf(s,"pm(%s)",subboards[cs].filename);
      oom(pm,s,nummsgs*sizeof(messagerec));
      for (i=1; i<=nummsgs; i++) {
        pm[i-1]=get_post(i)->msg;
      }
      check_type_1(nummsgs,pm,subboards[cs].filename,cs);
      check_type_2(nummsgs,pm,subboards[cs].filename,cs);
      close_sub();
      bbsfree(pm);
    }
  }
}


/****************************************************************************/


void ck_size(char *fn, int f, long rl)
{
  long l;

  l=filelength(f);
  if (l<rl) {
    printf("%s%s too short (%ld<%ld).\n",NOK,fn,l,rl);
    give_up();
  }

  if (l>rl) {
    printf("%s%s too long (%ld>%ld).\n",NOK,fn,l,rl);
    if (allow_changes)
      printf("%sAttempting to continue.\n",NOK);
    else
      give_up();
  }
}

/****************************************************************************/

void init(void)
{
  char s[81];
  int i;

  strcpy(s,"CONFIG.DAT");
  configfile=sh_open1(s,O_RDWR | O_BINARY);
  if (configfile<0) {
    printf("%s%s NOT FOUND.\n",NOK,s);
    give_up();
  }

  ck_size(s,configfile,sizeof(configrec));

  read(configfile,(void *) (&syscfg), sizeof(configrec));
  sh_close(configfile);

  strcpy(cdir,"X:\\");
  cdir[0]='A'+getdisk();
  getcurdir(0,&(cdir[3]));

  if (check_dir(syscfg.datadir,"Data")) {
    printf("%sMust find data directory to continue.\n",NOK);
    give_up();
  }

  if (!syscfg.userreclen)
    syscfg.userreclen=700;

  thisuser=(char *)bbsmalloc(syscfg.userreclen+1);
  if (!thisuser) {
    printf("%sCould not allocate %d bytes for userrec.\n",NOK,syscfg.userreclen);
    give_up();
  }
  thisuser_inact=thisuser+syscfg.inactoffset;

  sprintf(s,"%sSTATUS.DAT",syscfg.datadir);
  statusfile=sh_open1(s,O_RDWR | O_BINARY);
  if (statusfile<0) {
    printf("%s%s NOT FOUND.\n",NOK,s);
    if (allow_changes) {
      printf("%sRe-creating STATUS.DAT file.\n",OK);
      strcpy(status.date1,"00/00/00");
      strcpy(status.date2,status.date1);
      strcpy(status.date3,status.date1);
      strcpy(status.log1,"000000.LOG");
      strcpy(status.log2,status.log1);
      strcpy(status.gfiledate,"00/00/00");
      status.users=0;
      status.callernum=65535;
      status.callstoday=0;
      status.msgposttoday=0;
      status.localposts=0;
      status.emailtoday=0;
      status.fbacktoday=0;
      status.uptoday=0;
      status.activetoday=0;
      status.qscanptr=0L;
      status.amsganon=0;
      status.amsguser=0;
      status.callernum1=0L;
      status.net_edit_stuff=0;
      status.wwiv_version=0;
      status.net_version=0;
      status.net_bias=0.001;
    } else
      give_up();
  } else {

    ck_size(s,statusfile,sizeof(statusrec));
    read(statusfile,(void *)(&status), sizeof(statusrec));
    sh_close(statusfile);

    if (status.wwiv_version > wwiv_num_version) {
      printf("%sYou need a newer version of fix (this is for %d, you need %d)\n",
            NOK, wwiv_num_version, status.wwiv_version);
      give_up();
    }

  }


  sprintf(s,"%sDIRS.DAT",syscfg.datadir);
  i=sh_open1(s,O_RDWR | O_BINARY);
  if (i<0) {
    printf("%s%s NOT FOUND.\n",NOK,s);
    maybe_give_up();
  } else {
    directories=(directoryrec *)bbsmalloc(filelength(i)+1);
    if (!directories) {
      printf("%sCouldn't allocate %ld bytes for %s.\n",NOK,filelength(i), s);
      give_up();
    }
    num_dirs=(read(i,directories, (filelength(i))))/
              sizeof(directoryrec);
    sh_close(i);
  }

  sprintf(s,"%sSUBS.DAT",syscfg.datadir);
  i=sh_open1(s,O_RDWR | O_BINARY);
  if (i<0) {
    printf("%s%s NOT FOUND.\n",NOK,s);
    maybe_give_up();
  } else {
    subboards=(subboardrec *)bbsmalloc(filelength(i)+1);
    if (!subboards) {
      printf("%sCouldn't allocate %ld bytes for %s.\n",NOK,filelength(i),s);
      give_up();
    }
    num_subs=(read(i, subboards, (filelength(i))))/
             sizeof(subboardrec);
    sh_close(i);

  }
}

/****************************************************************************/


void main(int argc, char *argv[])
{
  int i;
  char *ss;

  for (i=1; i<argc; i++) {
    ss=argv[i];
    if ((*ss=='/') || (*ss=='-')) {
      switch(toupper(ss[1])) {
        case 'Y': force_yes=1; break;
        case 'U': no_usercheck=1; break;
      }
    } else {
      printf("%sUnknown argument: '%s'\n",NOK,ss);
    }
  }

  printf("\n");
  printf("Fix - Fix OrbitBBS files.\n");
  printf("\n");

  if (getenv("BBS")) {
    printf("Fix should only be run OUTSIDE the BBS.\n");
    exit(-1);
  }

  init();

  check_all_dirs();
  check_userlist();
  if (!no_usercheck)
    check_nameslist();
  find_max_qscan();
  check_msg_consistency();

}


