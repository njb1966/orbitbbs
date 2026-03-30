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

#include "subxtr.h"
#include "vardec1.h"

#include <conio.h>
#include <ctype.h>


#ifdef STATS
#include <sys\timeb.h>
#endif


char *QWKFrom="0QWKFrom:";
int qwk_bi_mode;
static int qwk_percent;
static char qwk_opened_filename[81];
static int qwk_opened_file;
static unsigned max_msgs;





void build_qwk_packet(void)
{
  struct qwk_junk qwk_info;
  struct qwk_config qwk_cfg;
  char filename[201];
  int i, msgs_ok, abort=0, save_conf=0;
  long *save_qsc_p;

  save_qsc_p=qwk_save_qscan();
  if(!save_qsc_p)
    return;

  remove_from_temp("*.*", QWK_DIRECTORY, 0);

#ifdef STATS
{
  timeb time1, time2;
  long double diff1, diff2;
  float diff3;
  char temp[81];
  ftime(&time1);
#endif



  if ((uconfsub[1].confnum!=-1) && (okconf(&thisuser)))
  {
    save_conf=1;
    tmp_disable_conf(1);
  }
  tmp_disable_pause(1);


  read_qwk_cfg(&qwk_cfg);
  max_msgs=qwk_cfg.max_msgs;
  if(thisuser.qwk_max_msgs < max_msgs && thisuser.qwk_max_msgs)
    max_msgs=thisuser.qwk_max_msgs;

  if(!qwk_cfg.fu)
    qwk_cfg.fu=time(NULL);


  ++qwk_cfg.timesd;
  write_qwk_cfg(&qwk_cfg);
  close_qwk_cfg(&qwk_cfg);


  write_inst(INST_LOC_QWK, usub[cursub].subnum, INST_FLAGS_ONLINE);
  read_status();


  sprintf(filename, "%sMESSAGES.DAT", QWK_DIRECTORY);
  qwk_info.file=sh_open1(filename, O_RDWR | O_BINARY | O_CREAT);

  if(qwk_info.file<1)
  {
    pl(get_string(1530));
    sysoplog(get_stringx(1,120));
    return;
  }


  // Setup my optimized open files
  qwk_opened_filename[0]=0;
  qwk_opened_file=-1;

  // Setup index and other values
  qwk_info.index=-1;
  qwk_info.cursub=-1;

  qwk_info.in_email=0;
  qwk_info.personal=-1;
  qwk_info.zero=-1;


  memset(&qwk_info.qwk_rec, ' ', sizeof(qwk_info.qwk_rec));
  strcpy((char *)&qwk_info.qwk_rec, get_string(1532));
  append_block(qwk_info.file, (void *)&qwk_info.qwk_rec, sizeof(qwk_info.qwk_rec));

  // Logical record number
  qwk_info.qwk_rec_num=1;
  qwk_info.qwk_rec_pos=2;

  qwk_info.abort=0;

  if(!thisuser.qwk_dont_scan_mail && !qwk_info.abort)
    qwk_gather_email(&qwk_info);

  checka(&abort, &abort);
  if(abort)
    qwk_info.abort=abort;

  CLS();
  if(!qwk_info.abort)
  {
    ansic(9);
    outchr('Ú');   /* "9ÚÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄ¿" */
    repeat_char('Ä', 4);
    outchr('Â');
    repeat_char('Ä', 60);
    outchr('Â');
    repeat_char('Ä', 5);
    outchr('Â');
    repeat_char('Ä', 4);
    outchr('¿');
    nl();
  }

  
  checka(&abort, &abort);
  if(abort)
    qwk_info.abort=abort;

  if(!qwk_info.abort)
  {
    ansic(9);  /* "9³2Sub1 ³2Sub name                                                    9³8Total9³5New 9³" */
    outchr('³');
    ansic(2);
    outstr(get_string(1533));
    ansic(9);
    outchr('³');
    ansic(3);
    outstr(get_string(1534));
    repeat_char(' ', 52);
    ansic(9);
    outchr('³');
    ansic(8);
    outstr(get_string(1535));
    ansic(9);
    outchr('³');
    ansic(5);
    outstr(get_string(1536));
    ansic(9);
    outchr('³');
    nl();
  }


  checka(&abort, &abort);
  if(abort)
    qwk_info.abort=abort;

  if(!qwk_info.abort)
  {
    ansic(9);     /* "9ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄ´" */
    outchr('Ã');
    repeat_char('Ä', 4);
    outchr('Å');
    repeat_char('Ä', 60);
    outchr('Å');
    repeat_char('Ä', 5);
    outchr('Å');
    repeat_char('Ä', 4);
    outchr('´');
    nl();
  }

  msgs_ok=1;

  for (i=0; (usub[i].subnum!=-1) && (i<num_subs) && (!hangup) && !qwk_info.abort && msgs_ok; i++)
  {

    msgs_ok = (max_msgs ? qwk_info.qwk_rec_num <= max_msgs : 1);

    if(qsc_q[usub[i].subnum/32]&(1L<<(usub[i].subnum%32)))
      qwk_gather_sub(i,&qwk_info);
  }

  ansic(9);      /* "9ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÙ" */
  outchr('À');
  repeat_char('Ä', 4);
  outchr('Á');
  repeat_char('Ä', 60);
  outchr('Á');
  repeat_char('Ä', 5);
  outchr('Á');
  repeat_char('Ä', 4);
  outchr('Ù');
  nln(2);

  if(qwk_info.abort)
  {
    ansic(1);
    outstr(get_string(1537));
    if(!yn())
      qwk_info.abort=0;
  }

  qwk_info.file=sh_close(qwk_info.file);
  qwk_info.index=sh_close(qwk_info.index);
  qwk_info.personal=sh_close(qwk_info.personal);
  qwk_info.zero=sh_close(qwk_info.zero);


  if(!qwk_info.abort)
    build_control_dat(&qwk_info);


#ifdef STATS
    ftime(&time2);
    diff1=(long double)time1.time+((long double)time1.millitm/(long double)1000);
    diff2=(long double)time2.time+((long double)time2.millitm/(long double)1000);

    diff3=diff2-diff1;

    sprintf(temp, get_stringx(1,136), diff3);
    sysoplog(temp);
  }
#endif



  if(!qwk_info.abort)
    finish_qwk(&qwk_info);


  // Restore on hangup too, someone might have hungup in the middle of
  // building the list
  if(qwk_info.abort || thisuser.qwk_dontsetnscan || hangup)
    qwk_restore_qscan(save_qsc_p);  // restore nscan pointers if we aborted

  if(qwk_info.abort)
    sysoplog(get_stringx(1,119));

  bbsfree(save_qsc_p);         /* free up memory allocated to save qsc pointers */

  if(thisuser.qwk_delete_mail && !qwk_info.abort)
    qwk_remove_email(); // Delete email

  read_status();

  if(save_conf)
    tmp_disable_conf(0);
  tmp_disable_pause(0);

  // Close down my optimized open files
  qwk_opened_filename[0]=0;
  if(qwk_opened_file>0)
    sh_close(qwk_opened_file);

}




