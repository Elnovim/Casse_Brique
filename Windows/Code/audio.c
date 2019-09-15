struct Playing_Sound {
	b32 active;
	f32 pan;
	Loaded_Sound *sound;
	f32 position; 
	f32 volume;
	b32 looping;
	f32 speed_multiplier;

	struct Playing_Sound *next_free;
} typedef Playing_Sound;

Playing_Sound playing_sounds[32];
int next_playing_sound;
Playing_Sound *first_free_sound;

internal Playing_Sound*
play_sound(Loaded_Sound *sound, b32 looping) {
	Playing_Sound *result = first_free_sound;

	if (result) first_free_sound = result->next_free;
	else {
		if (next_playing_sound >= array_count(playing_sounds)) { 
			assert(0);
			return 0;
		}
		result = playing_sounds + next_playing_sound++;
	}

	result->active = true;
	result->sound = sound;
	result->pan = 0.f;
	result->volume = 1.f;
	result->position = 0;
	result->looping = looping;
	result->speed_multiplier = 1.f;

	return result;
}

internal void
stop_sound(Playing_Sound *sound) {
	sound->active = false;
	sound->next_free = first_free_sound;
	first_free_sound = sound;
}

internal void
update_audio(Game_Sound_Buffer *sound_buffer, f32 dt) {

	s16 *at = sound_buffer->samples;
	for (int i = 0; i <sound_buffer->samples_to_write; ++i) {

		f32 left_sample = 0;
		f32 right_sample = 0;

		for (Playing_Sound *sound = playing_sounds; sound != playing_sounds + array_count(playing_sounds); sound++) {
			if (!sound->active) continue;

			int sample = (int)(sound->position*(f32)sound->sound->channel_count);

			f32 left_sound_sample = (f32)sound->sound->samples[sample]*sound->volume;
			f32 right_sound_sample = (f32)sound->sound->samples[sample+sound->sound->channel_count-1]*sound->volume;

			left_sample += left_sound_sample*clampf(0, (1.f-sound->pan), 1.f) + right_sound_sample*clampf(0, -sound->pan, 1.f);
			right_sample += right_sound_sample*clampf(0, (1.f+sound->pan), 1.f) + left_sound_sample*clampf(0, sound->pan, 1.f);

			sound->position += sound->speed_multiplier;
			if (sound->position >= sound->sound->sample_count) {
				if (sound->looping) sound->position = 0.f;
				else stop_sound(sound);
			}

		}
		f32 min = (f32)MIN_S16;
		f32 max = (f32)MAX_S16;
		*at++ = (s16)clampf(min, left_sample*.5f, max);
		*at++ = (s16)clampf(min, right_sample*.5f, max);
	}
}