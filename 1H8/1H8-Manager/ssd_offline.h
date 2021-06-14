#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <algorithm>
#include <sys/time.h>
#include <iomanip>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <map>
#include <math.h>
#include <utility>
#include <vector>
#include "cnrt.h"

typedef unsigned int uint;


typedef struct{
  char *input_data;
  float *output_data;
  int  input_size;
  int  output_size;
} img_detection;


using namespace std;

class Detector {
  //需要定义新的函数请直接在cpp中定义,此处不能添加新的接口或变量
  public:
  Detector(string model_path);
  ~Detector();

  void detection(img_detection *img_det);

  
  private:
  void SetMean(const string& mean_file, const string& mean_value);
  void WrapInputLayer(std::vector<std::vector<cv::Mat> >* input_imgs);
  void Preprocess(const std::vector<cv::Mat>& imgs,
                  std::vector<std::vector<cv::Mat> >* input_imgs);
  cv::Size input_geometry_;
  int batch_size_;
  int num_channels_;
  cv::Mat mean_;
  void** inputCpuPtrS;
  void** outputCpuPtrS;
  void** inputMluPtrS;
  void** outputMluPtrS;
  cnrtDataDescArray_t inputDescS, outputDescS;
  cnrtStream_t stream;
  int inputNum, outputNum;
  cnrtFunction_t function;
  unsigned int out_n, out_c, out_h, out_w;
  int out_count;

}; 