void qwk_gather_sub(int bn, struct qwk_junk *qwk_info)
{
  int i,os,sn;
  char subinfo[201], thissub[81];
  unsigned long qscnptrx,sd;

  float temp_percent;



  sn=usub[bn].subnum;

  if ((hangup) || (sn<0))
    return;


  qscnptrx=qsc_p[sn];
  sd=sub_dates[sn];

  if (qwk_percent || ((!sd) || (sd>qscnptrx)) )
  {
    int ab=0;
    os=cursub;
    cursub=bn;
    i=1;


    // Get total amount of messages in base
    if (!qwk_iscan(cursub))
      return;

    qscnptrx=qsc_p[sn];

    if(!qwk_percent)
    {
      // Find out what message number we are on
      for (i=nummsgs; (i>1) && (get_post(i-1)->qscan>qscnptrx); i--)
        ;
    }
    else  // Get last qwk_percent of messages in sub
    {
      temp_percent=(float)qwk_percent/100.0;
      if(temp_percent>1.0)
        temp_percent=1.0;
      i=nummsgs-(temp_percent * nummsgs);
    }


    strncpy(thissub, subboards[curlsub].name, 65);
    thissub[60]=0;
    sprintf(subinfo, "9³2%-4d9³2%-60s9³ 8%-4d9³5%-4d9³",
                     bn+1, thissub, nummsgs,
                     nummsgs-i+1-(qwk_percent ? 1 : 0));
    pl(subinfo);

    checka(&ab, &ab);
    if(ab)
      qwk_info->abort=1;


    if ((nummsgs>0) && (i<=nummsgs) && !qwk_info->abort)
    {
      if ((get_post(i)->qscan>qsc_p[curlsub]) || qwk_percent)
        qwk_start_read(i, qwk_info);   // read messsage
    }

    read_status();
    qsc_p[curlsub]=status.qscanptr-1;
    
    cursub=os;
  }
  else
  {
    int ab=0;

    os=cursub;
    cursub=bn;
    i=1;

    qwk_iscan(cursub);
    
    strncpy(thissub, subboards[curlsub].name, 65);
    thissub[60]=0;
    sprintf(subinfo, "9³2%-4d9³2%-60s9³ 8%-4d9³5%-4d9³",
                      bn+1, thissub, nummsgs, 0);
    pl(subinfo);

    cursub=os;

    checka(&ab, &ab);

    if(ab)
      qwk_info->abort=1;
  }

  ansic(0);
}


void qwk_start_read(int msgnum, struct qwk_junk *qwk_info)
{
  statusbarrec sb;
  int done, val;
  int amount=1;
  int abort=0;


  irt[0]=0;
  irt_name[0]=0;
  done=0;
  val=0;

  if (curlsub<0)
    return;


  sb.width=10;
  sb.amount_per_square=4;
  sb.square_list[0]='þ';
  sb.square_list[1]='°';
  sb.square_list[2]='±';
  sb.square_list[3]='²';
  sb.empty_space='-';
  sb.side_char1='[';
  sb.side_char2=']';
  sb.surround_color=BLUE;
  sb.box_color=GREEN+(BLUE<<4);
  sb.total_items=nummsgs-msgnum+1;
  sb.current_item=0;

  outchr('\t');
  statusbar(&sb);

  // Used to be inside do loop
  if (xsubs[curlsub].num_nets)
    set_net_num(xsubs[curlsub].nets[0].net_num);
  else
    set_net_num(0);


  do
  {
    ++sb.current_item;
    statusbar(&sb);

    if ((msgnum>0) && (msgnum<=nummsgs))
      make_pre_qwk(msgnum, &val, qwk_info);

    ++msgnum;
    if (msgnum>nummsgs)
      done=1;

    if(thisuser.qwk_max_msgs_per_sub ? amount > thisuser.qwk_max_msgs_per_sub : 0)
      done=1;

    if(max_msgs ? qwk_info->qwk_rec_num > max_msgs : 0)
      done=1;


    ++amount;

    checka(&abort, &abort);
    if(abort)
      qwk_info->abort=abort;

  } while ((!done) && (!hangup) && !qwk_info->abort);

  outchr('\r');
}



void make_pre_qwk(int msgnum, int *val, struct qwk_junk *qwk_info)
{
  postrec p;
  int nn;

  p=*get_post(msgnum);

  if (p.status & (status_unvalidated | status_delete))
  {
    if (!lcs())
      return;
    *val |= 1;
  }

	nn=net_num;

	if (p.status & status_post_new_net)
		set_net_num(p.title[80]);

  put_in_qwk(&p,(subboards[curlsub].filename), msgnum, qwk_info);

  if (nn!=net_num)
    set_net_num(nn);

  ++thisuser.msgread;
  ++msgreadlogon;


  if (p.qscan>qsc_p[curlsub])  // Update qscan pointer right here
    qsc_p[curlsub]=p.qscan;    // And here
  if (p.qscan>=status.qscanptr)
  {
    lock_status();
    if (p.qscan>=status.qscanptr)
      status.qscanptr=p.qscan+1;
    save_status();
  }
}

