#pragma once

#include <stddef.h>
#include <stdint.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

struct convert {
    int     input_fd;
    void*   input_buf;
    size_t  input_bufsize;

    AVFormatContext*        inFmtCtx;
    AVFormatContext*        outFmtCtx;
    const AVOutputFormat*   outFmt;
};

int convert_init(void);
char *convert_err2str(int errnum);
int convert_open(struct convert *c, int input_fd, char *outFilename);
int convert_process_all(struct convert *c);
int convert_close(struct convert *c);
