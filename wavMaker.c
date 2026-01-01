#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef int16_t i16;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float f32;

// Bad name needs fixed
typedef struct {
    f32 freq;
    f32 dur;
    f32 vol;
} Song;

void write_16(FILE* f, u16 n) { fwrite(&n, 2, 1, f); }
void write_32(FILE* f, u32 n) { fwrite(&n, 4, 1, f); }

#define PI 3.1415926535f
#define WRITE_STR_LIT(f, s) fwrite((s), 1, sizeof(s) - 1, f)

#define FREQ 44100

// Notes
typedef enum {
    NOTE_C, NOTE_CS, NOTE_D, NOTE_DS, NOTE_E, NOTE_F, NOTE_FS,
    NOTE_G, NOTE_GS, NOTE_A, NOTE_AS, NOTE_B
} NoteName;

float note_freq(NoteName note, int octave) {
    int n = (octave - 4) * 12 + (note - NOTE_A);
    return 440.0f * powf(2.0f, n / 12.0f);
}

// Note lengths in beats
#define NOTE_QUARTER   1.0f
#define NOTE_EIGHTH    0.5f
#define NOTE_HALF      2.0f
#define NOTE_WHOLE     4.0f

f32 note_duration(f32 bpm, f32 beats)
{
    return (60.0f / bpm) * beats;
}

float sign(float x) {
    return (x >= 0.0f) ? 1.0f : -1.0f;
}

void write_notes(Song* notes, u32 num_notes)
{
    FILE* f = fopen("test.wav", "wb");

    f32 duration = 0.0f;
    for (u32 i = 0; i < num_notes; i++)
    {
        duration += notes[i].dur;
    }

    u32 num_samples = (u32)(duration * FREQ);
    u32 data_size = num_samples * sizeof(i16);

    WRITE_STR_LIT(f, "RIFF");
    write_32(f, 36 + data_size);
    WRITE_STR_LIT(f, "WAVE");

    WRITE_STR_LIT(f, "fmt ");
    write_32(f, 16);
    write_16(f, 1);
    write_16(f, 1);
    write_32(f, FREQ);
    write_32(f, FREQ * sizeof(i16));
    write_16(f, sizeof(i16));
    write_16(f, 16);

    WRITE_STR_LIT(f, "data");
    write_32(f, data_size);

    u32 cur_note = 0;
    f32 cur_note_start = 0.0f;
    for (u32 i = 0; i < num_samples; i++)
    {
        f32 t = (f32)i / FREQ;
        f32 y = 0.0f;

        if (cur_note < num_notes)
        {
            // Sine wave
            //y = notes[cur_note].vol * sinf(t * notes[cur_note].freq * 2.0f * PI);
            // Saw wave
            //y = notes[cur_note].vol * (fmodf(notes[cur_note].freq * t, 1.0f) - 0.5f);
            // Square wave
            y = notes[cur_note].vol * sign(sin(t * notes[cur_note].freq * 2.0f * PI));

            if (t > cur_note_start + notes[cur_note].dur)
            {
                cur_note++;
                cur_note_start = t;
            }
        }

        i16 sample = (i16)(y * INT16_MAX);
        write_16(f, sample);
    }

    fclose(f);
}

NoteName char_to_note(char c, bool sharp)
{
    if (sharp == false)
    {
        switch (c)
        {
            case 'C': return NOTE_C;
            case 'D': return NOTE_D;
            case 'E': return NOTE_E;
            case 'F': return NOTE_F;
            case 'G': return NOTE_G;
            case 'A': return NOTE_A;
            case 'B': return NOTE_B;
            default: return NOTE_C;
        }
    }
    else
    {
        switch (c)
        {
            case 'C': return NOTE_CS;
            case 'D': return NOTE_DS;
            case 'E': return NOTE_E;
            case 'F': return NOTE_FS;
            case 'G': return NOTE_GS;
            case 'A': return NOTE_AS;
            case 'B': return NOTE_B;
            default: return NOTE_CS;
        }  
    }
}

Song* read_song(const char* song_name, int* out_num_notes, f32 bpm)
{
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "songs\\%s", song_name);

    FILE* song = fopen(file_path, "r");
    if (!song) {
        perror("Failed to open file");
        return NULL;
    }

    char buffer[32];
    int line_count = 0;
    while (fgets(buffer, sizeof(buffer), song)) {
        line_count++;
    }
    rewind(song);

    Song* notes = malloc(line_count * sizeof(Song));
    if (!notes) {
        perror("malloc failed");
        fclose(song);
        return NULL;
    }

    for (int i = 0; i < line_count; i++) {
        if (fgets(buffer, sizeof(buffer), song)) {
            bool sharp = false;
            char* token = strtok(buffer, ",");
            if (!token) continue;

            char note_char = token[0];
            char octave_char;
            if (token[1] == '#')
            {
                octave_char = token[2];
            }
            else
            {
                octave_char = token[1];
            }
            int octave = octave_char - '0';

            char* duration_str = strtok(NULL, ",");
            float duration = duration_str ? atof(duration_str) : 0.25f;

            NoteName n = char_to_note(note_char, sharp);
            notes[i].freq = note_freq(n, octave);
            notes[i].dur = note_duration(bpm, duration);
            notes[i].vol = 0.25f;
        }
    }

    fclose(song);
    *out_num_notes = line_count;
    return notes;
}

void remove_new_line(char* s)
{
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

int main(void)
{
    printf("   _____                       _ __  __       _             \n");
    printf("  / ____|                     | |  \\/  |     | |            \n");
    printf(" | (___   ___  _   _ _ __   __| |  /\\  | __ _| | _____ _ __ \n");
    printf("  \\___ \\ / _ \\| | | | '_ \\ / _` | |\\/| |/ _` | |/ / _ \\ '__|\n");
    printf("  ____) | (_) | |_| | | | | (_| | |  | | (_| |   <  __/ |   \n");
    printf(" |_____/ \\___/ \\__,_|_| |_|\\__,_|_|  |_|\\__,_|_|\\_\\___|_|   \n");
    printf("\n");
    printf("*----------------------------------------------------------*\n");
    printf("\n");
    printf("Please type the name of the file you would like to generate...\n");

    char song_title[256];
    fgets(song_title, sizeof(song_title), stdin);
    remove_new_line(song_title);

    printf("What BPM?\n");
    char bpm_str[32];
    fgets(bpm_str, sizeof(bpm_str), stdin);
    remove_new_line(bpm_str);
    f32 bpm = (f32)atof(bpm_str);

    int num_notes;
    Song* notes = read_song(song_title, &num_notes, bpm);
    if (!notes) return 1;

    write_notes(notes, num_notes);


    free(notes);
    return 0;
}