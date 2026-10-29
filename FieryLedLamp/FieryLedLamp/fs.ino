// ******************************************************************************* fs.ino ***************************************************************
#include "Ota.h"
// --------------

// ===================================================================== ЗАГРУЗКА НАСТРОЕК АВТОРИЗАЦИИ ==================================================

void loadLocalAuthSettings() {
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, configSetup);
  if (error) {
    configSetup = "{}";
    deserializeJson(doc, configSetup);
  }
  JsonObject configObj = doc.as<JsonObject>();

  if (configObj.containsKey("local_auth")) {
    String val = configObj["local_auth"].as<String>();
    val.trim();
    requirePasswordInLocalNetwork = (val == "1" || val.equalsIgnoreCase("true") || val.equalsIgnoreCase("on"));
  } else {
    requirePasswordInLocalNetwork = false;
    jsonWrite(configSetup, "local_auth", "0");
    configChanged = true;
  }
#if GENERAL_LOG
  SYSLOG.add("Запрашивать пароль для редактора: %s", requirePasswordInLocalNetwork ? "да" : "нет");
#endif
}

// ====================================================================== ПУБЛИЧНЫЕ РЕСУРСЫ =============================================================

static const char* public_prefixes[] = {
  "/index.htm", "/index.html", "/index.htm.gz", "/index.html.gz",
  "/update.htm", "/update.html", "/update.htm.gz", "/update.html.gz",
  "/backup.htm", "/backup.html", "/backup.htm.gz", "/backup.html.gz",
  "/logs.htm",  "/logs.html", "/logs.htm.gz", "/logs.html.gz",
  "/setup_ir.htm",  "/setup_ir.html", "/setup_ir.htm.gz", "/setup_ir.html.gz",
  "/favicon.ico",
  "/style.css", "/styles.css",
  "/script.js", "/scripts.js",
  "/css/", "/js/", "/images/", "/fonts/", "/assets/",
  "/ace/", "/vendor/ace/", "/lib/ace/",
  "/monaco/", "/vendor/monaco/",
  "/codemirror/", "/lib/codemirror/",
  "/vendor/", "/lib/", "/modules/",
  nullptr
};

bool isPublicResource(const String &path) {
  String p = path;
  p.toLowerCase();

  if (p == "/edit.htm" || p == "/edit.html" || p == "/edit" || p == "/edit.htm.gz" || p == "/edit.html.gz") {
    return false;
  }

  String checkPath = p;
  if (checkPath.endsWith(".gz")) {
    checkPath = checkPath.substring(0, checkPath.length() - 3);
  }

  for (int i = 0; public_prefixes[i]; i++) {
    String prefix = public_prefixes[i];
    String prefixLower = prefix;
    prefixLower.toLowerCase();
    if (checkPath == "/edit.htm" || checkPath == "/edit.html" || checkPath == "/edit") return false;
  }

  if (checkPath.endsWith(".js") || checkPath.endsWith(".css") || checkPath.endsWith(".json") || checkPath.endsWith(".svg") || checkPath.endsWith(".woff") || checkPath.endsWith(".woff2") || checkPath.endsWith(".png") || checkPath.endsWith(".jpg") || checkPath.endsWith(".ico")) {
    return true;
  }

  return false;
}

// ===================================================================== КЛИЕНТ ЛОКАЛЬНОЙ СЕТИ ==========================================================

bool isLocalClient(const IPAddress &ip) {
  return (ip[0] == 192 && ip[1] == 168) || (ip[0] == 10) || (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31);
}

// ================================================================== АВТОРИЗАЦИЯ ФАЙЛОВОГО МЕНЕДЖЕРА ===================================================

bool requireFileManagerAuth(AsyncWebServerRequest *request, bool allowLocalWithoutPass) {
  if (!request) return false;

  IPAddress clientIP = request->client() ? request->client()->remoteIP() : IPAddress(0, 0, 0, 0);

  if (allowLocalWithoutPass && isLocalClient(clientIP)) {
    if (requirePasswordInLocalNetwork) {
      if (!request->authenticate(FILEMANAGER_USERNAME, FILEMANAGER_PASSWORD)) {
        request->requestAuthentication();
        return false;
      }
    }
    return true;
  }

  if (!request->authenticate(FILEMANAGER_USERNAME, FILEMANAGER_PASSWORD)) {
    request->requestAuthentication();
    return false;
  }
  return true;
}

// ========================================================================== MIME-ТИПЫ =================================================================

String getContentType(const String &filename) {
  if (filename.endsWith(".htm")) return "text/html";
  if (filename.endsWith(".html")) return "text/html";
  if (filename.endsWith(".json")) return "application/json";
  if (filename.endsWith(".css")) return "text/css";
  if (filename.endsWith(".js")) return "application/javascript";
  if (filename.endsWith(".svg")) return "image/svg+xml";
  if (filename.endsWith(".png")) return "image/png";
  if (filename.endsWith(".jpg")) return "image/jpeg";
  if (filename.endsWith(".jpeg")) return "image/jpeg";
  if (filename.endsWith(".gif")) return "image/gif";
  if (filename.endsWith(".ico")) return "image/x-icon";
  if (filename.endsWith(".xml")) return "text/xml";
  if (filename.endsWith(".pdf")) return "application/pdf";
  if (filename.endsWith(".zip")) return "application/zip";
  if (filename.endsWith(".gz")) return "application/gzip";
  if (filename.endsWith(".woff")) return "font/woff";
  if (filename.endsWith(".woff2")) return "font/woff2";
  return "text/plain";
}

// ====================================================================== ПРОВЕРКИ ПУТЕЙ ================================================================

bool isDangerousPath(const String &path) {
  if (path.length() == 0) return true;
  String p = path;
  p.toLowerCase();
  return p.indexOf("..") >= 0 || p.indexOf("\\") >= 0 || p.startsWith("/.") || p.indexOf("/./") >= 0 || p.indexOf("//") >= 0;
}

bool isProtectedFile(const String &path) {
  String p = path;
  p.toLowerCase();
  return p == "/.env" || p == "/.htaccess" || p.endsWith(".key") || p.endsWith(".pem") || p.endsWith(".passwd") || p.endsWith(".sqlite") || p.endsWith(".db") || p.indexOf("/secret/") >= 0 || p.indexOf("/private/") >= 0;
}

bool isForbiddenExtension(const String &filename) {
  String name = filename;
  name.toLowerCase();
  return name.endsWith(".bin") || name.endsWith(".elf") || name.endsWith(".uf2") || name.endsWith(".hex") || name.endsWith(".db") || name.endsWith(".sqlite") || name.endsWith(".key") || name.endsWith(".pem") || name.endsWith(".env");
}

// ====================================================================== РЕКУРСИВНЫЕ ОПЕРАЦИИ ==========================================================

bool removeRecursive(const String &path) {
  File f = LittleFS.open(path);
  if (!f) return false;
  bool isDir = f.isDirectory();
  f.close();

  if (!isDir) {
    return LittleFS.remove(path);
  }

  File dir = LittleFS.open(path);
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return false;
  }

  File entry = dir.openNextFile();
  while (entry) {
    String name = entry.name();
    int lastSlash = name.lastIndexOf('/');
    if (lastSlash >= 0) name = name.substring(lastSlash + 1);

    String fullPath = path;
    if (!fullPath.endsWith("/")) fullPath += "/";
    fullPath += name;

    bool entryIsDir = entry.isDirectory();
    entry.close();

    if (entryIsDir) {
      removeRecursive(fullPath);
    } else {
      LittleFS.remove(fullPath);
    }

    dir.close();
    dir = LittleFS.open(path);
    if (!dir) break;
    entry = dir.openNextFile();
  }
  dir.close();

  String dirPath = path;
  if (dirPath.endsWith("/")) dirPath = dirPath.substring(0, dirPath.length() - 1);
  return LittleFS.rmdir(dirPath);
}

bool copyFile(const String &src, const String &dst) {
  File s = LittleFS.open(src, "r");
  File d = LittleFS.open(dst, "w");
  if (!s || !d) {
    if (s) s.close();
    if (d) d.close();
    return false;
  }

  uint8_t buf[512];
  while (s.available()) {
    size_t n = s.read(buf, sizeof(buf));
    if (n == 0) break;
    d.write(buf, n);
  }
  s.close();
  d.close();
  return true;
}

