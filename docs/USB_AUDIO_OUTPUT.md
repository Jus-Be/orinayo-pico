# USB Audio Output

## Overview

The Orinayo device now supports USB Audio output in addition to USB MIDI. The device presents itself as a stereo audio input device (microphone) to the host computer, allowing it to send audio data alongside MIDI messages.

## Configuration

### Audio Specifications

- **Sample Rate**: 48,000 Hz
- **Bit Depth**: 16-bit signed integer
- **Channels**: 2 (Stereo)
- **Format**: PCM, interleaved (Left, Right, Left, Right, ...)

### USB Descriptors

The USB descriptors have been configured to include:
- Audio Control Interface (ITF_NUM_AUDIO_CONTROL = 0)
- Audio Streaming Interface (ITF_NUM_AUDIO_STREAMING = 1)
- MIDI Control Interface (ITF_NUM_MIDI = 2)
- MIDI Streaming Interface (ITF_NUM_MIDI_STREAMING = 3)

The audio interface uses endpoint 0x81 (IN) for transmitting audio data to the host.

## Usage

### Initialization

The audio output system is automatically initialized in `main.c`:

```c
audio_output_init();
```

This should be called once during system startup, after `tusb_init()`.

### Main Loop Integration

The audio task should be called regularly from the main loop:

```c
while (true) {
    tud_task();           // TinyUSB device task
    audio_output_task();  // USB audio task
    // ... other tasks
}
```

### Writing Audio Data

To send custom audio data to the host, use the `audio_output_write()` function:

```c
// Prepare stereo audio samples (interleaved: L, R, L, R, ...)
int16_t samples[NUM_FRAMES * 2];  // 2 channels

// Fill samples array with audio data
// samples[0] = left channel, samples[1] = right channel, etc.

// Write to USB audio output
uint32_t written = audio_output_write(samples, NUM_FRAMES);
```

### Default Behavior

By default, the audio output generates a 440 Hz sine wave test tone. This is useful for verifying that the audio output is working correctly.

To disable the test tone and use your own audio data:
1. Modify the `generate_test_tone` variable in `audio_output.c` to `false`
2. Use `audio_output_write()` to provide your audio samples

### Checking Status

To check if the USB audio interface is mounted and ready:

```c
if (audio_output_ready()) {
    // Audio interface is connected and ready
}
```

## Implementation Details

### Files Modified

1. **tusb_config.h**: Added `CFG_TUD_AUDIO` configuration and audio-specific settings
2. **usb_descriptors.c**: Added audio interface descriptors using `TUD_AUDIO_MIC_TWO_CH_DESCRIPTOR`
3. **main.c**: Added audio initialization and task calls
4. **CMakeLists.txt**: Added `audio_output.c` to the build

### Files Added

1. **audio_output.h**: Header file with API declarations
2. **audio_output.c**: Implementation of USB audio output functionality

### TinyUSB Callbacks

The implementation provides the following TinyUSB audio callbacks:

- `tud_audio_tx_done_pre_load_cb()`: Called when audio data needs to be sent to the host
- `tud_audio_set_req_ep_cb()`: Handles set requests for endpoints
- `tud_audio_get_req_ep_cb()`: Handles get requests for endpoints
- `tud_audio_set_req_itf_cb()`: Handles set requests for interfaces
- `tud_audio_get_req_itf_cb()`: Handles get requests for interfaces

## Future Enhancements

Potential improvements for the audio output system:

1. **MIDI-to-Audio Synthesis**: Convert MIDI note events to audio waveforms
2. **Audio Effects**: Add reverb, delay, or other effects to the audio output
3. **Multiple Audio Sources**: Mix audio from different sources (e.g., MIDI synthesis + audio samples)
4. **Dynamic Sample Rate**: Support for different sample rates (16kHz, 44.1kHz, 96kHz, etc.)
5. **Volume Control**: Implement USB audio volume control

## Troubleshooting

### Audio Device Not Detected

- Verify that the device is properly connected via USB
- Check that the USB cable supports data transfer (not just charging)
- On Windows, check Device Manager for "Audio inputs and outputs"
- On macOS, check Audio MIDI Setup application
- On Linux, use `arecord -l` to list audio capture devices

### No Audio Output

- Verify the test tone is enabled (or your audio data is being written)
- Check the host computer's audio recording settings
- Ensure the Orinayo device is selected as the input device in your DAW or recording software
- Verify that `audio_output_task()` is being called in the main loop

### Distorted Audio

- Check sample rate settings match (48kHz)
- Verify samples are 16-bit signed integers in the correct range (-32768 to 32767)
- Ensure samples are properly interleaved (Left, Right, Left, Right)

## Example: Using with a DAW

To record the audio output in a Digital Audio Workstation (DAW):

1. Connect the Orinayo device to your computer via USB
2. Open your DAW (e.g., Ableton Live, FL Studio, Reaper)
3. Add an audio track
4. Set the input to "Orinayo Device" (or similar name)
5. Arm the track for recording
6. You should now see/hear the 440 Hz test tone or your custom audio

## References

- [TinyUSB Audio Class Documentation](https://github.com/hathach/tinyusb/tree/master/src/class/audio)
- [USB Audio Device Class 1.0 Specification](https://www.usb.org/document-library/audio-device-document-10)
- [Raspberry Pi Pico SDK Documentation](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html)
