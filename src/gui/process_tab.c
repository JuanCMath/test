#include "../matcomguard.h"

void setup_process_monitor_tab(MatComGuardApp *app_data) {
    GtkWidget *process_box;
    GtkWidget *process_toolbar;
    GtkWidget *scrolled_window;
    GtkWidget *config_box;
    GtkWidget *cpu_label, *ram_label;
    GtkWidget *cpu_spin, *ram_spin;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    
    // Main container
    process_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(process_box), 10);
    
    // Configuration box
    config_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(process_box), config_box, FALSE, FALSE, 5);
    
    cpu_label = gtk_label_new("Umbral CPU (%):");
    gtk_box_pack_start(GTK_BOX(config_box), cpu_label, FALSE, FALSE, 0);
    
    cpu_spin = gtk_spin_button_new_with_range(10.0, 100.0, 5.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(cpu_spin), app_data->config.cpu_threshold);
    gtk_box_pack_start(GTK_BOX(config_box), cpu_spin, FALSE, FALSE, 0);
    
    ram_label = gtk_label_new("Umbral RAM (%):");
    gtk_box_pack_start(GTK_BOX(config_box), ram_label, FALSE, FALSE, 0);
    
    ram_spin = gtk_spin_button_new_with_range(10.0, 100.0, 5.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ram_spin), app_data->config.ram_threshold);
    gtk_box_pack_start(GTK_BOX(config_box), ram_spin, FALSE, FALSE, 0);
    
    // Toolbar
    process_toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(process_toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    gtk_box_pack_start(GTK_BOX(process_box), process_toolbar, FALSE, FALSE, 0);
    
    // Scan button
    app_data->process_scan_button = gtk_tool_button_new(
        gtk_image_new_from_icon_name("system-monitor", GTK_ICON_SIZE_BUTTON),
        "Escanear Procesos"
    );
    gtk_toolbar_insert(GTK_TOOLBAR(process_toolbar), GTK_TOOL_ITEM(app_data->process_scan_button), -1);
    g_signal_connect(app_data->process_scan_button, "clicked", G_CALLBACK(on_process_scan_clicked), app_data);
    
    // Status label
    app_data->process_status_label = gtk_label_new("Guardias del tesoro real en espera...");
    gtk_label_set_markup(GTK_LABEL(app_data->process_status_label),
                        "<i>Guardias del tesoro real en espera...</i>");
    gtk_box_pack_start(GTK_BOX(process_box), app_data->process_status_label, FALSE, FALSE, 5);
    
    // Create list store
    app_data->process_store = gtk_list_store_new(7,
        G_TYPE_INT,     // PID
        G_TYPE_STRING,  // Name
        G_TYPE_DOUBLE,  // CPU %
        G_TYPE_DOUBLE,  // RAM %
        G_TYPE_STRING,  // Status
        G_TYPE_STRING,  // Last Check
        G_TYPE_STRING   // Alert Level
    );
    
    // Create tree view
    app_data->process_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app_data->process_store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(app_data->process_treeview), TRUE);
    
    // Add columns
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("PID", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Proceso", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("CPU %", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("RAM %", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 3);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Estado", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Última Verificación", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->process_treeview), column);
    
    // Scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), app_data->process_treeview);
    gtk_box_pack_start(GTK_BOX(process_box), scrolled_window, TRUE, TRUE, 0);
    
    // Add tab to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app_data->notebook), process_box,
                            gtk_label_new("⚔️ Guardias del Tesoro"));
}

void on_process_scan_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    if (app_data->scanning_active) {
        return;
    }
    
    gtk_widget_set_sensitive(app_data->process_scan_button, FALSE);
    gtk_label_set_markup(GTK_LABEL(app_data->process_status_label),
                        "<span color='blue'><b>Monitoreando procesos y memoria...</b></span>");
    
    update_progress_bar(app_data, 0.0);
    
    // Start scan in separate thread
    g_thread_new("process_scan", process_scan_thread, app_data);
}

gpointer process_scan_thread(gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    app_data->scanning_active = TRUE;
    
    // Perform process scan
    scan_processes(app_data->processes, &app_data->process_count, &app_data->config);
    
    // Check for suspicious processes
    for (int i = 0; i < app_data->process_count; i++) {
        if (app_data->processes[i].cpu_usage > app_data->config.cpu_threshold &&
            !is_whitelisted_process(app_data->processes[i].name, &app_data->config)) {
            
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg),
                    "Proceso '%s' (PID: %d) usando %.1f%% de CPU (umbral: %.1f%%)",
                    app_data->processes[i].name, app_data->processes[i].pid,
                    app_data->processes[i].cpu_usage, app_data->config.cpu_threshold);
            add_alert(app_data->alerts, &app_data->alert_count, "HIGH_CPU_USAGE", alert_msg, 2);
        }
        
        if (app_data->processes[i].memory_usage > app_data->config.ram_threshold &&
            !is_whitelisted_process(app_data->processes[i].name, &app_data->config)) {
            
            char alert_msg[256];
            snprintf(alert_msg, sizeof(alert_msg),
                    "Proceso '%s' (PID: %d) usando %.1f%% de RAM (umbral: %.1f%%)",
                    app_data->processes[i].name, app_data->processes[i].pid,
                    app_data->processes[i].memory_usage, app_data->config.ram_threshold);
            add_alert(app_data->alerts, &app_data->alert_count, "HIGH_MEMORY_USAGE", alert_msg, 2);
        }
    }
    
    // Update GUI
    gdk_threads_add_idle((GSourceFunc)update_process_display, app_data);
    
    app_data->scanning_active = FALSE;
    
    return NULL;
}

void update_process_display(MatComGuardApp *app_data) {
    GtkTreeIter iter;
    char time_str[64];
    char status_msg[256];
    int suspicious_count = 0;
    
    // Clear existing data
    gtk_list_store_clear(app_data->process_store);
    
    // Add processes to list (only show significant ones)
    for (int i = 0; i < app_data->process_count; i++) {
        if (app_data->processes[i].cpu_usage > 5.0 || 
            app_data->processes[i].memory_usage > 5.0) {
            
            strftime(time_str, sizeof(time_str), "%H:%M:%S",
                    localtime(&app_data->processes[i].last_check));
            
            char status[32] = "OK";
            if (app_data->processes[i].cpu_usage > app_data->config.cpu_threshold) {
                strcpy(status, "HIGH_CPU");
                suspicious_count++;
            } else if (app_data->processes[i].memory_usage > app_data->config.ram_threshold) {
                strcpy(status, "HIGH_RAM");
                suspicious_count++;
            }
            
            gtk_list_store_append(app_data->process_store, &iter);
            gtk_list_store_set(app_data->process_store, &iter,
                0, app_data->processes[i].pid,
                1, app_data->processes[i].name,
                2, app_data->processes[i].cpu_usage,
                3, app_data->processes[i].memory_usage,
                4, status,
                5, time_str,
                -1);
        }
    }
    
    // Update status
    if (suspicious_count > 0) {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='red'><b>%d proceso(s) sospechoso(s) de %d monitoreados</b></span>",
                suspicious_count, app_data->process_count);
    } else {
        snprintf(status_msg, sizeof(status_msg),
                "<span color='green'><b>%d procesos monitoreados - Todo normal</b></span>",
                app_data->process_count);
    }
    
    gtk_label_set_markup(GTK_LABEL(app_data->process_status_label), status_msg);
    gtk_widget_set_sensitive(app_data->process_scan_button, TRUE);
    update_progress_bar(app_data, 1.0);
    
    // Update alerts display
    update_alerts_display(app_data);
}
