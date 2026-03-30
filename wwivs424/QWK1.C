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


#define SET_BLOCK(file, pos, size) sh_lseek(file, (long)pos * (long)size, SEEK_SET)


#define qwk_iscan(x)         (iscan1(usub[x].subnum, 1))
#define qwk_iscan_literal(x) (iscan1(x, 1))

#define MAXMAIL 255
#define EMAIL_STORAGE 2

extern char *QWKFrom;
extern int qwk_percent;
extern int qwk_bi_mode;

extern int numlock;


void qwk_remove_email(void)
{
  int i,f,mw,mfl,curmail,done;
	mailrec m;
	tmpmailrec *mloc;

	emchg=0;

	mloc=(tmpmailrec *)malloca(MAXMAIL*sizeof(tmpmailrec));
	if (!mloc)
	{
		pl(get_string(1272));
		return;
	}

  f=open_email(1);

	if (f<0)
	{
		farfree(mloc);
		return;
	}
	mfl=filelength(f)/sizeof(mailrec);
	mw=0;

  for (i=0; (i<mfl) && (mw<MAXMAIL); i++)
	{
		sh_lseek(f,((long) (i)) * (sizeof(mailrec)), SEEK_SET);
		sh_read(f,(void *)(&m),sizeof(mailrec));
		if ((m.tosys==0) && (m.touser==usernum))
		{
			mloc[mw].index=i;
			mloc[mw].fromsys=m.fromsys;
			mloc[mw].fromuser=m.fromuser;
			mloc[mw].daten=m.daten;
			mloc[mw].msg=m.msg;
			mw++;
		}
	}
	thisuser.waiting=mw;

	if (usernum==1)
		fwaiting=mw;


	if (mw==0)
	{
		farfree(mloc);
                f=sh_close(f);
		return;
	}

	if (mw==1)
		curmail=0;

	curmail=0;
	done=0;


        do {
          delmail(f, mloc[curmail].index);

          ++curmail;
          if (curmail>=mw)
            done=1;

        } while ((!hangup) && (!done));

        f=sh_close(f);
}

  

void qwk_gather_email(struct qwk_junk *qwk_info)
{
  int i,f,mw,mfl,curmail,done,tp,nn;
	char filename[201];
	mailrec m;
  postrec junk;
	slrec ss;
	tmpmailrec *mloc;

	emchg=0;

	mloc=(tmpmailrec *)malloca(MAXMAIL*sizeof(tmpmailrec));
	if (!mloc)
	{
		pl(get_string(1272));
		return;
	}

	ss=syscfg.sl[actsl];
	f=open_email(0);
	if (f<0)
	{
                nln(2);
		pl(get_string(702));
		nl();
		farfree(mloc);
		return;
	}
	mfl=filelength(f)/sizeof(mailrec);
	mw=0;
	for (i=0; (i<mfl) && (mw<MAXMAIL); i++)
	{
		sh_lseek(f,((long) (i)) * (sizeof(mailrec)), SEEK_SET);
		sh_read(f,(void *)(&m),sizeof(mailrec));
		if ((m.tosys==0) && (m.touser==usernum))
		{
			mloc[mw].index=i;
			mloc[mw].fromsys=m.fromsys;
			mloc[mw].fromuser=m.fromuser;
			mloc[mw].daten=m.daten;
			mloc[mw].msg=m.msg;
			mw++;
		}
	}
	f=sh_close(f);
	thisuser.waiting=mw;

	if (usernum==1)
		fwaiting=mw;


	if (mw==0)
	{
		nl();
		pl(get_string(27));
		nl();
		farfree(mloc);
		return;
	}
  
  ansic(7);
  pl(get_string(1559));

	if (mw==1)
		curmail=0;

	curmail=0;
	done=0;


	qwk_info->in_email=1;

  sprintf(filename, "%sPERSONAL.NDX", QWK_DIRECTORY);
  qwk_info->personal=sh_open1(filename, O_RDWR | O_APPEND | O_BINARY | O_CREAT);
  sprintf(filename, "%s000.NDX", QWK_DIRECTORY);
	qwk_info->zero=sh_open1(filename, O_RDWR | O_APPEND | O_BINARY | O_CREAT);

	do
	{
    read_same_email(mloc, mw, curmail, &m, 0, 0);

		strupr(m.title);
		strncpy(qwk_info->email_title,m.title, 25);


		i=((ability_read_email_anony & ss.ability)!=0);

		if ((m.fromsys) && (!m.fromuser))
      grab_user_name(&(m.msg),"EMAIL");
		else
			net_email_name[0]=0;
		tp=80;

		if (m.status & status_new_net)
		{
			tp -= 1;
			if (strlen(m.title)<=tp)
				nn=m.title[tp+1];
			else
				nn=0;
		}
		else
			nn=0;


		set_net_num(nn);
//    if (nn==255)
//      setorigin(0,0);
//    else
//      setorigin(m.fromsys, m.fromuser);



    // Hope this isn't killed in the future
    strcpy(junk.title, m.title);
    junk.anony=m.anony;
    junk.status=m.status;
    junk.ownersys=m.fromsys;
    junk.owneruser=m.fromuser;
    junk.daten=m.daten;
    junk.msg=m.msg;


    put_in_qwk(&junk, "EMAIL", curmail, qwk_info);


    ++curmail;
		if (curmail>=mw)
			done=1;

  } while ((!hangup) && (!done));

	qwk_info->in_email=0;
	farfree(mloc);
}



