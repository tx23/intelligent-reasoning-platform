// Copyright [2018] <cambricon>
// This is a demo code for using a SSD model to do detection.
// The code is modified from examples/cpp_classification/classification.cpp.
// Usage:
//    ssd_detect [FLAGS] model_file weights_file list_file
//
// where model_file is the .prototxt file defining the network architecture, and
// weights_file is the .caffemodel file containing the network parameters, and
// list_file contains a list of image files with the format as follows:
//    folder/img1.JPEG
//    folder/img2.JPEG
// list_file can also contain a list of video files with the format as follows:
//    folder/video1.mp4
//    folder/video2.mp4
//
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
#include <utility>
#include <vector>
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "cnrt.h"
#include "ssd_offline.h"
#include "yolov2.pb.h"

using namespace cv;
using namespace std;

void IntersectBBox(const NormalizedBBox &bbox1, const NormalizedBBox &bbox2,
                   NormalizedBBox *intersect_bbox)
{
  if (bbox2.xmin() > bbox1.xmax() || bbox2.xmax() < bbox1.xmin() ||
      bbox2.ymin() > bbox1.ymax() || bbox2.ymax() < bbox1.ymin())
  {
    // Return [0, 0, 0, 0] if there is no intersection.
    intersect_bbox->set_xmin(0);
    intersect_bbox->set_ymin(0);
    intersect_bbox->set_xmax(0);
    intersect_bbox->set_ymax(0);
  }
  else
  {
    intersect_bbox->set_xmin(std::max(bbox1.xmin(), bbox2.xmin()));
    intersect_bbox->set_ymin(std::max(bbox1.ymin(), bbox2.ymin()));
    intersect_bbox->set_xmax(std::min(bbox1.xmax(), bbox2.xmax()));
    intersect_bbox->set_ymax(std::min(bbox1.ymax(), bbox2.ymax()));
  }
}

float BBoxSize(const NormalizedBBox &bbox, const bool normalized = true)
{
  if (bbox.xmax() < bbox.xmin() || bbox.ymax() < bbox.ymin())
  {
    // If bbox is invalid (e.g. xmax < xmin or ymax < ymin), return 0.
    return 0;
  }
  else
  {
    if (bbox.has_size())
    {
      return bbox.size();
    }
    else
    {
      float width = bbox.xmax() - bbox.xmin();
      float height = bbox.ymax() - bbox.ymin();
      if (normalized)
      {
        return width * height;
      }
      else
      {
        // If bbox is not within range [0, 1].
        return (width + 1) * (height + 1);
      }
    }
  }
}

float JaccardOverlap(const NormalizedBBox &bbox1, const NormalizedBBox &bbox2,
                     const bool normalized)
{
  NormalizedBBox intersect_bbox;
  IntersectBBox(bbox1, bbox2, &intersect_bbox);
  float intersect_width, intersect_height;
  if (normalized)
  {
    intersect_width = intersect_bbox.xmax() - intersect_bbox.xmin();
    intersect_height = intersect_bbox.ymax() - intersect_bbox.ymin();
  }
  else
  {
    intersect_width = intersect_bbox.xmax() - intersect_bbox.xmin() + 1;
    intersect_height = intersect_bbox.ymax() - intersect_bbox.ymin() + 1;
  }
  if (intersect_width > 0 && intersect_height > 0)
  {
    float intersect_size = intersect_width * intersect_height;
    float bbox1_size = BBoxSize(bbox1);
    float bbox2_size = BBoxSize(bbox2);
    return intersect_size / (bbox1_size + bbox2_size - intersect_size);
  }
  else
  {
    return 0.;
  }
}

