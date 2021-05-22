#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<pthread.h>
#include<inttypes.h>
#include<stdarg.h>
#include<ctype.h>
#include<glib.h>
#include<math.h>
#include"dft.h"
#include"audio_app.h"

#define STRSIZE 1023


char *tone_save_filename;
char *convert_from_filename;
char *convert_to_filename;
char *tstr;

GtkBuilder *bld;
GtkWidget *window;
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
GtkWidget *from_file_label;
GtkWidget *to_file_label;

GdkCursor *busy;
GdkDisplay *dis;

#define DEBUG 1

char *tone_type_str[] = {"", "Sine", "Square", "Saw Tooth"};

int (*sigfunc[])(cmp_t *, double, double, double, int, int64_t) = \
    {NULL, sigsine, sigsquare, sigsaw};

pthread_mutex_t tone_generate_lock;
pthread_mutex_t args_lock;
pthread_mutex_t busy_lock;

cmp_t *tone_data;
thread_args targs;
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

int error_dialog(char *fmt, ...) {
    GtkWidget *dialog;
    char *msg;
    va_list ap;

    msg = (char *) malloc(STRSIZE + 1);
    if (msg == NULL) {
        return -1;
    }

    va_start(ap, fmt);
    vsnprintf(msg, STRSIZE, fmt, ap);
    va_end(ap);

    dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            msg
            );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    free(msg);
    return 0;
}

int set_entry_text(GtkWidget *w, char *fmt, ...) {
    char *msg;
    GtkEntry *e;
    va_list ap;

    msg = (char *) malloc(STRSIZE + 1);
    e = GTK_ENTRY(w);
    va_start(ap, fmt);
    vsnprintf(msg, STRSIZE, fmt, ap);
    va_end(ap);
    dbgprintf("setting text to %s\n", msg);
    gtk_entry_set_text(e, (const gchar *) msg);
    free(msg);
    return 0;
}

int update_entrys() {
    set_entry_text(tone_amp_entry, "%f", tone_amp);
    set_entry_text(tone_freq_entry, "%f", tone_freq);
    set_entry_text(tone_phase_entry, "%f", tone_phase);
    set_entry_text(tone_sr_entry, "%d", tone_sr);
    set_entry_text(tone_secs_entry, "%f", tone_secs);
}

