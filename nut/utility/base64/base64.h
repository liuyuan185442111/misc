#ifndef __BASE64_H__
#define __BASE64_H__

#if defined __cplusplus
extern "C" {
#endif
    void base64_encode(void *output, const void *input, unsigned long len);
    unsigned long base64_decode(void *output, const void *input, unsigned long len);
#if defined __cplusplus
};
#endif

#endif