template <typename Dtype>
void setNormalizedBBox(NormalizedBBox *bbox,
                       Dtype x, Dtype y, Dtype w, Dtype h)
{
  Dtype xmin = x - w / 2.0;
  Dtype xmax = x + w / 2.0;
  Dtype ymin = y - h / 2.0;
  Dtype ymax = y + h / 2.0;

  if (xmin < 0.0)
  {
    xmin = 0.0;
  }
  if (xmax > 1.0)
  {
    xmax = 1.0;
  }
  if (ymin < 0.0)
  {
    ymin = 0.0;
  }
  if (ymax > 1.0)
  {
    ymax = 1.0;
  }
  bbox->set_xmin(xmin);
  bbox->set_ymin(ymin);
  bbox->set_xmax(xmax);
  bbox->set_ymax(ymax);
  float bbox_size = BBoxSize(*bbox, true);
  bbox->set_size(bbox_size);
}

template <typename Dtype>
class PredictionResult
{
public:
  Dtype x;
  Dtype y;
  Dtype w;
  Dtype h;
  Dtype objScore;
  Dtype classScore;
  Dtype confidence;
  int classType;
};
template <typename Dtype>
void class_index_and_score(Dtype *input, int classes,
                           PredictionResult<Dtype> *predict)
{
  Dtype sum = 0;
  Dtype large = input[0];
  int classIndex = 0;
  for (int i = 0; i < classes; ++i)
  {
    if (input[i] > large)
      large = input[i];
  }
  for (int i = 0; i < classes; ++i)
  {
    Dtype e = exp(input[i] - large);
    sum += e;
    input[i] = e;
  }

  for (int i = 0; i < classes; ++i)
  {
    input[i] = input[i] / sum;
  }
  large = input[0];
  classIndex = 0;

  for (int i = 0; i < classes; ++i)
  {
    if (input[i] > large)
    {
      large = input[i];
      classIndex = i;
    }
  }
  predict->classType = classIndex;
  predict->classScore = large;
}

template <typename Dtype>
inline Dtype sigmoid(Dtype x)
{
  return 1. / (1. + exp(-x));
}

template <typename Dtype>
void get_region_box(Dtype *x, PredictionResult<Dtype> *predict,
                    vector<Dtype> biases, int n, int index,
                    int i, int j, int w, int h)
{
  predict->x = (i + sigmoid(x[index + 0])) / w;
  predict->y = (j + sigmoid(x[index + 1])) / h;
  predict->w = exp(x[index + 2]) * biases[2 * n] / w;
  predict->h = exp(x[index + 3]) * biases[2 * n + 1] / h;
}

template <typename Dtype>
void ApplyNms(vector<PredictionResult<Dtype>> *boxes,
              vector<int> *idxes, Dtype threshold)
{
  map<int, int> idx_map;
  for (int i = 0; i < (*boxes).size() - 1; ++i)
  {
    if (idx_map.find(i) != idx_map.end())
    {
      continue;
    }
    for (int j = i + 1; j < (*boxes).size(); ++j)
    {
      if (idx_map.find(j) != idx_map.end())
      {
        continue;
      }
      NormalizedBBox Bbox1, Bbox2;
      setNormalizedBBox(&Bbox1, (*boxes)[i].x, (*boxes)[i].y,
                        (*boxes)[i].w, (*boxes)[i].h);
      setNormalizedBBox(&Bbox2, (*boxes)[j].x, (*boxes)[j].y,
                        (*boxes)[j].w, (*boxes)[j].h);

      float overlap = JaccardOverlap(Bbox1, Bbox2, true);

      if (overlap >= threshold)
      {
        idx_map[j] = 1;
      }
    }
  }
  for (int i = 0; i < (*boxes).size(); ++i)
  {
    if (idx_map.find(i) == idx_map.end())
    {
      (*idxes).push_back(i);
    }
  }
}

