#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sys/socket.h"
#include "netdb.h"
#include "errno.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <string>
#include "../control/command.h"

// Bazowane na https://github.com/espressif/esp-idf/blob/v6.0.2/examples/protocols/sockets/non_blocking/main/non_blocking_socket_example.c

#define INVALID_SOCK (-1)

#define YIELD_TO_ALL_MS 50

#define SERVER_BIND_PORT "2000"


static void log_socket_error(const char *tag, const int sock, const int err, const char *message) {
    ESP_LOGE(tag, "[sock=%d]: %s\n"
                  "error=%d: %s", sock, message, err, strerror(err));
}


static int try_receive(const char *tag, const int sock, char * data, size_t max_len) {
    int len = recv(sock, data, max_len, 0);
    if (len < 0) {
        if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;   // Not an error
        }
        if (errno == ENOTCONN) {
            ESP_LOGW(tag, "[sock=%d]: Connection closed", sock);
            return -2;  // Socket has been disconnected
        }
        log_socket_error(tag, sock, errno, "Error occurred during receiving");
        return -1;
    }

    return len;
}


static int socket_send(const char *tag, const int sock, const char * data, const size_t len) {
    int to_write = len;
    while (to_write > 0) {
        int written = send(sock, data + (len - to_write), to_write, 0);
        if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK) {
            log_socket_error(tag, sock, errno, "Error occurred during sending");
            return -1;
        }
        to_write -= written;
    }
    return len;
}


/**
 * @brief Returns the string representation of client's address (accepted on this server)
 */
static inline char* get_clients_address(struct sockaddr_storage *source_addr)
{
    static char address_str[128];
    char *res = NULL;
    // Convert ip address to string
    if (source_addr->ss_family == PF_INET) {
        res = inet_ntoa_r(((struct sockaddr_in *)source_addr)->sin_addr, address_str, sizeof(address_str) - 1);
    }
#ifdef CONFIG_LWIP_IPV6
    else if (source_addr->ss_family == PF_INET6) {
        res = inet6_ntoa_r(((struct sockaddr_in6 *)source_addr)->sin6_addr, address_str, sizeof(address_str) - 1);
    }
#endif
    if (!res) {
        address_str[0] = '\0'; // Returns empty string if conversion didn't succeed
    }
    return address_str;
}

typedef struct {
    SemaphoreHandle_t* server_ready;
    DriverState* state;
} TPCTaskInput;

