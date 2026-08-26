// ************************************************************************** ФАЙЛОВАЯ СИСТЕМА **********************************************************
#include "Ota.h"
// --------------

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

  // Редактор всегда будет требовать авторизацию
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

    if (checkPath == prefixLower || checkPath.startsWith(prefixLower)) {
      return true;
    }
  }

  if (checkPath.endsWith(".js") || checkPath.endsWith(".css") ||  checkPath.endsWith(".json") || checkPath.endsWith(".svg") ||  checkPath.endsWith(".woff") || checkPath.endsWith(".woff2") || checkPath.endsWith(".png") || checkPath.endsWith(".jpg") || checkPath.endsWith(".ico")) {
    return true;
  }

  return false;
}

bool isLocalClient() {
  IPAddress ip = HTTP.client().remoteIP();
  return (ip[0] == 192 && ip[1] == 168) || (ip[0] == 10) || (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31);
}

bool requireFileManagerAuth(bool allowLocalWithoutPass = true) {
  IPAddress clientIP = HTTP.client().remoteIP();
  String path = HTTP.uri();

  if (isLocalClient()) {
    if (requirePasswordInLocalNetwork) {
      if (!HTTP.authenticate(FILEMANAGER_USERNAME, FILEMANAGER_PASSWORD)) {
        HTTP.requestAuthentication();
        return false;
      }
    }
    return true;
  }

  if (!HTTP.authenticate(FILEMANAGER_USERNAME, FILEMANAGER_PASSWORD)) {
    HTTP.requestAuthentication();
    return false;
  }
  return true;
}

String getContentType(String filename) {
  if (HTTP.hasArg("download")) return F("application/octet-stream");
  else if (filename.endsWith(".htm"))  return F("text/html");
  else if (filename.endsWith(".html")) return F("text/html");
  else if (filename.endsWith(".json")) return F("application/json");
  else if (filename.endsWith(".css"))  return F("text/css");
  else if (filename.endsWith(".js"))   return F("application/javascript");
  else if (filename.endsWith(".svg"))  return F("image/svg+xml");
  else if (filename.endsWith(".png"))  return F("image/png");
  else if (filename.endsWith(".jpg"))  return F("image/jpeg");
  else if (filename.endsWith(".gif"))  return F("image/gif");
  else if (filename.endsWith(".ico"))  return F("image/x-icon");
  else if (filename.endsWith(".xml"))  return F("text/xml");
  else if (filename.endsWith(".pdf"))  return F("application/pdf");
  else if (filename.endsWith(".zip"))  return F("application/zip");
  else if (filename.endsWith(".gz"))   return F("application/gzip");
  return F("text/plain");
}

bool isDangerousPath(const String& path) {
  if (path.length() == 0) return true;
  String p = path;
  p.toLowerCase();
  return p.indexOf("..") >= 0 || p.indexOf("\\") >= 0 || p.startsWith("/.") || p.indexOf("/./") >= 0 || p.indexOf("//") >= 0;
}

bool isProtectedFile(const String& path) {
  String p = path;
  p.toLowerCase();
  if (p == "/.env" || p == "/.htaccess" || p.endsWith(".key") || p.endsWith(".pem") || p.endsWith(".passwd") || p.endsWith(".sqlite") || p.endsWith(".db") || p.indexOf("/secret/") >= 0 || p.indexOf("/private/") >= 0) {
    return true;
  }
  return false;
}

bool isForbiddenExtension(const String& filename) {
  String name = filename;
  name.toLowerCase();
  return name.endsWith(".bin") || name.endsWith(".elf") || name.endsWith(".uf2") || name.endsWith(".hex") || name.endsWith(".db") || name.endsWith(".sqlite") || name.endsWith(".key") || name.endsWith(".pem") || name.endsWith(".env");
}

bool handleFileRead(String path) {
  if (isDangerousPath(path)) {
    HTTP.send(400, F("text/plain"), F("Invalid path"));
    return true;
  }

  if (path.endsWith("/")) path += F("index.htm");
  if (isPublicResource(path)) {
  }
  else {
    if (!requireFileManagerAuth()) {
      return true;
    }

    if (isProtectedFile(path)) {
      HTTP.send(403, F("text/plain"), F("Access denied"));
      return true;
    }
  }

  String pathWithGz = path + ".gz";
  String finalPath = path;

  if (LittleFS.exists(pathWithGz)) {
    finalPath = pathWithGz;
  } else if (!LittleFS.exists(path)) {
    return false;
  }

  File file = LittleFS.open(finalPath, "r");
  if (!file) {
    HTTP.send(500, F("text/plain"), F("Failed to open file"));
    return true;
  }

  HTTP.streamFile(file, getContentType(path));
  file.close();
  return true;
}

