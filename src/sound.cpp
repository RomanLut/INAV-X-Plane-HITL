#include "sound.h"
#include "util.h"

#if LIN
#include <AL/al.h>
#include <AL/alc.h>
#elif APL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

TSound g_sound;

#ifdef USE_OPENAL

#define RIFF_ID 0x46464952			// 'RIFF'
#define FMT_ID  0x20746D66			// 'FMT '
#define DATA_ID 0x61746164			// 'DATA'

#define SWAP_16(value)                 \
        (((((unsigned short)value)<<8) & 0xFF00)   | \
         ((((unsigned short)value)>>8) & 0x00FF))

#define SWAP_32(value)                     \
        (((((unsigned int)value)<<24) & 0xFF000000)  | \
         ((((unsigned int)value)<< 8) & 0x00FF0000)  | \
         ((((unsigned int)value)>> 8) & 0x0000FF00)  | \
         ((((unsigned int)value)>>24) & 0x000000FF))

#define FAIL(X) { LOG(X); free(mem); return 0; }

static ALuint			snd_src = 0;				// Sample source and buffer - this is one "sound" we play.
static ALuint			snd_buffer = 0;

static ALCdevice *		my_dev = NULL;			// We make our own device and context to play sound through.
static ALCcontext *		my_ctx = NULL;

//==============================================================
//==============================================================
struct chunk_header
{ 
	int			id;
	int			size;
};

//==============================================================
//==============================================================
struct format_info
{
	short		format;				// PCM = 1, not sure what other values are legal.
	short		num_channels;
	int			sample_rate;
	int			byte_rate;
	short		block_align;
	short		bits_per_sample;
};

//==============================================================
//==============================================================
static char*	find_chunk(char * file_begin, char * file_end, int desired_id, int swapped)
{
	while(file_begin < file_end)
	{
		chunk_header * h = (chunk_header *) file_begin;
		if(h->id == desired_id && !swapped)
			return file_begin+sizeof(chunk_header);
		if(h->id == SWAP_32(desired_id) && swapped)
			return file_begin+sizeof(chunk_header);
		int chunk_size = swapped ? SWAP_32(h->size) : h->size;
		char * next = file_begin + chunk_size + sizeof(chunk_header);
		if(next > file_end || next <= file_begin)
			return NULL;
		file_begin = next;		
	}
	return NULL;
}

//==============================================================
//==============================================================
// Given a chunk, find its end by going back to the header.
static char* chunk_end(char * chunk_start, int swapped)
{
	chunk_header * h = (chunk_header *) (chunk_start - sizeof(chunk_header));
	return chunk_start + (swapped ? SWAP_32(h->size) : h->size);
}

//==============================================================
//==============================================================
ALuint load_wave(const char * file_name)
{
	// First: we open the file and copy it into a single large memory buffer for processing.

	FILE * fi = fopen(file_name,"rb");
	if(fi == NULL)
	{
		LOG("WAVE file load failed - could not open.\n");	
		return 0;
	}
	fseek(fi,0,SEEK_END);
	int file_size = ftell(fi);
	fseek(fi,0,SEEK_SET);
	char * mem = (char*) malloc(file_size);
	if(mem == NULL)
	{
		LOG("WAVE file load failed - could not allocate memory.\n");
		fclose(fi);
		return 0;
	}
	if (fread(mem, 1, file_size, fi) != file_size)
	{
		LOG("WAVE file load failed - could not read file.\n");	
		free(mem);
		fclose(fi);
		return 0;
	}
	fclose(fi);
	char * mem_end = mem + file_size;
	
	// Second: find the RIFF chunk.  Note that by searching for RIFF both normal
	// and reversed, we can automatically determine the endian swap situation for
	// this file regardless of what machine we are on.
	
	int swapped = 0;
	char * riff = find_chunk(mem, mem_end, RIFF_ID, 0);
	if(riff == NULL)
	{
		riff = find_chunk(mem, mem_end, RIFF_ID, 1);
		if(riff)
			swapped = 1;
		else
			FAIL("Could not find RIFF chunk in wave file.\n")
	}
	
	// The wave chunk isn't really a chunk at all. :-(  It's just a "WAVE" tag 
	// followed by more chunks.  This strikes me as totally inconsistent, but
	// anyway, confirm the WAVE ID and move on.
	
	if (riff[0] != 'W' ||
		riff[1] != 'A' ||
		riff[2] != 'V' ||
		riff[3] != 'E')
		FAIL("Could not find WAVE signature in wave file.\n")

	char * format = find_chunk(riff+4, chunk_end(riff,swapped), FMT_ID, swapped);
	if(format == NULL)
		FAIL("Could not find FMT  chunk in wave file.\n")
	
	// Find the format chunk, and swap the values if needed.  This gives us our real format.
	
	format_info * fmt = (format_info *) format;
	if(swapped)
	{
		fmt->format = SWAP_16(fmt->format);
		fmt->num_channels = SWAP_16(fmt->num_channels);
		fmt->sample_rate = SWAP_32(fmt->sample_rate);
		fmt->byte_rate = SWAP_32(fmt->byte_rate);
		fmt->block_align = SWAP_16(fmt->block_align);
		fmt->bits_per_sample = SWAP_16(fmt->bits_per_sample);
	}
	
	// Reject things we don't understand...expand this code to support weirder audio formats.
	if(fmt->format != 1) FAIL("Wave file is not PCM format data.\n")
	if(fmt->num_channels != 1 && fmt->num_channels != 2) FAIL("Must have mono or stereo sound.\n")
	if(fmt->bits_per_sample != 8 && fmt->bits_per_sample != 16) FAIL("Must have 8 or 16 bit sounds.\n")
	char * data = find_chunk(riff+4, chunk_end(riff,swapped), DATA_ID, swapped) ;
	if(data == NULL)
		FAIL("I could not find the DATA chunk.\n")
	
	int sample_size = fmt->num_channels * fmt->bits_per_sample / 8;
	int data_bytes = (int)(chunk_end(data,swapped) - data);
	int data_samples = data_bytes / sample_size;
	
	// If the file is swapped and we have 16-bit audio, we need to endian-swap the audio too or we'll 
	// get something that sounds just astoundingly bad!
	if(fmt->bits_per_sample == 16 && swapped)
	{
		short * ptr = (short *) data;
		int words = data_samples * fmt->num_channels;
		while(words--)
		{
			*ptr = SWAP_16(*ptr);
			++ptr;
		}
	}
	
	// Finally, the OpenAL crud.  Build a new OpenAL buffer and send the data to OpenAL, passing in
	// OpenAL format enums based on the format chunk.
	ALuint buf_id = 0;
	alGenBuffers(1, &buf_id);
	if(buf_id == 0) FAIL("Could not generate buffer id.\n");
	
	alBufferData(buf_id, fmt->bits_per_sample == 16 ? 
							(fmt->num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16) :
							(fmt->num_channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8),
					data, data_bytes, fmt->sample_rate);
	free(mem);
	return buf_id;
}

