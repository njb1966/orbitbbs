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

#include "vardec1.h"

#include <stdarg.h>
#include <ctype.h>
#include <conio.h>



void CLS(void)
{
  if (okansi())
    outstr("\x1b[2J");
  else
    outchr(12);
}

void CURSORUP( int x )
{
  npr("\033[%uA",(x));
}

void CURSORDOWN( int x )
{
  npr("\033[%uB",(x));
}

void CURSORRIGHT(int x)
{
  npr("\033[%uC",(x));
}

void CURSORLEFT(int x)
{
  npr("\033[%uD",(x));
}

char *justify_string(char *string, int length, int bg, int type)
{
  int x,x1;

  string[length]=0;
  x=strlen(string);

  if (x<length) {

    if (type==JUSTIFY_LEFT) {

      memset(string+x, bg, length-x);

    } else if (type==JUSTIFY_RIGHT) {

      memmove(string+length-x, string, x);
      memset(string, bg, length-x);

    } else if (type==JUSTIFY_CENTER) {

      x1=(length-x)/2;

      memmove(string+x1, string, x);
      memset(string, bg, x1);
      memset(string+x+x1, bg, length-(x+x1));

    }
  }
  return(string);
}


unsigned char *trimstr1(unsigned char *s)
{
  int i,i1;

  /* find real end of it */
  i=strlen(s);
  while ((i>0) && (isspace(s[i-1])))
    --i;

  /* find real beginning */
  i1=0;
  while ((i1<i) && (isspace(s[i1])))
    ++i1;

  /* knock spaces off the length */
  i -= i1;

  /* move over the desired subsection */
  memmove(s, s+i1, i);

  /* ensure null-terminated */
  s[i]=0;

  return(s);
}

void outstr_color(char *s)
{
  g_flags |= g_flag_pipe_colors;
  outstr(s);
  g_flags &= ~g_flag_pipe_colors;
}

void npr_color(char *fmt, ...)
{
  va_list ap;
  char s[512];

  va_start(ap, fmt);
  vsprintf(s, fmt, ap);
  va_end(ap);

  outstr_color(s);

}

void repeat_char(unsigned char x, int amount)
{
  int cur=0;

  while(cur<amount && !hangup)
  {
    outchr(x);
    ++cur;
  }
}

char *strstr_nocase(char *s1, char *s2)
{
  int len=strlen(s2), pos=0;

  while(s1[pos])
  {
    if(strncmpi(s1+pos, s2, len)==0)
      return(s1+pos);

    ++pos;
  }
  return NULL;
}

void statusbar(statusbarrec *sb)
{
  float pos;
  int maj_pos, min_pos;
  int total_fractions=(sb->width-1)*sb->amount_per_square;


  if(sb->current_item==0) // Initialize
  {
    int x=0;

    ansic(sb->surround_color);
    outchr(sb->side_char1);

    setc(sb->box_color);
    while(x<sb->width)
    {
      outchr(sb->empty_space);
      ++x;
    }

    ansic(sb->surround_color);
    outchr(sb->side_char2);

    if(okansi())
      CURSORLEFT(sb->width);
    else
    {
      x=0;
      while(x<sb->width+1)
      {
        outchr('\b');
        ++x;
      }
    }

    sb->last_maj_pos=0;
    sb->last_min_pos=0;

    return;
  }

  pos=((float)sb->current_item/sb->total_items);
  pos=pos * total_fractions;

  maj_pos=pos/sb->amount_per_square;
  min_pos=pos-(maj_pos*sb->amount_per_square);

  if(min_pos==0)
    min_pos=sb->amount_per_square-1;
  else
    --min_pos;

  if(maj_pos==sb->last_maj_pos)
  {
    if(min_pos==sb->last_min_pos)
      return;

    setc(sb->box_color);
    outchr('\b');
    outchr(sb->square_list[min_pos]);

    sb->last_min_pos=min_pos;

    return;
  }

  setc(sb->box_color);
  outchr('\b');
  outchr(sb->square_list[sb->amount_per_square-1]);
  sb->last_min_pos=min_pos;

  ++sb->last_maj_pos;
  while(sb->last_maj_pos < maj_pos)
  {
    ++sb->last_maj_pos;
    outchr(sb->square_list[sb->amount_per_square-1]);
  }
  outchr(sb->square_list[min_pos]);

  sb->last_maj_pos=maj_pos;

  return;
}

void strip_heart_colors(char *text)
{
  int pos=0;
  int len=strlen(text);

  while(pos<len && text[pos] != 26 && !hangup)
  {
    if(text[pos]==3)
    {
      memmove(text+pos, text+pos+2, len-pos);
      --len;
      --len;
    }
    else
      ++pos;
  }
}

// Set up a new control break handler so that my getch's won't break out of the bbs
int new_control_break(void)
{
  sysoplog("Control Break pressed.  Program NOT aborting");
  return 1;
}


