#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <stdlib.h>
#include <ctype.h>

#define v407                   /* Sets for WWIV v4.07 or later */

int debuglevel=0;

int incom;
unsigned int  usernum,         /* user number for the user */
              system_number=0, /* WWIV Node Number - 0=not networked */
              okansi,          /* non-zero if user can support ANSI */
              screenchars,     /* chars/line user has specified */
              screenlines,     /* lines/screen user has specified */
              sl,              /* sec lev for user (0-255) */
              so,              /* non-zero if user is sysop (sl=255) */
              cs,              /* non-zero if user is co-sysop */
              age,             /* age of the user */
              uploaded=0,      /* # files uploaded */
              downloaded=0,    /* # files downloaded */
              comport,         /* com port user is on */
              com_speed=0,     /* com port baud rate */
              modem_speed=0;   /* modem baud rate */
unsigned char name[31],        /* name/alias of user */
              realname[31],    /* real name of user */
              callsign[9],     /* amateur radio callsign of user */
              laston[9],       /* date user was last on */
              sex,             /* sex of user, M or F */
              parity[5],       /* parity 8N1 or 7E1 */
              curspeed[81],    /* speed user is on at, "KB" if local */
              data[81],        /* directory for non-text files, ends in \ */
              gfiles[81],      /* directory for text files, ends in \ */
              syslog[81],      /* full path & filename for sysop log */
              sysopname[81],   /* name of sysop */
              systemname[81];  /* name of BBS */
double        gold,            /* gold user has */
              timeallowed,     /* number of seconds before user logged off */
              version=-1.0;    /* version of WWIV running under */
long          timeon=0,        /* number of seconds since midnight logged on */
              timespent=0,     /* number of seconds spent online so far */
              uk=0,            /* # k uploaded */
              dk=0;            /* # k downloaded */

#ifdef v407
void far * far *funcs;
void far interrupt (*inli)(char far *, char far *, int, int);
void far interrupt (*checka)(int far *, int far *);
void far interrupt (*pla)(char far *, int far *);
void far interrupt (*outchr)(char);
void far interrupt (*outstr)(char far *);
void far interrupt (*nl)();
void far interrupt (*pl)(char far *);
int  far interrupt (*empty)();
char far interrupt (*inkey)();
unsigned char far interrupt (*getkey)();
void far interrupt (*input)(char far *, int);
void far interrupt (*inputl)(char far *, int);
int  far interrupt (*yn)();
int  far interrupt (*ny)();
void far interrupt (*ansic)(int);
char far interrupt (*onek)(char far *);
void far interrupt (*prt)(int, char far *);
void far interrupt (*mpl)(int);
#endif


