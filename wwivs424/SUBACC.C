/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/


#ifndef NOT_BBS

#include "vars.h"

#pragma hdrstop

#include "subxtr.h"

#endif


/****************************************************************************/

#ifndef MAX_TO_CACHE
#define MAX_TO_CACHE 15             /* max postrecs to hold in cache */
#endif

static postrec *cache;              /* points to sub cache memory */
static int believe_cache;           /* non-zero if cache is valid */
static unsigned short cache_start;  /* starting msgnum of cache */
static unsigned short last_msgnum;  /* last msgnum read */
static int sub_f=-1;                /* file handle for .sub file, if open */
static char subdat_fn[81];          /* filename of .sub file */

/****************************************************************************/

void close_sub(void)
{
  if (sub_f>=0) {
    sub_f=sh_close(sub_f);
  }
}

/****************************************************************************/

int open_sub(int wr)
{
  postrec p;

  if (sub_f>=0)
    close_sub();

  if (wr) {
#if (NOT_BBS == 2)
    sub_f = open_wc(subdat_fn);
#else
    sub_f = sh_open1(subdat_fn, O_RDWR|O_BINARY|O_CREAT);
#endif
    if (sub_f>=0) {

      /* re-read info from file, to be safe */
      believe_cache = 0;
      sh_lseek(sub_f, 0L, SEEK_SET);
      sh_read(sub_f, &p, sizeof(postrec));
      nummsgs=p.owneruser;
    }
  } else {
    sub_f = sh_open1(subdat_fn, O_RDONLY|O_BINARY);
  }

  return(sub_f);
}

/****************************************************************************/

int iscan1(int si, int quick)
/*
 * Initializes use of a sub value (subboards[], not usub[]).  If quick, then
 * don't worry about anything detailed, just grab qscan info.
 */
{
  postrec p;

  /* make sure we have cache space */
  if (!cache) {
    cache=malloca(MAX_TO_CACHE*sizeof(postrec));
    if (!cache)
      return(0);
  }

  /* forget it if an invalid sub # */
  if ((si<0) || (si>=num_subs))
    return(0);

  /* skip this stuff if being called from the WFC cache code */
  if (!quick) {

#ifndef NOT_BBS
    /* go to correct net # */
    if (xsubs[si].num_nets)
      set_net_num(xsubs[si].nets[0].net_num);
    else
      set_net_num(0);
#endif

    /* see if a sub has changed */
    read_status();
    if (subchg)
      curlsub=-1;

    /* if already have this one set, nothing more to do */
    if (si==curlsub)
      return(1);
  }

  /* sub cache no longer valid */
  believe_cache=0;

  /* set sub filename */
  sprintf(subdat_fn,"%s%s.SUB",syscfg.datadir,subboards[si].filename);

  /* open file, and create it if necessary */
  if (open_sub(0)<0) {
    if (open_sub(1)<0)
      return(0);
    p.owneruser=0;
    sh_write(sub_f,(void *) &p,sizeof(postrec));
  }

  /* set sub */
  curlsub=si;
  subchg=0;
  last_msgnum=0;

  /* read in first rec, specifying # posts */
  sh_lseek(sub_f,0L,SEEK_SET);
  sh_read(sub_f,&p, sizeof(postrec));
  nummsgs=p.owneruser;

#ifndef NOT_BBS
  /* read in sub date, if don't already know it */
  if (sub_dates[si]==0) {
    if (nummsgs) {
      sh_lseek(sub_f, ((long) nummsgs)*sizeof(postrec), SEEK_SET);
      sh_read(sub_f, &p, sizeof(postrec));
      sub_dates[si]=p.qscan;
    } else {
      sub_dates[si]=1;
    }
  }
#endif

  /* close file */
  close_sub();

  /* iscanned correctly */
  return(1);
}


/****************************************************************************/

#ifndef NOT_BBS

int iscan(int b)
/*
 * Initializes use of a sub (usub[] value, not subboards[] value).
 */
{
  return(iscan1(usub[b].subnum, 0));
}

#endif

/****************************************************************************/

postrec *get_post(unsigned short mn)
/*
 * Returns info for a post.  Maintains a cache.  Does not correct anything
 * if the sub has changed.
 */
{
  postrec p;
  int close_file;

  if (mn==0)
    return(NULL);

  if (subchg==1) {
    /* sub has changed (detected in read_status); invalidate cache */
    believe_cache=0;

    /* kludge: subch==2 leaves subch indicating change, but the '2' value */
    /* indicates, to this routine, that it has been handled at this level */
    subchg=2;
  }

  /* see if we need new cache info */
  if ((!believe_cache) ||
      (mn<cache_start) ||
      (mn>=(cache_start+MAX_TO_CACHE))) {

    if (sub_f < 0) {
      /* open the sub data file, if necessary */
      if (open_sub(0)<0)
        return(NULL);

      close_file = 1;
    } else {
      close_file = 0;
    }

    /* re-read # msgs, if needed */
    if (subchg==2) {
      sh_lseek(sub_f, 0L, SEEK_SET);
      sh_read(sub_f, &p, sizeof(postrec));
      nummsgs=p.owneruser;

      /* another kludge: subch==3 indicates we have re-read # msgs also */
      /* only used so we don't do this every time through */
      subchg=3;

      /* adjust msgnum, if it is no longer valid */
      if (mn>nummsgs)
        mn=nummsgs;
    }

    /* select new starting point of cache */
    if (mn >= last_msgnum) {
      /* going forward */

      if (nummsgs<=MAX_TO_CACHE)
        cache_start=1;

      else if (mn>(nummsgs-MAX_TO_CACHE))
        cache_start=nummsgs-MAX_TO_CACHE+1;

      else
        cache_start = mn;

    } else {
      /* going backward */
      if (mn>MAX_TO_CACHE)
        cache_start = mn-MAX_TO_CACHE+1;
      else
        cache_start = 1;
    }

    if (cache_start<1)
      cache_start=1;

    /* read in some sub info */
    sh_lseek(sub_f, ((long) cache_start)*sizeof(postrec), SEEK_SET);
    sh_read(sub_f, cache, MAX_TO_CACHE*sizeof(postrec));

    /* now, close the file, if necessary */
    if (close_file) {
      close_sub();
    }

    /* cache is now valid */
    believe_cache = 1;
  }

  /* error if msg # invalid */
  if ((mn<1) || (mn>nummsgs))
    return(NULL);

  last_msgnum=mn;

  return(cache+(mn-cache_start));
}

