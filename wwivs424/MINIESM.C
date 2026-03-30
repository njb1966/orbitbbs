/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/


#pragma warn -par
void sysoplog(char *s) {}
void giveup_timeslice(void) {}
#pragma warn +par


#include <ctype.h>
#include "strings.c"
#include "share.c"

char *progname="miniesm";

int debuglevel=0;
int incom=0;                                     /* keep share.c happy */


/****************************************************************************/

void parse_str(char *is, char *os)
{
  char escstr[20];
  int escstrptr=0;
  int escaped=0;
  char ch;


  while ((ch=*is++)!=0) {
    switch(escaped) {
      case 0:
        if (ch=='\\') {
          escaped=1;
        } else {
          *os++=ch;
        }
        break;
      case 1:
        if ((ch>='0') && (ch<='7')) {
          escaped=2;
          escstrptr=0;
          escstr[escstrptr++]=ch;
          escstr[escstrptr]=0;
        } else if ((ch=='x') || (ch=='X')) {
          escaped=3;
          escstrptr=0;
          escstr[escstrptr]=0;
        } else {
          switch(ch) {
            case 'a': ch='\a'; break;
            case 'b': ch='\b'; break;
            case 'f': ch='\f'; break;
            case 'n': ch='\n'; break;
            case 'r': ch='\r'; break;
            case 't': ch='\t'; break;
            case 'v': ch='\v'; break;
          }
          *os++=ch;
          escaped=0;
        }
        break;
      case 2: /* octal */
        if ((ch>='0') && (ch<='7') && (escstrptr<3)) {
          escstr[escstrptr++]=ch;
          escstr[escstrptr]=0;
        } else {
          is--;
          escaped=0;
          *os++=strtol(escstr, NULL, 8);
        }
        break;
      case 3: /* hex */
        if (isxdigit(ch) && (escstrptr<2)) {
          escstr[escstrptr++]=ch;
          escstr[escstrptr]=0;
        } else {
          is--;
          escaped=0;
          *os++=strtol(escstr, NULL, 16);
        }
        break;
    }
  }

  switch(escaped) {
    case 2: /* octal */
      *os++=strtol(escstr, NULL, 8);
      break;
    case 3: /* hex */
      *os++=strtol(escstr, NULL, 16);
      break;
  }

  *os=0;
}

/****************************************************************************/

void descr_str(FILE *f, unsigned char *s)
{
  putc('\"', f);
  for (; *s; s++) {
    /* if ((*s>=' ') && (*s<='~')) { */
    if (*s>=' ') {
      if ((*s=='"') || (*s=='\\'))
        putc('\\', f);
      putc(*s, f);
    } else {
      putc('\\', f);
      switch(*s) {
      case '\a': putc('a', f); break;
      case '\b': putc('b', f); break;
      case '\f': putc('f', f); break;
      case '\n': putc('n', f); break;
      case '\r': putc('r', f); break;
      case '\t': putc('t', f); break;
      case '\v': putc('v', f); break;
      default: fprintf(f,"x%02.2x",((int)*s)&0xff); break;
      }
    }
  }
  putc('\"', f);
}

/****************************************************************************/

void usage(void)
{
  printf("%s stringfn [opts]\n", progname);
  printf("   -Anum      - Add num blank strings\n");
  printf("   -Mnum      - Modify string #num\n");
  printf("   -Lnum      - List string #num\n");
  printf("   -Dfile     - Show differences with another string file\n");

  exit(0);
}

/****************************************************************************/

void main(int argc, char **argv)
{
  int i,i1,i2,nums, nums2;
  char *str_fn="";
  char buf[256], buf1[300], *ss;

  if (argc<2)
    usage();

  str_fn=argv[1];

  if (set_strings_fn(1, NULL, str_fn, 1)) {
    printf("Couldn't open/create '%s'.\n", str_fn);
    exit(-1);
  }

  nums=num_strings(1);

  printf("%s: %d strings\n\n",str_fn, nums);

  for (i=2; i<argc; i++) {
    if (argv[i][0]!='-')
      usage();

    switch(toupper(argv[i][1])) {
      case 'A':
        i1=atoi(argv[i]+2);
        if (i1<1)
          usage();
        put_string(1, nums+i1, "");
        printf("%s: Added %d blank entries\n",str_fn, i1);
        nums=num_strings(1);
        break;

      case 'L':
        i1=atoi(argv[i]+2);
        if (i1<1)
          usage();
        if (i1>nums) {
          printf("%s: Only has %d string entries\n",str_fn, nums);
        } else {
          ss=get_stringx(1,i1);
          printf("String #%u: ", i1);
          descr_str(stdout, ss);
          printf("\n\n");
        }
        break;

      case 'M':
        i1=atoi(argv[i]+2);
        if (i1<1)
          usage();
        if (i1>nums) {
          printf("%s: Only has %d string entries\n",str_fn, nums);
        } else {
          ss=get_stringx(1,i1);
          printf("String #%u: ", i1);
          descr_str(stdout, ss);
          printf("\n\nNew? ");
          fgets(buf1, 295, stdin);
          buf1[295]=0;
          if (buf1[0]) {
            ss=buf1+strlen(buf1)-1;
            if (*ss=='\n')
              *ss=0;

            if (buf1[0]) {
              parse_str(buf1, buf);
              printf("\n\nChange string #%u to:\n",i1);
              descr_str(stdout, buf);
              printf("\n\nSure? ");
              fgets(buf1, 10, stdin);
              if (toupper(buf1[0])=='Y') {
                put_string(1, i1, buf);
                printf("\nUpdated string #%u\n\n",i1);
              }
            }
          }
        }
        break;

      case 'D':
        if (set_strings_fn(2, NULL, argv[i]+2, 0)) {
          printf("Couldn't open %s.\n", argv[i]+2);
        } else {
          nums2=num_strings(2);
          if (nums2>nums)
            i2=nums;
          else
            i2=nums2;
          for (i1=1; i1<=i2; i1++) {
            ss=get_stringx(1,i1);
            if (ss) {
              strcpy(buf, ss);
              ss=get_stringx(2,i1);
              if (ss) {
                strcpy(buf1, ss);
                if (strcmp(buf, buf1)) {
                  printf("\nString #%u:\nFROM: ", i1);
                  descr_str(stdout, buf);
                  printf("\nTO  : ");
                  descr_str(stdout, buf1);
                  printf("\n");
                }
              }
            }
          }
          for (; i1<=nums2; i1++) {
            ss=get_stringx(2, i1);
            printf("\n\nAdd string #%u: ", i1);
            descr_str(stdout, ss);
          }
        }
        break;

      default:
        printf("Unknown argument: '%s'\n",argv[i]);
        usage();
    }

  }

  close_strfiles();

  exit(0);
}
