#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<pthread.h>
#include<stdarg.h>
#include<ctype.h>
#include<glib.h>
#include<math.h>
#include"dft.h"
#include"audio_app.h"

#define MAXFNSIZE 1023
#define STRSIZE 255

char tone_save_filename[MAXFNSIZE + 1];
char convert_from_filename[MAXFNSIZE + 1];
char convert_to_filename[MAXFNSIZE + 1];

char tstr[STRSIZE + 1];

GtkWidget *window;
GtkBuilder *bld;
GtkWidget *tone_combo;
GtkWidget *tone_amp_entry;
GtkWidget *tone_freq_entry;
GtkWidget *tone_phase_entry;
GtkWidget *tone_sr_entry;
GtkWidget *tone_secs_entry;

GtkWidget *data2wave_radio;
GtkWidget *wave2data_radio;
GtkWidget *convert_to_file_button;
GtkWidget *convert_from_button;
GtkWidget *convert_sr_entry;
GtkWidget *from_file_entry;
GtkWidget *to_file_entry;
GtkWidget *convert_file_button;
GtkWidget *show_variables_button;

GdkCursor *busy;
GdkDisplay *dis;

#define DEBUG 1

char *tone_type_str[] = {"", "Sine", "Square", "Saw Tooth"};

int (*sigfunc[])(cmp_t *, double, double, double, int, int) = {NULL, sigsine, sigsquare, sigsaw};

pthread_mutex_t tone_generate_lock;
pthread_mutex_t gsargs_lock;
pthread_mutex_t busy_lock;

cmp_t *tone_data;
gensig_args gsargs;
int tone_type;
double tone_amp;
double tone_freq;
double tone_phase;
double tone_secs;
int tone_sr;
int src_is_wave; // Is the source file wave or data
int convert_sr;

extern char _binary_audio_xml_glade_end[];
extern char _binary_audio_xml_glade_size[];
extern char _binary_audio_xml_glade_start[];

// Print the message if DEBUG is set. NOP if not

int dbgprintfimp(const char *fmt, va_list ap) {
    if (DEBUG) {
        vfprintf(stderr, fmt, ap);
    }
    return 0;
}

#if DEBUG

int dbgprintf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    dbgprintfimp(fmt, ap);
    va_end(ap);
    return 0;
}
#else

int dbgprintf(const char *fmt, ...) {
    return 0;
}
#endif

GtkWidget *get_widget(char *name) {
    return GTK_WIDGET(gtk_builder_get_object(bld, name));
}

int set_entry_text(GtkWidget *w, char *fmt, ...) {
    GtkEntry *e;
    va_list ap;

    e = GTK_ENTRY(w);
    va_start(ap, fmt);
    if (strcmp(fmt, "%f") == 0) {
        snprintf(tstr, STRSIZE, "%f", va_arg(ap, double));
    } else if (strcmp(fmt, "%d") == 0) {
        snprintf(tstr, STRSIZE, "%d", va_arg(ap, int));
    } else {
        strcpy(tstr, "");
    }
    va_end(ap);
    dbgprintf("setting text to %s\n", tstr);
    gtk_entry_set_text(e, (const gchar *) tstr);
    return 0;
}

int update_entrys() {
    set_entry_text(tone_amp_entry, "%f", tone_amp);
    set_entry_text(tone_freq_entry, "%f", tone_freq);
    set_entry_text(tone_phase_entry, "%f", tone_phase);
    set_entry_text(tone_sr_entry, "%d", tone_sr);
    set_entry_text(tone_secs_entry, "%f", tone_secs);
    set_entry_text(convert_sr_entry, "%d", convert_sr);
}