//==============================================================
//==============================================================
// This is a stupid logging error function...useful for debugging, but not good error checking.
#define CHECK_ERR() __CHECK_ERR(__FILE__,__LINE__)
static void __CHECK_ERR(const char * f, int l)
{
	ALuint e = alGetError();
	if (e != AL_NO_ERROR)
		LOG("ERROR: %d (%s:%d\n", e, f, l);
}

//=============================================================
//=============================================================
void TSound::init()
{
	CHECK_ERR();
	
	// We have to save the old context and restore it later, so that we don't interfere with X-Plane
	// and other plugins.
	ALCcontext * old_ctx = alcGetCurrentContext();
	
	if(old_ctx == NULL)
	{
		LOG("I found no OpenAL, I will be the first to init.");
		my_dev = alcOpenDevice(NULL);
		if(my_dev == NULL)
		{
			LOG("Could not open the default OpenAL device.");
			return;		
		}	
		my_ctx = alcCreateContext(my_dev, NULL);
		if(my_ctx == NULL)
		{
			if(old_ctx)
				alcMakeContextCurrent(old_ctx);
			alcCloseDevice(my_dev);
			my_dev = NULL;
			LOG("Could not create a context.");
			return;				
		}
		
		// Make our context current, so that OpenAL commands affect our, um, stuff.
		
		alcMakeContextCurrent(my_ctx);
		LOG("created the context.",my_ctx);

		ALCint major_version, minor_version;
		const char * al_hw=alcGetString(my_dev,ALC_DEVICE_SPECIFIER	);
		const char * al_ex=alcGetString(my_dev,ALC_EXTENSIONS);

		alcGetIntegerv(NULL,ALC_MAJOR_VERSION,sizeof(major_version),&major_version);
		alcGetIntegerv(NULL,ALC_MINOR_VERSION,sizeof(minor_version),&minor_version);
		
		LOG("OpenAL version   : %d.%d",major_version,minor_version);
		LOG("OpenAL hardware  : %s", (al_hw?al_hw:"(none)"));
		LOG("OpenAL extensions: %s", (al_ex?al_ex:"(none)"));
		CHECK_ERR();
	} 
	else
	{
		LOG("found someone else's context 0x%08x.", old_ctx);
	}
}

//=============================================================
//=============================================================
static void deleteSound()
{
  LOG("deleting snd %d", snd_buffer);
  if (snd_src)
  {
    alDeleteSources(1, &snd_src);
    snd_src = 0;
  }
  if (snd_buffer)
  {
    alDeleteBuffers(1, &snd_buffer);
    snd_buffer = 0;
  }
}

//=============================================================
//=============================================================
void TSound::destroy()
{
  // Cleanup: nuke our context if we have it.  This is hacky and bad - we should really destroy
	// our buffers and sources.  I have _no_ idea if OpenAL will leak memory.
	if(alcGetCurrentContext() != NULL)
	{
    deleteSound();
	}
	if(my_ctx) 
	{
		LOG("deleting my context 0x%08x\n", my_ctx);
		alcMakeContextCurrent(NULL);
		alcDestroyContext(my_ctx);
	}

  if(my_dev) alcCloseDevice(my_dev);
}

//=============================================================
//=============================================================
void TSound::play(const char* pFileName)
{
  deleteSound();

  ALfloat	zero[3] = { 0 };

  // Generate 1 source and load a buffer of audio.
  alGenSources(1, &snd_src);
  CHECK_ERR();
  snd_buffer = load_wave(pFileName);
  LOG("Loaded %d from %s\n", snd_buffer, pFileName);
  CHECK_ERR();

  // Basic initialization code to play a sound: specify the buffer the source is playing, as well as some 
  // sound parameters. This doesn't play the sound - it's just one-time initialization.
  alSourcei(snd_src, AL_BUFFER, snd_buffer);
  alSourcef(snd_src, AL_PITCH, 1.0f);
  alSourcef(snd_src, AL_GAIN, 1.0f);
  alSourcei(snd_src, AL_LOOPING, 0);
  alSourcefv(snd_src, AL_POSITION, zero);
  alSourcefv(snd_src, AL_VELOCITY, zero);
  CHECK_ERR();

	alSourcePlay(snd_src);
}

#else
void TSound::init() {};
void TSound::play(const char* pFileName) {};
void TSound::destroy() {};
#endif
