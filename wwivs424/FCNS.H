#ifndef _FCNS_H_
#define _FCNS_H_

#include "vardec.h"
#include "net.h"
#include "qwk.h"


/* File: bbs.c */

void getcaller(void);
void main(int argc, char *argv[]);


/* File: bbsutl.c */

void far *malloca(unsigned long nbytes);
void stuff_in(char *s, char *s1, char *f1, char *f2, char *f3, char *f4, char *f5);
void copy_line(char *s, char *b, long *ptr, long len);
char *mmkey(int dl);
int inli2(char *s, char *rollover, int maxlen, int crend, int wb);
void inli(char *s, char *rollover, int maxlen, int crend);
int so(void);
int cs(void);
int lcs(void);
void checka(int *abort, int *next);
void pla(char *s, int *abort);
void plal(char *s, int limit, int *abort);
char *ctim(double d);
int sysop2(void);
int checkcomp(char *s);
int check_ansi(void);
int read_in_file(char *fn, messagerec *m, int maxary);
int set_language_1(int n);
int set_language(int n);
void cd_to(char *s);
void get_dir(char *s, int be);


/* File: bbsutl1.c */

int ok_local(void);
int finduser1(unsigned char *sx);
void slname(char *d, char *s);
void islname(char *s);
void catsl(void);
void sl1(int cmd, char *s);
void sysopchar(char *s);
void sysoplog(char *s);
void ssm(unsigned int un, unsigned int sy, char *s);
void write_inst(unsigned short loc, unsigned short subloc, unsigned short flags);
void parse_email_info(char *s, unsigned short *un1, unsigned short *sy1);
int checkpw(void);
void end_bbs(int lev);
int date_changed(void);
void print_local_file(char *ss, char *ss1);
void hang_it_up(void);
int play_sdf(unsigned char *sdfn, int abortable);


/* File: bbsutl2.c */

void CLS(void);
void CURSORUP( int x );
void CURSORDOWN( int x );
void CURSORRIGHT(int x);
void CURSORLEFT(int x);
char *justify_string(char *string, int length, int bg, int type);
unsigned char *trimstr1(unsigned char *s);
void outstr_color(char *s);
void npr_color(char *fmt, ...);
void repeat_char(unsigned char x, int amount);
char *strstr_nocase(char *s1, char *s2);
void statusbar(statusbarrec *sb);
void strip_heart_colors(char *text);
int new_control_break(void);


/* File: com.c */

int check_comport(int pn);
void savel(char *cl, char *atr, char *xl, char *cc);
void restorel(char *cl, char *atr, char *xl, char *cc);
void ptime(void);
void reprint(void);
void print_help(int n);
void setbeep(int i);
void far interrupt async_isr(void);
void outcomch(char ch);
char peek1c(void);
char get1c(void);
int comhit(void);
void dump(void);
void set_baud(unsigned int rate);
void initport(int port_num);
void closeport(void);
void dtr(int i);
void rts(int i);
int cdet(void);
void checkhangup(void);
void addto(char *s, int i);
void makeansi(unsigned char attr, char *s, int forceit);
void setfgc(int i);
void setbgc(int i);
void execute_ansi(void);
void outchr(unsigned char c);
void outstr(unsigned char *s);
void nl(void);
void nln(int n);
void backspace(void);
void setc(unsigned char ch);
void pausescr(void);
void npr(char *fmt, ...);
void pl(char *s);
void pln(int n);
int kbhitb(void);
int empty(void);
void skey1(unsigned char *ch);
unsigned char getchd(void);
unsigned char getchd1(void);
unsigned char inkey(void);
void mpl(int i);
unsigned char upcase(unsigned char ch);
unsigned char locase(unsigned char ch);
unsigned char getkey(void);
void input1(unsigned char *s, int maxlen, int lc, int crend);
void input(unsigned char *s, int len);
void inputl(unsigned char *s, int len);
void inputp(unsigned char *s, int len);
void inputf(unsigned char *s, int len);
int yn(void);
int ny(void);
char ynq(void);
void ansic(int n);
void ansic_x(int n);
char onek(char *s);
void prt(int i, char *s);
void reset_colors(void);
void goxy(int x, int y);
unsigned char *charstr(int len, unsigned char rc);
unsigned char *stripcolors(unsigned char *instr);
void trimstr(unsigned char *s);
char onek1(char *s);
void outstr1(unsigned char *s);
void npr1(char *fmt, ...);


/* File: conio.c */

