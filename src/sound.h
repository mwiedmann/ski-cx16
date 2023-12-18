#ifndef SOUND_H
#define SOUND_H

#define SOUND_PRIORITY_MUSIC 0
#define SOUND_PRIORITY_HIT 1
#define SOUND_PRIORITY_ACTION 2

#define SOUND_MUSIC_NONE 0
#define SOUND_MUSIC_TITLE 1
#define SOUND_MUSIC_FINISH 2
#define SOUND_MUSIC_CHILL 3

#define SOUND_SFX_SPRAY 0
#define SOUND_SFX_WOO 1
#define SOUND_SFX_CRASH 2
#define SOUND_SFX_YES 3
#define SOUND_SFX_NO 4

void soundInit();
void soundPlaySFX(unsigned char effect, unsigned char priority) ;
void soundStopChannel(unsigned char priority);

void soundLoadMusic(unsigned char music);
void soundPlayMusic(unsigned char music);
void soundAnticipateMusic(unsigned char music); //mutes current music if new music is different

#endif
