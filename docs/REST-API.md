# QuickHub REST API Documentation

## Overview

The QuickHub REST API provides HTTP access to all core resources of the QuickHub framework:
synchronized lists, key/value objects, services (RPC), image collections, and file uploads.

**Default Port:** `8080`
**Content-Type:** All responses are `application/json` unless binary data is returned (images, files).

---

## Authentication

All endpoints except `/login` require a valid session token.

### Token Sources (checked in order)

| Priority | Method | Example |
|----------|--------|---------|
| 1 | `Authorization` header | `Authorization: Bearer abc123` |
| 2 | Session cookie | Set automatically after `/login` |
| 3 | Query parameter | `?token=abc123` |

The query parameter fallback is useful for embedding resources in HTML tags like `<img src="/images/my.collection/photo1?token=abc123">`.

### Login

```
POST /login
Content-Type: application/json

{"user": "admin", "pass": "secret"}
```

**Response (200):**
```json
{"token": "a1b2c3d4-e5f6-7890-abcd-ef1234567890"}
```

**Error (401):**
```json
{"error": true, "code": 401, "message": "Incorrect password"}
```

### Logout

```
POST /logout
Authorization: Bearer <token>
```

**Response (200):**
```json
{"success": true}
```

---

## Error Format

All errors follow a consistent JSON format:

```json
{
  "error": true,
  "code": 400,
  "message": "Description of what went wrong"
}
```

| Code | Meaning |
|------|---------|
| 400 | Invalid request data / missing parameters |
| 403 | Invalid token or insufficient permissions |
| 404 | Resource, item, or property not found |
| 405 | HTTP method not supported |
| 500 | Internal server error |
| 504 | Service call timed out |

---

## Resource Naming

Resource names in URLs use **dots** as namespace separators. Dots are internally converted to `/`.

| URL Path | Internal Resource Path |
|----------|----------------------|
| `/lists/lab.machines` | `lab/machines` |
| `/objects/config.server.settings` | `config/server/settings` |
| `/images/users.avatars` | `users/avatars` |

---

## Lists (`/lists`)

Synchronized list resources. Each item has a UUID assigned by the server.

### Get Entire List

```
GET /lists/{resource}
Authorization: Bearer <token>
```

**Response (200):** JSON array of all items.

```json
[
  {"uuid": "abc-123", "name": "Item 1", "value": 42},
  {"uuid": "def-456", "name": "Item 2", "value": 17}
]
```

### Get Single Item

```
GET /lists/{resource}/{uuid}
Authorization: Bearer <token>
```

**Response (200):** The item data.

**Response (404):**
```json
{"error": true, "code": 404, "message": "Not found: Item not found"}
```

### Append Item

```
POST /lists/{resource}
Authorization: Bearer <token>
Content-Type: application/json

{"data": {"name": "New Item", "value": 99}}
```

### Insert Item at Index

```
POST /lists/{resource}?index=0
Authorization: Bearer <token>
Content-Type: application/json

{"data": {"name": "First Item"}}
```

### Replace Item

```
PUT /lists/{resource}/{uuid}
Authorization: Bearer <token>
Content-Type: application/json

{"data": {"name": "Replaced Item", "value": 100}}
```

### Update Item Properties

```
PATCH /lists/{resource}/{uuid}
Authorization: Bearer <token>
Content-Type: application/json

{"data": {"name": "Updated Name", "status": "active"}}
```

Only the provided keys are modified. Other properties remain unchanged.

### Remove Item

```
DELETE /lists/{resource}/{uuid}
Authorization: Bearer <token>
```

### Delete Entire List

```
DELETE /lists/{resource}
Authorization: Bearer <token>
```

---

## Objects (`/objects`)

Synchronized key/value objects.

### Get Full Object

```
GET /objects/{resource}
Authorization: Bearer <token>
```

**Response (200):**
```json
{"name": "Server Config", "port": 8080, "debug": false}
```

### Get Single Property

```
GET /objects/{resource}/{property}
Authorization: Bearer <token>
```

**Response (200):** The property value as JSON.

```json
8080
```

**Response (404):**
```json
{"error": true, "code": 404, "message": "Not found: Property not found"}
```

### Set Single Property

```
PUT /objects/{resource}/{property}
Authorization: Bearer <token>
Content-Type: application/json

{"data": 9090}
```

### Set Multiple Properties

```
PATCH /objects/{resource}
Authorization: Bearer <token>
Content-Type: application/json

{"data": {"port": 9090, "debug": true}}
```

---

## Services (`/services`)

Remote Procedure Calls (RPC). Service calls are internally asynchronous but the REST endpoint waits synchronously for the response (timeout: 30 seconds).

### List All Services

```
GET /services
Authorization: Bearer <token>
```

