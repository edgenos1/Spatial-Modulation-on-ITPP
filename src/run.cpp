#include "./mimo/simulator.hpp"
#include "./encoder/encoder.hpp"


int main(int argc, char *argv[]) {
  int nTx;
  int nRx;
  int nC;
  char modulation[16];
  char inputName[256];
  char outputName[256];
  std::string strBits;
  itpp::bvec transmitBits, receivedBits;


  if (argc != 7) {
    std::cout << "Usage: ./sm nTx nRx nC Tc inputImageFilename OutputCSVFilename"
              << std::endl
              << "Example: ./sm 2 2 4 sm lena.bmp output.csv  (2x2 QPSK on fast fading)"
              << std::endl;
    exit(1);
  } else {
    sscanf(argv[1], "%i", &nTx);
    sscanf(argv[2], "%i", &nRx);
    sscanf(argv[3], "%i", &nC);
    sscanf(argv[4], "%s", modulation);
    sscanf(argv[5], "%s", inputName);
    sscanf(argv[6], "%s", outputName);
  }


  ImageEncoder encoder(inputName);
  strBits = encoder.get_bytes_of_image();

  transmitBits = encoder.convert(strBits);
  //transmitBits = itpp::randb(1e9);

  if (strcmp(modulation, "sm") == 0) {
    SMSimulator sm(nTx, nRx, nC, 1, outputName);
    sm.init(transmitBits);

    receivedBits = sm.simulate(30);
  } else {
    SMxSimulator smx(nTx, nRx, nC, 1, outputName);
    smx.init(transmitBits);

    receivedBits = smx.simulate(30);
  }

  encoder.decode(receivedBits);

	return 0;
}
