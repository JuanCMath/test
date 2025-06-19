#include "../matcomguard.h"

void setup_usb_monitor_tab(MatComGuardApp *app_data) {
    GtkWidget *usb_box;
    GtkWidget *usb_toolbar;
    GtkWidget *scrolled_window;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    
    // Main container
    usb_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(usb_box), 10);
    
    // Toolbar
    usb_toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(usb_toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    gtk_box_pack_start(GTK_BOX(usb_box), usb_toolbar, FALSE, FALSE, 0);
    
    // Scan button
    app_data->usb_scan_button = gtk_tool_button_new(
        gtk_image_new_from_icon_name("media-removable", GTK_ICON_SIZE_BUTTON),
        "Escanear USB"
    );
    gtk_toolbar_insert(GTK_TOOLBAR(usb_toolbar), GTK_TOOL_ITEM(app_data->usb_scan_button), -1);
    g_signal_connect(app_data->usb_scan_button, "clicked", G_CALLBACK(on_usb_scan_clicked), app_data);
    
    // Status label
    app_data->usb_status_label = gtk_label_new("Listo para patrullar las fronteras...");
    gtk_label_set_markup(GTK_LABEL(app_data->usb_status_label), 
                        "<i>Listo para patrullar las fronteras...</i>");
    gtk_box_pack_start(GTK_BOX(usb_box), app_data->usb_status_label, FALSE, FALSE, 5);
    
    // Create list store
    app_data->usb_store = gtk_list_store_new(6,
        G_TYPE_STRING,  // Device Path
        G_TYPE_STRING,  // Mount Point
        G_TYPE_STRING,  // Mount Time
        G_TYPE_INT,     // File Count
        G_TYPE_STRING,  // Status
        G_TYPE_STRING   // Last Check
    );
    
    // Create tree view
    app_data->usb_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app_data->usb_store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(app_data->usb_treeview), TRUE);
    
    // Add columns
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Dispositivo", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Punto de Montaje", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Montado", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Archivos", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Estado", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Última Verificación", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->usb_treeview), column);
    
    // Scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), app_data->usb_treeview);
    gtk_box_pack_start(GTK_BOX(usb_box), scrolled_window, TRUE, TRUE, 0);
    
    // Add tab to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app_data->notebook), usb_box,
                            gtk_label_new("🛡️ Patrullas Fronterizas"));
}

void on_usb_scan_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    if (app_data->scanning_active) {
        return;
    }
    
    gtk_widget_set_sensitive(app_data->usb_scan_button, FALSE);
    gtk_label_set_markup(GTK_LABEL(app_data->usb_status_label),
                        "<span color='blue'><b>Escaneando dispositivos USB...</b></span>");
    
    update_progress_bar(app_data, 0.0);
    
    // Start scan in separate thread
    g_thread_new("usb_scan", usb_scan_thread, app_data);
}

gpointer usb_scan_thread(gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    app_data->scanning_active = TRUE;
    
    // Perform USB scan
    scan_usb_devices(app_data->devices, &app_data->device_count);
    
    // Monitor changes if devices found
    if (app_data->device_count > 0) {
        gdk_threads_add_idle(G_CALLBACK(update_progress_bar), GINT_TO_POINTER(50));
        
        for (int i = 0; i < app_data->device_count; i++) {
            monitor_device_changes(&app_data->devices[i], 
                                 app_data->alerts, 
                                 &app_data->alert_count);
        }
    }
    
    // Update GUI
    gdk_threads_add_idle((GSourceFunc)update_usb_display, app_data);
    
    app_data->scanning_active = FALSE;
    
    return NULL;
}

void update_usb_display(MatComGuardApp *app_data) {
    GtkTreeIter iter;
    char time_str[64];
    char status_msg[256];
    
    // Clear existing data
    gtk_list_store_clear(app_data->usb_store);
    
    // Add devices to list
    for (int i = 0; i < app_data->device_count; i++) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&app_data->devices[i].mount_time));
        
        gtk_list_store_append(app_data->usb_store, &iter);
        gtk_list_store_set(app_data->usb_store, &iter,
            0, app_data->devices[i].device_path,
            1, app_data->devices[i].mount_point,
            2, time_str,
            3, app_data->devices[i].file_count,
            4, "Monitoreado",
            5, time_str,
            -1);
    }
    
    // Update status
    if (app_data->device_count > 0) {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='green'><b>%d dispositivo(s) USB detectado(s) y monitoreado(s)</b></span>",
                app_data->device_count);
    } else {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='orange'><i>No se detectaron dispositivos USB</i></span>");
    }
    
    gtk_label_set_markup(GTK_LABEL(app_data->usb_status_label), status_msg);
    gtk_widget_set_sensitive(app_data->usb_scan_button, TRUE);
    update_progress_bar(app_data, 1.0);
    
    // Update alerts display
    update_alerts_display(app_data);
}
