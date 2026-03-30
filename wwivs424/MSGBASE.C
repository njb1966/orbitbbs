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

#include <mem.h>
#include <errno.h>
#include <io.h>
#include "subxtr.h"

int hasrip=0;
#ifdef RIPDRIVE
  int special = 0;
  char ripbuffer[255];
#endif

#define ALLOW_FULLSCREEN 1
#define EMAIL_STORAGE 2

/****************************************************************************/
void describe_area_code(int areacode, char *description)
{
  int f,done=0,i;
  char s[81],*ss,*ss1;

  description[0]=0;
  sprintf(s,"%sREGIONS.DAT",syscfg.datadir);
  f=sh_open1(s,O_RDONLY | O_BINARY);
  if (f<1)
    return;
  ss=malloca(filelength(f));
  i=sh_read(f,ss,filelength(f));
  ss[i]=0;
  sh_close(f);
  ss1=strtok(ss,"\r\n");
  while (ss1 && (!done)) {
    i=atoi(ss1);
    if (i) {
      if (i==areacode)
        done=1;
    } else
      strcpy(description,ss1);
    ss1=strtok(NULL,"\r\n");
  }

  bbsfree(ss);
}



/****************************************************************************/
static char origin_str[128];
static char origin_str2[81];

void setorigin(int sysnum, int usernum)
{
  char s[81],s1[81],st[12];
  net_system_list_rec *csne;

  if (net_num_max>1)
    sprintf(s1,"%s - ",net_networks[net_num].name);
  else
    s1[0]=0;

  origin_str[0]=0;
  origin_str2[0]=0;

  if (sysnum && (net_type == net_type_wwivnet)) {
    csne=next_system(sysnum);
    if (csne) {
      strcpy(st,"");
      if (usernum==1) {
        if (csne->other & other_net_coord)
          strcpy(st,get_string(1189));
        else if (csne->other & other_group_coord)
          sprintf(st,"{GC%d}",csne->group);
        else if (csne->other & other_coordinator)
          strcpy(st,get_string(1190));
      }

      describe_area_code(atoi(csne->phone),s);
      if (s[0]) {
        sprintf(origin_str,"%s%s [%s] %s",s1,csne->name,csne->phone,st);
        strcpy(origin_str2,s);
      } else {
        sprintf(origin_str,"%s%s [%s] %s",s1,csne->name,csne->phone,st);
        strcpy(origin_str2,get_string(1007));
      }
    } else {
      strcpy(origin_str,s1);
      strcat(origin_str,get_string(622));
      strcpy(origin_str2,get_string(1007));
    }
  }
}


int okfsed(void)
{
  int ok;

  ok=ALLOW_FULLSCREEN;
  if (!okansi())
    ok=0;
  if (!thisuser.defed)
    ok=0;
  if (thisuser.defed>numed)
    ok=0;
  return(ok);
}


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
      if (f>0) {
        set_gat_section(f,(int) (m.stored_as/2048L));
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


int open_file(char *fn)
{
  int f,i;
  char s[81];

  read_status();

  sprintf(s,"%s%s.DAT",syscfg.msgsdir,fn);
  f=sh_open1(s,O_RDWR | O_BINARY);
  if (f<0) {
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    for (i=0; i<2048; i++)
      gat[i]=0;
    sh_write(f,(void *)gat,4096);
    strcpy(gatfn,fn);
    chsize(f,4096L + (75L * 1024L));
    gat_section=0;
  }
  if (strcmp(gatfn,fn) || 1) {
    sh_lseek(f,0L,SEEK_SET);
    sh_read(f,(void *)gat,4096);
    strcpy(gatfn,fn);
    gat_section=0;
  }
  return(f);
}

#define GATSECLEN (4096L+2048L*512L)
#ifndef MSG_STARTING
#define MSG_STARTING (((long)gat_section)*GATSECLEN + 4096L)
#endif


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
    sh_lseek(f,l1,SEEK_SET);
    if (l<(l1+4096L)) {
      for (i=0; i<2048; i++)
        gat[i]=0;
      sh_write(f,(void *)gat, 4096);
    } else {
      sh_read(f,(void *)gat, 4096);
    }
    gat_section=section;
  }
}

void save_gat(int f)
{
  long l;

  l=((long)gat_section)*GATSECLEN;
  sh_lseek(f,l,SEEK_SET);
  sh_write(f,(void *)gat,4096);
  lock_status();
  status.filechange[filechange_posts]++;
  save_status();
}


void savefile(char *b, long l1, messagerec *m1, char *aux)
{
  int f,gatp,i5,i4,gati[128],section;
  messagerec m;
  char s[81],s1[81];
  long l2;

  m=*m1;
  switch(m.storage_type) {
    case 0:
    case 1:
      lock_status();
      m.stored_as=status.qscanptr++;
      save_status();
      ltoa(m.stored_as,s1,16);
      strcpy(s,syscfg.msgsdir);
      if (m.storage_type==1) {
        strcat(s,aux);
        strcat(s,"\\");
      }
      strcat(s,s1);
      f=sh_open(s,O_RDWR | O_CREAT | O_BINARY,S_IREAD | S_IWRITE);
      sh_write(f, (void *)b,l1);
      sh_close(f);
      break;
    case 2:
      f=open_file(aux);
      if (f>0) {
        for (section=0; section<1024; section++) {
          set_gat_section(f,section);
          gatp=0;
          i5=(int) ((l1 + 511L)/512L);
          i4=1;
          while ((gatp<i5) && (i4<2048)) {
            if (gat[i4]==0)
              gati[gatp++]=i4;
            ++i4;
          }
          if (gatp>=i5) {
            l2=MSG_STARTING;
            gati[gatp]=-1;
            for (i4=0; i4<i5; i4++) {
              sh_lseek(f,l2 + 512L * (long)(gati[i4]),SEEK_SET);
              sh_write(f,(void *)(&b[i4*512]),512);
              gat[gati[i4]]=gati[i4+1];
            }
            save_gat(f);
            break;
          }
        }
        sh_close(f);
      }
      m.stored_as=((long) gati[0]) + ((long)gat_section)*2048L;
      break;
    case 255:
      f=sh_open(aux,O_RDWR | O_CREAT | O_BINARY,S_IREAD | S_IWRITE);
      sh_write(f, (void *)b,l1);
      sh_close(f);
      break;
    default:
      npr(get_stringx(1,36),m.storage_type);
      break;
  }
  bbsfree((void *)b);
  *m1=m;
}