int read_in_data(char *fn)
{
  char buf[1024],*ptr[50],*ss;
  float fl;
  int i,i1,len;
  unsigned short c_s,c_o;

  ss=getenv("BBS");
  if (!ss || strncmp(ss,"WWIV",4)) {
#ifdef v407
    printf("\nThis program MUST be run under WWIV v4.07 or later.\n");
    exit(1);
#else
    printf("\nWARNING: Not running under WWIV\n");
    return(1);
#endif
  }
  for(i=i1=0;i1<strlen(ss);i1++) {
    if (!isdigit(ss[i1])) continue;
    i*=10;
    i+=ss[i1]-'0';
  }
  version=((double)i)/100.0;
#ifdef v407
  if (i<407) {
    printf("\nThis program requires WWIV v4.07 or later.\n");
    exit(2);
  }
  if (i<421) funcs=(void far *)getvect(0x6a);
  else {
    ss=getenv("WWIV_FP");
    if (!ss) {
      printf("\nWWIV_FP enviornment variable not set.\n\n");
      exit(3);
    }
    c_s=strtol(ss,NULL,16);
    c_o=strtol(ss+5,NULL,16);
    if (c_s && c_o) funcs=(void far *)MK_FP(c_s,c_o);
    else {
      printf("\nWWIV_FP not set to valid value (%04lX:%04lX).\n\n",c_s,c_o);
      exit(4);
    }
  }
  inli=funcs[0];
  checka=funcs[1];
  pla=funcs[2];
  outchr=funcs[3];
  outstr=funcs[4];
  nl=funcs[5];
  pl=funcs[8];
  empty=funcs[9];
  inkey=funcs[10];
  getkey=funcs[11];
  input=funcs[12];
  inputl=funcs[13];
  yn=funcs[14];
  ny=funcs[15];
  ansic=funcs[16];
  onek=funcs[17];
  prt=funcs[18];
  mpl=funcs[19];
#endif
  if ((i=open(fn,O_RDONLY|O_BINARY))<0) return(i);
  len=read(i,(void *)buf,1024);
  close(i);
  for (i=i1=0;i<len;i++)
    if (buf[i]=='\r') {
      buf[i]=0;
      ptr[++i1]=&buf[i+2];
    }
  ptr[0]=buf;
  for (i=i1;i<50;i++) ptr[i]=NULL;
  while (*ptr[6]==' ') ++(ptr[6]);
  while (*ptr[15]==' ') ++(ptr[15]);
#ifdef DEBUG
  printf("%2u Parameters:\n",i1);
  for(i=0;i<i1;i++) printf("Parameter %2u: \"%s\"\n",i,ptr[i]);
#endif
  usernum=atoi(ptr[0]);
  strcpy(name,ptr[1]);
  strcpy(realname,ptr[2]);
  strcpy(callsign,ptr[3]);
  age=atoi(ptr[4]);
  sex=*ptr[5];
  sscanf(ptr[6],"%f",&fl);
  gold=(double)fl;
  strcpy(laston,ptr[7]);
  screenchars=atoi(ptr[8]);
  screenlines=atoi(ptr[9]);
  sl=atoi(ptr[10]);
  so=atoi(ptr[11]);
  cs=atoi(ptr[12]);
  okansi=atoi(ptr[13]);
  incom=atoi(ptr[14]);
  sscanf(ptr[15],"%f",&fl);
  timeallowed=(double)fl;
  strcpy(gfiles,ptr[16]);
  strcpy(data,ptr[17]);
  sprintf(syslog,"%s%s",gfiles,ptr[18]);
  strcpy(curspeed,ptr[19]);
  if (curspeed[0]!='K') modem_speed=atoi(curspeed);
  comport=atoi(ptr[20]);
  strcpy(systemname,(ptr[21]!=NULL)?ptr[21]:"");
  strcpy(sysopname,(ptr[22]!=NULL)?ptr[22]:"");
  if (ptr[23]!=NULL) timeon=atol(ptr[23]);
  if (ptr[24]!=NULL) timespent=atol(ptr[24]);
  if (ptr[25]!=NULL) uk=atol(ptr[25]);
  if (ptr[26]!=NULL) uploaded=atol(ptr[26]);
  if (ptr[27]!=NULL) dk=atol(ptr[27]);
  if (ptr[28]!=NULL) downloaded=atol(ptr[28]);
  strcpy(parity,(ptr[29]!=NULL)?ptr[29]:"");
  if (ptr[30]!=NULL) com_speed=atol(ptr[30]);
  if (ptr[31]!=NULL) system_number=atol(ptr[31]);
  return(0);
}

void main(int argc, char *argv[])
{
  int rc;
  char s[81];

  strcpy(s,(argc<2)?"CHAIN.TXT":argv[1]);
  if ((rc=read_in_data(s)) != 0) {
    printf("\nData file not found (RC=%d)\n\n",rc);
    abort();
  }
  printf("user #%d\n",usernum);
  printf("username %s\n",name);
  printf("realname %s\n",realname);
  printf("callsign %s\n",callsign);
  printf("age %d\n",age);
  printf("sex %c\n",sex);
  printf("gold %f\n",gold);
  printf("laston %s\n",laston);
  printf("screensize %dx%d\n",screenchars,screenlines);
  printf("sl %d\n",sl);
  printf("sysop %s\n",so?"Yes":"No");
  printf("cosysop %s\n",cs?"Yes":"No");
  printf("ansi %s\n",okansi?"Yes":"No");
  printf("incom %s\n",incom?"Yes":"No");
  printf("timeallowed %f\n",timeallowed);
  printf("gfiles %s\n",gfiles);
  printf("data %s\n",data);
  printf("syslog %s\n",syslog);
  printf("curspeed %s\n",curspeed);
  printf("com port %d\n",comport);
  printf("systemname %s\n",systemname);
  printf("sysopname %s\n",sysopname);
  printf("timeon %ld\n",timeon);
  printf("timespent %ld\n",timespent);
  printf("uk %ld\n",uk);
  printf("uploaded %d\n",uploaded);
  printf("dk %ld\n",dk);
  printf("downloaded %d\n",downloaded);
  printf("parity %s\n",parity);
  printf("com_speed %u\n",com_speed);
  printf("modem_speed %u\n",modem_speed);
  printf("system_number %u\n",system_number);
#ifdef v407
  sprintf(s,"Running under WWIV v%4.2f.",version);
  pl(s);
#endif
}
