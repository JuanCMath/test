#include "../matcomguard.h"

void update_status_bar(MatComGuardApp *app_data, const char *message) {
    guint context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app_data->status_bar), "main");
    gtk_statusbar_pop(GTK_STATUSBAR(app_data->status_bar), context_id);
    gtk_statusbar_push(GTK_STATUSBAR(app_data->status_bar), context_id, message);
}

void update_progress_bar(MatComGuardApp *app_data, gdouble fraction) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(app_data->progress_bar), fraction);
    
    if (fraction >= 1.0) {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app_data->progress_bar), "Completado");
        g_timeout_add(2000, (GSourceFunc)gtk_progress_bar_pulse, app_data->progress_bar);
    } else if (fraction > 0.0) {
        char progress_text[64];
        snprintf(progress_text, sizeof(progress_text), "%.0f%% completado", fraction * 100);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app_data->progress_bar), progress_text);
    } else {
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(app_data->progress_bar), "Iniciando...");
    }
    
    // Force GUI update
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
}

void on_scan_all_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    if (app_data->scanning_active) {
        return;
    }
    
    update_status_bar(app_data, "Iniciando escaneo completo del reino...");
    
    // Trigger all scans sequentially
    g_thread_new("full_scan", full_scan_thread, app_data);
}

gpointer full_scan_thread(gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    app_data->scanning_active = TRUE;
    
    // USB Scan
    gdk_threads_add_idle_full(G_PRIORITY_HIGH, 
        (GSourceFunc)update_status_bar, app_data, 
        "Escaneando dispositivos USB...");
    update_progress_bar(app_data, 0.1);
    
    scan_usb_devices(app_data->devices, &app_data->device_count);
    for (int i = 0; i < app_data->device_count; i++) {
        monitor_device_changes(&app_data->devices[i], app_data->alerts, &app_data->alert_count);
    }
    gdk_threads_add_idle((GSourceFunc)update_usb_display, app_data);
    
    // Process Scan
    gdk_threads_add_idle_full(G_PRIORITY_HIGH,
        (GSourceFunc)update_status_bar, app_data,
        "Monitoreando procesos y memoria...");
    update_progress_bar(app_data, 0.4);
    
    scan_processes(app_data->processes, &app_data->process_count, &app_data->config);
    gdk_threads_add_idle((GSourceFunc)update_process_display, app_data);
    
    // Port Scan
    gdk_threads_add_idle_full(G_PRIORITY_HIGH,
        (GSourceFunc)update_status_bar, app_data,
        "Escaneando puertos de red...");
    update_progress_bar(app_data, 0.7);
    
    scan_ports(1, 1024, app_data->ports, &app_data->port_count);
    gdk_threads_add_idle((GSourceFunc)update_port_display, app_data);
    
    // Complete
    update_progress_bar(app_data, 1.0);
    gdk_threads_add_idle_full(G_PRIORITY_HIGH,
        (GSourceFunc)update_status_bar, app_data,
        "Escaneo completo finalizado - Reino inspeccionado");
    
    app_data->scanning_active = FALSE;
    
    return NULL;
}

void on_export_report_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;
    
    dialog = gtk_file_chooser_dialog_new("Exportar Reporte",
                                        GTK_WINDOW(app_data->main_window),
                                        action,
                                        "_Cancelar", GTK_RESPONSE_CANCEL,
                                        "_Guardar", GTK_RESPONSE_ACCEPT,
                                        NULL);
    
    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_name(chooser, "matcom_guard_report.txt");
    
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        
        // Generate and export report
        generate_report(app_data->devices, app_data->device_count,
                       app_data->processes, app_data->process_count,
                       app_data->ports, app_data->port_count,
                       app_data->alerts, app_data->alert_count);
        
        update_status_bar(app_data, "Reporte exportado exitosamente");
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

gboolean periodic_scan_callback(gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    if (!app_data->scanning_active) {
        // Perform lightweight monitoring updates
        // This could be used for real-time monitoring
    }
    
    return TRUE; // Continue periodic callbacks
}
