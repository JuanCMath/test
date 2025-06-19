# MatCom Guard 🛡️

Sistema de Seguridad y Monitoreo para UNIX - Protector del Reino Digital

## Descripción

MatCom Guard es un sistema de monitoreo y seguridad diseñado para vigilar y proteger sistemas basados en UNIX de amenazas informáticas y actividades sospechosas. El sistema actúa como una fortaleza digital que detecta y neutraliza amenazas antes de que causen daños.

## Características Principales

### 🔍 Detección de Dispositivos USB (Patrullas Fronterizas)
- Monitoreo automático de dispositivos USB conectados
- Detección de cambios en archivos (creación, modificación, eliminación)
- Cálculo de hashes SHA-256 para verificación de integridad
- Alertas en tiempo real sobre archivos sospechosos

### 💾 Monitoreo de Procesos (Guardias del Tesoro Real)
- Vigilancia continua del uso de CPU y memoria
- Detección de procesos con consumo anómalo de recursos
- Lista blanca de procesos conocidos como seguros
- Umbrales configurables para alertas

### 🌐 Escaneo de Puertos (Defensores de las Murallas)
- Escaneo de puertos TCP en rangos configurables
- Identificación de servicios por puerto
- Detección de puertos potencialmente comprometidos
- Alertas sobre puertos sospechosos

### 🖥️ Interfaz de Consola (Gran Salón del Trono)
- Menú interactivo con 4 opciones principales
- Reportes detallados en consola
- Exportación de reportes (preparado para PDF)
- Interfaz intuitiva estilo medieval

## Compilación

### Dependencias
```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev build-essential

# CentOS/RHEL
sudo yum install openssl-devel gcc make

# Arch Linux
sudo pacman -S openssl gcc make
```

### Construcción
```bash
make all
```

### Instalación
```bash
make install
```

## Uso

### Ejecución
```bash
./matcomguard
```

### Menú Principal
```
╔══════════════════════════════════════╗
║           MATCOM GUARD MENU          ║
╠══════════════════════════════════════╣
║ 1. Escanear Sistema de Archivos     ║
║ 2. Escanear Memoria y Procesos      ║
║ 3. Escanear Puertos de Red          ║
║ 4. Escaneo Completo del Sistema     ║
║ 0. Salir                            ║
╚══════════════════════════════════════╝
```

## Configuración

### Archivo de Configuración
Crear `/etc/matcomguard.conf`:

```ini
# Umbrales de alertas (porcentajes)
cpu_threshold=70.0
ram_threshold=50.0

# Intervalo de escaneo (segundos)
scan_interval=5

# Procesos en lista blanca
whitelist_0=gcc
whitelist_1=make
whitelist_2=gnome-shell
whitelist_3=Xorg
whitelist_4=systemd
```

## Casos de Prueba

### Pruebas de Dispositivos USB
```bash
# Insertar USB y verificar detección
sudo mount /dev/sdb1 /mnt/usb
echo "test" > /mnt/usb/test.txt  # Debería generar alerta

# Crear archivo sospechoso
cp /bin/bash /mnt/usb/malware.exe  # Alerta de archivo sospechoso
```

### Pruebas de Procesos
```bash
# Simular proceso con alto uso de CPU
while true; do :; done &  # Debería generar alerta

# Simular fuga de memoria
tail /dev/zero &  # Debería generar alerta de memoria
```

### Pruebas de Puertos
```bash
# Abrir puerto sospechoso
nc -l 31337 &  # Debería generar alerta

# Abrir puerto legítimo
sudo systemctl start ssh  # No debería generar alerta
```

## Estructura de Archivos

```
matcomguard/
├── matcomguard.h      # Definiciones y declaraciones
├── main.c             # Programa principal
├── usb_monitor.c      # Monitoreo de dispositivos USB
├── process_monitor.c  # Monitoreo de procesos
├── port_scanner.c     # Escáner de puertos
├── config.c           # Gestión de configuración
├── interface.c        # Interfaz de usuario
├── Makefile           # Configuración de compilación
└── README.md          # Documentación
```

## Ejemplos de Salida

### Detección de USB
```
[INFO] Iniciando patrulla fronteriza - Detectando dispositivos USB...
[DETECTED] Dispositivo USB: /dev/sdb1 montado en /mnt/usb
[INFO] Baseline establecido: 15 archivos catalogados
[WARNING] FILE_MODIFIED: Archivo modificado detectado: /mnt/usb/document.txt (hash cambiado)
[CRITICAL] SUSPICIOUS_FILE: Archivo sospechoso detectado: /mnt/usb/malware.exe (extensión potencialmente peligrosa)
```

### Monitoreo de Procesos
```
[INFO] Iniciando guardia del tesoro real - Monitoreando procesos...
PID      NOMBRE               CPU%     RAM%     ESTADO    
--------------------------------------------------------
1234     firefox              85.2     15.3     HIGH_CPU  
5678     chrome               45.1     25.8     OK        
[WARNING] HIGH_CPU_USAGE: Proceso 'firefox' (PID: 1234) usando 85.2% de CPU (umbral: 70.0%)
```

### Escaneo de Puertos
```
[INFO] Iniciando defensores de las murallas - Escaneando puertos 1-1024...
[OK] Puerto 22/tcp (SSH) abierto
[ALERTA] Puerto 31337/tcp abierto (potencialmente comprometido)
PUERTO   ESTADO       SERVICIO        SEGURIDAD 
------------------------------------------------
22       ABIERTO      SSH             OK        
31337    ABIERTO      Desconocido     SOSPECHOSO
```

## Seguridad

- **Permisos**: Requiere permisos de administrador para algunas funciones
- **Validación**: Todas las entradas son validadas
- **Límites**: Protección contra desbordamiento de buffer
- **Logging**: Registro detallado de todas las actividades

## Limitaciones Conocidas

1. Requiere OpenSSL para cálculo de hashes
2. Funciona solo en sistemas tipo UNIX
3. Escaneo de puertos limitado a localhost
4. Monitoreo de USB requiere montaje manual en algunos casos

## Contribución

1. Fork del repositorio
2. Crear rama para nuevas características
3. Implementar cambios con pruebas
4. Enviar pull request

## Licencia

Este proyecto es desarrollado para fines educativos en el contexto del curso de Sistemas Operativos.

## Contacto

Desarrollado como proyecto académico - MatCom Guard Team

---
*¡Que la protección de tu reino perdure!* 🏰⚔️
