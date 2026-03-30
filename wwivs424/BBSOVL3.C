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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <conio.h>


int numlock=NOTNUMBERS;



unsigned char pd_getkey(void)
{
  unsigned short x;

  g_flags |= g_flag_allow_extended;
  x = getkey();
  g_flags &= ~g_flag_allow_extended;

  return x;
}



unsigned get_kb_event(void)
{
  time_t time1, time2;
  unsigned key;

  ctrlbrk(new_control_break); // Disable control break
  
  tleft(1);
  time(&time1);
  

  
  do{
    time(&time2);
    if(difftime(time2,time1) > 180 ) // greater than 3 minutes
    {
      hangup=1;
      return 0;
    }
    if(hangup)
      return 0;

    if(comhit() || kbhitb())
    {
      if(!incom || kbhitb())   // Check for local keys
      {
        key=getch();      // getch();

        if(key==127)
          return(COMMAND_DELETE);
        if(key==22)
          return(COMMAND_INSERT);


        if(key==13 || key==12)
        {
          return(EXECUTE);
        }
        

        if(!key)
        {
          if(kbhit())
          {
            key=getch();      // getch();
            return(key+256);
          }
        }
        else 
        {
          if(numlock==NOTNUMBERS)
          {
            switch(key)
            {
              case '8':
                return(COMMAND_UP);
              case '4':
                return(COMMAND_LEFT);
              case '2':
                return(COMMAND_DOWN);
              case '6':
                return(COMMAND_RIGHT);
              case '0':
                return(COMMAND_INSERT);
              case '.':
                return(COMMAND_DELETE);
              case '9':
                return(COMMAND_PAGEUP);
              case '3':
                return(COMMAND_PAGEDN);
              case '7':
                return(COMMAND_HOME);
              case '1':
                return(COMMAND_END);
            }
          } 
          switch(key)
          {
            case TAB:
              return(TAB); 
            case ESC:
              return(GET_OUT);
            default:
            {
              return(key);
            }
          }
        }
      }
      else if(comhit())
      {
        key=pd_getkey();

        if(key==127)
          return(COMMAND_DELETE);
        if(key==22)
          return(COMMAND_INSERT);



        if(key==13 || key==12)
        {
          return(EXECUTE);
        }
        
        else if(key==ESC)
        {
          time_t time1;
          time_t time2;

          time(&time1);
          time(&time2);

          do
          {
            time(&time2);

            if(comhit())
            {
              key=pd_getkey();
              if(key==OB||key==O)
              {
                key=pd_getkey();

                // Check for a second set of brackets
                if(key==OB||key==O)
                  key=pd_getkey();

                switch(key)
                {
                  case A_UP:
                    return(COMMAND_UP);
                  case A_LEFT:
                    return(COMMAND_LEFT);
                  case A_DOWN:
                    return(COMMAND_DOWN);
                  case A_RIGHT:
                    return(COMMAND_RIGHT);
                  case A_INSERT:
                   return(COMMAND_INSERT);
                  case A_DELETE:
                   return(COMMAND_DELETE);
                  case A_HOME:
                    return(COMMAND_HOME);
                  case A_END:
                    return(COMMAND_END);

                  default:
                   return(key);
                }
              }
              else
                return(GET_OUT);
            }
          } while(difftime(time2,time1) < 1 && !hangup);

          if(difftime(time2,time1) >= 1)  // if no keys followed ESC
            return(GET_OUT);
        }
        else
        {

          if(!key)
          {
            if(kbhitb())
            {
              key=getch();           // getch();
              return(key+256);
            }
          }


          if(numlock==NOTNUMBERS)
          {
            switch(key)
            {
              case '8':
                return(COMMAND_UP);
              case '4':
                return(COMMAND_LEFT);
              case '2':
                return(COMMAND_DOWN);
              case '6':
                return(COMMAND_RIGHT);
              case '0':
                return(COMMAND_INSERT);
              case '.':
                return(COMMAND_DELETE);
              case '9':
                return(COMMAND_PAGEUP);
              case '3':
                return(COMMAND_PAGEDN);
              case '7':
                return(COMMAND_HOME);
              case '1':
                return(COMMAND_END);
            }
          }  
          switch(key)
          {
            default:
              return(key);
          }
        }
      }
      time(&time1);  // reset timer
    }
    else
      giveup_timeslice();
    
  }while(!hangup);
  return 0; // must have hung up
}



// Like onek but does not put cursor down a line
// One key, no carriage return
char onek_ncr(char *s)
{
  char ch;

  while (!strchr(s, ch = upcase(getkey())) && !hangup)
    ;
  if (hangup)
    ch = s[0];
  outchr(ch);
  return (ch);
}


int do_sysop_command(unsigned command)
{
  unsigned x;
  int needredraw=0;

  switch(command)
  {
    // Commands that cause screen to need to be redrawn go here
    case COMMAND_F1:
    case COMMAND_CF1:
    case COMMAND_CF9:
    case COMMAND_F10:
    case COMMAND_CF10:
      needredraw=1;
      x=command-256;
      break;

    // Commands that don't change the screen around
    case COMMAND_SF1:
    case COMMAND_F2:
    case COMMAND_SF2:
    case COMMAND_CF2:
    case COMMAND_F3:
    case COMMAND_SF3:
    case COMMAND_CF3:
    case COMMAND_F4:
    case COMMAND_SF4:
    case COMMAND_CF4:
    case COMMAND_F5:
    case COMMAND_SF5:
    case COMMAND_CF5:
    case COMMAND_F6:
    case COMMAND_SF6:
    case COMMAND_CF6:
    case COMMAND_F7:
    case COMMAND_SF7:
    case COMMAND_CF7:
    case COMMAND_F8:
    case COMMAND_SF8:
    case COMMAND_CF8:
    case COMMAND_F9:
    case COMMAND_SF9:
    case COMMAND_SF10:
      needredraw=0;
      x=command-256;
      break;
      
    default:
      x=0;
      break;
  }
      
  if(x)
  {
    if(needredraw)
      CLS();

    skey(x);

    if(needredraw)
      CLS();
  }
  
  return(needredraw);
}