/****************************************************************************/

#define BUFSIZE 32000

void delete(int mn)
{
  postrec *p1, p;
  int close_file=0;
  unsigned int nb;
  char *buf;
  long len, l, cp;

  /* open file, if needed */
  if (sub_f <= 0) {
    open_sub(1);
    close_file=1;
  }

  /* see if anything changed */
  read_status();

  if (sub_f >= 0) {
    if ((mn>0) && (mn<=nummsgs)) {
      buf=(char *)malloca(BUFSIZE);
      if (buf) {
        p1=get_post(mn);
        remove_link(&(p1->msg),(subboards[curlsub].filename));

        cp=((long)mn+1)*sizeof(postrec);
        len=((long)(nummsgs+1))*sizeof(postrec);

        do {
          l=len-cp;
          if (l<BUFSIZE)
            nb=(int)l;
          else
            nb=BUFSIZE;
          if (nb) {
            sh_lseek(sub_f, cp, SEEK_SET);
            sh_read(sub_f, buf, nb);
            sh_lseek(sub_f, cp-sizeof(postrec), SEEK_SET);
            sh_write(sub_f, buf, nb);
            cp += nb;
          }
        } while (nb==BUFSIZE);

        /* update # msgs */
        sh_lseek(sub_f, 0L, SEEK_SET);
        sh_read(sub_f, &p, sizeof(postrec));
        p.owneruser--;
        nummsgs=p.owneruser;
        sh_lseek(sub_f, 0L, SEEK_SET);
        sh_write(sub_f, &p, sizeof(postrec));

        /* cache is now invalid */
        believe_cache = 0;

        bbsfree(buf);
      }
    }
  }

  /* close file, if needed */
  if (close_file)
    close_sub();

}


/****************************************************************************/

void write_post(int mn, postrec *pp)
{
  postrec *p1;

  if (sub_f>=0) {
    sh_lseek(sub_f, ((long) mn)*sizeof(postrec), SEEK_SET);
    sh_write(sub_f, pp, sizeof(postrec));
    if (believe_cache) {
      if ((mn>=cache_start) && (mn<(cache_start+MAX_TO_CACHE))) {
        p1=cache+(mn-cache_start);
        if (p1!=pp) {
          *p1=*pp;
        }
      }
    }
  }
}


/****************************************************************************/

void add_post(postrec *pp)
{
  postrec p;
  int close_file=0;

  /* open the sub, if necessary  */

  if (sub_f <= 0) {
    open_sub(1);
    close_file=1;
  }

  if (sub_f>=0) {

    /* get updated info */
    read_status();
    sh_lseek(sub_f, 0L, SEEK_SET);
    sh_read(sub_f, &p, sizeof(postrec));

    /* one more post */
    p.owneruser++;
    nummsgs=p.owneruser;
    sh_lseek(sub_f, 0L, SEEK_SET);
    sh_write(sub_f, &p, sizeof(postrec));

    /* add the new post */
    sh_lseek(sub_f, ((long)nummsgs)*sizeof(postrec), SEEK_SET);
    sh_write(sub_f, pp, sizeof(postrec));

    /* we've modified the sub */
    believe_cache=0;
    subchg=0;
#ifndef NOT_BBS
    sub_dates[curlsub]=pp->qscan;
#endif
  }

  if (close_file)
    close_sub();

}

/****************************************************************************/

#ifndef NOT_BBS

static int postsame(postrec *p1, postrec *p2)
{
  if (p1 && p2 &&
      (p1->daten==p2->daten) &&
      (p1->qscan==p2->qscan) &&
      (p1->ownersys==p2->ownersys) &&
      (p1->owneruser==p2->owneruser) &&
      (strcmp(p1->title, p2->title)==0))
    return(1);
  else
    return(0);
}

void resynch(int subnum, int *msgnum, postrec *pp)
{
  postrec p, *pp1;
  int i;

  /* don't care about this param now */
  i=subnum;

  if (pp)
    p=*pp;
  else {
    pp1=get_post(*msgnum);
    if (!pp1)
      return;
    p=*pp1;
  }

  read_status();

  if (subchg || pp) {

    pp1=get_post(*msgnum);
    if (postsame(pp1,&p)) {
      return;
    } else if (!pp1 || (p.qscan<pp1->qscan)) {
      if (*msgnum>nummsgs)
        *msgnum=nummsgs+1;
      for (i=*msgnum-1; i>0; i--) {
        pp1=get_post(i);
        if (postsame(&p, pp1) || (p.qscan>=pp1->qscan)) {
          *msgnum=i;
          return;
        }
      }
      *msgnum=0;
    } else {
      for (i=*msgnum+1; i<=nummsgs; i++) {
        pp1=get_post(i);
        if (postsame(&p, pp1) || (p.qscan<=pp1->qscan)) {
          *msgnum=i;
          return;
        }
      }
      *msgnum=nummsgs;
    }
  }
}

#endif
