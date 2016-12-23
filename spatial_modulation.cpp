#include <iostream>
#include <itpp/itcomm.h>
#include <itpp/itbase.h>	// Addition (norm)
#include <itpp/itstat.h>	// Addition (norm)

using namespace itpp;
using namespace std;

// -- modulation and channel parameters (taken from command line input) --
int nC;                    // type of constellation  (1=QPSK, 2=16-QAM, 3=64-QAM)
int nRx;                   // number of receive antennas
int nTx;                   // number of transmit antennas
int Tc;                    // coherence time (number of channel vectors with same H)
char outputName[256];
// int nsnr;


cvec select_anntena(PSK chan, bvec bits, int nAntenna) {
	cvec s = zeros_c(nTx);	// アンテナ数

	int Nbits = length(bits);

	int selected_anntena = bin2dec(bits(0, nAntenna - 1));

	if (nAntenna == 0) {
		complex<double> symbol = chan.get_symbols()(bin2dec(bits(nAntenna, Nbits - 1)));
		s(0) = symbol;
	} else {
		complex<double> symbol = chan.get_symbols()(bin2dec(bits(nAntenna, Nbits - 1)));
		s(selected_anntena) = symbol;
	}

	return s;
}

Array<cvec> get_samples(PSK chan, int nSymbols) {
	cvec symbols = chan.get_symbols();
	Array<cvec> samples(nSymbols * nTx);
	cvec tmp;

	for (int i = 0; i < nTx; i++) {
	    for (int j = 0; j < nSymbols; j++) {
	        tmp = zeros_c(nTx);
	        tmp(i) = symbols(j);

	        samples(nSymbols * i + j) = tmp;
	    }
	}

	return samples;
}

vec sm_demodulator(cvec y, cmat h, cvec s, Array<cvec> samples, int nSymbols) {
	int i = 0, j, q, argmax_P;
	const double coef = pow(pi, -nRx);

	vec P(nTx * nSymbols);	// アンテナ数 * シンボル数
	vec j_q_pair(2);

	for (j = 0; j < nTx; j++) {	// アンテナ数
		for	(q = 0; q < nSymbols; q++) {	// シンボル数
			if (nTx == 1) {
				P(i++) = -norm(y - h * samples(nSymbols * j + q));
			} else {
				P(i++) = coef * exp(-norm(y - h * samples(nSymbols * j + q), "fro"));
			}

			// MRC
			// P(i++) = -norm(y - (pow(norm(h(0)), 2) + pow(norm(h(1)), 2)) * samples((nSymbols * j + q)));
		}
	}

	argmax_P = max_index(P);

	j_q_pair(0) = argmax_P / nSymbols;
	j_q_pair(1) = argmax_P % nSymbols;

	return j_q_pair;
}