int select_qwk_archiver(struct qwk_junk *qwk_info, int ask)
{
  int x;
  int archiver;
  char temp[101];
  char allowed[20];

  strcpy(allowed, "Q\r");

  nl();
  pl(get_string(1560));
  nl();
  if(ask)
    pl(get_string(1561));
  for(x=0;x<4;++x)
  {
    strcpy(temp,syscfg.arcs[x].extension);
    trimstr1(temp);

    if(temp[0])
    {
      sprintf(temp, "%d", x+1);
      strcat(allowed, temp);
      npr("1%d) 3%s", x+1, syscfg.arcs[x].extension);
      nl();
    }
  }
  nl();
  outstr(get_string(1562));

  if(ask)
    strcat(allowed, "0");

  archiver=onek(allowed);

  if(archiver=='\r')
    archiver='1';

  if(archiver=='Q')
  {
    qwk_info->abort=1;
    return 0;
  }
  archiver=archiver-'0';
  return(archiver);

}

void qwk_which_zip(char *thiszip)
{
  if(thisuser.qwk_archive>4)
    thisuser.qwk_archive=0;

  if(syscfg.arcs[thisuser.qwk_archive-1].extension[0]==0)
    thisuser.qwk_archive=0;

  if(thisuser.qwk_archive==0)
    strcpy(thiszip, "ASK");
  else
    strcpy(thiszip, syscfg.arcs[thisuser.qwk_archive-1].extension);
}

void qwk_which_protocol(char *thisprotocol)
{
  if(thisuser.qwk_protocol==1)
    thisuser.qwk_protocol=0;

  if(thisuser.qwk_protocol==0)
    strcpy(thisprotocol, "ASK");
  else {
    strncpy(thisprotocol, prot_name(thisuser.qwk_protocol), 22);
    thisprotocol[22]=0;
  }
}
void upload_reply_packet(void)
{
  char name[21], namepath[101];
  int rec=1, save_conf=0, save_sub, do_it;
  struct qwk_config qwk_cfg;


  read_qwk_cfg(&qwk_cfg);

  if(!qwk_cfg.fu)
    qwk_cfg.fu=time(NULL);

  ++qwk_cfg.timesu;
  write_qwk_cfg(&qwk_cfg);
  close_qwk_cfg(&qwk_cfg);



  save_sub=cursub;
  if ((uconfsub[1].confnum!=-1) && (okconf(&thisuser)))
  {
    save_conf=1;
    tmp_disable_conf(1);
  }

  qwk_system_name(name);
	strcat(name, ".REP");

  npr("%s%s%s", get_string(1563), name, get_string(1564));

  sprintf(namepath, "%s%s", QWK_DIRECTORY, name);

  if(!qwk_bi_mode)
    do_it=yn();
  else
    do_it=1;

  if(do_it)
  {
    char ch;

    if(!qwk_bi_mode && incom)
    {
      qwk_receive_file(namepath, &rec, &ch, thisuser.qwk_protocol);
      wait_sec_or_hit(1);
    }

    if(rec)
    {
      ready_reply_packet(namepath);

      qwk_system_name(name);
      strcat(name, ".MSG");
      sprintf(namepath, "%s%s", QWK_DIRECTORY, name);
      process_reply_dat(namepath);
    }
    else
    {
      sysoplog(get_stringx(1,119));
      nl();
      npr("%s%s", get_string(1565), name);
      nl();
    }
  }
  if(save_conf)
    tmp_disable_conf(0);

  cursub=save_sub;
}



void ready_reply_packet(char *name)
{
  char command[161];
  int archiver;

  archiver=match_archiver(name);
  stuff_in(command, syscfg.arcs[archiver].arce, name, QWK_DIRECTORY, "", "", "");

  extern_prog(command, EFLAG_NOPAUSE | EFLAG_SHRINK);
}


// Takes reply packet and converts '227' (�) to '13'
void make_text_ready(char *text, long len)
{
  int pos=0;

  while(pos<len && !hangup)
  {
    if((unsigned char)text[pos]==227)
      text[pos]=13;

    ++pos;
  }
}


char * make_text_file(int filenumber, long *size, int curpos, int blocks)
{
  struct qwk_junk *qwk;
  char *temp;


  *size=0;

  // This memory has to be freed later, after text is 'emailed' or 'posted'
	// Enough memory is allocated for all blocks, plus 2k extra for other
  // 'addline' stuff
  qwk=(struct qwk_junk *) malloca((blocks * sizeof(struct qwk_junk))+2048);
  if(!qwk)
    return NULL;


  SET_BLOCK(filenumber, curpos, sizeof(struct qwk_record));

  sh_read(filenumber, (void *)qwk, sizeof(struct qwk_record) * blocks);
  make_text_ready((char *)qwk, sizeof(struct qwk_record)*blocks);

  *size=sizeof(struct qwk_record) * blocks;

  // Remove trailing spaces
  temp=(char *)qwk;
  while(isspace(temp[*size-1]) && *size && !hangup)
    --*size;

  return(temp);
}





