#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct{
    uint16_t width;
    uint16_t height;
    const uint8_t* data;
} ImageType;

extern const ImageType img_meihua;
extern const ImageType img_error;
extern const ImageType img_wifi;

extern const ImageType icon_na;
extern const ImageType icon_duoyun;
extern const ImageType icon_leizhenyu;
extern const ImageType icon_qing;
extern const ImageType icon_wenduji;
extern const ImageType icon_wifi;
extern const ImageType icon_yintian;
extern const ImageType icon_yueliang;
extern const ImageType icon_zhongxue;
extern const ImageType icon_zhongyu;

#endif /* __IMAGE_H__ */
