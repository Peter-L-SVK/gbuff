#include "lib.h"
#include <gtk/gtk.h>
#include <string.h>
#include <glib/gprintf.h>

// Global variables
GtkWidget *text_view, *line_numbers, *hpaned;
GtkTextTag *bold_tag, *italic_tag;
GtkTextBuffer *buffer;
gboolean show_line_numbers = FALSE;

// File filters for open/save dialogs
void add_file_filters(GtkFileChooser *chooser) {
    GtkFileFilter *filter_text = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_text, "Text files");
    gtk_file_filter_add_pattern(filter_text, "*.txt");
    gtk_file_chooser_add_filter(chooser, filter_text);

    GtkFileFilter *filter_any = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_any, "All files");
    gtk_file_filter_add_pattern(filter_any, "*");
    gtk_file_chooser_add_filter(chooser, filter_any);
}

// Function to update line numbers
void update_line_numbers() {
  if (!show_line_numbers) {
    gtk_widget_hide(line_numbers);
    gtk_paned_set_position(GTK_PANED(hpaned), 0);
  } else {
    gtk_widget_show(line_numbers);
    GtkTextIter iter;
    int count = 1;
    int max_line = gtk_text_buffer_get_line_count(buffer);
    int digits = 1;
    
    // Calculate needed width with more padding
    for (int n = max_line; n > 0; n /= 10) digits++;
    int width = digits * 10 + 20;
    gtk_widget_set_size_request(line_numbers, width, -1);
    gtk_paned_set_position(GTK_PANED(hpaned), width);
    
    GString *text = g_string_new(NULL);
    
    gtk_text_buffer_get_start_iter(buffer, &iter);
    while (!gtk_text_iter_is_end(&iter)) {
      if (gtk_text_iter_get_line(&iter) == count-1) {
	g_string_append_printf(text, "%d\n", count);
	count++;
      }
      gtk_text_iter_forward_char(&iter);
    }
    
    if (count > 1 || gtk_text_buffer_get_line_count(buffer) > 0) {
      g_string_append_printf(text, "%d", count);
    }
    
    GtkTextBuffer *line_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(line_numbers));
    gtk_text_buffer_set_text(line_buffer, text->str, -1);
    
    // Apply right alignment with padding
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(line_buffer, &start);
    gtk_text_buffer_get_end_iter(line_buffer, &end);
    
    // First remove any existing tags to prevent stacking
    gtk_text_buffer_remove_all_tags(line_buffer, &start, &end);
    
    // Then apply the right-align tag
    gtk_text_buffer_apply_tag_by_name(line_buffer, "right-align", &start, &end);
    
    g_string_free(text, TRUE);
  }
    
  GtkWidget *scrolled_window = gtk_widget_get_parent(text_view);
  gtk_widget_queue_resize(scrolled_window);
}

// Toggle line numbers
void toggle_line_numbers() {
    show_line_numbers = !show_line_numbers;
    update_line_numbers();
}

// Connect this to buffer changes
void on_text_changed() {
    update_line_numbers();
}

