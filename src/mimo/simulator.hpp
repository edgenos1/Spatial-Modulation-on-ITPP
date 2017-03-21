#ifndef SMOD_SIMULATOR_HPP
#define SMOD_SIMULATOR_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <itpp/itcomm.h>
#include <itpp/itbase.h>
#include <itpp/itstat.h>


class SMxSimulator
{
private:
  const int nTx;
  const int nRx;
  const int nC;
  const int Tc;

  std::ofstream ofs;

  int Nt;
  int m;
  int Nu;

  itpp::ND_UPSK modulator;

  int Nctx;
  int Nvec;
  int Nbitspvec;

  itpp::bvec inputBits;

public:
  SMxSimulator(
              int _nTx,
              int _nRx,
              int _nC,
              int _Tc,
              char *outputName
              ) : nTx(_nTx),
                  nRx(_nRx),
                  nC(_nC),
                  Tc(_Tc),
                  ofs(outputName, std::ios::out){};
  virtual ~SMxSimulator(){};

  void init(itpp::bvec _inputBits);
  itpp::bvec simulate(int TotalNumSimulate);
};


class SMSimulator
{
private:
  const int nTx;
  const int nRx;
  const int nC;
  const int Tc;

  std::ofstream ofs;

  int Nt;
  int m;
  int Nu;
  int nAntenna;
  int nSymbols;
  int nSymbin;

  itpp::PSK modulator;

  int Nctx;
  int Nvec;
  int Nbitspvec;

  itpp::bvec inputBits;

public:
  SMSimulator(
              int _nTx,
              int _nRx,
              int _nC,
              int _Tc,
              char *outputName
              ) : nTx(_nTx),
                  nRx(_nRx),
                  nC(_nC),
                  Tc(_Tc),
                  ofs(outputName, std::ios::out){};
  virtual ~SMSimulator(){};

  void init(itpp::bvec _inputBits);
  itpp::cvec select_antenna(itpp::bvec bits);
  itpp::vec demodulate_bits(itpp::cvec Y,
                            itpp::cmat H,
                            itpp::cvec S,
                            itpp::Array<itpp::cvec> samples
                            );
  itpp::Array<itpp::cvec> get_samples(void);
  itpp::bvec simulate(int TotalNumSimulate);
};

#endif
