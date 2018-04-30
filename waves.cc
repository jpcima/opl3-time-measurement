
#include "opl3_waves.h"
#include "envelope.h"
#include <getopt.h>
#include <algorithm>
#include <numeric>
#include <memory>
#include <cmath>
#include <cstdio>

template <class T> static T square(T x) { return x * x; }

void wave_period(unsigned w, double *s, unsigned n, double a)
{
    for (unsigned i = 0; i < n; ++i)
        s[i] = a * OPL3_Waves::compute_wave(w, i / (double)n);
}

double period_rms(const double *s, unsigned n)
{
    double sum = 0.0;
    for (unsigned i = 0; i < n; ++i)
        sum += s[i];
    double avg = sum / n;
    double rms = 0.0;
    for (unsigned i = 0; i < n; ++i)
        rms += square(s[i] - avg);
    rms /= n - 1;
    rms = std::sqrt(rms);
    return rms;
}

double compute_rms(unsigned wave, double amplitude)
{
    constexpr unsigned nsamples = 1024;
    double samples[nsamples];
    wave_period(wave, samples, nsamples, amplitude);
    return period_rms(samples, nsamples);
}

double ref_rms(double amplitude, unsigned nsamples)
{
    std::unique_ptr<double[]> samples(new double[nsamples]);
    for (unsigned i = 0; i < nsamples; ++i)
        samples[i] = std::sin(2.0 * M_PI * i / nsamples);
    return period_rms(samples.get(), nsamples);
}

int main(int argc, char *argv[])
{
    // unsigned wave = 0;

    for (int c; (c = getopt(argc, argv, "w:")) != -1;) {
        switch (c) {
        default:
            return 1;
        // case 'w':
        //     wave = boost::lexical_cast<unsigned>(optarg);
        //     break;
        }
    }

    if (optind != argc)
        return 1;

    fprintf(stderr, "RMS ref: %.17f (%.17f)\n",
            ref_rms(1.0, 1024), M_SQRT1_2);

    for (unsigned w = 0; w < 8; ++w)
        printf("%u %.17f\n", w, wave_rms(w));

    return 0;
}
