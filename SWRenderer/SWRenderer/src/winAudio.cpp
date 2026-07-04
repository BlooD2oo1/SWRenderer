#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <cmath>
#include <cstdio>

// Helper macro/function to safely release COM interfaces
template <class T> void SafeRelease(T** ppT) {
    if (*ppT) { (*ppT)->Release(); *ppT = nullptr; }
}

static const int SAMPLE_RATE = 48000;
static constexpr float PI2   = 6.28318530717958647692f;

// Global frequencies controlled by the main thread
float gAudioFreqLeft  = 440.0f;
float gAudioFreqRight = 442.0f;

// Thread control flag
bool bRunning = true;

// Phase tracking variables
static float gPhaseL = 0.0f;
static float gPhaseR = 0.0f;

// WASAPI specific interfaces and handles
static IMMDeviceEnumerator* gEnumerator   = nullptr;
static IMMDevice* gDevice       = nullptr;
static IAudioClient* gAudioClient  = nullptr;
static IAudioRenderClient* gRenderClient = nullptr;
static HANDLE               gAudioEvent   = nullptr;

// Fills the floating-point buffer provided by WASAPI
void FillBuffer(float* dst, DWORD numFrames)
{
    for (DWORD i = 0; i < numFrames; i++)
    {
        float left  = sinf(gPhaseL * PI2);
        float right = sinf(gPhaseR * PI2);

        // Modern Windows audio engine operates natively on 32-bit float (0.0f to 1.0f)
        // No need for short conversion or multiplying by 32767.
        dst[i * 2 + 0] = left  * 0.3f; // Left channel (30% volume)
        dst[i * 2 + 1] = right * 0.3f; // Right channel (30% volume)

        gPhaseL += gAudioFreqLeft / SAMPLE_RATE;
        gPhaseR += gAudioFreqRight / SAMPLE_RATE;

        // Wrap around the phase to maintain precision
        if (gPhaseL >= 1.0f) gPhaseL -= 1.0f;
        if (gPhaseR >= 1.0f) gPhaseR -= 1.0f;
    }
}

bool Audio_Init()
{
    HRESULT hr;

    // Initialize COM library on this thread
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) return false;

    // 1. Create the device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&gEnumerator);
    if (FAILED(hr)) return false;

    // 2. Get the default multimedia audio endpoint
    hr = gEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &gDevice);
    if (FAILED(hr)) return false;

    // 3. Activate the Audio Client interface
    hr = gDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&gAudioClient);
    if (FAILED(hr)) return false;

    // 4. Get the preferred mix format from the OS (typically 48kHz, Stereo, Float)
    WAVEFORMATEX* pwfx = nullptr;
    hr = gAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) return false;

    // Enforce our target sample rate just in case
    pwfx->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    pwfx->nSamplesPerSec = SAMPLE_RATE;

    // Request a 10 ms internal buffer duration
    REFERENCE_TIME bufferDuration = 10 * 10000; // 10ms expressed in 100-nanosecond units

    // Initialize the audio client in Shared Mode with Event-driven callback
    hr = gAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK, // Event-driven flag
        bufferDuration,
        0,
        pwfx,
        NULL
    );
    if (FAILED(hr)) return false;

    // 5. Create an Event handle and link it to WASAPI
    gAudioEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    hr = gAudioClient->SetEventHandle(gAudioEvent);
    if (FAILED(hr)) return false;

    // 6. Get the actual Render Client interface to write data
    hr = gAudioClient->GetService(__uuidof(IAudioRenderClient), (void**)&gRenderClient);
    if (FAILED(hr)) return false;

    // Pre-fill the buffer with initial audio data before starting the stream
    UINT32 bufferFrameCount;
    gAudioClient->GetBufferSize(&bufferFrameCount);

    BYTE* pData;
    hr = gRenderClient->GetBuffer(bufferFrameCount, &pData);
    if (SUCCEEDED(hr)) {
        FillBuffer((float*)pData, bufferFrameCount);
        gRenderClient->ReleaseBuffer(bufferFrameCount, 0);
    }

    // Start playing the audio stream
    hr = gAudioClient->Start();
    return SUCCEEDED(hr);
}

void Audio_Update()
{
    // Wait until Windows signals that the audio engine needs more data.
    // This provides rock-solid hardware-linked timing without consuming CPU.
    WaitForSingleObject(gAudioEvent, INFINITE);

    UINT32 bufferFrameCount;
    UINT32 numFramesPadding;

    // Get total size of the circular buffer
    if (FAILED(gAudioClient->GetBufferSize(&bufferFrameCount))) return;

    // Get the amount of valid, unplayed data currently sitting in the buffer
    if (FAILED(gAudioClient->GetCurrentPadding(&numFramesPadding))) return;

    // Calculate how much space is available to write
    UINT32 numFramesAvailable = bufferFrameCount - numFramesPadding;

    if (numFramesAvailable > 0)
    {
        BYTE* pData = nullptr;
        // Request the buffer pointer from WASAPI
        HRESULT hr = gRenderClient->GetBuffer(numFramesAvailable, &pData);
        if (SUCCEEDED(hr))
        {
            // Synthesize the new samples directly into the buffer
            FillBuffer((float*)pData, numFramesAvailable);

            // Release the buffer back to the system to be queued for playback
            gRenderClient->ReleaseBuffer(numFramesAvailable, 0);
        }
    }
}

void Audio_Shutdown()
{
    if (gAudioClient) gAudioClient->Stop();

    if (gAudioEvent) { CloseHandle(gAudioEvent); gAudioEvent = nullptr; }

    // Clean up COM resources
    SafeRelease(&gRenderClient);
    SafeRelease(&gAudioClient);
    SafeRelease(&gDevice);
    SafeRelease(&gEnumerator);

    CoUninitialize();
}
