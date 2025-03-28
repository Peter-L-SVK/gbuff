#include <gtk/gtk.h>
#include <string.h>
#include <glib/gprintf.h>
#include "lib.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    GtkWidget *window, *menu_bar, *file_menu, *format_menu;
    GtkWidget *file_item, *format_item, *open_item, *save_item;
    GtkWidget *bold_item, *italic_item, *line_numbers_item, *vbox, *toolbar;
    GtkToolItem *bold_button, *italic_button;
    GtkWidget *scrolled_window;
    
    gtk_init(&argc, &argv);
    g_setenv("GTK_THEME", "Adwaita", TRUE);
    
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "gbuff");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    // Create menu bar
    menu_bar = gtk_menu_bar_new();
    
    // File menu
    file_menu = gtk_menu_new();
    file_item = gtk_menu_item_new_with_label("File");
    open_item = gtk_menu_item_new_with_label("Open");
    save_item = gtk_menu_item_new_with_label("Save");
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    
    g_signal_connect(open_item, "activate", G_CALLBACK(open_file), NULL);
    g_signal_connect(save_item, "activate", G_CALLBACK(save_file), NULL);
    
    // Format menu
    format_menu = gtk_menu_new();
    format_item = gtk_menu_item_new_with_label("Format");
    bold_item = gtk_menu_item_new_with_label("Bold");
    italic_item = gtk_menu_item_new_with_label("Italic");
    line_numbers_item = gtk_check_menu_item_new_with_label("Line Numbers");
    
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(format_item), format_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), bold_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), italic_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(format_menu), line_numbers_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), format_item);
    
    g_signal_connect(bold_item, "activate", G_CALLBACK(apply_bold), NULL);
    g_signal_connect(italic_item, "activate", G_CALLBACK(apply_italic), NULL);
    g_signal_connect(line_numbers_item, "toggled", G_CALLBACK(toggle_line_numbers), NULL);
    
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);
    
    // Create toolbar
    toolbar = gtk_toolbar_new();
    bold_button = gtk_tool_button_new(NULL, "Bold");
    italic_button = gtk_tool_button_new(NULL, "Italic");
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bold_button, -1);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), italic_button, -1);
    
    g_signal_connect(G_OBJECT(bold_button), "clicked", G_CALLBACK(apply_bold), NULL);
    g_signal_connect(G_OBJECT(italic_button), "clicked", G_CALLBACK(apply_italic), NULL);
    
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    // Create paned container for line numbers and text view
    hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), hpaned, TRUE, TRUE, 0);
    
    // Create text view with monospace font
    text_view = gtk_text_view_new();
    PangoFontDescription *font_desc = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(text_view, font_desc);
    pango_font_description_free(font_desc);
    
    // Create line numbers display
    line_numbers = gtk_text_view_new();
    PangoFontDescription *line_font = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(line_numbers, line_font);
    pango_font_description_free(line_font);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(line_numbers), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(line_numbers), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(line_numbers), GTK_WRAP_NONE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(line_numbers), 6);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(line_numbers), 6);
    gtk_widget_set_size_request(line_numbers, 40, -1);

    // Create scrolled window for text view
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                GTK_POLICY_AUTOMATIC, 
                                GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
    
    // Add widgets to paned container
    gtk_paned_pack1(GTK_PANED(hpaned), line_numbers, FALSE, FALSE);
    gtk_paned_pack2(GTK_PANED(hpaned), scrolled_window, TRUE, TRUE);
    
    // Get text buffer and configure it
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Focus and scroll settings
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
    
    // Line numbers buffer configuration
    GtkTextBuffer *line_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(line_numbers));
    gtk_text_buffer_create_tag(line_buffer, "right-align", 
                            "justification", GTK_JUSTIFY_RIGHT, NULL);
    
    // Connect signals
    g_signal_connect(buffer, "changed", G_CALLBACK(on_text_changed), NULL);
    g_signal_connect(text_view, "scroll-event", G_CALLBACK(on_scroll_event), NULL);
    
    // Synchronize scrolling
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    GtkAdjustment *line_adj = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(line_numbers));
    gtk_adjustment_configure(line_adj,
                           gtk_adjustment_get_value(adj),
                           gtk_adjustment_get_lower(adj),
                           gtk_adjustment_get_upper(adj),
                           gtk_adjustment_get_step_increment(adj),
                           gtk_adjustment_get_page_increment(adj),
                           gtk_adjustment_get_page_size(adj));
    
    g_signal_connect(adj, "value-changed",
                   G_CALLBACK((GCallback)gtk_adjustment_set_value), line_adj);
    g_signal_connect(line_adj, "value-changed",
                   G_CALLBACK((GCallback)gtk_adjustment_set_value), adj);
    
    // Initially hide line numbers
    gtk_widget_hide(line_numbers);
    gtk_paned_set_position(GTK_PANED(hpaned), 0);
    
    gtk_widget_show_all(window);
    gtk_main();
    
    exit(0);
}
