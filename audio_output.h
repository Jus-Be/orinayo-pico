/**
 * USB Audio Output Interface for Orinayo
 * 
 * Provides stereo audio output at 48kHz, 16-bit samples via USB Audio Class
 */

#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TUD_AUDIO_DESC_LEN              0x64
#define TUD_AUDIO_AS_INT_DESC_CNT       2
#define CFG_TUD_AUDIO_CTRL_BUF_SIZE     64
#define CFG_TUD_AUDIO_EP_SZ_IN          192

#define CFG_TUD_AUDIO_FUNC_DESC_LEN      0x64
#define CFG_TUD_AUDIO_FUNC_N_AS_INT      2
#define CFG_TUD_AUDIO_FUNC_EP_IN_SZ      192

// Audio configuration
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_CHANNELS 2
#define AUDIO_BYTES_PER_SAMPLE 2

/**
 * Initialize the USB audio output system
 */
void audio_output_init(void);

/**
 * Task function to be called regularly from main loop
 * Handles audio buffer management and USB audio transmission
 */
void audio_output_task(void);

/**
 * Write audio samples to the output buffer
 * @param samples Pointer to interleaved stereo samples (left, right, left, right, ...)
 * @param num_samples Number of sample frames (each frame has 2 samples for stereo)
 * @return Number of sample frames actually written
 */
uint32_t audio_output_write(const int16_t* samples, uint32_t num_samples);

/**
 * Check if audio output is ready and connected
 * @return true if USB audio is enumerated and ready
 */
bool audio_output_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_OUTPUT_H */
