
#include "nukedopl3_eg.h"
#include <getopt.h>
#include <iostream>

Bit16s OPL3_EnvelopeCalcSin0(Bit16u phase, Bit16u envelope);

struct Eg_Parameters
{
    int ar = 7; /* 0-15 */
    int dr = 7; /* 0-15 */
    int sl = 7; /* 0-15 */
    int rr = 7; /* 0-15 */
    int tl = 0; /* 0-63 output attenuation */
    int nts = false;
    int ksr = false;
    int ksl = 0; /* 0-3 */
    bool sustained = false;
};

std::ostream &operator<<(std::ostream &o, const Eg_Parameters &p)
{
    return o
        <<  "ar=" << p.ar << " dr=" << p.dr
        << " sl=" << p.sl << " rr=" << p.rr
        << " tl=" << p.tl << " nts=" << p.nts
        << " ksr=" << p.ksr << " ksl=" << p.ksl
        << " sustained=" << p.sustained;
}

void init(opl3_chip &chip, const Eg_Parameters &egp, int f_num, int block)
{
    OPL3_Reset(&chip);

    //
    chip.f_num = f_num;
    chip.block = block;
    chip.ksv = (chip.block << 1) |
        ((chip.f_num >> (0x09 - egp.nts)) & 0x01);
    OPL3_EnvelopeUpdateKSL(&chip);

    //
    chip.reg_ar = egp.ar;
    chip.reg_dr = egp.dr;
    chip.reg_sl = egp.sl;
    chip.reg_rr = egp.rr;
    chip.reg_tl = egp.tl;
    chip.reg_ksr = egp.ksr;
    chip.reg_ksl = egp.ksl;
    chip.reg_type = egp.sustained;
}

int tick(opl3_chip &chip)
{
    OPL3_EnvelopeCalc(&chip);
    int out = chip.eg_out;
    OPL3_Tick(&chip);
    return out;
}

#include <stdio.h>
#include <string>
#include <memory>

struct FILE_deleter { void operator()(FILE *x) { fclose(x); } };
typedef std::unique_ptr<FILE, FILE_deleter> unique_FILE;

