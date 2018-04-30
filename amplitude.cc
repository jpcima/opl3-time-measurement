
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <tr1/cmath>

double harmonic_amplitudes(double *bands, unsigned band_count, unsigned opl3_fb)
{
    if (band_count <= 0)
        throw std::logic_error("Count out of range");
    if (opl3_fb > 7)
        throw std::logic_error("FB out of range");
    if (opl3_fb == 0) {
        bands[0] = 1;
        std::fill(bands + 1, bands + band_count, 0);
        return 1.0;
    }
    double fm_index = 1.0 / (1u << (9u - opl3_fb));
    double rms = 0;
    for (unsigned i = 0; i < band_count; ++i) {
        double amp = std::tr1::cyl_bessel_j(i, fm_index);
        if (bands) bands[i] = amp;
        rms += amp * M_SQRT1_2;
    }
    return rms;
}