static void tcp_server_task(void *pvParameters) {
    static char rx_buffer[128];
    static const char *TAG = "text-over-tpc";
    SemaphoreHandle_t *server_ready = static_cast<TPCTaskInput* >(pvParameters)->server_ready;
    DriverState* state = static_cast<TPCTaskInput* >(pvParameters)->state;
    pvParameters = NULL;
    struct addrinfo hints{};
    
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *address_info;
    int listen_sock = INVALID_SOCK;
    const size_t max_socks = CONFIG_LWIP_MAX_SOCKETS / 2 - 1;
    static int sock[CONFIG_LWIP_MAX_SOCKETS / 2 - 1];
    static char line[CONFIG_LWIP_MAX_SOCKETS / 2 - 1][512];
    static int index[CONFIG_LWIP_MAX_SOCKETS / 2 - 1];

    int flags;
    int err;
    int new_sock_index;
    socklen_t  addr_len;
    // Prepare a list of file descriptors to hold client's sockets, mark all of them as invalid, i.e. available
    for (int i=0; i<max_socks; ++i) {
        sock[i] = INVALID_SOCK;
    }

    // Translating the hostname or a string representation of an IP to address_info
    int res = getaddrinfo("0.0.0.0", SERVER_BIND_PORT, &hints, &address_info);
    if (res != 0 || address_info == NULL) {
        goto error;
    }

    // Creating a listener socket
    listen_sock = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);

    if (listen_sock < 0) {
        log_socket_error(TAG, listen_sock, errno, "Unable to create socket");
        goto error;
    }
    ESP_LOGI(TAG, "Listener socket created");

    // Marking the socket as non-blocking
    flags = fcntl(listen_sock, F_GETFL);
    if (fcntl(listen_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        log_socket_error(TAG, listen_sock, errno, "Unable to set socket non blocking");
        goto error;
    }
    ESP_LOGI(TAG, "Socket marked as non blocking");

    // Binding socket to the given address
    err = bind(listen_sock, address_info->ai_addr, address_info->ai_addrlen);
    if (err != 0) {
        log_socket_error(TAG, listen_sock, errno, "Socket unable to bind");
        goto error;
    }
    ESP_LOGI(TAG, "Socket bound on %s:%s", "0.0.0.0", SERVER_BIND_PORT);

    // Set queue (backlog) of pending connections to one (can be more)
    err = listen(listen_sock, 1);
    if (err != 0) {
        log_socket_error(TAG, listen_sock, errno, "Error occurred during listen");
        goto error;
    }
    ESP_LOGI(TAG, "Socket listening");
    xSemaphoreGive(*server_ready);

    // Main loop for accepting new connections and serving all connected clients
    while (1) {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        addr_len = sizeof(source_addr);

        // Find a free socket
        for (new_sock_index=0; new_sock_index<max_socks; ++new_sock_index) {
            if (sock[new_sock_index] == INVALID_SOCK) {
                break;
            }
        }

        // We accept a new connection only if we have a free socket
        if (new_sock_index < max_socks) {
            // Try to accept a new connections
            sock[new_sock_index] = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
            index[new_sock_index] = 0;

            if (sock[new_sock_index] < 0) {
                if (errno == EWOULDBLOCK) { // The listener socket did not accepts any connection
                                            // continue to serve open connections and try to accept again upon the next iteration
                    ESP_LOGV(TAG, "No pending connections...");
                } else {
                    log_socket_error(TAG, listen_sock, errno, "Error when accepting connection");
                    goto error;
                }
            } else {
                // We have a new client connected -> print it's address
                ESP_LOGI(TAG, "[sock=%d]: Connection accepted from IP:%s", sock[new_sock_index], get_clients_address(&source_addr));

                // ...and set the client's socket non-blocking
                flags = fcntl(sock[new_sock_index], F_GETFL);
                if (fcntl(sock[new_sock_index], F_SETFL, flags | O_NONBLOCK) == -1) {
                    log_socket_error(TAG, sock[new_sock_index], errno, "Unable to set socket non blocking");
                    goto error;
                }
                ESP_LOGI(TAG, "[sock=%d]: Socket marked as non blocking", sock[new_sock_index]);
            }
        }

        // We serve all the connected clients in this loop
        for (int i=0; i<max_socks; ++i) {
            if (sock[i] != INVALID_SOCK) {
                // This is an open socket -> try to serve it
                int len = try_receive(TAG, sock[i], rx_buffer, sizeof(rx_buffer));
                if (len < 0) {
                    // Error occurred within this client's socket -> close and mark invalid
                    ESP_LOGI(TAG, "[sock=%d]: try_receive() returned %d -> closing the socket", sock[i], len);
                    close(sock[i]);
                    sock[i] = INVALID_SOCK;
                } else if (len > 0) {
                    int a = -1;

                    while(a < len) {
                        for (a++; a < len; a++) {
                            char c = rx_buffer[a];

                            if (c == '\r') continue;
                            
                            if (c == '\n' || index[i] >= 500) {
                                break;
                            }

                            if (c == '\b') {
                                if (index[i] > 0) {
                                    index[i]--;
                                }
                                continue;
                            }

                            line[i][index[i]++] = c;
                        }

                        if (index[i] == 0) {
                            continue;
                        }

                        line[i][index[i]] = 0;
                        
                        parseAndExecuteMulti(state, [i](std::string text) {
                            socket_send(TAG, sock[i], (text + "\n").c_str(), text.length() + 1);
                        }, std::string(line[i]));
                        index[i] = 0;
                    }

                    ESP_LOGI(TAG, "[sock=%d]: Received %.*s", sock[i], len, rx_buffer);
                } // one client's socket
            } // for all sockets

            // Yield to other tasks
            vTaskDelay(pdMS_TO_TICKS(YIELD_TO_ALL_MS));
        }
    }

error:
    if (listen_sock != INVALID_SOCK) {
        close(listen_sock);
    }

    for (int i=0; i<max_socks; ++i) {
        if (sock[i] != INVALID_SOCK) {
            close(sock[i]);
        }
    }

    free(address_info);
    vTaskDelete(NULL);
}


void setup_textovertpc(DriverState* state) {
    SemaphoreHandle_t server_ready = xSemaphoreCreateBinary();
    TPCTaskInput input = {&server_ready, state};
    assert(server_ready);
    xTaskCreate(tcp_server_task, "tcp_server", 4096, &input, 5, NULL);
    xSemaphoreTake(server_ready, portMAX_DELAY);
    vSemaphoreDelete(server_ready);
}