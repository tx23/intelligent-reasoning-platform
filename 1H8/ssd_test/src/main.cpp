#include "ssd_offline.h"

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
#include "yolov2.pb.h"

using namespace cv;
using namespace std;


cv::Scalar HSV2RGB(const float h, const float s, const float v) {
  const int h_i = static_cast<int>(h * 6);
  const float f = h * 6 - h_i;
  const float p = v * (1 - s);
  const float q = v * (1 - f*s);
  const float t = v * (1 - (1 - f) * s);
  float r, g, b;
  switch (h_i) {
    case 0:
      r = v; g = t; b = p;
      break;
    case 1:
      r = q; g = v; b = p;
      break;
    case 2:
      r = p; g = v; b = t;
      break;
    case 3:
      r = p; g = q; b = v;
      break;
    case 4:
      r = t; g = p; b = v;
      break;
    case 5:
      r = v; g = p; b = q;
      break;
    default: 
      r = 1; g = 1; b = 1;
      break;
  }
  return cv::Scalar(r * 255, g * 255, b * 255);
}


vector<cv::Scalar> GetColors(const int n) {
  vector<cv::Scalar> colors;
  cv::RNG rng(12345);
  const float golden_ratio_conjugate = 0.618033988749895;
  const float s = 0.3;
  const float v = 0.99;
  for (int i = 0; i < n; ++i) {
    const float h = std::fmod(rng.uniform(0.f, 1.f) + golden_ratio_conjugate,
                              1.f);
    colors.push_back(HSV2RGB(h, s, v));
  }
  return colors;
}

void WriteVisualizeBBox_offline(const vector<cv::Mat>& images,
                   const vector<vector<vector<float > > > detections,
                   const float threshold, const vector<cv::Scalar>& colors,
                   const map<int, string>& label_to_display_name,
                   const vector<string>& img_names) {
  // Retrieve detections.
  const int num_img = images.size();
  vector< map< int, vector<NormalizedBBox> > > all_detections(num_img);
  for (int i = 0; i < num_img; ++i) {
    for (int j = 0; j < detections[i].size(); j++) {
      const int img_idx = i;
      const int label = detections[i][j][1];
      const float score = detections[i][j][2];
      if (score < threshold) {
        continue;
      }
      NormalizedBBox bbox;
      bbox.set_xmin(detections[i][j][3] *
                    images[i].cols);
      bbox.set_ymin(detections[i][j][4] *
                    images[i].rows);
      bbox.set_xmax(detections[i][j][5] *
                    images[i].cols);
      bbox.set_ymax(detections[i][j][6] *
                    images[i].rows);
      bbox.set_score(score);
      all_detections[img_idx][label].push_back(bbox);
    }
  }

  int fontface = cv::FONT_HERSHEY_SIMPLEX;
  double scale = 1;
  int thickness = 2;
  int baseline = 0;
  char buffer[50];
  for (int i = 0; i < num_img; ++i) {
    cv::Mat image = images[i];
    // Show FPS.
//    snprintf(buffer, sizeof(buffer), "FPS: %.2f", fps);
//    cv::Size text = cv::getTextSize(buffer, fontface, scale, thickness,
//                                    &baseline);
//    cv::rectangle(image, cv::Point(0, 0),
//                  cv::Point(text.width, text.height + baseline),
//                  CV_RGB(255, 255, 255), CV_FILLED);
//    cv::putText(image, buffer, cv::Point(0, text.height + baseline / 2.),
//                fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
    // Draw bboxes.
    std::string name = img_names[i];
    int pos = img_names[i].rfind("/");
    if (pos > 0 && pos < img_names[i].size()) {
      name = name.substr(pos + 1);
    }
    pos = name.rfind(".");
    if (pos > 0 && pos < name.size()) {
      name = name.substr(0, pos);
    }
    name = name + ".txt";
    name = name.substr(4);
    std::ofstream file(name.c_str());
    for (map<int, vector<NormalizedBBox> >::iterator it =
         all_detections[i].begin(); it != all_detections[i].end(); ++it) {
      int label = it->first;
      string label_name = "Unknown";
      if (label_to_display_name.find(label) != label_to_display_name.end()) {
        label_name = label_to_display_name.find(label)->second;
      }
      const cv::Scalar& color = colors[label];
      const vector<NormalizedBBox>& bboxes = it->second;
      for (int j = 0; j < bboxes.size(); ++j) {
        cv::Point top_left_pt(bboxes[j].xmin(), bboxes[j].ymin());
        cv::Point bottom_right_pt(bboxes[j].xmax(), bboxes[j].ymax());
        cv::rectangle(image, top_left_pt, bottom_right_pt, color, 4);
        cv::Point bottom_left_pt(bboxes[j].xmin(), bboxes[j].ymax());
        snprintf(buffer, sizeof(buffer), "%s: %.2f", label_name.c_str(),
                 bboxes[j].score());
        cv::Size text = cv::getTextSize(buffer, fontface, scale, thickness,
                                        &baseline);
        cv::rectangle(
            image, bottom_left_pt + cv::Point(0, 0),
            bottom_left_pt + cv::Point(text.width, -text.height - baseline),
            color, CV_FILLED);
        cv::putText(image, buffer, bottom_left_pt - cv::Point(0, baseline),
                    fontface, scale, CV_RGB(0, 0, 0), thickness, 8);
        file << label_name << " " << bboxes[j].score() << " "
            << bboxes[j].xmin() / image.cols << " "
            << bboxes[j].ymin() / image.rows << " "
            << bboxes[j].xmax() / image.cols
            << " " << bboxes[j].ymax() / image.rows << std::endl;
      }
    }
    file.close();
    cv::imwrite("output.jpg", image);
  }
};