void my_video_int(void);
void set_global_handle(int i);
void global_char(char ch);
void set_x_only(int tf, char *fn, int ovwr);
void movecsr(int x,int y);
int WhereX(void);
int WhereY(void);
void lf(void);
void cr(void);
void clrscrb(void);
void bs(void);
void out1chx(unsigned char ch);
void out1ch(unsigned char ch);
void outs(unsigned char *s);
void outfast(char *s);
void pr_Wait(int i1);
void set_protect(int l);
void savescreen(screentype *s);
void restorescreen(screentype far *s);
void temp_cmd(char *s);
char scan_to_char(unsigned char ch);
void alt_key(unsigned char ch);
void skey(char ch);
void tleft(int x);
void topscreen(void);
void set_autoval(int n);


/* File: connect1.c */

void zap_call_out_list(void);
void read_call_out_list(void);
void zap_bbs_list(void);
void read_bbs_list(void);
void read_bbs_list_index(void);
int system_index(unsigned int ts);
int valid_system(unsigned int ts);
net_system_list_rec *next_system(unsigned int ts);
void zap_contacts(void);
void read_contacts(void);
void set_net_num(int n);


/* File: extrn.c */



/* File: mmenu.c */

void mainmenu(void);
void dlmainmenu(void);


/* File: modem.c */

void pr1(unsigned char *s);
void get_modem_line(char *s, double d, int allowa);
void do_result(result_info *ri);
void process_full_result(char *s);
int mode_switch(double d, int allowa);
void holdphone(int d);
void imodem(int x);
void answer_phone(void);


/* File: msgbase.c */

void describe_area_code(int areacode, char *description);
void setorigin(int sysnum, int usernum);
int okfsed(void);
void remove_link(messagerec *m1, char *aux);
int open_file(char *fn);
void set_gat_section(int f, int section);
void save_gat(int f);
void savefile(char *b, long l1, messagerec *m1, char *aux);
char *readfile(messagerec *m1, char *aux, long *l);
void change_storage(messagerec *oldm, char *olda, messagerec *newm, char *newa);
void load_workspace(char *fnx, int no_edit);
void osan(char *s, int *abort, int *next);
void addline(char *b, char *s, long *ll);
void stuff(char *s, char *old, char *new);
void inmsg(messagerec *m1, char *title, int *anony, int needtitle, char *aux, int fsed, char *dest, int flags);
int forwardm(unsigned short *u, unsigned short *s);
int open_email(int wrt);
void sendout_email(char *title, messagerec *msg, int anony, unsigned un, unsigned sy, int an, unsigned uf, unsigned sf, int fwd, int fnn);
int ok_to_mail(unsigned un, unsigned sy, int forceit);
void email(unsigned short un, unsigned short sy, int forceit, int anony);
void imail(unsigned short u, unsigned short s);
void plan(char *s, int *abort, int *next);
void read_message1(messagerec *m1, char an, int readit, int *next, char *fn);
int maybeprint(char *fn, int force);
int printfile(char *fn);
int existprint(unsigned char *fn);
void read_message(int n, int *next, int *val);
void lineadd(messagerec *m1, char *sx, char *aux);


/* File: msgbase1.c */

void send_net_post(postrec *p, char *extra, int subnum);
void post(void);
void grab_user_name(messagerec *m, char *fn);
void scan(int msgnum, int optype, int *nextsub);
void qscan(int bn, int *ns);
void nscan(int ss);
void scan2(void);
void printmenu(int i);
void delmail(int f, int loc);
void remove_post(void);
int external_edit(char *fn1, char *direc, int ednum, int numlines, char *dest, char *title, int flags);
void grab_quotes(messagerec *m, char *aux);


/* File: share.c */

int sh_open(char *path, int file_access, unsigned mode);
int sh_open1(char *path, int access);
FILE *fsh_open(char *path, char *mode);
int sh_close(int f);
void fsh_close(FILE *f);
void share_installed(void);
long sh_lseek(int handle, long offset, int fromwhere);
int sh_read(int handle, void *buf, unsigned len);
int sh_write(int handle, void *buf, unsigned len);
size_t fsh_read(void *ptr, size_t size, size_t n, FILE *stream);
size_t fsh_write(void *ptr, size_t size, size_t n, FILE *stream);


/* File: strings.c */

int set_strings_fn(int filen, char *dir, char *fn, int allowwrite);
void put_string(int filen, int n, char *s);
int cachestat(void);
char *get_stringx(int filen, int n);
char *get_string(int n);
int num_strings(int filen);
char *getrandomstring(int filen);
void close_strfiles(void);


