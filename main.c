#include "matcomguard.h"

int main(int argc, char *argv[]) {
    MatComGuardApp app_data = {0};
    int status;
    
    // Initialize GTK
    app_data.app = gtk_application_new("com.matcom.guard", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app_data.app, "activate", G_CALLBACK(activate_app), &app_data);
    
    // Load configuration
    if (load_config(&app_data.config) != 0) {
        g_print("[WARNING] No se pudo cargar configuración, usando valores por defecto.\n");
        init_default_config(&app_data.config);
    }
    
    // Initialize data
    app_data.device_count = 0;
    app_data.process_count = 0;
    app_data.port_count = 0;
    app_data.alert_count = 0;
    app_data.scanning_active = FALSE;
    
    status = g_application_run(G_APPLICATION(app_data.app), argc, argv);
    g_object_unref(app_data.app);
    
    return status;
}

void activate_app(GtkApplication *app, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    app_data->main_window = create_main_window(app_data);
    gtk_application_add_window(app, GTK_WINDOW(app_data->main_window));
    gtk_widget_show_all(app_data->main_window);
    
    // Start periodic scanning
    app_data->scan_timer_id = g_timeout_add(GUI_UPDATE_INTERVAL, 
                                           periodic_scan_callback, 
                                           app_data);
    
    update_status_bar(app_data, "MatCom Guard iniciado - Reino protegido");
}

GtkWidget* create_main_window(MatComGuardApp *app_data) {
    GtkWidget *window;
    GtkWidget *header_bar;
    GtkWidget *main_box;
    GtkWidget *toolbar;
    GtkWidget *scan_all_button;
    GtkWidget *export_button;
    
    // Main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "MatCom Guard - Protector del Reino Digital");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    // Header bar
    header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "MatCom Guard");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Sistema de Seguridad UNIX");
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    
    // Main container
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // Toolbar
    toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);
    
    // Toolbar buttons
    scan_all_button = gtk_tool_button_new(gtk_image_new_from_icon_name("system-search", GTK_ICON_SIZE_LARGE_TOOLBAR), "Escaneo Completo");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(scan_all_button), -1);
    g_signal_connect(scan_all_button, "clicked", G_CALLBACK(on_scan_all_clicked), app_data);
    
    export_button = gtk_tool_button_new(gtk_image_new_from_icon_name("document-export", GTK_ICON_SIZE_LARGE_TOOLBAR), "Exportar Reporte");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(export_button), -1);
    g_signal_connect(export_button, "clicked", G_CALLBACK(on_export_report_clicked), app_data);
    
    // Notebook for tabs
    app_data->notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), app_data->notebook, TRUE, TRUE, 0);
    
    // Setup tabs
    setup_usb_monitor_tab(app_data);
    setup_process_monitor_tab(app_data);
    setup_port_scanner_tab(app_data);
    setup_alerts_tab(app_data);
    
    // Status bar
    app_data->status_bar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(main_box), app_data->status_bar, FALSE, FALSE, 0);
    
    // Progress bar
    app_data->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(app_data->progress_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(main_box), app_data->progress_bar, FALSE, FALSE, 0);
    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    return window;
}
