// Author: Felix Fung 
// 2022-8-19

//#define _GLIBCXX_USE_CXX11_ABI 0
#include "sigpack-1.2.7/sigpack/sigpack.h"
#include "opencv4/opencv2/opencv.hpp"
#include "iostream"
#include "vector"


using namespace sp;

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


cv::Mat Normalize(cv::Mat);
cv::Mat min_max_Normalize(cv::Mat origin);

template<typename T>
void Arma_mat_to_cv_mat(arma::Mat<T>& arma_mat_in,cv::Mat_<T>& cv_mat_out);

int main()
{
  wav_header wavHeader;
  // Get THX DeepNote .wav file at
  FILE *wavFile = fopen("test_2.wav", "r");
  // Read the header
  fread(&wavHeader, 1, sizeof(wav_header), wavFile);
  // Read the data
  uint16_t Nb = wavHeader.bitsPerSample / 8;     // Number of bytes per sample
  uint32_t Ns = (wavHeader.chunkSize - 36) / Nb; // Number of samples
  arma::Mat<int16_t> x(wavHeader.numOfChan, Ns / wavHeader.numOfChan);
  fread(x.memptr(), Nb, Ns, wavFile);
  fclose(wavFile);
  // Get the left channel
  arma::Col<int16_t> x_left = x.row(0).t();

  // Spectrogram
  const int FFT_SIZE = 256;
  const int FFT_OVERLAP = 60;

  arma::mat P = 10 * log10(abs(specgram_bl(x_left, FFT_SIZE, FFT_OVERLAP)));

  // Plot
  arma::mat Q =
      P.rows(FFT_SIZE / 2, FFT_SIZE-1); // Cut out the positive parts

  // 将图片旋转正常
  Q = Q.t();
  Q = arma::fliplr(Q);
  Q = arma::flipud(Q);


  cv::Mat_<double> spec;
  cv::Mat target;
  cv::Mat final;
  Arma_mat_to_cv_mat<double>(Q,spec);

  double minVal, maxVal;
  

  cv::minMaxLoc(spec, &minVal, &maxVal);

  //spec += abs(minVal);
  //spec = Normalize(spec);
  spec = min_max_Normalize(spec);
  spec = spec * 2.7 - 0.3;
  //spec = Normalize(spec);
  //spec = spec * 0.9;

  //cv::normalize(spec, spec, cv::NORM_MINMAX);
  //spec = spec;

  cv::minMaxLoc(spec, &minVal, &maxVal);
  std::cout<< "max number: " << maxVal << ", min number: " << minVal;

  spec.convertTo(target, CV_8UC1, 100);
  

  cv::applyColorMap(target, final, cv::COLORMAP_PARULA);


  cv::Mat resize;
  cv::Size s(800, 800);
  cv::resize(final, resize, s);

  cv::imwrite("./save.png", resize);
  return 0;
    
}


template<typename T>
static void Arma_mat_to_cv_mat(arma::Mat<T>& arma_mat_in,cv::Mat_<T>& cv_mat_out)
{
    cv::transpose(cv::Mat_<T>(static_cast<int>(arma_mat_in.n_cols),
                              static_cast<int>(arma_mat_in.n_rows),
                              const_cast<T*>(arma_mat_in.memptr())),
                  cv_mat_out);
};


// z-score 标准化
cv::Mat Normalize(cv::Mat origin)
{
  cv::Mat mat_std;
  cv::Mat mat_mat_mean;
  cv::meanStdDev(origin, mat_mat_mean, mat_std);
  double mat_mean = mat_mat_mean.at<double>(0, 0);
  double std = mat_std.at<double>(0,0);

  cv::Mat x = (origin - mat_mean) / std;
  return x;

}

cv::Mat min_max_Normalize(cv::Mat origin)
{
  double minVal, maxVal;
  cv::minMaxLoc(origin, &minVal, &maxVal);

  return (origin - minVal) / (maxVal - minVal);
}