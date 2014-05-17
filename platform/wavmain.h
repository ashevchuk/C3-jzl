#ifndef _wavmain_
#define _wavmain_

#include <stdio.h>

int PlayWavFile(int argc, char* argv[]);
int PlayMP3(int IsWiegandKeyPad, char *mp3name, int IsTestMP3);
void OpenSoundDevice(void);
void CloseSoundDevice(void);

#endif

