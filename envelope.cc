#include "envelope.h"
#include <cstdint>
#include <cassert>
#include <cmath>

static const uint8_t kslrom[16] = {
    0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
};

static const uint8_t kslshift[4] = {
    8, 1, 2, 0
};

static unsigned effective_rate(unsigned rate, unsigned ksr, unsigned nts, unsigned fnum, unsigned block)
{
    unsigned effective_rate = 4 * rate;
    if (!ksr)
        effective_rate += block >> 1;
    else
        effective_rate += (block << 1) | ((fnum >> (9 - nts)) & 1);
    return effective_rate;
}

static constexpr double attack_a = 1.149779557179130,
                        attack_b = 1.189578077537087,
                        attack_c = -1.771203939904738;

double attack_value(double t, const Eg_Parameters &egp)
{
    double tlv = (egp.tl << 2) / 512.0;
    double kslv = (((kslrom[egp.fnum >> 6] << 2) - ((8 - egp.block) << 5)) >> kslshift[egp.ksl]) / 512.0;
    unsigned rate = egp.ar;
    double value;
    if (rate <= 0)
        value = 0;
    else if (rate >= 15)
        value = 1;
    else {
        unsigned eff_rate = effective_rate(rate, egp.ksr, egp.nts, egp.fnum, egp.block);
        value = std::exp(-attack_a*std::pow(attack_b, eff_rate + attack_c) * t);
    }
    return value - tlv - kslv;
}

double attack_time(double v, const Eg_Parameters &egp)
{
    double tlv = (egp.tl << 2) / 512.0;
    double kslv = (((kslrom[egp.fnum >> 6] << 2) - ((8 - egp.block) << 5)) >> kslshift[egp.ksl]) / 512.0;
    v = v + tlv + kslv;
    if (v <= 0)
        return 0;
    unsigned rate = egp.ar;
    unsigned eff_rate = effective_rate(rate, egp.ksr, egp.nts, egp.fnum, egp.block);
    return std::log(1.0 - v) / (-attack_a * std::pow(attack_b, eff_rate + attack_c));
}

double wave_rms(unsigned wave)
{
    static constexpr double opl3_wave_rms[8] = {
        0.70561416881800276, 0.38471346311423527,
        0.30705309587280949, 0.38471346311423527,
        0.49798248543363655, 0.38426129383516933,
        0.99767965006992609, 0.21419520618172677,
    };
    return opl3_wave_rms[wave & 7];
}

double solve_attack(
    double vrms, const Eg_Parameters egp[], unsigned egcount)
{
    constexpr unsigned egmax = 6; /* 2 notes, 3 carriers */
    assert(egcount <= egmax);

    double arg[egmax];
    for (unsigned i = 0; i < egcount; ++i) {
        unsigned eff_rate = effective_rate(egp[i].ar, egp[i].ksr, egp[i].nts, egp[i].fnum, egp[i].block);
        arg[i] = -attack_a * std::pow(attack_b, eff_rate + attack_c);
    }

    auto evaluate =
        [&arg, &egp, egcount](double t) -> double
            {
                double v = 0.0;
                for (unsigned i = 0; i < egcount; ++i) {
                    double e = 1.0 - std::exp(t * arg[i]);
                    double w = e * wave_rms(egp[i].wave);
                    v += w * w;
                }
                return std::sqrt(v);
            };

    double t1 = 0.0, t2 = 10.0;
    double v1 = evaluate(t1), v2 = evaluate(t2);
    double t = (t2 - t1) * 0.5;

    constexpr unsigned iterations = 16;
    for (unsigned i = 0; i < iterations; ++i) {
        double v = evaluate(t);
        if (vrms < v) { t2 = t; v2 = v; }
        else { t1 = t; v1 = v; }
        t = (t2 - t1) * 0.5;
    }
    return t;
}
