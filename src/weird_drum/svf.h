// State Variable Filter — Hal Chamberlin / Andrew Simper style
// LP, HP, BP modes for noise section filtering.

#pragma once
#include <cmath>

namespace weird_drum {

enum FilterType { FILTER_LP = 0, FILTER_HP = 1, FILTER_BP = 2 };

class SVF {
public:
    void init(float sample_rate) {
        sr_ = sample_rate;
        reset();
    }

    void set_type(int t) {
        type_ = (FilterType)(t < 0 ? 0 : (t > 2 ? 2 : t));
    }

    void set_cutoff(float freq) {
        // Clamp to Nyquist
        if (freq > sr_ * 0.49f) freq = sr_ * 0.49f;
        if (freq < 20.0f) freq = 20.0f;
        f_ = 2.0f * sinf(3.1415926f * freq / sr_);
    }

    void set_resonance(float res) {
        // res: 0.1 to 5.0 (Q-like)
        // damp = 1/Q, but we want higher res values = more resonance
        if (res < 0.1f) res = 0.1f;
        damp_ = 1.0f / res;
    }

    void reset() {
        lp_ = 0.0f;
        bp_ = 0.0f;
    }

    float process(float input) {
        // Two-pass for better stability at high frequencies
        for (int i = 0; i < 2; i++) {
            lp_ += f_ * 0.5f * bp_;
            float hp = input - lp_ - damp_ * bp_;
            bp_ += f_ * 0.5f * hp;
        }

        switch (type_) {
            case FILTER_LP: return lp_;
            case FILTER_HP: return input - lp_ - damp_ * bp_;
            case FILTER_BP: return bp_;
        }
        return lp_;
    }

private:
    float sr_ = 44100.0f;
    float f_ = 0.1f;
    float damp_ = 1.0f;
    float lp_ = 0.0f;
    float bp_ = 0.0f;
    FilterType type_ = FILTER_LP;
};

}  // namespace weird_drum
