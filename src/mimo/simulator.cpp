#include "simulator.hpp"
#include <float.h>


void SMSimulator::init(itpp::bvec _inputBits) {
  inputBits = _inputBits;

  Nt = itpp::length(inputBits);

  ofs << "SNR[dB],BER" << std::endl;

  m = log2(nC * nTx);

  if (Nt % m != 0) {
    inputBits = itpp::concat(inputBits, itpp::zeros_b(Nt % m));
    Nt = itpp::length(inputBits);  // Refresh
  }

  Nu = Nt;
  /* !!!Warning!!! : nAntenna must be modified */
  nAntenna = itpp::length(itpp::dec2bin(nTx)) - 1;
  nSymbols = nC;
  nSymbin = itpp::length(itpp::dec2bin(nSymbols)) - 1;

  modulator.set_M(nSymbols);

  Nctx = Nu;
  Nvec = Nctx / m;
  Nbitspvec = m;

  std::cout << m << " " << Nu << " " << Nctx << " " << Nvec << " " << Nbitspvec << std::endl;

  return;
}


void SMxSimulator::init(itpp::bvec _inputBits) {
  inputBits = _inputBits;

  Nt = itpp::length(inputBits);

  ofs << "SNR[dB],BER" << std::endl;

  m = log2(nC) * nTx;

  if (Nt % m != 0) {
    inputBits = itpp::concat(inputBits, itpp::zeros_b(Nt % m));
    Nt = itpp::length(inputBits);  // Refresh
  }

  Nu = Nt;

  modulator.set_M(nTx, nC);

  Nctx = Nu;
  Nvec = Nctx / m;
  Nbitspvec = m;

  std::cout << m << " " << Nu << " " << Nctx << " " << Nvec << " " << Nbitspvec << std::endl;

  return;
}


itpp::cvec SMSimulator::select_antenna(itpp::bvec bits) {
  itpp::cvec s = itpp::zeros_c(nTx); // アンテナ数
  int Nbits = itpp::length(bits);
  int selected_antenna = itpp::bin2dec(bits(0, nAntenna - 1));

  std::complex<double> symbol = modulator.get_symbols()(itpp::bin2dec(bits(nAntenna, Nbits - 1)));

  if (nAntenna == 0) {
    s(0) = symbol;
  } else {
    s(selected_antenna) = symbol;
  }

  return s;
}


itpp::vec SMSimulator::demodulate_bits(itpp::cvec Y, itpp::cmat H, itpp::cvec S, itpp::Array<itpp::cvec> samples) {
  int i = 0, argmax_P;
  const double coef = pow(itpp::pi, -nRx);

  itpp::vec P(nTx * nSymbols);
  itpp::vec j_q_pair(2);

  for (int j = 0; j < nTx; j++) {
    for (int q = 0; q < nSymbols; q++) {
      if (nTx == 1) {
        P(i++) = -itpp::norm(Y - H * samples(nSymbols * j + q));
      } else {
        P(i++) = coef * exp(-itpp::norm(Y - H * samples(nSymbols * j + q), "fro"));
      }
    }
  }

  argmax_P = itpp::max_index(P);

  j_q_pair(0) = argmax_P / nSymbols;
  j_q_pair(1) = argmax_P % nSymbols;

  return j_q_pair;
}


itpp::Array<itpp::cvec> SMSimulator::get_samples() {
  itpp::cvec symbols = modulator.get_symbols();
  itpp::Array<itpp::cvec> samples(nSymbols * nTx);
  itpp::cvec tmp;

  for (int i = 0; i < nTx; i++) {
    for (int j = 0; j < nSymbols; j++) {
      tmp = itpp::zeros_c(nTx);
      tmp(i) = symbols(j);

      samples(nSymbols * i + j) = tmp;
    }
  }

  return samples;
}

