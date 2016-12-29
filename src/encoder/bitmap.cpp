#include <iostream>
#include <vector>
#include <string>
#include <itpp/itcomm.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


std::string get_bytes_of_image(cv::Mat src) {
  std::string dst;
  uchar *p;

  for (int y = 0; y < src.rows; y++) {
    p = src.ptr<uchar>(y);
    for (int x = 0; x < src.cols; x++) {
      dst += (p[x] == 255) ? "1" : "0";
      dst += " ";
    }
  }

  return dst;
}


int main(int argc, char const *argv[]) {
  std::string input_image_filename;
  std::string bytes_of_image;
  cv::Mat src_img, adaptive_img;

  input_image_filename = argv[1];

  // Grayscaleとして読込
  src_img = cv::imread(input_image_filename, 0);

  // 適応的二値化処理
  cv::adaptiveThreshold(
    src_img,
    adaptive_img,
    255,
    cv::ADAPTIVE_THRESH_GAUSSIAN_C,
    cv::THRESH_BINARY,
    7,
    8
  );

  bytes_of_image = get_bytes_of_image(adaptive_img);

  itpp::bvec bits = itpp::bvec(bytes_of_image);

  std::cout << bits << '\n';

  cv::imshow("adaptive_img", adaptive_img);
  cv::waitKey(0);

  return 0;
}
