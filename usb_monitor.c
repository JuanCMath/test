#include "matcomguard.h"
#include <mntent.h>

int scan_usb_devices(usb_device_t devices[], int *device_count) {
    FILE *mounts;
    struct mntent *mount_entry;
    int count = 0;
    
    printf("[INFO] Iniciando patrulla fronteriza - Detectando dispositivos USB...\n");
    
    mounts = setmntent("/proc/mounts", "r");
    if (mounts == NULL) {
        printf("[ERROR] No se pudo acceder a /proc/mounts\n");
        return -1;
    }
    
    while ((mount_entry = getmntent(mounts)) != NULL && count < MAX_DEVICES) {
        // Check for USB devices (common mount points and filesystem types)
        if (strstr(mount_entry->mnt_dir, "/media") || 
            strstr(mount_entry->mnt_dir, "/mnt") ||
            strcmp(mount_entry->mnt_type, "vfat") == 0 ||
            strcmp(mount_entry->mnt_type, "ntfs") == 0 ||
            strcmp(mount_entry->mnt_type, "exfat") == 0) {
            
            // Skip system mounts
            if (strcmp(mount_entry->mnt_dir, "/") == 0 ||
                strstr(mount_entry->mnt_dir, "/proc") ||
                strstr(mount_entry->mnt_dir, "/sys") ||
                strstr(mount_entry->mnt_dir, "/dev")) {
                continue;
            }
            
            strcpy(devices[count].device_path, mount_entry->mnt_fsname);
            strcpy(devices[count].mount_point, mount_entry->mnt_dir);
            devices[count].mount_time = time(NULL);
            devices[count].file_count = 0;
            
            printf("[DETECTED] Dispositivo USB: %s montado en %s\n", 
                   devices[count].device_path, devices[count].mount_point);
            
            // Scan files and create baseline
            devices[count].file_paths = malloc(1000 * sizeof(char*));
            devices[count].file_hashes = malloc(1000 * sizeof(char*));
            
            scan_directory_recursive(devices[count].mount_point, 
                                   devices[count].file_paths,
                                   devices[count].file_hashes,
                                   &devices[count].file_count);
            
            printf("[INFO] Baseline establecido: %d archivos catalogados\n", 
                   devices[count].file_count);
            count++;
        }
    }
    
    endmntent(mounts);
    *device_count = count;
    
    if (count == 0) {
        printf("[INFO] No se detectaron dispositivos USB conectados\n");
    }
    
    return 0;
}

int scan_directory_recursive(const char *path, char **file_paths, char **file_hashes, int *count) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[MAX_PATH];
    
    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                scan_directory_recursive(full_path, file_paths, file_hashes, count);
            } else if (S_ISREG(file_stat.st_mode)) {
                file_paths[*count] = malloc(strlen(full_path) + 1);
                strcpy(file_paths[*count], full_path);
                
                file_hashes[*count] = calculate_file_hash(full_path);
                (*count)++;
                
                if (*count >= 1000) break; // Prevent overflow
            }
        }
    }
    
    closedir(dir);
    return 0;
}

char* calculate_file_hash(const char *filepath) {
    FILE *file;
    unsigned char buffer[1024];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char *hash_string;
    SHA256_CTX sha256;
    size_t bytes;
    int i;
    
    file = fopen(filepath, "rb");
    if (file == NULL) {
        return NULL;
    }
    
    SHA256_Init(&sha256);
    
    while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
        SHA256_Update(&sha256, buffer, bytes);
    }
    
    SHA256_Final(hash, &sha256);
    fclose(file);
    
    hash_string = malloc(HASH_SIZE);
    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_string + (i * 2), "%02x", hash[i]);
    }
    hash_string[64] = '\0';
    
    return hash_string;
}

int monitor_device_changes(usb_device_t *device, alert_t alerts[], int *alert_count) {
    char **current_paths = malloc(1000 * sizeof(char*));
    char **current_hashes = malloc(1000 * sizeof(char*));
    int current_count = 0;
    int i, j, found;
    char alert_msg[512];
    
    printf("[INFO] Monitoreando cambios en %s...\n", device->mount_point);
    
    // Scan current state
    scan_directory_recursive(device->mount_point, current_paths, current_hashes, &current_count);
    
    // Check for new or modified files
    for (i = 0; i < current_count; i++) {
        found = 0;
        for (j = 0; j < device->file_count; j++) {
            if (strcmp(current_paths[i], device->file_paths[j]) == 0) {
                found = 1;
                // Check if hash changed
                if (strcmp(current_hashes[i], device->file_hashes[j]) != 0) {
                    snprintf(alert_msg, sizeof(alert_msg), 
                            "Archivo modificado detectado: %s (hash cambiado)", 
                            current_paths[i]);
                    add_alert(alerts, alert_count, "FILE_MODIFIED", alert_msg, 2);
                }
                break;
            }
        }
        
        if (!found) {
            snprintf(alert_msg, sizeof(alert_msg), 
                    "Nuevo archivo detectado: %s", current_paths[i]);
            add_alert(alerts, alert_count, "FILE_ADDED", alert_msg, 2);
            
            // Check for suspicious extensions
            if (strstr(current_paths[i], ".exe") || 
                strstr(current_paths[i], ".bat") ||
                strstr(current_paths[i], ".scr")) {
                snprintf(alert_msg, sizeof(alert_msg), 
                        "Archivo sospechoso detectado: %s (extensión potencialmente peligrosa)", 
                        current_paths[i]);
                add_alert(alerts, alert_count, "SUSPICIOUS_FILE", alert_msg, 3);
            }
        }
    }
    
    // Check for deleted files
    for (i = 0; i < device->file_count; i++) {
        found = 0;
        for (j = 0; j < current_count; j++) {
            if (strcmp(device->file_paths[i], current_paths[j]) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            snprintf(alert_msg, sizeof(alert_msg), 
                    "Archivo eliminado detectado: %s", device->file_paths[i]);
            add_alert(alerts, alert_count, "FILE_DELETED", alert_msg, 2);
        }
    }
    
    // Cleanup
    for (i = 0; i < current_count; i++) {
        free(current_paths[i]);
        free(current_hashes[i]);
    }
    free(current_paths);
    free(current_hashes);
    
    return 0;
}
