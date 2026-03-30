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
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <ctype.h>

#include "share.h"
#include "ini.h"


#ifndef bbsmalloc
#ifdef __OS2__
#define bbsmalloc(x) malloc(x)
#define bbsfree(x) free(x)
#else
#define bbsmalloc(x) farmalloc(x)
#define bbsfree(x) farfree(x)
#endif
#endif

typedef struct {
  int num;
  char *buf;
  char **key;
  char **value;
} ini_info_t;

static ini_info_t ini_prim, ini_sec;


/****************************************************************************/
/*
 * Trims a string on both the beginning and end
 */
static unsigned char *trimstr1(unsigned char *s)
{
  int i;
  static char *whitespace=" \r\n\t";

  i=strlen(s);

  while ((i>0) && (strchr(whitespace,s[i-1])))
    --i;

  while ((i>0) && (strchr(whitespace,*s))) {
    memmove(s,s+1,--i);
  }

  s[i]=0;
  return(s);
}

/****************************************************************************/

/* Allocates memory and returns pointer to location containing requested data
 * within a file.
 */

static unsigned char *mallocin_subsection(char *fn, long begin, long end)
{
  unsigned char *ss;
  int f;

  ss=NULL;

  f=sh_open1(fn, O_RDONLY|O_BINARY);
  if (f>0) {
    ss=(unsigned char *)bbsmalloc(end-begin+2);
    if (ss) {
      lseek(f, begin, SEEK_SET);
      read(f,ss,(end-begin+1));
      ss[(end-begin+1)]=0;
    }
    sh_close(f);
  }
  return(ss);
}

/****************************************************************************/

/* Returns begin and end locations for specified subsection within an INI file.
 * If subsection not found then *begin and *end are both set to -1L.
 */

static void find_subsection_area(unsigned char *fn, unsigned char *ssn, long *begin, long *end)
{
  FILE *f;
  unsigned char s[255], tmphdr[81], *ss;
  long pos=0L;

  *begin=*end=-1L;
  sprintf(tmphdr, "[%s]", ssn);

  f=fsh_open(fn, "rt");
  if (!f)
    return;

  /* Get current position */
  pos=0;

  while (fgets(s, sizeof(s)-1, f) != NULL) {
    /* Get rid of CR/LF at end */
    ss=strchr(s,'\n');
    if (ss)
      *ss=0;

    /* Get rid of trailing/leading spaces */
    trimstr1(s);

    /* A comment or blank line? */
    if ((s[0]) && (s[0]!=';')) {

      /* Is it a subsection header? */
      if ((strlen(s)>2) && (s[0]=='[') && (s[strlen(s)-1]==']')) {

        /* Does it match requested subsection name (ssn)? */
        if (strnicmp(&s[0],&tmphdr[0],strlen(tmphdr))==0) {
          if (*begin==-1L)
            *begin=ftell(f);
        } else {
          if (*begin!=-1L) {
            if (*end==-1L) {
              *end=pos-1;
              break;
            }
          }
        }
      }
    }

    /* Update file position pointer */
    pos=ftell(f);
  }

  /* Mark end as end of the file if not already found */
  if ((*begin!=-1L) && (*end==-1L))
    *end=ftell(f)-1;

  fsh_close(f);
}

/****************************************************************************/

/* Reads a subsection from specified .INI file, the subsection being specified
 * by *hdr. Returns a ptr to the subsection data if found and memory is
 * available, else returns NULL.
 */

static unsigned char *read_ini_file(unsigned char *fn, unsigned char *hdr)
{
  long beginloc=-1L, endloc=-1L;
  unsigned char *ss;

  /* Init pointer vars */
  ss=NULL;

  /* Header must be "valid", and file must exist */
  if (strlen(hdr)<1)
    return(NULL);

  /* Get area to read in */
  find_subsection_area(fn, hdr, &beginloc, &endloc);

  /* Validate */
  if (beginloc>=endloc)
    return(NULL);

  /* Allocate pointer to hold data */
  ss=mallocin_subsection(fn, beginloc, endloc);
  return(ss);
}

/****************************************************************************/

