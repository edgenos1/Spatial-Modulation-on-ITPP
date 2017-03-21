#include "./mimo/simulator.hpp"
#include "./encoder/encoder.hpp"


int main(int argc, char *argv[]) {
  int nTx;
  int nRx;
  int nC;
  int nsnr;
  char modulation[16];
  char inputName[256];
  char outputName[256];
  std::string strBits;
  itpp::bvec transmitBits, receivedBits;


  if (argc != 8) {
    std::cout << "Usage: ./simulator [Number of antennas to transmit] [Number of antennas to receive] [Constellation size] [dB] [sm|smx] [Image file to transmit] [Output csv]"
              << std::endl
              << "Example: ./simulator 2 2 4 10 sm lena.bmp output.csv  (2x2 QPSK Spatial Modulation at 10dB)"
              << std::endl
              << "Example: ./simulator 4 2 8 0 smx lena.bmp output.csv  (4x2 8PSK Spatial Multiplexing at 0dB)"
              << std::endl;
    exit(1);
  } else {
    sscanf(argv[1], "%i", &nTx);
    sscanf(argv[2], "%i", &nRx);
    sscanf(argv[3], "%i", &nC);
    sscanf(argv[4], "%i", &nsnr);
    sscanf(argv[5], "%s", modulation);
    sscanf(argv[6], "%s", inputName);
    sscanf(argv[7], "%s", outputName);
  }


  ImageEncoder encoder(inputName);
  strBits = encoder.get_bytes_of_image();

  transmitBits = encoder.convert(strBits);

  if (strcmp(modulation, "sm") == 0) {
    SMSimulator sm(nTx, nRx, nC, 1, outputName);
    sm.init(transmitBits);

    receivedBits = sm.simulate(nsnr);
  } else {
    SMxSimulator smx(nTx, nRx, nC, 1, outputName);
    smx.init(transmitBits);

    receivedBits = smx.simulate(nsnr);
  }

  encoder.decode(receivedBits);

	return 0;
}
