#include "matcomguard.h"

int scan_processes(process_info_t processes[], int *process_count, config_t *config) {
    DIR *proc_dir;
    struct dirent *entry;
    FILE *stat_file, *status_file;
    char path[MAX_PATH];
    char line[256];
    char process_name[256];
    pid_t pid;
    float cpu_usage, memory_usage;
    int count = 0;
    
    printf("[INFO] Iniciando guardia del tesoro real - Monitoreando procesos...\n");
    
    proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        printf("[ERROR] No se pudo acceder a /proc\n");
        return -1;
    }
    
    while ((entry = readdir(proc_dir)) != NULL && count < MAX_PROCESSES) {
        // Check if directory name is a PID (numeric)
        pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        
        // Read process name from /proc/PID/comm
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        stat_file = fopen(path, "r");
        if (stat_file == NULL) continue;
        
        if (fgets(process_name, sizeof(process_name), stat_file) != NULL) {
            // Remove newline
            process_name[strcspn(process_name, "\n")] = 0;
        }
        fclose(stat_file);
        
        // Get CPU and memory usage
        cpu_usage = get_cpu_usage(pid);
        memory_usage = get_memory_usage(pid);
        
        // Store process information
        processes[count].pid = pid;
        strcpy(processes[count].name, process_name);
        processes[count].cpu_usage = cpu_usage;
        processes[count].memory_usage = memory_usage;
        processes[count].last_check = time(NULL);
        
        count++;
    }
    
    closedir(proc_dir);
    *process_count = count;
    
    printf("[INFO] %d procesos monitoreados\n", count);
    return 0;
}

float get_cpu_usage(pid_t pid) {
    FILE *stat_file;
    char path[MAX_PATH];
    char line[1024];
    long utime, stime, starttime;
    static long system_uptime = 0;
    long process_time;
    float cpu_usage;
    
    // Get system uptime if not already retrieved
    if (system_uptime == 0) {
        FILE *uptime_file = fopen("/proc/uptime", "r");
        if (uptime_file != NULL) {
            fscanf(uptime_file, "%ld", &system_uptime);
            fclose(uptime_file);
        }
    }
    
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    stat_file = fopen(path, "r");
    if (stat_file == NULL) {
        return 0.0;
    }
    
    if (fgets(line, sizeof(line), stat_file) != NULL) {
        // Parse stat file - we need fields 14, 15, and 22
        char *token = strtok(line, " ");
        int field = 1;
        
        while (token != NULL && field <= 22) {
            if (field == 14) utime = atol(token);
            else if (field == 15) stime = atol(token);
            else if (field == 22) starttime = atol(token);
            
            token = strtok(NULL, " ");
            field++;
        }
        
        process_time = utime + stime;
        cpu_usage = (process_time * 100.0) / sysconf(_SC_CLK_TCK) / system_uptime;
    }
    
    fclose(stat_file);
    return cpu_usage > 100.0 ? 0.0 : cpu_usage; // Cap at reasonable value
}

float get_memory_usage(pid_t pid) {
    FILE *status_file;
    char path[MAX_PATH];
    char line[256];
    long vm_rss = 0; // Resident Set Size in KB
    long total_memory = 0;
    float memory_percentage;
    
    // Get total system memory
    FILE *meminfo = fopen("/proc/meminfo", "r");
    if (meminfo != NULL) {
        while (fgets(line, sizeof(line), meminfo) != NULL) {
            if (strncmp(line, "MemTotal:", 9) == 0) {
                sscanf(line, "MemTotal: %ld kB", &total_memory);
                break;
            }
        }
        fclose(meminfo);
    }
    
    // Get process memory usage
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    status_file = fopen(path, "r");
    if (status_file == NULL) {
        return 0.0;
    }
    
    while (fgets(line, sizeof(line), status_file) != NULL) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld kB", &vm_rss);
            break;
        }
    }
    
    fclose(status_file);
    
    if (total_memory > 0) {
        memory_percentage = (vm_rss * 100.0) / total_memory;
    } else {
        memory_percentage = 0.0;
    }
    
    return memory_percentage;
}

int is_whitelisted_process(const char *process_name, config_t *config) {
    int i;
    
    for (i = 0; i < config->whitelist_count; i++) {
        if (strcmp(process_name, config->whitelist_processes[i]) == 0) {
            return 1;
        }
    }
    
    return 0;
}