static void parse_ini_file(char *buf, ini_info_t *info)
{
  unsigned char *tempb, *ss1, *ss, *ss2;
  unsigned int i1,count=0;

  memset(info, 0, sizeof(ini_info_t));
  info->buf=buf;

  /* first, count # key-value pairs */
  i1=strlen(buf);
  tempb=(unsigned char *)bbsmalloc(i1+20);
  if (!tempb)
    return;

  memmove(tempb, buf, i1);
  tempb[i1]=0;

  for (ss=strtok(tempb,"\r\n"); ss; ss=strtok(NULL,"\r\n")) {
    trimstr1(ss);
    if ((ss[0]==0) || (ss[0]==';'))
      continue;

    ss1=strchr(ss,'=');
    if (ss1) {
      *ss1=0;
      trimstr1(ss);
      if (*ss)
        count++;
    }
  }

  bbsfree(tempb);

  if (!count)
    return;

  /* now, allocate space for key-value pairs */
  info->key=(char **)bbsmalloc(count*sizeof(char *));
  if (!info->key)
    return;
  info->value=(char **)bbsmalloc(count*sizeof(char *));
  if (!info->value) {
    bbsfree(info->key);
    info->key=NULL;
    return;
  }

  /* go through and add in key-value pairs */
  for (ss=strtok(buf,"\r\n"); ss; ss=strtok(NULL,"\r\n")) {
    trimstr1(ss);
    if ((ss[0]==0) || (ss[0]==';'))
      continue;

    ss1=strchr(ss,'=');
    if (ss1) {
      *ss1=0;
      trimstr1(ss);
      if (*ss) {
        ss1++;
        ss2=ss1;
        while ((ss2[0]) && (ss2[1]) && ((ss2=strchr(ss2+1,';'))!=NULL)) {
          if (isspace(*(ss2-1))) {
            *ss2=0;
            break;
          }
        }
        trimstr1(ss1);
        info->key[info->num]=ss;
        info->value[info->num]=ss1;
        info->num++;
      }
    }
  }


}
/****************************************************************************/

/*
 * Frees up any allocated ini files
 */

void ini_done(void)
{
  if (ini_prim.buf) {
    bbsfree(ini_prim.buf);
    if (ini_prim.key)
      bbsfree(ini_prim.key);
    if (ini_prim.value)
      bbsfree(ini_prim.value);
  }
  memset(&ini_prim, 0, sizeof(ini_prim));
  if (ini_sec.buf) {
    bbsfree(ini_sec.buf);
    if (ini_sec.key)
      bbsfree(ini_sec.key);
    if (ini_sec.value)
      bbsfree(ini_sec.value);
  }
  memset(&ini_sec, 0, sizeof(ini_sec));
}

/****************************************************************************/

/*
 * Reads in some ini files
 */

int ini_init(unsigned char *fn, unsigned char *prim, unsigned char *sec)
{
  unsigned char *buf;

  /* first, zap anything there currently */
  ini_done();

  /* read in primary info */
  buf=read_ini_file(fn, prim);

  if (buf) {
    /* parse the data */
    parse_ini_file(buf, &ini_prim);

    /* read in secondary file */
    buf=read_ini_file(fn, sec);
    if (buf)
      parse_ini_file(buf, &ini_sec);

  } else {

    /* read in the secondary info, as the primary one */
    buf=read_ini_file(fn, sec);
    if (buf)
      parse_ini_file(buf, &ini_prim);
  }

  if (ini_prim.buf)
    return(0);
  else
    return(1);
}



/****************************************************************************/

/* Reads a specified value from INI file data (contained in *inidata). The
 * name of the value to read is contained in *value_name. If such a name
 * doesn't exist in this INI file subsection, then *val is NULL, else *val
 * will be set to the string value of that value name. If *val has been set
 * to something, then this function returns 1, else it returns 0.
 */

unsigned char *ini_get(unsigned char *key, int index, unsigned char *index1)
{
  unsigned char key1[81],key2[81];
  int i,i1;
  ini_info_t *info;

  if (!ini_prim.buf || !key || !(*key))
    return(NULL);

  if (index==-1)
    strcpy(key1,key);
  else
    sprintf(key1,"%s[%d]",key,index);
  if (index1)
    sprintf(key2,"%s[%s]",key,index1);
  else
    key2[0]=0;

  /* loop through both sets of data and search them, in order */
  for (i=0; i<=1; i++) {

    /* get pointer to data area to use */
    if (i==0) {
      info=&ini_prim;
    } else if (ini_sec.buf) {
      info=&ini_sec;
    } else
      break;

    /* search for it */
    for (i1=0; i1<info->num; i1++) {
      if ((stricmp(info->key[i1], key1)==0) || (stricmp(info->key[i1], key2)==0)) {
        return(info->value[i1]);
      }
    }
  }


  /* nothing found */
  return(NULL);
}

/****************************************************************************/

unsigned long ini_flags(char yes_char, char *(*func)(int), ini_flags_rec *fs, int num, unsigned long flags) /**/
{
  int i;
  char *ss,*ss1;

  yes_char=toupper(yes_char);

  for (i=0; i<num; i++) {
    ss=func(fs[i].strnum);
    if (ss) {
      ss1=ini_get(ss, -1, NULL);
      if (ss1) {
        if (toupper(*ss1)==yes_char) {
          if (fs[i].sense) {
            flags &= ~fs[i].value;
          } else {
            flags |= fs[i].value;
          }
        } else {
          if (fs[i].sense) {
            flags |= fs[i].value;
          } else {
            flags &= ~fs[i].value;
          }
        }
      }
    }
  }

  return(flags);
}