// All the different functions that output strings to the screen and com
// buffer, all of these need to be modifed to get the following color in
// These don't count functions like npr and prt, since they end up calling
// outstr to do the string printing... wonder if Wayne thinks he needs any
// more functions to print string... hmmm.... <grin>
// outstr  OutString (COM.C)
// pla     Print Line with Abort (BBSUTL.C)
// plal    Print Line with Abort and Limit how much can be printed hmm npr... (BBSUTL.C)
// osan    OutString with Abort and Next (MSGBASE.C)
// plan    Print Line with Abort and Next (MSGBASE.C)

// Oh, colors are...
// |00 - |15 regular dos foreground colors
// |16 - |23 OR |b0- |b7 for regular dos background colors
// There MUST be two numbers following the pipe, ie |01 NOT |1
void colorize_text(char *buffer)
{
  int pos=0, fmt_pos=0;
  int attrib, col;
  char num[3];
  char *formated;
  char color[50];
  unsigned long x;


  x=strlen(buffer);
  x=(x)+100;

  formated=(char *)malloca(x);
  if(!formated)
  {
    return;
  }
  formated[0]=0;


  while(buffer[fmt_pos] && pos < x)
  {
    if(buffer[fmt_pos]=='|')
    {
      if(isdigit(buffer[fmt_pos+1]) || buffer[fmt_pos+1]==' ')
      {
        if(isdigit(buffer[fmt_pos+2]) || (buffer[fmt_pos+2]==' ' && buffer[fmt_pos+1]!=' '))
        {

          if(buffer[fmt_pos+1]!='b' && buffer[fmt_pos+1]!='B')
          {
            strncpy(num, buffer+fmt_pos+1, 2);
            num[2]=0;
            col=atoi(num);

            if(col<16)
            {
              attrib=col;
              buildfor(attrib, color);
            }
            else // For background colors 16-23)
            {
              col-=16;
              attrib=col;
              buildback(attrib, color);
            }

            fmt_pos+=3;
            strcat(formated, color);
            pos+=strlen(color);
          }
          else if(buffer[fmt_pos+1]=='b' || buffer[fmt_pos+1]=='B')
          {
            strncpy(num, buffer+fmt_pos+2, 1);
            num[1]=0;

            col=(atoi(num));
            attrib=col;
            buildback(attrib, color);

            fmt_pos+=3;
            strcat(formated, color);
            pos+=strlen(color);
          }
          else
          {
            formated[pos]=buffer[fmt_pos];
            formated[pos+1]=0;
            ++pos;
            ++fmt_pos;
          }
        }
        else
        {
          formated[pos]=buffer[fmt_pos];
          formated[pos+1]=0;
          ++pos;
          ++fmt_pos;
        }
      }
      else
      {
        formated[pos]=buffer[fmt_pos];
        formated[pos+1]=0;
        ++pos;
        ++fmt_pos;
      }
    }
    else
    {
      formated[pos]=buffer[fmt_pos];
      formated[pos+1]=0;
      ++pos;
      ++fmt_pos;
    }
  }
  formated[pos]=0;
  strcpy(buffer, formated);
  bbsfree(formated);
}

// Calls the above colorize_text to print out '|colors'
// This call and npr_color are only meant for formats that when expanded
// Will be less than 512 characters, this is fine for most cases, but not
// something like an extended description
void sprintf_color(char *buffer, char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vsprintf(buffer, fmt, ap);
  va_end(ap);

  colorize_text(buffer);
}



int replacefile(char *src, char *dst, int stats)
{
  if(exist(dst))
    unlink(dst);

  return(copyfile(src, dst, stats));
}


