/*
 * SPDX-FileCopyrightText: 2023 Toboter
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sound.h"
#include "config.h"
#include "utils.h"

#include <cbm.h>

unsigned char sfxAddressHigh[] = {0xa0, 0xa2, 0xa4};

unsigned char currentMusic = SOUND_MUSIC_NONE;
unsigned char loadedMusic = SOUND_MUSIC_NONE;

char * musicNames[] = {
	"",
	"title.zsm"
};

void loadSound(char* name, unsigned char index) {
	cbm_k_setlfs(0, 8, 2);
	cbm_k_setnam(name);
	cbm_k_load(0, ((unsigned short)sfxAddressHigh[index])<<8);
}


void soundInit() {
	unsigned char i=0;
	asm volatile ("lda #%b", ZSM_BANK);
	asm volatile ("jsr zsm_init_engine");
	asm volatile ("jsr zsmkit_setisr");

	BANK_NUM = SFX_BANK_1;

	loadSound("spray.zsm", 0);
	loadSound("woo.zsm", 1);
	loadSound("crash.zsm", 2);
}

unsigned char sound_tmp, param1, param2;

void soundPlaySFX(unsigned char effect, unsigned char priority) {
	unsigned char prevBank = BANK_NUM;
	BANK_NUM = SFX_BANK_1;

	param1 = sfxAddressHigh[effect];
	param2 = priority;

	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_stop");

	asm volatile ("lda #$00");
	asm volatile ("ldx %v", param2);
	asm volatile ("ldy %v", param1); //address hi to Y
	asm volatile ("jsr zsm_setmem");

	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_play");

	BANK_NUM = prevBank;
}

void soundStopChannel(unsigned char priority) {
	param2 = priority;
	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_stop");
}

void soundAnticipateMusic(unsigned char music) {
	if (music != currentMusic) {
		param2 = SOUND_PRIORITY_MUSIC;

		asm volatile ("ldx %v", param2);
		asm volatile ("jsr zsm_stop");
	}
}

void soundLoadMusic(unsigned char music) {
	param2 = SOUND_PRIORITY_MUSIC;

	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_stop");

	BANK_NUM = MUSIC_BANK_START;

	cbm_k_setlfs(0, 8, 2);
	cbm_k_setnam(musicNames[music]);
	cbm_k_load(0, 0xa000);
	loadedMusic = music;
}

void soundPlayMusic(unsigned char music) {
	if (music == currentMusic) return;

	currentMusic = music;

	param1 = 0xa0;
	param2 = SOUND_PRIORITY_MUSIC;

	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_stop");

	if (!music) return;

	if (loadedMusic != music) soundLoadMusic(music);

	BANK_NUM = MUSIC_BANK_START;

	asm volatile ("lda #$00");
	asm volatile ("ldx %v", param2);
	asm volatile ("ldy %v", param1); //address hi to Y
	asm volatile ("jsr zsm_setmem");


	asm volatile ("ldx %v", param2);
	asm volatile ("jsr zsm_play");

	asm volatile ("ldx %v", param2); //music loops
	asm volatile ("sec");
	asm volatile ("jsr zsm_setloop");
}
