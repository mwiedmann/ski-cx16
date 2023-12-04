#ifndef SOUND_H
#define SOUND_H

#define SOUND_PRIORITY_MUSIC 0
#define SOUND_PRIORITY_HIT 1
#define SOUND_PRIORITY_ACTION 2

#define SOUND_MUSIC_NONE 0

extern void sound_init();
extern void sound_playSFX(unsigned char effect, unsigned char priority) ;
extern void sound_stopChannel(unsigned char priority);

extern void sound_loadMusic(unsigned char music);
extern void sound_playMusic(unsigned char music);
extern void sound_anticipateMusic(unsigned char music); //mutes current music if new music is different

#endif