void qwk_email_text(char *text, long size, char *title, char *to)
{
  unsigned short sy,un;
  long thetime;
//  char qwk_address[201];
  char *st;

  strupr(to);

  // Remove text name from address, if it doesn't contain " AT " in it
  st=strstr(to, " AT ");
  if(!st)
  {
    st=strchr(to, '#');
    if(st)
      strcpy(to, st+1);
  }
  else  // Also try and strip off name of a gated user
  {
    st=strstr(to, "``");
    if(st)
    {
      st=strstr(st+1, "``");
      if(st)
        strcpy(to, st+2);
    }
  }



  helpl=0;
  irt[0]=0;
  irt_name[0]=0;
  parse_email_info(to,&un,&sy);
  grab_quotes(NULL, NULL);


  if (un || sy)
  {
//    int i;
    messagerec msg;
    char s2[81];
    userrec ur;
    net_system_list_rec *csne;


    if (freek1(syscfg.msgsdir)<10.0) {
      nl();
      pl(get_string(332));
      nl();
      pausescr();
      return;
    }

    if(forwardm(&un,&sy))
    {
      nl();
      pl(get_string(651));
      nl();
      if ((un==0) && (sy==0))
      {
        pl(get_string(652));
        pausescr();
        return;
      }
    }

    if (!un && !sy)
      return;

    if (sy)
      csne=next_system(sy);

    if (sy==0)
    {
      set_net_num(0);
      read_user(un,&ur);
      strcpy(s2,nam(&ur,un));
    }
    else
    {
      if (net_num_max>1)
      {
        if (un==0)
          sprintf(s2,"%s @%u.%s",net_email_name,sy, net_name);
        else
          sprintf(s2,"%u @%u.%s", un, sy, net_name);
      }
      else
      {
        if (un==0)
          sprintf(s2,"%s @%u",net_email_name,sy);
        else
          sprintf(s2,"%u @%u",un,sy);
      }
    }

    if (sy!=0)
    {
//      i=0;
      nl();
      outstr(get_string(654));
      pl(csne -> name);
      outstr(get_string(655));
      pln(csne->numhops);
      nl();
    }

    CLS();
    ansic(2);
    npr("%s 3%s", get_string(1632), s2);
    nl();
    ansic(2);
    npr("%s 3%s", get_string(1633), title);
    nln(2);
    ansic(5);
    outstr(get_string(1566));


    if(!yn())
      return;

    

//    if(i)
//    {
//      ansic(1);
//      outstr(get_string(485));
//      i=yn();
//    }


    msg.storage_type=EMAIL_STORAGE;

    time(&thetime);


    qwk_inmsg(text, size, &msg,"EMAIL", nam1(&thisuser,usernum,net_sysnum), thetime);

    if(msg.stored_as==0xffffffff)
      return;

    ansic(8);
    sendout_email(title, &msg, 0, un, sy, 1, usernum, net_sysnum, 0,net_num);
  }
}

void qwk_inmsg(char *text, long size, messagerec *m1, char *aux, char *name, long thetime)
{
  char s[181];
  int oiia;
  char *t;

	messagerec m;
  long pos;

	oiia=iia;
	setiia(0);

	m=*m1;


  t=(char *)malloca(size+2048);

  pos=0;
  addline(t, name, &pos);

  strcpy(s,ctime(&thetime));
  s[strlen(s)-1]=0;
  addline(t,s,&pos);

  memmove(t+pos, text, size);
  pos+=size;

  if (t[pos-1]!=26)
    t[pos++]=26;
  savefile(t,pos,&m,aux);


  *m1=m;

  charbufferpointer=0;
  charbuffer[0]=0;
  setiia(oiia);
}



void process_reply_dat(char *name)
{
  struct qwk_record qwk;
  char *text;
  long size;
	int repfile;
  int curpos=0;
  int done=0;
  int to_email=0;
  
  repfile=sh_open1(name, O_RDONLY | O_BINARY);

  if(repfile<0)
  {
    nl();
    ansic(3);
    pl(get_string(1567));
    pausescr();
    return;
  }

  SET_BLOCK(repfile, curpos, sizeof(struct qwk_record));
  sh_read(repfile, &qwk, sizeof(struct qwk_record));

  // Should check to makesure first block contains our bbs id


  ++curpos;

  CLS();

  while(!done && !hangup)
  {
    to_email=0;

    SET_BLOCK(repfile, curpos, sizeof(struct qwk_record));
    ++curpos;

    if(sh_read(repfile, (void *)&qwk, sizeof(struct qwk_record))<1)
			done=1;
    else
    {
      char blocks[7];
      char to[201];
      char title[26];
      char tosub[7];



      strncpy(blocks, qwk.amount_blocks, 6);
      blocks[6]=0;

      strncpy(tosub, qwk.msgnum, 7);
      tosub[7]=0;
      
      strncpy(title, qwk.subject, 25);
      title[25]=0;

      strncpy(to, qwk.to, 25);
      to[25]=0;
      strupr(to);
      trimstr1(to);

      
      // If in sub 0 or not public, possibly route into email
      if(atoi(tosub)==0)
        to_email=1;
      else if(qwk.status!=' ' && qwk.status !='-') // if not public
      {
        CLS();
        ansic(1);
        npr("Message '2%s1' is marked 3PRIVATE", title);
        nl();
        ansic(1);
        npr("It is addressed to 2%s", to);
        nln(2);
        ansic(7);
        outstr(get_string(1568));
        if(ny())
          to_email=1;
      }


      text = make_text_file(repfile, &size, curpos, atoi(blocks)-1);
      if(!text)
      {
        curpos+=atoi(blocks)-1;
        continue;
      }


      if(to_email)
      {
        char *temp;

        if((temp=strstr(text, QWKFrom+2)) != NULL)
        {
          char *s;

          temp+=strlen(QWKFrom+2);  // Get past 'QWKFrom:'
          s=strchr(temp, '\r');

          if(s)
          {
            int x;

            s[0]=0;

            trimstr1(temp);
            strupr(temp);

            if(strlen(s) != strlen(temp))
            {
              nl();
              ansic(3);
              npr("1) %s", to);
              nl();
              ansic(3);
              npr("2) %s", temp);
              nln(2);

              outstr(get_string(1569));
              mpl(1);

              x=onek("12");

              if(x=='2')
                strcpy(to, temp);
            }
          }
        }
      }

      
        
         

      if(to_email)
        qwk_email_text(text, size, title, to);
      else if (freek1(syscfg.msgsdir)<10.0)
      {       // Not enough disk space
        nl();
        pl(get_string(332));
        pausescr();
      }
      else
        qwk_post_text(text, size, title, atoi(tosub)-1);

			free(text);

			curpos+=atoi(blocks)-1;
		}
	}

	repfile=sh_close(repfile);
}




