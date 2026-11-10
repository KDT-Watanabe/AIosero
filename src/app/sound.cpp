#include "sound.h"
#include <DxLib.h>

namespace app {

static int flip_h = -1, cd_h = -1;

void sound_init() {
    flip_h = LoadSoundMem(L"assets/flip.wav");
    cd_h = LoadSoundMem(L"assets/cd_unlock.wav");
}

void play_flip() { if (flip_h >= 0) PlaySoundMem(flip_h, DX_PLAYTYPE_BACK); }
void play_cd_unlock() { if (cd_h >= 0) PlaySoundMem(cd_h, DX_PLAYTYPE_BACK); }

} // namespace app
