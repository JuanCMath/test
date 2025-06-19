#include "matcomguard.h"

int scan_ports(int start_port, int end_port, port_info_t ports[], int *port_count) {
    int port, count = 0;
    int sock;
    struct sockaddr_in target;
    
    printf("[INFO] Iniciando defensores de las murallas - Escaneando puertos %d-%d...\n", 
           start_port, end_port);
    
    for (port = start_port; port <= end_port && count < MAX_PORTS; port++) {
        ports[count].port = port;
        ports[count].is_open = is_port_open(port);
        
        if (ports[count].is_open) {
            strcpy(ports[count].service, get_service_name(port));
            ports[count].is_suspicious = is_suspicious_port(port);
            
            if (ports[count].is_suspicious) {
                printf("[ALERTA] Puerto %d/tcp abierto (potencialmente comprometido)\n", port);
            } else {
                printf("[OK] Puerto %d/tcp (%s) abierto\n", port, ports[count].service);
            }
        } else {
            strcpy(ports[count].service, "cerrado");
            ports[count].is_suspicious = 0;
        }
        
        count++;
        
        // Progress indicator for large scans
        if ((port - start_port + 1) % 100 == 0) {
            printf("[INFO] Progreso: %d/%d puertos escaneados\n", 
                   port - start_port + 1, end_port - start_port + 1);
        }
    }
    
    *port_count = count;
    return 0;
}

int is_port_open(int port) {
    int sock;
    struct sockaddr_in target;
    int result;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 0;
    }
    
    target.sin_family = AF_INET;
    target.sin_port = htons(port);
    target.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Set socket timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    result = connect(sock, (struct sockaddr*)&target, sizeof(target));
    close(sock);
    
    return (result == 0) ? 1 : 0;
}

const char* get_service_name(int port) {
    switch (port) {
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 80: return "HTTP";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 443: return "HTTPS";
        case 993: return "IMAPS";
        case 995: return "POP3S";
        case 3306: return "MySQL";
        case 5432: return "PostgreSQL";
        case 6379: return "Redis";
        case 8080: return "HTTP-Alt";
        default: return "Desconocido";
    }
}

int is_suspicious_port(int port) {
    // Common backdoor/malware ports
    int suspicious_ports[] = {
        31337, 12345, 27374, 1234, 6667, 6666, 4444, 5555, 9999,
        1337, 7777, 8888, 2222, 3333, 54321, 65000, 1981, 1991
    };
    
    int num_suspicious = sizeof(suspicious_ports) / sizeof(suspicious_ports[0]);
    int i;
    
    for (i = 0; i < num_suspicious; i++) {
        if (port == suspicious_ports[i]) {
            return 1;
        }
    }
    
    // High ports (>1024) without common services
    if (port > 1024 && strcmp(get_service_name(port), "Desconocido") == 0) {
        return 1;
    }
    
    return 0;
}
