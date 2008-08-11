// AtomicRTAI - Simple diagnostics.
// Copyright (C) 2000 Lineo ISG
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// AUTHOR: Brendan Knox <brendank@lineoisg.com>
// DATE  : Tue Jun 6 2000

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "ncurses.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define INSMOD        "/sbin/insmod"
#define RMMOD         "/sbin/rmmod"
#define MODPROBE      "/sbin/modprobe"

#define ZCOLOR_BASE    1
#define ZCOLOR_TITLE   2
#define ZCOLOR_LINE    3
#define ZCOLOR_HEAD    4
#define ZCOLOR_DATA1   5
#define ZCOLOR_DATA2   6

int show_jitter(int x, int y);
void show_jitter_hdr(int x, int y);
int show_context(int x, int y);
void show_context_hdr(int x, int y);
int show_interrupts(int x, int y);
void show_interrupts_hdr(int x, int y);
void quit(char reason[]);
void finish(int sig);
int insmod(char module[]);
int rmmod(char module[]);
int modprobe(char module[]);

int main(void)
{

   WINDOW *mywin;
   int i = 1;
   int j;
   int c;

   signal(SIGINT, finish);
   unsetenv("SHINIT");

   // Load up the RTAI modules we require for this test.
   if (modprobe("rtai") == -1)
      quit("Unable to load 'rtai' module.");
   if (modprobe("rtai_sched granularity=8192") == -1)
      quit("Unable to load 'rtai_sched' module.");


   mywin = initscr();
   keypad(stdscr, TRUE);
   nonl();
   cbreak();
   noecho();
   nodelay(mywin, TRUE);
   intrflush(stdscr, FALSE);
   start_color();

   init_pair(ZCOLOR_BASE,   COLOR_YELLOW, COLOR_BLUE);
   init_pair(ZCOLOR_TITLE,  COLOR_WHITE,  COLOR_BLUE);
   init_pair(ZCOLOR_LINE,   COLOR_CYAN,   COLOR_BLUE);
   init_pair(ZCOLOR_HEAD,   COLOR_BLUE,   COLOR_BLUE);
   init_pair(ZCOLOR_DATA1,  COLOR_GREEN,  COLOR_BLUE);
   init_pair(ZCOLOR_DATA2,  COLOR_RED,    COLOR_BLUE);


   // Splat blue all over the screen, probably a better way to do this!
   bkgdset(A_NORMAL | COLOR_PAIR(ZCOLOR_BASE) | ' ');
   for (j = 0; j < LINES; j++) {
      mvprintw(j,0, "%*s", COLS, " ");
   }

   attrset(A_BOLD);
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(0, 0, "AtomicRTAI v%d.%d System Diagnostics", VERSION_MAJOR, VERSION_MINOR);
   mvprintw(0, 54, "Copyright 2000 Lineo ISG.");
   color_set(ZCOLOR_LINE, NULL);
   mvprintw(1, 0, "-------------------------------------------------------------------------------");
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(2, 0, "Running test:");
   show_jitter_hdr(4,0);
   show_context_hdr(8,0);
   show_interrupts_hdr(11,0);
   mvprintw(LINES -1, 0, "Press Q to quit the diagnostics screen and drop to a shell.");
   refresh();

   while (1) {


      // Check for keypresses first, flush other keypresses.
      while ( (c = getch()) != ERR) {
         if (c == 'q' || c == 'Q')
            quit("User requested exit.");
      }
      

      if (i == 1) {

         color_set(ZCOLOR_DATA1, NULL);
         mvprintw(2,14, "jitter test...                   ");
         refresh();
         if (show_jitter(4,0) != 0)
            quit("Problem obtaining the jitter from /proc/jitter!");
         i = 0;

      } else {

         color_set(ZCOLOR_DATA1, NULL);
         mvprintw(2,14, "context switch test...           ");
         refresh();
         if (show_context(8,0) != 0)
            quit("Problem obtaining the context switch time from /proc/context!");
         i = 1;

      }

      if (show_interrupts(11,0) != 0)
          quit("Problem obtaining the interrupts from /proc/interrupts!");

   }

   endwin();

}


void finish(int sig)
{

   quit("User request exist.");

}

void quit(char reason[])
{

   endwin();
   rmmod("jitter_mod > /dev/null 2>&1");
   rmmod("cs_mod > /dev/null 2>&1");
   rmmod("rtai_sched");
   rmmod("rtai");
   printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
   printf("Exit reason: %s\n\n", reason);
   exit(-1);

}

void show_jitter_hdr(int x, int y)
{

   color_set(ZCOLOR_LINE, NULL);
   mvprintw(x, y, "-[");
   attroff(A_BOLD);
   printw("Jitter Measurements");
   attron(A_BOLD);
   printw("]---------------------------------------------------------");
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+1, y, "Jitter (before deadline):");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+1, y+26, "%10ld ns", 0);
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+2, y, "Jitter (after deadline):");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+2, y+26, "%10ld ns", 0);

}