int main(int argc, char *argv[])
{
    Eg_Parameters egp;

    int fnum = 512;
    int block = 4;

    static struct option long_options[] = {
        {"attack", required_argument, nullptr, 'a'},
        {"decay", required_argument, nullptr, 'd'},
        {"sustain", required_argument, nullptr, 's'},
        {"release", required_argument, nullptr, 'r'},
        {"fnum", required_argument, nullptr, 'fnum'},
        {"block", required_argument, nullptr, 'fblk'},
        {"tl", required_argument, nullptr, 'tl  '},
        {"nts", required_argument, nullptr, 'nts '},
        {"ksl", required_argument, nullptr, 'ksl '},
        {"ksr", required_argument, nullptr, 'ksr '},
        {},
    };

    for (int c; (c = getopt_long(argc, argv, "a:d:s:r:", long_options, nullptr)) != -1;) {
        switch (c) {
        case 'a':
            egp.ar = std::atoi(optarg); break;
        case 'd':
            egp.dr = std::atoi(optarg); break;
        case 's':
            egp.sl = std::atoi(optarg); break;
        case 'r':
            egp.rr = std::atoi(optarg); break;
        case 'fnum':
            fnum = std::atoi(optarg); break;
        case 'fblk':
            block = std::atoi(optarg); break;
        case 'tl  ':
            egp.tl = std::atoi(optarg); break;
        case 'nts ':
            egp.nts = std::atoi(optarg); break;
        case 'ksl ':
            egp.ksl = std::atoi(optarg); break;
        case 'ksr ':
            egp.ksr = std::atoi(optarg); break;
        default:
            return 1;
        }
    }

    if (egp.ar < 0 || egp.ar > 15) {
        fprintf(stderr, "invalid attack value (0-15)\n");
        return 1;
    }
    if (egp.dr < 0 || egp.dr > 15) {
        fprintf(stderr, "invalid decay value (0-15)\n");
        return 1;
    }
    if (egp.sl < 0 || egp.sl > 15) {
        fprintf(stderr, "invalid sustain value (0-15)\n");
        return 1;
    }
    if (egp.rr < 0 || egp.rr > 15) {
        fprintf(stderr, "invalid release value (0-15)\n");
        return 1;
    }
    if (egp.tl < 0 || egp.tl > 63) {
        fprintf(stderr, "invalid tl value (0-63)\n");
        return 1;
    }
    if (egp.nts < 0 || egp.nts > 1) {
        fprintf(stderr, "invalid nts value (0-1)\n");
        return 1;
    }
    if (egp.ksr < 0 || egp.ksr > 1) {
        fprintf(stderr, "invalid ksr value (0-1)\n");
        return 1;
    }
    if (egp.ksl < 0 || egp.ksl > 3) {
        fprintf(stderr, "invalid ksl value (0-3)\n");
        return 1;
    }
    if (fnum < 0 || fnum > 1023) {
        fprintf(stderr, "invalid fnum value (0-1023)\n");
        return 1;
    }
    if (block < 0 || block > 7) {
        fprintf(stderr, "invalid block value (0-7)\n");
        return 1;
    }

    std::cerr << "Measure parameters: fnum=" << fnum << " block=" << block << "\n";
    std::cerr << "EG parameters: " << egp << "\n";
    std::cerr << "--------------------------\n";

    //
    opl3_chip chip;
    init(chip, egp, fnum, block);

    unique_FILE adsr_outputs[4];
    const char *adsr_names[4] = {"attack", "decay", "sustain", "release"};
    const char *adsr_filenames[4] = {"data/eg_attack.dat", "data/eg_decay.dat", "data/eg_sustain.dat", "data/eg_release.dat"};

    for (unsigned i = 0; i < 4; ++i) {
        adsr_outputs[i].reset(fopen(adsr_filenames[i], "w"));
        if (!adsr_outputs[i]) {
            fprintf(stderr, "error opening output file\n");
            return 1;
        }
    }

    size_t adsr_phasecount[4] = {0, 0, 0, 0};
    size_t adsr_phase_start[4] = {0, 0, 0, 0};
    size_t adsr_phase_end[4] = {0, 0, 0, 0};
    double adsr_phase_startval[4] = {0, 0, 0, 0};
    double adsr_phase_endval[4] = {0, 0, 0, 0};
    size_t sample_index = 0;

    //
    constexpr double sample_rate = 49716.0;
    OPL3_EnvelopeKeyOn(&chip, egk_norm);
    int eg_sample;
    int eg_gen_old = -1;
    int eg_gen = -1;
    do {
        eg_sample = tick(chip);
        eg_gen_old = eg_gen;
        eg_gen = chip.eg_gen;

        if (!egp.sustained && eg_gen == envelope_gen_num_sustain)
            eg_gen = envelope_gen_num_release;
        double eg_value = (511 - eg_sample) / 512.0;

        if (eg_gen < eg_gen_old)
            throw std::logic_error("bad envelope progression");

        for (int i = std::max(0, eg_gen_old); i < eg_gen; ++i) {
            adsr_phase_end[i] = sample_index;
            adsr_phase_endval[i] = eg_value;
        }
        for (int i = eg_gen_old + 1; i <= eg_gen; ++i) {
            adsr_phase_start[i] = sample_index;
            adsr_phase_startval[i] = eg_value;
        }

        ++adsr_phasecount[eg_gen];
        // fprintf(stderr, "i=%zu eg=%d sample=%d\n", i, eg_gen, eg_sample);

        fprintf(adsr_outputs[eg_gen].get(), "%f %f\n", sample_index / sample_rate, eg_value);
        ++sample_index;
    } while (eg_gen < envelope_gen_num_release || 511 - eg_sample > 0);
    adsr_phase_end[envelope_gen_num_release] = sample_index;
    adsr_phase_endval[envelope_gen_num_release] = 0;

    for (unsigned i = 0; i < 4; ++i) {
        if (fflush(adsr_outputs[i].get()) != 0) {
            fprintf(stderr, "error writing file\n");
            return 1;
        }
    }

    for (unsigned i = 0; i < 4; ++i) {
        double dt = adsr_phasecount[i] / sample_rate;
        double t1 = adsr_phase_start[i] / sample_rate;
        double t2 = adsr_phase_end[i] / sample_rate;
        double y1 = adsr_phase_startval[i];
        double y2 = adsr_phase_endval[i];
        printf("%s time=%f start=%f@%f end=%f@%f\n",
               adsr_names[i], dt, t1, y1, t2, y2);
    }

    if (system("gnuplot data/eg.gp") != 0) {
        fprintf(stderr, "error starting gnuplot\n");
        return 1;
    }

    return 0;
}
