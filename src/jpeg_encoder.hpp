// //////////////////////////////////////////////////////////
// jpeg.h
// written by Stephan Brumme, 2018
// see https://create.stephan-brumme.com/toojpeg/
//

// This is a compact baseline JPEG/JFIF writer, written in C++ (but looks like
// C for the most part). Its interface has only one function: writeJpeg() -
// and that's it !
//
// basic example:
// => create an image with any content you like, e.g. 1024x768, RGB = 3 bytes
//    per pixel auto pixels = new unsigned char[1024*768*3];
// => you need to define a callback that receives the compressed data
//    byte-by-byte from my JPEG writer
//    void cbOutput(unsigned char oneByte) { fputc(oneByte, myFileHandle); } //
//    //    save byte to file => let's go ! writeJpeg(cbOutput, mypixels, 1024,
//    768);

#pragma once

#include <functional>
#include <cstdint>

namespace JpegEncoder {

/**
 * This callback is called for every byte generated by the encoder and
 * behaves similar to fputc().
 * If you prefer stylish C++11 syntax then it can be a lambda too:
 * auto myCallback = [](uint8_t oneByte) {fputc(oneByte, output); };
 */
typedef std::function<void(uint8_t)> write_byte_cb_t;

// clang-format off
/**
 * Encodes an image into the JPEG  format.
 *
 * @param output       Callback that stores a single byte (writes to disk,
 *                     memory, ...).
 * @param width        Image width in pixels.
 * @param height       Image width in pixels.
 * @param pixels       Stored in RGB format or grayscale, stored from
 *                     upper-left to lower-right.
 * @param isRGB        True if RGB format (3 bytes per pixel); false if
 *                     grayscale (1 byte per pixel).
 * @param quality      Between 1 (worst) and 100 (best).
 * @param downSample   If true then YCbCr 4:2:0 format is used (smaller size,
 *                     minor quality loss) instead of 4:4:4, not relevant
 *                     for grayscale.
 * @param comment      Optional JPEG comment (0/NULL if no comment).
 */
// clang-format on
bool writeJpeg(const write_byte_cb_t& output, const void* pixels,
               unsigned short width, unsigned short height, bool isRGB = true,
               unsigned int quality = 90, bool downSample = false,
               const char* comment = 0);

}  // namespace JpegEncoder