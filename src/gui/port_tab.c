#include "../matcomguard.h"

void setup_port_scanner_tab(MatComGuardApp *app_data) {
    GtkWidget *port_box;
    GtkWidget *port_toolbar;
    GtkWidget *scrolled_window;
    GtkWidget *range_box;
    GtkWidget *start_label, *end_label;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    
    // Main container
    port_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(port_box), 10);
    
    // Port range configuration
    range_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(port_box), range_box, FALSE, FALSE, 5);
    
    start_label = gtk_label_new("Puerto inicial:");
    gtk_box_pack_start(GTK_BOX(range_box), start_label, FALSE, FALSE, 0);
    
    app_data->port_range_start = gtk_spin_button_new_with_range(1, 65535, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_data->port_range_start), 1);
    gtk_box_pack_start(GTK_BOX(range_box), app_data->port_range_start, FALSE, FALSE, 0);
    
    end_label = gtk_label_new("Puerto final:");
    gtk_box_pack_start(GTK_BOX(range_box), end_label, FALSE, FALSE, 0);
    
    app_data->port_range_end = gtk_spin_button_new_with_range(1, 65535, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(app_data->port_range_end), 1024);
    gtk_box_pack_start(GTK_BOX(range_box), app_data->port_range_end, FALSE, FALSE, 0);
    
    // Toolbar
    port_toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(port_toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    gtk_box_pack_start(GTK_BOX(port_box), port_toolbar, FALSE, FALSE, 0);
    
    // Scan button
    app_data->port_scan_button = gtk_tool_button_new(
        gtk_image_new_from_icon_name("network-workgroup", GTK_ICON_SIZE_BUTTON),
        "Escanear Puertos"
    );
    gtk_toolbar_insert(GTK_TOOLBAR(port_toolbar), GTK_TOOL_ITEM(app_data->port_scan_button), -1);
    g_signal_connect(app_data->port_scan_button, "clicked", G_CALLBACK(on_port_scan_clicked), app_data);
    
    // Status label
    app_data->port_status_label = gtk_label_new("Defensores de las murallas preparados...");
    gtk_label_set_markup(GTK_LABEL(app_data->port_status_label),
                        "<i>Defensores de las murallas preparados...</i>");
    gtk_box_pack_start(GTK_BOX(port_box), app_data->port_status_label, FALSE, FALSE, 5);
    
    // Create list store
    app_data->port_store = gtk_list_store_new(5,
        G_TYPE_INT,     // Port
        G_TYPE_STRING,  // Status
        G_TYPE_STRING,  // Service
        G_TYPE_STRING,  // Security Level
        G_TYPE_STRING   // Description
    );
    
    // Create tree view
    app_data->port_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app_data->port_store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(app_data->port_treeview), TRUE);
    
    // Add columns
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Puerto", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->port_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Estado", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->port_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Servicio", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->port_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Seguridad", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->port_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Descripción", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->port_treeview), column);
    
    // Scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), app_data->port_treeview);
    gtk_box_pack_start(GTK_BOX(port_box), scrolled_window, TRUE, TRUE, 0);
    
    // Add tab to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app_data->notebook), port_box,
                            gtk_label_new("🏰 Defensores de Murallas"));
}

void on_port_scan_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    if (app_data->scanning_active) {
        return;
    }
    
    gtk_widget_set_sensitive(app_data->port_scan_button, FALSE);
    gtk_label_set_markup(GTK_LABEL(app_data->port_status_label),
                        "<span color='blue'><b>Escaneando puertos de red...</b></span>");
    
    update_progress_bar(app_data, 0.0);
    
    // Start scan in separate thread
    g_thread_new("port_scan", port_scan_thread, app_data);
}

gpointer port_scan_thread(gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    app_data->scanning_active = TRUE;
    
    int start_port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_data->port_range_start));
    int end_port = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(app_data->port_range_end));
    
    // Perform port scan
    scan_ports(start_port, end_port, app_data->ports, &app_data->port_count);
    
    // Check for suspicious ports and generate alerts
    for (int i = 0; i < app_data->port_count; i++) {
        if (app_data->ports[i].is_open && app_data->ports[i].is_suspicious) {
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg),
                    "Puerto sospechoso %d/tcp abierto - Servicio: %s",
                    app_data->ports[i].port, app_data->ports[i].service);
            add_alert(app_data->alerts, &app_data->alert_count, "SUSPICIOUS_PORT", alert_msg, 3);
        }
    }
    
    // Update GUI
    gdk_threads_add_idle((GSourceFunc)update_port_display, app_data);
    
    app_data->scanning_active = FALSE;
    
    return NULL;
}

void update_port_display(MatComGuardApp *app_data) {
    GtkTreeIter iter;
    char status_msg[256];
    int open_ports = 0, suspicious_ports = 0;
    
    // Clear existing data
    gtk_list_store_clear(app_data->port_store);
    
    // Add ports to list (only show open ports)
    for (int i = 0; i < app_data->port_count; i++) {
        if (app_data->ports[i].is_open) {
            open_ports++;
            if (app_data->ports[i].is_suspicious) {
                suspicious_ports++;
            }
            
            gtk_list_store_append(app_data->port_store, &iter);
            gtk_list_store_set(app_data->port_store, &iter,
                0, app_data->ports[i].port,
                1, "ABIERTO",
                2, app_data->ports[i].service,
                3, app_data->ports[i].is_suspicious ? "SOSPECHOSO" : "OK",
                4, app_data->ports[i].is_suspicious ? "Posible backdoor/malware" : "Servicio legítimo",
                -1);
        }
    }
    
    // Update status
    if (suspicious_ports > 0) {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='red'><b>%d puerto(s) sospechoso(s) de %d abiertos (%d escaneados)</b></span>",
                suspicious_ports, open_ports, app_data->port_count);
    } else if (open_ports > 0) {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='green'><b>%d puerto(s) abierto(s) de %d escaneados - Todo seguro</b></span>",
                open_ports, app_data->port_count);
    } else {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='blue'><b>No se encontraron puertos abiertos en el rango escaneado</b></span>");
    }
    
    gtk_label_set_markup(GTK_LABEL(app_data->port_status_label), status_msg);
    gtk_widget_set_sensitive(app_data->port_scan_button, TRUE);
    update_progress_bar(app_data, 1.0);
    
    // Update alerts display
    update_alerts_display(app_data);
}