void on_tone_save_button_clicked(GtkButton *b) {
    GdkWindow *win;
    gint res;
    char *file_name;
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    pthread_t th;

    win = gtk_widget_get_window(window);
    dbgprintf("tone save button clicked\n");
    dialog = gtk_file_chooser_dialog_new(
            "Save File",
            GTK_WINDOW(window),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "Cancel",
            GTK_RESPONSE_CANCEL,
            "Save",
            GTK_RESPONSE_ACCEPT,
            NULL);
    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_name(chooser, tone_save_filename);
    gtk_file_chooser_set_filename(chooser, tone_save_filename);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        file_name = gtk_file_chooser_get_filename(chooser);
        strncpy(tone_save_filename, file_name, MAXFNSIZE);
        g_free(file_name);
        dbgprintf("SAVEING %s\n", tone_save_filename);
        dbgprintf("save tone file: busy lock()\n");
        pthread_mutex_lock(&busy_lock);
        gdk_window_set_cursor(win, busy);
        dbgprintf("save tone file: busy unlock()\n");
        pthread_mutex_unlock(&busy_lock);
        dbgprintf("save tone file: gsargs lock()\n");
        pthread_mutex_lock(&gsargs_lock);
        gsargs.n = (int) (tone_sr * tone_secs + 1);
        gsargs.fn1 = tone_save_filename;
        dbgprintf("save tone file: gsargs unlock()\n");
        pthread_mutex_unlock(&gsargs_lock);
        if (pthread_create(&th, NULL, savetonecaller, NULL) == 0) {
            dbgprintf("Save tone file thread creqted\n");
            pthread_detach(th);
        }
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void *savetonecaller(void *args) {
    GdkWindow *win;
    char *file_name;
    int n;

    win = gtk_widget_get_window(window);
    dbgprintf("savetonecaller: lock gsargs\n");
    pthread_mutex_lock(&gsargs_lock);
    n = gsargs.n;
    file_name = gsargs.fn1;
    dbgprintf("savetonecaller: unlock gsargs\n");
    pthread_mutex_unlock(&gsargs_lock);
    dbgprintf("savetonecaller: lock tone_generate\n");
    pthread_mutex_lock(&tone_generate_lock);
    dbgprintf("savetonecaller: saving file\n");
    if (tone_data != NULL) {
        dft_fwrite(file_name, tone_data, n);
    }
    dbgprintf("savetonecaller: unlock tone_generate\n");
    pthread_mutex_unlock(&tone_generate_lock);
    dbgprintf("savetonecaller: lock busy\n");
    pthread_mutex_lock(&busy_lock);
    gdk_window_set_cursor(win, NULL);
    dbgprintf("savetonecaller: unlock nusy\n");
    pthread_mutex_unlock(&busy_lock);
    dbgprintf("Save file thread exit\n");
    pthread_exit(NULL);
}

void entry_to_variable(GtkEntry *e, void *val, enum dtype vtype) {
    char *str;
    str = (char *) gtk_entry_get_text(e);
    switch (vtype) {
        case DTYPE_INT:
            *(int *) val = atoi(str);
            break;
        case DTYPE_DOUBLE:
            *(double *) val = atof(str);
            break;
        case DTYPE_FLOAT:
            *(float *) val = atof(str);
            break;
    }
}

void on_tone_amp_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &tone_amp, DTYPE_DOUBLE);
    update_entrys();
}

void on_tone_freq_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &tone_freq, DTYPE_DOUBLE);
    update_entrys();
}

void on_tone_phase_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &tone_phase, DTYPE_DOUBLE);
    update_entrys();
}

void on_tone_sr_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &tone_sr, DTYPE_INT);
    if (tone_sr < 0) {
        tone_sr = 0;
    }
    update_entrys();
}

void on_tone_secs_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &tone_secs, DTYPE_DOUBLE);
    update_entrys();
}

void on_tone_combo_changed(GtkComboBoxText *c) {
    char *box_text;
    int i;

    box_text = gtk_combo_box_text_get_active_text(c);
    tone_type = 0;
    dbgprintf("tone_type box is %s\n", (char *) box_text);
    if (box_text != NULL) {
        for (i = 0; i <= 3; i++) {
            if (strcmp(tone_type_str[i], box_text) == 0) {
                tone_type = i;
            }
        }
        g_free(box_text);
    }
}

char *get_entry_text(char *buff, GtkWidget *entry) {
    strncpy(buff, (char *) gtk_entry_get_text(GTK_ENTRY(entry)),
            STRSIZE);
    return buff;
}

int print_variables() {
    char *in_format;
    char *out_format;
    dbgprintf("Tone Entries\n");
    dbgprintf("---------------------------------------------\n");
    dbgprintf("amplitutde: %s\n", get_entry_text(tstr, tone_amp_entry));
    dbgprintf("frequency: %s\n", get_entry_text(tstr, tone_freq_entry));
    dbgprintf("phase: %s\n", get_entry_text(tstr, tone_phase_entry));
    dbgprintf("sample rate: %s\n", get_entry_text(tstr, tone_sr_entry));
    dbgprintf("seconds: %s\n", get_entry_text(tstr, tone_secs_entry));
    dbgprintf("\n");
    dbgprintf("Tone values\n");
    dbgprintf("--------------------------------------------\n");
    dbgprintf("tone type:   %s\n", tone_type_str[tone_type]);
    dbgprintf("amplitude:   %f\n", tone_amp);
    dbgprintf("frequency:   %f\n", tone_freq);
    dbgprintf("phase:       %f\n", tone_phase);
    dbgprintf("sample rate: %d\n", tone_sr);
    dbgprintf("seconds:     %f\n", tone_secs);
    dbgprintf("\n");
    dbgprintf("\n");
    dbgprintf("Wave files\n");
    dbgprintf("---------------------------------------------\n");
    dbgprintf("convert_to_filename: %s\n", convert_from_filename);
    dbgprintf("convert_to_filename: %s\n", convert_to_filename);
    if (src_is_wave) {
        in_format = "wave";
        out_format = "data";
    } else {
        in_format = "data";
        out_format = "wave";
    }
    dbgprintf("Conversion sample rate: %d\n", convert_sr);
    dbgprintf("input format:  %s\n", in_format);
    dbgprintf("output format: %s\n", out_format);
    return 0;
}