int show_jitter(int x, int y)
{

   FILE *procfile;
   char jitterline[1024];
   long min, max;
   static long hmin = 0;
   static long hmax = 0;

   show_jitter_hdr(x,y);


   if (insmod("./jitter_mod.o") != 0) {
      return -1;
   }

   // Sleep for a little while to allow the jitter to settle.
   usleep(2000000);

jitteragain:
   // Grab the jitter from /proc
   procfile = fopen("/proc/jitter", "r");
   if (procfile == NULL) {
      return -1;
   }

   if (fread(jitterline, 1024, 1, procfile) <= 0) {
      if (ferror(procfile)) {
         return -1;
      }
   }
   sscanf(jitterline, "min: %ld ns, max: %ld ns", &min, &max);
   fclose(procfile);
  
   if (min == 0 && max == 0) goto jitteragain;

   if (hmin > min) hmin = min;
   if (hmax < max) hmax = max;
   
   // Print to x,y locations.
   mvprintw(x+1, y+26, "%10ld", hmin);
   mvprintw(x+2, y+26, "%10ld", hmax);


   if (rmmod("jitter_mod") != 0) {
      return -1;
   }

   refresh();
   return 0;

}

void show_context_hdr(int x, int y)
{


   color_set(ZCOLOR_LINE, NULL);
   mvprintw(x, y, "-[");
   attroff(A_BOLD);
   printw("Context Switch Measurements");
   attron(A_BOLD);
   printw("]-------------------------------------------------");
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+1, y, "Context Switch:");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+1, y+26, "%10ld ns", 0);
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+1, y+41, "Context Switch Avg:");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+1, y+66, "%10ld ns", 0);
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+2, y, "Worst Context Switch:");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+2, y+26, "%10ld ns", 0);
   color_set(ZCOLOR_TITLE, NULL);
   mvprintw(x+2, y+41, "Best Context Switch:");
   color_set(ZCOLOR_DATA2, NULL);
   mvprintw(x+2, y+66, "%10ld ns", 0);
}

int show_context(int x, int y)
{

   // Expect:
   //context switch time = 4222 ns

   FILE *procfile;
   char contextline[1024];
   long context;
   static long acontext = 0;
   static long worst = 0;
   static long best = 0;
   static long long totalcontext = 0;
   static long long numiterations = 0;

   show_context_hdr(x, y);

   if (insmod("./cs_mod.o") != 0) {
      return -1;
   }

   // Sleep for a little while to allow a context switch to be timed.
   usleep(3500000);

again:
   // Grab the context from /proc
   procfile = fopen("/proc/context", "r");
   if (procfile == NULL) {
      return -1;
   }

   if (fread(contextline, 1024, 1, procfile) != 1) {
      if (ferror(procfile)) {
         perror("fread");
         return -1;
      }
   }

   sscanf(contextline, "context switch time = %ld ns", &context);

   fclose(procfile);

   if (context == 0) goto again;

   if (worst < context) worst = context;
   if (best > context || best == 0) best = context;
   totalcontext += context;
   numiterations++;
   if (acontext == 0) {
      acontext = context;
   } else {
      acontext = totalcontext / numiterations;
   }

   // Print to x,y locations.
   mvprintw(x+1, y+26, "%10ld", context);
   mvprintw(x+1, y+66, "%10ld", acontext);
   mvprintw(x+2, y+26, "%10ld", worst);
   mvprintw(x+2, y+66, "%10ld", best);


   refresh();
   if (rmmod("cs_mod") != 0) {
      return -1;
   }
   return 0;

}

void show_interrupts_hdr(int x, int y)
{
   color_set(ZCOLOR_LINE, NULL);
   mvprintw(x, y, "-[");
   attroff(A_BOLD);
   printw("System Interrupts");
   attron(A_BOLD);
   printw("]-----------------------------------------------------------");
   
}

int show_interrupts(int x, int y)
{

   char interruptline[1024];
   FILE *procfile;
   int i=0;
   
   procfile = fopen("/proc/interrupts", "r");
   if (procfile == NULL) {
      return -1;
   }

   show_interrupts_hdr(x+i++, y);

   color_set(ZCOLOR_DATA1, NULL);

   while (!feof(procfile)) {
      if (fgets(interruptline, 1024, procfile) <= 0) {
         if (ferror(procfile)) {
            return -1;
         }
         if (feof(procfile)) {
            break;
         }
      }
      mvprintw(x+i++, y, "%s", interruptline);
   }

   fclose(procfile);

   refresh();
   return 0;

}


int insmod(char module[])
{

   char executethis[1024];

   sprintf(executethis, "%s %s", INSMOD, module);

   if (system(executethis) != 0) {
      return -1;
   }

   return 0;

}

int rmmod(char module[])
{

   char executethis[1024];

   sprintf(executethis, "%s %s", RMMOD, module);

   if (system(executethis) != 0) {
      return -1;
   }

   return 0;

}


int modprobe(char module[])
{

   char executethis[1024];

   sprintf(executethis, "%s %s", MODPROBE, module);

   if (system(executethis) != 0) {
      return -1;
   }

   return 0;

}