vector<vector<float>> detection_out(float *net_output, int out_n, int out_c, int out_h, int out_w)
{
  vector<vector<float>> result;
  float *swap_data = reinterpret_cast<float *>(malloc(sizeof(float) * out_n * out_c * out_h * out_w));
  int index = 0;
  for (int b = 0; b < out_n; ++b)
    for (int h = 0; h < out_h; ++h)
      for (int w = 0; w < out_w; ++w)
        for (int c = 0; c < out_c; ++c)
        {
          swap_data[index++] =
              net_output[b * out_c * out_h * out_w + c * out_h * out_w + h * out_w + w];
        }

  vector<PredictionResult<float>> predicts;
  PredictionResult<float> predict;
  predicts.clear();

  vector<float> biases_({58.64226974 / 256 * 32, 17.69161184 / 256 * 32, 46.24476295 / 256 * 32, 114.71113561 / 256 * 32, 29.80213465 / 256 * 32, 30.13492063 / 256 * 32, 112.28542094 / 256 * 32, 46.86036961 / 256 * 32, 18.25620496 / 256 * 32, 59.73658927 / 256 * 32});

  int side_ = 32;
  int num_box_ = 5;
  int num_classes_ = 3;
  float nms_threshold_ = 0.3;
  float confidence_threshold_ = 0.75;
  char *env = getenv("CONFIDENCE_THRESHOLD");
  if ((NULL != env) && (strlen(env) != 0))
    confidence_threshold_ = strtof(env, NULL);
  else
    confidence_threshold_ = 0.45;
  for (int b = 0; b < out_n; ++b)
  {
    for (int j = 0; j < side_; ++j)
      for (int i = 0; i < side_; ++i)
        for (int n = 0; n < num_box_; ++n)
        {
          int index = b * out_c * out_h * out_w + (j * side_ + i) * out_c +
                      n * out_c / num_box_;
          get_region_box(swap_data, &predict, biases_, n,
                         index, i, j, side_, side_);
          predict.objScore = sigmoid(swap_data[index + 4]);
          class_index_and_score(swap_data + index + 5, num_classes_, &predict);
          predict.confidence = predict.objScore * predict.classScore;
          if (predict.confidence >= confidence_threshold_)
          {
            predicts.push_back(predict);
          }
        }
    vector<int> idxes;
    int num_kept = 0;
    if (predicts.size() > 0)
    {
      ApplyNms(&predicts, &idxes, nms_threshold_);
      num_kept = idxes.size();
    }
    for (int i = 0; i < num_kept; i++)
    {
      std::vector<float> temp;
      temp.push_back(predicts[idxes[i]].classType); // label
      float x1 = predicts[idxes[i]].x - predicts[idxes[i]].w / 2;
      float y1 = predicts[idxes[i]].y - predicts[idxes[i]].h / 2;
      float x2 = predicts[idxes[i]].x + predicts[idxes[i]].w / 2;
      float y2 = predicts[idxes[i]].y - predicts[idxes[i]].h / 2;
      float x3 = predicts[idxes[i]].x - predicts[idxes[i]].w / 2;
      float y3 = predicts[idxes[i]].y + predicts[idxes[i]].h / 2;
      float x4 = predicts[idxes[i]].x + predicts[idxes[i]].w / 2;
      float y4 = predicts[idxes[i]].y + predicts[idxes[i]].h / 2;
      temp.push_back(x1);
      temp.push_back(y1);
      temp.push_back(x2);
      temp.push_back(y2);
      temp.push_back(x3);
      temp.push_back(y3);
      temp.push_back(x4);
      temp.push_back(y4);
      result.push_back(temp);
    }
  }

  free(swap_data);
  return result;
}

string ModelFile;

Detector::Detector(string model_file)
{
  ModelFile = model_file;
}