int copyfile(char *input, char *output, int stats)
{
  int f1, f2, i;
  char *b;
  struct ftime ft;
  statusbarrec sb;
  long fl;


  if(stats) outstr(get_string(1484));

  if ((strcmp(input, output) != 0) && (exist(input)) && (!exist(output)))
  {
    if ((b = malloca(16400)) == NULL)
      return 0;
    f1 = sh_open1(input, O_RDONLY | O_BINARY);
    if(!f1)
      { bbsfree(b); return 0; }

    getftime(f1, &ft);


    if(stats)
    {
      fl=filelength(f1);
      sb.width=20;            sb.amount_per_square=1;
      sb.square_list[0]='²';  sb.empty_space='-';
      sb.side_char1='[';      sb.side_char2=']';
      sb.surround_color=RED;  sb.box_color=YELLOW+(BLACK<<4);
      sb.total_items=fl;      sb.current_item=0;

      statusbar(&sb);
    }



    f2 = sh_open(output, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
    if(!f2)
      { bbsfree(b); sh_close(f1); return 0; }

    i = read(f1, (void *) b, 16384);


    if(stats)
      { sb.current_item+=i; statusbar(&sb); }

    while (i > 0)
    {
      giveup_timeslice();
      write(f2, (void *) b, i);
      i = read(f1, (void *) b, 16384);

      if(stats && i)
       { sb.current_item+=i; statusbar(&sb); }
    }
    f1=sh_close(f1);
    setftime(f2, &ft);
    f2=sh_close(f2);
    bbsfree(b);

    if(stats)
      { nl(); ansic(0); }
  }
#ifdef LAZY_WRITES
  wait1(LAZY_WRITES);
#endif

  return 1;
}




// Just like copyfile, except if the files are on the same drive, it will
// rename them (which is very fast) instead of copying them, also, after
// a copy, it will remove the source file (since it was 'moved')
int movefile(char *src, char *dst, int stats)
{
  char source[201], dest[201];
  int x;

  strcpy(source, src);
  strcpy(dest, dst);
  strupr(source);
  strupr(dest);
  if ((strcmp(source,dest)!=0) && (exist(source))) {
    int which_way=0;
    if ((source[1]!=':') && (dest[1]!=':'))
      which_way=1;
    if ((source[1]==':') && (dest[1]==':') && (source[0]==dest[0]))
      which_way=1;

    if (which_way) {
      rename(source, dest);
      return 2;
    }
  }

  x = copyfile(src, dst, stats);
  unlink(src);

  return x;
}



int check_arc(char *filename)
{
  char header[10];
  char *ext;
  int file;


  file=sh_open1(filename, O_RDONLY);

  if(file<0)
    return(COMPRESSION_UNKNOWN);

  sh_lseek(file, 0, SEEK_SET);
  sh_read(file, (void *)&header, 10);

  sh_close(file);

  switch(header[0])
  {
    case 0x60:
      if((unsigned char)header[1]==(unsigned char)0xEA)
        return(COMPRESSION_ARJ);
      break;

    case 0x1a:
      return(COMPRESSION_PAK);

    case 'P':
      if(header[1]=='K')
        return(COMPRESSION_ZIP);
      break;

    case 'Z':
      if(header[1]== 'O' && header[2] == 'O')
        return(COMPRESSION_ZOO);
      break;
  }

  if (header[0]!='P') {
    header[9]=0;
    if(strstr(header, "-lh"))
      return(COMPRESSION_LHA);

    // Guess on type, using extension to guess
    ext=strstr(filename, ".");

    if(ext)
    {
      ++ext;

      if(stricmp(ext, "ZIP")==0)
        return(COMPRESSION_ZIP);

      if(stricmp(ext, "LHA")==0)
        return(COMPRESSION_LHA);

      if(stricmp(ext, "LZH")==0)
        return(COMPRESSION_LHA);

      if(stricmp(ext, "ZOO")==0)
        return(COMPRESSION_ZOO);

      if(stricmp(ext, "ARC")==0)
        return(COMPRESSION_PAK);

      if(stricmp(ext, "PAK")==0)
        return(COMPRESSION_PAK);

      if(stricmp(ext, "ARJ")==0)
        return(COMPRESSION_ARJ);

      return(COMPRESSION_UNKNOWN);
    }
  }
  return(COMPRESSION_UNKNOWN);
}



// Returns which archive as a number in your archive config
// Returns the first archiver you have listed if unknown
// One thing to note, if an 'arc' is found, it uses pak, and returns that
// The reason being, PAK does all ARC does, plus a little more, I belive
// PAK has its own special modes, but still look like an ARC, thus, an ARC
// Will puke if it sees this
int match_archiver(char *filename)
{
  int x=0;
  char type[4];

  x=check_arc(filename);

  if(x==COMPRESSION_UNKNOWN)
    return 0;


  switch(x)
  {
    case COMPRESSION_ZIP:
      strcpy(type, "ZIP");
      break;

    case COMPRESSION_LHA:
      strcpy(type, "LHA");
      break;

    case COMPRESSION_ARJ:
      strcpy(type, "ARJ");
      break;

    case COMPRESSION_PAK:
      strcpy(type, "PAK");
      break;

    case COMPRESSION_ZOO:
      strcpy(type, "ZOO");
      break;
  }

  x=0;

  while(x<4)
  {
    if(stricmp(type, syscfg.arcs[x].extension)==0)
      return x;

    ++x;
  }

  return 0;
}

long file_daten(char *filename)
{
  int fn;

  struct ftime file;
  struct time  dtime;
  struct date  ddate;


  fn = sh_open1(filename, O_RDONLY);
  if (fn != -1)
  {
    getftime(fn, &file);
    fn=sh_close(fn);

    dtime.ti_hund = 0;
    dtime.ti_min = file.ft_min;
    dtime.ti_hour = file.ft_hour;
    dtime.ti_sec = file.ft_tsec * 2;
    ddate.da_year = file.ft_year + 1980;
    ddate.da_day = file.ft_day;
    ddate.da_mon = file.ft_month;

    return(dostounix(&ddate, &dtime));
  }
  return 0;
}

// Waits amount of secs, or until a key is hit
void wait_sec_or_hit(double seconds)
{
  time_t time1;
  time_t time2;

  time(&time1);
  time(&time2);

  if( seconds == 1 )
    ++seconds;

  while(difftime(time2,time1) < seconds && !hangup && !comhit() && !kbhitb()) {
    giveup_timeslice();
    checkhangup();
    time(&time2);
  }
}





varimenurec * addvarimenu(varimenurec *current, varimenurec *newitem)
{
  varimenurec *temp;
  int cur, amount;

  if(current==NULL)
  {
    current=(varimenurec *) malloc(sizeof(varimenurec));
    memmove((void *)current, (void *)newitem, sizeof(varimenurec));

    current->next=NULL;
    current->amount=1;

    return(current);
  }


  amount=current->amount;
  cur=0;
  temp=current;

  while(cur<amount-1)
  {
    temp=temp->next;
    ++cur;
  }

  temp->next=(varimenurec *) malloc(sizeof(varimenurec));
  temp=temp->next;

  memmove((void *)temp, (void *)newitem, sizeof(varimenurec));
  temp->next=NULL;

  ++current->amount;
  return(current);
}

void varimenuforward(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;

  info->last=info->pos;

  while(!done && !hangup)
  {
    if(info->pos)
    {
      --info->pos;
      temp=getvarimenurec(menu, info->pos);
      if(temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      info->pos=amount-1;

      temp=getvarimenurec(menu, info->pos);
      if(temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }

    ++cur;

    if(cur==amount)
      done=1;
  }
}

void varimenubackward(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;

  info->last=info->pos;

  while(!done && !hangup)
  {
    if(info->pos<amount-1)
    {
      ++info->pos;

      temp=getvarimenurec(menu, info->pos);
      if(temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      info->pos=0;

      temp=getvarimenurec(menu, info->pos);
      if(temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    ++cur;

    if(cur==amount)
      done=1;
  }
}

void varimenuleft(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;
  int looky=0;

  info->last=info->pos;

  temp=getvarimenurec(menu, info->pos);
  looky=temp->ypos;


  while(!done && !hangup)
  {
    int thisy=0;

    if(info->pos)
    {
      --info->pos;
      temp=getvarimenurec(menu, info->pos);

      thisy=temp->ypos;

      if(thisy==looky && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      if(looky)
        --looky;
      else
        looky=60;

      info->pos=amount-1;

      temp=getvarimenurec(menu, info->pos);
      thisy=temp->ypos;

      if(thisy==looky && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }

    ++cur;

//    if(cur==amount)
//      done=1;
  }
  
}

void varimenuup(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;
  int lookx=0;

  info->last=info->pos;

  temp=getvarimenurec(menu, info->pos);
  lookx=temp->xpos;


  while(!done && !hangup)
  {
    int thisx=0;

    if(info->pos)
    {
      --info->pos;
      temp=getvarimenurec(menu, info->pos);
      thisx=temp->xpos;

      if(thisx==lookx && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      if(lookx)
        --lookx;
      else
        lookx=80;



      info->pos=amount-1;

      temp=getvarimenurec(menu, info->pos);
      thisx=temp->xpos;

      if(thisx==lookx && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }

    ++cur;

//    if(cur==amount)
//      done=1;
  }
  
}

void varimenuright(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;
  int looky;

  info->last=info->pos;

  temp=getvarimenurec(menu, info->pos);
  looky=temp->ypos;


  while(!done && !hangup)
  {
    int thisy;

    if(info->pos<amount-1)
    {
      ++info->pos;

      temp=getvarimenurec(menu, info->pos);
      thisy=temp->ypos;

      if(thisy==looky && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      if(looky<=60)
        ++looky;
      else
        looky=0;


      info->pos=0;

      temp=getvarimenurec(menu, info->pos);
      thisy=temp->ypos;

      if(thisy==looky && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    ++cur;

//    if(cur==amount)
//      done=1;
  }
}

void varimenudown(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int done=0, cur=0, amount=menu->amount;
  int lookx;

  info->last=info->pos;

  temp=getvarimenurec(menu, info->pos);
  lookx=temp->xpos;


  while(!done && !hangup)
  {
    int thisx;

    if(info->pos<amount-1)
    {
      ++info->pos;

      temp=getvarimenurec(menu, info->pos);
      thisx=temp->xpos;

      if(thisx==lookx && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    else
    {
      if(lookx<120)
        ++lookx;
      else
        lookx=0;



      info->pos=0;

      temp=getvarimenurec(menu, info->pos);
      thisx=temp->xpos;

      if(thisx==lookx && temp->active)
      {
        done=1;
        redrawvarimenu(COMMON_PARTIAL, menu, info);
      }
    }
    ++cur;

//    if(cur==amount)
//      done=1;
  }
}


void varimenu(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;


  if(info->redraw!=COMMON_NONE)
    redrawvarimenu(info->redraw, menu, info);

  info->redraw=COMMON_NONE;

  while(!hangup)
  {
    temp=getvarimenurec(menu, info->pos);
    switch(temp->type)
    {
      case SHOW_TEXT_TYPE:
        showtext(&temp->types->st, info);
        break;

      case INPUT_EDIT_TYPE:
        input_edit(&temp->types->ie, info);
        break;

      case RADIO_BUTTON_TYPE:
        radio_button(&temp->types->rb, info);
        break;

      case CHECK_BOX_TYPE:
        check_box(&temp->types->cb, info);
        break;


      default:
        break;
    }

    if(varimenu_findhotkey(menu, info) > -1000)
    {
      redrawvarimenu(COMMON_PARTIAL, menu, info);

      temp=getvarimenurec(menu, info->pos);
      info->returnvalue=temp->returnvalue;

      return;
    }

    switch(info->event)
    {
      case COMMAND_STAB:
        varimenuforward(menu, info);
        break;

      case COMMAND_UP:
        varimenuup(menu, info);
        break;

      case COMMAND_LEFT:
        varimenuleft(menu, info);
        break;

      case COMMAND_DOWN:
        varimenudown(menu, info);
        break;

      case COMMAND_RIGHT:
        varimenuright(menu, info);
        break;

      case TAB:
        varimenubackward(menu, info);
        break;

      default:

        temp=getvarimenurec(menu, info->pos);
        info->returnvalue=temp->returnvalue;
        return;
    }
  }
}

varimenurec * getvarimenurec(varimenurec *menu, int which)
{
  varimenurec *temp;
  int pos=0;
  int amount=menu->amount;

  if(which>=amount)
  {
    return NULL;
  }

  temp=menu;
  if(which==0)
    return(temp);



  while(pos<which)
  {
    temp=temp->next;
    if(pos==which)
      return(temp);

    ++pos;
  }

  return(temp);
}




void redrawvarimenu(int redraw, varimenurec *menu, varimenuinfo *info)
{
  varimenurec *temp;
  int cur, savecolor=curatr;
  int mode;



  if(redraw==COMMON_FULL)
  {
    cur=0;
    temp=menu;

    setc(info->mi_normal);


    while((cur<menu->amount) && !hangup)
    {
      if(cur)
        temp=temp->next;
      else
        temp=menu;

      mode = temp->active==FALSE ? COMMON_INACTIVE : cur==info->pos ? COMMON_CURRENT : COMMON_ITEM;


      switch(temp->type)
      {
        case SHOW_TEXT_TYPE:
          redrawshowtext(COMMON_FULL, &temp->types->st, info, mode);
          break;

        case INPUT_EDIT_TYPE:
          redrawinputedit(COMMON_FULL, &temp->types->ie, info, mode);
          break;

        case RADIO_BUTTON_TYPE:
          redrawradiobutton(COMMON_FULL, &temp->types->rb, info, mode);
          break;

        case CHECK_BOX_TYPE:
          redrawcheckbox(COMMON_FULL, &temp->types->cb, info, mode);
          break;

        default:
          break;
      }

      ++cur;
    }
  }
  else if(redraw==COMMON_PARTIAL)
  {
    if(info->pos!=info->last)
    {
      temp=getvarimenurec(menu, info->pos);

      mode = temp->active==FALSE ? COMMON_INACTIVE : COMMON_CURRENT;

      switch(temp->type)
      {
        case SHOW_TEXT_TYPE:
          redrawshowtext(COMMON_NONE, &temp->types->st, info, mode);
          break;

        case INPUT_EDIT_TYPE:
          redrawinputedit(COMMON_NONE, &temp->types->ie, info, mode);
          break;

        case RADIO_BUTTON_TYPE:
          redrawradiobutton(COMMON_NONE, &temp->types->rb, info, mode);
          break;

        case CHECK_BOX_TYPE:
          redrawcheckbox(COMMON_NONE, &temp->types->cb, info, mode);
          break;


        default:
          break;
      }



      temp=getvarimenurec(menu, info->last);

      mode = temp->active==FALSE ? COMMON_INACTIVE : COMMON_ITEM;

      switch(temp->type)
      {
        case SHOW_TEXT_TYPE:
          redrawshowtext(COMMON_FULL, &temp->types->st, info, mode);
          break;

        case INPUT_EDIT_TYPE:
          redrawinputedit(COMMON_FULL, &temp->types->ie, info, mode);
          break;

        case RADIO_BUTTON_TYPE:
          redrawradiobutton(COMMON_FULL, &temp->types->rb, info, mode);
          break;

        case CHECK_BOX_TYPE:
          redrawcheckbox(COMMON_FULL, &temp->types->cb, info, mode);
          break;


        default:
          break;
      }
    }

  }
  else if(redraw==COMMON_NONE)
  {
    return;
  }
  else
  {
    return;
  }

  GOTO_XY(info->xpos, info->ypos);
  setc(savecolor);
}


void killvarimenu(varimenurec *menu)
{
  varimenurec *m[100];
  int cur=1, amount=menu->amount;

  m[0]=menu;

  while(cur < amount)
  {
    m[cur]=m[cur-1]->next;
    ++cur;
  }

  cur=0;
  while((cur < amount) && !hangup)
  {
    if(m[cur]->type==INPUT_EDIT_TYPE)
      kill_inputrec(&m[cur]->types->ie);
    else if(m[cur]->type==SHOW_TEXT_TYPE)
      kill_showtextrec(&m[cur]->types->st);
    else if(m[cur]->type==RADIO_BUTTON_TYPE)
      kill_radiobutton(&m[cur]->types->rb);
    else if(m[cur]->type==CHECK_BOX_TYPE)
      kill_checkbox(&m[cur]->types->cb);



    bbsfree(m[cur]);
    ++cur;
  }
}

void kill_inputrec(inputeditrec *input)
{
  bbsfree(input->text);
  bbsfree(input->show);
}

void kill_showtextrec(showtextrec *text)
{
  bbsfree(text->text);
  bbsfree(text->show);
}

void kill_radiobutton(radiobuttonrec *radio)
{
  if(radio!=radio)
    radio=radio;
}

void kill_checkbox(checkboxrec *check)
{
  if(check!=check)
    check=check;
}






int varimenu_findhotkey(varimenurec *menu, varimenuinfo *info)
{
  varimenurec *m;
  int cur=0, amount=menu->amount;

  unsigned e, h;


  m=menu;

  e=info->event;
  h=m->hotkey;

  if(!h || !e)
    return -1001;

  if(e>='A' && e <='Z')
    e=tolower(e);
  if(h>='A' && h <='Z')
    h=tolower(h);

  if(e==h && e)
  {
    if(menu->type==SHOW_TEXT_TYPE)
      info->event=EXECUTE;
    info->last=info->pos;
    info->pos=cur;

    return 1;
  }

  ++cur;  // we checked pos 0, inc up to pos 1
  while((cur < amount) && !hangup)
  {
    m=m->next;

    h=m->hotkey;
    if(h>='A' && h <='Z')
      h=tolower(h);


    if(e==h && e)
    {
      if(menu->type==SHOW_TEXT_TYPE)
        info->event=EXECUTE;
      info->last=info->pos;
      info->pos=cur;

      return 1;
    }

    ++cur;

  }
  return -1001;
}


void fillvarimenurec(varimenurec *menu, void *rec, int type, unsigned hotkey, int returnvalue, int active)
{
  memset(menu, 0, sizeof(varimenurec));

  (void *)menu->types=rec;

  menu->type=type;

  menu->hotkey=hotkey;
  menu->returnvalue=returnvalue;

  menu->active=active;

  switch(menu->type)
  {
    case SHOW_TEXT_TYPE:
      menu->xpos=menu->types->st.xpos;
      menu->ypos=menu->types->st.ypos;
      break;

    case INPUT_EDIT_TYPE:
      menu->xpos=menu->types->ie.xpos;
      menu->ypos=menu->types->ie.ypos;
      break;

    case RADIO_BUTTON_TYPE:
      menu->xpos=menu->types->rb.xpos;
      menu->ypos=menu->types->rb.ypos;
      break;

    case CHECK_BOX_TYPE:
      menu->xpos=menu->types->cb.xpos;
      menu->ypos=menu->types->cb.ypos;
      break;
  }
}



void build_inputrec(inputeditrec *input, int maxlen, int xpos, int ypos, int width, int insert, int char_case)
{
  memset(input, 0, sizeof(inputeditrec));

  input->pos=0;
  input->curlen=0;
  input->maxlen=maxlen;
  input->first=0;
  input->xpos=xpos;
  input->ypos=ypos;
  input->amount_to_show=width;
  input->curxpos=xpos;
  input->curypos=ypos;
  input->insert=insert;
  input->curmode=-1;
  input->text=(char *)malloc(maxlen+1);
  memset(input->text, 0, maxlen+1);
  input->show=(char *)malloc(width+5);
  memset(input->show, 0, width);
  input->char_case=char_case;
}

void build_radiobuttonrec(radiobuttonrec *radio, int xpos, int ypos, int amount, int pos, int rc, int fc)
{
  memset(radio, 0, sizeof(radiobuttonrec));
  radio->xpos=xpos;
  radio->ypos=ypos;
  radio->amount=amount;
  radio->pos=pos;

  if(pos >= amount)
    pos=amount-1;

  radio->radio_char=rc;
  radio->fill_char=fc;
}

void build_checkboxrec(checkboxrec *check, int xpos, int ypos, int amount, int pos, unsigned bi, int rc, int fc)
{
  memset(check, 0, sizeof(checkboxrec));
  check->xpos=xpos;
  check->ypos=ypos;
  check->amount=amount;
  check->bi.n.ni=bi;

  check->pos=pos;
  if(pos >= amount)
    pos=amount-1;


  check->check_char=rc;
  check->fill_char=fc;
}

void radio_button(radiobuttonrec *radio, varimenuinfo *info)
{
  if(info->redraw!=COMMON_NONE)
    redrawradiobutton(info->redraw, radio, info, COMMON_CURRENT);

  while(!hangup)
  {
    info->event=get_kb_event();

    switch(info->event)
    {
      case COMMAND_UP:
        radio->last=radio->pos;
        if(radio->pos)
        {
          --radio->pos;
          redrawradiobutton(COMMON_PARTIAL, radio, info, COMMON_CURRENT);
        }
        else
        {
          radio->pos=radio->amount-1;
          redrawradiobutton(COMMON_PARTIAL, radio, info, COMMON_CURRENT);
        }
        info->event=0;
        return;
        
      case COMMAND_DOWN:
        radio->last=radio->pos;
        if(radio->pos<radio->amount-1)
        {
          ++radio->pos;
          redrawradiobutton(COMMON_PARTIAL, radio, info, COMMON_CURRENT);
        }
        else
        {
          radio->pos=0;
          redrawradiobutton(COMMON_PARTIAL, radio, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      default:
        return;
    }
  }
}

void check_box(checkboxrec *check, varimenuinfo *info)
{
  if(info->redraw!=COMMON_NONE)
    redrawcheckbox(info->redraw, check, info, COMMON_CURRENT);

  while(!hangup)
  {
    info->event=get_kb_event();

    switch(info->event)
    {
      case COMMAND_UP:
        if(check->pos)
        {
          --check->pos;
          redrawcheckbox(COMMON_NONE, check, info, COMMON_CURRENT);
        }
        else
        {
          check->pos=check->amount-1;
          redrawcheckbox(COMMON_NONE, check, info, COMMON_CURRENT);
        }
        info->event=0;
        return;
        
      case COMMAND_DOWN:
        if(check->pos<check->amount-1)
        {
          ++check->pos;
          redrawcheckbox(COMMON_NONE, check, info, COMMON_CURRENT);
        }
        else
        {
          check->pos=0;
          redrawcheckbox(COMMON_NONE, check, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case ' ':
        if(BitTst(check->bi.n.ni, check->pos))
          check->bi.n.ni=BitClr(check->bi.n.ni, check->pos);
        else
          check->bi.n.ni=BitSet(check->bi.n.ni, check->pos);

        redrawcheckbox(COMMON_PARTIAL, check, info, COMMON_CURRENT);
        info->event=0;
        return;

      default:
        return;

    }
  }
}



void input_edit(inputeditrec *input, varimenuinfo *info)
{
  if(info->redraw!=COMMON_NONE)
    redrawinputedit(info->redraw, input, info, COMMON_CURRENT);

  while(!hangup)
  {
    info->event=get_kb_event();
    input->bs=FALSE;

    switch(info->event)
    {
      case COMMAND_LEFT:
        if(input->pos)
        {
          --input->pos;
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;
        
      case COMMAND_RIGHT:
        if(input->pos<input->curlen)
        {
          ++input->pos;
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_HOME:
        if(input->pos!=0)
        {
          input->pos=0;
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_END:
        if(input->pos!=input->maxlen)
        {
          input->pos=input->curlen;
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_INSERT:
        input->insert=!input->insert;
        info->event=0;
        return;

      case BACKSPACE:
        if(input->pos)
        {
          --input->pos;
          delete_inputedit(input, info);
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_DELETE:
        delete_inputedit(input, info);
        redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        info->event=0;
        return;

      case COMMAND_STAB:
      case TAB:
        return;

      default:

        if(ok_char_inputedit(input, info))
        {
          if(IE_UPPER & input->char_case)
            info->event=toupper(info->event);
          else if(IE_PROPPER & input->char_case)
          {
            if(input->pos && isspace(input->text[input->pos-1]))
              info->event=toupper(info->event);
          }

          add_char_inputedit(input, info);
          redrawinputedit(COMMON_PARTIAL, input, info, COMMON_CURRENT);

          info->event=0;
          return;
        }
        else
          return;
    }
  }
}

int ok_char_inputedit(inputeditrec *input, varimenuinfo *info)
{
  unsigned event;

  event=info->event;

  if(event >= ' ' && event < 255)
    return 1;

  // Cut off warning until I expand this function
  if(input!=input)
    input=input;

  return 0;
}

void add_char_inputedit(inputeditrec *input, varimenuinfo *info)
{
  if(input->insert)
  {
    if(input->curlen<input->maxlen)
    {
      memmove(input->text+input->pos+1, input->text+input->pos, input->curlen-input->pos+1);
      input->text[input->pos]=(char)info->event;
      ++input->pos;
      ++input->curlen;
    }
  }
  else
  {
    if(input->pos<input->maxlen)
    {
      input->text[input->pos]=(char)info->event;

      if(input->pos==input->curlen)
        ++input->curlen;

      ++input->pos;
    }
  }
}


void redrawinputedit(int type, inputeditrec *input, varimenuinfo *info, int mode)
{
  int savecolor=curatr;

  if(input->pos < input->first)
  {
    input->first=input->pos - (input->amount_to_show/2);
    if(input->first<0)
      input->first=0;

    type=COMMON_FULL;
  }
  else if(input->pos - input->first > input->amount_to_show)
  {
    input->first=input->pos - (input->amount_to_show/2);
    if(input->first > input->maxlen)
      input->first=input->maxlen;
    type=COMMON_FULL;
  }
  else if(input->curmode!=mode)
    type=COMMON_FULL;

  input->curmode=mode;

  input->curxpos=input->xpos+(input->pos-input->first);
  input->curypos=input->ypos;

  if(type==COMMON_FULL)
  {
    strncpy(input->show, input->text+input->first, input->amount_to_show);
    input->show[input->amount_to_show]=0;

    pad_string(input->show, input->amount_to_show);
    GOTO_XY(input->xpos, input->ypos);
    setc(mode==COMMON_INACTIVE ? info->inactive_color : mode ==COMMON_ITEM ? info->mi_normal : info->ci_normal);

    if(IE_PASSWORD & input->char_case)
      echo=0;
    else
      echo=1;
    outstr(input->show);
  }
  else if(type==COMMON_PARTIAL)
  {
    int where = input->first+(input->pos-input->first);
    int a=input->amount_to_show-(input->pos-input->first)+1;

    if(input->pos!=input->first)
    {
      strncpy(input->show, input->text+where-1, a+1);
      input->show[a]=0;
      input->show[input->amount_to_show]=0 ;

      if(input->bs && strlen(input->show) != input->amount_to_show)
        strcat(input->show, " ");

      GOTO_XY(input->xpos + input->pos - input->first - 1, input->ypos);
    }
    else
    {
      strncpy(input->show, input->text+where, a);
      input->show[a]=0;
      input->show[input->amount_to_show]=0;

      if(input->bs && strlen(input->show) != input->amount_to_show)
        strcat(input->show, " ");

      GOTO_XY(input->xpos + input->pos - input->first, input->ypos);
    }
    setc(mode==COMMON_INACTIVE ? info->inactive_color : mode ==COMMON_ITEM ? info->mi_normal : info->ci_normal);

    if(IE_PASSWORD & input->char_case)
      echo=0;
    else
      echo=1;
    outstr(input->show);
  }
  else if(type==COMMON_NONE)
  {
  }
  else
  {
  }


  if(mode==COMMON_CURRENT)
  {

    info->xpos=input->xpos + input->pos - input->first;
    info->ypos=input->ypos;
    GOTO_XY(info->xpos, info->ypos);
  }

  input->bs=FALSE;
  setc(savecolor);
}


void delete_inputedit(inputeditrec *input, varimenuinfo *info)
{
  if((input->pos>0 && input->pos==input->curlen) || !input->curlen)
  {
    info->redraw=COMMON_NONE;
    return;
  }

  memmove(input->text+input->pos, input->text+input->pos+1, input->curlen-input->pos);

  --input->curlen;
  input->bs=TRUE;
  info->redraw=COMMON_PARTIAL;
}


void redrawradiobutton(int redraw, radiobuttonrec *radio, varimenuinfo *info, int mode)
{
  int savecolor=curatr;
  int color;
  int x=0;

  if(redraw==COMMON_FULL)
  {

    while((x<radio->amount) && !hangup)
    {
      color = (mode == COMMON_INACTIVE ? info->inactive_color : x == radio->pos ? info->mi_normal : info->ci_normal);
      setc(color);

      GOTO_XY(radio->xpos, radio->ypos+x);
      if(x==radio->pos)
        outchr(radio->radio_char);
      else
        outchr(radio->fill_char);

      ++x;
    }
  }
  else if(redraw!=COMMON_NONE)
  {
    if(radio->pos!=radio->last)
    {
      while((x<radio->amount) && !hangup)
      {
        if(x==radio->pos || x==radio->last)
        {
          color = (mode == COMMON_INACTIVE ? info->inactive_color : x == radio->pos ? info->mi_normal : info->ci_normal);
          setc(color);

          GOTO_XY(radio->xpos, radio->ypos+x);
          if(x==radio->pos)
            outchr(radio->radio_char);
          else
            outchr(radio->fill_char);
        }
        ++x;
      }
    }
  }


  if(mode==COMMON_CURRENT)
  {
    info->xpos=radio->xpos;
    info->ypos=radio->ypos+radio->pos;
    GOTO_XY(info->xpos, info->ypos);
  }

  setc(savecolor);
}

void redrawcheckbox(int redraw, checkboxrec *check, varimenuinfo *info, int mode)
{
  int savecolor=curatr;
  int color;
  int x=0;

  if(redraw==COMMON_FULL)
  {

    while((x<check->amount) && !hangup)
    {
      color = (mode == COMMON_INACTIVE ? info->inactive_color : x == check->pos ? info->mi_normal : info->ci_normal);
      setc(color);

      GOTO_XY(check->xpos, check->ypos+x);
      if(BitTst(check->bi.n.ni, x))
        outchr(check->check_char);
      else
        outchr(check->fill_char);

      ++x;
    }
  }
  else if(redraw!=COMMON_NONE)
  {
    while((x<check->amount) && !hangup)
    {
      if(x==check->pos)
      {
        color = (mode == COMMON_INACTIVE ? info->inactive_color : x == check->pos ? info->mi_normal : info->ci_normal);
        setc(color);

        GOTO_XY(check->xpos, check->ypos+x);
        if(BitTst(check->bi.n.ni, x))
          outchr(check->check_char);
        else
          outchr(check->fill_char);
      }
      ++x;
    }
  }


  if(mode==COMMON_CURRENT)
  {
    info->xpos=check->xpos;
    info->ypos=check->ypos+check->pos;
    GOTO_XY(info->xpos, info->ypos);
  }

  setc(savecolor);
}



void redrawshowtext(int redraw, showtextrec *text, varimenuinfo *info, int mode)
{
  int savecolor=curatr;
  int color;

  strncpy(text->show, text->text, text->amount_to_show);
  text->show[text->amount_to_show]=0;

  justify_string(text->show, text->amount_to_show, text->bg, text->justify);

  color = (mode == COMMON_INACTIVE ? info->inactive_color : mode == COMMON_ITEM ? info->mi_normal : info->ci_normal);
  if(color!=text->cur_color)
    redraw=COMMON_FULL;
  text->cur_color=color;

  if(redraw!=COMMON_NONE)
  {
    setc(color);

    GOTO_XY(text->xpos, text->ypos);
    outstr(text->show);
  }

  if(mode==COMMON_CURRENT)
  {
    info->xpos=text->xpos;
    info->ypos=text->ypos;
    GOTO_XY(info->xpos, info->ypos);
  }

  setc(savecolor);
}

void showtext(showtextrec *text, varimenuinfo *info)
{

  if(info->redraw!=COMMON_NONE)
    redrawshowtext(info->redraw, text, info, COMMON_CURRENT);

  info->event=get_kb_event();
  return;
}

void build_showtextrec(showtextrec *text, int xpos, int ypos, int width, char *t, int justify, int bg)
{
  memset(text, 0, sizeof(showtextrec));

  text->xpos=xpos;
  text->ypos=ypos;
  text->amount_to_show=width;
  text->curxpos=xpos;
  text->curypos=ypos;

  text->justify=justify;
  text->bg=bg;

  text->text=(char *)malloc(width+1);
  strncpy(text->text, t, width);
  text->text[width]=0;

  text->show=(char *)malloc(width+1);
}
void build_inputpicrec(inputpicrec *input, int xpos, int ypos, int width, int insert, char *picture)
{
  memset(input, 0, sizeof(inputpicrec));

  input->pos=0;
  input->curlen=0;
  input->first=0;
  input->xpos=xpos;
  input->ypos=ypos;
  input->amount_to_show=width;
  input->curxpos=xpos;
  input->curypos=ypos;
  input->insert=insert;
  input->curmode=-1;
  input->show=(char *)malloc(width+5);
  memset(input->show, 0, width);
//  input->picture=get_picture(picture, input->maxlen);
  if(picture!=picture)
    picture=picture;
  input->text=(char *)malloc(input->maxlen+1);
  memset(input->text, ' ', input->maxlen+1);

}

void kill_inputpic(inputpicrec *input)
{
  bbsfree(input->text);
  bbsfree(input->show);

  if(input->picture)
    bbsfree(input->picture);
}

void input_picture(inputpicrec *input, varimenuinfo *info)
{
  if(info->redraw!=COMMON_NONE)
    redrawinputpic(info->redraw, input, info, COMMON_CURRENT);

  while(!hangup)
  {
    info->event=get_kb_event();
    input->bs=FALSE;

    switch(info->event)
    {
      case COMMAND_LEFT:
        if(input->pos)
        {
          --input->pos;
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;
        
      case COMMAND_RIGHT:
        if(input->pos<input->curlen)
        {
          ++input->pos;
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_HOME:
        if(input->pos!=0)
        {
          input->pos=0;
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_END:
        if(input->pos!=input->maxlen)
        {
          input->pos=input->curlen;
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case BACKSPACE:
        if(input->pos)
        {
          --input->pos;
          input->text[input->pos]=' ';
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        }
        info->event=0;
        return;

      case COMMAND_DELETE:
        input->text[input->pos]=' ';
        redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);
        info->event=0;
        return;

      case COMMAND_STAB:
      case TAB:
        return;

      default:

        if(ok_char_inputpic(input, info))
        {
          input->text[input->pos]=info->event;
          ++info->pos;
          redrawinputpic(COMMON_PARTIAL, input, info, COMMON_CURRENT);

          info->event=0;
          return;
        }
        else
          return;
    }
  }
}

int ok_char_inputpic(inputpicrec *input, varimenuinfo *info)
{
  unsigned event;

  event=info->event;

  if(event >= ' ' && event < 255)
    return 1;

  // Cut off warning until I expand this function
  if(input!=input)
    input=input;

  return 0;
}


char ** read_picture(char *picture)
{
  int amount;
  int pos=0;
  int x, done=0;
  int next=0;


  while(!done && !hangup)
  {

    x=picture[pos];

    if(!x)
      break;

    switch(x)
    {
      case '!': // Execlude
        next=1;
        ++pos;
        break;

      case '[': // Start range
        while(x!=']' && x)
        {
          ++pos;
          x=picture[pos];
        }
        next=0;
        break;

      case '@': // Predefined type
        next=1;
        ++pos;
        break;

      case '~': // Force next char
        ++pos;
        if(picture[pos])
          ++pos;

        next=0;
        break;

      default:
        next=0;
        ++pos;
        break;
    }

    if(next)
      ++amount;

  }

  return NULL;
}




void redrawinputpic(int type, inputpicrec *input, varimenuinfo *info, int mode)
{
  int savecolor=curatr;

  if(input->pos < input->first)
  {
    input->first=input->pos - (input->amount_to_show/2);
    if(input->first<0)
      input->first=0;

    type=COMMON_FULL;
  }
  else if(input->pos - input->first > input->amount_to_show)
  {
    input->first=input->pos - (input->amount_to_show/2);
    if(input->first > input->maxlen)
      input->first=input->maxlen;
    type=COMMON_FULL;
  }
  else if(input->curmode!=mode)
    type=COMMON_FULL;

  input->curmode=mode;

  input->curxpos=input->xpos+(input->pos-input->first);
  input->curypos=input->ypos;

  if(type==COMMON_FULL)
  {
    strncpy(input->show, input->text+input->first, input->amount_to_show);
    input->show[input->amount_to_show]=0;

    pad_string(input->show, input->amount_to_show);
    GOTO_XY(input->xpos, input->ypos);
    setc(mode==COMMON_INACTIVE ? info->inactive_color : mode ==COMMON_ITEM ? info->mi_normal : info->ci_normal);

    if(IE_PASSWORD & input->char_case)
      echo=0;
    else
      echo=1;
    outstr(input->show);
  }
  else if(type==COMMON_PARTIAL)
  {
    int where = input->first+(input->pos-input->first);
    int a=input->amount_to_show-(input->pos-input->first)+1;

    if(input->pos!=input->first)
    {
      strncpy(input->show, input->text+where-1, a+1);
      input->show[a]=0;
      input->show[input->amount_to_show]=0 ;

      if(input->bs && strlen(input->show) != input->amount_to_show)
        strcat(input->show, " ");

      GOTO_XY(input->xpos + input->pos - input->first - 1, input->ypos);
    }
    else
    {
      strncpy(input->show, input->text+where, a);
      input->show[a]=0;
      input->show[input->amount_to_show]=0;

      if(input->bs && strlen(input->show) != input->amount_to_show)
        strcat(input->show, " ");

      GOTO_XY(input->xpos + input->pos - input->first, input->ypos);
    }
    setc(mode==COMMON_INACTIVE ? info->inactive_color : mode ==COMMON_ITEM ? info->mi_normal : info->ci_normal);

    if(IE_PASSWORD & input->char_case)
      echo=0;
    else
      echo=1;
    outstr(input->show);
  }
  else if(type==COMMON_NONE)
  {
  }
  else
  {
  }


  if(mode==COMMON_CURRENT)
  {

    info->xpos=input->xpos + input->pos - input->first;
    info->ypos=input->ypos;
    GOTO_XY(info->xpos, info->ypos);
  }

  input->bs=FALSE;
  setc(savecolor);
}


