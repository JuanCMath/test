#include "../matcomguard.h"

void setup_alerts_tab(MatComGuardApp *app_data) {
    GtkWidget *alerts_box;
    GtkWidget *alerts_toolbar;
    GtkWidget *scrolled_window;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    
    // Main container
    alerts_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(alerts_box), 10);
    
    // Toolbar
    alerts_toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(alerts_toolbar), GTK_TOOLBAR_BOTH_HORIZ);
    gtk_box_pack_start(GTK_BOX(alerts_box), alerts_toolbar, FALSE, FALSE, 0);
    
    // Clear button
    app_data->alerts_clear_button = gtk_tool_button_new(
        gtk_image_new_from_icon_name("edit-clear-all", GTK_ICON_SIZE_BUTTON),
        "Limpiar Alertas"
    );
    gtk_toolbar_insert(GTK_TOOLBAR(alerts_toolbar), GTK_TOOL_ITEM(app_data->alerts_clear_button), -1);
    g_signal_connect(app_data->alerts_clear_button, "clicked", G_CALLBACK(on_alerts_clear_clicked), app_data);
    
    // Create list store
    app_data->alerts_store = gtk_list_store_new(5,
        G_TYPE_STRING,  // Timestamp
        G_TYPE_STRING,  // Type
        G_TYPE_STRING,  // Severity
        G_TYPE_STRING,  // Message
        G_TYPE_STRING   // Icon
    );
    
    // Create tree view
    app_data->alerts_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(app_data->alerts_store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(app_data->alerts_treeview), TRUE);
    
    // Add columns
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Tiempo", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->alerts_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Tipo", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->alerts_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Severidad", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_sort_column_id(column, 2);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->alerts_treeview), column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Mensaje", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_column_set_expand(column, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(app_data->alerts_treeview), column);
    
    // Scrolled window
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), app_data->alerts_treeview);
    gtk_box_pack_start(GTK_BOX(alerts_box), scrolled_window, TRUE, TRUE, 0);
    
    // Add tab to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(app_data->notebook), alerts_box,
                            gtk_label_new("🚨 Centro de Alertas"));
}

void on_alerts_clear_clicked(GtkButton *button, gpointer user_data) {
    MatComGuardApp *app_data = (MatComGuardApp*)user_data;
    
    // Clear alerts array
    app_data->alert_count = 0;
    
    // Clear GUI display
    gtk_list_store_clear(app_data->alerts_store);
    
    update_status_bar(app_data, "Alertas limpiadas - Reino en calma");
}

void update_alerts_display(MatComGuardApp *app_data) {
    GtkTreeIter iter;
    char time_str[64];
    const char *severity_str;
    
    // Clear existing data
    gtk_list_store_clear(app_data->alerts_store);
    
    // Add alerts to list
    for (int i = 0; i < app_data->alert_count; i++) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                localtime(&app_data->alerts[i].timestamp));
        
        switch (app_data->alerts[i].severity) {
            case 1: severity_str = "INFO"; break;
            case 2: severity_str = "WARNING"; break;
            case 3: severity_str = "CRITICAL"; break;
            default: severity_str = "UNKNOWN"; break;
        }
        
        gtk_list_store_append(app_data->alerts_store, &iter);
        gtk_list_store_set(app_data->alerts_store, &iter,
            0, time_str,
            1, app_data->alerts[i].type,
            2, severity_str,
            3, app_data->alerts[i].message,
            4, app_data->alerts[i].severity == 3 ? "⚠️" : 
               app_data->alerts[i].severity == 2 ? "⚡" : "ℹ️",
            -1);
    }
    
    // Auto-scroll to bottom to show latest alerts
    if (app_data->alert_count > 0) {
        GtkTreePath *path = gtk_tree_path_new_from_indices(app_data->alert_count - 1, -1);
        gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(app_data->alerts_treeview), 
                                    path, NULL, FALSE, 0.0, 0.0);
        gtk_tree_path_free(path);
    }
}
