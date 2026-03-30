/* RIPspeed support functions, copyright (C) 1994 by Zachary G. Ives.   *
 * All rights reserved.                                                 *
 *                                                                      *
 * Version 1.02, 6/6/94.                                                *
 * Version 1.50, 6/20/94.                                               *
 * For WWIV 4.24, 8/01/94.                                              */

#include "vars.h"
#include "ripint.h"

#pragma hdrstop

#include <dir.h>
#include <dos.h>
#include <ctype.h>

char rd_on(void)
{
#ifdef RIPDRIVE
  return (ripdrive && ((sysinfo.flags & OP_FLAGS_RIPDRIVE_ON) != 0));
#else
  return(0);
#endif
}
/****************************************************************************/


/* Function: return TRUE if rip menus are enabled.
 */
int menu_on(void)
{
//    return (rip && ((sysstatus_expert & thisuser.sysstatus)==0)); 
    return (rip_on() && ((sysstatus_expert & thisuser.sysstatus)==0));
}

/* Function: return TRUE if any RIP support is available.
 */
int rip_on(void)
{
  return (rip && ((thisuser.sysstatus & sysstatus_disable_rip)==0)); 
}


#ifdef RIPDRIVE
void rd_print(char *s)
{
  struct localrip_screeninfo r;
  if (rd_on()) {
    localrip_readscene(s);
    localrip_textinfo(&r);
    screenbottom = r.w_bottom - r.w_top - 1;
  }
}

void rd_str(char *s)
{
  struct localrip_screeninfo r;

  if (rd_on()) {
    localrip_display(s);
    localrip_textinfo(&r);
    screenbottom = r.w_bottom - r.w_top - 1;
  }
}
#endif

/* Function:  save the entire screen, mouse field, and clipboard information
   to the user's hard disk */
void rip_saveall(void)
{
    comr("\1|1\x1b""0000$SAVEALL$");
}

/* Function: restore the screen, mouse field, and clipboard info to the user's
   screen */
void rip_restoreall(void)
{
    comr("\1|1\x1b""0000$RESTOREALL$");
}

/* Function: paste whatever was captured to the clipboard back in place */
void rip_pcb(void)
{
    comr("\1|1\x1b""0000$PCB$");
}

/* Function: cursor on */
void rip_con(void)
{
    comr("\1|1\x1b""0000$CON$|");
}

#ifdef RIPDRIVE
void rd_coff(void)
{
    if (rd_on()) {
      rip_coff();
      localrip_disablemouse(1);
    }
}

void rd_con(void)
{
    if (rd_on()) {
      rip_con();
      localrip_disablemouse(0);
    }
}
#endif

/* Function: cursor off */
void rip_coff(void)
{
    comr("\1|1\x1b""0000$COFF$|");
}

/* Function: save the mouse fields */
void rip_smf(void)
{
    comr("\1|1\x1b""0000$SMF$|");
}

/* Function: restore mouse fields */
void rip_rmf(void)
{
    comr("\1|1\x1b""0000$RMF$|");
}

/* Function: restore text window, and then paste clipboard */
void rip_rtwpcb(void)
{
    comr("\1|1\x1b""0000$RTW$$PCB$|#");
}

/* Function:  show a file in the RIP directory. */
void rip_show(char *s)
{
  char f[68];

  sprintf(f,"%s%s", sysinfo.ripdir, s);
  ripcode = 1;
  maybeprint(f,2);
  ripcode = 0;
}

/* Function:  get a key, but display a [Continue] button on the bottom of the
   screen so the user can click the mouse on it.
*/
void getkeymouse(void)
{
  if (menu_on() && rip_popup && (!rip_subset)) {
    outstr("[A[s");
    rip_smf();
    printmenu(332);
  }

    /* Do not accept a submenu_code marker as a valid "pause-breaker" */
  while (getkey() == submenu_code && (hangup == 0));
  if (menu_on() && rip_popup && (!rip_subset)) {
    printmenu(333);
    rip_rmf();
    outstr("[u[B");
  }
}
/* Function: clear the user's screen with a RIP_RESET code.
   Doesn't display locally.
 */
void rip_cls(void)
{
    if (rip_on())
      comr("\1|*");
}

void out_cport(char c)
{
  if (change_color == 1) {
    change_color = 0;
    if ((c >= '0') && (c <= '9'))
      ansic(c - '0');
    return;
  } else if (change_color == 4) {
    // This byte is reserved for RIPscrip 2 font style commands & codes
    if (c == 15)
      change_color = 5;
    else
      change_color = 0;
    return;
  } else if (change_color == 5) {
    // Control or substitution codes
    change_color = 0;
    remstr(interpret(c));
    return;
  }
  if (c == 3) {
    change_color = 1;
    return;
  } else if (c == 15) {
    change_color = 4;
    return;
  }
  if (outcom)
    outcomch(c);
}

void remstr(char *s)
{
  int i=0;

  checkhangup();
  if (!hangup)
    while (s[i])
      out_cport(s[i++]);
}

/* Function: output a string on the com port, but don't display anything
   locally.  Com-port only version of outstr().
 */
void comstr(char *s)
{
  remstr(s);
#ifdef RIPDRIVE
  rd_str(s);
#endif
}

/* Output to com port with a CR but no LF. */
void comr(char *s)
{
    comstr(s);
    if (rip_ver >= 20000)
      comstr("\r");
    else
      comstr("\r ");
}

/* Output to com port with a CR and LF. */
void comnl(char *s)
{
    comstr(s);
    comstr("\r\n");
}

/* Function: Wait tm seconds for a keypress.  Returns 0 if none during the
   allocated time.
 */
char delaykey(int tm)
/* Delay for time, or until key pressed */
{
  time_t t1,t2;
  char c = 0;

  time(&t1);                /* Record initial time */
  while (((time(&t2) - t1 <= tm)) && (!hangup)) {
    if ((c = inkey())!=0)
      break;
    else {
      checkhangup();
      giveup_timeslice();
    }
  }
  return c;
}

