#ifndef AUDIO_H
#define AUDIO_H

#ifdef AUDIO
#include <mmsystem.h>
#endif

#include "global.h"

#if defined(AUDIO) || defined (AUDIO_SAVE)

typedef struct wavdata {

	unsigned int	riffid;
	int				riffsize;
	unsigned int	waveid;
	unsigned int	chunkid;
	int				chunksize;
	unsigned short	format;
	unsigned short	channels;
	int				samplerate;
	int				bytespersec;
	unsigned short	blockalign;
	unsigned short	bitspersample;
	unsigned int	dataid;
	int				datasize;
	short			data;

} wavdata_t;

void	audio_prerender(wavdata_t** wav, int duration, int samplerate);

#endif

#ifdef AUDIO
void	audio_play(wavdata_t* wav);
#endif

#ifdef AUDIO_SAVE
void	audio_save(wavdata_t* wav);
#endif

#endif