// WeirdDrums Voice — ported from dfilaretti/WeirdDrums (MIT)
// Combines oscillator + noise + filter + distortion into a drum voice.

#pragma once
#include <cmath>
#include <cstdint>
#include "envelope.h"
#include "oscillator.h"
#include "svf.h"

namespace weird_drum {

// Simple xorshift32 RNG for noise generation (no heap, no JUCE)
struct NoiseGen {
    uint32_t state = 0x12345678;

    void seed(uint32_t s) { state = s ? s : 1; }

    float next() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        // Convert to float in [-1, 1]
        return (float)(int32_t)state * (1.0f / 2147483648.0f);
    }
};

// Parameters shared across all voices
struct DrumParams {
    // Tone
    float freq           = 55.0f;
    float attack         = 0.001f;
    float decay          = 0.5f;
    int   wave_type      = 0;
    float pitch_env_amt  = 0.0f;
    float pitch_env_rate = 0.1f;
    float pitch_lfo_amt  = 0.0f;
    float pitch_lfo_rate = 0.45f;
    // Noise
    int   filter_type    = 0;
    float filter_cutoff  = 400.0f;
    float filter_res     = 1.0f;
    float noise_attack   = 0.01f;
    float noise_decay    = 0.4f;
    // Master
    float mix            = 0.5f;
    float distortion     = 0.0f;
    float level          = 0.0f;  // dB
};

class Voice {
public:
    void init(float sample_rate, uint32_t noise_seed) {
        sr_ = sample_rate;
        osc_.init(sample_rate);
        osc_amp_env_.init(sample_rate);
        osc_pitch_env_.init(sample_rate);
        noise_amp_env_.init(sample_rate);
        filter_.init(sample_rate);
        noise_.seed(noise_seed);
        lfo_phase_ = 0.0f;
        active_ = false;
        age_ = 0;
    }

    bool is_active() const { return active_; }
    uint32_t age() const { return age_; }

    void note_on(float velocity, const DrumParams &p) {
        velocity_ = velocity;
        age_ = 0;

        // Configure oscillator
        osc_.set_waveform(p.wave_type);
        osc_.reset_phase();
        osc_.set_frequency(p.freq);
        base_freq_ = p.freq;

        // Configure envelopes
        osc_amp_env_.set_params({p.attack, p.decay});
        osc_amp_env_.note_on();

        osc_pitch_env_.set_params({0.001f, p.pitch_env_rate});
        osc_pitch_env_.note_on();

        noise_amp_env_.set_params({p.noise_attack, p.noise_decay});
        noise_amp_env_.note_on();

        // Configure filter
        filter_.set_type(p.filter_type);
        filter_.set_cutoff(p.filter_cutoff);
        filter_.set_resonance(p.filter_res);

        // Store params for per-sample use
        pitch_env_amt_ = p.pitch_env_amt;
        pitch_lfo_amt_ = p.pitch_lfo_amt;
        lfo_freq_ = p.pitch_lfo_rate;
        lfo_phase_ = 0.0f;

        osc_gain_ = 1.0f - p.mix;
        noise_gain_ = p.mix;
        distortion_pre_ = p.distortion;
        master_gain_ = powf(10.0f, p.level * 0.05f);

        active_ = true;
    }

    float render() {
        if (!active_) return 0.0f;
        age_++;

        // Oscillator amplitude envelope
        float amp_env = osc_amp_env_.next();
        // Noise amplitude envelope
        float noise_env = noise_amp_env_.next();

        // Check if both envelopes are done
        if (!osc_amp_env_.is_active() && !noise_amp_env_.is_active()) {
            active_ = false;
            return 0.0f;
        }

        // Pitch LFO
        float lfo_out = sinf(lfo_phase_ * 6.2831853f);
        lfo_phase_ += lfo_freq_ / sr_;
        if (lfo_phase_ >= 1.0f) lfo_phase_ -= 1.0f;

        // Pitch modulation: LFO + pitch envelope
        // From original: freq * pow(2, 1/1200 + lfoOut * lfoAmount)
        float freq = base_freq_ * powf(2.0f, (1.0f / 1200.0f + lfo_out * pitch_lfo_amt_));

        // Pitch envelope: map from fMin to fMax
        float pitch_env = osc_pitch_env_.next();
        float f_min = freq;
        float f_max = f_min + 1000.0f * pitch_env_amt_;
        if (f_max > 20000.0f) f_max = 20000.0f;
        freq = f_min + pitch_env * (f_max - f_min);

        osc_.set_frequency(freq);

        // Oscillator output
        float osc_out = osc_.next() * amp_env * velocity_;

        // Noise output
        float noise_out = noise_.next() * noise_env * velocity_;
        noise_out = filter_.process(noise_out);

        // Mix
        float mixed = osc_out * osc_gain_ + noise_out * noise_gain_;

        // Distortion (tanh waveshaper with pre-gain)
        if (distortion_pre_ > 0.01f) {
            mixed = tanhf(mixed * distortion_pre_);
        }

        // Master level
        mixed *= master_gain_;

        return mixed;
    }

private:
    float sr_ = 44100.0f;
    bool active_ = false;
    uint32_t age_ = 0;
    float velocity_ = 0.0f;
    float base_freq_ = 55.0f;

    Oscillator osc_;
    Envelope osc_amp_env_;
    Envelope osc_pitch_env_;
    Envelope noise_amp_env_;
    SVF filter_;
    NoiseGen noise_;

    float lfo_phase_ = 0.0f;
    float lfo_freq_ = 0.45f;
    float pitch_env_amt_ = 0.0f;
    float pitch_lfo_amt_ = 0.0f;

    float osc_gain_ = 0.5f;
    float noise_gain_ = 0.5f;
    float distortion_pre_ = 0.0f;
    float master_gain_ = 1.0f;
};

}  // namespace weird_drum
