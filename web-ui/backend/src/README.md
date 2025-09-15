# Backend Documentation

This directory contains the source code for the backend of the Multi-Sonicator-IO Web UI. The backend is responsible for handling hardware communication, providing RESTful APIs, and managing real-time data streaming through WebSockets.

## Key Components

1. **Adapters**: Interfaces for hardware communication
2. **Routes**: RESTful API endpoints
3. **Services**: Business logic and test automation
4. **Websocket**: Real-time data streaming

## Setup

To set up the backend, follow these steps:

```bash
# Install dependencies
npm install

# Start development server
npm run dev
```

## API Documentation

API documentation is generated using TypeDoc. To generate the documentation, run:

```bash
# Generate API documentation
npm run docs
```