/* File: subacc.c */

void close_sub(void);
int open_sub(int wr);
int iscan1(int si, int quick);
int iscan(int b);
postrec *get_post(unsigned short mn);
void delete(int mn);
void write_post(int mn, postrec *pp);
void add_post(postrec *pp);
void resynch(int subnum, int *msgnum, postrec *pp);


/* File: utility.c */

void reset_act_sl(void);
void show_files(char *fn, char *dir);
void remove_from_temp(char *fn, char *dir, int po);
void check_event(void);
void run_event(void);
double freek(int dr);
unsigned char years_old(unsigned char m, unsigned char d, unsigned char y);
void itimer(void);
double timer(void);
long timer1(void);
int sysop1(void);
int okansi(void);
void tmp_disable_conf(int disable);
void tmp_disable_pause(int disable);
int okconf(userrec *u);
void frequent_init(void);
void fix_user_rec(userrec *u);
int number_userrecs(void);
void read_user(unsigned int un, userrec *u);
void write_user(unsigned int un, userrec *u);
int open_qscn(void);
void close_qscn(void);
void read_qscn(unsigned int un, unsigned long *qscn, int stayopen);
void write_qscn(unsigned int un, unsigned long *qscn, int stayopen);
double ratio(void);
double post_ratio(void);
unsigned char *nam(userrec *u1, unsigned int un);
unsigned char *nam1(userrec *u1, unsigned int un, unsigned int sy);
double nsl(void);
char *date(void);
char *times(void);
unsigned int finduser(unsigned char *s);
int access_conf(userrec *u, int sl, confrec *c);
int access_sub(userrec *u, int sl, subboardrec *s);
int access_dir(userrec *u, int sl, directoryrec *d);
void addusub(usersubrec *ss1, int ns, int sub, char key);
int setconf(unsigned int conftype, int which, int oldsub);
void setuconf(int conftype, int num, int oldsub);
void changedsl(void);
void isr(int un, unsigned char *name);
void dsr(unsigned char *name);
void wait1(long l);
void Wait(double d);
double freek1(char *s);
int exist(char *s);
void add_ass(int i, char *ss);
int find_interrupt(void);
void send_net(net_header_rec *nh, unsigned int *list, char *text, char *byname);
unsigned char dow(void);
void detect_multitask(void);
int get_dos_version(void);
int get_dv_version(void);
int get_win_version(void);
void begin_crit(void);
void end_crit(void);
void giveup_timeslice(void);
void dv_pause(void);
void win_pause(void);
void get_status(int mode, int lock);
void lock_status(void);
void read_status(void);
void save_status(void);
int inst_msg_waiting(void);
void setiia(int poll_ticks);
long huge_xfer(int fd, void huge *buf, unsigned sz, unsigned nel, int wr);
char *stripfn(char *fn);
void stripfn1(char *fn);


/* File: xfer.c */

void zap_ed_info(void);
void get_ed_info(void);
unsigned long bytes_to_k(unsigned long b);
int check_batch_queue(char *fn);
int check_ul_event(int dn, uploadsrec *u);
void finddevs(char (*devs)[9], int *count);
void find_devices(void);
int okfn(char *s);
void print_devices(void);
char *make_abs_cmd(char *out);
void get_arc_cmd(char *out, char *arcfn, int cmd, char *ofn);
int list_arc_out(char *fn, char *dir);
int ratio_ok(void);
int dcs(void);
void dliscan1(int dn);
void dliscan_hash(int dn);
void dliscan(void);
void add_extended_description(char *fn, char *desc);
void delete_extended_description(char *fn);
char *read_extended_description(char *fn);
void print_extended(char *fn, int *abort, unsigned char numlist, int indent);
void align(char *s);
int compare(char *s1, char *s2);
void printinfo(uploadsrec *u, int *abort);
void printtitle(int *abort);
void file_mask(char *s);
void listfiles(void);
void nscandir(int d, int *abort);
void nscanall(void);
void searchall(void);
int recno(char *s);
int nrecno(char *s,int i1);
int printfileinfo(uploadsrec *u, int dn);
long date_to_daten(char *datet);
void remlist(char *fn);


/* File: ripspeed.c */

