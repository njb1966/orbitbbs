/*****************************************************************************

				WWIV Version 4
                    Copyright (C) 1988-1995 by Wayne Bell

Distribution of the source code for WWIV, in any form, modified or unmodified,
without PRIOR, WRITTEN APPROVAL by the author, is expressly prohibited.
Distribution of compiled versions of WWIV is limited to copies compiled BY
THE AUTHOR.  Distribution of any copies of WWIV not compiled by the author
is expressly prohibited.

*****************************************************************************/

#ifndef _INSTMSG_H_
#define _INSTMSG_H_

/****************************************************************************/

#define INST_MSG_STRING      1  /* A string to print out to the user */
#define INST_MSG_SHUTDOWN    2  /* Hangs up, ends BBS execution */
#define INST_MSG_SYSMSG      3  /* Message from the system, not a user */

/****************************************************************************/

/*
 * Structure for inter-instance messages. File would be comprised of headers
 * using the following structure, followed by the "message" (if any).
 */

typedef struct {
  unsigned short
    main,                    /* Message main type */
    minor,                   /* Message minor type */
    from_inst,               /* Originating instance */
    from_user,               /* Originating usernum */
    dest_inst;               /* Destination instance */
  unsigned long
    daten,                   /* Secs-since-1970 Unix datetime */
    msg_size,                /* Length of the "message" */
    flags;                   /* Bit-mapped flags */
} inst_msg_header;

#endif

