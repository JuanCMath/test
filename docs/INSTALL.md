# Guía de Instalación - MatCom Guard

## Requisitos del Sistema

### Sistema Operativo
- Linux (Ubuntu 18.04+, CentOS 7+, Arch Linux)
- Kernel 3.10+
- GTK+ 3.0+

### Dependencias

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev libssl-dev build-essential pkg-config
```

#### CentOS/RHEL
```bash
sudo yum install gtk3-devel openssl-devel gcc make pkgconfig
# O para CentOS 8+
sudo dnf install gtk3-devel openssl-devel gcc make pkgconfig
```

#### Arch Linux
```bash
sudo pacman -S gtk3 openssl gcc make pkgconf
```

## Instalación

### Desde Código Fuente

1. **Clonar/Descargar el proyecto**
```bash
cd /tmp
# Asumir que tienes los archivos en el directorio actual
```

2. **Compilar**
```bash
make all
```

3. **Instalar (opcional)**
```bash
sudo make install
```

4. **Ejecutar**
```bash
# Si instalaste en el sistema
matcomguard

# Si no instalaste
./matcomguard
```

## Configuración

### Archivo de Configuración

El archivo de configuración se crea automáticamente en:
- `/etc/matcomguard.conf` (instalación del sistema)
- `./config/matcomguard.conf` (ejecución local)

### Permisos

MatCom Guard requiere ciertos permisos para funcionar correctamente:

```bash
# Para monitoreo de procesos
sudo setcap cap_sys_ptrace+ep ./matcomguard

# Para escaneo de puertos (alternativa a ejecutar como root)
sudo setcap cap_net_raw+ep ./matcomguard
```

## Verificación de Instalación

Ejecutar las pruebas integradas:

```bash
make test
```

## Solución de Problemas

### Error: "No se pudo acceder a /proc/mounts"
- Verificar que el sistema sea Linux
- Verificar permisos de lectura en /proc

### Error: "GTK libraries not found"
- Instalar las dependencias de desarrollo de GTK
- Verificar que pkg-config esté instalado

### Error: "OpenSSL libraries not found"
- Instalar las dependencias de desarrollo de OpenSSL
- En algunos sistemas: `sudo apt-get install libssl1.0-dev`

## Desinstalación

```bash
sudo make uninstall
```

## Estructura de Directorios

```
matcomguard/
├── matcomguard.h           # Header principal
├── main.c                  # Programa principal
├── usb_monitor.c          # Monitoreo USB
├── process_monitor.c      # Monitoreo de procesos
├── port_scanner.c         # Escáner de puertos
├── config.c               # Gestión de configuración
├── interface.c            # Interfaz GTK
├── Makefile              # Sistema de construcción
├── README.md             # Documentación principal
├── config/               # Archivos de configuración
│   └── matcomguard.conf  # Configuración de ejemplo
├── docs/                 # Documentación
│   └── INSTALL.md        # Esta guía
├── build/                # Archivos de construcción
└── logs/                 # Archivos de log
```
