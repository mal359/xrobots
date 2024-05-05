/*
 * score.c  --  xrobots
 *
 * 
 * Copyright 1989 Brian Warkentine
 * 
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the author's name not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE 
 * AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN 
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * The author's current employer has no connection with this software, as it
 * was written before being employed at Sun.  The following address is for 
 * contacting the author only and does not imply any responsibility on the 
 * behalf of Sun Microsystems.
 * 
 * Contact the author at:
 * 	Brian Warkentine  (brianw@Sun.COM)
 * 	Sun Microsystems
 * 	2550 Garcia Avenue
 * 	Mountain View, Ca 94043-1100
 *
 *----------------------------------------------------------------------
 * flock() is used to stop a race condition within... if you don't 
 * have a bsd-like flock(), you could probably comment it out.
 * If SYSV is defined, this will be #ifdef'd out.
 * The race condition (multiple users writing to the score file) is 
 * probably rare.
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifdef R3
#include <X11/Box.h>
#include <X11/Command.h>
#include <X11/Label.h>
#else
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#endif

#include <X11/Xos.h>	/* brings in <sys/file.h> */
#ifdef __linux__	/* ...not on Linux! */
#include <sys/file.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "xrobots.h"

/*----------------------------------------------------------------------*/

typedef struct {
  char 	score[6], 
	name[26];
} SCORE;

static SCORE scores[MAXSCORES];

static void 	show_scores(), 
		new_high_score(), 
		load_scores(), 
		write_out_scores();

static FILE *scorefile = 0;

/*----------------------------------------------------------------------*/

void
check_score(current_score)
  int current_score;
{

  load_scores();

  if(current_score > atoi(scores[MAXSCORES-1].score)) {
    new_high_score(current_score);
    write_out_scores();
  }
  if(scorefile) {
#ifndef SYSV
    flock(scorefile->_fileno, LOCK_UN);
#endif
    fclose(scorefile);
    show_scores();
  }
}


static void
load_scores()
{
  int i = 0;

  if( !(scorefile = fopen(app_data.score_filename,"r+")) ) {
    scorefile = fopen(app_data.score_filename, "w");
    return;
  }
#ifndef SYSV
  flock(scorefile->_fileno, LOCK_EX);
#endif
  for(i = 0; i < MAXSCORES; i++) {
    if(!fgets(scores[i].score, 6, scorefile)) 	/* get score */
      break;
    if(!fgets(scores[i].name, 26, scorefile)) 	/* get name */
      break;
    if(!fgetc(scorefile))			/* and newline */
      break;
  }
}


static void
new_high_score(current_score)
  int current_score;
{
  int i;
  char textscore[6],
       name[26];

  snprintf(textscore, sizeof(textscore), "%5d", current_score);
  snprintf(name, sizeof(name), "%s", getenv("USER") ? getenv("USER") : "Unknown");

  for(i=MAXSCORES-2;i>=0;i--)
    if( current_score < atoi(scores[i].score) ) {
	/* move new score into i+1 slot */
	snprintf(scores[i + 1].score, sizeof(scores[i + 1].score), "%s", scores[i].score);
	snprintf(scores[i + 1].name, sizeof(scores[i + 1].name), "%s", scores[i].name);
    } else {
	snprintf(scores[i + 1].score, sizeof(scores[i + 1].score), "%s", textscore);
	snprintf(scores[i + 1].name, sizeof(scores[i + 1].name), "%s", name);
	return;
    }
  /* if it got here, there is a new number 1 score */
  snprintf(scores[0].score, sizeof(scores[0].score), "%s", textscore);
  snprintf(scores[0].name, sizeof(scores[0].name), "%s", name);

}


static void
write_out_scores()
{
  int i;

  if( !scorefile )
    return;
  rewind(scorefile);
  for(i=0;i<MAXSCORES;i++)
    fprintf(scorefile,"%5s%25s\n",scores[i].score,scores[i].name);
}


/*----------------------------------------------------------------------*/


Widget score_popup;
Widget score_labels[MAXSCORES];

static Arg arglist_score_title[] = {
  {  XtNborderWidth, (XtArgVal) 0  },
  {  XtNresize, (XtArgVal) False  },
  {  XtNwidth, (XtArgVal) 300   },
  {  XtNheight, (XtArgVal) 30   },
  {  XtNlabel, (XtArgVal) "High Scores"  },
  {  XtNjustify, (XtArgVal) XtJustifyCenter  },
};

static Arg arglist_score_label[] = {
  {  XtNlabel, (XtArgVal) 0  },
  {  XtNborderWidth, (XtArgVal) 0  },
  {  XtNresize, (XtArgVal) False  },
  {  XtNwidth, (XtArgVal) 300   },
  {  XtNjustify, (XtArgVal) XtJustifyCenter  },
};

static Arg arglist_popdown[] = {
/*  {  XtNborderWidth, (XtArgVal) 2  },*/
  {  XtNresize, (XtArgVal) False  },
  {  XtNwidth, (XtArgVal) 300   },
  {  XtNlabel, (XtArgVal) "Pop Down"  },
  {  XtNjustify, (XtArgVal) XtJustifyCenter  },
};


/*ARGSUSED*/
static void
popdown_callback(w, closure, call_data)
  Widget w;
  caddr_t closure;
  caddr_t call_data;
{
  XtPopdown(score_popup);
}


void
create_high_score_popup(parent)
  Widget parent;
{
  int i;
  Widget score_box, popdown;

  score_popup = XtCreatePopupShell(
                                   "score_popup",
                                   transientShellWidgetClass,
                                   parent, 0,0);

  score_box = XtCreateManagedWidget(
                                    "score_box",
                                    boxWidgetClass,
                                    score_popup,
                                    0,0);

  (void)XtCreateManagedWidget(
                                    "score_title",
                                    labelWidgetClass,
                                    score_box,
                                    arglist_score_title,
                                    XtNumber(arglist_score_title));

  for(i=0;i<MAXSCORES;i++) {
    score_labels[i] = XtCreateManagedWidget(
                                    "alabel",
                                    labelWidgetClass,
                                    score_box,
                                    arglist_score_label,
                                    XtNumber(arglist_score_label));
  }

  popdown = XtCreateManagedWidget(
                                    "popdown",
                                    commandWidgetClass,
                                    score_box,
                                    arglist_popdown,
                                    XtNumber(arglist_popdown));
  XtAddCallback(popdown,XtNcallback,popdown_callback,0);
}



void
show_scores()
{
  int i;
  char tmp_str[64];
  Arg tmp_arg;

  for(i = 0;i<MAXSCORES;i++) {
    snprintf(tmp_str, sizeof(tmp_str), "%5s  %25s", scores[i].score, scores[i].name);
    XtSetArg(tmp_arg,XtNlabel,tmp_str);
    XtSetValues(score_labels[i],&tmp_arg,1);
  }
  XtPopup(score_popup, XtGrabNone);
}


/*----------------------------------------------------------------------*/


/*XtCallbackProc*/
void
show_scores_callback()
{
  load_scores();

  if(scorefile) {
#ifndef SYSV
    flock(scorefile->_fileno, LOCK_UN);
#endif
    fclose(scorefile);
    show_scores();
  }
}