void qwk_post_text(char *text, long size, char *title, int sub)
{
  messagerec m;
  postrec p;

  int i,dm,a,f, done=0, pass=0;
  slrec ss;
  long thetime;
  char user_name[101];


  while(!done && !hangup)
  {
    if(pass>0)
    {
      int done5=0;
      char substr[5];

      while(!done5 && !hangup)
      {
        nl();
        outstr(get_string(1570));
        input(substr, 3);

        trimstr1(substr);
        sub=usub[atoi(substr)-1].subnum;

        if(substr[0] == 'Q')
          return;
        else if(substr[0] == '?')
          sublist();
        else
          done5=1;
      }
    }


    if(sub >= num_subs || sub < 0)
    {
      ansic(5);
      pl(get_string(1571));

      ++pass;
      continue;
    }
    cursub=sub;

    // Busy files... allow to retry
    while(!hangup)
    {
      if (!qwk_iscan_literal(cursub))
      {
        nl();
        outstr(get_string(1572));
        if(!ny())
        {
          ++pass;
          continue;
        }
      }
      else
        break;
    }

    if(curlsub < 0)
    {
      ansic(5);
      pl(get_string(1571));

      ++pass;
      continue;
    }


    ss=syscfg.sl[actsl];


    // User is restricked from posting
    if ((restrict_post & thisuser.restrict) || (thisuser.posttoday>=ss.posts))
    {
      nl();
      pl(get_string(669));
      nl();

      ++pass;
      continue;
    }

    // User doesn't have enough sl to post on sub
    if (actsl<subboards[curlsub].postsl)
    {
      nl();
      pl(get_string(670));
      nl();
      ++pass;
      continue;
    }

    m.storage_type=subboards[curlsub].storage_type;

    a=0;

    if(xsubs[curlsub].num_nets)
    {
      a &= (anony_real_name);

      if (thisuser.restrict & restrict_net)
      {
        nl();
        pl(get_string(671));
        nl();
        ++pass;
        continue;
      }
    }

    CLS();
    ansic(2);
    outstr(get_string(1573));
    ansic(3);
    pl(title);

    ansic(2);
    outstr(get_string(1574));
    ansic(3);
    pl(stripcolors(subboards[curlsub].name));

    if(xsubs[curlsub].nets)
    {
      ansic(2);
      outstr(get_string(1575));
      ansic(3);
      pl(net_networks[xsubs[curlsub].nets[xsubs[curlsub].num_nets].net_num].name);
    }

    nl();
    ansic(5);
    outstr(get_string(1566));

    if(ny())
      done=1;
    else
      ++pass;
  }





  if(subboards[curlsub].anony & anony_real_name)
  {
    strcpy(user_name, thisuser.realname);
    properize(user_name);
  }
  else
    strcpy(user_name, nam1(&thisuser,usernum,net_sysnum));


  time(&thetime);
  qwk_inmsg(text, size, &m, subboards[curlsub].filename, user_name, thetime);

  if (m.stored_as!=0xffffffff)
  {
    char s[201];

    while(!hangup)
    {
      f=qwk_iscan_literal(curlsub);

      if (f==-1)
      {
        nl();
        outstr(get_string(1572));
        if(!ny())
          return;
      }
      else
        break;

    }

    // Anonymous
    if(a)
    {
      ansic(1);
      outstr(get_string(485));
      a=yn();
    }
    nl();

    strcpy(p.title, title);
    p.anony=a;
    p.msg=m;
    p.ownersys=0;
    p.owneruser=usernum;
    lock_status();
    p.qscan=status.qscanptr++;
    save_status();
    time((long *)(&p.daten));
    if ((thisuser.restrict & restrict_validate) && (actsl < 20))
      p.status=status_unvalidated;
    else
      p.status=0;

    open_sub(1);

    if ((xsubs[curlsub].num_nets) &&
            (subboards[curlsub].anony & anony_val_net) && (!lcs() || irt[0]))
    {
      p.status |= status_pending_net;
      dm=1;

      for (i=nummsgs; (i>=1) && (i>(nummsgs-28)); i--)
      {
        if (get_post(i)->status & status_pending_net) {
          dm=0;
          break;
        }
      }
      if (dm)
      {
        sprintf(s,get_stringx(1,37),subboards[curlsub].name);
        ssm(1,0,s);
      }
    }

    if (nummsgs>=subboards[curlsub].maxmsgs)
    {
      i=1;
      dm=0;
      while ((dm==0) && (i<=nummsgs) && !hangup)
      {
        if((get_post(i)->status & status_no_delete)==0)
          dm=i;
        ++i;
      }
      if (dm==0)
        dm=1;
      delete(dm);
    }

    add_post(&p);

    ++thisuser.msgpost;
    ++thisuser.posttoday;
    lock_status();
    ++status.msgposttoday;
    ++status.localposts;

    save_status();

    close_sub();

    sprintf(s,get_stringx(1,38),p.title,subboards[curlsub].name);
    sysoplog(s);

    if (xsubs[curlsub].num_nets)
    {
      ++thisuser.postnet;
      if (!(p.status & status_pending_net))
        send_net_post(&p, subboards[curlsub].filename, curlsub);
    }
  }
}