void Detector::SetMean(const string &mean_file, const string &mean_value)
{
  cv::Scalar channel_mean;
  if (!mean_value.empty())
  {
    CHECK(mean_file.empty()) << "Cannot specify mean_file and mean_value at the same time";
    stringstream ss(mean_value);
    vector<float> values;
    string item;
    while (getline(ss, item, ','))
    {
      float value = std::atof(item.c_str());
      values.push_back(value);
    }
    CHECK(values.size() == 1 || values.size() == num_channels_) << "Specify either 1 mean_value or as many as channels: " << num_channels_;
    std::vector<cv::Mat> channels;
    for (int i = 0; i < num_channels_; ++i)
    {
      /* Extract an individual channel. */
      cv::Mat channel(input_geometry_.height, input_geometry_.width, CV_32FC1,
                      cv::Scalar(values[i]));
      channels.push_back(channel);
    }
    cv::merge(channels, mean_);
  }
}

Detector::~Detector()
{

  free(inputCpuPtrS[0]);
  free(inputCpuPtrS);
  //free((float*)outputCpuPtrS[0]);
  free(outputCpuPtrS);
}

void charToMat(char *p2, cv::Mat &src)
{
  int img_width = src.cols;
  int img_height = src.rows;
  for (int c = 0; c < 3; c++)
    for (int i = 0; i < img_height; i++)
      for (int j = 0; j < img_width; j++)
      {
        src.at<Vec3b>(i, j)[c] = p2[c * img_width * img_height + i * img_width + j]; //BGR格式
                                                                                     //src.at<Vec3b>(i, j)[c] = p2[i*img_width+j];
      }
};