int update_convert_entrys() {
    set_entry_text(from_file_entry, "%s", convert_from_filename);
    set_entry_text(to_file_entry, "%s", convert_to_filename);
    set_entry_text(convert_sr_entry, "%d", convert_sr);
    return 0;
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
        strncpy(tone_save_filename, file_name, STRSIZE);
        g_free(file_name);
        dbgprintf("SAVEING %s\n", tone_save_filename);
        dbgprintf("save tone file: busy lock()\n");
        pthread_mutex_lock(&busy_lock);
        gdk_window_set_cursor(win, busy);
        dbgprintf("save tone file: busy unlock()\n");
        pthread_mutex_unlock(&busy_lock);
        dbgprintf("save tone file: gsargs lock()\n");
        pthread_mutex_lock(&args_lock);
        targs.n = (int) (tone_sr * tone_secs + 1);
        targs. tone_file = tone_save_filename;
        dbgprintf("save tone file: gsargs unlock()\n");
        pthread_mutex_unlock(&args_lock);
        if (pthread_create(&th, NULL, savetonecaller, NULL) == 0) {
            dbgprintf("Save tone file thread creqted\n");
            pthread_detach(th);
        }
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void *savetonecaller(void *args) {
    char *file_name;
    int64_t n;

    dbgprintf("savetonecaller: lock gsargs\n");
    pthread_mutex_lock(&args_lock);
    n = targs.n;
    file_name = targs.tone_file;
    dbgprintf("savetonecaller: unlock gsargs\n");
    pthread_mutex_unlock(&args_lock);
    dbgprintf("savetonecaller: lock tone_generate\n");
    pthread_mutex_lock(&tone_generate_lock);
    dbgprintf("savetonecaller: saving file\n");
    if (tone_data != NULL) {
        dft_fwrite(file_name, tone_data, n);
    }
    dbgprintf("savetonecaller: unlock tone_generate\n");
    pthread_mutex_unlock(&tone_generate_lock);
    stop_spinlock("savetonecaller");
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
        case DTYPE_I64:
            *(int64_t *) val = atol(str);
            break;
        case DTYPE_DOUBLE:
            *(double *) val = atof(str);
            break;
        case DTYPE_FLOAT:
            *(float *) val = atof(str);
            break;
        case DTYPE_STR:
            strcpy((char *) val, "");
            if (str != NULL) {
                strncpy((char *) val, str, STRSIZE);
            }
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

int start_spinlock(char *calling_func_name) {
    GdkWindow *win;

    win = gtk_widget_get_window(window);
    dbgprintf("%s: Busy lock()\n", calling_func_name);
    pthread_mutex_lock(&busy_lock);
    dbgprintf("%s: starting busy\n", calling_func_name);
    gdk_window_set_cursor(win, busy);
    dbgprintf("%s: busy unlock\n", calling_func_name);
    pthread_mutex_unlock(&busy_lock);
    return 0;
}

int stop_spinlock(char *calling_func_name) {
    GdkWindow *win;

    win = gtk_widget_get_window(window);
    dbgprintf("%s: trying busylock (to stop it)\n", calling_func_name);
    pthread_mutex_trylock(&busy_lock);
    dbgprintf("%s: clearing spinner\n", calling_func_name);
    gdk_window_set_cursor(win, NULL);
    dbgprintf("%s: unlocking busylock\n", calling_func_name);
    pthread_mutex_unlock(&busy_lock);
    return 0;
}

void on_generate_tone_button_clicked(GtkButton *b) {
    pthread_t th;
    ssize_t tsize;

    print_variables();
    start_spinlock("tone_button");
    pthread_mutex_lock(&args_lock);
    targs.amp = tone_amp;
    targs.freq = tone_freq;
    targs.phase = tone_phase;
    targs.sr = tone_sr;
    targs.n = (int64_t) (tone_sr * tone_secs + 1);
    tsize = sizeof (cmp_t) * targs.n;
    dbgprintf("n = %d tsize = %d\n", targs.n, tsize);
    dbgprintf("tone button: tone lock\n");
    pthread_mutex_lock(&tone_generate_lock);
    if (tone_data != NULL) {
        dbgprintf("tone button: Freeing tone_data\n");
        free(tone_data);
        tone_data = NULL;
    }
    dbgprintf("tone button: malloc %zi bytes", tsize);
    dbgprintf(" for tone_data\n");
    if (tsize > 0) {
        tone_data = (cmp_t *) malloc(tsize);
    } else {
        dbgprintf("tsize %zi out of range ", tsize);
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
            stop_spinlock("tone_button");
        }
    } else {
        dbgprintf("no thread created tone button: ");
        dbgprintf("unlock busy pthread failed\n");
        stop_spinlock("tone_button");
    }
    dbgprintf("tone button: gsargs unlock\n");
    pthread_mutex_unlock(&args_lock);
}

void *gensigcaller(void *args) {
    GdkWindow *win;
    char *fmt;
    double a;
    double f;
    double p;
    int s;
    int64_t n;
    int i;

    int (*funcptr)(cmp_t *, double, double, double, int, int64_t);


    if (tone_type < 1 || tone_type > 3) {
        dbgprintf("Error unknown tone type %d\n", tone_type);
        return NULL;
    }
    funcptr = sigfunc[tone_type];
    for (i = 0; i <= 3; i++) {
        dbgprintf("%d = %s = %p\n", i, tone_type_str[i], sigfunc[i]);
    }
    dbgprintf("gensig: gsargs lock\n");
    pthread_mutex_lock(&args_lock);
    a = targs.amp;
    f = targs.freq;
    p = targs.phase;
    s = targs.sr;
    n = targs.n;
    dbgprintf("gsigargs = %p,%f,%f,%f,%i,%i\n", tone_data, a, f, p, s, n);
    dbgprintf("gensig: gsargs unlock\n");
    pthread_mutex_unlock(&args_lock);
    dbgprintf("Current function pointer is set to ");
    dbgprintf("%d = %p\n", tone_type, funcptr);
    dbgprintf("gensig: tone lock\n");
    pthread_mutex_lock(&tone_generate_lock);
    (*funcptr)(tone_data, a, f, p, s, n);
    dbgprintf("gensig: tone unlock\n");
    pthread_mutex_unlock(&tone_generate_lock);
    stop_spinlock("gensig");
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
    update_convert_labels();
}

void on_data2wave_radio_toggled(GtkRadioButton *b) {
    if (is_toggled(b)) {
        src_is_wave = 0;
    } else {
        src_is_wave = 1;
    }
    update_convert_labels();
}

int update_convert_labels() {
    if (src_is_wave) {
        gtk_label_set_text(GTK_LABEL(from_file_label), "(Wav)");
        gtk_label_set_text(GTK_LABEL(to_file_label), "(Data)");
    } else {
        gtk_label_set_text(GTK_LABEL(from_file_label), "(Data)");
        gtk_label_set_text(GTK_LABEL(to_file_label), "(Wav)");
    }
    return 0;
}

int get_filename_dialog(char *fname, char *dialog_title) {
    gint res;
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    char *chosen_filename;
    dialog = gtk_file_chooser_dialog_new(
            dialog_title,
            GTK_WINDOW(window),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "Cancel",
            GTK_RESPONSE_CANCEL,
            "Select",
            GTK_RESPONSE_ACCEPT,
            NULL);
    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_name(chooser, fname);
    gtk_file_chooser_set_filename(chooser, fname);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        chosen_filename = gtk_file_chooser_get_filename(chooser);
        strncpy(fname, chosen_filename, STRSIZE);
        g_free(chosen_filename);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return 0;
}

void on_convert_to_file_button_clicked(GtkButton *b) {
    get_filename_dialog(convert_to_filename, "Destination File");
    update_convert_entrys();
}

void on_convert_from_file_button_clicked(GtkButton *b) {
    get_filename_dialog(convert_from_filename, "Source File");
    update_convert_entrys();
}

void on_convert_sample_rate_entry_changed(GtkEntry *e) {
    entry_to_variable(e, &convert_sr, DTYPE_INT);
}

void on_from_file_entry_changed(GtkEntry *e) {
    entry_to_variable(e, convert_from_filename, DTYPE_STR);
}

void on_to_file_entry_changed(GtkEntry *e) {
    entry_to_variable(e, convert_to_filename, DTYPE_STR);
}

void on_convert_file_button_clicked(GtkEntry *e) {
    dbgprintf("convert_file_button pressed\n");
    if (src_is_wave) {
        dbgprintf("src_is_wave\n");
        pthread_mutex_lock(&args_lock);

        pthread_mutex_unlock(&args_lock);
    } else {

    }
}

void on_show_variables_button_clicked(GtkButton *b) {
    print_variables();
}

int init_globals(int argc, char **argv) {
    char *glade_start;
    char *glade_end;
    int64_t glade_size;
    size_t tsize;

    glade_end = _binary_audio_xml_glade_end;
    glade_start = _binary_audio_xml_glade_start;
    glade_size = glade_end - glade_start;
    tsize = STRSIZE + 1;
    tone_save_filename = (char *) malloc(tsize);
    if (tone_save_filename == NULL) {
        printf("Failed to allocate %zi bytes for ", tsize);
        printf("tone save_filename\n");
    }
    convert_from_filename = (char *) malloc(tsize);
    if (convert_from_filename == NULL) {
        printf("Failed to allocate %zi bytes for ", tsize);
        printf("convert_from_filename\n");
    }
    convert_to_filename = (char *) malloc(tsize);
    if (convert_to_filename == NULL) {
        printf("Failed to allocate %zi bytes for ", tsize);
        printf("convert_to_filename\n");
    }
    tstr = (char *) malloc(tsize);
    if (tstr == NULL) {
        printf("Failed to allocate %zi bytes for ", tsize);
        printf("tstr\n");
    }

    strncpy(tone_save_filename, "untitled.dat", STRSIZE);
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
    from_file_label = get_widget("from_file_label");
    to_file_label = get_widget("to_file_label");


    gtk_combo_box_set_active(GTK_COMBO_BOX(tone_combo), 0);
    if (pthread_mutex_init(&tone_generate_lock, NULL) != 0) {
        printf("Failed to allocate pthread mutex lock for");
        printf(" tone generator\n");
        return -1;
    }
    if (pthread_mutex_init(&args_lock, NULL) != 0) {
        printf("Failed to allocate pthread mutex lock for");
        printf(" tone generator\n");
        return -1;
    }
    if (pthread_mutex_init(&busy_lock, NULL) != 0) {
        printf("Failed to allocate pthread mutex lock for");
        printf(" busy icon\n");
    }

    strncpy(tone_save_filename, "", STRSIZE);
    strncpy(convert_from_filename, "", STRSIZE);
    strncpy(convert_to_filename, "", STRSIZE);
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
    update_convert_labels();
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
