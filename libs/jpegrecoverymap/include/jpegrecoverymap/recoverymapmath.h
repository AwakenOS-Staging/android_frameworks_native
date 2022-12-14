/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_JPEGRECOVERYMAP_RECOVERYMAPMATH_H
#define ANDROID_JPEGRECOVERYMAP_RECOVERYMAPMATH_H

#include <stdint.h>

#include <jpegrecoverymap/recoverymap.h>

namespace android::recoverymap {

////////////////////////////////////////////////////////////////////////////////
// Framework

const float kSdrWhiteNits = 100.0f;
const float kHlgMaxNits = 1000.0f;
const float kPqMaxNits = 10000.0f;

struct Color {
  union {
    struct {
      float r;
      float g;
      float b;
    };
    struct {
      float y;
      float u;
      float v;
    };
  };
};

typedef Color (*ColorTransformFn)(Color);
typedef float (*ColorCalculationFn)(Color);

inline Color operator+=(Color& lhs, const Color& rhs) {
  lhs.r += rhs.r;
  lhs.g += rhs.g;
  lhs.b += rhs.b;
  return lhs;
}
inline Color operator-=(Color& lhs, const Color& rhs) {
  lhs.r -= rhs.r;
  lhs.g -= rhs.g;
  lhs.b -= rhs.b;
  return lhs;
}

inline Color operator+(const Color& lhs, const Color& rhs) {
  Color temp = lhs;
  return temp += rhs;
}
inline Color operator-(const Color& lhs, const Color& rhs) {
  Color temp = lhs;
  return temp -= rhs;
}

inline Color operator+=(Color& lhs, const float rhs) {
  lhs.r += rhs;
  lhs.g += rhs;
  lhs.b += rhs;
  return lhs;
}
inline Color operator-=(Color& lhs, const float rhs) {
  lhs.r -= rhs;
  lhs.g -= rhs;
  lhs.b -= rhs;
  return lhs;
}
inline Color operator*=(Color& lhs, const float rhs) {
  lhs.r *= rhs;
  lhs.g *= rhs;
  lhs.b *= rhs;
  return lhs;
}
inline Color operator/=(Color& lhs, const float rhs) {
  lhs.r /= rhs;
  lhs.g /= rhs;
  lhs.b /= rhs;
  return lhs;
}

inline Color operator+(const Color& lhs, const float rhs) {
  Color temp = lhs;
  return temp += rhs;
}
inline Color operator-(const Color& lhs, const float rhs) {
  Color temp = lhs;
  return temp -= rhs;
}
inline Color operator*(const Color& lhs, const float rhs) {
  Color temp = lhs;
  return temp *= rhs;
}
inline Color operator/(const Color& lhs, const float rhs) {
  Color temp = lhs;
  return temp /= rhs;
}

struct ShepardsIDW {
  ShepardsIDW(int mapScaleFactor) : mMapScaleFactor{mapScaleFactor} {
    const int size = mMapScaleFactor * mMapScaleFactor * 4;
    mWeights = new float[size];
    mWeightsNR = new float[size];
    mWeightsNB = new float[size];
    mWeightsC = new float[size];
    fillShepardsIDW(mWeights, 1, 1);
    fillShepardsIDW(mWeightsNR, 0, 1);
    fillShepardsIDW(mWeightsNB, 1, 0);
    fillShepardsIDW(mWeightsC, 0, 0);
  }
  ~ShepardsIDW() {
    delete[] mWeights;
    delete[] mWeightsNR;
    delete[] mWeightsNB;
    delete[] mWeightsC;
  }

  int mMapScaleFactor;
  // Image :-
  // p00 p01 p02 p03 p04 p05 p06 p07
  // p10 p11 p12 p13 p14 p15 p16 p17
  // p20 p21 p22 p23 p24 p25 p26 p27
  // p30 p31 p32 p33 p34 p35 p36 p37
  // p40 p41 p42 p43 p44 p45 p46 p47
  // p50 p51 p52 p53 p54 p55 p56 p57
  // p60 p61 p62 p63 p64 p65 p66 p67
  // p70 p71 p72 p73 p74 p75 p76 p77

  // Recovery Map (for 4 scale factor) :-
  // m00 p01
  // m10 m11

  // Recovery sample of curr 4x4, right 4x4, bottom 4x4, bottom right 4x4 are used during
  // reconstruction. hence table weight size is 4.
  float* mWeights;
  // TODO: check if its ok to mWeights at places
  float* mWeightsNR;  // no right
  float* mWeightsNB;  // no bottom
  float* mWeightsC;  // no right & bottom

