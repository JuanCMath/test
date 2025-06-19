#include "matcomguard.h"

int load_config(config_t *config) {
    FILE *config_file;
    char line[256];
    char key[128], value[128];
    
    config_file = fopen(CONFIG_FILE, "r");
    if (config_file == NULL) {
        return -1;
    }
    
    init_default_config(config);
    
    while (fgets(line, sizeof(line), config_file) != NULL) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        
        if (sscanf(line, "%127[^=]=%127s", key, value) == 2) {
            if (strcmp(key, "cpu_threshold") == 0) {
                config->cpu_threshold = atof(value);
            } else if (strcmp(key, "ram_threshold") == 0) {
                config->ram_threshold = atof(value);
            } else if (strcmp(key, "scan_interval") == 0) {
                config->scan_interval = atoi(value);
            } else if (strncmp(key, "whitelist_", 10) == 0) {
                if (config->whitelist_count < 1000) {
                    strcpy(config->whitelist_processes[config->whitelist_count], value);
                    config->whitelist_count++;
                }
            }
        }
    }
    
    fclose(config_file);
    return 0;
}

void init_default_config(config_t *config) {
    config->cpu_threshold = DEFAULT_CPU_THRESHOLD;
    config->ram_threshold = DEFAULT_RAM_THRESHOLD;
    config->scan_interval = DEFAULT_SCAN_INTERVAL;
    config->whitelist_count = 0;
    
    // Add default whitelisted processes
    strcpy(config->whitelist_processes[config->whitelist_count++], "gcc");
    strcpy(config->whitelist_processes[config->whitelist_count++], "make");
    strcpy(config->whitelist_processes[config->whitelist_count++], "gnome-shell");
    strcpy(config->whitelist_processes[config->whitelist_count++], "Xorg");
    strcpy(config->whitelist_processes[config->whitelist_count++], "systemd");
}

void add_alert(alert_t alerts[], int *alert_count, const char *type, const char *message, int severity) {
    if (*alert_count >= 1000) return; // Prevent overflow
    
    alerts[*alert_count].timestamp = time(NULL);
    strcpy(alerts[*alert_count].type, type);
    strcpy(alerts[*alert_count].message, message);
    alerts[*alert_count].severity = severity;
    
    (*alert_count)++;
    
    // Print alert immediately
    char *severity_str;
    switch (severity) {
        case 1: severity_str = "INFO"; break;
        case 2: severity_str = "WARNING"; break;
        case 3: severity_str = "CRITICAL"; break;
        default: severity_str = "UNKNOWN"; break;
    }
    
    printf("[%s] %s: %s\n", severity_str, type, message);
}

void print_alerts(alert_t alerts[], int alert_count) {
    int i;
    char time_str[64];
    
    printf("\n=== REPORTE DE ALERTAS ===\n");
    printf("Total de alertas: %d\n\n", alert_count);
    
    for (i = 0; i < alert_count; i++) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
                localtime(&alerts[i].timestamp));
        
        printf("[%s] %s - %s: %s\n", 
               time_str,
               alerts[i].severity == 3 ? "CRITICAL" : 
               alerts[i].severity == 2 ? "WARNING" : "INFO",
               alerts[i].type, 
               alerts[i].message);
    }
}

long get_file_size(const char *filepath) {
    struct stat file_stat;
    if (stat(filepath, &file_stat) == 0) {
        return file_stat.st_size;
    }
    return -1;
}

int file_exists(const char *filepath) {
    return access(filepath, F_OK) == 0;
}
