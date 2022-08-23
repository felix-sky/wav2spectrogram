#include "../spec.h"

int main()
{
    wave2spec("./test.wav", "./spectrogram.png");
    wave2mel("./test.wav", "./mel_spectrogram.png");
    return 0;
}