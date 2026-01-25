/**
 * USB Audio Output Interface for Orinayo
 * 
 * Provides stereo audio output at 48kHz, 16-bit samples via USB Audio Class
 */

#include "audio_output.h"
#include "tusb.h"
#include <string.h>
#include <math.h>

// Audio buffer configuration
#define AUDIO_BUFFER_SIZE 512  // Number of sample frames in buffer
#define AUDIO_SAMPLES_PER_PACKET (CFG_TUD_AUDIO_FUNC_1_EP_SZ_IN / (AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE))

// Internal audio buffer (stereo interleaved)
static int16_t audio_buffer[AUDIO_BUFFER_SIZE * AUDIO_CHANNELS];
static volatile uint32_t audio_buffer_write_pos = 0;
static volatile uint32_t audio_buffer_read_pos = 0;

// Test tone generation (440Hz sine wave)
static bool generate_test_tone = true;
static uint32_t test_tone_phase = 0;

void audio_output_init(void)
{
    // Clear audio buffer
    memset(audio_buffer, 0, sizeof(audio_buffer));
    audio_buffer_write_pos = 0;
    audio_buffer_read_pos = 0;
}

bool audio_output_ready(void)
{
    return tud_audio_mounted();
}

void audio_output_task(void)
{
    // Nothing to do here - audio callbacks handle everything
}

uint32_t audio_output_write(const int16_t* samples, uint32_t num_samples)
{
    uint32_t written = 0;
    
    for (uint32_t i = 0; i < num_samples; i++)
    {
        uint32_t next_write_pos = (audio_buffer_write_pos + 1) % AUDIO_BUFFER_SIZE;
        
        // Check if buffer is full
        if (next_write_pos == audio_buffer_read_pos)
        {
            break;  // Buffer full
        }
        
        // Write stereo samples
        audio_buffer[audio_buffer_write_pos * AUDIO_CHANNELS] = samples[i * AUDIO_CHANNELS];
        audio_buffer[audio_buffer_write_pos * AUDIO_CHANNELS + 1] = samples[i * AUDIO_CHANNELS + 1];
        
        audio_buffer_write_pos = next_write_pos;
        written++;
    }
    
    return written;
}

//--------------------------------------------------------------------+
// TinyUSB Audio Callbacks
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an EP
bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
    (void) rhport;
    (void) pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Mute control
    if (p_request->wValue == AUDIO_FU_CTRL_MUTE)
    {
        return true;
    }

    // Volume control
    if (p_request->wValue == AUDIO_FU_CTRL_VOLUME)
    {
        return true;
    }

    return false;
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
    (void) rhport;
    (void) pBuff;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    return false;
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
    (void) rhport;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    // Mute control
    if (p_request->wValue == AUDIO_FU_CTRL_MUTE)
    {
        audio_control_cur_1_t mute = { .bCur = 0 };
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &mute, sizeof(mute));
    }

    // Volume control
    if (p_request->wValue == AUDIO_FU_CTRL_VOLUME)
    {
        audio_control_cur_2_t volume = { .bCur = 0xDB00 };  // 0 dB
        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &volume, sizeof(volume));
    }

    return false;
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
    (void) rhport;

    // We do not support any set range requests here, only current value requests
    TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

    return false;
}

// Callback API for providing audio samples to host
bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
    (void) rhport;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    // Generate audio samples - for now, generate a simple test tone
    int16_t samples[AUDIO_SAMPLES_PER_PACKET * AUDIO_CHANNELS];
    uint32_t num_frames = AUDIO_SAMPLES_PER_PACKET;
    
    if (generate_test_tone)
    {
        // Generate 440Hz sine wave test tone
        for (uint32_t i = 0; i < num_frames; i++)
        {
            float t = (float)test_tone_phase / AUDIO_SAMPLE_RATE;
            int16_t sample = (int16_t)(sin(2.0f * 3.14159f * 440.0f * t) * 8192.0f);
            
            samples[i * AUDIO_CHANNELS] = sample;      // Left
            samples[i * AUDIO_CHANNELS + 1] = sample;  // Right
            
            test_tone_phase++;
            if (test_tone_phase >= AUDIO_SAMPLE_RATE)
            {
                test_tone_phase = 0;
            }
        }
    }
    else
    {
        // Read from buffer if available
        for (uint32_t i = 0; i < num_frames; i++)
        {
            if (audio_buffer_read_pos != audio_buffer_write_pos)
            {
                samples[i * AUDIO_CHANNELS] = audio_buffer[audio_buffer_read_pos * AUDIO_CHANNELS];
                samples[i * AUDIO_CHANNELS + 1] = audio_buffer[audio_buffer_read_pos * AUDIO_CHANNELS + 1];
                audio_buffer_read_pos = (audio_buffer_read_pos + 1) % AUDIO_BUFFER_SIZE;
            }
            else
            {
                // Buffer underrun - output silence
                samples[i * AUDIO_CHANNELS] = 0;
                samples[i * AUDIO_CHANNELS + 1] = 0;
            }
        }
    }

    // Write samples to USB audio
    tud_audio_write((uint8_t*)samples, sizeof(samples));
    
    return true;
}

// Callback when audio is mounted
void tud_audio_int_handler_mounted_cb(uint8_t rhport)
{
    (void) rhport;
}