bool copyDirRecursive(const String &src, const String &dst) {
  if (!LittleFS.mkdir(dst)) return false;

  File dir = LittleFS.open(src);
  if (!dir || !dir.isDirectory()) {
    if (dir) dir.close();
    return false;
  }

  File entry = dir.openNextFile();
  while (entry) {
    String name = entry.name();
    int lastSlash = name.lastIndexOf('/');
    if (lastSlash >= 0) name = name.substring(lastSlash + 1);

    String srcEntry = src + "/" + name;
    String dstEntry = dst + "/" + name;

    bool isDir = entry.isDirectory();
    entry.close();

    if (isDir) {
      copyDirRecursive(srcEntry, dstEntry);
    } else {
      copyFile(srcEntry, dstEntry);
    }
    entry = dir.openNextFile();
  }
  dir.close();
  return true;
}

// ===================================================================== ОТДАЧА ФАЙЛОВ (WLED-style) ====================================================

#define WLED_MIN_FREE_HEAP 8000

bool handleFileRead(AsyncWebServerRequest *request, String path) {
  #if GENERAL_LOG
SYSLOG.add(">>> REQ: %s heap=%u", path.c_str(), ESP.getFreeHeap());
#endif
  if (!request) return false;
  vTaskDelay(pdMS_TO_TICKS(2));

  int q = path.indexOf('?');
  if (q >= 0) path = path.substring(0, q);

  // DRAM + PSRAM
uint32_t totalFree = ESP.getFreeHeap();
if (totalFree < WLED_MIN_FREE_HEAP) {
  vTaskDelay(pdMS_TO_TICKS(50));
  totalFree = ESP.getFreeHeap() + ESP.getFreePsram();
  if (totalFree < WLED_MIN_FREE_HEAP) {
    #if GENERAL_LOG
    SYSLOG.add(">>> 503 LOW MEM: %s heap=%u psram=%u", path.c_str(), ESP.getFreeHeap(), ESP.getFreePsram());
    #endif
    request->send(503, "text/plain", "Low memory");
    return true;
  }
}

  if (isDangerousPath(path)) {
    request->send(400, "text/plain", "Bad path");
    return true;
  }
  if (path == "/" || path.endsWith("/")) path += "index.htm";

  if (!isPublicResource(path)) {
    if (!requireFileManagerAuth(request, true)) return true;
    if (isProtectedFile(path)) {
      request->send(403, "text/plain", "Denied");
      return true;
    }
  }

  // сначала .gz, потом plain
String pathWithGz = path + ".gz";
String finalPath = path;
bool isGz = false;
size_t fileSize = 0;
time_t lastWrite = 0;

File probe = LittleFS.open(pathWithGz, "r");
if (probe) {
  finalPath = pathWithGz;
  isGz = true;
  fileSize = probe.size();
  lastWrite = probe.getLastWrite();
  probe.close();
} else {
  probe = LittleFS.open(path, "r");
  if (!probe) {
#if GENERAL_LOG
    SYSLOG.add(">>> handleFileRead: NOT FOUND %s", path.c_str());
#endif
    return false;
  }
  if (probe.isDirectory()) { probe.close(); return false; }
  fileSize = probe.size();
  lastWrite = probe.getLastWrite();
  probe.close();
}

if (fileSize == 0) {
  request->send(404, "text/plain", "Empty");
  return true;
}

String ct = getContentType(path);

// HEAD (только заголовки, без body)
if (request->method() == HTTP_HEAD) {
  AsyncWebServerResponse *response = request->beginResponse(200, ct, "");
  if (response == nullptr) {
  request->send(503, "text/plain", "Low memory");
  return true;
}
  if (isGz) response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Connection", "close");
  request->send(response);
  #if GENERAL_LOG
  SYSLOG.add(">>> HEAD: %s size=%u", path.c_str(), (unsigned)fileSize);
  #endif
  return true;
}

// GET (стриминг из FS, пик 2-4 KB)
#if GENERAL_LOG
SYSLOG.add(">>> SEND: %s size=%u heap=%u", finalPath.c_str(), (unsigned)fileSize, ESP.getFreeHeap());
#endif

AsyncWebServerResponse *response = request->beginResponse(LittleFS, finalPath, ct);
if (response == nullptr) {
  #if GENERAL_LOG
  SYSLOG.add(">>> 503 NULL RESP: %s heap=%u", path.c_str(), ESP.getFreeHeap());
  #endif
  request->send(503, "text/plain", "Low memory");
  return true;
}
if (isGz) response->addHeader("Content-Encoding", "gzip");
response->addHeader("Connection", "close");
request->send(response);

#if GENERAL_LOG
SYSLOG.add(">>> SENT OK: %s", finalPath.c_str());
#endif
return true;
}