void put_in_qwk(postrec *m1, char *fn, int msgnum, struct qwk_junk *qwk_info)
{
  struct tm *time_now;
  char n[205],d[81],*ss, temp[101];
  char qwk_address[201];
  int f,cur, p, p1;
  messagerec m;
  char filename[101], date[10];
  float msbin;
  int cur_block=2;

  long len;
  postrec pr;

  int amount_blocks=0;


  pr=*get_post(msgnum);

  if (pr.status & (status_unvalidated | status_delete))
  {
    if (!lcs())
      return;
  }


  memset(&qwk_info->qwk_rec, ' ', sizeof(qwk_info->qwk_rec));


  ss=NULL;
  m=(m1->msg);
  f=-1;
  cur=0;

#ifdef SLOWER_BUT_SAFER
  ss=readfile(&m,fn,&len);
#else
  ss=qwk_readfile(&m,fn,&len);
#endif

  if (ss==NULL)
  {
    outstr(get_string(89));
    nl();
    return;
  }

  p=0;

  // n = name...
  while ((ss[p]!=13) && ((long)p<len) && (p<200) && !hangup)
    n[p]=ss[p++];
  n[p]=0;
  ++p;



  // if next is ascii 10 (linefeed?) go one more...
  p1=0;
  if (ss[p]==10)
    ++p;

  // d = date
  while ((ss[p+p1]!=13) && ((long)p+p1<len) && (p1<60) && !hangup)
    d[p1]=ss[(p1++)+p];
  d[p1]=0;

  // ss+cur now is where the text starts
  cur=p+p1+1;


  sprintf(qwk_address, "%s%s", QWKFrom,  n);  // Copy wwivnet address to qwk_address
  if(!strchr(qwk_address, '@'))
  {
    sprintf(temp, "@%d", m1->ownersys);
    strcat(qwk_address, temp);
  }



  // Took the annonomouse stuff out right here
  if(!qwk_info->in_email)
    strncpy(qwk_info->qwk_rec.to, "ALL", 3);
  else
  {

    strncpy(temp, thisuser.name, 25);
    temp[25]=0;
    strupr(temp);

    strncpy(qwk_info->qwk_rec.to, temp, 25);

  }

  strip_heart_colors(n);
  strncpy(qwk_info->qwk_rec.from, strupr(n), 25);

  time_now=localtime((time_t *)&m1->daten);
  strftime(date, 10, "%m-%d-%y", time_now);
  strncpy(qwk_info->qwk_rec.date, date, 8);



  p=0;
  p1=0;

  len=len-cur;
  make_qwk_ready(ss+cur, &len, qwk_address);



  amount_blocks=((int)len/sizeof(qwk_info->qwk_rec))+2;


  // Save Qwk Record
  sprintf(qwk_info->qwk_rec.amount_blocks, "%d", amount_blocks);
  sprintf(qwk_info->qwk_rec.msgnum, "%d", msgnum);

  strip_heart_colors(pr.title);
  strip_heart_colors(qwk_info->email_title);

  if(!qwk_info->in_email)
    strncpy(qwk_info->qwk_rec.subject, pr.title, 25);
  else
    strncpy(qwk_info->qwk_rec.subject, qwk_info->email_title, 25);

  qwk_remove_null((char *) &qwk_info->qwk_rec, 123);
  qwk_info->qwk_rec.conf_num=usub[cursub].subnum+1;
  qwk_info->qwk_rec.logical_num=qwk_info->qwk_rec_num;


  if(append_block(qwk_info->file, (void *)&qwk_info->qwk_rec, sizeof(qwk_info->qwk_rec)) != sizeof(qwk_info->qwk_rec))
  {
    qwk_info->abort=1;  // Must be out of disk space
    pl(get_string(1539));
    pausescr();
  }


  // Save Qwk NDX
  qwk_info->qwk_ndx.pos=qwk_info->qwk_rec_pos;
  _fieeetomsbin(&qwk_info->qwk_ndx.pos, &msbin);
  qwk_info->qwk_ndx.pos=msbin;

  qwk_info->qwk_ndx.nouse=0;


  if(!qwk_info->in_email)   // Only if currently doing messages...
  {
    // Create new index if it hasnt been already
    if(cursub != qwk_info->cursub || qwk_info->index < 0)
    {
      qwk_info->cursub=cursub;
      sprintf(filename, "%s%03d.NDX", QWK_DIRECTORY, usub[cursub].subnum+1);
      qwk_info->index=sh_close(qwk_info->index);
      qwk_info->index=sh_open1(filename, O_RDWR | O_APPEND | O_BINARY | O_CREAT);
    }

    append_block(qwk_info->index, (void *)&qwk_info->qwk_ndx, sizeof(qwk_info->qwk_ndx));
  }
  else  // Write to email indexes
  {
    append_block(qwk_info->zero, (void *)&qwk_info->qwk_ndx, sizeof(qwk_info->qwk_ndx));
    append_block(qwk_info->personal, (void *)&qwk_info->qwk_ndx, sizeof(qwk_info->qwk_ndx));
  }

  // Setup next NDX position
  qwk_info->qwk_rec_pos+=amount_blocks;

  while(cur_block<=amount_blocks && !hangup)
  {
    int this_pos;
    memset(&qwk_info->qwk_rec, ' ', sizeof(qwk_info->qwk_rec));

    this_pos=((cur_block-2)*sizeof(qwk_info->qwk_rec));

    if(this_pos < len)
    {
      memmove(&qwk_info->qwk_rec, ss+cur+this_pos, this_pos + sizeof(qwk_info->qwk_rec) > (int)len
                                       ? (int)len-this_pos-1 : sizeof(qwk_info->qwk_rec));
    }
    // Save this block
    append_block(qwk_info->file, (void *)&qwk_info->qwk_rec, sizeof(qwk_info->qwk_rec));


    this_pos+=sizeof(qwk_info->qwk_rec);
    ++cur_block;
  }
  // Global variable on total amount of records saved
  ++qwk_info->qwk_rec_num;

  if (f!=-1)
    sh_close(f);

  if(ss!=NULL)
    farfree(ss);

}

