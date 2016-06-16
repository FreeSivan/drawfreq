#include <stdio.h>
#include <unistd.h>

#include "Mp3FreqAnaly.h"

int main(int argc, char **argv) {
    char *mp3Name = 0;
    char *imgName = 0;
    int ch = -1;
    
    while (-1 != (ch=getopt(argc, argv, "m:i:"))) {
        switch(ch) {
        case 'm': {
            mp3Name = optarg;
            break;
        }
        case 'i': {
            imgName = optarg;
            break;
        }
        default: {
            break;
        }
        }
    }
    if (!mp3Name || !imgName) {
        return -1;     
    }
    DarwFreqPicture(mp3Name, imgName);
    return 0;
}
