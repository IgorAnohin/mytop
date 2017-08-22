#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <ncurses.h>



#include "top.h"

#define State 3
#define Pid 6
#define Userid 9
#define Virt 18
#define Res 22
#define Shr 24
#define Utime 11
#define Stime 12
#define Start_time 19
#define Priority 15
#define Ni 16
#define Command 1


data_top solution[table_size];

static void count_different_proc(char temp) {

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

static long int parse_str(char * string){

    char* temp;

    temp = strstr(string,":");
    temp[0] = temp[1];
    return(strtol(temp,&temp,10));
}





static void get_data_top_proc(char dir[]) {

	long int parametr, number_string;
	char buffer[1500];
	char fstatus[100],fstat[100];
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


    snprintf(fstatus, 100 ,"/proc/%s/status", dir);
	ptrfstatus = fopen(fstatus, "r");

	number_string = 1;
	char* eoff="start";
	while (eoff != NULL)
	{
		eoff = fgets(buffer,100,ptrfstatus);

		switch ( number_string) {

            case Command:
                temp = strstr(buffer,":");
                int w = 0;
                while (temp[w]>'z' || temp[w]<'a')          //find begin name
                    w++;
                for (int q = 0; q+w<strlen(temp);q++)
                    solution[count_proc].com[q] = temp[q+w];

                break;

			case Pid:
                solution[count_proc].pid = parse_str(buffer);
				break;

			case Userid:
                solution[count_proc].userid = parse_str(buffer);
				break;

			case Virt:
                solution[count_proc].virt = parse_str(buffer);
				break;

			case Res:
                solution[count_proc].res = parse_str(buffer);
				break;

			case Shr:
                solution[count_proc].shr = parse_str(buffer);
				break;
		}
		number_string++;

	}
	fclose(ptrfstatus);

//get pr , ni , %cpu


    snprintf(fstat, 100 ,"/proc/%s/stat", dir);
	ptrfstat = fopen(fstat, "r");
	fgets(buffer,1000,ptrfstat);

	temp = strstr(buffer, ") ");
	temp[0] = ' '; // delete ')'
    solution[count_proc].S = temp[2];
	count_different_proc(solution[count_proc].S);
	temp[2] = ' '; // delete 's' or 'r' or 'd'...


	for(int j = 1; j<=30; j++ ){
		parametr = strtol(temp, &temp, 10);
//        printf("%ld",parametr);

		switch (j) {
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
	count_proc++;
}





static void userid_to_user_name() {

	int i = 0;
	FILE * ptrfile;
	char buffer[100];

	while (i < count_proc)
	{
		char* temp;
		char useridstr[150];

		snprintf(useridstr,100,"%ld",solution[i].userid);
		ptrfile = fopen("/etc/passwd", "r");
		fgets(buffer,100,ptrfile);

		temp = strstr(buffer, useridstr);
		while (temp == NULL) {
			fgets(buffer,100,ptrfile);
			temp = strstr(buffer, useridstr);
		}

		temp = strstr(buffer, ":");
		memcpy (solution[i].user, buffer, strlen(buffer)-strlen(temp));

		fclose(ptrfile);
		i++;
	}
}








static void readproc(){
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


static void print_proc(int i,long int ID, char *usern, long int PR, long int NI, long int VIRT, long int RES,
           long int SHR, char s, float CPU, long int TIME, char * COMMAND) {


	if (PR > -100)
    mvprintw(i+5,0,"%6ld %16s %5ld %5ld %8ld %9ld %8ld %3c  %4.1f",
            ID, usern, PR, NI, VIRT, RES, SHR, s, CPU/*, TIME*/);
        else mvprintw(i+5,0,"%6ld %16s    rt %5ld %8ld %9ld %8ld %3c  %4.1f",
            ID, usern, NI, VIRT, RES, SHR, s, CPU/*, TIME*/);

                        printw("  %2ld:%ld%ld.%ld%ld %s",
                TIME / (60 * 60), ( (TIME / 60) % 60) / 10, ( (TIME / 60) % 60) % 10,
                                (TIME / 10) % 10, TIME % 10, COMMAND);

}


static void print_tasks() {

mvprintw(1,0,"Tasks: %6d total, %6d running, %6d sleeping, %6d stopped, %4d zombie\n",
		count_proc, quantity_run_proc, quantity_sleep_proc, quantity_stop_proc, quantity_zomb_proc);

}

//print_scroll

static void print_scroll(int table){

    clear();
    mvprintw(0,0,"My top - %s",sprint_uptime());
    print_tasks();
    print_mem();


    mvprintw(4,0,"   PID             USER    PR    NI     VIRT       RES      SHR   S  %%CPU   TIME    COMMAND");


    quicksort(solution, count_proc, sizeof(data_top),sorting,&sort_flag);
    for (int i = 0; i+table < count_proc && i<37; i++)
           print_proc(i,solution[i+table].pid, solution[i+table].user, solution[i+table].pr, solution[i+table].ni,
                    solution[i+table].virt, solution[i+table].res, solution[i+table].shr, solution[i+table].S, solution[i+table].cpu, solution[i+table].stime,
                    solution[i+table].com);

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
            mvprintw(4,19,"USER");
            break;

        case 4:
        case 5:
            mvprintw(4,27,"PR");
             break;
        case 6:
        case 7:
            mvprintw(4,33,"NI");
            break;
        case 8:
        case 9:
            mvprintw(4,40,"VIRT");
            break;
        case 10:
        case 11:
            mvprintw(4,51,"RES");
            break;

        case 12:
        case 13:
            mvprintw(4,60,"SHR");
            break;

        case 14:
        case 15:
            mvprintw(4,66,"S");
             break;
        case 16:
        case 17:
            mvprintw(4,69,"%%CPU");
            break;
        case 18:
        case 19:
            mvprintw(4,76,"TIME");
            break;
        case 20:
        case 21:
            mvprintw(4,84,"COMMAND");
            break;


    }

    attroff(COLOR_PAIR(1));

    refresh();




}


int main(int argc, char* argv[])
{
    int c;
    enum{UP,DOWN,RIGHT,LEFT};
    int table = 0;

    initscr();
    noecho();
    halfdelay(10);
    bool loop = 1;
    keypad(stdscr,TRUE);

    while (loop){


        count_proc = 0;
        meminfo();
        readproc();

    print_scroll(table);
       c=getch();
   switch(c){
      case KEY_UP:
      case 'u':
      case 'U': if (table > 0) table--;
                break;       
      case KEY_DOWN:
      case 'd':
      case 'D': if (table < count_proc-36) table++;
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
