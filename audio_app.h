#include<stdarg.h>
#include"dft.h"

#ifndef AUDIO_APP_H
#define AUDIO_aPP_H

typedef struct{
    char *fn1;
    char *fn2;
    int ret;
    double amp;
    double freq;
    double phase;
    int sr;
    int n;
}gensig_args; 


#endif

GtkWidget *get_widget(char *name);
int dbgprintfimp(const char *fmt, va_list ap);
int dbgprintf(const char *fmt, ...);
int set_entry_text(GtkWidget *w, char *fmt, ...);
int update_tone_entrys();
char *get_entry_text(char *buff, GtkWidget *entry);
int init_globals(int argc, char **argv);
void *gensigcaller(void *);
void *savetonecaller(void *);
int print_tone();

// Signals
void on_tone_save_button_clicked(GtkButton *b);
void on_tone_amp_entry_changed(GtkEntry *e);
void on_tone_freq_entry_changed(GtkEntry *e);
void on_tone_phase_entry_changed(GtkEntry *e);
void on_tone_sr_entry_changed(GtkEntry *e);
void on_tone_secs_entry_changed(GtkEntry *e);
void on_tone_combo_changed(GtkComboBoxText *c);
void on_generate_tone_button_clicked(GtkButton *b);