void MatTochar(char *p2, cv::Mat& src){
        int img_width = src.cols;
        int img_height = src.rows;
        for(int c = 0; c < 3; c++)
          for (int i = 0; i < img_height; i++)
            for(int j = 0; j < img_width; j++)
        {
                p2[c*img_height*img_width + i*img_width+j] = src.at<Vec3b>(i, j)[c];
        }
};



void get_point_position(const vector<float> pos,
                        cv::Point* p1, cv::Point* p2, int h, int w) {
  int left = (pos[1]) * w;
  int right = (pos[3]) * w;
  int top = (pos[2]) * h;
  int bottom = (pos[6]) * h;
  if (left < 0) left = 5;
  if (top < 0) top = 5;
  if (right > w) right = w-5;
  if (bottom > h) bottom = h-5;
  p1->x = left;
  p1->y = top;
  p2->x = right;
  p2->y = bottom;
  return;
}

void WriteVisualizeBBox_offline(const cv::Mat images,
                   const vector<vector<float>> detections,
                   const vector<string> labels_,
                   const string img_names) {
  // Retrieve detections.
  const int num_img = 1;

//label_to_display_name[5] = "bus";
//label_to_display_name[6] = "train";
//label_to_display_name[7] = "truck";
  for (int i = 0; i < num_img; ++i) {
    cv::Mat image = images;
    std::string name = img_names;
    int pos_map = img_names.rfind("/");
    if (pos_map > 0 && pos_map < img_names.size()) {
      name = name.substr(pos_map + 1);
    }
    pos_map = name.rfind(".");
    if (pos_map > 0 && pos_map < name.size()) {
      name = name.substr(0, pos_map);
    }
    name = name + ".txt";
    // this is used to cancel "yolov2_offline_" in name
    name = name.substr(15);
    std::ofstream file_map(name);
    for (auto box : detections) {
      if (box[0] != i) {
        continue;
      }
      cv::Point p1, p2;
      get_point_position(box, &p1, &p2,
                         image.rows, image.cols);
      //cv::rectangle(image, p1, p2, cv::Scalar(0, 255, 0), 2);
      std::stringstream s0;
      s0 << box[2];
      string s1 = s0.str();
      cv::putText(image, labels_[box[1]], cv::Point(p1.x, p1.y - 10), 2, 0.5, cv::Scalar(255, 0, 0), 0, 8, 0);
      cv::putText(image, s1.c_str(), cv::Point(p1.x + 20, p1.y - 10), 2, 0.5, cv::Scalar(255, 0, 0), 0, 8, 0);
      std::cout << ">>> label: " << labels_[box[1]] << " score: "
                << box[2] << " , position : ";
      for (int idx = 0; idx < 4; ++idx) {
        std::cout << box[3 + idx] << " ";
      }
      std::cout << std::endl;
      std::cout << "detection result: " << box[1] << " " << box[2]
          << " " << box[3] << " " << box[4] << " " << box[5]
          << " " << box[6] << std::endl;
      pos_map = labels_[box[1]].rfind(" ");
      // the order for mAP is: label score x_min y_min x_max y_max
      file_map << labels_[box[1]].substr(0,pos_map) << " "
          << static_cast<float>(box[2]) << " "
          << static_cast<float>(p1.x) / image.cols << " "
          << static_cast<float>(p1.y) / image.rows << " "
          << static_cast<float>(p2.x) / image.cols << " "
          << static_cast<float>(p2.y) / image.rows
          << std::endl;
    }
    std::cout << "Save image: " << img_names << std::endl;
    cv::imwrite(img_names, image);
    file_map.close();
  }
}