char rd_on(void);
int menu_on(void);
int rip_on(void);
void rd_print(char *s);
void rd_str(char *s);
void rip_saveall(void);
void rip_restoreall(void);
void rip_pcb(void);
void rip_con(void);
void rd_coff(void);
void rd_con(void);
void rip_coff(void);
void rip_smf(void);
void rip_rmf(void);
void rip_rtwpcb(void);
void rip_show(char *s);
void getkeymouse(void);
void rip_cls(void);
void out_cport(char c);
void remstr(char *s);
void comstr(char *s);
void comr(char *s);
void comnl(char *s);
char delaykey(int tm);


/* File: batch.c */

void listbatch(void);
void delbatch(int i);
void downloaded(char *fn, long cps);
void didnt_upload(int ind);
int try_to_ul(char *fn);
void uploaded(char *fn, long cps);
void ymbatchdl(int had);
void handle_dszline(char *l);
double ratio1(long a);
void make_ul_batch_list(char *listfn);
void make_dl_batch_list(char *listfn);
void run_cmd(char *cmdln, char *downlist, char *uplist, char *dl, int had);
void process_dszlog(void);
void dszbatchdl(int had, char *cmdln, char *desc);
void dszbatchul(int had, char *cmdln, char *desc);
void bibatch(int had, char *cmdln, char *desc);
void batchdl(int mode);
void bihangup(int up);


/* File: bbsovl1.c */

void chatsound(int sf, int ef, int uf, int dly1, int dly2, int rp);
/* void reqchat(void); -- removed in OrbitBBS */
void yourinfo(void);
void upload_post(void);
void copy_file(char *input, char *output);
void rsm(int un, userrec *u, int mode);
void show_chains(int *mapp, int *map);
void run_chain(int cn);
void do_chains(void);
void compress_file(char *fn, char *dir);
void extract_mod(char *b, long len);
void extract_out(char *b, long len, char *title);
void send_email(void);
void edit_confs(void);
void feedback(int nuf);
void sublist(void);
void dirlist(void);
void text_edit(void);


/* File: bbsovl2.c */

void makewindow(int x, int y, int xlen, int ylen);
void editline(unsigned char *s, int len, int status, int *returncode, unsigned char *ss);
void val_cur_user(void);
void read_char_xy(int x, int y, int *c, int *a);
void set_attr_xy(int x, int y, int a);
void select_chat_name(unsigned char *s);
void two_way_chat(char *s, char *rollover, int maxlen, int crend, unsigned char *sysopname);
void chat1(char *chatline, int two_way);


/* File: bbsovl3.c */

unsigned char pd_getkey(void);
unsigned get_kb_event(void);
char onek_ncr(char *s);
int do_sysop_command(unsigned command);
void colorize_text(char *buffer);
void sprintf_color(char *buffer, char *fmt, ...);
int replacefile(char *src, char *dst, int stats);
int copyfile(char *input, char *output, int stats);
int movefile(char *src, char *dst, int stats);
int check_arc(char *filename);
int match_archiver(char *filename);
long file_daten(char *filename);
void wait_sec_or_hit(double seconds);
varimenurec * addvarimenu(varimenurec *current, varimenurec *newitem);
void varimenuforward(varimenurec *menu, varimenuinfo *info);
void varimenubackward(varimenurec *menu, varimenuinfo *info);
void varimenuleft(varimenurec *menu, varimenuinfo *info);
void varimenuup(varimenurec *menu, varimenuinfo *info);
void varimenuright(varimenurec *menu, varimenuinfo *info);
void varimenudown(varimenurec *menu, varimenuinfo *info);
void varimenu(varimenurec *menu, varimenuinfo *info);
varimenurec * getvarimenurec(varimenurec *menu, int which);
void redrawvarimenu(int redraw, varimenurec *menu, varimenuinfo *info);
void killvarimenu(varimenurec *menu);
void kill_inputrec(inputeditrec *input);
void kill_showtextrec(showtextrec *text);
void kill_radiobutton(radiobuttonrec *radio);
void kill_checkbox(checkboxrec *check);
int varimenu_findhotkey(varimenurec *menu, varimenuinfo *info);
void fillvarimenurec(varimenurec *menu, void *rec, int type, unsigned hotkey, int returnvalue, int active);
void build_inputrec(inputeditrec *input, int maxlen, int xpos, int ypos, int width, int insert, int char_case);
void build_radiobuttonrec(radiobuttonrec *radio, int xpos, int ypos, int amount, int pos, int rc, int fc);
void build_checkboxrec(checkboxrec *check, int xpos, int ypos, int amount, int pos, unsigned bi, int rc, int fc);
void radio_button(radiobuttonrec *radio, varimenuinfo *info);
void check_box(checkboxrec *check, varimenuinfo *info);
void input_edit(inputeditrec *input, varimenuinfo *info);
int ok_char_inputedit(inputeditrec *input, varimenuinfo *info);
void add_char_inputedit(inputeditrec *input, varimenuinfo *info);
void redrawinputedit(int type, inputeditrec *input, varimenuinfo *info, int mode);
void delete_inputedit(inputeditrec *input, varimenuinfo *info);
void redrawradiobutton(int redraw, radiobuttonrec *radio, varimenuinfo *info, int mode);
void redrawcheckbox(int redraw, checkboxrec *check, varimenuinfo *info, int mode);
void redrawshowtext(int redraw, showtextrec *text, varimenuinfo *info, int mode);
void showtext(showtextrec *text, varimenuinfo *info);
void build_showtextrec(showtextrec *text, int xpos, int ypos, int width, char *t, int justify, int bg);
void build_inputpicrec(inputpicrec *input, int xpos, int ypos, int width, int insert, char *picture);
void kill_inputpic(inputpicrec *input);
void input_picture(inputpicrec *input, varimenuinfo *info);
int ok_char_inputpic(inputpicrec *input, varimenuinfo *info);
char ** read_picture(char *picture);
void redrawinputpic(int type, inputpicrec *input, varimenuinfo *info, int mode);