// ========================================================================== ЗАГРУЗКА ФАЙЛОВ ===========================================================

static String uploadPath;
static bool   uploadAuthorized = false;

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!request) return;

  if (index == 0) {
    uploadAuthorized = false;

#if USE_OTA
    if (Ota::instance().isOtaActive()) {
      request->send(503, "text/plain", "OTA in progress, try later");
      return;
    }
#endif

    if (!requireFileManagerAuth(request, true)) {
      return;
    }
    uploadAuthorized = true;

    String fname = filename;
    if (!fname.startsWith("/")) fname = "/" + fname;

    if (fname.length() <= 1 || isDangerousPath(fname)) {
      request->send(400, "text/plain", "Invalid filename");
      uploadAuthorized = false;
      return;
    }
    if (isProtectedFile(fname)) {
      request->send(403, "text/plain", "Protected file");
      uploadAuthorized = false;
      return;
    }
    if (isForbiddenExtension(fname)) {
      request->send(403, "text/plain", "Forbidden file type");
      uploadAuthorized = false;
      return;
    }

    if (fsUploadFile) {
      fsUploadFile.close();
    }

    uploadPath = fname;
    fsUploadFile = LittleFS.open(uploadPath, "w");
    if (!fsUploadFile) {
      request->send(500, "text/plain", "Failed to create file");
      uploadPath = "";
      uploadAuthorized = false;
      return;
    }
  } else {
    if (!uploadAuthorized) {
      return;
    }

    if (len && fsUploadFile) {
      fsUploadFile.write(data, len);
    }

    if (final) {
      if (fsUploadFile) {
        fsUploadFile.close();
        request->send(200, "text/plain", "OK");
      } else {
        request->send(500, "text/plain", "Upload failed");
      }
      uploadPath = "";
      uploadAuthorized = false;
    }
  }
}

// =========================================================================== JSON-UPLOAD ==============================================================

static String jsonUploadPath;

void handleJsonUpload(AsyncWebServerRequest *request, String filename, size_t index,
                      uint8_t *data, size_t len, bool final) {
  if (!request) return;

  if (index == 0) {
#if USE_OTA
    if (Ota::instance().isOtaActive()) {
      request->send(503, "text/plain", "OTA in progress");
      return;
    }
#endif
    if (!requireFileManagerAuth(request, true)) return;

    String fname = filename;
    if (!fname.startsWith("/")) fname = "/" + fname;
    if (isDangerousPath(fname) || isForbiddenExtension(fname) || isProtectedFile(fname)) {
      request->send(403, "text/plain", "Forbidden");
      return;
    }

    if (fsUploadFile) fsUploadFile.close();
    jsonUploadPath = fname;
    fsUploadFile = LittleFS.open(jsonUploadPath, "w");
    if (!fsUploadFile) {
      request->send(500, "text/plain", "Cannot create file");
      jsonUploadPath = "";
    }
    return;
  }

  if (fsUploadFile && len) {
    fsUploadFile.write(data, len);
  }

  if (final && fsUploadFile) {
    fsUploadFile.close();
    request->send(200, "text/plain", "OK");
    jsonUploadPath = "";
  }
}

// ======================================================================= УДАЛЕНИЕ ФАЙЛА/ПАПКИ =========================================================