int find_qwk_sub(struct qwk_sub_conf *subs, int amount, int fromsub, char *title)
{
  int x=0;

  if(title==title)
    title=title;

  while(x<amount && !hangup)
  {
    if(subs[x].import_num==fromsub)
      return subs[x].to_num;

    ++x;
  }
  return -1;
}
/* Start DAW */
void qwk_receive_file(char *fn, int *received, char *ft, int i)
{
  if ((i<=1) || (i == 5))
    i=get_protocol(xf_up_temp);

  switch(i) {
    case -1:
    case 0:
    case 1:
    case 5:
      *received=0;
      break;
    case 2:
    case 3:
    case 4:
      maybe_internal(fn, received, NULL, 0, ft, 0, i);
      break;
    default:
      if (incom) {
        extern_prot(i-6,fn,0);
        *received=exist(fn);
      }
      break;
  }
}
/* End DAW */

void qwk_sysop(void)
{
  struct qwk_config qwk_cfg;
  char temp[10];
  char sn[10];
  int done=0;
  int x;

  if(!so())
    return;

  read_qwk_cfg(&qwk_cfg);

  while(!done && !hangup)
  {
    qwk_system_name(sn);
    CLS();
    npr("%s %s", get_string(1576), qwk_cfg.hello);
    nl();
    npr("%s %s", get_string(1577), qwk_cfg.news);
    nl();
    npr("%s %s", get_string(1578), qwk_cfg.bye);
    nl();
    npr("%s %s", get_string(1579), sn);
    nl();
    npr("%s %d", get_string(1580), qwk_cfg.max_msgs);
    nl();
    npr("%s %d", get_string(1581), qwk_cfg.amount_blts);
    nln(2);
    npr(get_string(1582));

    x=onek("Q123456\r\n");
    if(x=='1' || x=='2' || x=='3')
    {
      nl();
      ansic(1);  
      outstr(get_string(1583));
      mpl(12);
    }

    switch(x)
    {
      case '1':
        input(qwk_cfg.hello, 12);
        break;
      case '2':
        input(qwk_cfg.news, 12);
        break;
      case '3':
        input(qwk_cfg.bye, 12);
        break;

      case '4':
        write_qwk_cfg(&qwk_cfg);
        qwk_system_name(sn);
        nl();
        ansic(1);
        npr("Current name : %s", sn);
        nl();
        outstr(get_string(1584));
        input(sn, 8);
        if(sn[0])
          strcpy(qwk_cfg.packet_name, sn);

        write_qwk_cfg(&qwk_cfg);
        break;

      case '5':
        ansic(1);
        outstr(get_string(1585));
        mpl(5);
        input(temp, 5);
        qwk_cfg.max_msgs=atoi(temp);
        break;
      case '6':
        modify_bulletins(&qwk_cfg);
        break;
      default:
        done=1;
    }
  }

  write_qwk_cfg(&qwk_cfg);
  close_qwk_cfg(&qwk_cfg);
}

