#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct{
    uint16_t width;
    uint16_t height;
    const uint8_t *data;
} ImageType;

extern const ImageType image_tv;
//extern const Image_t img_meihua;
//extern const Image_t img_error;
//extern const Image_t img_wifi;

//extern const Image_t icon_na;
//extern const Image_t icon_duoyun;
//extern const Image_t icon_leizhenyu;
//extern const Image_t icon_qing;
//extern const Image_t icon_wenduji;
//extern const Image_t icon_wifi;
//extern const Image_t icon_yintian;
//extern const Image_t icon_yueliang;
//extern const Image_t icon_zhongxue;
//extern const Image_t icon_zhongyu;

#endif /* __IMAGE_H__ */