/* File: chnedit.c */

void chaindata(int n, char *s);
void showchains(void);
void modify_chain(int n);
void insert_chain(int n);
void delete_chain(int n);
void chainedit(void);


/* File: conf.c */

int get_conf_info(unsigned int conftype, unsigned int *num, confrec **cpp, char *fn, unsigned int *num_s, userconfrec **uc);
void jump_conf(unsigned int conftype);
void update_conf(unsigned int conftype, SUBCONF_TYPE *sub1, SUBCONF_TYPE *sub2, unsigned int action);
unsigned int str_to_arword(unsigned char *arstr);
unsigned char *word_to_arstr(unsigned int ar);
unsigned char first_available_designator(unsigned int conftype);
int in_conference(unsigned int subnum, confrec *c);
void save_confs(unsigned int conftype, int whichnum, confrec *c);
void showsubconfs(unsigned int conftype, confrec *c);
SUBCONF_TYPE *str_to_numrange(unsigned char *s, int *numinlist);
void addsubconf(unsigned int conftype, confrec *c, SUBCONF_TYPE *which);
void delsubconf(unsigned int conftype, confrec *c, SUBCONF_TYPE *which);
int modify_conf(unsigned int conftype, unsigned int n);
void insert_conf(unsigned int conftype, unsigned int n);
void delete_conf(unsigned int conftype, unsigned int n);
void conf_edit(unsigned int conftype);
void list_confs(unsigned int conftype, int ssc);
int select_conf(unsigned char *s, unsigned int conftype, int listconfs);
int create_conf_file(unsigned int conftype);
confrec *read_conferences(unsigned char *cf, unsigned int *nc, int max);
void read_in_conferences(int conftype);
void read_all_conferences(void);
unsigned int get_num_conferences(unsigned char *cf);
unsigned int wordcount(unsigned char *instr, unsigned char *delimstr);
unsigned char *extractword(unsigned ww,unsigned char *instr,unsigned char *delimstr);
void sort_conf_str(unsigned char *s);


/* File: defaults.c */

void select_editor(void);
void print_cur_stat(void);
char *cn(char c);
char *describe(char col);
void color_list(void);
void change_colors(void);
void l_config_qscan(void);
void config_qscan(void);
void list_macro(unsigned char *s);
void make_macros(void);
void input_pw1(void);
void modify_mailbox(void);
void optional_lines(void);
void enter_regnum(void);
void defaults(void);


/* File: diredit.c */

void dirdata(int n, char *s);
void showdirs(void);
void modify_dir(int n);
void swap_dirs(int dir1, int dir2);
void insert_dir(int n);
void delete_dir(int n);
void dlboardedit(void);


/* File: extrn1.c */

int extern_prog(char *cmdline, unsigned short flags);
void create_filename(int which, char *name);
char *create_chain_file(void);
void save_state(int state,int ctc, char *restorefn, char *statfn);


/* File: gfiles.c */

char *get_file(char *fn, long *len);
gfilerec *read_sec(int sn, int *nf);
void list_sec(int *map, int nmap);
void list_gfiles(gfilerec *g, int nf);
void gfile_sec(int sn);
void gfiles(void);
void hop_sub(void);
void hop_dir(void);
void valscan(void);


