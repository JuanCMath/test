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

// USB Device structure
typedef struct {
    char device_path[MAX_PATH];
    char mount_point[MAX_PATH];
    time_t mount_time;
    int file_count;
    char **file_hashes;
    char **file_paths;
} usb_device_t;

// Process information structure
typedef struct {
    pid_t pid;
    char name[256];
    float cpu_usage;
    float memory_usage;
    time_t last_check;
} process_info_t;

// Port information structure
typedef struct {
    int port;
    int is_open;
    char service[64];
    int is_suspicious;
} port_info_t;

// Configuration structure
typedef struct {
    float cpu_threshold;
    float ram_threshold;
    int scan_interval;
    char whitelist_processes[1000][256];
    int whitelist_count;
} config_t;

// Alert structure
typedef struct {
    time_t timestamp;
    char type[32];
    char message[512];
    int severity; // 1=Info, 2=Warning, 3=Critical
} alert_t;

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
gpointer full_scan_thread(gpointer user_data);
gboolean periodic_scan_callback(gpointer user_data);

// Configuration functions
int load_config(config_t *config);
void init_default_config(config_t *config);

// USB monitoring functions
int scan_usb_devices(usb_device_t devices[], int *device_count);
int monitor_device_changes(usb_device_t *device, alert_t alerts[], int *alert_count);
char* calculate_file_hash(const char *filepath);
int scan_directory_recursive(const char *path, char **file_paths, char **file_hashes, int *count);

// Process monitoring functions
int scan_processes(process_info_t processes[], int *process_count, config_t *config);
float get_cpu_usage(pid_t pid);
float get_memory_usage(pid_t pid);
int is_whitelisted_process(const char *process_name, config_t *config);

// Port scanning functions
int scan_ports(int start_port, int end_port, port_info_t ports[], int *port_count);
int is_port_open(int port);
const char* get_service_name(int port);
int is_suspicious_port(int port);

// Alert and reporting functions
void add_alert(alert_t alerts[], int *alert_count, const char *type, const char *message, int severity);
void print_alerts(alert_t alerts[], int alert_count);
void generate_report(usb_device_t devices[], int device_count, 
                    process_info_t processes[], int process_count,
                    port_info_t ports[], int port_count,
                    alert_t alerts[], int alert_count);
void export_report_pdf(const char *filename);

// Utility functions
long get_file_size(const char *filepath);
int file_exists(const char *filepath);

#endif
