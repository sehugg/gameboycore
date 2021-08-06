#include <gameboycore/gameboycore.h>

#include <iostream>
#include <string>
#include <functional>

using namespace gb;

extern "C" {

#define MAX_SAMPLES 4096
float sample_buffer[MAX_SAMPLES];
unsigned int sample_count = 0;

void audio_callback_fn(const float* samples, int num_samples, void* user_data) {
    if (sample_count + num_samples < MAX_SAMPLES) {
        memcpy(sample_buffer+sample_count, samples, num_samples*sizeof(float));
        sample_count += num_samples;
    }
}


#define STATE_SIZE 70000
#define CPU_STATE_SIZE 12

typedef struct SYS_T {
    GameboyCore core;
    int curline;
    uint8_t joypad;
    uint32_t pixels[160 * 144];
} SYS_T;

typedef struct STATE_T {
    uint8_t state[STATE_SIZE];
} STATE_T;

typedef struct CPU_STATE_T {
    uint8_t state[CPU_STATE_SIZE];
} CPU_STATE_T;

typedef struct CONTROLS_STATE_T {
    uint8_t joypad;
} CONTROLS_STATE_T;

SYS_T* machine_init(char* bios);
void machine_hardreset(SYS_T* sys);
void machine_reset(SYS_T* sys);
bool machine_load_rom(SYS_T* sys, const uint8_t* ptr, int num_bytes);
void machine_start_frame(SYS_T* sys);
void machine_advance_frame(SYS_T* sys);
int machine_tick(SYS_T* sys);
int machine_exec(SYS_T* sys, uint32_t steps);
void* machine_get_pixel_buffer(const SYS_T* sys);
uint8_t machine_mem_read(SYS_T* sys, uint16_t address);
void machine_mem_write(SYS_T* sys, uint16_t address, uint8_t value);
int machine_get_state_size();
int machine_get_cpu_state_size();
int machine_get_controls_state_size();
void machine_save_state(const SYS_T* sys, STATE_T* state);
void machine_load_state(SYS_T* sys, const STATE_T* state);
void machine_save_cpu_state(SYS_T* sys, CPU_STATE_T* state);
void machine_load_cpu_state(SYS_T* sys, const CPU_STATE_T* state);
void machine_save_controls_state(const SYS_T* sys, CONTROLS_STATE_T* state);
void machine_load_controls_state(SYS_T* sys, const CONTROLS_STATE_T* state);
void machine_key_down(SYS_T* sys, int key_code);
void machine_key_up(SYS_T* sys, int key_code);
unsigned int machine_cpu_get_pc(SYS_T* sys);
unsigned int machine_cpu_get_sp(SYS_T* sys);
bool machine_cpu_is_stable(SYS_T* sys);
int machine_get_raster_line(SYS_T* sys);
float* machine_get_sample_buffer();
unsigned int machine_get_sample_count();

////

SYS_T* machine_init(char* bios) {
    SYS_T* sys = new SYS_T();
    sys->core.setScanlineCallback([sys](const GPU::Scanline& scanline, int line) {
        sys->curline = line;
        for (int i=0; i<160; i++) {
            uint32_t rgba = 0xff000000;
            const Pixel& pix = scanline[i];
            rgba |= pix.r << 0;
            rgba |= pix.g << 8;
            rgba |= pix.b << 16;
            sys->pixels[i + line*160] = rgba;
        }
    });
    return sys;
}

void machine_hardreset(SYS_T* sys) {
    sys->core.reset();
}

void machine_reset(SYS_T* sys) {
    sys->core.reset();
}

bool machine_load_rom(SYS_T* sys, const uint8_t* ptr, int num_bytes) {
    sys->core.loadROM(ptr, num_bytes);
    return true; // TODO
}

void machine_start_frame(SYS_T* sys) {
}

int machine_tick(SYS_T* sys) {
    sys->core.update(1);
    return 1; // TODO
}

int machine_exec(SYS_T* sys, uint32_t steps) {
    sys->core.update(steps);
    return steps; // TODO
}

void machine_advance_frame(SYS_T* sys) {
    sys->core.emulateFrame();
}

void* machine_get_pixel_buffer(const SYS_T* sys) {
    return (void*) sys->pixels;
}

uint8_t machine_mem_read(SYS_T* sys, uint16_t address) {
    return sys->core.readMemory(address);
}

void machine_mem_write(SYS_T* sys, uint16_t address, uint8_t value) {
    sys->core.writeMemory(address, value);
}

int machine_get_state_size() {
    return STATE_SIZE;
}
int machine_get_cpu_state_size() {
    return CPU_STATE_SIZE;
}
int machine_get_controls_state_size() {
    return sizeof(CONTROLS_STATE_T);
}

void machine_save_state(const SYS_T* sys, STATE_T* state) {
    auto v = sys->core.serialize();
    std::copy(v.begin(), v.end(), state->state);
}
void machine_load_state(SYS_T* sys, const STATE_T* state) {
    auto v = std::vector<uint8_t>(std::begin(state->state), std::end(state->state));
    sys->core.deserialize(v);
}
void machine_save_cpu_state(SYS_T* sys, CPU_STATE_T* state) {
    auto v = sys->core.getCPU()->serialize();
    std::copy(v.begin(), v.end(), state->state);
}
void machine_save_controls_state(const SYS_T* sys, CONTROLS_STATE_T* state) {
    state->joypad = sys->joypad;
}
void machine_load_controls_state(SYS_T* sys, const CONTROLS_STATE_T* state) {
    sys->joypad = state->joypad;
    for (int i=0; i<8; i++) {
        if (sys->joypad & (1<<i))
            sys->core.getJoypad()->press((gb::Joy::Key)i);
        else
            sys->core.getJoypad()->release((gb::Joy::Key)i);
    }
}

void machine_key_down(SYS_T* sys, int key_code){
    //TODO
}
void machine_key_up(SYS_T* sys, int key_code) {
    //TODO
}
unsigned int machine_cpu_get_pc(SYS_T* sys) {
    return sys->core.getCPU()->getStatus().sp;
}
unsigned int machine_cpu_get_sp(SYS_T* sys) {
    return sys->core.getCPU()->getStatus().pc;
}
bool machine_cpu_is_stable(SYS_T* sys) {
    return true;
}
int machine_get_raster_line(SYS_T* sys) {
    return sys->curline;
}

float* machine_get_sample_buffer() {
    return sample_buffer;
}

unsigned int machine_get_sample_count() {
    unsigned int n = sample_count;
    sample_count = 0;
    return n;
}

} // extern "C"


/////////////////////

//#define RUNTEST

#ifdef RUNTEST

#include <stdio.h>
#include "testroms.c"

int main(int argc, char * argv[])
{
    SYS_T* sys = machine_init(NULL);
    STATE_T state;
    machine_load_rom(sys, TESTROM, 65536);
    machine_reset(sys);
    machine_exec(sys, 100000);
    machine_save_state(sys, &state);
    printf("%lu\n", sys->core.serialize().size());
    FILE* f = fopen("test.rgba", "wb");
    fwrite(sys->pixels, 1, sizeof(sys->pixels), f);
    fclose(f);
    return 0;
}

#else

int main(int argc, char * argv[])
{
    return 0;
}

#endif