// Takes text, deletes all ascii '10' and converts '13' to '227' (ã)
// And does other conversions as specified
void make_qwk_ready(char *text, long *len, char *address)
{
  unsigned pos=0, new_pos=0;
  char *temp;
  int x;
  long new_size=*len+PAD_SPACE+1;

  temp=(char *)malloca(new_size);

  if(!temp)
  {
    sysoplog(get_stringx(1,121));
    return;
  }



  while(pos<*len && new_pos < new_size && !hangup)
  {
    x=(unsigned char)text[pos];

    if(x==0)
      break;

    if(x==13)
    {
      temp[new_pos]=227;
      ++pos;
      ++new_pos;
    }
    // Strip out Newlines, NULLS, 1's and 2's
    else if(x==10 || x < 3)
      ++pos;
    else if(thisuser.qwk_remove_color && x==3)
      pos+=2;
    else if(thisuser.qwk_convert_color && x==3)
    {
      char ansi_string[30];
      int save_curatr=curatr;

      curatr=255;


      // Only convert to ansi if we have memory for it, but still strip heart
      // code even if we don't have the memory.
      if(new_pos+10 < new_size)
      {
        makeansi(text[pos+1], ansi_string, 1);
        temp[new_pos]=0;
        strcat(temp, ansi_string);
        new_pos=strlen(temp);
      }

      pos+=2;

      curatr=save_curatr;
    }
    else if(thisuser.qwk_keep_routing == FALSE && x==4 && text[pos+1]=='0')
    {
      if(text[pos+1]==0)
        ++pos;
      else
      {
        while((unsigned char)text[pos]!=(unsigned char)227 && text[pos]!= '\r' && pos<*len && text[pos] != 0 && !hangup)
          ++pos;
      }
      ++pos;
      if(text[pos]=='\n')
        ++pos;
    }
    else if(x==4 && text[pos+1]!='0')
      pos+=2;
    else
    {
      temp[new_pos]=x;
      ++pos;
      ++new_pos;
    }
  }

  temp[new_pos]=0;
  strcpy(text, temp);
  bbsfree(temp);

  *len=new_pos;

  // Only add address if it does not yet exist
  if(!strstr(text, QWKFrom+2))  // Don't search for dimond or number, just text after that
    insert_after_routing(text, address, len);
}



void qwk_remove_null(char *memory, int size)
{
  int pos=0;

  while(pos<size && !hangup)
  {
    if((char *)memory[pos]==0)
      ((char *)memory)[pos]=' ';

    ++pos;
  }
}




void build_control_dat(struct qwk_junk *qwk_info)
{
  FILE *fp;
  struct qwk_config qwk_cfg;
  int amount=0;
  int cur=0;
  char file[201];
  char system_name[20], temp[20];
  char date_time[51];
  time_t secs_now;
  struct tm *time_now;

  time(&secs_now);
  time_now=localtime(&secs_now);

  // Creates a string like 'mm-dd-yyyy,hh:mm:ss'
  strftime(date_time, 50, "%m-%d-%Y,%H:%M:%S", time_now);

  sprintf(file, "%sCONTROL.DAT", QWK_DIRECTORY);
  fp=fsh_open(file, "wb");

  if (!fp)
    return;

  read_qwk_cfg(&qwk_cfg);

  
  qwk_system_name(system_name);


  fprintf(fp, "%s.QWK\r\n", system_name);
  fprintf(fp, "%s\r\n", "");   // System City and State
  fprintf(fp, "%s\r\n", syscfg.systemphone);
  fprintf(fp, "%s\r\n", syscfg.sysopname);
  fprintf(fp, "%s,%s\r\n", "00000", system_name);
  fprintf(fp, "%s\r\n", date_time);
  fprintf(fp, "%s\r\n", thisuser.name);
  fprintf(fp, "%s\r\n", "");
  fprintf(fp, "%s\r\n", "0");
  fprintf(fp, "%d\r\n", qwk_info->qwk_rec_num);


  for (cur=0; (usub[cur].subnum!=-1) && (cur<num_subs) && (!hangup); cur++)
  {
    if (qsc_q[usub[cur].subnum/32]&(1L<<(usub[cur].subnum%32)))
      ++amount;
  }

  fprintf(fp, "%d\r\n", amount);


  fprintf(fp, "0\r\n");
  fprintf(fp, "E-Mail\r\n");

  for (cur=0; (usub[cur].subnum!=-1) && (cur<num_subs) && (!hangup); cur++)
  {
    if (qsc_q[usub[cur].subnum/32]&(1L<<(usub[cur].subnum%32)))
    {
      strncpy(temp, stripcolors(subboards[usub[cur].subnum].name), 13);
      temp[13]=0;

      fprintf(fp, "%d\r\n", usub[cur].subnum+1);
      fprintf(fp, "%s\r\n", temp);
    }
  }

  fprintf(fp, "%s\r\n", qwk_cfg.hello);
  fprintf(fp, "%s\r\n", qwk_cfg.news);
  fprintf(fp, "%s\r\n", qwk_cfg.bye);

  fsh_close(fp);
  close_qwk_cfg(&qwk_cfg);
}

