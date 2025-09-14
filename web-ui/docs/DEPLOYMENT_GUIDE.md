# Multi-Sonicator-IO Web UI Production Deployment Guide

## Overview

This guide provides comprehensive instructions for deploying the Multi-Sonicator-IO Web UI in production environments, including sandbox mode setup with firmware upload workflow.

## Prerequisites

### System Requirements

- **Operating System**: Linux (Ubuntu 20.04+ recommended) or macOS
- **Node.js**: Version 18.x or higher
- **Python**: Version 3.8 or higher
- **Docker**: Version 20.10+ (optional, for containerized deployment)
- **Hardware**: Arduino Uno R3 + ATmega32A DUT setup

### Hardware Setup

1. **Arduino Uno R3** with USB connection
2. **ATmega32A DUT** connected per pin matrix specification
3. **USB cables** for both devices
4. **Power supply** for DUT (if required)

## Deployment Methods

### Method 1: Native Deployment (Recommended)

#### 1. Environment Setup

```bash
# Clone repository
git clone https://github.com/Cannasol-Tech/multi-sonicator-io.git
cd multi-sonicator-io

# Install system dependencies
sudo apt-get update
sudo apt-get install -y python3 python3-pip nodejs npm build-essential

# Install PlatformIO for firmware builds
pip3 install platformio

# Install Arduino CLI for hardware programming
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
sudo mv bin/arduino-cli /usr/local/bin/
arduino-cli core update-index
arduino-cli core install arduino:avr
```

#### 2. Web UI Installation

```bash
# Install Web UI dependencies
make web-ui-install

# Build production assets
make web-ui-build

# Run tests to verify installation
make web-ui-test
```

#### 3. Production Configuration

Create production environment file:

```bash
# Create production environment
cat > web-ui/.env.production << EOF
NODE_ENV=production
PORT=3001
FRONTEND_PORT=3101
PYTHONPATH=/app
LOG_LEVEL=info
EOF
```

#### 4. Start Production Services

```bash
# Start backend service
cd web-ui/backend
npm start &

# Start frontend service (if serving separately)
cd web-ui/frontend
npm run preview &
```

### Method 2: Docker Deployment

#### 1. Build Docker Images

```bash
# Build all Web UI Docker images
make web-ui-docker-build
```

#### 2. Start Production Environment

```bash
# Start production containers
make web-ui-docker-prod
```

#### 3. Verify Deployment

```bash
# Check container status
docker ps

# Check health endpoints
curl http://localhost:3001/api/health
curl http://localhost:3101/health
```

## Sandbox Mode Deployment

### Full Sandbox Workflow

The sandbox mode provides a complete testing environment with automatic firmware programming:

```bash
# Launch complete sandbox environment
make web-ui-sandbox
```

This command executes the following workflow:

1. **Hardware Detection**: Automatically detects Arduino port
2. **Arduino ISP Setup**: Uploads Arduino as ISP sketch
3. **Firmware Build**: Builds latest production firmware
4. **DUT Programming**: Programs ATmega32A via Arduino ISP
5. **Test Harness Upload**: Uploads Arduino test harness firmware
6. **Web UI Launch**: Starts backend and frontend services
7. **Browser Launch**: Opens interface in default browser
8. **Connectivity Verification**: Confirms hardware communication

### Sandbox Mode Configuration

The sandbox mode uses these configuration files:
- `config/hardware-config.yaml` - Hardware pin mappings (SOLE SOURCE OF TRUTH)
- `docs/planning/pin-matrix.md` - Pin connection documentation
- `platformio.ini` - Firmware build configuration

### Troubleshooting Sandbox Mode

**Hardware Not Detected**:
```bash
# Manual hardware detection
python3 scripts/detect_hardware.py

# Check USB permissions
sudo usermod -a -G dialout $USER
# Logout and login again
```

**Firmware Upload Fails**:
```bash
# Check Arduino ISP setup
python3 scripts/setup_arduino_isp.py

# Manual firmware upload
make upload-to-device
```

**Web UI Connection Issues**:
```bash
# Check service status
curl http://localhost:3001/api/health

# Restart services
make web-ui-stop
make web-ui-dev
```

## Production Monitoring

### Health Checks

The Web UI provides several health check endpoints:

