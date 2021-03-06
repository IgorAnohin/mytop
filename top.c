#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>

#include "top.h"
#include "memory.h"
#include "time.h"
#include "sort.h"

#define BAD_OPEN_DIR 						\
"Error: /proc must be mounted\n"				\
"  To mount /proc at boot you need an /etc/fstab line like:\n"	\
"      /proc   /proc   proc    defaults\n"			\
"  In the meantime, mount /proc /proc -t proc\n"


enum {
    Utime = 11,
    Stime = 12,
    Priority = 15,
    Ni = 16,
    Start_time = 19,
};


int count_proc;
int	quantity_sleep_proc;
int	quantity_run_proc;
int	quantity_stop_proc;
int quantity_zomb_proc;

#define table_size 1000
#define stat_count 20
#define comand_table_size 36


enum {
    PID_Ascending, //0
    PID_Descending,
    User_Ascending,
    User_Descending,
    Pr_Ascending,
    Pr_Descending,
    Ni_Ascending,
    Ni_Descending,
    Virt_Ascending,
    Virt_Descending,
    Res_Ascending,
    Res_Descending,
    Shr_Ascending,
    Shr_Descending,
    S_Ascending,
    S_Descending,
    Cpu_Ascending,
    Cpu_Descending,
    Time_Ascending,
    Time_Descending,
    Command_Ascending,
    Command_Descending  //21
};


static int sorting(const void * x,const void * x1, void * arg) {
    data_top * P = (data_top *)x;
    data_top * P1 = (data_top *)x1;
    int flag = *(int*)arg;

    switch( flag) {
        case PID_Ascending:
            return (P->pid - P1->pid);
            break;
        case PID_Descending:
            return (P1->pid - P->pid);
            break;

        case User_Ascending:
            return (strcmp(P->user, P1->user) );
            break;
        case User_Descending:
            return (strcmp(P1->user, P->user) );
            break;

        case Pr_Ascending:
            return (P->pr - P1->pr);
            break;
        case Pr_Descending:
            return (P1->pr - P->pr);
            break;

        case Ni_Ascending:
            return (P->ni - P1->ni);
            break;
        case Ni_Descending:
            return (P1->ni - P->ni);
            break;

        case Virt_Ascending:
            return (P->virt - P1->virt);
            break;
        case Virt_Descending:
            return (P1->virt - P->virt);
            break;

        case Res_Ascending:
            return (P->res - P1->res);
            break;
        case Res_Descending:
            return (P1->res - P->res);
            break;

        case Shr_Ascending:
            return (P->shr - P1->shr);
            break;
        case Shr_Descending:
            return (P1->shr - P->shr);
            break;

        case S_Ascending:
            return (P->S - P1->S);
            break;
        case S_Descending:
            return (P1->S - P->S);
            break;

        case Cpu_Ascending:
            return (P->cpu - P1->cpu);
            break;
        case Cpu_Descending:
            return (P1->cpu - P->cpu);
            break;

        case Time_Ascending:
            return (P->stime - P1->stime);
            break;
        case Time_Descending:
            return (P1->stime - P->stime);
            break;

        case Command_Ascending:
            return (strcmp(P1->com, P->com) );
            break;
        case Command_Descending:
            return (strcmp(P->com, P1->com) );
            break;



    }

}

data_top solution[table_size];

static void count_different_proc( char temp)
{

    switch (temp){
        case 'S':
        case 'D':
            quantity_sleep_proc++;
            break;
        case 'R':
            quantity_run_proc++;
            break;
        case 'Z':
            quantity_zomb_proc++;
            break;
        default:
            quantity_stop_proc++;
            break;
    }
}

static long int parse_str(char * string)
{

    char* temp;
    temp = strstr(string, ":");
    temp[0] = temp[1];
    return( strtol(temp, &temp, 10) );
}





