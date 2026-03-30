/*************************************************************************
 RIPINT.H:  RIPdrive interface function declarations and data types.
 
 Copyright (C) 1994 by Zachary G. Ives.  All Rights Reserved.
 
 Licensed to Wayne Bell and WWIV Software Services for WWIV 4.24.
 *************************************************************************/

struct localrip_screeninfo {
  char c_row;
  char c_column;
  char w_top;
  char w_bottom;
  char w_left;
  char w_right;
  char t_foreground;
  char t_background;
};

unsigned int far localrip_detect(void);
unsigned int far localrip_activate(char *rippth, char *fontpth);
void far localrip_disablemouse(int dis);
void far localrip_keyfilter(int key, char far *keybuf);
void far localrip_display(char far *str);
void far localrip_readscene(char far *file);
void far localrip_textinfo(struct localrip_screeninfo far *r);
void far localrip_deactivate(void);
char far localrip_ripmode(void);
void far localansi_string(char *s);
void far localansi_char(char c);
