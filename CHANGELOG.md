# Changelog - quickhub-rest

## [2.0.0] - 2025-10-18

### Major Changes
- **Qt 6 Migration**: Complete migration from Qt 5 to Qt 6
- Updated CMake build system to 3.26+
- C++17 standard compliance

### Fixed
- Updated `QStandardPaths::DataLocation` → `AppLocalDataLocation` in HttpServerPlugin
- Changed `QString::SkipEmptyParts` → `Qt::SkipEmptyParts` in multiple files:
  - RequestMapper.cpp
  - IResourceHttpController.cpp
- Fixed iterator type: `QMapIterator` → `QMultiMapIterator` for QMultiMap usage
- Updated deprecated SSL protocol: `QSsl::TlsV1SslV3` → `QSsl::TlsV1_0OrLater`
- Added `Q_INVOKABLE` to `requires()` method override for C++20 compatibility

### Changed
- Updated all Qt module references to Qt6:: namespace
- Added Qt6::Gui dependency for QImage support
- Modernized CMake configuration with:
  - Proper Qt6 component discovery
  - Version-specific compile definitions
  - Generator expressions for include paths
  - Plugin output directory configuration

### Dependencies
- Added Qt6::Gui to link libraries for image resource support

### Technical Details
- Module builds as shared library: `libQHRest.so` (~1.5 MB)
- Installed as plugin in `bin/plugins/`
- Tracks `upgrade-to-qt6` branch
- Compatible with Qt 6.2+

### Warnings
- Deprecated warning for `QSsl::TlsV1_0OrLater` (suggests using TlsV1_2OrLater)
  - This is a deprecation warning, not an error
  - Can be updated to TlsV1_2OrLater for better security in future versions

### QtWebApp Integration
- QtWebApp HTTP server component successfully migrated to Qt 6
- All HTTP request/response handling updated
- Session management compatible with Qt 6
- Static file controller operational

## [1.0.0] - Previous Release

Initial Qt 5 implementation with:
- HTTP server based on QtWebApp
- REST API controllers
- Static file serving
- Authentication integration
- SSL/TLS support