/* File: gfledit.c */

void gfiledata(int n, char *s);
void showsec(void);
int exist_dir(char *s);
void modify_sec(int n);
void insert_sec(int n);
void delete_sec(int n);
void gfileedit(void);
int fill_sec(int sn);
void pack_all_subs(void);
void pack_sub(int si);


/* File: ini.c */

void ini_done(void);
int ini_init(unsigned char *fn, unsigned char *prim, unsigned char *sec);
unsigned char *ini_get(unsigned char *key, int index, unsigned char *index1);


/* File: instmsg.c */

void send_inst_msg(inst_msg_header *ih, unsigned char *msg);
void send_inst_str1(unsigned short m, int whichinst, unsigned char *sendstr);
void send_inst_str(int whichinst, unsigned char *sendstr);
void send_inst_sysstr(int whichinst, unsigned char *sendstr);
void send_inst_shutdown(int whichinst);
void broadcast(unsigned char *sendstr);
int handle_inst_msg(inst_msg_header *ih, unsigned char *msg);
void process_inst_msgs(void);
/* void chat_room(void); -- removed in OrbitBBS */
int get_inst_info(int wi, instancerec *ir);
int inst_available(instancerec *ir);
int inst_available_chat(instancerec *ir);
int num_instances(void);
int user_online(int un, int *wi);
void instance_edit(void);


/* File: lilo.c */

void random_screen(char *s1);
int usa_phone_convention(userrec *u);
void getuser(void);
void logon(void);
void logoff(void);


/* File: misccmd.c */

/* AutoMessage removed in OrbitBBS */
void bbslist(void);
void kill_old_email(void);
void list_users(int mode);
void print_quest(int mapp, int map[21]);
int print_question(int i, int ii);
void vote_question(int i, int ii);
void vote(void);
void time_bank(void);
void remotenotify(char *name, char *desc);
int remoteupload(char *message);
int getnetnum(char *netnam);


/* File: multinst.c */

void make_inst_str(short wi, unsigned char *ss);
void multi_instance(void);
int inst_ok(unsigned short loc, unsigned short subloc);


/* File: multmail.c */

void multimail(int *un, int numu);
int oneuser(void);
void add_list(int *un, int *numu, int maxu, int allowdup);
void slash_e(void);


/* File: netsup.c */

void rename_pend(char *dir, char *file);
void checkup(struct ftime *f1, struct ftime *f2, int *tf);
int checkup2(struct ftime *f, char *x);
int check_bbsdata(void);
void cleanup_net(void);
void do_callout(int sn);
int ok_to_call(int i);
void fixup_long(long *f, long l);
void free_vars(float **weight, int **try);
void attempt_callout(void);
void force_callout(int dw);
void print_pending_list(void);
void gate_msg(net_header_rec *nh, char *text, unsigned short nn, char *byname, unsigned int *list, unsigned short fnn);


/* File: newuser.c */

void input_phone(void);
void input_dataphone(void);
void input_language(void);
int check_name(unsigned char *nn);
void input_name(void);
void input_realname(void);
void input_callsign(void);
int valid_phone(char *phone);
void input_street(void);
void input_city(void);
void input_state(void);
void input_country(void);
void input_zipcode(void);
void input_sex(void);
void input_age(userrec *u);
void input_comptype(void);
void input_screensize(void);
void input_pw(void);
void input_ansistat(void);
int find_new_usernum(userrec *u, unsigned long *qsc);
void newuser(void);
int check_zip(char *zip, int mode);
void properize(char *s);
int check_dupes(char *phone);


/* File: readmail.c */

int same_email(tmpmailrec *tm, mailrec *m);
void purgemail(tmpmailrec *mloc,int mw,int *curmail, mailrec *m1, slrec *ss);
void resynch_email(tmpmailrec *mloc, int mw, int rec, mailrec *m, int del, unsigned short stat);
int read_same_email(tmpmailrec *mloc, int mw, int rec, mailrec *m, int del, unsigned short stat);
void add_netsubscriber(unsigned short sn);
void readmail(void);


/* File: sr.c */

void calc_CRC(unsigned char b);
char gettimeout(double d, int *abort);
int extern_prot(int pn, char *fn1, int sending);
int ok_prot(int pn, xfertype xt);
char *prot_name(int pn);
int get_protocol(xfertype xt);
void ascii_send(char *fn, int *sent, double *percent);
void maybe_internal(char *fn, int *xferred, double *percent, char ft, char *ftp, int send, int prot);
void send_file(char *fn, int *sent, int *abort, char ft, char *sfn, int dn, long fs);
void receive_file(char *fn, int *received, char *ft, char *sfn, int dn);
char end_batch1(void);
void endbatch(void);