void on_generate_tone_button_clicked(GtkButton *b) {
    pthread_t th;
    ssize_t tsize;
    GdkWindow *win;

    print_variables();
    win = gtk_widget_get_window(window);
    dbgprintf("tone button: Busy lock()\n");
    pthread_mutex_lock(&busy_lock);
    dbgprintf("tone button: starting busy\n");
    gdk_window_set_cursor(win, busy);
    pthread_mutex_unlock(&busy_lock);
    dbgprintf("tone_button: sbusy unlock\n");
    pthread_mutex_lock(&gsargs_lock);
    gsargs.amp = tone_amp;
    gsargs.freq = tone_freq;
    gsargs.phase = tone_phase;
    gsargs.sr = tone_sr;
    gsargs.n = (int) (tone_sr * tone_secs + 1);
    tsize = sizeof (cmp_t) * gsargs.n;
    dbgprintf("n = %d tsize = %d\n", gsargs.n, tsize);
    dbgprintf("tone button: tone lock\n");
    pthread_mutex_lock(&tone_generate_lock);
    if (tone_data != NULL) {
        dbgprintf("tone button: Freeing tone_data\n");
        free(tone_data);
        tone_data = NULL;
    }
    dbgprintf("tone button: malloc %d bytes", tsize);
    dbgprintf(" for tone_data\n");
    if (tsize > 0) {
        tone_data = (cmp_t *) malloc(tsize);
    } else {
        dbgprintf("tsize %d out of range ", tsize);
        dbgprintf("for tone_data %p\n", tone_data);
    }
    if (tone_data == NULL) {
        dbgprintf("Error allocating %d bytes", tsize);
        dbgprintf("for tone signal data storage\n");
    }
    dbgprintf("tone button: tone unlock\n");
    pthread_mutex_unlock(&tone_generate_lock);
    if (tone_data != NULL) {
        dbgprintf("tone button: creating thread\n");
        if (pthread_create(&th, NULL, gensigcaller, NULL) == 0) {
            dbgprintf("tone button: detaching thread\n");
            pthread_detach(th);
        } else {
            dbgprintf("no thread created tone button: ");
            dbgprintf("unlock busy");
            dbgprintf(" pthread failed\n");
            gdk_window_set_cursor(win, NULL);
            pthread_mutex_unlock(&busy_lock);
        }
    } else {
        dbgprintf("no thread created tone button: ");
        dbgprintf("unlock busy pthread failed\n");
        gdk_window_set_cursor(win, NULL);
        pthread_mutex_unlock(&busy_lock);
    }
    dbgprintf("tone button: gsargs unlock\n");
    pthread_mutex_unlock(&gsargs_lock);
}

void *gensigcaller(void *args) {
    GdkWindow *win;
    char *fmt;
    double a;
    double f;
    double p;
    int s;
    int n;

    int (*funcptr)(cmp_t *, double, double, double, int, int);
    int i;


    if (tone_type < 1 || tone_type > 3) {
        dbgprintf("Error unknown tone type %d\n", tone_type);
        return NULL;
    }
    funcptr = sigfunc[tone_type];
    for (i = 0; i <= 3; i++) {
        dbgprintf("%d = %s = %p\n", i, tone_type_str[i], sigfunc[i]);
    }
    dbgprintf("gensig: gsargs lock\n");
    pthread_mutex_lock(&gsargs_lock);
    a = gsargs.amp;
    f = gsargs.freq;
    p = gsargs.phase;
    s = gsargs.sr;
    n = gsargs.n;
    dbgprintf("gsigargs = %p,%f,%f,%f,%i,%i\n", tone_data, a, f, p, s, n);
    dbgprintf("gensig: gsargs unlock\n");
    pthread_mutex_unlock(&gsargs_lock);
    dbgprintf("Current function pointer is set to ");
    dbgprintf("%d = %p\n", tone_type, funcptr);
    dbgprintf("gensig: tone lock\n");
    pthread_mutex_lock(&tone_generate_lock);
    (*funcptr)(tone_data, a, f, p, s, n);
    dbgprintf("gensig: tone unlock\n");
    pthread_mutex_unlock(&tone_generate_lock);
    dbgprintf("gensig: busy lock\n");
    pthread_mutex_lock(&busy_lock);
    dbgprintf("gensig: stoppong busy\n");
    win = gtk_widget_get_window(window);
    gdk_window_set_cursor(win, NULL);
    dbgprintf("gensig: busy unlock\n");
    pthread_mutex_unlock(&busy_lock);
    dbgprintf("gensig: thread exit\n");
    pthread_exit(NULL);
    dbgprintf("gensig: post exit\n");
}

