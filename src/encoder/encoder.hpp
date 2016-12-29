#ifndef SMOD_IMAGE_ENCODER_HPP
#define SMOD_IMAGE_ENCODER_HPP

#include <string>
#include <itpp/itcomm.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

class ImageEncoder
{
private:
  cv::Mat src_img;
  cv::Mat adaptive_img;
public:
  ImageEncoder(std::string inputFilename);
  virtual ~ImageEncoder() {};

  std::string get_bytes_of_image(cv::Mat src);
  itpp::bvec convert(std::string bytes);
};



#endif