void Detector::detection(img_detection *img_det)
{

  //-------------
  // offline model
  // 1. init runtime_lib and device
  const string mean_file = "";
  const string mean_value = "";
  cnrtInit(0);
  unsigned dev_num;
  cnrtGetDeviceCount(&dev_num);
  if (dev_num == 0)
  {
    std::cout << "no device found" << std::endl;
    exit(-1);
  }
  cnrtDev_t dev;
  cnrtGetDeviceHandle(&dev, 0);
  cnrtSetCurrentDevice(dev);
  // 2. load model and get function
  cnrtModel_t model;
  string model_file = ModelFile;
  printf("load file: %s\n", model_file.c_str());
  cnrtLoadModel(&model, model_file.c_str());
  string name = "subnet0";
  cnrtCreateFunction(&function);
  cnrtExtractFunction(&function, model, name.c_str());
  cnrtInitFunctionMemory(function, CNRT_FUNC_TYPE_BLOCK);
  // 3. get function's I/O DataDesc
  cnrtGetInputDataDesc(&inputDescS, &inputNum, function);
  cnrtGetOutputDataDesc(&outputDescS, &outputNum, function);
  // 4. allocate I/O data space on CPU memory and prepare Input data
  inputCpuPtrS = (void **)malloc(sizeof(void *) * inputNum);
  outputCpuPtrS = (void **)malloc(sizeof(void *) * outputNum);
  int in_count;

  std::cout << "input blob num is " << inputNum << std::endl;
  for (int i = 0; i < inputNum; i++)
  {
    unsigned int in_n, in_c, in_h, in_w;
    cnrtDataDesc_t inputDesc = inputDescS[i];
    cnrtGetHostDataCount(inputDesc, &in_count);
    //    inputCpuPtrS[i] = (void*)malloc(sizeof(float) * in_count);
    inputCpuPtrS[i] = (void *)malloc(sizeof(float) * in_count);
    cnrtSetHostDataLayout(inputDesc, CNRT_FLOAT32, CNRT_NCHW);
    cnrtGetDataShape(inputDesc, &in_n, &in_c, &in_h, &in_w);
    std::cout << "shape " << in_n << std::endl;
    std::cout << "shape " << in_c << std::endl;
    std::cout << "shape " << in_h << std::endl;
    std::cout << "shape " << in_w << std::endl;
    if (i == 0)
    {
      batch_size_ = in_n;
      num_channels_ = in_c;
      input_geometry_ = cv::Size(in_w, in_h);
    }
    else
    {
      cnrtGetHostDataCount(inputDesc, &in_count);
      float *data = (float *)inputCpuPtrS[1];
      for (int j = 0; j < in_count; j++)
      {
        data[j] = 1;
      }
    }
  }

  for (int i = 0; i < outputNum; i++)
  {
    cnrtDataDesc_t outputDesc = outputDescS[i];
    cnrtSetHostDataLayout(outputDesc, CNRT_FLOAT32, CNRT_NCHW);
    cnrtGetHostDataCount(outputDesc, &out_count);
    cnrtGetDataShape(outputDesc, &out_n, &out_c, &out_h, &out_w);
    outputCpuPtrS[i] = (void *)malloc(sizeof(float) * out_count);
    std::cout << "output shape " << out_n << std::endl;
    std::cout << "output shape " << out_c << std::endl;
    std::cout << "output shape " << out_h << std::endl;
    std::cout << "output shape " << out_w << std::endl;
  }

  // 5. allocate I/O data space on MLU memory and copy Input data
  cnrtMallocByDescArray(&inputMluPtrS, inputDescS, inputNum);
  cnrtMallocByDescArray(&outputMluPtrS, outputDescS, outputNum);
  cnrtCreateStream(&stream);

  /* Load the binaryproto mean file. */
  SetMean(mean_file, mean_value);
  //-----------------

  // cnrtInit(0);
  // unsigned dev_num;
  // cnrtGetDeviceCount(&dev_num);
  // if (dev_num == 0)
  // {
  //   std::cout << "no device found" << std::endl;
  //   exit(-1);
  // }
  // cnrtDev_t dev;
  // cnrtGetDeviceHandle(&dev, 0);
  // cnrtSetCurrentDevice(dev);

  vector<Mat> imgs;
  int imgsize = sqrt((img_det->input_size) / 3);
  // Mat img(imgsize, imgsize, CV_8UC3);
  Mat img(imgsize, imgsize, CV_64FC3);

  charToMat((*img_det).input_data, img);
  (*img_det).output_size = out_count;
  // Mat img = cv::imread("input.jpg", -1);
  //cv::imwrite("input2.jpg", img);
  imgs.push_back(img);
  vector<vector<Mat>> input_imgs;
  WrapInputLayer(&input_imgs);
  Preprocess(imgs, &input_imgs);

  float time_use;
  struct timeval tpend, tpstart;
  gettimeofday(&tpstart, NULL);

  std::cout << __LINE__ << "----------ipuMemcpy: copy input-----------" << std::endl;
  cnrtMemcpyByDescArray(inputMluPtrS, inputCpuPtrS, inputDescS,
                        inputNum, CNRT_MEM_TRANS_DIR_HOST2DEV);

  cnrtDim3_t dim = {1, 1, 1};
  void *param[] = {inputMluPtrS[0],
                   outputMluPtrS[0]};
  cnrtInvokeFunction(function, dim, param,
                     CNRT_FUNC_TYPE_BLOCK, stream, NULL);
  cnrtSyncStream(stream);
  gettimeofday(&tpend, NULL);
  time_use = 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec;
  std::cout << "Forward execution time: "
            << time_use << " us" << std::endl;

  std::cout << __LINE__ << "----------ipuMemcpy-----------" << std::endl;
  cnrtMemcpyByDescArray(outputCpuPtrS, outputMluPtrS, outputDescS,
                        outputNum, CNRT_MEM_TRANS_DIR_DEV2HOST);
  float *result = (float *)outputCpuPtrS[0];

  //后处理:格式:标签,左上x,左上y,右上x,右上y,左下x,左下y,右下x,右下y
  //  vector<vector<float> > boxes =
  //        detection_out(net_output, out_n, out_c, out_h, out_w);
  /*
  for(int i = 0; i<50; i++)
  {
	  std::cout<<*(result+i)<<" ";
  }
  printf("\n");
  printf("\n");
*/
  vector<vector<float>> boxes;
  for (int k = 0; k < out_count / 6; ++k)
  {
    if (result[0] == 0 && result[1] == 0 &&
        result[2] == 0 && result[3] == 0 &&
        result[4] == 0 && result[5] == 1)
    {
      // Skip invalid detection.
      result += 6;
      continue;
    }
    int batch = k * 6 / (out_c * out_h * out_w);
    vector<float> detection(9, 0);
    detection[0] = result[5];
    detection[1] = result[0] * imgsize;
    detection[2] = result[1] * imgsize;
    detection[3] = result[2] * imgsize;
    detection[4] = result[1] * imgsize;
    detection[5] = result[0] * imgsize;
    detection[6] = result[3] * imgsize;
    detection[7] = result[2] * imgsize;
    detection[8] = result[3] * imgsize;
    if (result[4] > 0.6)
      boxes.push_back(detection);
    result += 6;
  }
  cnrtunloadmodel(model);
  cnrtDestroy();

  //必要步骤:释放上次分配的输出内存
  if ((*img_det).output_data != NULL)
    free((*img_det).output_data);
  //重新分配内存,大小为此次的输出长度
  (*img_det).output_size = boxes.size() * 9; //输出长度(float)
  (*img_det).output_data = (float *)malloc(boxes.size() * 9 * sizeof(float));
  //将输出结果拷入结构体中的输出指针
  for (int i = 0; i < boxes.size(); i++)
    for (int j = 0; j < boxes[i].size(); j++)
      memcpy((*img_det).output_data + i * 9 + j, &boxes[i][j], sizeof(float));
}