/* File: srrcv.c */

char modemkey(int *tout);
int receive_block(char *b, unsigned char *bln, int ucrc);
void xymodem_receive(char *fn, char *ft, int *received, int ucrc);


/* File: srsend.c */

void send_block(char *b, int type, int ucrc, char bn);
char send_b(int f, long pos, int type, char bn, int *ucrc, char *fn, int *terr, int *abort);
int okstart(int *ucrc, int *abort);
void xymodem_send(char *fn, int *sent, double *percent, char ft, int ucrc, int ym, int ymb);


/* File: subedit.c */

void save_subs(void);
void boarddata(int n, char *s);
void showsubs(void);
void modify_sub(int n);
void swap_subs(int sub1, int sub2);
void insert_sub(int n);
void delete_sub(int n);
void boardedit(void);


/* File: subreq.c */

void sub_req(int main_type, int minor_type, int tosys, char *extra);
int find_hostfor(char *type, unsigned short *ui, char *desc, unsigned short *opt);
void sub_xtr_del(int n, int nn, int f);
void sub_xtr_add(int n, int nn);
int display_sub_categories(void);


/* File: subxtr.c */

int read_subs_xtr(int max_subs, int num_subs, subboardrec *subboards);


/* File: sysopf.c */

void isr1(int un, unsigned char *name);
void reset_files(void);
void outstr_x1(char *s);
void prstatus(void);
void valuser(int un);
void print_net_listing(unsigned int tp);
void read_new_stuff(void);
void mailr(void);
void chuser(void);
void zlog(void);
void beginday(void);


/* File: tedit.c */

struct line *read_file(char *fn, int *numlines);
void kill_file(struct line *topline);
void save_file(char *fn, struct line *topline);
int printl(int n, struct line *l);
void tedit(char *fn);


/* File: uedit.c */

void deluser(int un);
void outstr_x2(char *s);
void print_data(int un, userrec *u, int lng, int cls);
int matchuser(int un);
int match_user(userrec *u);
void changeopt(void);
void auto_val(int n, userrec *u);
void uedit(int usern, int other);
unsigned char *daten_to_date(long dt);
void print_affil(userrec *u);


/* File: voteedit.c */

void print_quests(void);
void set_question(int ii);
void ivotes(void);
void voteprint(void);


/* File: xferovl.c */

void move_file(void);
int comparedl(uploadsrec *x, uploadsrec *y, int type);
void quicksort(int l,int r,int type);
void sortdir(int dn, int type);
void sort_all(int type);
void rename_file(void);
int upload_file(char *fn, int dn, char *desc);
int maybe_upload(char *fn, int dn, char *desc);
void upload_files(char *fn, int dn, int type);
int uploadall(int dn);
void relist(void);
void edit_database(void);
long db_index(int af, char *fn);
void modify_database(char *fn, int add);
int is_uploadable(char *fn);
void l_config_nscan(void);
void config_nscan(void);
void xfer_defaults(void);
void finddescription(void);
void arc_l(void);


/* File: xferovl2.c */

void modify_extended_description(char **sss, char *dest, char *title);
void upload(int dn);
void get_file_idz(uploadsrec *u, int dn);
void read_idz(void);
void tag_it(void);
void tag_files(void);
int try_to_download(char *s, int dn,int title);
void download(void);
char fancy_prompt(char *prompt,char *accept);
void endlist(int mode);
void yourinfodl(void);
void setldate(void);


/* File: xfertmp.c */

long bad_filename(char *fn);
int check_for_files_arc(char *fn);
int check_for_files_zip(char *fn);
int check_for_files_lzh(char *fn);
int check_for_files_arj(char *fn);
int check_for_files(char *fn);
void download_temp_arc(char *fn, int xfer);
void add_arc(char *arc, char *fn, int dos);
void add_temp_arc(void);
void del_temp(void);
void list_temp_dir(void);
void temp_extract(void);
void list_temp_text(void);
void list_temp_arc(void);
void temporary_stuff(void);
void move_file_t(void);
void removefile(void);


/* File: xinit.c */

