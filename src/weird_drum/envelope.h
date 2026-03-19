// WeirdDrums AD Envelope — ported from dfilaretti/WeirdDrums (MIT)
// Exponential attack-decay envelope for percussive sounds.

#pragma once
#include <cmath>
#include <cstddef>

namespace weird_drum {

struct EnvelopeParams {
    float attack = 0.001f;   // seconds
    float decay  = 0.5f;     // seconds
};

class Envelope {
public:
    void init(float sample_rate) {
        sr_ = sample_rate;
        reset();
    }

    void set_params(const EnvelopeParams &p) {
        params_ = p;
        recalc();
    }

    void note_on() {
        current_sample_ = 0;
        envelope_val_ = kMinValue;
        if (attack_mult_ > 0.0)
            state_ = kAttack;
        else if (decay_mult_ > 0.0)
            state_ = kDecay;
    }

    void note_off() {
        // AD envelope — one-shot, no release needed for drums
    }

    bool is_active() const { return state_ != kIdle; }

    float next() {
        if (state_ == kIdle)
            return 0.0f;

        if (state_ == kAttack) {
            envelope_val_ *= attack_mult_;
            current_sample_++;
            if (current_sample_ >= attack_len_) {
                envelope_val_ = 1.0;
                current_sample_ = 0;
                state_ = kDecay;
            }
        } else if (state_ == kDecay) {
            envelope_val_ *= decay_mult_;
            current_sample_++;
            if (current_sample_ >= decay_len_) {
                reset();
            }
        }

        return (float)envelope_val_;
    }

    void reset() {
        envelope_val_ = kMinValue;
        current_sample_ = 0;
        state_ = kIdle;
    }

private:
    static constexpr double kMinValue = 0.0001;

    enum State { kIdle, kAttack, kDecay };
    State state_ = kIdle;

    EnvelopeParams params_;
    double sr_ = 44100.0;
    double envelope_val_ = kMinValue;
    size_t current_sample_ = 0;
    size_t attack_len_ = 0;
    size_t decay_len_ = 0;
    double attack_mult_ = 0.0;
    double decay_mult_ = 0.0;

    static double calc_multiplier(double start, double end, size_t len) {
        if (len == 0) return 1.0;
        return 1.0 + (log(end) - log(start)) / (double)len;
    }

    void recalc() {
        attack_len_ = (size_t)(params_.attack * sr_);
        decay_len_  = (size_t)(params_.decay  * sr_);
        if (attack_len_ < 1) attack_len_ = 1;
        if (decay_len_  < 1) decay_len_  = 1;
        attack_mult_ = calc_multiplier(kMinValue, 1.0, attack_len_);
        decay_mult_  = calc_multiplier(1.0, kMinValue, decay_len_);
    }
};

}  // namespace weird_drum
