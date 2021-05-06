#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>
#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<math.h>
#include<ctype.h>
#include<dft.h>

GtkWidget *window;
GtkBuilder *bld;

GtkWidget *tone_combo;

extern char _binary_audio_xml_glade_end[];
extern char _binary_audio_xml_glade_size[];
extern char _binary_audio_xml_glade_start[];

int init_globals(int argc, char **argv){
    char *glade_start;
    char *glade_end;
    int glade_size;

    glade_end = _binary_audio_xml_glade_end;
    glade_start = _binary_audio_xml_glade_start;
    glade_size = glade_end - glade_start;

    gtk_init(&argc, &argv);
    bld=gtk_builder_new_from_string(glade_start, glade_size);
    window=GTK_WIDGET(gtk_builder_get_object(bld,"window"));
    tone_combo=GTK_WIDGET(gtk_builder_get_object(bld,"tone_combo"));
    g_signal_connect(window,"destroy",
        G_CALLBACK(gtk_main_quit),NULL);
    return 0;
}

int main(int argc, char **argv){
    init_globals(argc, argv);
    gtk_builder_connect_signals(bld, NULL);
    gtk_widget_show(window);
    gtk_main();  
}