static void get_data_top_proc(char dir[])
{

	long int parametr, number_string;
	char buffer[1500];
    char fstatus[PATH_MAX],fstat[PATH_MAX];
	char * temp;
	FILE * ptrfstatus;
	FILE * ptrfstat;
    FILE * ptrproc_stat;
    long int totalcpu;

	if (count_proc == 0)
		{

		quantity_sleep_proc = 0,
		quantity_run_proc = 0,
		quantity_stop_proc = 0,
		quantity_zomb_proc = 0;

        }


    snprintf(fstatus, PATH_MAX ,"/proc/%s/status", dir);
	ptrfstatus = fopen(fstatus, "r");

	char* eoff="start";
	while (eoff != NULL)
	{
        eoff = fgets(buffer,100,ptrfstatus);

            if ( strstr(buffer,"Name:") != NULL)
            {
                temp = strstr(buffer, ":");
                int w = 0;
                while (temp[w] > 'z' || temp[w] < 'a')          //find begin name
                    w++;
                for (int q = 0; (q + w) < strlen(temp); q++)
                    solution[count_proc].com[q] = temp[q+w];
            }

            if (strstr(buffer,"Pid:") == buffer)
                solution[count_proc].pid = parse_str(buffer);

            if ( strstr(buffer,"Uid:") != NULL)
                solution[count_proc].userid = parse_str(buffer);

            if (strstr(buffer,"Vmsize:") != NULL)
                solution[count_proc].virt = parse_str(buffer);

            if (strstr(buffer,"VmRSS:") != NULL)
                solution[count_proc].res = parse_str(buffer);

            if (strstr(buffer,"RssFile:") != NULL)
                solution[count_proc].shr = parse_str(buffer);

	}
	fclose(ptrfstatus);

//get pr , ni , %cpu


    snprintf(fstat, PATH_MAX, "/proc/%s/stat", dir);
	ptrfstat = fopen(fstat, "r");
    fgets(buffer, 1000, ptrfstat);

	temp = strstr(buffer, ") ");
	temp[0] = ' '; // delete ')'
    solution[count_proc].S = temp[2];
	count_different_proc(solution[count_proc].S);
	temp[2] = ' '; // delete 's' or 'r' or 'd'...


    for(int j = 1; j<=stat_count; j++ )
    {
		parametr = strtol(temp, &temp, 10);

        switch (j)
        {
			case Utime:
				solution[count_proc].utime = parametr;
				break;

            case Stime:
                solution[count_proc].stime = parametr;
                break;

            case Start_time:
                solution[count_proc].start_time = parametr;

                break;

			case Priority:
				solution[count_proc].pr = parametr;
				break;

			case Ni:
				solution[count_proc].ni = parametr;
				break;
		}
	}

	fclose(ptrfstat);

    //get total cpu
    ptrproc_stat = fopen("/proc/stat", "r");
    fgets(buffer, 100, ptrproc_stat);
    fclose(ptrproc_stat);
    temp = strstr(buffer, " ");
    totalcpu = strtol(temp, &temp, 10);

    solution[count_proc].cpu = (solution[count_proc].stime + solution[count_proc].utime) * 100 / totalcpu;
    if (solution[count_proc].cpu > 100) solution[count_proc].cpu = 0;

    count_proc++;
}





static void userid_to_user_name()
{

	int i = 0;
	FILE * ptrfile;
	char buffer[100];

	while (i < count_proc)
	{
		char* temp;
		char useridstr[150];

        snprintf(useridstr, 100, "%ld", solution[i].userid);
		ptrfile = fopen("/etc/passwd", "r");
        fgets(buffer, 100, ptrfile);

		temp = strstr(buffer, useridstr);
		while (temp == NULL) {
            fgets(buffer, 100, ptrfile);
			temp = strstr(buffer, useridstr);
		}

		temp = strstr(buffer, ":");
		memcpy (solution[i].user, buffer, strlen(buffer)-strlen(temp));

		fclose(ptrfile);
		i++;
	}
}








static void readproc()
{
	DIR *proc;
	struct dirent * path;

	proc = opendir("/proc");

	if ( !proc )
		fprintf(stderr, BAD_OPEN_DIR);

	path = readdir(proc);
	while (  *path -> d_name < '0' || *path -> d_name > '9' )
		path = readdir(proc);

	while ( path != NULL  &&  *path->d_name >= '1' && *path->d_name <= '9' ) {
        get_data_top_proc(path->d_name);
		path = readdir(proc) ;
	}

	userid_to_user_name();

        if ( proc )
            closedir(proc);
}


//print


static void print_mem() {

mvprintw(2,0,"KiB Mem : %10d total, %10d free, %10d used, %d buff/cached\n",
			kb_main_total, kb_main_free, kb_main_used, kb_main_buffers+kb_main_cached);
mvprintw(3,0,"KiB Swap: %10d total, %10d free, %10d used\n" ,
			kb_swap_total, kb_swap_free, kb_swap_used);

}


