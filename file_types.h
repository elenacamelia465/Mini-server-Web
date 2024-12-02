#ifndef FILE_TYPES_H
#define FILE_TYPES_H


const char* const ERROR_HEADER = "HTTP/1.1 404 Not found\r\nContent-Type: text/html\r\n\r\n";
const char* const ERROR_BODY = "<html><head><title>404 Not found</title></head><body><h1>Not found</h1><p>Your request is malformed or invalid.</p></body></html>";

const char* const DOWNLOAD_HEADER = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Disposition: attachment\"\r\n\r\n";

const char *const RESPONSE_NOT_FOUND = "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

const char* const FILE_ALREADY_EXISTS = "HTTP/1.1 409 Conflict\r\nContent-Type: text/html\r\n\r\n<html><head><title>409 Conflict</title></head><body><h1>Conflict!</h1><p>A file with the same name is already on the server!</p></body></html>";

const char* const UPLOAD_FAILED_HEADER = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n";;
const char *const UPLOAD_FAILED_BODY = "<html><head><title>500 Internal Server Error</title></head><body><h1>Error!</h1><p>The upload failed due to a server error!</p></body></html>";

const char* const UPLOAD_SUCCESS = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><title>File Uploaded Successfully!</title></head><body><h1>The file has been uploaded successfully to the server!</h1></body></html>";
const char* const RESOURCE_EXISTING = "HTTP/1.1 204 No Content\r\nContent-Type: text/html\r\n\r\n<html><head><title>The resource is already on the server!</title></head><body><h1>The resource has been updated successfully!</h1></body></html>";
const char* const PUT_SUCCESS = "HTTP/1.1 201 Created\r\nContent-Type: text/html\r\n\r\n<html><head><title>The resource was not present on the server!</title></head><body><h1>The resource has been created successfully!</h1></body></html>";
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

const char * const HEAD_SUCCESS_HEADER =  "HTTP/1.1 200 OK\r\n"
            "Content-Length: %ld\r\n"
            "Content-Type: %s\r\n"
            "Last-Modified: %s\r\n"
            "Connection: %s\r\n"
            "\r\n";

const char* const badRequestResponse_HEADER =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

 const char* const forbiddenResponse_HEADER =
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

const char* const notFoundResponse_HEADER =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

const char* const internalServerErrorResponse_HEADER =
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";



#endif // FILE_TYPES_H
