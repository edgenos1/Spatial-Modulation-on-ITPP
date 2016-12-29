#include "encoder.hpp"

ImageEncoder::ImageEncoder(std::string inputFilename) {
  src_img = cv::imread(inputFilename, cv::IMREAD_GRAYSCALE);

  cv::adaptiveThreshold(src_img, adaptive_img, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 7, 8);

  cv::imshow("transmit", adaptive_img);
}

std::string ImageEncoder::get_bytes_of_image() {
  std::string dst;
  uchar *p;

  for (int y = 0; y < adaptive_img.rows; y++) {
    p = adaptive_img.ptr<uchar>(y);
    for (int x = 0; x < adaptive_img.cols; x++) {
      dst += (p[x] == 255) ? "1" : "0";
      dst += " ";
    }
  }

  dst.pop_back();  // 最後の空白を除去

  return dst;
}

itpp::bvec ImageEncoder::convert(std::string bytes) {
  return itpp::bvec(bytes);
}


void ImageEncoder::decode(itpp::bvec received) {
  cv::Mat dst = cv::Mat::zeros(adaptive_img.rows, adaptive_img.cols, cv::IMREAD_GRAYSCALE);

  for (int y = 0; y < adaptive_img.rows; y++) {
    for (int x = 0; x < adaptive_img.cols; x++) {
      dst.ptr<uchar>(y)[x] = (int)received(adaptive_img.cols * y + x) * 255;
    }
  }

  cv::imshow("receive", dst);
  cv::waitKey(0);
}
