// Author: Felix Fung
// 2022-8-19
// 20ztfeng@stu.edu.cn


#ifndef SPEC_H
#define SPEC_H

//#define _GLIBCXX_USE_CXX11_ABI 0
#include "sigpack-1.2.7/sigpack/sigpack.h"
#include "opencv4/opencv2/opencv.hpp"
#include "iostream"
#include "vector"

typedef struct WAV_HEADER {
  uint8_t RIFF[4];        // RIFF Header Magic header
  uint32_t chunkSize;     // RIFF Chunk Size
  uint8_t WAVE[4];        // WAVE Header
  uint8_t fmt[4];         // FMT header
  uint32_t subchunk1Size; // Size of the fmt chunk
  uint16_t audioFormat;   // Audio format
  uint16_t numOfChan;     // Number of channels
  uint32_t samplesPerSec; // Sampling Frequency in Hz
  uint32_t bytesPerSec;   // bytes per second
  uint16_t blockAlign;    // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample; // Number of bits per sample
  uint8_t subchunk2ID[4]; // "data"  string
  uint32_t subchunk2Size; // Sampled data length
} wav_header;

////////////////////////////////////////
/// \brief 主函数，读取wav，然后返回频谱图
/// 鉴于某matlab使用了未知归一化方法，所以只能保证大概和它一样
/// @param path: 要读取的wav的路径
/// @param save_path: 要保存的路径
//////////////////////////////////
void wave2spec(const char *, const char *, int width=800, int height=800);

/////////////////////////////////////
/// \brief z-score 归一化，会把噪点全
/// 部去掉，看上去效果很好但是并不符合要求
/// @param origin 要归一化的矩阵
/////////////////////////////////////
cv::Mat Normalize(cv::Mat);

/////////////////////////////////////
/// \brief 最大最小规范化，看上去和MATLAB
/// 用的很相似，但是并不是
/// @param origin 要归一化的矩阵
/////////////////////////////////////
cv::Mat min_max_Normalize(cv::Mat origin);

/////////////////////////////////////
/// \brief 将Arma矩阵转为cv矩阵
/// 
/// @param arma_mat_int 要转化的arma矩阵
/// @param cv_mat_out 变成cv矩阵
/////////////////////////////////////
template<typename T>
void Arma_mat_to_cv_mat(arma::Mat<T>& arma_mat_in,cv::Mat_<T>& cv_mat_out);



#endif