char *readfile(messagerec *m1, char *aux, long *l)
{
  int f,csec;
  long l1,l2;
  char *b,s[81],s1[81];
  messagerec m;

  *l=0L;
  m=*m1;
  switch(m.storage_type) {
    case 0:
    case 1:
      strcpy(s,syscfg.msgsdir);
      ltoa(m.stored_as,s1,16);
      if (m.storage_type==1) {
        strcat(s,aux);
        strcat(s,"\\");
      }
      strcat(s,s1);
      f=sh_open1(s,O_RDONLY | O_BINARY);
      if (f==-1) {
        *l=0L;
        return(NULL);
      }
      l1=filelength(f);
      if ((b=malloca(l1))==NULL) {
        sh_close(f);
        return(NULL);
      }
      sh_read(f,(void *)b,l1);
      sh_close(f);
      *l=l1;
      break;
    case 2:
      f=open_file(aux);
      set_gat_section(f,(int) (m.stored_as/2048L));
      csec=m.stored_as % 2048;
      l1=0;
      while ((csec>0) && (csec<2048)) {
        l1+=512L;
        csec=gat[csec];
      }
      if (!l1) {
        nl();
        pl(get_string(623));
        nl();
        sh_close(f);
        return(NULL);
      }
      if ((b=malloca(l1+3))==NULL) {
        sh_close(f);
        return(NULL);
      }
      csec=m.stored_as % 2048;
      l1=0;
      l2=MSG_STARTING;
      while ((csec>0) && (csec<2048)) {
        sh_lseek(f,l2 + 512L*((long)csec),SEEK_SET);
        l1+=(long)sh_read(f,(void *)(&(b[l1])),512);
        csec=gat[csec];
      }
      sh_close(f);
      l2=l1-512;
      while ((l2<l1) && (b[l2]!=26))
        ++l2;
      *l=l2;
      b[l2+1]=0;
      break;
    case 255:
      f=sh_open1(aux,O_RDONLY | O_BINARY);
      if (f==-1) {
        *l=0L;
        return(NULL);
      }
      l1=filelength(f);
      if ((b=malloca(l1+256L))==NULL) {
        sh_close(f);
        return(NULL);
      }
      sh_read(f,(void *)b,l1);
      sh_close(f);
      *l=l1;
      break;
    default:
      /* illegal storage type */
      *l=0L;
      b=NULL;
      break;
  }
  return(b);
}


void change_storage(messagerec *oldm, char *olda, messagerec *newm, char *newa)
{
  long len;
  char *b;

  b=readfile(oldm,olda,&len);
  remove_link(oldm,olda);
  savefile(b,len,newm,newa);
}


void load_workspace(char *fnx, int no_edit)
{
  int i;
  long l;
  char *b,s[81];

  i=sh_open1(fnx,O_RDONLY | O_BINARY);
  if (i<1) {
    nl();
    pl(get_string(89));
    nl();
    return;
  }
  l=filelength(i);
  if ((b=malloca(l+1024))==NULL) {
    sh_close(i);
    return;
  }
  sh_read(i, (void *) b,l);
  sh_close(i);
  if (b[l-1]!=26)
    b[l++]=26;
  sprintf(s,"%sINPUT.MSG",syscfgovr.tempdir);
  i=sh_open(s,O_RDWR | O_CREAT | O_BINARY,S_IREAD | S_IWRITE);
  sh_write(i, (void *)b,l);
  sh_close(i);
  bbsfree(b);
  if ((no_edit) || (!okfsed()))
    use_workspace=1;
  else
    use_workspace=0;
  nl();
  pl(get_string(624));
  nl();
  if (!use_workspace)
    pl(get_string(625));
}


void osan(char *s, int *abort, int *next)
{
  int i;
  char ansicode[30];

  i=0;
  checkhangup();
  if (hangup)
    *abort=1;
  checka(abort,next);
#ifdef RIPDRIVE
  if (special) {    
    while (s[i]) {
      if (s[i] == 3) {                  // Interpret heart codes
        strcat(ripbuffer, "\x1b[40m");
        makeansi((thisuser.sysstatus & sysstatus_color) ?
        thisuser.colors[s[++i] - '0'] :
        thisuser.bwcolors[s[++i] - '0'],ansicode, 0);
        strcat(ripbuffer, ansicode);
      } else if (s[i] == 15) {
        out_cport(s[i++]);
        if (s[i] == 15) {
          out_cport(s[i++]);
          strcat(ripbuffer, interpret(s[i]));
        }
      } else {
        ripbuffer[strlen(ripbuffer)+1] = 0;
        ripbuffer[strlen(ripbuffer)] = s[i];
      }
      out_cport(s[i]);
      if (s[i])
        i++;
      if (ripcode == 0) {           /* Don't allow abort of RIP menus; */
        checka(abort,next);         /* can cause problems otherwise */
        if (*abort)
          break;
      }
    }
  } else {
#endif
    while ((s[i]) && (!(*abort))) {
      if (ripcode)
        out_cport(s[i++]);
      else {
        outchr(s[i++]);
        checka(abort,next);
      }
    }
#ifdef RIPDRIVE
  }
#endif
}



void addline(char *b, char *s, long *ll)
{
  strcpy(&(b[*ll]),s);
  *ll +=strlen(s);
  strcpy(&(b[*ll]),"\r\n");
  *ll += 2;
}


#define LEN 161

void stuff(char *s, char *old, char *new)
{
  char s1[LEN],*ss;

  if (strlen(s)-strlen(old)+strlen(new)>=LEN)
    return;
  ss=strstr(s,old);
  if (ss==NULL)
    return;
  ss[0]=0;
  sprintf(s1,"%s%s%s",s,new,ss+strlen(old));
  strcpy(s,s1);
}

static char *preserve_lines[] = {
  "0Usenet References:",
  "0Usenet Message_ID:",
  "0U_WWIV_Thread:",
};