void modify_bulletins(struct qwk_config *qwk_cfg)
{
  int x, abort=0, key, done=0;
  char s[101], t[101];

  while(!done && !hangup)
  {
    nl();
    outstr(get_string(1586));
    mpl(1);

    key=onek("Q\rAD?");

    switch(key)
    {
      case 'Q':
      case '\r':
        return;

      case 'D':
        nl();
        outstr(get_string(1587));
        mpl(2);

        input(s, 2);
        x=atoi(s);

        if(x<=qwk_cfg->amount_blts)
        {
          strcpy(qwk_cfg->blt[x], qwk_cfg->blt[qwk_cfg->amount_blts-1]);
          strcpy(qwk_cfg->bltname[x], qwk_cfg->bltname[qwk_cfg->amount_blts-1]);

          bbsfree(qwk_cfg->blt[qwk_cfg->amount_blts-1]);
          bbsfree(qwk_cfg->bltname[qwk_cfg->amount_blts-1]);

          --qwk_cfg->amount_blts;
        }
        break;

      case 'A':
        nl();
        pl(get_string(1588));
        input(s, 80);

        if(!exist(s))
        {
          outstr(get_string(1589));
          if(!yn())
            break;
        }

        pl(get_string(1590));
        input(t, BNAME_SIZE);

        if(strncmpi(t, "BLT-", 4) != 0)
        {
          pl(get_string(1591));
          break;
        }

        qwk_cfg->blt[qwk_cfg->amount_blts]=(char *)calloc(BULL_SIZE, sizeof(char));
        qwk_cfg->bltname[qwk_cfg->amount_blts]=(char *)calloc(BNAME_SIZE, sizeof(char));

        strcpy(qwk_cfg->blt[qwk_cfg->amount_blts], s);
        strcpy(qwk_cfg->bltname[qwk_cfg->amount_blts], t);
        ++qwk_cfg->amount_blts;
        break;


      case '?':
        abort=0;
        x=0;
        while(x<qwk_cfg->amount_blts && !abort && !hangup)
        {
          npr("[%d] %s %s", x+1, qwk_cfg->bltname[x], get_string(1592));
          nl();
          repeat_char(' ', 5);
          outstr(qwk_cfg->blt[x]);
          nl();

          checka(&abort, &abort);

          ++x;
        }
        break;
    }
  }
}
void config_qwk_bw(void)
{
  char text[101];
  int done=0, key;


  while(!done && !hangup)
  {
    npr(get_string(1593));
    npr(qwk_current_text(0, text));
    nl();
    npr(get_string(1594));
    npr(qwk_current_text(1, text));
    nl();
    npr(get_string(1595));
    npr(qwk_current_text(2, text));
    nl();
    npr(get_string(1596));
    npr(qwk_current_text(3, text));
    nl();
    npr(get_string(1597));
    npr(qwk_current_text(4, text));
    nl();
    npr(get_string(1598));
    npr(qwk_current_text(5, text));
    nl();
    npr(get_string(1599));
    npr(qwk_current_text(6, text));
    nl();
    npr(get_string(1600));
    npr(qwk_current_text(7, text));
    nl();
    npr(get_string(1601));
    npr(qwk_current_text(8, text));
    nl();
    npr(get_string(1602));
    npr(qwk_current_text(9, text));
    nl();
    npr(get_string(1603));
    npr(qwk_current_text(10, text));
    nl();
    pl(get_string(1604));

    key=onek("QABCDEFGHIJK");

    if(key=='Q')
      done=1;

    key=key-'A';

    switch(key)
    {
      case 0:
        thisuser.qwk_dont_scan_mail=!thisuser.qwk_dont_scan_mail;
        break;
      case 1:
        thisuser.qwk_delete_mail=!thisuser.qwk_delete_mail;
        break;
      case 2:
        thisuser.qwk_dontsetnscan=!thisuser.qwk_dontsetnscan;
        break;
      case 3:
        thisuser.qwk_remove_color=!thisuser.qwk_remove_color;
        break;
      case 4:
        thisuser.qwk_convert_color=!thisuser.qwk_convert_color;
        break;
      case 5:
        thisuser.qwk_leave_bulletin=!thisuser.qwk_leave_bulletin;
        break;
      case 6:
        thisuser.qwk_dontscanfiles=!thisuser.qwk_dontscanfiles;
        break;
      case 7:
        thisuser.qwk_keep_routing=!thisuser.qwk_keep_routing;
        break;

      case 8:
      {
        struct qwk_junk qj;
        int a;

        memset(&qj, 0, sizeof(struct qwk_junk));
        CLS();

        a=select_qwk_archiver(&qj, 1);
        if(!qj.abort)
          thisuser.qwk_archive=a;

        break;
      }

      case 9:
      {
        struct qwk_junk qj;
        int a;

        memset(&qj, 0, sizeof(struct qwk_junk));
        CLS();

        a=select_qwk_protocol(&qj);
        if(!qj.abort)
          thisuser.qwk_protocol=a;

        break;
      }

      case 10:
      {
        unsigned int max_msgs, max_per_sub;

        if(get_qwk_max_msgs(&max_msgs, &max_per_sub))
        {
          thisuser.qwk_max_msgs=max_msgs;
          thisuser.qwk_max_msgs_per_sub=max_per_sub;
        }
        break;
      }

    }
  }
}
void config_qwk_rip(void)
{
  char text[101];
  int done=0, key;


  while(!done && !hangup)
  {
    printmenu(367);

    if (strcmp(qwk_current_text(0, text), "YES") == 0)
      comstr("\r!|@2S34X");
    if (strcmp(qwk_current_text(1, text), "YES") == 0)
      comstr("\r!|@2S3SX");
    if (strcmp(qwk_current_text(2, text), "YES") == 0)
      comstr("\r!|@2S54X");
    if (strcmp(qwk_current_text(3, text), "YES") == 0)
      comstr("\r!|@2S6GX");
    if (strcmp(qwk_current_text(4, text), "YES") == 0)
      comstr("\r!|@2S74X");
    if (strcmp(qwk_current_text(5, text), "YES") == 0)
      comstr("\r!|@9G34X");
    if (strcmp(qwk_current_text(6, text), "YES") == 0)
      comstr("\r!|@9G3SX");
    if (strcmp(qwk_current_text(7, text), "YES") == 0)
      comstr("\r!|@9G4GX");
    comstr("\r!|@BW5Z\\");
    comstr(qwk_current_text(8, text));
    comstr("\r!|@BW6O\\");
    comstr(qwk_current_text(9, text));
    comstr("\r!|Y02000400|@BW7A\\");
    comstr(qwk_current_text(10, text));
    comstr("\r");

    key=onek("QSDMECBFRAPX");

    if(key=='Q')
      done=1;

    switch(toupper(key))
    {
      case 'S':
            thisuser.qwk_dont_scan_mail=!thisuser.qwk_dont_scan_mail;
            break;
      case 'D':
            thisuser.qwk_delete_mail=!thisuser.qwk_delete_mail;
            break;
      case 'M':
            thisuser.qwk_dontsetnscan=!thisuser.qwk_dontsetnscan;
            break;
      case 'E':
            thisuser.qwk_remove_color=!thisuser.qwk_remove_color;
            break;
      case 'C':
            thisuser.qwk_convert_color=!thisuser.qwk_convert_color;
            break;
      case 'B':
            thisuser.qwk_leave_bulletin=!thisuser.qwk_leave_bulletin;
            break;
      case 'F':
            thisuser.qwk_dontscanfiles=!thisuser.qwk_dontscanfiles;
            break;
      case 'R':
            thisuser.qwk_keep_routing=!thisuser.qwk_keep_routing;
            break;

      case 'A':
      {
            struct qwk_junk qj;
            int a;

            memset(&qj, 0, sizeof(struct qwk_junk));
            CLS();

            a=select_qwk_archiver(&qj, 1);
            if(!qj.abort)
              thisuser.qwk_archive=a;

            break;
      }

      case 'P':
      {
            struct qwk_junk qj;
            int a;

            memset(&qj, 0, sizeof(struct qwk_junk));
            CLS();

            a=select_qwk_protocol(&qj);
            if(!qj.abort)
              thisuser.qwk_protocol=a;

            break;
      }

      case 'X':
      {
            unsigned int max_msgs, max_per_sub;

            if(get_qwk_max_msgs(&max_msgs, &max_per_sub))
            {
              thisuser.qwk_max_msgs=max_msgs;
              thisuser.qwk_max_msgs_per_sub=max_per_sub;
            }
            break;
      }

    }
  }
}
void config_qwk(void)
{
  int done=0, save_num=numlock, x;
  char temp[51];
  inputeditrec max_subs, max_packet;
  radiobuttonrec color_convert, archiver;
  checkboxrec options;
  showtextrec protocol, quit_save, abort;
  unsigned int options_bitfield=0, color_bitfield=0;


  varimenurec *menu=NULL, newmenu;

  varimenuinfo info={YELLOW+(BLUE<<4), YELLOW+(BLUE<<4),
                     YELLOW+(BLUE<<4), YELLOW+(BLUE<<4), DARKGRAY+(CYAN<<4),
                     COMMON_FULL, 0, 0, 0, 0, 0};

  /* Start DAW */
  if (!((thisuser.sysstatus & sysstatus_ansi) && (thisuser.sysstatus & sysstatus_color)))
  /* End DAW */
  {
    config_qwk_bw();
    return;
  }

  if (rip_on()) {
    config_qwk_rip();
    return;
  }
  numlock=NUMBERS;


  if(!thisuser.qwk_dont_scan_mail)
    options_bitfield = BitSet(options_bitfield, 0);
  if(thisuser.qwk_delete_mail)
    options_bitfield = BitSet(options_bitfield, 1);
  if(!thisuser.qwk_dontsetnscan)
    options_bitfield = BitSet(options_bitfield, 2);
  if(!thisuser.qwk_leave_bulletin)
    options_bitfield = BitSet(options_bitfield, 3);
  if(!thisuser.qwk_dontscanfiles)
    options_bitfield = BitSet(options_bitfield, 4);
  if(!thisuser.qwk_keep_routing)
    options_bitfield = BitSet(options_bitfield, 5);

  build_checkboxrec(&options, 28, 4, 6, 0, options_bitfield, 'X', ' ');
  fillvarimenurec(&newmenu, &options, CHECK_BOX_TYPE, 0, 0, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);

  if(thisuser.qwk_remove_color)
    color_bitfield=1;
  else if(thisuser.qwk_convert_color)
    color_bitfield=2;
  else
    color_bitfield=0;

  build_radiobuttonrec(&color_convert, 28, 11, 3, color_bitfield, '*', ' ');
  fillvarimenurec(&newmenu, &color_convert, RADIO_BUTTON_TYPE, 0, QWK_SELECT, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);


  build_radiobuttonrec(&archiver, 34, 5, 5, thisuser.qwk_archive, '*', ' ');
  fillvarimenurec(&newmenu, &archiver, RADIO_BUTTON_TYPE, 0, QWK_ARCHIVER, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);

  build_inputrec(&max_packet, 5, 64, 5, 5, 0, IE_UPPER);
  fillvarimenurec(&newmenu, &max_packet, INPUT_EDIT_TYPE, 0, QWK_MAX_PACKET, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);
  sprintf(max_packet.text, "%u", thisuser.qwk_max_msgs);
  max_packet.curlen=strlen(max_packet.text);

  build_inputrec(&max_subs, 5, 64, 6, 5, 0, IE_UPPER);
  fillvarimenurec(&newmenu, &max_subs, INPUT_EDIT_TYPE, 0, QWK_MAX_SUB, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);
  sprintf(max_subs.text, "%u", thisuser.qwk_max_msgs_per_sub);
  max_subs.curlen=strlen(max_subs.text);

  build_showtextrec(&protocol, 39, 13, 0, "", JUSTIFY_CENTER, ' ');
  fillvarimenurec(&newmenu, &protocol, SHOW_TEXT_TYPE, 0, QWK_SELECT_PROTOCOL, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);

  build_showtextrec(&quit_save, 56, 13, 0, "", JUSTIFY_CENTER, ' ');
  fillvarimenurec(&newmenu, &quit_save, SHOW_TEXT_TYPE, EXECUTE, QWK_QUIT_SAVE, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);

  build_showtextrec(&abort, 69, 13, 0, "", JUSTIFY_CENTER, ' ');
  fillvarimenurec(&newmenu, &abort, SHOW_TEXT_TYPE, GET_OUT, QWK_ABORT, COMMON_ACTIVE);
  menu=addvarimenu(menu, &newmenu);


  printfile("QWKCFG");
  GOTO_XY(67,2);
  setc(LIGHTBLUE+(BLACK<<4));
  outstr(numlock==NUMBERS ? get_string(1605) : get_string(1606));

  setc(RED+(LIGHTGRAY<<4));
  for(x=0;x<4;++x)
  {
    GOTO_XY(37, 6+x);
    outstr(syscfg.arcs[x].extension);
  }

  GOTO_XY(52, 11);
  qwk_which_protocol(temp);
  outstr(temp);
  ansic(0);


  done=0;
  while(!done && !hangup)
  {
    varimenu(menu, &info);
    if(do_sysop_command(info.event))
    {
      info.last=0;
      info.redraw=COMMON_FULL;

      CLS();
      printfile("QWKCFG");

      GOTO_XY(67,2);
      setc(LIGHTBLUE+(BLACK<<4));
      outstr(numlock==NUMBERS ? get_string(1605) : get_string(1606));

      setc(RED+(LIGHTGRAY<<4));
      for(x=0;x<4;++x)
      {
        GOTO_XY(37, 6+x);
        outstr(syscfg.arcs[x].extension);
      }

      GOTO_XY(52, 11);
      qwk_which_protocol(temp);
      outstr(temp);

      info.redraw=COMMON_FULL;
      ansic(0);
    }

    switch(info.event)
    {
      case EXECUTE:
        switch(info.returnvalue)
        {
          case QWK_SELECT_PROTOCOL:
            CLS();

            x=get_protocol(xf_down_temp);
            if(x>=0)
              thisuser.qwk_protocol=x;

            CLS();
            printfile("QWKCFG");

            GOTO_XY(67,2);
            setc(LIGHTBLUE+(BLACK<<4));
            outstr(numlock==NUMBERS ? get_string(1605) : get_string(1606));

            setc(RED+(LIGHTGRAY<<4));
            for(x=0;x<4;++x)
            {
              GOTO_XY(37, 6+x);
              outstr(syscfg.arcs[x].extension);
            }

            GOTO_XY(52, 11);
            qwk_which_protocol(temp);
            outstr(temp);

            info.redraw=COMMON_FULL;
            ansic(0);

            break;

          case QWK_ABORT:
            done=1;
            break;


          case QWK_QUIT_SAVE:
          default:
            thisuser.qwk_dont_scan_mail = !options.bi.i.one;
            thisuser.qwk_delete_mail    = options.bi.i.two;
            thisuser.qwk_dontsetnscan   = !options.bi.i.three;
            thisuser.qwk_leave_bulletin = !options.bi.i.four;
            thisuser.qwk_dontscanfiles  = !options.bi.i.five;
            thisuser.qwk_keep_routing   = !options.bi.i.six;

            switch(color_convert.pos)
            {
              case 0:
                thisuser.qwk_remove_color=0;
                thisuser.qwk_convert_color=0;
                break;
              case 1:
                thisuser.qwk_remove_color=1;
                thisuser.qwk_convert_color=0;
                break;
              case 2:
                thisuser.qwk_remove_color=0;
                thisuser.qwk_convert_color=1;
                break;
            }

            thisuser.qwk_archive=archiver.pos;
            thisuser.qwk_max_msgs=atoi(max_packet.text);
            thisuser.qwk_max_msgs_per_sub=atoi(max_subs.text);


            done=1;
            break;

        }
        break;

      case CN:  // Toggle the numlock key, should it act on the numbers or arrows?
        numlock=!numlock;
        if(numlock==NUMBERS)
        {
          int save_c=curatr;
          GOTO_XY(67,2)
          setc(LIGHTBLUE);
          outstr(get_string(1605));
          setc(save_c);
        }
        else
        {
          int save_c=curatr;
          GOTO_XY(67,2);
          setc(LIGHTBLUE);
          outstr(get_string(1606));
          setc(save_c);
        }
        break;



      case GET_OUT:
        done=1;
        break;
    }
  }

  ansic(0);
  killvarimenu(menu);
  numlock=save_num;
}

