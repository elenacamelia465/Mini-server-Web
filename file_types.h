#ifndef FILE_TYPES_H
#define FILE_TYPES_H


const char* const ERROR = "HTTP/1.1 404 Not found\r\nContent-Type: text/html\r\n\r\n<html><head><title>404 Not found</title></head><body><h1>Not found</h1><p>Your request is malformed or invalid.</p></body></html>";
const char* const DOWNLOAD_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Disposition: attachment\"\r\n\r\n";
const char* const TEXT_HTML = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const char* const TEXT_PLAIN = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
const char* const TEXT_CSS = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\n\r\n";
const char* const TEXT_JS = "HTTP/1.1 200 OK\r\nContent-Type: text/javascript\r\n\r\n";
const char* const TEXT_CSV = "HTTP/1.1 200 OK\r\nContent-Type: text/csv\r\n\r\n";
const char* const APP_JSON = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
const char* const APP_XML = "HTTP/1.1 200 OK\r\nContent-Type: application/xml\r\n\r\n";
const char* const APP_PDF = "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\n\r\n";
const char* const APP_MSWORD = "HTTP/1.1 200 OK\r\nContent-Type: application/msword\r\n\r\n";
const char* const APP_GZIP = "HTTP/1.1 200 OK\r\nContent-Type: application/gzip\r\n\r\n";
const char* const APP_JS_ARCHIVE = "HTTP/1.1 200 OK\r\nContent-Type: application/java-archive\r\n\r\n";
const char* const APP_OGG = "HTTP/1.1 200 OK\r\nContent-Type: application/ogg\r\n\r\n";
const char* const APP_RTF = "HTTP/1.1 200 OK\r\nContent-Type: application/rtf\r\n\r\n";
const char* const APP_X_TAR = "HTTP/1.1 200 OK\r\nContent-Type: application/x-tar\r\n\r\n";
const char* const APP_VND_MSEXCEL = "HTTP/1.1 200 OK\r\nContent-Type: application/vnd.ms-excel\r\n\r\n";
const char* const APP_SPREADSHEET_MSEXCEL = "HTTP/1.1 200 OK\r\nContent-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet\r\n\r\n";
const char* const APP_ZIP = "HTTP/1.1 200 OK\r\nContent-Type: application/zip\r\n\r\n";
const char* const APP_7Z = "HTTP/1.1 200 OK\r\nContent-Type: application/x-7z-compressed\r\n\r\n";
const char* const IMAGE_JPEG = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n";
const char* const IMAGE_GIF = "HTTP/1.1 200 OK\r\nContent-Type: image/gif\r\n\r\n";
const char* const IMAGE_BMP = "HTTP/1.1 200 OK\r\nContent-Type: image/bmp\r\n\r\n";
const char* const IMAGE_WEBP = "HTTP/1.1 200 OK\r\nContent-Type: image/webp\r\n\r\n";
const char* const IMAGE_PNG = "HTTP/1.1 200 OK\r\nContent-Type: image/png\r\n\r\n";
const char* const IMAGE_TIFF = "HTTP/1.1 200 OK\r\nContent-Type: image/tiff\r\n\r\n";
const char* const IMAGE_VND_MSICON = "HTTP/1.1 200 OK\r\nContent-Type: image/vnd.microsoft.icon\r\n\r\n";
const char* const AUDIO_MPEG = "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\n\r\n";
const char* const AUDIO_WAV = "HTTP/1.1 200 OK\r\nContent-Type: audio/wav\r\n\r\n";
const char* const VIDEO_MPEG = "HTTP/1.1 200 OK\r\nContent-Type: video/mpeg\r\n\r\n";
const char* const VIDEO_MP4 = "HTTP/1.1 200 OK\r\nContent-Type: video/mp4\r\n\r\n";
const char* const VIDEO_WEBM = "HTTP/1.1 200 OK\r\nContent-Type: video/webm\r\n\r\n";
const char* const FILE_ALREADY_EXISTS = "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\n\r\n<html><head><title>409 Conflict</title></head><body><h1>Conflict!</h1><p>A file with the same name is already on the server!</p></body></html>";
const char* const UPLOAD_FAILED = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n<html><head><title>500 Internal Server Error</title></head><body><h1>Error!</h1><p>The upload failed due to a server error!</p></body></html>";
const char* const UPLOAD_SUCCESS = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>File Uploaded Successfully!</title></head><body><h1>The file has been uploaded successfully to the server!</h1></body></html>";
const char* const RESOURCE_EXISTING = "HTTP/1.1 204 No Content\r\nContent-Type: text/html\r\n\r\n<html><head><title>The resource is already on the server!</title></head><body><h1>The resource has been updated successfully!</h1></body></html>";
const char* const PUT_SUCCESS = "HTTP/1.1 201 Created\r\nContent-Type: text/html\r\n\r\n<html><head><title>The resource was not present on the server!</title></head><body><h1>The resource has been created successfully!</h1></body></html>";
//const char* const HEAD_SUCCESS = "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n";
const char* const LOGIN_SUCCESS = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: text/html\r\n"
                   "Connection: close\r\n\r\n"
                   "<html><body><h1>Login Successful</h1><p>Welcome, ";
const char* const LOGIN_FAILED =  "HTTP/1.1 401 Unauthorized\r\n"
                   "Content-Type: text/html\r\n"
                   "Connection: close\r\n\r\n"
                   "<html><body><h1>Login Failed</h1><p>Invalid credentials.</p></body></html>";
const char* const DELETE_SUCCESS =  "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html; charset=UTF-8\r\n"
                               "Connection: close\r\n"
                               "\r\n"
                               "<html><body><h1>File Deleted Successfully</h1></body></html>";
const char *const RESPONSE_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";
const char * const HEAD_SUCCESS =  "HTTP/1.1 200 OK\r\n"
            "Content-Length: %ld\r\n"
            "Content-Type: %s\r\n"
            "Last-Modified: %s\r\n"
            "Connection: %s\r\n"
            "\r\n";


#endif // FILE_TYPES_H
