#ifndef GBUFF_LIB_H
#define GBUFF_LIB_H

#include <gtk/gtk.h>

// Global variables
extern GtkWidget *text_view, *line_numbers, *hpaned;
extern GtkTextTag *bold_tag, *italic_tag;
extern GtkTextBuffer *buffer;
extern gboolean show_line_numbers;

// Function declarations
void add_file_filters(GtkFileChooser *chooser);
void update_line_numbers(void);
void toggle_line_numbers(void);
void on_text_changed(void);
void open_file(void);
void save_file(void);
void apply_bold(void);
void apply_italic(void);
gboolean on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
void setup_text_view(GtkWidget *window);

#endif