  float euclideanDistance(float x1, float x2, float y1, float y2);
  void fillShepardsIDW(float *weights, int incR, int incB);
};


////////////////////////////////////////////////////////////////////////////////
// sRGB transformations
// NOTE: sRGB has the same color primaries as BT.709, but different transfer
// function. For this reason, all sRGB transformations here apply to BT.709,
// except for those concerning transfer functions.

/*
 * Calculate the luminance of a linear RGB sRGB pixel, according to IEC 61966-2-1.
 *
 * [0.0, 1.0] range in and out.
 */
float srgbLuminance(Color e);

/*
 * Convert from OETF'd srgb YUV to RGB, according to ECMA TR/98.
 */
Color srgbYuvToRgb(Color e_gamma);

/*
 * Convert from OETF'd srgb RGB to YUV, according to ECMA TR/98.
 */
Color srgbRgbToYuv(Color e_gamma);

/*
 * Convert from srgb to linear, according to IEC 61966-2-1.
 *
 * [0.0, 1.0] range in and out.
 */
float srgbInvOetf(float e_gamma);
Color srgbInvOetf(Color e_gamma);
float srgbInvOetfLUT(float e_gamma);
Color srgbInvOetfLUT(Color e_gamma);

////////////////////////////////////////////////////////////////////////////////
// Display-P3 transformations

/*
 * Calculated the luminance of a linear RGB P3 pixel, according to SMPTE EG 432-1.
 *
 * [0.0, 1.0] range in and out.
 */
float p3Luminance(Color e);


////////////////////////////////////////////////////////////////////////////////
// BT.2100 transformations - according to ITU-R BT.2100-2

/*
 * Calculate the luminance of a linear RGB BT.2100 pixel.
 *
 * [0.0, 1.0] range in and out.
 */
float bt2100Luminance(Color e);

/*
 * Convert from OETF'd BT.2100 RGB to YUV.
 */
Color bt2100RgbToYuv(Color e_gamma);

/*
 * Convert from OETF'd BT.2100 YUV to RGB.
 */
Color bt2100YuvToRgb(Color e_gamma);

/*
 * Convert from scene luminance to HLG.
 *
 * [0.0, 1.0] range in and out.
 */
float hlgOetf(float e);
Color hlgOetf(Color e);
float hlgOetfLUT(float e);
Color hlgOetfLUT(Color e);

/*
 * Convert from HLG to scene luminance.
 *
 * [0.0, 1.0] range in and out.
 */
float hlgInvOetf(float e_gamma);
Color hlgInvOetf(Color e_gamma);
float hlgInvOetfLUT(float e_gamma);
Color hlgInvOetfLUT(Color e_gamma);

/*
 * Convert from scene luminance to PQ.
 *
 * [0.0, 1.0] range in and out.
 */
float pqOetf(float e);
Color pqOetf(Color e);
float pqOetfLUT(float e);
Color pqOetfLUT(Color e);

/*
 * Convert from PQ to scene luminance in nits.
 *
 * [0.0, 1.0] range in and out.
 */
float pqInvOetf(float e_gamma);
Color pqInvOetf(Color e_gamma);
float pqInvOetfLUT(float e_gamma);
Color pqInvOetfLUT(Color e_gamma);


////////////////////////////////////////////////////////////////////////////////
// Color space conversions

/*
 * Convert between color spaces with linear RGB data, according to ITU-R BT.2407 and EG 432-1.
 *
 * All conversions are derived from multiplying the matrix for XYZ to output RGB color gamut by the
 * matrix for input RGB color gamut to XYZ. The matrix for converting from XYZ to an RGB gamut is
 * always the inverse of the RGB gamut to XYZ matrix.
 */
Color bt709ToP3(Color e);
Color bt709ToBt2100(Color e);
Color p3ToBt709(Color e);
Color p3ToBt2100(Color e);
Color bt2100ToBt709(Color e);
Color bt2100ToP3(Color e);

/*
 * Identity conversion.
 */
inline Color identityConversion(Color e) { return e; }

/*
 * Get the conversion to apply to the HDR image for recovery map generation
 */
ColorTransformFn getHdrConversionFn(jpegr_color_gamut sdr_gamut, jpegr_color_gamut hdr_gamut);


////////////////////////////////////////////////////////////////////////////////
// Recovery map calculations

/*
 * Calculate the 8-bit unsigned integer recovery value for the given SDR and HDR
 * luminances in linear space, and the hdr ratio to encode against.
 */
uint8_t encodeRecovery(float y_sdr, float y_hdr, float hdr_ratio);

/*
 * Calculates the linear luminance in nits after applying the given recovery
 * value, with the given hdr ratio, to the given sdr input in the range [0, 1].
 */
Color applyRecovery(Color e, float recovery, float hdr_ratio);

/*
 * Helper for sampling from YUV 420 images.
 */
Color getYuv420Pixel(jr_uncompressed_ptr image, size_t x, size_t y);

/*
 * Helper for sampling from P010 images.
 *
 * Expect narrow-range image data for P010.
 */
Color getP010Pixel(jr_uncompressed_ptr image, size_t x, size_t y);

/*
 * Sample the image at the provided location, with a weighting based on nearby
 * pixels and the map scale factor.
 */
Color sampleYuv420(jr_uncompressed_ptr map, size_t map_scale_factor, size_t x, size_t y);

/*
 * Sample the image at the provided location, with a weighting based on nearby
 * pixels and the map scale factor.
 *
 * Expect narrow-range image data for P010.
 */
Color sampleP010(jr_uncompressed_ptr map, size_t map_scale_factor, size_t x, size_t y);

/*
 * Sample the recovery value for the map from a given x,y coordinate on a scale
 * that is map scale factor larger than the map size.
 */
float sampleMap(jr_uncompressed_ptr map, float map_scale_factor, size_t x, size_t y);
float sampleMap(jr_uncompressed_ptr map, size_t map_scale_factor, size_t x, size_t y,
                ShepardsIDW& weightTables);

/*
 * Convert from Color to RGBA1010102.
 *
 * Alpha always set to 1.0.
 */
uint32_t colorToRgba1010102(Color e_gamma);

} // namespace android::recoverymap

#endif // ANDROID_JPEGRECOVERYMAP_RECOVERYMAPMATH_H
