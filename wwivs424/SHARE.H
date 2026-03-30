/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.


*****************************************************************************/


extern int debuglevel;
int sh_open(char *path, int file_access, unsigned mode);
int sh_open1(char *path, int access);
FILE *fsh_open(char *path, char *mode);
int sh_close(int f);
void fsh_close(FILE *f);
