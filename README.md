# Brief
Customed for a specific project.

# Usage
```cpp
#include "spech.h"

int main()
{
    wave2spec("path_to_wav_file", "path_to_save_file");
}
```

# Compile
**Opencv** required
$ g++ -o outputFile YourFile.cpp spech.cpp -lstdc++ -lm -lopencv_imgcodecs $(pkg-config --cflags --libs opencv)

# Aknowledgement
Sigpack
Opencv
