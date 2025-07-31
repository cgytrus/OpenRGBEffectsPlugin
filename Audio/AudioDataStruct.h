#ifndef AUDIODATASTRUCT_H
#define AUDIODATASTRUCT_H

namespace Audio
{
    struct AudioDataStruct
    {
        float waveform[512];

        float         fft[256];
        float         fft_nrml[256];
        float         fft_fltr[256] = { 0 };

        float         win_hanning[256];
        float         win_hamming[256];
        float         win_blackman[256];
};
}

#endif // AUDIODATASTRUCT_H
