/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init, // add register may cause undefined error(free), remove it
  reg_count,
  nr_reg
};

static uint32_t r_ptr = 0;
static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static void audio_pla(void *userdata, uint8_t *stream, int len){
	int nread = len;
	if(audio_base[reg_count] > 0){
		if(audio_base[reg_count] < len) nread = audio_base[reg_count];
		int i = 0;
		for(; i < nread; ++i) stream[i] = sbuf[(i + r_ptr) % CONFIG_SB_SIZE];
		if(nread < len) memset(stream + nread, 0, len - nread);
		i = 0;
		audio_base[reg_count] -= nread;
		r_ptr = (r_ptr + nread) % CONFIG_SB_SIZE;
	}else{
		memset(stream, 0, len);
	} 
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
	if(is_write && offset == 16 && len == 1){
		SDL_AudioSpec s = {};
		s.format = AUDIO_S16SYS;
		s.userdata = NULL;
		s.freq = audio_base[reg_freq];
		s.channels = audio_base[reg_channels];
		s.samples = audio_base[reg_samples];
		s.callback = audio_pla;
		audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
		audio_base[reg_count] = 0;
//		audio_base[reg_r] = 0;
//		audio_base[reg_w] = 0;
		audio_base[reg_init] = 1;
		SDL_InitSubSystem(SDL_INIT_AUDIO);
		SDL_OpenAudio(&s, NULL);
		SDL_PauseAudio(0);
	}  
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif
  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
