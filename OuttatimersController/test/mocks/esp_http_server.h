#pragma once
// Mock ESP HTTP server for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#endif
typedef int esp_err_t;

// HTTP method types
typedef enum {
    HTTP_GET = 1,
    HTTP_POST,
    HTTP_PUT,
    HTTP_PATCH,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS
} httpd_method_t;

// HTTP error codes
#define HTTPD_400_BAD_REQUEST "Bad Request"
#define HTTPD_500_INTERNAL_SERVER_ERROR "Internal Server Error"
#define HTTPD_RESP_USE_STRLEN (-1)

// Forward declarations
typedef struct httpd_req httpd_req_t;
typedef void* httpd_handle_t;

// HTTP request handler function type
typedef esp_err_t (*httpd_uri_handler_t)(httpd_req_t *req);

// HTTP request structure (simplified)
struct httpd_req {
    httpd_handle_t handle;
    httpd_method_t method;
    const char *uri;
    void *user_ctx;
};

// URI handler structure
typedef struct {
    const char *uri;
    httpd_method_t method;
    httpd_uri_handler_t handler;
    void *user_ctx;
} httpd_uri_t;

// HTTP config structure
typedef struct {
    uint16_t server_port;
    uint16_t ctrl_port;
    uint16_t max_uri_handlers;
    uint16_t max_resp_headers;
    uint16_t backlog_conn;
    uint16_t lru_purge_enable;
    uint16_t recv_wait_timeout;
    uint16_t send_wait_timeout;
    size_t max_open_sockets;
    size_t stack_size;
} httpd_config_t;

#define HTTPD_DEFAULT_CONFIG() {    \
    .server_port        = 80,       \
    .ctrl_port          = 32768,    \
    .max_uri_handlers   = 8,        \
    .max_resp_headers   = 8,        \
    .backlog_conn       = 5,        \
    .lru_purge_enable   = false,    \
    .recv_wait_timeout  = 5,        \
    .send_wait_timeout  = 5,        \
    .max_open_sockets   = 7,        \
    .stack_size         = 4096      \
}

// Mock HTTP server functions
static inline esp_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config) {
    *handle = (httpd_handle_t)1; // Fake handle
    return ESP_OK;
}

static inline esp_err_t httpd_stop(httpd_handle_t handle) {
    return ESP_OK;
}

static inline esp_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t *uri_handler) {
    return ESP_OK;
}

static inline esp_err_t httpd_unregister_uri_handler(httpd_handle_t handle, const char *uri, httpd_method_t method) {
    return ESP_OK;
}

static inline esp_err_t httpd_req_get_url_query_str(httpd_req_t *req, char *buf, size_t buf_len) {
    // Mock: return empty query string
    if (buf && buf_len > 0) {
        buf[0] = '\0';
    }
    return ESP_OK;
}

static inline esp_err_t httpd_query_key_value(const char *qry, const char *key, char *val, size_t val_size) {
    // Mock: return not found
    return ESP_FAIL;
}

static inline esp_err_t httpd_resp_set_type(httpd_req_t *req, const char *type) {
    return ESP_OK;
}

static inline esp_err_t httpd_resp_send(httpd_req_t *req, const char *buf, ssize_t buf_len) {
    return ESP_OK;
}

static inline esp_err_t httpd_resp_send_err(httpd_req_t *req, int error, const char *msg) {
    return ESP_OK;
}

static inline esp_err_t httpd_resp_send_500(httpd_req_t *req) {
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif