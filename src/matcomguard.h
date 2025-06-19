#ifndef MATCOMGUARD_H
#define MATCOMGUARD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <openssl/sha.h>
#include <gtk/gtk.h>
#include <glib.h>

#define MAX_PATH 1024
#define MAX_DEVICES 10
#define MAX_PROCESSES 1000
#define MAX_PORTS 65535
#define HASH_SIZE 65

// Configuration defaults
#define DEFAULT_CPU_THRESHOLD 70.0
#define DEFAULT_RAM_THRESHOLD 50.0
#define DEFAULT_SCAN_INTERVAL 5
#define CONFIG_FILE "/etc/matcomguard.conf"

// GUI Update intervals (milliseconds)
#define GUI_UPDATE_INTERVAL 2000
#define PROGRESS_UPDATE_INTERVAL 100

// ...existing structures...

// GTK Application structure
typedef struct {
    GtkApplication *app;
    GtkWidget *main_window;
    GtkWidget *notebook;
    
    // USB Monitor tab
    GtkWidget *usb_treeview;
    GtkWidget *usb_scan_button;
    GtkWidget *usb_status_label;
    GtkListStore *usb_store;
    
    // Process Monitor tab
    GtkWidget *process_treeview;
    GtkWidget *process_scan_button;
    GtkWidget *process_status_label;
    GtkListStore *process_store;
    
    // Port Scanner tab
    GtkWidget *port_treeview;
    GtkWidget *port_scan_button;
    GtkWidget *port_range_start;
    GtkWidget *port_range_end;
    GtkWidget *port_status_label;
    GtkListStore *port_store;
    
    // Alerts tab
    GtkWidget *alerts_treeview;
    GtkWidget *alerts_clear_button;
    GtkListStore *alerts_store;
    
    // Status bar and progress
    GtkWidget *status_bar;
    GtkWidget *progress_bar;
    
    // Configuration
    config_t config;
    
    // Data storage
    usb_device_t devices[MAX_DEVICES];
    process_info_t processes[MAX_PROCESSES];
    port_info_t ports[MAX_PORTS];
    alert_t alerts[1000];
    
    int device_count;
    int process_count;
    int port_count;
    int alert_count;
    
    // Threading
    gboolean scanning_active;
    guint scan_timer_id;
    
} MatComGuardApp;

// GUI Function declarations
void activate_app(GtkApplication *app, gpointer user_data);
GtkWidget* create_main_window(MatComGuardApp *app_data);
void setup_usb_monitor_tab(MatComGuardApp *app_data);
void setup_process_monitor_tab(MatComGuardApp *app_data);
void setup_port_scanner_tab(MatComGuardApp *app_data);
void setup_alerts_tab(MatComGuardApp *app_data);

// Callback functions
void on_usb_scan_clicked(GtkButton *button, gpointer user_data);
void on_process_scan_clicked(GtkButton *button, gpointer user_data);
void on_port_scan_clicked(GtkButton *button, gpointer user_data);
void on_alerts_clear_clicked(GtkButton *button, gpointer user_data);
void on_scan_all_clicked(GtkButton *button, gpointer user_data);
void on_export_report_clicked(GtkButton *button, gpointer user_data);

// GUI Update functions
void update_usb_display(MatComGuardApp *app_data);
void update_process_display(MatComGuardApp *app_data);
void update_port_display(MatComGuardApp *app_data);
void update_alerts_display(MatComGuardApp *app_data);
void update_status_bar(MatComGuardApp *app_data, const char *message);
void update_progress_bar(MatComGuardApp *app_data, gdouble fraction);

// Threading functions
gpointer usb_scan_thread(gpointer user_data);
gpointer process_scan_thread(gpointer user_data);
gpointer port_scan_thread(gpointer user_data);
gboolean periodic_scan_callback(gpointer user_data);

// ...existing function declarations...

#endif