int main(int argc, char** argv) {

  //从本地加载图片,构造结构体
  Mat test_img = imread("./input.jpg", -1);
  char* p = (char*)malloc(sizeof(char) * test_img.cols*test_img.rows*3);
  MatTochar(p, test_img);
  img_detection imd;
  imd.input_size = test_img.cols*test_img.rows*3;
  imd.input_data = (char*)malloc(sizeof(char) * imd.input_size); //分配内存
  imd.output_data = NULL;
  for(int i=0; i<imd.input_size; i++)
    *(imd.input_data+i) = *(p+i); //复制指针指向的值
  Detector* detect = new Detector("./model/ssd.cambricon");

  map<int, string> label_to_display_name;


  label_to_display_name[0] = "satelite";
  label_to_display_name[1] = "solar-array";
  label_to_display_name[2] = "major-structure";

  vector<Scalar> colors = GetColors(label_to_display_name.size());

  //调用推理函数,传入结构体
  detect->detection(&imd);

  for(int k = 0; k < imd.output_size/9; k++)
  {
	 for(int h = 0; h < 9; h++)
	 {
		 std::cout<<*(imd.output_data + h)<<" ";
	 }
	 printf("\n");
	 imd.output_data = imd.output_data + 9;
  }
/*  
  float scale = 1;
    for (int j = 0; j < boxes.size(); ++j) {
      cv::Point p1, p2;
      get_point_position(boxes[j], &p1, &p2,
                         test_img.size().height, test_img.size().width);

      p1.x = p1.x / scale;
      p1.y = p1.y / scale;
      p2.x = p2.x / scale;
      p2.y = p2.y / scale;
			//string label_name = labels_[boxes[j][1]];
//			if(label_name == "person"){
      cv::rectangle(test_img, p1, p2, cv::Scalar(0, 255, 0), 2, 8, 0);
      //std::stringstream s0;
			//boxes[j][2] = boxes[j][2];
     // s0 << boxes[j][2];
      string s1 = label_to_display_name[boxes[j][0]];
      //cv::putText(img, labels_[boxes[j][1]],    cv::Point(p1.x, p1.y - 10), 2, 0.5, cv::Scalar(255, 0, 0), 0, 8, 0);
      cv::putText(test_img, s1.c_str(), cv::Point(p1.x, p1.y - 9), 2, 0.5, cv::Scalar(0, 0, 255), 0, 8, 0);
      std::cout << ">>> label: " << label_to_display_name[boxes[j][0]] << " score: "
                << " , position : ";
//			}
      for (int idx = 0; idx < 8; ++idx) {
        std::cout << boxes[j][1 + idx] << " ";
      }
      std::cout << std::endl;
    }
    delete detect; 
    //int pos = file.find_last_of("/");
   // string img_name;
   // std::stringstream ss;
    //ss << "yolov2_offline_" << file.substr(pos + 1, file.length() - pos - 1);
   // ss >> img_name;
    cv::imwrite("out_img.jpg" , test_img);
    //WriteVisualizeBBox_offline(test_img, boxes, labels_, img_name);
*/  
  return 0;
  
}
