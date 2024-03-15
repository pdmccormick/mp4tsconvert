#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/log.h>

struct convert {
    int     input_fd;
    void*   input_buf;
    size_t  input_bufsize;

    AVFormatContext*        inFmtCtx;
    AVFormatContext*        outFmtCtx;
    const AVOutputFormat*   outFmt;
};

enum convert_loglevel {
    CONVERT_LOG_QUIET   = AV_LOG_QUIET,
    CONVERT_LOG_PANIC   = AV_LOG_PANIC,
    CONVERT_LOG_FATAL   = AV_LOG_FATAL,
    CONVERT_LOG_ERROR   = AV_LOG_ERROR,
    CONVERT_LOG_WARNING = AV_LOG_WARNING,
    CONVERT_LOG_INFO    = AV_LOG_INFO,
    CONVERT_LOG_VERBOSE = AV_LOG_VERBOSE,
    CONVERT_LOG_DEBUG   = AV_LOG_DEBUG,
};

int convert_init(void);
void convert_set_logging(enum convert_loglevel level);
char *convert_err2str(int errnum);
int convert_open(struct convert *c, int input_fd, char *outFilename);
int convert_process_all(struct convert *c);
int convert_close(struct convert *c);