void inmsg(messagerec *m1, char *title, int *anony, int needtitle, char *aux, int fsed, char *dest, int flags)
{
  char s[181],s1[181],ro[81],fnx[81],chx,*ss,*ss1, fn1[81], fn2[81],*nd;
  int maxli,curli,done,save,savel,i,i1,i2,i3,i4,i5,setanon,oiia;
  messagerec m;
  long ll,l1;
  char *lin, *b;
  int real_name=0;
  FILE *result, *q_fp;
  struct {char tlen, ttl[81], anon; } fedit_data;
  xtrasubsnetrec *xnp;
  char q_txt[256];

  oiia=iia;
  setiia(0);

  if ((fsed!=0) && (!okfsed()))
    fsed=0;
  sprintf(fnx,"%sINPUT.MSG",syscfgovr.tempdir);
  if (fsed)
    fsed=1;
  if (use_workspace) {
    if (!exist(fnx))
      use_workspace=0;
    else
      fsed=2;
  }
  if (menu_on()) {
     rip_saveall();
     if (fsed == 0)
       printmenu(2);
     else {
       sprintf(s,"%sEDIT%d.RIP",languagedir,thisuser.defed);
       ripcode = 1;
       if (exist(s))
         maybeprint(s,2);
       ripcode = 0;
     }
     cleared = NEEDCLEAR;
  }
  done=0;
  setanon=0;
  save=0;
  curli=0;
  m=*m1;

  if (fsed) {
    if (so())
      maxli=120;
    else if (cs())
      maxli=100;
    else
      maxli=80;
  } else {
    if (actsl<45)
      maxli=30;
    else
      if (actsl<60)
        maxli=50;
      else
        if (actsl<80)
          maxli=60;
        else
          maxli=80;
  }
  if (!fsed) {
    if ((lin=malloca((long)(maxli+10)*LEN))==NULL) {
      m1->stored_as=0xffffffff;
      setiia(oiia);
     if (menu_on())
       rip_restoreall();
      return;
    }
    for (i=0; i<maxli; i++)
      lin[i*LEN]=0;
    ro[0]=0;
  }

  nl();
  helpl=6;
  if (okansi()) {
    prt(2,get_string(1006));
    mpl(60);
    inputl(title,60);
  } else {
    pl(get_string(626));
    outstr(get_string(1006));
    inputl(title,60);
  }
  if ((title[0]==0) && (needtitle)) {
    pl(get_string(14));
    m.stored_as=0xffffffff;
    *m1=m;
    if (!fsed)
      bbsfree((void *)lin);
    setiia(oiia);
    if (menu_on())
      rip_restoreall();
    return;
  }

  if (!fsed) {
    outstr(get_string(627));
    pln(maxli);
    pl(get_string(628));
    strcpy(s,get_string(629));
    s[thisuser.screenchars]=0;
    pl(s);

    while (!done && !hangup) {
      helpl=27;
      while (inli2(s,ro,160,1,curli)) {
        --curli;
        strcpy(ro,&(lin[(curli)*LEN]));
        if (strlen(ro)>thisuser.screenchars-1)
          ro[thisuser.screenchars-2]=0;
      }
      if (hangup)
        done=1;
      savel=1;
      if (s[0]=='/') {
        if (stricmp(s,get_string(942))==0) {
          savel=0;
          printmenu(2);
        }
        if (stricmp(s,get_string(943))==0) {
          savel=0;
          if (okansi())
            i1=0;
          else {
            prt(5,get_string(630));
            i1=yn();
          }
          i2=0;
          for (i=0; (i<curli) && (!i2); i++) {
            if (i1)
              npr("%d:\r\n",i+1);
            strcpy(s1,&(lin[i*LEN]));
            i3=strlen(s1);
            if (s1[i3-1]==1)
              s1[i3-1]=0;
            if (s1[0]==2) {
              strcpy(s1,&(s1[1]));
              i5=0;
              for(i4=0; i4<strlen(s1); i4++)
                if ((s1[i4]==8) || (s1[i4]==3))
                  --i5;
                else
                  ++i5;
              for (i4=0; (i4<(thisuser.screenchars-i5)/2) && (!i2); i4++)
                osan(" ",&i2,&i1);
            }
            pla(s1,&i2);
          }
          if ((!okansi()) || i1) {
            nl();
            pl(get_string(631));
          }
        }
        if ((stricmp(s,get_string(944))==0) || (stricmp(s,get_string(945))==0)) {
          save=1;
          done=1;
          savel=0;
        }
        if ((stricmp(s,get_string(946))==0) || (stricmp(s,get_string(947))==0)) {
          save=1;
          done=1;
          savel=0;
          setanon=1;
        }
        if ((stricmp(s,get_string(948))==0) || (stricmp(s,get_string(949))==0)) {
          save=1;
          done=1;
          savel=0;
          setanon=-1;
        }
        if (stricmp(s,get_string(950))==0) {
          done=1;
          savel=0;
        }
        if (stricmp(s,get_string(951))==0) {
          savel=0;
          curli=0;
          pl(get_string(632));
          nl();
        }
        if (stricmp(s,get_string(952))==0) {
          savel=0;
          if (curli) {
            --curli;
            pl(get_string(633));
          } else {
            pl(get_string(634));
          }
        }
        if (stricmp(s,get_string(953))==0) {
          savel=0;
          helpl=26;
          if (okansi()) {
            prt(2,get_string(326));
            mpl(60);
            inputl(title,60);
          } else {
            pl(get_string(626));
            outstr(get_string(326));
            inputl(title,60);
          }
          pl(get_string(631));
          nl();
        }
        strcpy(s1,s);
        s1[3]=0;
        if (stricmp(s1,get_string(954))==0) {
          s1[0]=2;
          strcpy((&s1[1]),&(s[3]));
          strcpy(s,s1);
        }
        if ((stricmp(s1,get_string(955))==0) && (s[3]=='/') && (curli)) {
          strcpy(s1,&(s[4]));
          ss=strstr(s1,"/");
          if (ss) {
            ss1=&(ss[1]);
            ss[0]=0;
            stuff(&(lin[(curli-1)*LEN]),s1,ss1);
            pl(get_string(635));
            pl(&(lin[(curli-1)*LEN]));
            pl(get_string(956));
          }
          savel=0;
        }
      }
      if (savel) {
        strcpy(&(lin[(curli++)*LEN]),s);
        if (curli==(maxli+1)) {
          nl();
          pl(get_string(636));
          pl(get_string(957));
          nl();
          --curli;
        } else if (curli==maxli) {
          pl(get_string(637));
        } else if ((curli+5)==maxli) {
          pl(get_string(638));
        }
      }
    }
    if (curli==0)
      save=0;
  } else {
    if (fsed==1) {
      sprintf(fn1,"%sfedit.inf",syscfgovr.tempdir);
      sprintf(fn2,"%sresult.ed",syscfgovr.tempdir);
      _chmod(fn1,1,0);
      unlink(fn1);
      _chmod(fn2,1,0);
      unlink(fn2);
      fedit_data.tlen=60;
      strcpy(fedit_data.ttl,title);
      fedit_data.anon=0;
      result=fsh_open(fn1,"wb");
      if (result) {
        fsh_write(&fedit_data, sizeof(fedit_data), 1, result);
        fsh_close(result);
      }
      save=external_edit("INPUT.MSG",syscfgovr.tempdir,(int) (thisuser.defed)-1,
        maxli, dest, title, flags);
      if (save) {
        if ((result=fsh_open(fn2,"rt"))!=NULL) {
          if (fgets(s,80,result)) {
            ss=strchr(s,'\n');
            if (ss) *ss=0;
            setanon=atoi(s);
            if (fgets(title,80,result)) {
              ss=strchr(title,'\n');
              if (ss) *ss=0;
              fsh_close(result);
            }
          }
        } else if ((result=fsh_open(fn1,"rb"))!=NULL) {
          if (fsh_read(&fedit_data, sizeof(fedit_data), 1, result)==1) {
            strcpy(title,fedit_data.ttl);
            setanon=fedit_data.anon;
          }
          fsh_close(result);
        }
      }
      unlink(fn1);
      unlink(fn2);
    } else {
      save=exist(fnx);
      if (save) {
        pl(get_string(639));
      }
      use_workspace=0;
    }
  }


  if (save) {
    switch(*anony) {
      case 0: /* no anony */
        *anony=0;
        break;
      case anony_enable_anony:
        if (setanon) {
          if (setanon==1)
            *anony=anony_sender;
          else
            *anony=0;
        } else {
          prt(5,get_string(485));
          if (yn())
            *anony=anony_sender;
          else
            *anony=0;
        }
        break;
      case anony_enable_dear_abby:
        nl();
        npr("1. %s\r\n",nam(&thisuser,usernum));
        pl(get_string(640));
        pl(get_string(641));
        nl();
        prt(5,get_string(297));
        chx=onek("\r123");
        switch(chx) {
          case '\r':
          case '1':
            *anony=0;
            break;
          case '2':
            *anony=anony_sender_da;
            break;
          case '3':
            *anony=anony_sender_pp;
        }
        break;
      case anony_force_anony:
        *anony=anony_sender;
        break;
      case anony_real_name:
        real_name=1;
        *anony=0;
        break;
    }
    outstr(get_string(91));
    if (fsed) {
      i5=sh_open1(fnx,O_RDONLY | O_BINARY);
      l1=filelength(i5);
      if (l1>32760)
        l1=32760;
    } else {
      l1=0;
      for (i5=0; i5<curli; i5++) {
        l1 += strlen(&(lin[i5*LEN]));
        l1 += 2;
      }
    }
    l1 += 1024;
    if ((b=malloca(l1))==NULL) {
      sh_close(i5);
      bbsfree(lin);
      pl(get_string(642));
      m1->stored_as=0xffffffff;
      setiia(oiia);
      if (menu_on())
        rip_restoreall();
      return;
    }

    l1=0;
    if (real_name)
      addline(b,thisuser.realname,&l1);
    else
      addline(b,nam1(&thisuser,usernum,net_sysnum),&l1);
    time(&ll);
    strcpy(s,ctime(&ll));
    s[strlen(s)-1]=0;
    addline(b,s,&l1);

    sprintf(q_txt,"%squotes.txt",syscfgovr.tempdir);
    q_fp=fsh_open(q_txt, "rt");
    if (q_fp) {
      while (fgets(q_txt, sizeof(q_txt)-1, q_fp)) {
        ss1=strchr(q_txt,'\n');
        if (ss1)
          *ss1=0;
        i3=0;
        for (i4=0; i4<sizeof(preserve_lines)/sizeof(preserve_lines[0]); i4++) {
          if (strncmp(preserve_lines[i4], q_txt, strlen(preserve_lines[i4]))==0) {
            i3=1;
            break;
          }
        }
        if (i3) {
          addline(b, q_txt, &l1);
        }
      }
      fsh_close(q_fp);
    }

    if (irt[0]) {
      sprintf(s,"%s%s",get_string(940),irt);
      addline(b,s,&l1);
      if (irt_sub[0]) {
        sprintf(s,"%s%s", get_string(1509), irt_sub);
        addline(b,s,&l1);
      }
      if (irt_name[0] && strcmp(aux, "EMAIL")) {
        sprintf(s,"%s%s",get_string(941),irt_name);
        addline(b,s,&l1);
      }
      addline(b,"",&l1);
    }

    if (fsed) {
      ll=filelength(i5);
      sh_read(i5, (void *) (& (b[l1]) ),ll);
      l1 += ll;
      sh_close(i5);
    } else {
      for (i5=0; i5<curli; i5++)
        addline(b,&(lin[i5*LEN]),&l1);
    }

    if (sysinfo.flags & OP_FLAGS_MSG_TAG) {
      if ((xsubs[curlsub].num_nets) && (strcmp(aux,"EMAIL")!=0)
          && (!(subboards[curlsub].anony & anony_no_tag))
          && (strcmp(strupr(irt),strupr(get_string(333)))!=0)) {
        for (i=0; i<xsubs[curlsub].num_nets; i++) {
          xnp=&xsubs[curlsub].nets[i];
          nd=net_networks[xnp->net_num].dir;
          sprintf(s,"%s%s.TAG",nd,xnp->stype);
          if (exist(s))
            break;
          sprintf(s,"%sGENERAL.TAG",nd);
          if (exist(s))
            break;
          sprintf(s,"%s%s.TAG",syscfg.datadir,xnp->stype);
          if (exist(s))
            break;
          sprintf(s,"%sGENERAL.TAG",syscfg.datadir);
          if (exist(s))
            break;
        }
        result=fsh_open(s,"rb");
        if (result) {
          i=0;
          while (!feof(result)) {
            s[0]=0; s1[0]=0;
            fgets(s,180,result);
            if (strlen(s)>1)
              if (s[strlen(s)-2]==13)
                s[strlen(s)-2]=0;
            if (s[0]!=4)
              sprintf(s1,"%c%c%s",4,i+'2',s);
            else
              strncpy(s1,s,sizeof(s1));
            if (!i)
              addline(b,"1",&l1);
            addline(b,s1,&l1);
            if (i<7)
              i++;
          }
          fsh_close(result);
        }
      }
    }

    if (b[l1-1]!=26)
      b[l1++]=26;
    savefile(b,l1,&m,aux);
    if (fsed)
      unlink(fnx);
  } else {
    if (fsed)
      unlink(fnx);
    pl(get_string(14));
    m.stored_as=0xffffffff;
  }
  *m1=m;
  if (!fsed) {
    bbsfree((void *)lin);
  }
  charbufferpointer=0;
  charbuffer[0]=0;
  setiia(oiia);
  if (menu_on())
    rip_restoreall();
  grab_quotes(NULL, NULL);
}



