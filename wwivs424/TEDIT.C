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



#define utoa(s,v,r) ultoa((unsigned long)(s),v,r)


struct line *read_file(char *fn, int *numlines)
{
  char b[1024],s[160];
  int f,nb,cb,sp,oor;
  struct line *l,*l1,*topline;

  *numlines=-1;
  oor=0;
  topline=NULL;
  f=sh_open1(fn,O_RDWR | O_BINARY);
  if (f<0) {
    nl();
    pl(get_string(89));
    nl();
    return(NULL);
  }
  nb=sh_read(f,(void *)b,1024);
  cb=0;
  sp=0;
  *numlines=0;
  while ((nb) && (!oor)) {
    if (b[cb]==13) {
      s[sp]=0;
      sp=0;
      if (bbscoreleft()<10240) {
        nl();
        pl(get_string(90));
        nl();
        oor=1;
      } else {
        l1=(struct line *)malloca((long) sizeof(struct line));
        strcpy((l1->text),s);
        l1->next=NULL;
        l1->prev=NULL;
        if (topline==NULL) {
          topline=l1;
          l=l1;
        } else {
          l->next=l1;
          l1->prev=l;
          l=l1;
        }
        ++(*numlines);
      }
    } else {
      if (b[cb]!=10)
        s[sp++]=b[cb];
    }
    ++cb;
    if (cb==nb) {
      nb=sh_read(f,(void *)b,1024);
      cb=0;
    }
  }
  return(topline);
}

void kill_file(struct line *topline)
{
  struct line *l,*l1;

  l=topline;
  while (l!=NULL) {
    l1=l->next;
    bbsfree((void *)l);
    l=l1;
  }
}

void save_file(char *fn, struct line *topline)
{
  int f;
  char s[170];
  struct line *l;

  nl();
  pl(get_string(91));
  f=sh_open(fn,O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  if (f>0) {
    l=topline;
    while (l!=NULL) {
      strcpy(s,(l->text));
      strcat(s,"\r\n");
      sh_write(f,(void *)s,strlen(s));
      l=l->next;
    }
    s[0]=26;
    sh_write(f,(void *)s,1);
    sh_close(f);
  } else {
    pl(get_string(1526));
  }
}

int printl(int n, struct line *l)
{
  int abort;
  char s[160];

  abort=0;
  if (n<1)
    strcpy(s,get_string(92));
  else
    if (l==NULL)
      strcpy(s,get_string(93));
    else
      sprintf(s,"%3d: %s",n,(l->text));
  pla(s,&abort);
  if (abort)
    nl();
  return(abort);
}


void tedit(char *fn)
{
  struct line *topline,*l,*c,*bottomline;
  int numlines,curline,i,i1,done,save;
  char s[160],s1[160],rollover[160];

  rollover[0]=0;
  thisuser.screenchars -= 5;
  topline=NULL;
  numlines=0;
  curline=0;
  topline=read_file(fn,&numlines);
  npr("%d ",numlines);
  pl(get_string(94));
  nl();
  done=0;
  save=0;
  if (numlines>0) {
    curline=1;
    bottomline=topline;
    while ((bottomline->next)!=NULL)
      bottomline=bottomline->next;
  } else
    curline=0;
  if (numlines<1)
    numlines=0;
  c=topline;
  printl(curline,c);
  do {
    outstr(":");
    input(s,10);
    i=atoi(s);
    if ((s[0]>='0') && (s[0]<='9')) {
      i-=curline;
      if (i==0)
        strcpy(s,"~");
      else
        if (i>0) {
          s[0]='+';
          itoa(i,&(s[1]),10);
        } else {
          itoa(i,s,10);
        }
    }
    if (s[0]==0)
      strcpy(s,"+");
    switch(s[0]) {
      case '?':
        printmenu(11);
        break;
      case 'Q':
        done=1;
        save=0;
        break;
      case 'S':
        done=1;
        save=1;
        break;
      case 'L':
        i=curline;
        l=c;
        while ((l!=NULL) && (printl(i,l)==0)) {
          l=l->next;
          ++i;
        }
        break;
      case 'T':
        c=topline;
        if (topline==NULL)
          curline=0;
        else
          curline=1;
        printl(curline,c);
        break;
      case 'B':
        c=NULL;
        curline=numlines+1;
        printl(curline,c);
        break;
      case '-':
        if (curline) {
          i=atoi(&(s[1]));
          if (i==0)
            i=1;
          for (i1=0; (i1<i) && (curline); i1++) {
            if (c==NULL)
              c=bottomline;
            else
              c=c->prev;
            --curline;
          }
        }
        printl(curline,c);
        break;
      case '+':
        if (curline!=numlines+1) {
          i=atoi(&(s[1]));
          if (i==0)
            i=1;
          for (i1=0; (i1<i) && (curline!=numlines+1); i1++) {
            if (c==NULL)
              c=topline;
            else
              c=c->next;
            ++curline;
          }
        }
        printl(curline,c);
        break;
      case 'D':
        i=atoi(&(s[1]));
        if (i==0)
          i=1;
        for (i1=0; (i1<i) && (c!=NULL); i1++) {
          if (c->prev==NULL)
            if (c->next==NULL) {
              topline=NULL;
              bottomline=NULL;
            } else
              topline=c->next;
          else
            if (c->next==NULL)
              bottomline=c->prev;
          l=c;
          if (c->prev!=NULL)
            c->prev->next=c->next;
          if (c->next!=NULL)
            c->next->prev=c->prev;
          c=c->next;
          bbsfree((void *)l);
          --numlines;
        }
        printl(curline,c);
        break;
      case 'P':
        printl(curline,c);
        break;
      case 'C':
        nl();
        prt(5,get_string(95));
        if (yn()) {
          kill_file(topline);
          topline=NULL;
          bottomline=NULL;
          c=NULL;
          numlines=0;
          curline=0;
          pl(get_string(96));
        }
        break;
      case 'I':
        nl();
        pl(get_string(97));
        i1=0;
        do {
          if (bbscoreleft()<10240) {
            nl();
            pl(get_string(98));
            nl();
            i1=1;
          } else {
            sprintf(s1,"%3d: ",curline);
            if (curline<1)
              strcpy(s1,"  1: ");
            outstr(s1);
            inli(s1,rollover,150,1);
            if (strcmp(s1,".")==0)
              i1=1;
            else {
              l=(struct line *)malloca(sizeof(struct line));
              strcpy((l->text),s1);
              if (c!=NULL) {
                l->next=c;
                if (c->prev!=NULL) {
                  c->prev->next=l;
                  l->prev=c->prev;
                  c->prev=l;
                } else {
                  c->prev=l;
                  l->prev=NULL;
                  topline=l;
                }
              } else {
                if (topline==NULL) {
                  l->prev=NULL;
                  l->next=NULL;
                  topline=l;
                  bottomline=l;
                  curline=1;
                } else
                  if (curline==numlines+1) {
                    l->prev=bottomline;
                    bottomline->next=l;
                    l->next=NULL;
                    bottomline=l;
                  } else {
                    l->prev=NULL;
                    l->next=topline;
                    topline->prev=l;
                    c=topline;
                    topline=l;
                    curline=1;
                  }
              }
              ++numlines;
              ++curline;
            }
          }
        } while ((!i1) && (!hangup));
        break;
    }
  } while ((!done) && (!hangup));
  thisuser.screenchars += 5;
  if (save)
    save_file(fn,topline);
  kill_file(topline);
}