void Detector::WrapInputLayer(std::vector<std::vector<cv::Mat>> *input_imgs)
{
  int width = input_geometry_.width;
  int height = input_geometry_.height;
  float *input_data = (float *)inputCpuPtrS[0];
  for (int i = 0; i < batch_size_; ++i)
  {
    (*input_imgs).push_back(std::vector<cv::Mat>());
    for (int j = 0; j < num_channels_; ++j)
    {
      cv::Mat channel(height, width, CV_32FC1, input_data);
      (*input_imgs)[i].push_back(channel);
      input_data += width * height;
    }
  }
}
void Detector::Preprocess(const std::vector<cv::Mat> &imgs,
                          std::vector<std::vector<cv::Mat>> *input_imgs)
{
  /* Convert the input image to the input image format of the network. */
  CHECK(imgs.size() == input_imgs->size())
      << "Size of imgs and input_imgs doesn't match";

  for (int i = 0; i < imgs.size(); ++i)
  {
    cv::Mat sample;
    if (imgs[i].channels() == 3 && num_channels_ == 1)
      cv::cvtColor(imgs[i], sample, cv::COLOR_BGR2GRAY);
    else if (imgs[i].channels() == 4 && num_channels_ == 1)
      cv::cvtColor(imgs[i], sample, cv::COLOR_BGRA2GRAY);
    else if (imgs[i].channels() == 4 && num_channels_ == 3)
      cv::cvtColor(imgs[i], sample, cv::COLOR_BGRA2BGR);
    else if (imgs[i].channels() == 1 && num_channels_ == 3)
      cv::cvtColor(imgs[i], sample, cv::COLOR_GRAY2BGR);
    else
      sample = imgs[i];

    cv::Mat sample_resized;

    if (sample.size() != input_geometry_)
      cv::resize(sample, sample_resized, input_geometry_);
    else
      sample_resized = sample;
    cv::Mat sample_float;
    if (num_channels_ == 3)
      sample_resized.convertTo(sample_float, CV_32FC3);
    else
      sample_resized.convertTo(sample_float, CV_32FC3);
    /* This operation will write the separate BGR planes directly to the
     * input layer of the network because it is wrapped by the cv::Mat
     * objects in input_channels. */

    cv::split(sample_float, (*input_imgs)[i]);
  }
}