int forwardm(unsigned short *u, unsigned short *s)
{
  userrec ur;
  char *ss;
  int i,cu,nn;

  if (*s)
    return(0);
  read_user(*u,&ur);
  if (ur.inact & inact_deleted)
    return(0);
  if ((ur.forwardusr==0) && (ur.forwardsys==0))
    return(0);
  if (ur.forwardsys) {
    if ((ur.forwardusr>0) && (ur.forwardusr<32767)) {
      nn=net_num;
      set_net_num(ur.net_num);
      if (!valid_system(ur.forwardsys)) {
        set_net_num(nn);
        return(0);
      }
      *u=ur.forwardusr;
      *s=ur.forwardsys;
      return(1);
    } else {
      *u=0;
      *s=0;
      return(0);
    }
  }
  cu=ur.forwardusr;
  if (cu==-1) {
    pl(get_string(643));
    if (so()) {
      pl(get_string(644));
    } else {
      *u=0;
      *s=0;
    }
    return(0);
  }
  if ((ss=malloca((long) syscfg.maxusers+ 300L))==NULL)
    return(0);
  for (i=0; i<syscfg.maxusers+300; i++)
    ss[i]=0;
  ss[*u]=1;
  read_user(cu,&ur);
  while ((ur.forwardusr) || (ur.forwardsys)) {
    if (ur.forwardsys) {
      if (!valid_system(ur.forwardsys))
        return(0);
      *u=ur.forwardusr;
      *s=ur.forwardsys;
      bbsfree(ss);
      set_net_num(ur.net_num);
      return(1);
    }
    if (ss[cu]) {
      bbsfree(ss);
      return(0);
    }
    ss[cu]=1;
    if (ur.forwardusr==65535) {
      pl(get_string(643));
      if (so()) {
        pl(get_string(644));
        *u=cu;
        *s=0;
      } else {
        *u=0;
        *s=0;
      }
      bbsfree(ss);
      return(0);
    }
    cu=ur.forwardusr;
    read_user(cu,&ur);
  }
  bbsfree(ss);
  *s=0;
  *u=cu;
  return(1);
}


