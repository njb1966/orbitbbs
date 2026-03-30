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
#include <string.h>
#include <alloc.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <mem.h>
#include <stdlib.h>

#include "share.h"


#ifndef bbsmalloc
#ifdef __OS2__
#define bbsmalloc(x) malloc(x)
#define bbsfree(x) free(x)
#else
#define bbsmalloc(x) farmalloc(x)
#define bbsfree(x) farfree(x)
#endif
#endif

#define CACHE_LEN 4096
#define WRITE_SUPPORT
#define MAX_STRFILES 8

typedef struct {
  int str_f;
  long str_l;
  long str_o;
  long str_num;
  int allowwrite;
  int stayopen;
  char fn[81];
} strfile_t;

static char str_buf[256];

#ifdef CACHE_LEN

typedef struct {
  int num;
  int nxt;
} strhdr;

static char *str_cache;
static int str_first;
static double str_total, str_hits;

#endif

static strfile_t fileinfo[MAX_STRFILES];


/****************************************************************************/

static void close_strfile(strfile_t *sfi)
{
  if (sfi->str_f>0) {
    sh_close(sfi->str_f);
    sfi->str_f=0;
  }
}

/****************************************************************************/

static int open_strfile(strfile_t *sfi)
{
  unsigned int i;
  unsigned char ch;

  if (sfi->str_f)
    return(sfi->str_f);

  if (sfi->fn[0]==0)
    return(0);

#ifdef WRITE_SUPPORT
  if (sfi->allowwrite)
    sfi->str_f = sh_open(sfi->fn, O_RDWR|O_BINARY|O_CREAT, S_IREAD|S_IWRITE);
  else
    sfi->str_f = sh_open1(sfi->fn, O_RDONLY|O_BINARY);
#else
  sfi->str_f = sh_open1(sfi->fn, O_RDONLY|O_BINARY);
#endif
  if (sfi->str_f<0)
    sfi->str_f=0;
  else {
    if (sfi->str_l==0) {

#ifdef WRITE_SUPPORT
      if ((filelength(sfi->str_f)==0) && (sfi->allowwrite)) {
        i=0xffff;
        write(sfi->str_f,&i,2);
        ch=100;
        write(sfi->str_f,&ch,1);
        lseek(sfi->str_f, 0, SEEK_SET);
      }
#endif

      i=0;
      read(sfi->str_f,&i,2);
      if (i!=0xffff) {
        sfi->str_l=161;
        sfi->str_o=0;
      } else {
        read(sfi->str_f,&ch,1);
        sfi->str_l=ch;
        sfi->str_o=3;
      }
    }
    if (sfi->str_l)
      sfi->str_num=(filelength(sfi->str_f) - sfi->str_o) / sfi->str_l;
  }

  return(sfi->str_f);
}

/****************************************************************************/

int set_strings_fn(int filen, char *dir, char *fn, int allowwrite)
{
  strfile_t *sfi;
  int stayopen;
#ifdef CACHE_LEN
  strhdr *sp;
#endif

  if (filen<0) {
    stayopen=1;
    filen = (-filen);
  } else {
    stayopen=0;
  }

  if ((filen>=MAX_STRFILES) || (filen<0))
    return(-1);

#ifndef WRITE_SUPPORT
  if (allowwrite)
    return(-1);
#endif

  sfi=&(fileinfo[filen]);

  close_strfile(sfi);

  memset(sfi, 0, sizeof(strfile_t));

  if (!fn)
    return(0);

  sfi->allowwrite = allowwrite;
  sfi->stayopen = stayopen;

  if (dir && dir[0]) {
    strcpy(sfi->fn,dir);
    if (sfi->fn[strlen(sfi->fn)-1]!='\\')
      strcat(sfi->fn,"\\");
  } else
    sfi->fn[0]=0;
  strcat(sfi->fn,fn);

  if (open_strfile(sfi)==0) {
    memset(sfi, 0, sizeof(strfile_t));
    return(-1);
  }

  if (filen==0) {

#ifdef CACHE_LEN
    if (!str_cache)
      str_cache=(char *)bbsmalloc(CACHE_LEN);

    if (str_cache) {
      sp=(strhdr *)str_cache;
      sp->num=sp->nxt=-1;
      str_first=0;
      str_total=0.0;
      str_hits=0.0;
    }
#endif
  }

  if (!sfi->stayopen)
    close_strfile(sfi);

  return(0);
}

/****************************************************************************/

