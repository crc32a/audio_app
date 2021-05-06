#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<math.h>
#include<ctype.h>
#include<dft.h>

#define MAXFNSIZE 1023
#define STRSIZE 255

char tone_save_file_name[MAXFNSIZE];
char tstr[STRSIZE];
GtkWidget *window;
GtkBuilder *bld;
GtkWidget *tone_file_save_dialog;
GtkWidget *tone_combo;
GtkWidget *tone_amp_entry;
GtkWidget *tone_freq_entry;
GtkWidget *tone_phase_entry;
GtkWidget *tone_sr_entry;
GtkWidget *tone_secs_entry;

char *tone_type_str[]={"", "Sine", "Square", "Sawtooth"};
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

int set_entry_text(GtkWidget *w, char *fmt, void *text){
    GtkEntry *e = GTK_ENTRY(w);
    snprintf(tstr,STRSIZE,fmt,text);
    gtk_entry_set_text(e,(char *)tstr);
    return 0;
}

int update_tone_entrys(){
    set_entry_text(tone_amp_entry,"%f", tone_amp_entry);
    set_entry_text(tone_freq_entry,"%f", tone_freq_entry);
    set_entry_text(tone_phase_entry,"%f", tone_phase_entry);
    set_entry_text(tone_sr_entry,"%d", tone_sr_entry);
    set_entry_text(tone_secs_entry,"%f", tone_secs_entry);
}


void on_tone_save_button_clicked(GtkButton *b){
    gint res;
    char *file_name;
    GtkDialog *dialog;
    GtkFileChooser *chooser;

    printf("tone save button clicked\n");
    dialog = GTK_DIALOG(tone_file_save_dialog);
    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_action(chooser,
        GTK_FILE_CHOOSER_ACTION_SAVE);
    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
    gtk_file_chooser_set_current_name(chooser, tone_save_file_name);
    gtk_dialog_add_button(dialog, "Save", GTK_RESPONSE_ACCEPT);
    gtk_dialog_add_button(dialog, "Cancel", GTK_RESPONSE_CANCEL);
    res = gtk_dialog_run(dialog);
    if(res == GTK_RESPONSE_ACCEPT){
        file_name = gtk_file_chooser_get_filename(chooser);
        strncpy(tone_save_file_name, file_name, MAXFNSIZE);
        g_free(file_name);

    }else{
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

char *get_entry_text(char *buff, GtkWidget *entry){
    strncpy(buff, (char *)gtk_entry_get_text(GTK_ENTRY(entry)),
        STRSIZE);
    return buff;
}

void on_generate_tone_button_clicked(GtkButton *b){
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
    printf("amplitude:   %f\n", tone_amp);
    printf("frequency:   %f\n", tone_freq);
    printf("phase:       %f\n", tone_phase);
    printf("sample rate: %d\n", tone_sr);
    printf("seconds:     %f\n", tone_secs);
    update_tone_entrys();
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
    tone_file_save_dialog = get_widget("tone_file_save_dialog");
    g_signal_connect(window,"destroy",
        G_CALLBACK(gtk_main_quit),NULL);

    tone_amp_entry = get_widget("tone_amp_entry");
    tone_freq_entry = get_widget("tone_freq_entry");
    tone_phase_entry = get_widget("tone_phase_entry");
    tone_sr_entry = get_widget("tone_sr_entry");
    tone_secs_entry = get_widget("tone_secs_entry");

    tone_amp = 1.0;
    tone_freq = 440.0;
    tone_phase = 0;
    tone_sr = 440;
    tone_secs = 10.0;

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
