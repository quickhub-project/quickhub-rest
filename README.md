# quickhub-rest

HTTP/REST server plugin for QuickHub framework.

## Overview

QuickHub-rest provides HTTP and REST API functionality for the QuickHub framework. It includes a complete HTTP server implementation (based on QtWebApp by Stefan Frings) with controllers for resource access, file serving, authentication, and static content.

## Features

### HTTP Server
- Multi-threaded HTTP request handling
- Session management with cookies
- SSL/TLS support (optional)
- Connection pooling
- Static file serving
- Request routing and mapping

### REST Controllers
- **ListController**: RESTful access to list resources
- **ObjectController**: RESTful access to object resources
- **ImageController**: Image resource handling
- **FileController**: File upload/download
- **LoginController**: Authentication endpoints

### Integration
- Token-based authentication with QuickHub core
- Resource manager integration
- WebSocket session coordination

## Requirements

- Qt 6.2 or later
- CMake 3.26 or later
- C++17 compatible compiler
- OpenSSL (optional, for HTTPS support)

## Building

This module is part of the 2log.io project and is built as a submodule:

```bash
# From the main 2log.io directory
mkdir build && cd build
cmake ..
cmake --build . --target QHRest
```

## Qt 6 Migration

As of version 2.0, this module has been migrated to Qt 6. Major changes include:
- Updated to Qt6:: module namespace
- Fixed `QStandardPaths::DataLocation` → `AppLocalDataLocation`
- Changed `QString::SkipEmptyParts` → `Qt::SkipEmptyParts`
- Updated `QMapIterator` → `QMultiMapIterator` for QMultiMap
- Replaced deprecated `QSsl::TlsV1SslV3` → `QSsl::TlsV1_0OrLater`
- Added `Q_INVOKABLE` to `requires()` method
- Added Qt6::Gui dependency for QImage support

## Configuration

The HTTP server can be configured via QSettings. Example configuration:

```ini
[listener]
port=8080
minThreads=1
maxThreads=100
cleanupInterval=1000

[ssl]
sslKeyFile=/path/to/key.pem
sslCertFile=/path/to/cert.pem
```

## API Endpoints

- `GET /login` - Login page
- `POST /login` - Authentication
- `GET|POST|PUT|PATCH|DELETE /lists/{resource}[/{id}]` - List resource operations (see below)
- `GET|POST /objects/{resource}` - Object resource operations
- `GET|POST /images/{resource}` - Image resource operations
- `GET|POST /files/{resource}` - File operations
- `GET /*` - Static file serving

### List API

The `{id}` parameter in list endpoints can be either a **UUID string** or an **integer index**.
If the value is a valid integer it is interpreted as a positional index into the list;
otherwise it is treated as a UUID.

| Method | Path | Description |
|--------|------|-------------|
| GET | `/lists/{resource}` | Get full list |
| GET | `/lists/{resource}/{id}` | Get single item by UUID or index |
| POST | `/lists/{resource}` | Append item (`{"data": ...}`) |
| POST | `/lists/{resource}?index={n}` | Insert item at index |
| PUT | `/lists/{resource}/{id}` | Replace item by UUID or index |
| PATCH | `/lists/{resource}/{id}` | Update item properties by UUID or index |
| DELETE | `/lists/{resource}/{id}` | Remove item by UUID or index |
| DELETE | `/lists/{resource}` | Delete entire list |

## Dependencies

### Qt Modules
- Qt6::Core
- Qt6::Network
- Qt6::WebSockets
- Qt6::Qml
- Qt6::Gui

### Internal Dependencies
- QHCore
- QHPluginSystem
- 2log-core

### Third-Party
- QtWebApp HTTP server by Stefan Frings (LGPL)

## Outputs

- **libQHRest.so** (~1.5 MB) - HTTP/REST server plugin

## Technical Details

- Installed as plugin in `bin/plugins/`
- Uses thread pool for request handling
- Session storage with configurable timeout
- Supports both HTTP and HTTPS
- Request/response cycle managed by QtWebApp

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

### Third-Party Components

**QtWebApp by Stefan Frings** (http://stefanfrings.de/qtwebapp/)
Licensed under LGPL - Used for HTTP server implementation.