static void print_proc(int i, long int ID, char *usern, long int PR, long int NI, long int VIRT, long int RES,
           long int SHR, char s, float CPU, time_t time, char * COMMAND) {

    struct tm *runtime;
    runtime = gmtime( &time );

	if (PR > -100)
    mvprintw(i+5,0,"%6ld %18s %5ld %5ld %8ld %9ld %8ld %3c  %4.1f %02d:%02d.%02d %s",
            ID, usern, PR, NI, VIRT, RES, SHR, s, CPU,runtime->tm_hour, runtime->tm_min, runtime->tm_sec, COMMAND);
        else mvprintw(i+5,0,"%6ld %18s    rt %5ld %8ld %9ld %8ld %3c  %4.1f %02d:%02d.%02d %s",
            ID, usern, NI, VIRT, RES, SHR, s, CPU,runtime->tm_hour, runtime->tm_min, runtime->tm_sec, COMMAND);


}


static void print_tasks() {

mvprintw(1,0,"Tasks: %6d total, %6d running, %6d sleeping, %6d stopped, %4d zombie\n",
		count_proc, quantity_run_proc, quantity_sleep_proc, quantity_stop_proc, quantity_zomb_proc);

}

//print_scroll

static void print_scroll(int table,int sort_flag)
{

    clear();
    mvprintw(0,0,"My top - %s",sprint_uptime());
    print_tasks();
    print_mem();

    mvprintw(4,0,"   PID               USER    PR    NI     VIRT       RES      SHR   S  %%CPU   TIME    COMMAND");


    quicksort(solution, count_proc, sizeof(data_top),sorting,&sort_flag);

    for (int i = 0; i + table < count_proc && i-1 <= comand_table_size; i++)
           print_proc(i,  solution[i+table].pid,  solution[i+table].user,  solution[i+table].pr,  solution[i+table].ni,
                    solution[i+table].virt,  solution[i+table].res,  solution[i+table].shr,  solution[i+table].S,
                    solution[i+table].cpu,  solution[i+table].stime,  solution[i+table].com);

    start_color(); //set color
    init_pair(1,COLOR_GREEN, COLOR_BLUE);
    attron(COLOR_PAIR(1));

    switch(sort_flag) {
        case 0:
        case 1:
            mvprintw(4,0,"   PID");
            break;

        case 2:
        case 3:
            mvprintw(4,21,"USER");
            break;

        case 4:
        case 5:
            mvprintw(4,29,"PR");
             break;
        case 6:
        case 7:
            mvprintw(4,35,"NI");
            break;
        case 8:
        case 9:
            mvprintw(4,42,"VIRT");
            break;
        case 10:
        case 11:
            mvprintw(4,53,"RES");
            break;

        case 12:
        case 13:
            mvprintw(4,62,"SHR");
            break;

        case 14:
        case 15:
            mvprintw(4,68,"S");
             break;
        case 16:
        case 17:
            mvprintw(4,71,"%%CPU");
            break;
        case 18:
        case 19:
            mvprintw(4,78,"TIME");
            break;
        case 20:
        case 21:
            mvprintw(4,86,"COMMAND");
            break;


    }

    attroff(COLOR_PAIR(1));

    refresh();




}


int main(int argc, char* argv[])
{
    int sort_flag = 0;


    int c;
    int table = 0;
    enum
    {
        UP, DOWN, RIGHT, LEFT
    };

    initscr();
    noecho();
    halfdelay(30);
    bool loop = 1;
    keypad(stdscr,TRUE);

    while (loop){

    count_proc = 0;
    meminfo();
    readproc();

    print_scroll(table, sort_flag);
       c=getch();
    switch(c){
       case KEY_UP:
       case 'u':
       case 'U': if (table > 0) table--;
                 break;
       case KEY_DOWN:
       case 'd':
       case 'D': if (table < count_proc-comand_table_size) table++;
                 break;
       case KEY_LEFT:
       case 'l': if (sort_flag > 0) sort_flag--;
                 break;
       case KEY_RIGHT:
       case 'r': if (sort_flag < 21) sort_flag++;
                  break;
       case 'q':
       case 'Q': loop=FALSE;

       case 'i': sort_flag=0;
                 break;
       case 'I': sort_flag=1;
                 break;

       case 'n': sort_flag=2;
                 break;
       case 'N': sort_flag=3;
                 break;

       case 'p': sort_flag=4;
                 break;
       case 'P': sort_flag=5;
                 break;

      default: break;
   }

}
    endwin();
	return 0;
}