int _fmsbintoieee(float *src4, float *dest4)
{
   unsigned char *msbin = (unsigned char *)src4;
   unsigned char *ieee = (unsigned char *)dest4;
   unsigned char sign = 0x00;
   unsigned char ieee_exp = 0x00;
   int i;

   /* MS Binary Format                         */
   /* byte order =>    m3 | m2 | m1 | exponent */
   /* m1 is most significant byte => sbbb|bbbb */
   /* m3 is the least significant byte         */
   /*      m = mantissa byte                   */
   /*      s = sign bit                        */
   /*      b = bit                             */

   sign = msbin[2] & 0x80;      /* 1000|0000b  */
                                               
   /* IEEE Single Precision Float Format       */
   /*    m3        m2        m1     exponent   */
   /* mmmm|mmmm mmmm|mmmm emmm|mmmm seee|eeee  */
   /*          s = sign bit                    */
   /*          e = exponent bit                */
   /*          m = mantissa bit                */

   for (i=0; i<4; i++) ieee[i] = 0;
 
   /* any msbin w/ exponent of zero = zero */
   if (msbin[3] == 0) return 0;
   
   ieee[3] |= sign;

   /* MBF is bias 128 and IEEE is bias 127. ALSO, MBF places   */
   /* the decimal point before the assumed bit, while          */
   /* IEEE places the decimal point after the assumed bit.     */

   ieee_exp = msbin[3] - 2;    /* actually, msbin[3]-1-128+127 */

   /* the first 7 bits of the exponent in ieee[3] */
   ieee[3] |= ieee_exp >> 1;   

   /* the one remaining bit in first bin of ieee[2] */
   ieee[2] |= ieee_exp << 7;   

   /* 0111|1111b : mask out the msbin sign bit */
   ieee[2] |= msbin[2] & 0x7f;

   ieee[1] = msbin[1];
   ieee[0] = msbin[0];

   return 0;
}

int _fieeetomsbin(float *src4, float *dest4)
{
   unsigned char *ieee = (unsigned char *)src4;
   unsigned char *msbin = (unsigned char *)dest4;
   unsigned char sign = 0x00;
   unsigned char msbin_exp = 0x00;
   int i;

   /* See _fmsbintoieee() for details of formats   */
   sign = ieee[3] & 0x80;
   msbin_exp |= ieee[3] << 1;
   msbin_exp |= ieee[2] >> 7;

   /* An ieee exponent of 0xfe overflows in MBF    */
   if (msbin_exp == 0xfe)
    return 1;

   msbin_exp += 2;     /* actually, -127 + 128 + 1 */

   for (i=0; i<4; i++) msbin[i] = 0;

   msbin[3] = msbin_exp;

   msbin[2] |= sign;
   msbin[2] |= ieee[2] & 0x7f;
   msbin[1] = ieee[1];
   msbin[0] = ieee[0];

   return 0;
}




char * qwk_system_name(char *qwkname)
{
  int x;
  struct qwk_config qwk_cfg;

  read_qwk_cfg(&qwk_cfg);

  strcpy(qwkname, qwk_cfg.packet_name);
  trimstr1(qwkname);

  close_qwk_cfg(&qwk_cfg);

  if(!qwkname[0])
    strncpy(qwkname, syscfg.systemname, 8);

  qwkname[8]=0;
  x=0;
  while(qwkname[x] && !hangup && x < 9)
  {
    if(qwkname[x]==' ' || qwkname[x]=='.' )
      qwkname[x]='-';
    ++x;
  }
  qwkname[8]=0;
  strupr(qwkname);
  return(qwkname);
}



void qwk_menu(void)
{
  int done=0;
  int key;
  char temp[101], namepath[101];

  qwk_percent=0.0;
  qwk_bi_mode=0;

  while(!done && !hangup)
  {
    if (rip_on()) {
      if (so())
        printmenu(369);
      else
        printmenu(368);
    } else {
      CLS();
      printfile("QWK");
      if(so())
      {
        pl(get_string(1541));
      }
    }

    if(qwk_percent)
    {
      ansic(3);
      nl();
      npr("%s %d%%\r\n", get_string(1636), qwk_percent);
    }
    nl();
    strcpy(temp, "7[3Q1DCUBS%");
    if(so())
      strcat(temp, "1");
    strcat(temp, "7] ");
    outstr(temp);
    mpl(1);

    strcpy(temp, "Q\r?CDUBS%");
    if(so())
      strcat(temp, "1");
    key=onek(temp);
    if (rip_on())
      rip_cls();

    switch(key)
    {
      case '?':
        break;

      case 'Q':
      case '\r':
        done=1;
        break;

      case 'U':
        sysoplog(get_stringx(1,122));
        qwk_bi_mode=0;
        upload_reply_packet();
        break;

      case 'D':
        sysoplog(get_stringx(1,123));
        qwk_system_name(temp);
        strcat(temp, ".REP");
        sprintf(namepath, "%s%s", QWK_DIRECTORY, temp);
        unlink(namepath);

        build_qwk_packet();

        if(exist(namepath))
        {
          sysoplog(get_stringx(1,124));
          qwk_bi_mode=1;
          upload_reply_packet();
        }
        break;

      case 'B':
        sysoplog(get_stringx(1,125));
        qwk_bi_mode=1;

        qwk_system_name(temp);
        strcat(temp, ".REP");
        sprintf(namepath, "%s%s", QWK_DIRECTORY, temp);
        unlink(namepath);

        build_qwk_packet();

        if(exist(namepath))
          upload_reply_packet();
        break;


      case 'S':
        sysoplog(get_stringx(1,126));
        config_qscan();
        break;

      case 'C':
        sysoplog(get_stringx(1,127));
        config_qwk();
        break;

      case '%':
        sysoplog(get_stringx(1,129));
        ansic(2);
        outstr(get_string(1548));
        mpl(3);
        input(temp, 3);
        qwk_percent=atoi(temp);
        if(qwk_percent>100.0)
          qwk_percent=100.0;
        break;

      case '1':
        if(so())
        {
          sysoplog(get_stringx(1,128));
          qwk_sysop();
        }
        break;

    }
  }
}