void far *mallocx(unsigned long l, char *where);
unsigned short str2spawnopt(unsigned char *s);
unsigned short str2restrict(unsigned char *s);
int read_ini_info(void);
int read_config(void);
int save_config(void);
void read_nextern(void);
void read_editors(void);
void read_nintern(void);
int read_subs(void);
void read_networks(void);
int read_names(void);
void read_voting(void);
int read_dirs(void);
void read_chains(void);
int read_language(void);
int read_modem(void);
void read_gfile(void);
int make_abs_path(unsigned char *checkdir);
void fix_paths(void);
void init(void);
void gotcaller(unsigned int ms, unsigned int cs);


/* File: qwk.c */

void build_qwk_packet(void);
void qwk_gather_sub(int bn, struct qwk_junk *qwk_info);
void qwk_start_read(int msgnum, struct qwk_junk *qwk_info);
void make_pre_qwk(int msgnum, int *val, struct qwk_junk *qwk_info);
void put_in_qwk(postrec *m1, char *fn, int msgnum, struct qwk_junk *qwk_info);
void make_qwk_ready(char *text, long *len, char *address);
void qwk_remove_null(char *memory, int size);
void build_control_dat(struct qwk_junk *qwk_info);
int _fmsbintoieee(float *src4, float *dest4);
int _fieeetomsbin(float *src4, float *dest4);
char * qwk_system_name(char *qwkname);
void qwk_menu(void);
void qwk_send_file(char *fn, int *sent, int *abort);
int select_qwk_protocol(struct qwk_junk *qwk_info);
long * qwk_save_qscan(void);
void qwk_restore_qscan(long *save_qsc_p);
void insert_after_routing(char *text, char *text2insert, long *len);
void close_qwk_cfg(struct qwk_config *qwk_cfg);
void read_qwk_cfg(struct qwk_config *qwk_cfg);
void write_qwk_cfg(struct qwk_config *qwk_cfg);
int get_qwk_max_msgs(unsigned int *max_msgs, unsigned int *max_per_sub);
void qwk_nscan(void);
void finish_qwk(struct qwk_junk *qwk_info);
char *qwk_readfile(messagerec *m1, char *aux, long *l);
int qwk_open_file(char *fn);


/* File: qwk1.c */

void qwk_remove_email(void);
void qwk_gather_email(struct qwk_junk *qwk_info);
int select_qwk_archiver(struct qwk_junk *qwk_info, int ask);
void qwk_which_zip(char *thiszip);
void qwk_which_protocol(char *thisprotocol);
void upload_reply_packet(void);
void ready_reply_packet(char *name);
void make_text_ready(char *text, long len);
char * make_text_file(int filenumber, long *size, int curpos, int blocks);
void qwk_email_text(char *text, long size, char *title, char *to);
void qwk_inmsg(char *text, long size, messagerec *m1, char *aux, char *name, long thetime);
void process_reply_dat(char *name);
void qwk_post_text(char *text, long size, char *title, int sub);
int find_qwk_sub(struct qwk_sub_conf *subs, int amount, int fromsub, char *title);
void qwk_receive_file(char *fn, int *received, char *ft, int i);
void qwk_sysop(void);
void modify_bulletins(struct qwk_config *qwk_cfg);
void config_qwk_bw(void);
void config_qwk_rip(void);
void config_qwk(void);
char *qwk_current_text(int pos, char *text);


/* File: ripovl.c */

int rip_check(void);
int transmit(char menutype);
void setmenu(char *ext);
void set_name(void);
void choosemenu(void);
int current_menu(char *fn);
void rip_menus(void);
int pausecheck(int localflag, char *ext);
void end_rip(void);
void restore_msg_menu(void);
void msgheader(int startat);
void setmsgview(int smallby);
void submenu(void);
void hypertext(void);
void rip_show_chains(int *mapp, int *map, int start);
void rip_show_chains(int *mapp, int *map, int start);
void rip_list_gfiles(gfilerec *g, int nf, int start);
void rip_list_gfiles(gfilerec *g, int nf, int start);
void rip_list_sec(int *map, int nmap, int start);
void rip_list_sec(int *map, int nmap, int start);
void rip_timebank(void);
void rip_yourinfo(void);
int rip_printfileinfo(uploadsrec *u, int dn);
void rip_print_extended(char *fn, int *abort, unsigned char numlist);
void rip_detect(void);
char *interpret(char c);
int todownload(char array[][13], int start, char *fspec, char *fpath);
void menuxfer(char *fpath, char files[][13], int num, int *sent, double *percent);
int rip_file(char *fn, ripmsgrec *m, int maxary);
void rd_disable(void);
void rip_list_qscan(int start);
void rip_config_qscan(void);

#endif