void handleFileUpload() {
  if (!requireFileManagerAuth(true)) return;
  if (HTTP.uri() != "/edit") return;
  
  #if USE_OTA
  if (Ota::instance().isOtaActive()) {
    HTTP.send(503, "text/plain", "OTA in progress, try later");
    return;
  }
#endif

  HTTPUpload& upload = HTTP.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    if (filename.length() <= 1 || isDangerousPath(filename)) {
      HTTP.send(400, F("text/plain"), F("Invalid filename"));
      upload.status = UPLOAD_FILE_ABORTED;
      return;
    }

    if (isProtectedFile(filename)) {
      HTTP.send(403, F("text/plain"), F("Protected file"));
      upload.status = UPLOAD_FILE_ABORTED;
      return;
    }
    if (isForbiddenExtension(filename)) {
      HTTP.send(403, F("text/plain"), F("Forbidden file type"));
      upload.status = UPLOAD_FILE_ABORTED;
      return;
    }

    fsUploadFile = LittleFS.open(filename, "w");
    if (!fsUploadFile) {
      HTTP.send(500, F("text/plain"), F("Failed to create file"));
      upload.status = UPLOAD_FILE_ABORTED;
    }
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      HTTP.send(200, F("text/plain"), F("OK"));
    } else {
      HTTP.send(500, F("text/plain"), F("Upload failed"));
    }
  }
  else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (fsUploadFile) {
      fsUploadFile.close();
      LittleFS.remove(upload.filename);
    }
  }
}

void handleJsonUpload() {
  #if USE_OTA
  if (Ota::instance().isOtaActive()) {
    HTTP.send(503, "text/plain", "OTA in progress");
    return;
  }
#endif
  HTTPUpload& upload = HTTP.upload();

  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;

    if (isDangerousPath(filename) || isForbiddenExtension(filename) || isProtectedFile(filename)) {
      HTTP.send(403, "text/plain", "Forbidden");
      upload.status = UPLOAD_FILE_ABORTED;
      return;
    }
    fsUploadFile = LittleFS.open(filename, "w");
  }
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
      HTTP.send(200, "text/plain", "OK");
    }
  }
}

void handleFileDelete() {
  if (!requireFileManagerAuth(true)) return;
  if (HTTP.args() == 0) return HTTP.send(500, F("text/plain"), F("BAD ARGS"));
  
  #if USE_OTA
  if (Ota::instance().isOtaActive()) {
    HTTP.send(503, "text/plain", "OTA in progress");
    return;
  }
#endif

  String path = HTTP.arg(0);
  path.trim();

  if (path.length() == 0 || path == "/") {
    return HTTP.send(400, F("text/plain"), F("Invalid path"));
  }

  if (isDangerousPath(path)) {
    return HTTP.send(400, F("text/plain"), F("Invalid path"));
  }

  if (isProtectedFile(path)) {
    return HTTP.send(403, F("text/plain"), F("Protected file"));
  }

  if (!LittleFS.exists(path)) {
    return HTTP.send(404, F("text/plain"), F("FileNotFound"));
  }

  if (LittleFS.remove(path)) {
    HTTP.send(200, F("text/plain"), F("Deleted"));
  } else {
    HTTP.send(500, F("text/plain"), F("Delete failed"));
  }
}

void handleFileCreate() {
  if (!requireFileManagerAuth(true)) return;
  if (HTTP.args() == 0) {
    return HTTP.send(500, F("text/plain"), F("BAD ARGS"));
  }

  #if USE_OTA
  if (Ota::instance().isOtaActive()) {
    HTTP.send(503, "text/plain", "OTA in progress");
    return;
  }
#endif

  String path = HTTP.arg(0);
  path.trim();

  if (path.length() == 0 || path == "/") {
    return HTTP.send(400, F("text/plain"), F("Invalid path"));
  }

  if (isDangerousPath(path)) {
    return HTTP.send(400, F("text/plain"), F("Invalid path"));
  }

  if (isProtectedFile(path)) {
    return HTTP.send(403, F("text/plain"), F("Protected path"));
  }

  if (LittleFS.exists(path)) {
    return HTTP.send(409, F("text/plain"), F("File already exists"));
  }

  File file = LittleFS.open(path, "w");
  if (file) {
    file.close();
    HTTP.send(200, F("text/plain"), F("Created"));
  } else {
    HTTP.send(500, F("text/plain"), F("Create failed"));
  }
}

void handleFileList() {
  if (!requireFileManagerAuth(true)) return;
  if (!HTTP.hasArg("dir")) {
    HTTP.send(400, F("text/plain"), F("Missing dir parameter"));
    return;
  }
  String path = HTTP.arg("dir");
  path.trim();
  if (path.length() == 0 || path == "/") {
    path = "/";
  }
  if (isDangerousPath(path) || isProtectedFile(path)) {
    HTTP.send(403, F("text/plain"), F("Access denied to this directory"));
    return;
  }
  if (!path.startsWith("/")) {
    path = "/" + path;
  }

  File root = LittleFS.open(path);
  if (!root || !root.isDirectory()) {
    if (root) root.close();
    HTTP.send(404, F("text/plain"), F("Directory not found"));
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

  HTTP.send(200, F("text/json"), output);
}

// ******************************************************************************************************************************************************
