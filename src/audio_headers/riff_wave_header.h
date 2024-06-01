#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    // RIFF Header
    char riff_header[4];      // Contains "RIFF"
    uint32_t file_size;       // Size of the file in bytes minus 8 bytes for the two fields not included in this count: the RIFF header and file_size
    char wave_header[4];      // Contains "WAVE"

    // fmt subchunk
    char fmt_header[4];       // Contains "fmt "
    uint32_t fmt_chunk_size;  // Size of the fmt chunk (16 for PCM)
    uint16_t audio_format;    // Audio format, 1 for PCM
    uint16_t num_channels;    // Number of channels, 1 for mono, 2 for stereo
    uint32_t sample_rate;     // Sampling frequency (e.g., 44100)
    uint32_t byte_rate;       // sample_rate * num_channels * bytes_per_sample
    uint16_t block_align;     // num_channels * bytes_per_sample
    uint16_t bits_per_sample; // Bits per sample (e.g., 16)

    // data subchunk
    char data_header[4];      // Contains "data"
    uint32_t data_size;       // Number of bytes in data. Number of samples * num_channels * bytes_per_sample
} RiffWaveHeader;

typedef struct {
    RiffWaveHeader header;
    int16_t* data;
} WavFile;



int load_wav_file(WavFile* wav, const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        //std::cerr << "Cannot open WAV file: " << file_path << std::endl;
        fprintf(stderr, "Cannot open WAV file: %s\n", file_path);
        return 0;
    }

    // Read the header
    if (fread(&wav->header, sizeof(RiffWaveHeader), 1, file) != 1) {
        //std::cerr << "Error reading WAV header" << std::endl;
        fprintf(stderr, "Error reading WAV header\n");
        fclose(file);
        return 0;
    }

    // Validate headers
    if (strncmp(wav->header.riff_header, "RIFF", 4) != 0 ||
        strncmp(wav->header.wave_header, "WAVE", 4) != 0 ||
        strncmp(wav->header.fmt_header, "fmt ", 4) != 0 ||
        strncmp(wav->header.data_header, "data", 4) != 0) {
        //std::cerr << "Invalid WAV file format" << std::endl;
        fprintf(stderr, "Invalid WAV file format\n");
        fclose(file);
        return 0;
    }

    wav->data = (int16_t*)malloc(wav->header.data_size);
    if(!wav->data) {
        fprintf(stderr, "Error allocating memory for WAV data\n");
        fclose(file);
        return 0;
    }

    // Read the data
    if (fread(wav->data, wav->header.data_size, 1, file) != 1) {
        //std::cerr << "Error reading WAV data" << std::endl;
        fprintf(stderr, "Error reading WAV data\n");
        free(wav->data);
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

void free_wav_file(WavFile* wav) {
    free(wav->data);
}