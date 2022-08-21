#include "spec.h"
using namespace sp;
void wave2spec(const char * path, const char * save_path, int width, int height)
{
    wav_header wavHeader;
    // 读取wav文件
    FILE *wavFile = fopen(path, "r");
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

    // Spectrogram的计算，从源文件直接翻译
    const int FFT_SIZE = 256;
    const int FFT_OVERLAP = 60;

    arma::mat P = 10 * log10(abs(specgram_bl(x_left, FFT_SIZE, FFT_OVERLAP)));
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

    // 2.7 和 -0.3是魔法数字，不知道怎么出来的，反正是这个就是了
    spec = min_max_Normalize(spec);
    spec = spec * 2.7 - 0.3;

    // 将float转为8UC1
    spec.convertTo(target, CV_8UC1, 100);

    // PARULA是MATLAB默认的colormap
    cv::applyColorMap(target, final, cv::COLORMAP_PARULA);

    // 拉伸频谱图
    cv::Mat resize;
    cv::Size s(width, height);
    cv::resize(final, resize, s);

    // 保存图片
    cv::imwrite(save_path, resize);
}

template<typename T>
static void Arma_mat_to_cv_mat(arma::Mat<T>& arma_mat_in,cv::Mat_<T>& cv_mat_out)
{
    cv::transpose(cv::Mat_<T>(static_cast<int>(arma_mat_in.n_cols),
                              static_cast<int>(arma_mat_in.n_rows),
                              const_cast<T*>(arma_mat_in.memptr())),
                  cv_mat_out);
};

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