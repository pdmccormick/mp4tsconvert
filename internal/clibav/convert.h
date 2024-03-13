#pragma once

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

struct convert {
    AVFormatContext     *inFmtCtx;
    AVFormatContext     *outFmtCtx;
    AVOutputFormat      *outFmt;
};

int convert_open(struct convert *c, char *inFilename, char *outFilename);
int convert_process_all(struct convert *c);
int convert_close(struct convert *c);
