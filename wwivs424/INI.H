#ifndef _INI_H_
#define _INI_H_

typedef struct {
  int strnum;
  int sense;
  unsigned long value;
} ini_flags_rec;

void ini_done(void);
int ini_init(unsigned char *fn, unsigned char *prim, unsigned char *sec);
unsigned char *ini_get(unsigned char *key, int index, unsigned char *index1);
unsigned long ini_flags(char yes_char,
                        char *(*func)(int),
                        ini_flags_rec *fs,
                        int num,
                        unsigned long flags);


#endif
