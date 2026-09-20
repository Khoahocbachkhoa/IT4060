// server_poll.c
// A simple multi-client TCP server using poll() on port 8080.
// It accepts multiple clients and prints any data received from each client.
//
// Build:   gcc -std=c11 -Wall -Wextra -O2 server_poll.c -o server_poll
// Run:     ./server_poll
//
// Test:    nc 127.0.0.1 8080   (type messages)
// Notes:
//  - This example is single-threaded and event-driven using poll().
//  - Sockets are set to non-blocking mode to drain read buffers without blocking.
//  - Message boundaries are not preserved in TCP; the server prints data chunks as received.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_PORT 8080
#define MAX_FDS     1024       // Maximum total pollable fds (including the listening socket)
#define BACKLOG     128        // listen() backlog
#define BUFSZ       4096       // Read buffer size
#define POLL_TIMEOUT -1        // Block indefinitely; change to e.g., 1000 for 1s ticks

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int signo) {
    (void)signo;
    g_stop = 1;
}

// Set a file descriptor to non-blocking mode.
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

// Print a human-readable peer address "ip:port" into buf.
static void format_peer_addr(const struct sockaddr_storage *ss, char *buf, size_t buflen) {
    char host[NI_MAXHOST], serv[NI_MAXSERV];
    if (getnameinfo((const struct sockaddr*)ss, (socklen_t)(
                        ss->ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6)),
                    host, sizeof(host), serv, sizeof(serv),
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
        snprintf(buf, buflen, "%s:%s", host, serv);
    } else {
        snprintf(buf, buflen, "unknown:?");
    }
}

// Remove fd at index `i` from arrays by swapping with the last element.
static void remove_fd(struct pollfd fds[], struct sockaddr_storage addrs[],
                      int *nfds, int i) {
    close(fds[i].fd);
    int last = *nfds - 1;
    if (i != last) {
        fds[i]   = fds[last];
        addrs[i] = addrs[last];
    }
    (*nfds)--;
}

int main(void) {
    // Install Ctrl+C handler for graceful shutdown.
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    // Create listening socket (IPv4). For IPv6, duplicate with AF_INET6 if desired.
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    // Allow quick reuse of the port after restart.
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(listen_fd);
        return 1;
    }

    // Bind to 0.0.0.0:8080
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, BACKLOG) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    if (set_nonblocking(listen_fd) < 0) {
        perror("fcntl(O_NONBLOCK) on listen_fd");
        close(listen_fd);
        return 1;
    }

    printf("Server is listening on 0.0.0.0:%d (Ctrl+C to stop)\n", SERVER_PORT);

    // poll() arrays: index 0 is the listening socket; 1..nfds-1 are clients.
    struct pollfd fds[MAX_FDS];
    struct sockaddr_storage addrs[MAX_FDS]; // Peer addresses aligned with fds[]
    int nfds = 1;

    memset(fds, 0, sizeof(fds));
    memset(addrs, 0, sizeof(addrs));

    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    while (!g_stop) {
        int ready = poll(fds, nfds, POLL_TIMEOUT);
        if (ready < 0) {
            if (errno == EINTR) continue; // Interrupted by signal; check g_stop and continue.
            perror("poll");
            break;
        }
        if (ready == 0) {
            // Timeout path (unused when POLL_TIMEOUT == -1). Could do periodic housekeeping here.
            continue;
        }

        // 1) Handle new incoming connections on the listening socket.
        if (fds[0].revents & POLLIN) {
            for (;;) {
                struct sockaddr_storage cli_ss;
                socklen_t cli_len = sizeof(cli_ss);
                int cfd = accept(listen_fd, (struct sockaddr*)&cli_ss, &cli_len);
                if (cfd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // No more to accept right now.
                        break;
                    }
                    if (errno == EINTR) continue;
                    perror("accept");
                    break;
                }

                if (nfds >= MAX_FDS) {
                    // Too many clients; politely refuse.
                    fprintf(stderr, "Too many clients, closing new connection.\n");
                    close(cfd);
                    continue;
                }

                if (set_nonblocking(cfd) < 0) {
                    perror("fcntl(O_NONBLOCK) on client");
                    close(cfd);
                    continue;
                }

                fds[nfds].fd = cfd;
                fds[nfds].events = POLLIN; // Monitor readable events
                addrs[nfds] = cli_ss;

                char who[128];
                format_peer_addr(&cli_ss, who, sizeof(who));
                printf("[+] New connection from %s (fd=%d)\n", who, cfd);

                nfds++;
            }
        }

        // 2) Handle events on client sockets.
        for (int i = 1; i < nfds; ) {
            short ev = fds[i].revents;

            if (ev & (POLLERR | POLLHUP | POLLNVAL)) {
                // Error or hang-up: close and remove this client.
                char who[128];
                format_peer_addr(&addrs[i], who, sizeof(who));
                printf("[-] Disconnect (err/hup) from %s (fd=%d)\n", who, fds[i].fd);
                remove_fd(fds, addrs, &nfds, i);
                continue; // Do not increment i; we swapped in the last entry
            }

            if (ev & POLLIN) {
                // Read as much as available without blocking.
                char buf[BUFSZ];
                int drained = 0;
                for (;;) {
                    ssize_t n = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if (n > 0) {
                        drained = 1;
                        char who[128];
                        format_peer_addr(&addrs[i], who, sizeof(who));
                        // Print the raw chunk as received. Not line-buffered; TCP has no message boundaries.
                        printf("[from %s] %.*s", who, (int)n, buf);
                        // Ensure output is flushed promptly for interactive feel.
                        fflush(stdout);
                        // Optional: echo back to client (disabled)
                        // send(fds[i].fd, buf, (size_t)n, 0);
                    } else if (n == 0) {
                        // Peer performed an orderly shutdown.
                        char who[128];
                        format_peer_addr(&addrs[i], who, sizeof(who));
                        printf("[-] Peer closed connection: %s (fd=%d)\n", who, fds[i].fd);
                        remove_fd(fds, addrs, &nfds, i);
                        goto next_client; // Move to the next index after removal handling
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No more data right now.
                            break;
                        }
                        if (errno == EINTR) continue; // Try again on EINTR
                        // Other errors => close connection.
                        perror("recv");
                        char who[128];
                        format_peer_addr(&addrs[i], who, sizeof(who));
                        printf("[-] Closing %s (fd=%d) due to recv error.\n", who, fds[i].fd);
                        remove_fd(fds, addrs, &nfds, i);
                        goto next_client;
                    }
                }

                // If we reach here without removal, keep the client and continue.
                (void)drained;
            }

            // Move to next client.
            i++;
        next_client:
            ;
        }
    }

    // Cleanup: close all fds.
    for (int i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) close(fds[i].fd);
    }

    printf("Server stopped.\n");
    return 0;
}