void qwk_send_file(char *fn, int *sent, int *abort)
{
  int i,i1;
  double percent;

  *sent=0;
  *abort=0;


  if(thisuser.qwk_protocol<=1 || qwk_bi_mode)
  {
    if(qwk_bi_mode)
      i=get_protocol(xf_bi);
    else
      i=get_protocol(xf_down_temp);
  }
  else
    i=thisuser.qwk_protocol;

  percent=0.0;

  switch(i)
  {
    case -1:
      *abort=1;

      break;
    case 0:
      break;

    case 2:
    case 3:
    case 4:
      maybe_internal(fn, sent, &percent, 0, NULL, 1, i);
      break;

    default:
      i1=extern_prot(i-6,fn,1);
      *abort=0;
      if (i1==externs[i-6].ok1)
        *sent=1;
      break;
  }
}

int select_qwk_protocol(struct qwk_junk *qwk_info)
{
  int i;

  i=get_protocol(xf_down_temp);

  switch(i)
  {
    case -1:
      qwk_info->abort=1;
      return i;

    default:
      return i;
  }
}




long * qwk_save_qscan(void)
{
  long * save_qsc_p;
  int i;

  save_qsc_p=(long *)malloca(max_subs*sizeof(long));
  if(!save_qsc_p)
    return save_qsc_p;

	for (i=0; i<max_subs; i++)
		save_qsc_p[i]=qsc_p[i];

	return save_qsc_p;
}

void qwk_restore_qscan(long *save_qsc_p)
{
	int i;

	for (i=0; i<max_subs; i++)
		qsc_p[i]=save_qsc_p[i];
}


void insert_after_routing(char *text, char *text2insert, long *len)
{
	int pos=0;

  strip_heart_colors(text2insert);

  while(pos<*len && text[pos] != 0 && !hangup)
	{
		if(text[pos]==4 && text[pos+1]=='0')
		{
      while(pos<*len && text[pos] != 'ã' && !hangup)
				++pos;

      if(text[pos]== 'ã')
        ++pos;
    }
		else if(pos<*len)
		{
      char *dst, *src;
      long size;
      int addsize;

      strcat(text2insert, "ãã");
      addsize=strlen(text2insert);

      dst=text+pos+addsize;
      src=text+pos;
      size=(*len)-pos+1;


      memmove(dst, src, size);
      strncpy(src, text2insert, addsize);


      *len= (*len) + addsize;

      return;
    }
  }
}


void close_qwk_cfg(struct qwk_config *qwk_cfg)
{
  int x=0;

  while(x<qwk_cfg->amount_blts)
  {
    if(qwk_cfg->blt[x])
      bbsfree(qwk_cfg->blt[x]);
    if(qwk_cfg->bltname[x])
      bbsfree(qwk_cfg->bltname[x]);

    ++x;
  }
}



void read_qwk_cfg(struct qwk_config *qwk_cfg)
{
  int f;
  char s[201];
  int x=0;
  long pos;

  sprintf(s, "%s%s", syscfg.datadir, "QWK.CFG");

  memset(qwk_cfg, 0, sizeof(struct qwk_config));

  f=sh_open1(s, O_BINARY | O_RDONLY);
  if (f<0)
    return;

  sh_read(f,(void *) qwk_cfg, sizeof(struct qwk_config));

  x=0;
  while(x<qwk_cfg->amount_blts)
  {
    pos=sizeof(struct qwk_config)+(x*BULL_SIZE);
    sh_lseek(f, pos, SEEK_SET);
    qwk_cfg->blt[x]=(char *)malloca(BULL_SIZE);
    sh_read(f, (void *)qwk_cfg->blt[x], BULL_SIZE);

    ++x;
  }

  x=0;
  while(x<qwk_cfg->amount_blts)
  {
    pos=sizeof(struct qwk_config)+(qwk_cfg->amount_blts * BULL_SIZE) + (x*BNAME_SIZE);
    sh_lseek(f, pos, SEEK_SET);
    qwk_cfg->bltname[x]=(char *)malloca(BNAME_SIZE * qwk_cfg->amount_blts);
    sh_read(f, (void *)qwk_cfg->bltname[x], BNAME_SIZE);

    ++x;
  }

  sh_close(f);
}



void write_qwk_cfg(struct qwk_config *qwk_cfg)
{
  int f,x, new_amount=0;
  char s[201];
  long pos;

  sprintf(s, "%s%s", syscfg.datadir, "QWK.CFG");

  f=sh_open1(s, O_BINARY | O_RDWR | O_CREAT | O_TRUNC);
  if (f<0)
    return;

  sh_write(f,(void *) qwk_cfg, sizeof(struct qwk_config));

  x=0;
  while(x<qwk_cfg->amount_blts)
  {
    pos=sizeof(struct qwk_config)+(new_amount*BULL_SIZE);
    sh_lseek(f, pos, SEEK_SET);

    if(qwk_cfg->blt[x])
    {
      sh_write(f, (void *)qwk_cfg->blt[x], BULL_SIZE);
      ++new_amount;
    }
    ++x;
  }

  x=0;
  while(x<qwk_cfg->amount_blts)
  {
    pos=sizeof(struct qwk_config)+(qwk_cfg->amount_blts * BULL_SIZE) + (x*BNAME_SIZE);
    sh_lseek(f, pos, SEEK_SET);

    if(qwk_cfg->bltname[x])
      sh_write(f, (void *)qwk_cfg->bltname[x], BNAME_SIZE);

    ++x;
  }

  qwk_cfg->amount_blts=new_amount;
  sh_lseek(f, 0, SEEK_SET);
  sh_write(f,(void *) qwk_cfg, sizeof(struct qwk_config));

  sh_close(f);
}