#ifdef WRITE_SUPPORT
void put_string(int filen, int n, char *s)
{
  long l;
  strfile_t *sfi;
#ifdef CACHE_LEN
  strhdr *sp;
#endif

  if ((filen>=MAX_STRFILES) || (filen<0))
    return;

  sfi=&(fileinfo[filen]);

  n--;

  if ((n<0) || (!open_strfile(sfi)))
    return;

  memset(str_buf, 0, sizeof(str_buf));

  while (sfi->str_num<n) {
    l = (((long)sfi->str_num) * sfi->str_l) + sfi->str_o;
    lseek(sfi->str_f, l, SEEK_SET);
    write(sfi->str_f, str_buf, sfi->str_l);
    sfi->str_num++;
  }

  l = (((long)n) * sfi->str_l) + sfi->str_o;
  lseek(sfi->str_f, l, SEEK_SET);
  strncpy(str_buf, s, sfi->str_l-1);
  write(sfi->str_f, str_buf, sfi->str_l);

  if (sfi->str_num<=n)
    sfi->str_num=n+1;

#ifdef CACHE_LEN
  if (str_cache) {
    sp=(strhdr *)str_cache;
    sp->num=sp->nxt=-1;
  }
#endif

  if (filen || (!sfi->stayopen))
    close_strfile(sfi);

}
#endif


/****************************************************************************/

int cachestat(void)
{
#ifdef CACHE_LEN
  if (str_total<1.0)
    return(0);

  return((int) ((str_hits*100.0)/str_total));
#else
  return(0);
#endif
}

/****************************************************************************/

char *get_stringx(int filen, int n)
{
  char *ss;
  strfile_t *sfi;
#ifdef CACHE_LEN
  int cur_idx, next_idx, last_idx;
  strhdr *sp, *sp1;
#endif

  n--;

  if ((filen>=MAX_STRFILES) || (filen<0))
    return("");

  sfi=&(fileinfo[filen]);

  if (n<0)
    return("");

  if (n>=sfi->str_num) {
    return("");
  }
#ifdef CACHE_LEN
  if (str_cache && (filen==0)) {
    str_total+=1.0;

    for (last_idx=cur_idx=str_first, sp=(strhdr *) (str_cache+cur_idx);
         sp->num!=-1;
         last_idx=cur_idx, cur_idx=sp->nxt, sp=(strhdr *) (str_cache+cur_idx)) {
      if (sp->num==n) {
        str_hits+=1.0;
        return(str_cache+cur_idx+sizeof(strhdr));
      }
    }
  }
#endif
  if(!open_strfile(sfi))
    return("");

  lseek(sfi->str_f,(((long)n) * sfi->str_l) + sfi->str_o, SEEK_SET);
  read(sfi->str_f, str_buf, sfi->str_l);
  ss=str_buf;

#ifdef CACHE_LEN
  if (str_cache && (filen==0)) {
    next_idx=cur_idx+sizeof(strhdr)+strlen(str_buf)+1;
    if (next_idx+sizeof(strhdr)>CACHE_LEN) {
      sp1=(strhdr *) (str_cache+last_idx);
      sp1->nxt=0;
      sp=(strhdr *) str_cache;
      next_idx -= cur_idx;
      while ((str_first>cur_idx) || (str_first<next_idx+sizeof(strhdr))) {
        sp1=(strhdr *) (str_cache+str_first);
        str_first=sp1->nxt;
      }
      cur_idx=0;
    } else {
      while ((str_first>cur_idx) && (next_idx+sizeof(strhdr)>str_first)) {
        sp1=(strhdr *) (str_cache+str_first);
        str_first=sp1->nxt;
      }
    }
    sp->num=n;
    sp->nxt=next_idx;
    ss=str_cache+cur_idx+sizeof(strhdr);
    strcpy(ss, str_buf);
    sp=(strhdr *)(str_cache+next_idx);
    sp->num=sp->nxt=-1;
  }
#endif

  if (!sfi->stayopen)
    close_strfile(sfi);

  return(ss);
}

/****************************************************************************/

char *get_string(int n)
{
  return(get_stringx(0,n));
}

/****************************************************************************/

int num_strings(int filen)
{
  if ((filen>=MAX_STRFILES) || (filen<0))
    return(0);

  return(fileinfo[filen].str_num);
}

/****************************************************************************/

char *getrandomstring(int filen)
{
  strfile_t *sfi;

  if ((filen>=MAX_STRFILES) || (filen<0))
    return("");

  sfi=&(fileinfo[filen]);

  if (!open_strfile(sfi))
    return("");
  else
    return(get_stringx(filen,1+random((int) ((filelength(sfi->str_f)-sfi->str_o)/sfi->str_l))));
}

/****************************************************************************/

void close_strfiles(void)
{
  int i;

  for (i=0; i<MAX_STRFILES; i++) {
    close_strfile(&(fileinfo[i]));
  }
}
