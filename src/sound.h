#ifndef SOUND_H
#define SOUND_H

#define SOUND_PRIORITY_MUSIC 0
#define SOUND_PRIORITY_HIT 1
#define SOUND_PRIORITY_ACTION 2

#define SOUND_MUSIC_NONE 0
#define SOUND_MUSIC_TITLE 1

extern void soundInit();
extern void soundPlaySFX(unsigned char effect, unsigned char priority) ;
extern void soundStopChannel(unsigned char priority);

extern void soundLoadMusic(unsigned char music);
extern void soundPlayMusic(unsigned char music);
extern void soundAnticipateMusic(unsigned char music); //mutes current music if new music is different

#endif
