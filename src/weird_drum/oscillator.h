// WeirdDrums Oscillator — ported from dfilaretti/WeirdDrums (MIT)
// Simple sine/saw/square oscillator with phase accumulator.

#pragma once
#include <cmath>

namespace weird_drum {

enum Waveform { WAVE_SINE = 0, WAVE_SAW = 1, WAVE_SQUARE = 2 };

class Oscillator {
public:
    void init(float sample_rate) {
        sr_ = sample_rate;
        phase_ = 0.0f;
        phase_inc_ = 0.0f;
        freq_ = 440.0f;
    }

    void set_frequency(float freq) {
        freq_ = freq;
        phase_inc_ = freq_ / sr_;
    }

    void set_waveform(int w) {
        waveform_ = (Waveform)(w < 0 ? 0 : (w > 2 ? 2 : w));
    }

    void reset_phase() {
        phase_ = 0.0f;
    }

    float next() {
        float out = 0.0f;
        switch (waveform_) {
            case WAVE_SINE:
                out = sinf(phase_ * 6.2831853f);
                break;
            case WAVE_SAW:
                out = 2.0f * phase_ - 1.0f;
                break;
            case WAVE_SQUARE:
                out = phase_ < 0.5f ? 1.0f : -1.0f;
                break;
        }
        phase_ += phase_inc_;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        if (phase_ < 0.0f) phase_ += 1.0f;
        return out;
    }

private:
    float sr_ = 44100.0f;
    float phase_ = 0.0f;
    float phase_inc_ = 0.0f;
    float freq_ = 440.0f;
    Waveform waveform_ = WAVE_SINE;
};

}  // namespace weird_drum
