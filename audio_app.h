#include<stdarg.h>
#include"dft.h"

#ifndef AUDIO_APP_H
#define AUDIO_aPP_H

typedef struct {
    char *fn1;
    char *fn2;
    int ret;
    double amp;
    double freq;
    double phase;
    int sr;
    int n;
} gensig_args;

enum dtype {
    DTYPE_NONE, DTYPE_INT, DTYPE_FLOAT, DTYPE_DOUBLE, DTYPE_STR
};

#endif

void entry_to_variable(GtkEntry *e, void *var, enum dtype vtype);
GtkWidget *get_widget(char *name);
int dbgprintfimp(const char *fmt, va_list ap);
int dbgprintf(const char *fmt, ...);
int set_entry_text(GtkWidget *w, char *fmt, ...);
int update_entrys();
char *get_entry_text(char *buff, GtkWidget *entry);
int init_globals(int argc, char **argv);
void *gensigcaller(void *);
void *savetonecaller(void *);
int print_variables();
int is_toggled(GtkRadioButton *b);

// Signals
void on_show_variables_button_clicked(GtkButton *b);
void on_tone_save_button_clicked(GtkButton *b);
void on_tone_amp_entry_changed(GtkEntry *e);
void on_tone_freq_entry_changed(GtkEntry *e);
void on_tone_phase_entry_changed(GtkEntry *e);
void on_tone_sr_entry_changed(GtkEntry *e);
void on_tone_secs_entry_changed(GtkEntry *e);
void on_tone_combo_changed(GtkComboBoxText *c);
void on_generate_tone_button_clicked(GtkButton *b);
void on_wav2data_radio_toggled(GtkRadioButton *b);
void on_data2wave_radio_toggled(GtkRadioButton *b);
void on_convert_to_file_button_clicked(GtkButton *b);
void on_convert_from_button_clicked(GtkButton *b);
void on_convert_sample_rate_entry_changed(GtkEntry *e);
void on_from_file_entry_changed(GtkEntry *e);
void on_to_file_entry_changed(GtkEntry *e);
void on_convert_file_button_clicked(GtkEntry *e);