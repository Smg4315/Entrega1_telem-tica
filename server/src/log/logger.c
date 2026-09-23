/* Registro de solicitudes/respuestas relevantes (docx, sección 5). */
#include "../../include/logger.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define LOG_DIR  "logs"
#define LOG_FILE LOG_DIR "/server.log"
#define LOG_LINE_LEN 768

void log_request(const char *node_id, const char *msg_type, const char *result) {
    char stamp[32];
    char line[LOG_LINE_LEN];
    time_t now = time(NULL);
    struct tm tm_now;

    if (mkdir(LOG_DIR, 0755) < 0 && errno != EEXIST) {
        perror("log_request: mkdir");
        return;
    }

    localtime_r(&now, &tm_now);
    strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%S", &tm_now);

    int n = snprintf(line, sizeof(line), "%s pid=%ld node=%s type=%s result=%s\n",
                     stamp, (long)getpid(),
                     (node_id != NULL && node_id[0] != '\0') ? node_id : "-",
                     (msg_type != NULL) ? msg_type : "-",
                     (result != NULL) ? result : "-");
    if (n < 0) {
        return;
    }
    if ((size_t)n >= sizeof(line)) {
        n = (int)sizeof(line) - 1;
        line[n - 1] = '\n'; /* línea truncada, pero siempre terminada */
    }

    int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("log_request: open");
        return;
    }

    if (write(fd, line, (size_t)n) < 0) {
        perror("log_request: write");
    }

    close(fd);
}