itpp::bvec SMSimulator::simulate(int TotalNumSimulate) {
  std::cout.setf(std::ios::fixed, std::ios::floatfield);
  std::cout.setf(std::ios::showpoint);
  //std::cout.setf(std::ios::scientific, std::ios::floatfield);
  std::cout.precision(10);

  for (int nsnr = 0; nsnr <= TotalNumSimulate; nsnr++) {
    itpp::cvec Y;
    itpp::cmat H;
    itpp::cvec S;
    itpp::cvec e;
    itpp::Array<itpp::cvec> Samples = SMSimulator::get_samples();

    const double sigma2 = itpp::inv_dB(-nsnr);

    itpp::BERC bercu;

    itpp::bvec bitstmp;

    itpp::vec j_q_pair;
    itpp::bvec transmittmp;
    itpp::bvec receivetmp;

    itpp::bvec transmitted(Nctx);
    itpp::bvec received(Nctx);

    for (int k = 0; k < Nvec; k++) {
      H = itpp::randn_c(nRx, nTx);

      bitstmp = inputBits(k * Nbitspvec, (k + 1) * Nbitspvec - 1);

      S = SMSimulator::select_antenna(bitstmp);

      e = sqrt(sigma2) * itpp::randn_c(nRx);

      Y = H * S + e;

      j_q_pair = SMSimulator::demodulate_bits(Y, H, S, Samples);

      transmittmp = bitstmp;
      receivetmp = itpp::concat(itpp::dec2bin(nAntenna, static_cast<int>(j_q_pair(0))),
                                itpp::dec2bin(nSymbin, static_cast<int>(j_q_pair(1)))
                                );

      transmitted.set_subvector(k * Nbitspvec, transmittmp);
      received.set_subvector(k * Nbitspvec, receivetmp);
    }

    bercu.count(transmitted(0, Nu - 1), received(0, Nu - 1));

    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << "Eb/N0: " << nsnr << " dB. Simulated " << Nctx << " bits." << std::endl;
    //std::cout << " Uncoded BER: " << error_rate << " (ML)" << std::endl;
    std::cout << " Uncoded BER: " << bercu.get_errorrate() << " (ML)" << std::endl;
    ofs << std::dec << nsnr << "," << std::scientific << bercu.get_errorrate() << std::endl;

    // Debug
    return received;
  }

  return itpp::bvec(1);
}


itpp::bvec SMxSimulator::simulate(int TotalNumSimulate) {
  const int Nmethods = 1;

  std::cout.setf(std::ios::fixed, std::ios::floatfield);
  std::cout.setf(std::ios::showpoint);
  std::cout.precision(10);

  for (int nsnr = 0; nsnr <= TotalNumSimulate; nsnr++) {
    itpp::Array<itpp::cvec> Y(Nvec);
    itpp::Array<itpp::cmat> H(Nvec + 1);
    itpp::cvec e;

    const double sigma2 = itpp::inv_dB(-nsnr);

    itpp::BERC bercu;

    itpp::bvec bitstmp;

    itpp::bvec received(Nu);

    for (int k = 0; k < Nvec; k++) {
      H(k) = itpp::randn_c(nRx, nTx);

      bitstmp = inputBits(k * Nbitspvec, (k + 1) * Nbitspvec - 1);

      e = sqrt(sigma2) * itpp::randn_c(nRx);

      //std::cout << length(bitstmp) << " " << sum(modulator.get_k()) << "\n";
      itpp::cvec x = modulator.modulate_bits(bitstmp);

      Y(k) = H(k) * x + e;

      //std::cout << modulator(Y(k)) << std::endl;
      /*
      received.set_subvector(k * Nbitspvec,
                             modulator.get_symbols()(0)(itpp::bin2dec(Y(k)))
                             );
      */
    }

    // -- demodulate --
    itpp::Array<itpp::QLLRvec> LLRin(Nmethods);
    for (int i = 0; i < Nmethods; i++) {
      LLRin(i) = itpp::zeros_i(Nctx);
    }

    itpp::QLLRvec llr_apr = itpp::zeros_i(Nbitspvec);
    itpp::QLLRvec llr_apost = itpp::zeros_i(Nbitspvec);

    for (int k = 0; k < Nvec; k++) {
      // ML demodulation
      modulator.demodulate_soft_bits(Y(k), H(k), sigma2, llr_apr, llr_apost);
      LLRin(0).set_subvector(k * Nbitspvec, llr_apost);
    }

    // -- decode and count errors --
    for (int i = 0; i < Nmethods; i++) {
      bercu.count(inputBits(0, Nu - 1), LLRin(0)(0, Nu - 1) < 0);
    }

    // -- received bits --
    for (int i = 0; i < Nu; i++) {
      received(i) = LLRin(0)(i) < 0;
    }


    //bercu.count(transmitted(0, Nu - 1), received(0, Nu - 1));

    std::cout << "-----------------------------------------------------" << std::endl;
    std::cout << "Eb/N0: " << nsnr << " dB. Simulated " << Nctx << " bits." << std::endl;
    //std::cout << " Uncoded BER: " << error_rate << " (ML)" << std::endl;
    std::cout << " Uncoded BER: " << bercu.get_errorrate() << " (ML)" << std::endl;
    ofs << std::dec << nsnr << "," << std::scientific << bercu.get_errorrate() << std::endl;

    // Debug
    return received;
  }

  return itpp::bvec(1);
}