int open_email(int wrt)
{
  char s[100];
  int f,i;

  sprintf(s,"%sEMAIL.DAT", syscfg.datadir);

  for (i=0; i<5; i++) {
    if (wrt)
      f=sh_open(s, O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
    else
      f=sh_open1(s, O_RDONLY|O_BINARY);
    if ((f>0) || (errno!=EACCES))
      break;
    wait1(9);
  }

  return(f);
}

void sendout_email(char *title, messagerec *msg, int anony, unsigned un, unsigned sy, int an, unsigned uf, unsigned sf, int fwd, int fnn)
{
  mailrec m,m1;
  net_header_rec nh;
  int f,len,i,i1;
  char *b,*b1,s[256],s1[200];
  long len1;
  userrec ur;

  strcpy(m.title, title);
  m.msg=*msg;
  m.anony=anony;
  if (sf == net_sysnum)
    m.fromsys=0;
  else
    m.fromsys=sf;
  m.fromuser=uf;
  m.tosys=sy;
  m.touser=un;
  m.status=0;
  time((long *)&(m.daten));

  if (m.fromsys && (net_num_max>1)) {
    m.status|=status_new_net;
    m.title[79]=0;
    m.title[80]=fnn;
  }

  if (sy==0) {
    f=open_email(1);
    if (f<0) {
      return;
    }
    len=(int) filelength(f)/sizeof(mailrec);
    if (len==0)
      i=0;
    else {
      i=len-1;
      sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
      sh_read(f,(void *)&m1,sizeof(mailrec));
      while ((i>0) && (m1.tosys==0) && (m1.touser==0)) {
        --i;
        sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
        i1=sh_read(f,(void *)&m1,sizeof(mailrec));
        if (i1==-1)
          pl(get_string(1193));
      }
      if ((m1.tosys) || (m1.touser))
        ++i;
    }
    sh_lseek(f,((long) (i))*(sizeof(mailrec)), SEEK_SET);
    i1=sh_write(f,(void *)&m,sizeof(mailrec));
    sh_close(f);
    if (i1==-1) {
      pl(get_string(1194));
    }
  } else {
    if ((b=readfile(&(m.msg),"EMAIL",&len1))==NULL)
      return;
    remove_link(&(m.msg),"EMAIL");
    nh.tosys=sy;
    nh.touser=un;
    if (sf)
      nh.fromsys=sf;
    else
      nh.fromsys=net_sysnum;
    nh.fromuser=uf;
    nh.main_type=main_type_email;
    nh.minor_type=0;
    nh.list_len=0;
    nh.daten=m.daten;
    nh.method=0;
    if ((b1=malloca(len1+768))==NULL) {
      bbsfree(b);
      return;
    }
    i=0;
    if ((un==0) && (fnn==net_num)) {
      nh.main_type=main_type_email_name;
      strcpy(&(b1[i]),net_email_name);
      i+= strlen(net_email_name)+1;
    }
    strcpy(&(b1[i]),m.title);
    i += strlen(m.title)+1;
    memmove(&(b1[i]),b,(unsigned int) len1);
    nh.length=len1+(long)i;
    if (nh.length > 32760) {
      outstr(get_string(645));
      npr("%lu",nh.length-32760L);
      outstr(get_string(646));
      nh.length = 32760;
    }
    if (fnn!=net_num) {
      gate_msg(&nh, b1,net_num, net_email_name, NULL, fnn);
    } else {
      if (fwd)
        sprintf(s,"%sP1%s",net_data,nete);
      else
        sprintf(s,"%sP0%s",net_data,nete);
      f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
      sh_lseek(f,0L,SEEK_END);
      sh_write(f,(void *)&nh,sizeof(net_header_rec));
      sh_write(f,(void *)b1,nh.length);
      sh_close(f);
    }
    bbsfree(b);
    bbsfree(b1);
  }
  strcpy(s,get_string(647));
  if (sy==0) {
    read_user(un,&ur);
    ++ur.waiting;
    write_user(un,&ur);
    if (un==1)
      ++fwaiting;
    if (user_online(un, &i)) {
      send_inst_sysstr(i, get_string(1510));
    }
    if (an) {
      strcat(s,nam(&ur,un));
      sysoplog(s);
    } else {
      strcpy(s1,s);
      strcat(s1,nam(&ur,un));
      sysoplog(s1);
      strcat(s,get_string(482));
    }
  } else {
    if (net_num_max>1) {
      if (un==0)
        sprintf(s1,"%s @%u.%s",net_email_name, sy, net_name);
      else
        sprintf(s1,"#%u @%u.%s", un, sy, net_name);
    } else {
      if (un==0)
        sprintf(s1,"%s @%u",net_email_name,sy);
      else
        sprintf(s1,"#%u @%u",un,sy);
    }
    strcat(s,s1);
    sysoplog(s);
  }
  lock_status();
  if ((un==1) && (sy==0)) {
    ++status.fbacktoday;
    ++thisuser.feedbacksent;
    ++thisuser.fsenttoday1;
    ++fsenttoday;
  } else {
    ++status.emailtoday;
    ++thisuser.etoday;
    if (sy==0) {
      ++thisuser.emailsent;
    } else {
      ++thisuser.emailnet;
    }
  }
  save_status();
  if (!wfc)
    topscreen();
  pl(s);
}


int ok_to_mail(unsigned un, unsigned sy, int forceit)
{
  userrec ur;
  slrec ss;

  ss=syscfg.sl[actsl];
  if ((sy!=0) && (net_sysnum==0)) {
    nl();
    pl(get_string(648));
    nl();
    return(0);
  }
  if (sy==0) {
    if (un==0)
      return(0);
    read_user(un,&ur);
    if (((ur.sl==255) && (ur.waiting>(syscfg.maxwaiting * 5))) ||
        ((ur.sl!=255) && (ur.waiting>syscfg.maxwaiting)) ||
        (ur.waiting>200)) {
      if (!forceit) {
        nl();
        pl(get_string(340));
        nl();
        return(0);
      }
    }
    if (ur.inact & inact_deleted) {
      nl();
      pl(get_string(341));
      nl();
      return(0);
    }
  } else {
    if (!valid_system(sy)) {
      nl();
      pl(get_string(649));
      nl();
      return(0);
    }
    if (thisuser.restrict & restrict_net) {
      nl();
      pl(get_string(650));
      return(0);
    }
  }
  if (forceit==0) {
    if ((((un==1) && (sy==0) && ((fsenttoday>=5) || (thisuser.fsenttoday1>=10))) ||
         (((un!=1) || (sy!=0)) && (thisuser.etoday>=ss.emails))) && (!cs())) {
      nl();
      pl(get_string(344));
      nl();
      return(0);
    }
    if ((restrict_email & thisuser.restrict) && (un!=1)) {
      nl();
      pl(get_string(345));
      nl();
      return(0);
    }
  }
  return(1);
}

void email(unsigned short un, unsigned short sy, int forceit, int anony)
{
  int i,an;
  messagerec msg;
  char s2[81],t[81];
  userrec ur;
  slrec ss;
  net_system_list_rec *csne;


  if (freek1(syscfg.msgsdir)<10.0) {
    nl();
    pl(get_string(332));
    nl();
    return;
  }
  nl();
  ss=syscfg.sl[actsl];
  if (forwardm(&un,&sy)) {
    nl();
    pl(get_string(651));
    nl();
    if ((un==0) && (sy==0)) {
      pl(get_string(652));
      return;
    }
  }
  if (!un && !sy)
    return;
  if (!ok_to_mail(un, sy, forceit))
    return;
  if (sy)
    csne=next_system(sy);
  if (ss.ability & ability_read_email_anony)
    an=1;
  else
    if (anony & (anony_sender | anony_sender_da | anony_sender_pp))
      an=0;
    else
      an=1;
  if (sy==0) {
    set_net_num(0);
    if (an) {
      read_user(un,&ur);
      strcpy(s2,nam(&ur,un));
    } else
      strcpy(s2,get_string(482));
  } else {
    if (net_num_max>1) {
      if (un==0)
        sprintf(s2,"%s @%u.%s", net_email_name, sy, net_name);
      else
        sprintf(s2,"#%u @%u.%s", un, sy, net_name);
    } else {
      if (un==0)
        sprintf(s2,"%s @%u",net_email_name,sy);
      else
        sprintf(s2,"#%u @%u",un,sy);
    }
  }
  outstr(get_string(653));
  pl(s2);
  if (ss.ability & ability_email_anony)
    i=anony_enable_anony;
  else
    i=0;
  if (anony & (anony_receiver_pp | anony_receiver_da))
    i=anony_enable_dear_abby;
  if (anony & anony_receiver)
    i=anony_enable_anony;
  if ((i==anony_enable_anony) && (thisuser.restrict & restrict_anony))
    i=0;
  if (sy!=0) {
    i=0;
    anony=0;
    nl();
    outstr(get_string(654));
    pl(csne -> name);
    outstr(get_string(655));
    pln(csne->numhops);
    nl();
  }

  write_inst(INST_LOC_EMAIL, (sy==0)?un:0, INST_FLAGS_NONE);

  msg.storage_type=EMAIL_STORAGE;
  inmsg(&msg,t,&i,!forceit,"EMAIL",ALLOW_FULLSCREEN, s2, 0);
  if (msg.stored_as==0xffffffff)
    return;
  if (anony & anony_sender)
    i|=anony_receiver;
  if (anony & anony_sender_da)
    i|=anony_receiver_da;
  if (anony & anony_sender_pp)
    i|=anony_receiver_pp;

  sendout_email(t, &msg, i, un, sy, an, usernum, net_sysnum, 0, net_num);
}


void imail(unsigned short u, unsigned short s)
{
  char s1[81];
  int i;
  userrec ur;

  if (forwardm(&u,&s))
    pl(get_string(656));

  if (!u && !s)
    return;

  i=1;
  helpl=0;
  if (s==0) {
    read_user(u,&ur);
    if ((ur.inact & inact_deleted)==0) {
      sprintf(s1,"%s %s? ",get_string(657), nam(&ur,u));
      prt(5,s1);
      if (yn()==0)
        i=0;
    } else
      i=0;
  } else {
    sprintf(s1,"%s %s %u @%u ? ",get_string(657), get_string(658), u,s);
    prt(5,s1);
    if (yn()==0)
      i=0;
  }
  grab_quotes(NULL, NULL);
  if (i)
    email(u,s,0,0);
}


void plan(char *s, int *abort, int *next)
{
  int i;

  i=0;
  checkhangup();
  if (hangup)
    *abort=1;
  checka(abort,next);
  while ((s[i]) && (!(*abort))) {
    outchr(s[i++]);
    checka(abort,next);
  }
  if (!(*abort))
    nl();
}


#define buf_size 512

#define STR_1002 (E_C?1002:1369)
#define STR_661 (E_C?661:1370)
#define STR_662 (E_C?662:1374)
#define STR_1008 (E_C?1008:1375)

void read_message1(messagerec *m1, char an, int readit, int *next, char *fn)
{
  char n[205],d[81],s[205],s1[81],s2[81],*ss,ch;
  int f,abort,done,end,cur,p,p1,printit,ctrla,centre,i,i1,ansi,ctrld;
  messagerec m;
  char *buf;
  long len,l1;

  hasrip=0;
  if ((buf=malloca(buf_size))==NULL)
    return;
  ss=NULL;
  ansi=0;
  m=*m1;
  *next=0;
  f=-1;
  done=0;
  cur=0;
  end=0;
  abort=0;
  ctrld=0;
  switch(m.storage_type) {
    case 0:
    case 1:
    case 2:
      ss=readfile(&m,fn,&len);
      if (m.storage_type!=2) {
        strcpy(s,syscfg.msgsdir);
        ltoa(m.stored_as,s1,16);
        if (m.storage_type==1) {
          strcat(s,fn);
          strcat(s,"\\");
        }
        strcat(s,s1);
        strcpy(s2,get_string(659));
        strcat(s2,s1);
        if (so())
          pl(s2);
        else {
          strcat(s2,"\r\n");
          outs(s2);
        }
      }
      if (ss==NULL) {
        plan(get_string(89),&abort,next);
        nl();
        bbsfree(buf);
        return;
      }
      p=0;
      while ((ss[p]!=13) && ((long)p<len) && (p<200))
        n[p]=ss[p++];
      n[p]=0;
      ++p;
      p1=0;
      if (ss[p]==10)
        ++p;
      while ((ss[p+p1]!=13) && ((long)p+p1<len) && (p1<60))
        d[p1]=ss[(p1++)+p];
      d[p1]=0;
      cur=p+p1+1;
      break;
    case 255:
      strcpy(s,fn);
      f=sh_open1(s,O_RDONLY | O_BINARY);
      if (f==-1) {
        plan(get_string(89),&abort,next);
        nl();
        bbsfree(buf);
        return;
      }
      sh_lseek(f,m.stored_as,SEEK_SET);
      end=sh_read(f,(void *)buf,buf_size);
      break;
    default:
      /* illegal storage type */
      nl();
      pl(get_string(660));
      nl();
      bbsfree(buf);
      return;
  }
#ifdef RIPDRIVE
  *ripbuffer = 0;         // Clear out
#endif

  irt_name[0]=0;
  if (m.storage_type!=255) {
    g_flags |= g_flag_disable_mci;
    switch(an) {
      default:
      case 0:
        osan(get_string(STR_1002),&abort,next);
        ansic_x(sysinfo.msg_color);
        plan(n,&abort,next);
        strcpy(irt_name,n);
        osan(get_string(STR_661),&abort,next);
        ansic_x(sysinfo.msg_color);
        plan(d,&abort,next);
        if (origin_str[0]) {
          osan(get_string(STR_662),&abort,next);
          plan(origin_str,&abort,next);
        }
        if (origin_str2[0]) {
          osan(get_string(STR_1008),&abort,next);
          plan(origin_str2,&abort,next);
        }
        break;
      case anony_sender:
        if (readit) {
          osan(get_string(STR_1002),&abort,next);
          ansic_x(sysinfo.msg_color);
          sprintf(s,"<<< %s >>>",n);
          plan(s,&abort,next);
          osan(get_string(STR_661),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(d,&abort,next);
        } else {
          osan(get_string(STR_1002),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(get_string(482),&abort,next);
          osan(get_string(STR_661),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(get_string(482),&abort,next);
        }
        break;
      case anony_sender_da:
      case anony_sender_pp:
        if (an==anony_sender_da) {
          osan(get_string(STR_1002),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(get_string(663),&abort,next);
        } else {
          osan(get_string(STR_1002),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(get_string(664),&abort,next);
        }
        if (readit) {
          osan(get_string(STR_1002),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(n,&abort,next);
          osan(get_string(STR_661),&abort,next);
          plan(d,&abort,next);
        } else {
          osan(get_string(STR_661),&abort,next);
          ansic_x(sysinfo.msg_color);
          plan(get_string(482),&abort,next);
        }
        break;
    }
    if (rip_on()) {
      sprintf(s,"!|w000%c271610|#\r ", smally);
      lines_listed = 0;
      comstr(s);
    }
  } else {
    g_flags &= ~g_flag_disable_mci;
  }
#ifdef RIPDRIVE
  rd_coff();
#endif
  nl();
  p=0;
  p1=0;
  done=0;
  printit=0;
  ctrla=0;
  centre=0;
  l1=(long) cur;

  while ((!done) && (!abort) && (!hangup)) {
    switch(m.storage_type) {
      case 0:
      case 1:
      case 2:
        ch=ss[l1];
        if (l1>=len)
          ch=26;
        break;
      case 255:
        if (cur>=end) {
          cur=0;
          end=sh_read(f,(void *)buf,buf_size);
          if (end==0)
            buf[0]=26;
        }
        if ((buf[cur]=='`') && (m.stored_as) && (cur>0) && (buf[cur-1]==10))
          buf[cur]=26;
        ch=buf[cur];
        break;
    }
    if (ch==26)
      done=1;
    else {
      if (hasrip==1)
        if (ch=='|') {
          hasrip=2;
          tmp_disable_pause(1);     // Don't pause in RIPscrip mode
        } else
          hasrip=0;
      if (ch == 1 || ch == '!' || ch == 2)
        hasrip = 1;
      if (ch!=10) {
        if ((ch==13) || (!ch)) {
          printit=1;
        } else if (ch==1)
          ctrla=1;
        else if (ch==2)
          centre=1;
        else if (ch==4)
          ctrld=1;
        else if (ctrld==1) {
          if ((ch>='0') && (ch<='9')) {
            if (thisuser.optional_val<(ch-'0'))
              ctrld=0;
            else
              ctrld=-1;
          } else
            ctrld=0;
        } else {
          if (ch==27) {
            if ((topline) && (screenbottom==24) && (!ansi))
              set_protect(0);
            ansi=1;
            /* lines_listed reset removed: color ANSI sequences (ESC[...m)
             * were resetting the page counter on every line, preventing
             * pause-on-page from ever triggering in ANSI feed files.     */
          }
          s[p++]=ch;
          if ((ch==3) || (ch==8))
            --p1;
          else
            ++p1;
          if ((ch==32) && (!centre))
            printit=1;
        }

        if ((printit) || (ansi) || (p1>=80)) {
          if (centre && (ctrld!=-1)) {
            i1=(thisuser.screenchars-WhereX()-p1)/2;
            for (i=0; (i<i1) && (!abort) && (!hangup); i++) {
#ifdef RIPDRIVE
              if (rd_on() && (ripcode || (hasrip == 2)))
                special = -1;
#endif
              osan(" ",&abort,next);
#ifdef RIPDRIVE
              special = 0;
#endif
            }
          }
          if (p) {
            if (ctrld!=-1) {
              if ((WhereX() + p1 >= thisuser.screenchars) && (!centre) && (!ansi)) {
#ifdef RIPDRIVE
                if (rd_on() && (ripcode || (hasrip == 2))) {
                  strcat(ripbuffer, "\n");
                  remstr("\r\n");
                } else
#endif
                  nl();
              }
              s[p]=0;
#ifdef RIPDRIVE
              if (rd_on() && (ripcode || (hasrip == 2)))
                special = -1;
#endif
              osan(s,&abort,next);
#ifdef RIPDRIVE
              special = 0;
#endif
              if ((ctrla) && (s[p-1]!=32) && (!ansi)) {
                if (WhereX()<(thisuser.screenchars)-1) {
#ifdef RIPDRIVE
                  if (rd_on() && (ripcode || (hasrip == 2))) {
                    strcat(ripbuffer, " ");
                    out_cport(' ');
                  } else
#endif
                    outchr(32);
                } else {
#ifdef RIPDRIVE
                  if (rd_on() && (ripcode || (hasrip == 2))) {
                    strcat(ripbuffer, "\n");
                    remstr("\r\n");
                  } else
#endif
                    nl();
                }
                checka(&abort,next);
              }
            }
            p1=0;
            p=0;
          }
          centre=0;
        }
        if (ch==13) {
          if (ctrla==0) {
            if (ctrld!=-1) {
#ifdef RIPDRIVE
              if (rd_on() && (ripcode || (hasrip == 2))) {
                rd_str(ripbuffer);
                *ripbuffer = 0;
                remstr("\r\n");
              } else {
#endif
                if (ripcode)
                  out_cport('\r');
                else
                  nl();
#ifdef RIPDRIVE                  
              }
#endif                  
              checka(&abort,next);
            }
          } else
            ctrla=0;
          if (printit)
            ctrld=0;
        }
        printit=0;
      } else
        ctrld=0;
    }
    ++cur;
    ++l1;
  }
  if ((!abort) && (p)) {
    s[p]=0;
    pl(s);
  }
  ansic(0);
  nl();
  if (f!=-1)
    sh_close(f);
  if ((m.storage_type==255) && (abort))
    *next=1;
  if ((express) && (abort) && (!(*next)))
    expressabort=1;
  bbsfree(buf);
  if (ss!=NULL)
    bbsfree(ss);
#ifdef RIPDRIVE
  tmp_disable_pause(0);
#endif
  if ((ansi) && (topdata) && (useron))
    topscreen();
  g_flags &= ~g_flag_disable_mci;
#ifdef RIPDRIVE
  rd_con();
#endif
}


int maybeprint(char *fn, int force)
{
  char s[81],s1[81];
  messagerec m;
  int next,i;

  for (i=0; i<3; i++) {
    switch(i) {
      case 0: strcpy(s,languagedir); break;
      case 1: strcpy(s,syscfg.gfilesdir); break;
      case 2: s[0]=0; break;
    }
    m.stored_as=0L;
    m.storage_type=255;
    strcat(s,fn);
    if (strchr(s,'.')==NULL) {
      if (thisuser.sysstatus & sysstatus_ansi) {
        if (thisuser.sysstatus & sysstatus_color) {
          strcpy(s1,s);
          strcat(s1,".ANS");
          if (exist(s1))
            strcat(s,".ANS");
        }
        if (strchr(s,'.')==NULL) {
          strcpy(s1,s);
          strcat(s1,".B&W");
          if (exist(s1))
            strcat(s,".B&W");
          else
            strcat(s,".MSG");
        }
      } else
        strcat(s,".MSG");
    }
    if (exist(s))
      break;
  }
  next=0;
  if (force) {
    read_message1(&m,0,0,&next,s);
  } else {
    if (exist(s)) {
      printfile(fn);
      next=1;
    }
  }
  return(next);
}


int printfile(char *fn)
{
  return(maybeprint(fn, 1));
}

int existprint(unsigned char *fn)
{
  return(maybeprint(fn, 0));
}

void read_message(int n, int *next, int *val)
{
  char s[100];
  postrec p;
  int abort,a,nn;
  slrec ss;

  nl();
  abort=0;
  *next=0;
  msgheader(1);
  if (E_C) {
    sprintf(s,"1%u/%u0",n,nummsgs);
    strcat(s,charstr(12-strlen(stripcolors(s)),'.'));
    strcat(s," 2");
  } else {
    sprintf(s,"%u/%u: ",n,nummsgs);
  }
  osan(s,&abort,next);
  ansic_x(sysinfo.msg_color);
  p=*get_post(n);
  if (p.status & (status_unvalidated | status_delete)) {
    plan(get_string(665),&abort,next);
    if (!lcs())
      return;
    *val |= 1;
    osan(s,&abort,next);
    ansic_x(sysinfo.msg_color);
  }
  strcpy(irt,p.title);
  irt_name[0]=0;
  plan(p.title,&abort,next);
  if ((p.status & status_no_delete) && (lcs())) {
    if (E_C) {
      plan(get_string(666),&abort,next);
    } else {
      plan(get_string(1376),&abort,next);
    }
  }
  if (p.status & status_pending_net) {
    if (E_C) {
      plan(get_string(667),&abort,next);
    } else {
      plan(get_string(1377),&abort,next);
    }
    *val |= 2;
  }
  if (!abort) {
    ss=syscfg.sl[actsl];
    if ((lcs()) || (ss.ability & ability_read_post_anony))
      a=1;
    else
      a=0;
    nn=net_num;

    if (p.status & status_post_new_net)
      set_net_num(p.title[80]);
    setorigin(p.ownersys, p.owneruser);
    read_message1(&(p.msg),(p.anony & 0x0f),a,next,(subboards[curlsub].filename));

    if (nn!=net_num)
      set_net_num(nn);

    ++thisuser.msgread;
    ++msgreadlogon;
  } else
    if ((express) && (!(*next)))
      expressabort=1;
  if (p.qscan>qsc_p[curlsub])
    qsc_p[curlsub]=p.qscan;
  if (p.qscan>=status.qscanptr) {
    lock_status();
    if (p.qscan>=status.qscanptr)
      status.qscanptr=p.qscan+1;
    save_status();
  }
}

void lineadd(messagerec *m1, char *sx, char *aux)
{
  messagerec m;
  char s1[81],s[81],s2[181],*b;
  int f,i,j,new;

  strcpy(s2,sx);
  strcat(s2,"\r\n\x1a");
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
      f=sh_open1(s,O_RDWR | O_BINARY);
      if (f>0) {
        sh_lseek(f,-1L,SEEK_END);
        sh_write(f,(void *)s2,strlen(s2));
        sh_close(f);
      }
      break;
    case 2:
      f=open_file(aux);
      set_gat_section(f,m.stored_as/2048);
      new=1;
      while ((new<2048) && (gat[new]!=0))
        ++new;
      i=(int)(m.stored_as % 2048);
      while (gat[i]!=65535)
        i=gat[i];
      if ((b=malloca(2048))==NULL) {
        sh_close(f);
        return;
      }
      sh_lseek(f,MSG_STARTING + ((long)i)*512L,SEEK_SET);
      sh_read(f,(void *)b,512);
      j=0;
      while ((j<512) && (b[j]!=26))
        ++j;
      strcpy(&(b[j]),s2);
      sh_lseek(f,MSG_STARTING + ((long)i)*512L,SEEK_SET);
      sh_write(f,(void *)b,512);
      if (((j+strlen(s2))>512) && (new!=2048)) {
        sh_lseek(f,MSG_STARTING + ((long)new)*512L,SEEK_SET);
        sh_write(f,(char *)b+512,512);
        gat[new]=65535;
        gat[i]=new;
        save_gat(f);
      }
      bbsfree((void *)b);
      sh_close(f);
      break;
    default:
      /* illegal storage type */
      break;
  }
}

