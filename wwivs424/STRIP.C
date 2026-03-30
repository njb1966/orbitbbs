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
#include <stdlib.h>

void strip_file(char *fn, FILE *out)
{
  FILE *f;
  char s[161],*ss;

  f=fopen(fn,"r");
  if (!f)
    return;

  fprintf(out,"\n\n/* File: %s */\n\n",fn);

  while (fgets(s,160,f)) {
    ss=strchr(s,'\n');
    if (ss)
      *ss=0;
    if ((s[0]) && (strchr("{}\t/# ",s[0])==NULL) && (s[strlen(s)-1]==')')) {
      if (strncmp(s,"static",6))
        fprintf(out,"%s;\n",s);
    }
  }
  fclose(f);
}


/****************************************************************************/

void main(int argc, char *argv[])
{
  int i,i1;
  FILE *out, *tmpin;
  char *ss,s[161];

  if (argc!=3) {
    printf("Run the STRIP program only from the makefile.\n\n");
    exit(-1);
  }

  out=fopen(argv[1],"w");
  fprintf(out,"#ifndef _FCNS_H_\n#define _FCNS_H_\n\n");
  fprintf(out,"#include \"vardec.h\"\n#include \"net.h\"\n");
  fprintf(out,"#include \"qwk.h\"\n");

  tmpin = fopen(argv[2],"r");
  do {
    i1=fscanf(tmpin,"%s",s);
    if (i1>0) {
      if ((ss=strstr(s,".obj"))!=NULL) {
        *ss=0;
        strcat(s,".c");
        ss=strrchr(s,'\\');
        if (!ss)
          ss=s;
        else
          ++ss;
      } else ss=s;
      strip_file(ss,out);
    }
  } while (i1>0);
  fclose(tmpin);

  fprintf(out,"\n#endif\n");
  fclose(out);

}
