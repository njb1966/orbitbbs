#ifndef _QWK_H_
#define _QWK_H_


#define QWK_DIRECTORY (syscfgovr.batchdir)
/* #define QWK_DIRECTORY (sysinfo.qwk_dir) */


#define qwk_iscan(x)         (iscan1(usub[x].subnum, 1))
#define qwk_iscan_literal(x) (iscan1(x, 1))


// If you have a HUGE transfer section, this define will not read extended
// descriptions
// #define HUGE_TRAN


/* #define SLOWER_BUT_SAFER */

#define MAXMAIL 255
#define EMAIL_STORAGE 2

#define append_block(file, memory, size) sh_write(file, memory, size)
#define SETREC(f,i)  sh_lseek(f,((long) (i))*((long)sizeof(uploadsrec)),SEEK_SET);
#define SET_BLOCK(file, pos, size) sh_lseek(file, (long)pos * (long)size, SEEK_SET)

#define GATSECLEN (4096L+2048L*512L)
#ifndef MSG_STARTING
#define MSG_STARTING (((long)gat_section)*GATSECLEN + 4096L)
#endif

#define DOTS 5



#define MAX_BULLETINS 50
#define BULL_SIZE     81
#define BNAME_SIZE    13


// Give us 3000 extra bytes to play with in the message text
#define PAD_SPACE 3000

extern int numlock;


struct qwk_record
{
  char status;   // ' ' for public

  char msgnum[7]; // all strings are space padded
  char date[8];
  char time[5];

  char to[25]; // Uppercase, left justified
  char from[25]; // Uppercase left justified
  char subject[25];
  char password[12];
  char reference[8];

  char amount_blocks[6];
  char flag;

  unsigned int conf_num;
  unsigned int logical_num;
  char tagline;
};


struct qwk_index
{
  float pos;
  char nouse;
};

struct qwk_junk
{
  int qwk_rec_num;
  int qwk_rec_pos;

  // File number for the MESSAGES.DAT file
  int file;

  // File number for the *.NDX files
  int index;
  int cursub;
  int personal;  // personal.ndx
  int zero;      // 000.ndx for email

  struct qwk_record qwk_rec;
  struct qwk_index qwk_ndx;

  char abort;

  char in_email;
  char email_title[25];
};

struct qwk_config
{
  long fu;
  long timesd;
  long timesu;
  unsigned max_msgs;

  char hello[13];
  char news[13];
  char bye[13];
  char packet_name[9];
  char res[190];

  int  amount_blts;
  char *blt[MAX_BULLETINS];
  char *bltname[MAX_BULLETINS];
};

struct qwk_sub_conf
{
  int import_num;
  char import_name[14];
  int to_num;
  char to_name[41];
};

enum CONFIG_QWK_RETURNS
{
  QWK_SELECT,
  QWK_ARCHIVER,
  QWK_MAX_PACKET,
  QWK_MAX_SUB,
  QWK_SELECT_PROTOCOL,
  QWK_QUIT_SAVE,
  QWK_ABORT
};


#endif
