#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "convert.h"

static AVOutputFormat *mpegts_output_format;

int init_convert()
{
    mpegts_output_format = av_guess_format("mpegts", NULL, NULL);
    if (mpegts_output_format == NULL) {
        return -1;
    }

    return 0;
}

int convert_open(struct convert *c, char *inFilename, char *outFilename)
{
    int rc = 0;

    if ((rc = avformat_open_input(&c->inFmtCtx, inFilename, NULL, NULL)) < 0) {
        goto _err;
    }

    if ((rc = avformat_find_stream_info(c->inFmtCtx, NULL)) < 0) {
        goto _err;
    }

    if ((rc = avformat_alloc_output_context2(&c->outFmtCtx, mpegts_output_format, NULL, NULL)) < 0) {
        goto _err;
    }

    c->outFmt = c->outFmtCtx->oformat;

    int i;
    for (i = 0; i < c->inFmtCtx->nb_streams; i++) {
        AVStream *inStream = c->inFmtCtx->streams[i];

        AVStream *outStream = avformat_new_stream(c->outFmtCtx, NULL);
        if (!outStream) {
            rc = 1;
            goto _err;
        }

        if ((rc = avcodec_parameters_copy(outStream->codecpar, inStream->codecpar)) < 0) {
            goto _err;
        }
    }

    if (!(c->outFmt->flags & AVFMT_NOFILE)) {
        if ((rc = avio_open(&c->outFmtCtx->pb, outFilename, AVIO_FLAG_WRITE)) < 0) {
            goto _err;
        }
    }

    if ((rc = avformat_write_header(c->outFmtCtx, NULL)) < 0) {
        goto _err;
    }

    rc = 0;

_err:
    return rc;
}

int convert_process_all(struct convert *c)
{
    int rc = 0;
    AVPacket packet = {};

    while (av_read_frame(c->inFmtCtx, &packet) >= 0) {
        AVStream *inStream = c->inFmtCtx->streams[packet.stream_index];
        AVStream *outStream = c->outFmtCtx->streams[packet.stream_index];

        packet.pts = av_rescale_q_rnd(packet.pts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.dts = av_rescale_q_rnd(packet.dts, inStream->time_base, outStream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.duration = av_rescale_q(packet.duration, inStream->time_base, outStream->time_base);
        packet.pos = -1;

        if ((rc = av_interleaved_write_frame(c->outFmtCtx, &packet)) < 0) {
            goto _err;
        }

        av_packet_unref(&packet);
    }

    rc = 0;

_err:
    return rc;
}

int convert_close(struct convert *c)
{
    int rc = 0;

    av_write_trailer(c->outFmtCtx);
    avformat_close_input(&c->inFmtCtx);

    if (c->outFmtCtx && !(c->outFmt->flags & AVFMT_NOFILE)) {
        avio_closep(&c->outFmtCtx->pb);
    }

    avformat_free_context(c->outFmtCtx);
    c->outFmtCtx = NULL;

    rc = 0;

_err:
    return rc;
}