char *qwk_current_text(int pos, char *text)
{
  char *yesorno[]={"YES", "NO"};

  switch(pos)
  {
    case 0:
      if(thisuser.qwk_dont_scan_mail)
        return(yesorno[1]);
      else
        return(yesorno[0]);
    case 1:
      if(thisuser.qwk_delete_mail)
        return(yesorno[0]);
      else
       return(yesorno[1]);
    case 2:
      if(thisuser.qwk_dontsetnscan)
        return(yesorno[1]);
      else
        return(yesorno[0]);
    case 3:
      if(thisuser.qwk_remove_color)
        return(yesorno[0]);
      else
        return(yesorno[1]);
    case 4:
      if(thisuser.qwk_convert_color)
        return(yesorno[0]);
      else
        return(yesorno[1]);

    case 5:
      if(thisuser.qwk_leave_bulletin)
        return(yesorno[1]);
      else
        return(yesorno[0]);

    case 6:
      if(thisuser.qwk_dontscanfiles)
        return(yesorno[1]);
      else
        return(yesorno[0]);

    case 7:
      if(thisuser.qwk_keep_routing)
        return(yesorno[1]);
      else
        return(yesorno[0]);

    case 8:
      qwk_which_zip(text);
      return(text);
    
    case 9:
      qwk_which_protocol(text);
      return(text);

    case 10:
      if(!thisuser.qwk_max_msgs_per_sub && !thisuser.qwk_max_msgs)
        return(get_string(1607));
      else if(!thisuser.qwk_max_msgs_per_sub)
      {
        sprintf(text, "%s/%u", get_string(1634), thisuser.qwk_max_msgs);
        return(text);
      }
      else if(!thisuser.qwk_max_msgs)
      {
        sprintf(text, "%u/%s", thisuser.qwk_max_msgs_per_sub, get_string(1634));
        return(text);
      }
      else
      {
        sprintf(text, "%u/%u", thisuser.qwk_max_msgs, thisuser.qwk_max_msgs_per_sub);
        return(text);
      }

    case 11:
      strcpy(text, get_string(1635));
      return(text);
  }

  return(NULL);
}

