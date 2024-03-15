#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// NB: We must include this particular libav header first, before the other
// ones, in order to process this undef that will trigger headers on Ubuntu to
// not drop certain `const` qualifiers (see `ff_const59`)
#include <libavformat/version.h>
#undef FF_API_AVIOFORMAT

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#include "convert.h"

static ssize_t read_all(int fd, void *data, size_t count);
static int64_t convert_input_seek(void *opaque, int64_t offset, int whence);
static int convert_input_read(void *opaque, uint8_t *buf, int buf_size);

static const AVInputFormat *mp4bmff_input_format;
static const AVOutputFormat *mpegts_output_format;

int convert_init()
{
    mp4bmff_input_format = av_find_input_format("mov,mp4,m4a,3gp,3g2,mj2");
    if (mp4bmff_input_format == NULL) {
        return -1;
    }

    mpegts_output_format = av_guess_format("mpegts", NULL, NULL);
    if (mpegts_output_format == NULL) {
        return -1;
    }

    // av_log_set_level(AV_LOG_DEBUG);
    av_log_set_level(AV_LOG_INFO);

    return 0;
}

char *convert_err2str(int errnum)
{
    char *str = av_err2str(errnum);
    return strdup(str);
}

static ssize_t read_all(int fd, void *data, size_t count)
{
    ssize_t nbytes = 0;
    size_t remaining = count;
    uint8_t *start = (uint8_t *) data;
    uint8_t *stop = start + remaining;
    uint8_t *cur = start;
    ssize_t rc = 0;

    while (cur < stop && remaining > 0) {
        rc = read(fd, cur, remaining);
        if (rc < 0) {
            return rc;
        } else if (rc == 0) {
            break;
        }

        cur += rc;
        nbytes += rc;
        remaining -= rc;
    }

    return nbytes;
}

static int convert_input_read(void *opaque, uint8_t *buf, int buf_size)
{
    struct convert *c = (struct convert *)opaque;

    ssize_t rc = read_all(c->input_fd, buf, buf_size);
    if (rc < 0) {
        return AVERROR(rc);
    } else if (rc == 0) {
        return AVERROR_EOF;
    }

    return (int)rc;
}

static int64_t convert_input_seek(void *opaque, int64_t offset, int whence)
{
    struct convert *c = (struct convert *)opaque;

    switch (whence) {
    case AVSEEK_SIZE:
        // NB: return filesize if you can, otherwise -1
        off_t cur = lseek(c->input_fd, 0, SEEK_CUR);
        off_t size = lseek(c->input_fd, 0, SEEK_END);
        lseek(c->input_fd, cur, SEEK_SET);

        return (int64_t)size;

    case SEEK_SET:
        break;

    case SEEK_CUR:
        break;

    case SEEK_END:
        break;

    default:
        return AVERROR(EINVAL);
    }

    off_t rc = lseek(c->input_fd, (off_t)offset, whence);

    return (int64_t)rc;
}

int convert_open(struct convert *c, int input_fd, char *outFilename)
{
    int rc = 0;

    c->input_fd = input_fd;

    c->input_bufsize = 4 << 10;
    if ((c->input_buf = av_malloc(c->input_bufsize)) == NULL) {
        rc = AVERROR(ENOMEM);
        goto _err;
    }

    if ((c->inFmtCtx = avformat_alloc_context()) == NULL) {
        rc = AVERROR(ENOMEM);
        goto _err;
    }

    AVIOContext *ioCtx = avio_alloc_context(
            (unsigned char *) c->input_buf,
            (int) c->input_bufsize,
            0,
            (void *) c,
            &convert_input_read,
            NULL, /* write_packet */
            &convert_input_seek
            );
    if (ioCtx == NULL) {
        rc = AVERROR(ENOMEM);
        goto _err;
    }

    c->inFmtCtx->pb = ioCtx;
    c->inFmtCtx->flags = AVFMT_FLAG_CUSTOM_IO;
    c->inFmtCtx->iformat = mp4bmff_input_format;

    if ((rc = avformat_open_input(&c->inFmtCtx, "file:", mp4bmff_input_format, NULL)) < 0) {
        goto _err;
    }

    if ((rc = avformat_find_stream_info(c->inFmtCtx, NULL)) < 0) {
        goto _err;
    }

    av_dump_format(c->inFmtCtx, 0, "file:", 0 /* input */);
    printf("\n");

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

    av_dump_format(c->outFmtCtx, 0, "file:", 1 /* output */);

    if (!(c->outFmt->flags & AVFMT_NOFILE)) {
        if ((rc = avio_open(&c->outFmtCtx->pb, outFilename, AVIO_FLAG_WRITE)) < 0) {
            goto _err;
        }
    }

    if ((rc = avformat_write_header(c->outFmtCtx, NULL)) < 0) {
        goto _err;
    }

    return 0;

_err:
    if (c->input_buf != NULL) {
        av_free(c->input_buf);
    }

    if (c->inFmtCtx != NULL) {
        avformat_free_context(c->inFmtCtx);
    }

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

    if (c->input_buf) {
        av_free(c->input_buf);
        c->input_buf = NULL;
    }

    rc = 0;

_err:
    return rc;
}
