/*
 *  Copyright (C) 2011 Savoir-Faire Linux Inc.
 *  Author: Rafaël Carré <rafael.carre@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include <string>
#include <cstring> // for memset
#include <vector>
#include <climits>
#include <stdexcept>

#include "logger.h"

extern "C" {
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
}

#include "video_v4l2.h"

namespace sfl_video
{
using std::vector;
using std::string;

static const unsigned pixelformats_supported[] = {
    /* pixel format        depth  description   */

    /* prefered formats, they can be fed directly to the video encoder */
    V4L2_PIX_FMT_YUV420,   /* 12  YUV 4:2:0     */
    V4L2_PIX_FMT_YUV422P,  /* 16  YVU422 planar */
    V4L2_PIX_FMT_YUV444,   /* 16  xxxxyyyy uuuuvvvv */

    /* Luminance+Chrominance formats */
    V4L2_PIX_FMT_YVU410,   /*  9  YVU 4:1:0     */
    V4L2_PIX_FMT_YVU420,   /* 12  YVU 4:2:0     */
    V4L2_PIX_FMT_YUYV,     /* 16  YUV 4:2:2     */
    V4L2_PIX_FMT_YYUV,     /* 16  YUV 4:2:2     */
    V4L2_PIX_FMT_YVYU,     /* 16 YVU 4:2:2 */
    V4L2_PIX_FMT_UYVY,     /* 16  YUV 4:2:2     */
    V4L2_PIX_FMT_VYUY,     /* 16  YUV 4:2:2     */
    V4L2_PIX_FMT_YUV411P,  /* 16  YVU411 planar */
    V4L2_PIX_FMT_Y41P,     /* 12  YUV 4:1:1     */
    V4L2_PIX_FMT_YUV555,   /* 16  YUV-5-5-5     */
    V4L2_PIX_FMT_YUV565,   /* 16  YUV-5-6-5     */
    V4L2_PIX_FMT_YUV32,    /* 32  YUV-8-8-8-8   */
    V4L2_PIX_FMT_YUV410,   /*  9  YUV 4:1:0     */
    V4L2_PIX_FMT_HI240,    /*  8  8-bit color   */
    V4L2_PIX_FMT_HM12,     /*  8  YUV 4:2:0 16x16 macroblocks */

    /* two planes -- one Y, one Cr + Cb interleaved  */
    V4L2_PIX_FMT_NV12,     /* 12  Y/CbCr 4:2:0  */
    V4L2_PIX_FMT_NV21,     /* 12  Y/CrCb 4:2:0  */
    V4L2_PIX_FMT_NV16,     /* 16  Y/CbCr 4:2:2  */
    V4L2_PIX_FMT_NV61,     /* 16  Y/CrCb 4:2:2  */

#if 0
    /* RGB formats */
    V4L2_PIX_FMT_RGB332,   /*  8  RGB-3-3-2     */
    V4L2_PIX_FMT_RGB444,   /* 16  xxxxrrrr ggggbbbb */
    V4L2_PIX_FMT_RGB555,   /* 16  RGB-5-5-5     */
    V4L2_PIX_FMT_RGB565,   /* 16  RGB-5-6-5     */
    V4L2_PIX_FMT_RGB555X,  /* 16  RGB-5-5-5 BE  */
    V4L2_PIX_FMT_RGB565X,  /* 16  RGB-5-6-5 BE  */
    V4L2_PIX_FMT_BGR666,   /* 18  BGR-6-6-6     */
    V4L2_PIX_FMT_BGR24,    /* 24  BGR-8-8-8     */
    V4L2_PIX_FMT_RGB24,    /* 24  RGB-8-8-8     */
    V4L2_PIX_FMT_BGR32,    /* 32  BGR-8-8-8-8   */
    V4L2_PIX_FMT_RGB32,    /* 32  RGB-8-8-8-8   */

    /* Grey formats */
    V4L2_PIX_FMT_GREY,     /*  8  Greyscale     */
    V4L2_PIX_FMT_Y4,       /*  4  Greyscale     */
    V4L2_PIX_FMT_Y6,       /*  6  Greyscale     */
    V4L2_PIX_FMT_Y10,      /* 10  Greyscale     */
    V4L2_PIX_FMT_Y16,      /* 16  Greyscale     */

    /* Palette formats */
    V4L2_PIX_FMT_PAL8,     /*  8  8-bit palette */
#endif
};

/* Returns a score for the given pixelformat
 *
 * Lowest score is the best, the first entries in the array are the formats
 * supported as an input for the video encoders.
 *
 * Other entries in the array are YUV formats
 *
 * RGB / grey / palette formats are not supported because most cameras support
 * YUV input
 *
 */

namespace {
unsigned int pixelformat_score(unsigned pixelformat)
{
    size_t n = sizeof pixelformats_supported / sizeof *pixelformats_supported;
    for (unsigned int i = 0; i < n ; ++i)
        if (pixelformats_supported[i] == pixelformat)
            return i;

    return UINT_MAX - 1;
}
}

VideoV4l2Size::VideoV4l2Size(unsigned height, unsigned width) :
    height(height), width(width), rates_() {}

vector<string> VideoV4l2Size::getRateList()
{
    vector<string> v;

    for (vector<float>::const_iterator i = rates_.begin() ; i != rates_.end(); ++i) {
        std::stringstream ss;
        ss << *i;
        v.push_back(ss.str());
    }

    return v;
}

