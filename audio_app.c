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

char tone_save_file_name[MAXFNSIZE];
char tstr[STRSIZE];
GtkWidget *window;
GtkBuilder *bld;
GtkWidget *tone_combo;
GtkWidget *tone_amp_entry;
GtkWidget *tone_freq_entry;
GtkWidget *tone_phase_entry;
GtkWidget *tone_sr_entry;
GtkWidget *tone_secs_entry;
GdkCursor *busy;
GdkDisplay *dis;

char *tone_type_str[]={"", "Sine", "Square", "Saw Tooth"};

int (*sigfunc[])(cmp_t *,double,double,double,int,int) =
        { NULL, sigsine, sigsquare, sigsaw };

pthread_mutex_t tone_generate_lock;
pthread_mutex_t gsargs_lock;
pthread_mutex_t busy_lock;
cmp_t *tone_data;
gensig_args gsargs;
int tone_type;
double tone_amp;
double tone_freq;
double tone_phase;
int tone_sr;
double tone_secs;

extern char _binary_audio_xml_glade_end[];
extern char _binary_audio_xml_glade_size[];
extern char _binary_audio_xml_glade_start[];

GtkWidget *get_widget(char *name){
    return GTK_WIDGET(gtk_builder_get_object(bld, name));
}

int set_entry_text(GtkWidget *w, char *fmt, ...){
    GtkEntry *e;
    va_list ap;

    e = GTK_ENTRY(w);
    va_start(ap, fmt);
    if(strcmp(fmt, "%f")==0){
        snprintf(tstr,STRSIZE,"%f", va_arg(ap, double));
    } else if(strcmp(fmt,"%d")==0){
        snprintf(tstr,STRSIZE,"%d", va_arg(ap, int));
    } else {
        strcpy(tstr, "");
    }
    va_end(ap);
    printf("setting text to %s\n", tstr);
    gtk_entry_set_text(e,(const gchar *)tstr);
    return 0;
}

int update_tone_entrys(){
    set_entry_text(tone_amp_entry,"%f", tone_amp);
    set_entry_text(tone_freq_entry,"%f", tone_freq);
    set_entry_text(tone_phase_entry,"%f", tone_phase);
    set_entry_text(tone_sr_entry,"%d", tone_sr);
    set_entry_text(tone_secs_entry,"%f", tone_secs);
}