**Response (200):**
```json
[
  {"name": "deviceService", "methods": ["getDevices", "triggerFunction"]},
  {"name": "labService", "methods": ["getStats", "resetCounter"]}
]
```

### Get Service Info

```
GET /services/{name}
Authorization: Bearer <token>
```

**Response (200):**
```json
{"name": "deviceService", "methods": ["getDevices", "triggerFunction"]}
```

**Response (404):**
```json
{"error": true, "code": 404, "message": "Not found: Service not found: unknownService"}
```

### Call Service Method

```
POST /services/{name}/{method}
Authorization: Bearer <token>
Content-Type: application/json

{"deviceId": "my-device-001", "action": "restart"}
```

**Response (200):** The return value of the service method (structure depends on the service).

```json
{"result": "ok", "timestamp": 1700000000}
```

**Response (404):** Service or method not found.

**Response (504):**
```json
{"error": true, "code": 504, "message": "Service call timed out"}
```

---

## Images (`/images`)

Image collection resources. Images are stored and served as PNG.

### List All Images

```
GET /images/{resource}
Authorization: Bearer <token>
```

**Response (200):**
```json
{
  "ids": ["abc123", "def456"],
  "metadata": {
    "abc123": {"originalName": "photo.jpg"},
    "def456": {"originalName": "logo.png"}
  }
}
```

### Upload Image

```
POST /images/{resource}
Authorization: Bearer <token>
Content-Type: multipart/form-data

file=@photo.jpg
```

The multipart key must be `file`.

### Get Image

```
GET /images/{resource}/{id}
Authorization: Bearer <token>
```

**Response:** Binary image data with `Content-Type: image/png`.

For HTML embeds, use the token query parameter:
```html
<img src="/images/users.avatars/abc123?token=mytoken">
```

### Delete Image

```
DELETE /images/{resource}/{id}
Authorization: Bearer <token>
```

---

## Files (`/files`)

General-purpose file upload and download with persistent storage.

Files are stored in a configurable directory (environment variable `FILE_UPLOAD_DIR`, default: `<AppLocalDataLocation>/uploads`). Each file gets a UUID-based filename with a JSON sidecar file for metadata.

### Upload File

```
POST /files
Authorization: Bearer <token>
Content-Type: multipart/form-data

file=@document.pdf
```

The multipart key must be `file`.

**Response (201):**
```json
{
  "id": "a1b2c3d4-e5f6-7890-abcd-ef1234567890",
  "filename": "document.pdf",
  "mimeType": "application/pdf",
  "url": "/files/a1b2c3d4-e5f6-7890-abcd-ef1234567890"
}
```

### Download File

```
GET /files/{id}
Authorization: Bearer <token>
```

**Response:** Binary file data with correct `Content-Type` header (detected via `QMimeDatabase`).

For HTML embeds:
```html
<a href="/files/abc123?token=mytoken">Download</a>
<video src="/files/abc123?token=mytoken"></video>
```

### Delete File

```
DELETE /files/{id}
Authorization: Bearer <token>
```

**Response (200):**
```json
{"error": false}
```

---

## curl Examples

### Complete Workflow

```bash
# Login
TOKEN=$(curl -s -X POST http://localhost:8080/login \
  -H "Content-Type: application/json" \
  -d '{"user":"admin","pass":"secret"}' | jq -r '.token')

# Create a list item
curl -X POST http://localhost:8080/lists/my.items \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"data": {"name": "Test", "value": 42}}'

# Get the list
curl http://localhost:8080/lists/my.items \
  -H "Authorization: Bearer $TOKEN"

# Update a property
curl -X PATCH http://localhost:8080/lists/my.items/ITEM_UUID \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"data": {"value": 100}}'

# Set an object property
curl -X PUT http://localhost:8080/objects/my.config/debug \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"data": true}'

# Call a service
curl -X POST http://localhost:8080/services/deviceService/getDevices \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{}'

# Upload an image
curl -X POST http://localhost:8080/images/my.gallery \
  -H "Authorization: Bearer $TOKEN" \
  -F "file=@photo.jpg"

# Upload a file
curl -X POST http://localhost:8080/files \
  -H "Authorization: Bearer $TOKEN" \
  -F "file=@document.pdf"

# Logout
curl -X POST http://localhost:8080/logout \
  -H "Authorization: Bearer $TOKEN"
```

---

## Thread Safety

All resource operations are executed thread-safely. HTTP requests arrive on worker threads from the connection handler pool, while resources live on the main thread. The REST controllers use `invokeOnResourceThread()` with `Qt::BlockingQueuedConnection` to marshal calls to the correct thread.

---

## OpenAPI Specification

The complete OpenAPI 3.0.3 specification is available at [`docs/openapi.yaml`](openapi.yaml). It can be imported into tools like Swagger UI, Postman, or Insomnia for interactive API exploration.