void VideoV4l2Size::getFrameRates(int fd, unsigned int pixel_format)
{
    v4l2_frmivalenum frmival;
    memset(&frmival, 0x0, sizeof frmival);
    frmival.pixel_format = pixel_format;
    frmival.width = width;
    frmival.height = height;

    if (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival)) {
        rates_.push_back(25);
        ERROR("could not query frame interval for size");
        return;
    }

    switch(frmival.type) {
        case V4L2_FRMIVAL_TYPE_DISCRETE:
            do {
                rates_.push_back(frmival.discrete.denominator/frmival.discrete.numerator);
                ++frmival.index;
            } while (!ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival));
            break;
        case V4L2_FRMIVAL_TYPE_CONTINUOUS:
            rates_.push_back(25);
            // TODO
            ERROR("Continuous Frame Intervals not supported");
            break;
        case V4L2_FRMIVAL_TYPE_STEPWISE:
            rates_.push_back(25);
            // TODO
            ERROR("Stepwise Frame Intervals not supported");
            break;
    }
}

VideoV4l2Channel::VideoV4l2Channel(unsigned idx, const char *s) :
    idx(idx), name(s), sizes_() {}

void VideoV4l2Channel::setFourcc(unsigned code)
{
    fourcc_[0] = code;
    fourcc_[1] = code >> 8;
    fourcc_[2] = code >> 16;
    fourcc_[3] = code >> 24;
    fourcc_[4] = '\0';
}

const char *
VideoV4l2Channel::getFourcc() const
{
    return fourcc_;
}

vector<string> VideoV4l2Channel::getSizeList() const
{
    vector<string> v;

    for (size_t i = 0 ; i < sizes_.size(); ++i) {
        std::stringstream ss;
        ss << sizes_[i].width << "x" << sizes_[i].height;
        v.push_back(ss.str());
    }

    return v;
}

unsigned int
VideoV4l2Channel::getSizes(int fd, unsigned int pixelformat)
{
    v4l2_frmsizeenum frmsize;
    memset(&frmsize, 0x0, sizeof frmsize);
    frmsize.index = 0;
    frmsize.pixel_format = pixelformat;
    if (!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize)) {

        switch (frmsize.type) {
            case V4L2_FRMSIZE_TYPE_DISCRETE:
                do {
                    VideoV4l2Size size(frmsize.discrete.height, frmsize.discrete.width);
                    size.getFrameRates(fd, frmsize.pixel_format);
                    sizes_.push_back(size);
                    ++frmsize.index;
                } while (!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize));
                return pixelformat;

                // TODO, we dont want to display a list of 2000x2000
                // resolutions if the camera supports continuous framesizes
                // from 1x1 to 2000x2000
                // We should limit to a list of known standard sizes
            case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                ERROR("Continuous Frame sizes not supported");
                break;
            case V4L2_FRMSIZE_TYPE_STEPWISE:
                ERROR("Stepwise Frame sizes not supported");
                break;
        }
    }

    v4l2_format fmt;
    memset(&fmt, 0x0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) < 0)
        throw std::runtime_error("Could not get format");

    VideoV4l2Size size(fmt.fmt.pix.height, fmt.fmt.pix.width);
    size.getFrameRates(fd, fmt.fmt.pix.pixelformat);
    sizes_.push_back(size);

    return fmt.fmt.pix.pixelformat;
}

void VideoV4l2Channel::getFormat(int fd)
{
    if (ioctl(fd, VIDIOC_S_INPUT, &idx))
        throw std::runtime_error("VIDIOC_S_INPUT failed");

    v4l2_fmtdesc fmt;
    memset(&fmt, 0x0, sizeof fmt);
    unsigned fmt_index;
    fmt.index = fmt_index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    unsigned int best_score = UINT_MAX;
    unsigned int best_idx = 0;
    unsigned int pixelformat = 0;
    while (!ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) {
        if (fmt_index != fmt.index)
            break;

        unsigned int score = pixelformat_score(fmt.pixelformat);
        if (score < best_score) {
            pixelformat = fmt.pixelformat;
            best_idx = fmt_index;
            best_score = score;
        }

        fmt.index = ++fmt_index;
    }
    if (fmt_index == 0)
        throw std::runtime_error("Could not enumerate formats");

    fmt.index = best_idx;
    pixelformat = getSizes(fd, pixelformat);

    setFourcc(pixelformat);
}

VideoV4l2Size VideoV4l2Channel::getSize(const string &name) const
{
    for (vector<VideoV4l2Size>::const_iterator i = sizes_.begin(); i != sizes_.end(); ++i) {
        std::stringstream ss;
        ss << i->width << "x" << i->height;
        if (ss.str() == name)
            return *i;
    }

    // fallback to last size
    return sizes_.back();
}


VideoV4l2Device::VideoV4l2Device(int fd, const string &device) :
    device(device), name(), channels()
{
    v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap))
        throw std::runtime_error("could not query capabilities");

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        throw std::runtime_error("not a capture device");

    name = string(reinterpret_cast<const char*>(cap.card));

    v4l2_input input;
    memset(&input, 0x0, sizeof input);
    unsigned idx;
    input.index = idx = 0;
    while (!ioctl(fd, VIDIOC_ENUMINPUT, &input)) {
        if (idx != input.index)
            break;

        if (input.type & V4L2_INPUT_TYPE_CAMERA) {
            VideoV4l2Channel channel(idx, (const char*) input.name);
            channel.getFormat(fd);
            channels.push_back(channel);
        }

        input.index = ++idx;
    }
}

vector<string> VideoV4l2Device::getChannelList() const
{
    vector<string> v;

    const size_t n = channels.size();
    for (size_t i = 0 ; i < n ; ++i)
        v.push_back(channels[i].name);

    return v;
}

const VideoV4l2Channel &VideoV4l2Device::getChannel(const string &name) const
{
    for (size_t i = 0; i < channels.size(); ++i)
        if (channels[i].name == name)
            return channels[i];

    return channels.back();
}

} // end namespace sfl