#include<stdarg.h>
#include<inttypes.h>
#include"dft.h"

#ifndef AUDIO_APP_H
#define AUDIO_aPP_H

typedef struct {
    char *from_file;
    char *to_file;
    char *tone_file;
    int ret;
    double amp;
    double freq;
    double phase;
    int sr;
    int64_t n;
} thread_args;

enum dtype {
    DTYPE_NONE, DTYPE_INT, DTYPE_FLOAT, DTYPE_DOUBLE, DTYPE_STR, DTYPE_I64
};

#endif

int error_dialog(char *fmt, ...);
void entry_to_variable(GtkEntry *e, void *var, enum dtype vtype);
GtkWidget *get_widget(char *name);
int dbgprintfimp(const char *fmt, va_list ap);
int dbgprintf(const char *fmt, ...);
int set_entry_text(GtkWidget *w, char *fmt, ...);
int update_entrys();
int update_convert_entrys();
char *get_entry_text(char *buff, GtkWidget *entry);
int init_globals(int argc, char **argv);
gboolean error_dialog_idle(gpointer msg);
gboolean stop_spinlock_idle(gpointer nop);
int print_variables();
int is_toggled(GtkRadioButton *b);
int get_filename_dialog(char *fname, char *dialog_title);
int update_convert_labels();
int thread_args_init(thread_args **ta);
int thread_args_destroy(thread_args *ta);

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
void on_convert_from_file_button_clicked(GtkButton *b);
void on_convert_sample_rate_entry_changed(GtkEntry *e);
void on_from_file_entry_changed(GtkEntry *e);
void on_to_file_entry_changed(GtkEntry *e);
void on_convert_file_button_clicked(GtkEntry *e);
int start_spinlock(char *calling_func_name);
int stop_spinlock(char *calling_func_name);

// Thread workers
void *save_to_wave(void *vargs);
void *save_to_data(void *vargs);
void *gensigcaller(void *);
void *savetonecaller(void *);