int main(int argc, char **argv) {
	if (argc != 6) {
		cout << "Usage: cm nTx nRx nC Tc Output" << endl << "Example: cm 2 2 1 100000 output.csv (2x2 QPSK MIMO on slow fading channel)" << endl;
		exit(1);
	} else {
		sscanf(argv[1], "%i", &nTx);
		sscanf(argv[2], "%i", &nRx);
		sscanf(argv[3], "%i", &nC);
		sscanf(argv[4], "%i", &Tc);
		sscanf(argv[5], "%s", outputName);
	}

	cout << "Initializing.. " << nTx << " TX antennas, " << nRx << " RX antennas, "
		<< nC << " constellation, coherence time " << Tc << endl;

  /* --- simulation control parameters --- */
  const vec EbN0db = "0:1:30";    // SNR range
  const int Nmethods = 1;         // number of demodulators to try

  const int Nt = 1e6;          // number of times to simulate bits per SNR point

  /* --- output streams --- */
  ofstream ofs(outputName, ios::out);
  ofs << "SNR[dB],BER" << endl;   // CSV header

  /* ==================== Initialize ==================== */
  const int m = log2(nC * nTx);  // each transmitted bits at a particular time instance
  const int Nu = m * Nt;  // length of data packet (before applyng any channel coding)

  const int nAntenna = length(dec2bin(nTx)) - 1;   // if nTx=4: Nanntena=2, if nTx=8: Nanntena=3
  const int nSymbols = nC;
  const int nSymbin = length(dec2bin(nSymbols)) - 1;

  /* initialize MIMO channel with uniform QAM per complex dimension and Gray coding */
	PSK chan;
	chan.set_M(nSymbols);

  const int Nctx = Nu;        // Total number of bits to transmit
  const int Nvec = Nctx / m;  // Number of channel vectors to transmit
  const int Nbitspvec = m;    // Number of bits per channel vector

  cout << m << " " << Nu << " " << Nctx << " " << Nvec << " " << Nbitspvec << endl;

	ivec Contflag = ones_i(Nmethods);   // flag to determine whether to run a given demodulator

  cout << "Running methods: " << Contflag << endl;

	cout.setf(ios::fixed, ios::floatfield);
	cout.setf(ios::showpoint);
	cout.precision(5);

    /* ==================== Running simulation ==================== */
    for (int nsnr = 0; nsnr < length(EbN0db); nsnr++) {
				Array<cvec> Y(Nvec);        // received data
				Array<cmat> H(Nvec / Tc + 1);   // channel matrix (new matrix for each coherence interval)
				Array<cvec> S(Nvec);
				Array<cvec> Samples = get_samples(chan, nSymbols);

				const double Eb = 1.0;						// transmitted energy per information bit (1 / n)
				const double N0 = inv_dB(-EbN0db(nsnr));
				const double sigma2 = N0;					// Variance of each scalar complex noise sample
				// const double Es = 2 * nC * Eb;		        // Energy per complex scalar symbol
				// 											// (Each transmitted scalar complex symbol contains rate*2*nC
				// 											// information bits.)
				// const double Ess = sqrt(Es);
				const double Es = Eb / sigma2;
				const double Ess = sqrt(Es);

				Array<BERC> berc(Nmethods);			// counter for coded BER
				Array<BERC> bercu(Nmethods);		// counter for uncoded BER
				Array<BLERC> ferc(Nmethods);		// counter for coded FER

				for (int i = 0; i < Nmethods; i++) {
					ferc(i).set_blocksize(Nu);
				}

        bvec transmitted(Nctx);
        bvec received(Nctx);


        for (int k = 0; k < Nvec; k++) {
            if (k % Tc == 0) {  // generate a new channel realization every Tc intervals
                H(k / Tc) = randn_c(nRx, nTx);
            }

            /* --- modulate and transmit bits --- */
            bvec bitstmp = randb(Nbitspvec);

            S(k) = select_anntena(chan, bitstmp, nAntenna);

            cvec e = sqrt(sigma2) * randn_c(nRx);

            Y(k) = H(k / Tc) * S(k) + e;

            vec j_q_pair = sm_demodulator(Y(k), H(k / Tc), S(k), Samples, nSymbols);

            bvec transmittmp = bitstmp;
            bvec receivetmp = concat(
                dec2bin(nAntenna, (int)j_q_pair(0)),   // アンテナ, 2ビットにpadding
                dec2bin(nSymbin, (int)j_q_pair(1))    // シンボル, 2ビットにpadding
            );

            transmitted.set_subvector(k * Nbitspvec, transmittmp);
            received.set_subvector(k * Nbitspvec, receivetmp);
        }

        for (int i = 0; i < Nmethods; i++) {
            if (Contflag(i)) {
                bercu(i).count(transmitted(0, Nu - 1), received(0, Nu - 1));
            }
        }

		cout << "-----------------------------------------------------" << endl;
		cout << "Eb/N0: " << EbN0db(nsnr) << " dB. Simulated " << Nctx << " bits." << endl;
		cout << " Uncoded BER: " << bercu(0).get_errorrate() << " (ML)" << endl;
        ofs << dec << nsnr << "," << scientific << bercu(0).get_errorrate() << endl;

        cout.flush();
    }

    return 0;
}