void on_tone_save_button_clicked(GtkButton *b){
    gint res;
    char *file_name;
    GtkWidget *dialog;
    GtkFileChooser *chooser;

    printf("tone save button clicked\n");
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
    gtk_file_chooser_set_current_name(chooser, tone_save_file_name);
    gtk_file_chooser_set_filename(chooser, tone_save_file_name);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if(res == GTK_RESPONSE_ACCEPT){
        file_name = gtk_file_chooser_get_filename(chooser);
        strncpy(tone_save_file_name, file_name, MAXFNSIZE);
        g_free(file_name);
        printf("SAVEING %s\n", tone_save_file_name);
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void on_tone_amp_entry_changed(GtkEntry *e){
    char *str;
    double val;

    str = (char *)gtk_entry_get_text(e);
    val = atof(str);
    tone_amp = val;
    update_tone_entrys();   
}

void on_tone_freq_entry_changed(GtkEntry *e){
    char *str;
    double val;

    str = (char *)gtk_entry_get_text(e);
    val = atof(str);
    tone_freq = val;
    update_tone_entrys();   
}

void on_tone_phase_entry_changed(GtkEntry *e){
    char *str;
    double val;

    str = (char *)gtk_entry_get_text(e);
    val = atof(str);
    tone_phase = val;
    update_tone_entrys();   
}

void on_tone_sr_entry_changed(GtkEntry *e){
    char *str;
    int val;

    str = (char *)gtk_entry_get_text(e);
    val = atoi(str);
    if(val >= 0){
        tone_sr = val;
    }
    update_tone_entrys();   
}

void on_tone_secs_entry_changed(GtkEntry *e){
    char *str;
    double val;

    str = (char *)gtk_entry_get_text(e);
    val = atof(str);
    tone_secs = val;
    update_tone_entrys();   
}

void on_tone_combo_changed(GtkComboBoxText *c){
    char *box_text;
    int i;

    box_text = gtk_combo_box_text_get_active_text(c);
    tone_type = 0;
    printf("tone_type box is %s\n", (char *)box_text);
    if(box_text != NULL){
        for(i=0;i<=3;i++){
            if(strcmp(tone_type_str[i], box_text)==0){
                tone_type = i;
            }
        }
        g_free(box_text);
    }
}

char *get_entry_text(char *buff, GtkWidget *entry){
    strncpy(buff, (char *)gtk_entry_get_text(GTK_ENTRY(entry)),
        STRSIZE);
    return buff;
}

int print_tone(){
    printf("Tone Entries\n");
    printf("---------------------------------------------\n");
    printf("amplitutde: %s\n", get_entry_text(tstr,tone_amp_entry));
    printf("frequency: %s\n", get_entry_text(tstr,tone_freq_entry));
    printf("phase: %s\n", get_entry_text(tstr,tone_phase_entry));
    printf("sample rate: %s\n", get_entry_text(tstr,tone_sr_entry));
    printf("seconds: %s\n", get_entry_text(tstr,tone_secs_entry));
    printf("\n");
    printf("Tone values\n");
    printf("--------------------------------------------\n");
    printf("tone type:   %s\n", tone_type_str[tone_type]);
    printf("amplitude:   %f\n", tone_amp);
    printf("frequency:   %f\n", tone_freq);
    printf("phase:       %f\n", tone_phase);
    printf("sample rate: %d\n", tone_sr);
    printf("seconds:     %f\n", tone_secs);
    return 0;
}

void on_generate_tone_button_clicked(GtkButton *b){    
    pthread_t th;
    size_t tsize;
    GdkWindow *win;

    win = gtk_widget_get_window(window);
    printf("tone button: Busy lock()\n");
    pthread_mutex_lock(&busy_lock);
        printf("tone button: starting busy\n");
        gdk_window_set_cursor(win, busy);
    pthread_mutex_unlock(&busy_lock);
    printf("tone_button: sbusy unlock\n");
    pthread_mutex_lock(&gsargs_lock);
        gsargs.amp = tone_amp;
        gsargs.freq = tone_freq;
        gsargs.phase = tone_phase;
        gsargs.sr = tone_phase;
        gsargs.n = (double)tone_sr * tone_secs + 1;
        printf("tone button: gsargs lock\n");
        pthread_mutex_lock(&tone_generate_lock);
            if(tone_data !=NULL){
                printf("tone button: Freeing tone_data\n");
                tone_data = NULL;
                free(tone_data);
            }
            tsize = sizeof(cmp_t) * gsargs.n;
            printf("tone button: malloc tone_data\n");
            tone_data = (cmp_t*)malloc(tsize);
            if(tone_data == NULL){
                printf("Error allocating %d bytes", tsize);
                printf("for tone signal data storage\n");
            }
        printf("tone button: tone unlock\n");
        pthread_mutex_unlock(&tone_generate_lock);
        if(tone_data != NULL){
            printf("tone button: creating thread\n");
            if(pthread_create(&th, NULL, gensigcaller, NULL) == 0){
                printf("detaching thread\n");
                pthread_detach(th);
            }else{
                printf("tone button: unlock busy");
                printf(" pthread failed\n");
                gdk_window_set_cursor(win, NULL);
                pthread_mutex_unlock(&busy_lock);
            }
        }
    printf("tone button: gsargs unlock\n");
    pthread_mutex_unlock(&gsargs_lock);
}

void *gensigcaller(void *args){
    GdkWindow *win;
    char *fmt;
    double a;
    double f;
    double p;
    int s;
    int n;

    int (*funcptr)(cmp_t *,double,double,double,int,int);
    int i;


    if(tone_type < 1 || tone_type > 3){
        printf("Error unknown tone type %d\n", tone_type);
        return NULL;
    }
    funcptr = sigfunc[tone_type];
    for(i=0;i<=3;i++){
        printf("%d = %s = %p\n", i, tone_type_str[i], sigfunc[i]);
    }
    printf("gensig: gsargs lock\n");
    pthread_mutex_lock(&gsargs_lock);
        a = gsargs.amp;
        f = gsargs.freq;
        p = gsargs.phase;
        s = gsargs.sr;
        n = gsargs.n;
    printf("gensig: gsargs unlock\n");
    pthread_mutex_unlock(&gsargs_lock);
    printf("Current function pointer is set to ");
    printf("%d = %p\n", tone_type,funcptr);
    printf("gensig: tone lock\n");
    pthread_mutex_lock(&tone_generate_lock);
    sleep(3);
    printf("gensig: tone unlock\n");
    pthread_mutex_unlock(&tone_generate_lock);
    printf("gensig: busy lock\n");
    pthread_mutex_lock(&busy_lock);
        printf("gensig: stoppong busy\n");
        win = gtk_widget_get_window(window);
        gdk_window_set_cursor(win, NULL);
    printf("gensig: busy unlock\n");
    pthread_mutex_unlock(&busy_lock);
    printf("gensig: thread exit\n");
    pthread_exit(NULL);
    printf("gensig: post exit\n");
}

int init_globals(int argc, char **argv){
    char *glade_start;
    char *glade_end;
    int glade_size;

    glade_end = _binary_audio_xml_glade_end;
    glade_start = _binary_audio_xml_glade_start;
    glade_size = glade_end - glade_start;

    strncpy(tone_save_file_name,"untitled.dat", MAXFNSIZE);    
    gtk_init(&argc, &argv);
    bld=gtk_builder_new_from_string(glade_start, glade_size);
    window=get_widget("window");
    tone_combo=get_widget("tone_combo");
    g_signal_connect(window,"destroy",
        G_CALLBACK(gtk_main_quit),NULL);

    tone_amp_entry = get_widget("tone_amp_entry");
    tone_freq_entry = get_widget("tone_freq_entry");
    tone_phase_entry = get_widget("tone_phase_entry");
    tone_sr_entry = get_widget("tone_sr_entry");
    tone_secs_entry = get_widget("tone_secs_entry");
    gtk_combo_box_set_active(GTK_COMBO_BOX(tone_combo),0);
    if(pthread_mutex_init(&tone_generate_lock,NULL) != 0){
        printf("Failed to allocate pthread mutex lock for");
        printf(" tone generator\n");
        return -1;
    }
    if(pthread_mutex_init(&gsargs_lock,NULL) != 0){
        printf("Failed to allocate pthread mutex lock for");
        printf(" tone generator\n");
        return -1;
    }
    if(pthread_mutex_init(&busy_lock, NULL) != 0){
        printf("Failed to allocate pthread mutex lock for");
        printf(" busy icon\n");
    }
    tone_type = 1;
    tone_amp = 1.0;
    tone_freq = 440.0;
    tone_phase = 0;
    tone_sr = 440;
    tone_secs = 10.0;
    tone_data = NULL;
    dis = gtk_widget_get_display(window);
    busy = gdk_cursor_new_for_display(dis,GDK_WATCH);
    update_tone_entrys();   
    return 0;
}

int main(int argc, char **argv){
    if(init_globals(argc, argv) < 0){
        return 0;
    };
    gtk_builder_connect_signals(bld, NULL);
    gtk_widget_show(window);
    gtk_main();  
}