```bash
# Backend health
curl http://localhost:3001/api/health

# Frontend health
curl http://localhost:3101/health

# Hardware connectivity
curl http://localhost:3001/api/connection
```

### Log Monitoring

```bash
# Backend logs (if using PM2 or systemd)
tail -f /var/log/multi-sonicator-backend.log

# Docker logs
docker logs multi-sonicator-backend-prod
docker logs multi-sonicator-frontend-prod
```

### Performance Monitoring

Key metrics to monitor:
- **Response Time**: Hardware commands should complete within 100ms
- **WebSocket Connections**: Monitor active connections
- **Memory Usage**: Backend Python processes
- **CPU Usage**: During test execution

## Security Considerations

### Network Security

```bash
# Firewall configuration (Ubuntu)
sudo ufw allow 3001/tcp  # Backend API
sudo ufw allow 3101/tcp  # Frontend
sudo ufw enable
```

### Hardware Access

```bash
# Secure hardware access
sudo usermod -a -G dialout multi-sonicator-user
sudo chmod 660 /dev/ttyUSB*
```

### Container Security

For Docker deployments:
- Containers run as non-root users
- Hardware access requires privileged mode
- Volumes are mounted read-only where possible

## Backup and Recovery

### Configuration Backup

```bash
# Backup critical configuration
tar -czf multi-sonicator-config-$(date +%Y%m%d).tar.gz \
  config/ \
  docs/planning/pin-matrix.md \
  web-ui/.env.production
```

### Data Recovery

```bash
# Restore configuration
tar -xzf multi-sonicator-config-YYYYMMDD.tar.gz
```

## Scaling and Load Balancing

### Multiple Hardware Interfaces

For multiple DUT testing:

```bash
# Start additional backend instances
PORT=3002 cd web-ui/backend && npm start &
PORT=3003 cd web-ui/backend && npm start &
```

### Load Balancer Configuration (Nginx)

```nginx
upstream multi-sonicator-backend {
    server localhost:3001;
    server localhost:3002;
    server localhost:3003;
}

server {
    listen 80;
    server_name multi-sonicator.local;
    
    location /api/ {
        proxy_pass http://multi-sonicator-backend;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection 'upgrade';
        proxy_set_header Host $host;
        proxy_cache_bypass $http_upgrade;
    }
    
    location / {
        proxy_pass http://localhost:3101;
    }
}
```

## Maintenance

### Regular Updates

```bash
# Update dependencies
cd web-ui/frontend && npm update
cd web-ui/backend && npm update
pip3 install --upgrade -r web-ui/backend/requirements.txt

# Rebuild and test
make web-ui-build
make web-ui-test
```

### Database Maintenance

If using Redis for session management:

```bash
# Redis maintenance
redis-cli FLUSHDB  # Clear session data
redis-cli BGSAVE  # Background save
```

## Troubleshooting

### Common Issues

**Port Already in Use**:
```bash
# Kill processes on Web UI ports
make web-ui-stop
lsof -ti:3001,3101 | xargs kill -9
```

**Permission Denied (Hardware)**:
```bash
# Fix hardware permissions
sudo chmod 666 /dev/ttyUSB*
sudo usermod -a -G dialout $USER
```

**High Memory Usage**:
```bash
# Monitor Python processes
ps aux | grep python
# Restart backend if needed
pkill -f "node.*backend"
cd web-ui/backend && npm start &
```

### Log Analysis

```bash
# Check for errors in logs
grep -i error /var/log/multi-sonicator-*.log
journalctl -u multi-sonicator-backend -f
```

## Support and Documentation

- **API Documentation**: [web-ui/docs/API.md](./API.md)
- **User Guide**: [web-ui/docs/USER_GUIDE.md](./USER_GUIDE.md)
- **Architecture**: [web-ui/docs/ADR-001-Integration-Architecture.md](./ADR-001-Integration-Architecture.md)
- **Hardware Configuration**: `config/hardware-config.yaml`
- **Pin Matrix**: `docs/planning/pin-matrix.md`

## Version Information

- **Web UI Version**: 1.1.0
- **API Version**: 1.1.0
- **Node.js**: 18.x+
- **Python**: 3.8+
- **Docker**: 20.10+
