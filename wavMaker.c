#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef int16_t i16;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float f32;

typedef struct {
    f32 freq;
    f32 dur;
} Song;

void write_16(FILE* f, u16 n) { fwrite(&n, 2, 1, f); }
void write_32(FILE* f, u32 n) { fwrite(&n, 4, 1, f); }

#define WRITE_STR_LIT(f, s) fwrite((s), 1, sizeof(s) - 1, f)

#define FREQ 44100

// Notes
#define C 261.63
#define D 293.66
#define E 329.63
#define F 349.23
#define G 392.00
#define A 440.00
#define B 493.88

// Note lengths in beats
#define NOTE_QUARTER   1.0f
#define NOTE_EIGHTH    0.5f
#define NOTE_HALF      2.0f
#define NOTE_WHOLE     4.0f

// BPM
#define BPM 109

f32 note_duration(f32 bpm, f32 beats)
{
    return (60.0f / bpm) * beats;
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
            //y = 0.25f * sinf(t * notes[cur_note].freq * 2.0f * 3.1415926535f);
            // Saw wave
            y = 2.0f * (fmodf(notes[cur_note].freq * t, 1.0f) - 0.5f);

            if (t > cur_note_start + notes[cur_note].dur)
            {
                cur_note++;
                cur_note_start = t;
            }
        }

        i16 sample = (i16)(y * 32767.0f);
        write_16(f, sample);
    }

    fclose(f);
}

int main(void)
{
    Song notes[] = {
        {E, note_duration(BPM, NOTE_QUARTER)},
        {D, note_duration(BPM, NOTE_QUARTER)},
        {C, note_duration(BPM, NOTE_QUARTER)},
        {D, note_duration(BPM, NOTE_QUARTER)},
        {E, note_duration(BPM, NOTE_QUARTER)},
        {E, note_duration(BPM, NOTE_QUARTER)},
        {E, note_duration(BPM, NOTE_HALF)},
    };

    u32 num_notes = sizeof(notes) / sizeof(notes[0]);
    write_notes(notes, num_notes);

    return 0;
}