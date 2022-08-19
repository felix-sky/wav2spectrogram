# Brief
从

# Usage
```cpp
#include "spec.h"

int main()
{
    wave2spec("path_to_wav_file", "path_to_save_file");
}
```

# Compile
$ g++ -o outputfile YourFiel.cpp spech.cpp -lstdc++ -lm -lopencv_imgcodecs $(pkg-config --cflags --libs opencv)