// Open file function
void open_file() {
    GtkWidget *dialog;
    GtkWidget *window = gtk_widget_get_toplevel(text_view);
    
    dialog = gtk_file_chooser_dialog_new("Open File", GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL);
    
    add_file_filters(GTK_FILE_CHOOSER(dialog));
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        gchar *contents;
        gsize length;
        GError *error = NULL;
        
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (g_file_get_contents(filename, &contents, &length, &error)) {
            gtk_text_buffer_set_text(buffer, contents, length);
            g_free(contents);
            
            // Simple syntax highlighting for demonstration
            GtkTextIter iter;
            gtk_text_buffer_get_start_iter(buffer, &iter);
            
            // Highlight numbers (very basic example)
            while (!gtk_text_iter_is_end(&iter)) {
                if (g_unichar_isdigit(gtk_text_iter_get_char(&iter))) {
                    GtkTextIter start_match = iter;
                    gtk_text_iter_forward_char(&iter);
                    while (!gtk_text_iter_is_end(&iter) && 
                           g_unichar_isdigit(gtk_text_iter_get_char(&iter))) {
                        gtk_text_iter_forward_char(&iter);
                    }
                    gtk_text_buffer_apply_tag_by_name(buffer, "blue_foreground", 
                                                     &start_match, &iter);
                } else {
                    gtk_text_iter_forward_char(&iter);
                }
            }
            
            // Update line numbers after loading file
            update_line_numbers();
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to open file: %s", error->message);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            g_error_free(error);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Save file function
void save_file() {
    GtkWidget *dialog;
    GtkTextIter start, end;
    GtkWidget *window = gtk_widget_get_toplevel(text_view);
    
    dialog = gtk_file_chooser_dialog_new("Save File", GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL);
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    add_file_filters(GTK_FILE_CHOOSER(dialog));
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        gchar *text;
        
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        
        if (!g_file_set_contents(filename, text, -1, NULL)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to save file");
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(text);
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Apply bold formatting
void apply_bold() {
    GtkTextIter start, end;
    
    if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
        gtk_text_buffer_apply_tag(buffer, bold_tag, &start, &end);
    }
}

// Apply italic formatting
void apply_italic() {
    GtkTextIter start, end;
    
    if (gtk_text_buffer_get_selection_bounds(buffer, &start, &end)) {
        gtk_text_buffer_apply_tag(buffer, italic_tag, &start, &end);
    }
}

gboolean on_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    GtkAdjustment *adj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(widget));
    double value = gtk_adjustment_get_value(adj);
    double step = gtk_adjustment_get_step_increment(adj);
    double page = gtk_adjustment_get_page_increment(adj);
    
    if (event->direction == GDK_SCROLL_UP || 
        (event->direction == GDK_SCROLL_SMOOTH && event->delta_y < 0)) {
        gtk_adjustment_set_value(adj, MAX(value - step, gtk_adjustment_get_lower(adj)));
        return TRUE;
    }
    else if (event->direction == GDK_SCROLL_DOWN || 
             (event->direction == GDK_SCROLL_SMOOTH && event->delta_y > 0)) {
        gtk_adjustment_set_value(adj, MIN(value + page, 
                               gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj)));
        return TRUE;
    }
    return FALSE;
}

void setup_text_view(GtkWidget *window) {
    // Create paned container
    hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    
    // Create line numbers display
    line_numbers = gtk_text_view_new();
    GtkCssProvider *line_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(line_provider, 
        "textview { font-family: Monospace; font-size: 12pt; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(line_numbers),
        GTK_STYLE_PROVIDER(line_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_text_view_set_editable(GTK_TEXT_VIEW(line_numbers), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(line_numbers), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(line_numbers), GTK_WRAP_NONE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(line_numbers), 6);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(line_numbers), 6);
    gtk_widget_set_size_request(line_numbers, 40, -1);
    
    // Create scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    // Create main text view
    text_view = gtk_text_view_new();
    GtkCssProvider *text_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(text_provider, 
        "textview { font-family: Monospace; font-size: 11pt; }", -1, NULL);
    gtk_style_context_add_provider(gtk_widget_get_style_context(text_view),
        GTK_STYLE_PROVIDER(text_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    
    // Connect scroll event
    g_signal_connect(text_view, "scroll-event", 
                   G_CALLBACK(on_scroll_event), NULL);
    
    // Add widgets to paned container
    gtk_paned_pack1(GTK_PANED(hpaned), line_numbers, FALSE, FALSE);
    gtk_paned_pack2(GTK_PANED(hpaned), scrolled_window, TRUE, TRUE);
    
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Focus and event handling
    gtk_widget_set_can_focus(text_view, TRUE);
    gtk_widget_grab_focus(text_view);
    gtk_widget_set_events(line_numbers, gtk_widget_get_events(line_numbers) & ~GDK_SCROLL_MASK);
    
    // Create text tags
    bold_tag = gtk_text_buffer_create_tag(buffer, "bold",
                                        "weight", PANGO_WEIGHT_BOLD, NULL);
    italic_tag = gtk_text_buffer_create_tag(buffer, "italic",
                                          "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(buffer, "blue_foreground",
                             "foreground", "blue", NULL);
    
    // Line numbers setup
    GtkTextBuffer *line_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(line_numbers));
    gtk_text_buffer_create_tag(line_buffer, "right-align", 
                             "justification", GTK_JUSTIFY_RIGHT,
                             "right-margin", 6,
                             "left-margin", 6,
                             NULL);
    
    // Connect signals
    g_signal_connect(buffer, "changed", G_CALLBACK(on_text_changed), NULL);
    
    // Synchronize scrolling
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    gtk_scrollable_set_vadjustment(GTK_SCROLLABLE(line_numbers), adj);
    
    // Initial state
    gtk_widget_hide(line_numbers);
    gtk_paned_set_position(GTK_PANED(hpaned), 0);
}
