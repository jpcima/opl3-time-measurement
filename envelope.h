#pragma once

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
    unsigned wave = 0; /* 0-7 */
    unsigned fnum = 0; /* 0-1023 */
    unsigned block = 0; /* 0-7 */
};

double attack_value(double t, const Eg_Parameters &egp);
double attack_time(double v, const Eg_Parameters &egp);

double wave_rms(unsigned wave);

double solve_attack(
    double vrms, const Eg_Parameters egp[], unsigned egcount);