int get_qwk_max_msgs(unsigned int *max_msgs, unsigned int *max_per_sub)
{
  char temp[6];

  CLS();
  nl();
  ansic(2);
  npr(get_string(1550));
  mpl(5);
  input(temp, 5);

  if(!temp[0])
    return 0;

  *max_msgs=atoi(temp);

  npr(get_string(1551));
  mpl(5);
  input(temp, 5);

  if(!temp[0])
    return 0;

  *max_per_sub=atoi(temp);

  return 1;
}









void qwk_nscan(void)
{
  uploadsrec u;
  int abort=0, od, newfile, i, i1, i5, f, count, color=3;
  char s[201], *ext;

  ansic(3);
  pl(get_string(1552));

  sprintf(s, "%s%s", QWK_DIRECTORY, "NEWFILES.DAT");
  newfile=sh_open1(s, O_BINARY| O_RDWR | O_TRUNC | O_CREAT);
  if(newfile<1)
  {
    pl(get_string(1553));
    return;
  }

  for (i=0; (i<num_dirs) && (!abort) && (udir[i].subnum!=-1); i++)
  {
    checka(&abort, &abort);
    count++;

    npr("%d.",color);
    if (count>=DOTS)
    {
      outstr("\r");
      outstr(get_string(1184));
      color++;
      count=0;
      if (color==4)
        color++;
      if (color==10)
        color=0;
    }

    i1=udir[i].subnum;
    if (qsc_n[i1/32]&(1L<<(i1%32)))
    {
      if ((dir_dates[udir[i].subnum]) && (dir_dates[udir[i].subnum]<nscandate))
        continue;

      od=curdir;
      curdir=i;
      dliscan();
      if (this_date>=nscandate)
      {
        sprintf(s,"\r\n\r\n%s - #%s, %d %s.\r\n\r\n",directories[udir[curdir].subnum].name,
                                udir[curdir].keys,numf, get_string(742));
        sh_write(newfile, (void *) s, strlen(s));

        f=sh_open1(dlfn,O_RDONLY | O_BINARY);
        for (i5=1; (i5<=numf) && (!(abort)) && (!hangup); i5++)
        {
          SETREC(f,i5);
          sh_read(f,(void *)&u,sizeof(uploadsrec));
          if (u.daten>=nscandate)
          {
            sprintf(s, "%s %5ldk  %s\r\n", u.filename, (long) bytes_to_k(u.numbytes), u.description);
            sh_write(newfile, (void *) s, strlen(s));


#ifndef HUGE_TRAN
            if(u.mask & mask_extended)
            {
              int pos=0;
              ext=read_extended_description(u.filename);

              if(ext)
              {
                int spos=21, x;

                strcpy(s, "                     ");
                while(ext[pos])
                {
                  x=ext[pos];

                  if(x != '\r' && x != '\n' && x > 2)
                    s[spos]=x;


                  if(x=='\n' || x==0)
                  {
                    s[spos]=0;
                    sh_write(newfile, (void *)s, strlen(s));
                    sh_write(newfile, (void *)"\r\n", 2);
                    strcpy(s, "                     ");
                    spos=21;
                  }

                  if(x != '\r' && x != '\n' && x > 2)
                    ++spos;

                  ++pos;
                }
                bbsfree(ext);
              }
            }
#endif

          }
          else if(!empty())
            checka(&abort,&abort);

        }
        f=sh_close(f);
      }
      curdir=od;

    }
  }


  newfile=sh_close(newfile);
}


void finish_qwk(struct qwk_junk *qwk_info)
{
  char command[201], parem1[201], parem2[201];
  char qwkname[201];
  int f, sent=0;
  long numbytes;

  struct qwk_config qwk_cfg;
  int x, done=0;
  int archiver;


  if(!thisuser.qwk_dontscanfiles)
    qwk_nscan();

  read_qwk_cfg(&qwk_cfg);
  if(!thisuser.qwk_leave_bulletin)
  {
    pl(get_string(1554));

    if(qwk_cfg.hello[0])
    {
      sprintf(parem1, "%s%s", syscfg.gfilesdir, qwk_cfg.hello);
      sprintf(parem2, "%s%s", QWK_DIRECTORY, qwk_cfg.hello);
      copyfile(parem1, parem2, 1);
    }

    if(qwk_cfg.news[0])
    {
      sprintf(parem1, "%s%s", syscfg.gfilesdir, qwk_cfg.news);
      sprintf(parem2, "%s%s", QWK_DIRECTORY, qwk_cfg.news);
      copyfile(parem1, parem2, 1);
    }

    if(qwk_cfg.bye[0])
    {
      sprintf(parem1, "%s%s", syscfg.gfilesdir, qwk_cfg.bye);
      sprintf(parem2, "%s%s", QWK_DIRECTORY, qwk_cfg.bye);
      copyfile(parem1, parem2, 1);
    }

    x=0;
    while(x<qwk_cfg.amount_blts)
    {
      if(exist(qwk_cfg.blt[x]))
      {
        // Only copy if bulletin is newer than the users laston date
        if(file_daten(qwk_cfg.blt[x]) > date_to_daten(thisuser.laston))
        {
          sprintf(parem2, "%s%s", QWK_DIRECTORY, qwk_cfg.bltname[x]);
          copyfile(qwk_cfg.blt[x], parem2, 1);
        }
        ++x;
      }
    }
  }
  qwk_system_name(qwkname);

  close_qwk_cfg(&qwk_cfg);


  strcat(qwkname, ".QWK");
  
  if(!thisuser.qwk_archive || !syscfg.arcs[thisuser.qwk_archive-1].extension[0])
    archiver=select_qwk_archiver(qwk_info, 0)-1;
  else
    archiver=thisuser.qwk_archive-1;


  if(!qwk_info->abort)
  {
    sprintf(parem1, "%s%s", QWK_DIRECTORY, qwkname);
    sprintf(parem2, "%s*.*", QWK_DIRECTORY);

    stuff_in(command, syscfg.arcs[archiver].arca, parem1, parem2, "", "", "");
    extern_prog(command, EFLAG_NOPAUSE | EFLAG_SHRINK);


    sprintf(command, "%s%s", QWK_DIRECTORY, qwkname);
    make_abs_cmd(command);

    f=sh_open1(command,O_RDONLY | O_BINARY);
    if (f<0)
    {
      pl(get_string(849));
      nl();
      qwk_info->abort=1;
      return;
    }
    numbytes=filelength(f);

    sh_close(f);

    if (numbytes==0L) {
      pl(get_string(850));
      qwk_info->abort=1;
      return;
    }
  }

  if(incom)
  {
    while(!done && !qwk_info->abort && !hangup)
    {
      int abort=0;
      qwk_send_file(command, &sent, &abort);
      if (sent)
        done=1;
      else
      {
        nl();
        ansic(2);
        pl(get_string(1555));
        ansic(1);
        outstr(get_string(1556));

        if(!ny())
        {
          done=1;
          abort=1;
          qwk_info->abort=1;
        }
        else
          abort=0;

      }
      if(abort)
        qwk_info->abort=1;
    }
  }
  else while(!done && !hangup)
  {
    char new_dir[61];
    char nfile[81];

    ansic(2);
    outstr(get_string(1557));
    mpl(60);
    input(new_dir, 60);

    trimstr1(new_dir);

    if(new_dir[0])
    {

      if(new_dir[strlen(new_dir)-1]!='\\')
        strcat(new_dir, "\\");

      sprintf(nfile, "%s%s", new_dir, qwkname);
    }
    else
      nfile[0]=0;

    if(!replacefile(parem1, nfile, 1))
    {
      ansic(7);
      outstr(get_string(1558));
      if(!ny())
      {
        qwk_info->abort=1;
        done=1;
      }
    }
    else
      done=1;
  }
}