void handleFileDelete(AsyncWebServerRequest *request) {
  if (!request) return;
  if (!requireFileManagerAuth(request, true)) return;

  if (request->params() == 0) {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }

#if USE_OTA
  if (Ota::instance().isOtaActive()) {
    request->send(503, "text/plain", "OTA in progress");
    return;
  }
#endif

  String path = request->getParam((size_t)0)->value();
  path.trim();

  if (path.length() == 0 || path == "/") {
    request->send(400, "text/plain", "Invalid path");
    return;
  }
  if (isDangerousPath(path)) {
    request->send(400, "text/plain", "Invalid path");
    return;
  }
  if (isProtectedFile(path)) {
    request->send(403, "text/plain", "Protected file");
    return;
  }

  bool isDir = path.endsWith("/");
  String cleanPath = isDir ? path.substring(0, path.length() - 1) : path;

  if (!LittleFS.exists(cleanPath)) {
    request->send(404, "text/plain", "FileNotFound");
    return;
  }

  bool ok = isDir ? removeRecursive(cleanPath) : LittleFS.remove(cleanPath);
  request->send(ok ? 200 : 500, "text/plain", ok ? "Deleted" : "Delete failed");
}

// ====================================================================== СОЗДАНИЕ ФАЙЛА/ПАПКИ ==========================================================

void handleFileCreate(AsyncWebServerRequest *request) {
  if (!request) return;
  if (!requireFileManagerAuth(request, true)) return;

  if (request->params() == 0) {
    request->send(500, "text/plain", "BAD ARGS");
    return;
  }

#if USE_OTA
  if (Ota::instance().isOtaActive()) {
    request->send(503, "text/plain", "OTA in progress");
    return;
  }
#endif

  String path = request->getParam((size_t)0)->value();
  path.trim();

  if (path.length() == 0 || path == "/") {
    request->send(400, "text/plain", "Invalid path");
    return;
  }
  if (isDangerousPath(path)) {
    request->send(400, "text/plain", "Invalid path");
    return;
  }
  if (isProtectedFile(path)) {
    request->send(403, "text/plain", "Protected path");
    return;
  }

  if (path.endsWith("/")) {
    String dirPath = path.substring(0, path.length() - 1);
    if (LittleFS.exists(dirPath)) {
      request->send(409, "text/plain", "Directory already exists");
      return;
    }
    if (LittleFS.mkdir(dirPath)) {
      request->send(200, "text/plain", "Directory created");
    } else {
      request->send(500, "text/plain", "Directory create failed");
    }
    return;
  }

  if (LittleFS.exists(path)) {
    request->send(409, "text/plain", "File already exists");
    return;
  }
  File file = LittleFS.open(path, "w");
  if (file) {
    file.close();
    request->send(200, "text/plain", "Created");
  } else {
    request->send(500, "text/plain", "Create failed");
  }
}

// ====================================================================== СПИСОК ФАЙЛОВ =================================================================

void handleFileList(AsyncWebServerRequest *request) {
  if (!request) return;
  if (!requireFileManagerAuth(request, true)) return;

  if (!request->hasParam("dir")) {
    request->send(400, "text/plain", "Missing dir parameter");
    return;
  }

  String path = request->getParam("dir")->value();
  path.trim();
  if (path.length() == 0) path = "/";

  if (isDangerousPath(path) || isProtectedFile(path)) {
    request->send(403, "text/plain", "Access denied to this directory");
    return;
  }
  if (!path.startsWith("/")) path = "/" + path;

  File root = LittleFS.open(path);
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    request->send(404, "text/plain", "Directory not found");
    return;
  }

  String output = "[";
  File file = root.openNextFile();
  while (file) {
    if (output != "[") output += ',';

    bool isDir = file.isDirectory();
    String name = file.name();
    if (name.startsWith("/")) name = name.substring(1);

    output += F("{\"type\":\"");
    output += isDir ? F("dir") : F("file");
    output += F("\",\"name\":\"");
    output += name;
    output += "\"}";

    file.close();
    file = root.openNextFile();
  }
  output += "]";
  root.close();

  request->send(200, "text/json", output);
}

// ******************************************************************************************************************************************************