int is_toggled(GtkRadioButton *b) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b))) {
        return 1;
    }
    return 0;
}

int set_toggle(GtkWidget *w, int boolval) {
    GtkToggleButton *tb;
    tb = GTK_TOGGLE_BUTTON(w);
    if (boolval) {
        gtk_toggle_button_set_active(tb, TRUE);
        return 0;
    }
    gtk_toggle_button_set_active(tb, FALSE);
    return 0;
}

void on_wav2data_radio_toggled(GtkRadioButton *b) {
    if (is_toggled(b)) {
        src_is_wave = 1;
    } else {
        src_is_wave = 0;
    }
}

void on_data2wave_radio_toggled(GtkRadioButton *b) {
    if (is_toggled(b)) {
        src_is_wave = 0;
    } else {
        src_is_wave = 1;
    }
}

void on_convert_to_file_button_clicked(GtkButton *b) {
}

void on_convert_from_button_clicked(GtkButton *b) {
}

void on_convert_sample_rate_entry_changed(GtkEntry *e) {
}

void on_from_file_entry_changed(GtkEntry *e) {
}

void on_to_file_entry_changed(GtkEntry *e) {
}

void on_convert_file_button_clicked(GtkEntry *e) {
}

void on_show_variables_button_clicked(GtkButton *b) {
    print_variables();
}

int init_globals(int argc, char **argv) {
    char *glade_start;
    char *glade_end;
    int glade_size;

    glade_end = _binary_audio_xml_glade_end;
    glade_start = _binary_audio_xml_glade_start;
    glade_size = glade_end - glade_start;

    strncpy(tone_save_filename, "untitled.dat", MAXFNSIZE);
    gtk_init(&argc, &argv);
    bld = gtk_builder_new_from_string(glade_start, glade_size);
    window = get_widget("window");
    tone_combo = get_widget("tone_combo");
    g_signal_connect(window, "destroy",
            G_CALLBACK(gtk_main_quit), NULL);

    tone_amp_entry = get_widget("tone_amp_entry");
    tone_freq_entry = get_widget("tone_freq_entry");
    tone_phase_entry = get_widget("tone_phase_entry");
    tone_sr_entry = get_widget("tone_sr_entry");
    tone_secs_entry = get_widget("tone_secs_entry");

    data2wave_radio = get_widget("data2wave_radio");
    wave2data_radio = get_widget("wave2data_radio");
    convert_to_file_button = get_widget("convert_to_file_button");
    convert_from_button = get_widget("convert_from_file_BUTTON");
    convert_sr_entry = get_widget("convert_sample_rate_entry");
    from_file_entry = get_widget("from_file_entry");
    to_file_entry = get_widget("to_file_entry");
    convert_file_button = get_widget("convert_file_button");
    show_variables_button = get_widget("show_variales_button");


    gtk_combo_box_set_active(GTK_COMBO_BOX(tone_combo), 0);
    if (pthread_mutex_init(&tone_generate_lock, NULL) != 0) {
        dbgprintf("Failed to allocate pthread mutex lock for");
        dbgprintf(" tone generator\n");
        return -1;
    }
    if (pthread_mutex_init(&gsargs_lock, NULL) != 0) {
        dbgprintf("Failed to allocate pthread mutex lock for");
        dbgprintf(" tone generator\n");
        return -1;
    }
    if (pthread_mutex_init(&busy_lock, NULL) != 0) {
        dbgprintf("Failed to allocate pthread mutex lock for");
        dbgprintf(" busy icon\n");
    }

    strncpy(tone_save_filename, "", MAXFNSIZE);
    strncpy(convert_from_filename, "", MAXFNSIZE);
    strncpy(convert_to_filename, "", MAXFNSIZE);
    tone_type = 1;
    tone_amp = 1.0;
    tone_freq = 440.0;
    tone_phase = 0.0;
    tone_sr = 44100;
    tone_secs = 10.0;
    tone_data = NULL;
    dis = gtk_widget_get_display(window);
    busy = gdk_cursor_new_for_display(dis, GDK_WATCH);
    src_is_wave = 1;
    set_toggle(data2wave_radio, 0);
    set_toggle(wave2data_radio, 1);
    convert_sr = 44100;
    update_entrys();
    return 0;
}

int main(int argc, char **argv) {
    if (init_globals(argc, argv) < 0) {
        return 0;
    };
    gtk_builder_connect_signals(bld, NULL);
    gtk_widget_show(window);
    gtk_main();
}