#ifndef SLOWER_BUT_SAFER
char *qwk_readfile(messagerec *m1, char *aux, long *l)
{
  int f,csec;
  long l1,l2;
  char *b,s[81],s1[81];
  messagerec m;

  *l=0L;
  m=*m1;
  switch(m.storage_type)
  {
    case 0:
    case 1:
      strcpy(s,syscfg.msgsdir);
      ltoa(m.stored_as,s1,16);
      if (m.storage_type==1) {
        strcat(s,aux);
        strcat(s,"\\");
      }
      strcat(s,s1);
      f=sh_open1(s,O_RDONLY | O_BINARY);
      if (f==-1) {
        *l=0L;
        return(NULL);
      }
      l1=filelength(f);
      if ((b=(char *)malloca(l1+PAD_SPACE))==NULL)
      {
        sh_close(f);
        return(NULL);
      }
      sh_read(f,(void *)b,l1);
      sh_close(f);
      *l=l1;
      b[*l]=0;  // Null terminate out text
      break;
    case 2:

      // You will notice that this case opens, but does not close its file
      // This is becuase of the optimized open file routine, the file gets
      // closed if AUX != OPENED_FILE, and when the build build_qwk_packet
      // function is complete

      if(strcmp(aux, qwk_opened_filename) == 0)
        f=qwk_opened_file;
      else
      {
        // Close currently qwk_opened_file (if any)
        if(qwk_opened_file>0)
          qwk_opened_file=sh_close(qwk_opened_file);

        // Open new file
        f=qwk_open_file(aux);

        if (f<0) {
          *l=0;
          return(NULL);
        }

        // Set it qwk_opened_files variables so we can check and match next time
        qwk_opened_file=f;
        strcpy(qwk_opened_filename, aux);
      }


      if (gat_section!=m.stored_as/2048)
        set_gat_section(f,m.stored_as/2048);

      csec=m.stored_as % 2048;
      l1=0;
      while ((csec>0) && (csec<2048))
      {
        l1+=512L;
        csec=gat[csec];
      }

      if (!l1)
      {
        nl();
        pl(get_string(623));
        nl();
        return(NULL);
      }
      if ((b=(char *)malloca(l1+PAD_SPACE))==NULL)
      {
        return(NULL);
      }
      csec=m.stored_as % 2048;
      l1=0;
      l2=MSG_STARTING;
      while ((csec>0) && (csec<2048)) {
        sh_lseek(f,l2 + 512L*csec,SEEK_SET);
        l1+=(long)sh_read(f,(void *)(&(b[l1])),512);
        csec=gat[csec];
      }
      l2=l1-512;
      while ((l2<l1) && (b[l2]!=26))
        ++l2;
      *l=l2;
      b[*l]=0;  // Null terminate out text
      break;
    case 255:
      f=sh_open1(aux,O_RDONLY | O_BINARY);
      if (f==-1) {
        *l=0L;
        return(NULL);
      }
      l1=filelength(f);
      if ((b=(char *)malloca(l1+PAD_SPACE))==NULL)
      {
        sh_close(f);
        return(NULL);
      }
      sh_read(f,(void *)b,l1);
      sh_close(f);
      *l=l1;
      b[*l]=0;  // Null terminate out text
      break;
    default:
      /* illegal storage type */
      *l=0L;
      b=NULL;
      break;
  }
  return(b);
}

int qwk_open_file(char *fn)
{
  int f,i;
  char s[81];

  sprintf(s,"%s%s.DAT",syscfg.msgsdir,fn);
  f=sh_open1(s,O_RDWR | O_BINARY);

  if (f<0)
  {
    f=sh_open(s,O_RDWR | O_BINARY | O_CREAT, S_IREAD | S_IWRITE);
    for (i=0; i<2048; i++)
      gat[i]=0;

    if (f<0)
      return(-1);
    sh_write(f,(void *)gat,4096);
    strcpy(gatfn,fn);
    chsize(f,4096L + (75L * 1024L));
    gat_section=0;
  }
  if (strcmp(gatfn,fn) || 1)
  {
    sh_lseek(f,0L,SEEK_SET);
    sh_read(f,(void *)gat,4096);
    strcpy(gatfn,fn);
    gat_section=0;
  }
  return(f);
}
#endif
