#include "encoder.hpp"

ImageEncoder::ImageEncoder(std::string inputFilename) {
  src_img = cv::imread(inputFilename, 0);

  cv::adaptiveThreshold(src_img, adaptive_img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 7, 8);
}

std::string ImageEncoder::get_bytes_of_image(cv::Mat src) {
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

itpp::bvec ImageEncoder::convert(std::string bytes) {
  return itpp::bvec(bytes);
}
