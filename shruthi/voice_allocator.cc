// Copyright 2010 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Polyphonic voice allocator. This is used when the Shruthi-1 is configured
// for poly-chaining.

#include "voice_allocator.h"

#include <string.h>

namespace shruthi {

/* static */
VoiceEntry VoiceAllocator::pool_[kMaxPolyphony];

/* static */
uint8_t VoiceAllocator::size_;

/* static */
uint8_t VoiceAllocator::lru_[kMaxPolyphony];

/* static */
void VoiceAllocator::Clear() {
  memset(&pool_, 0, sizeof(pool_));
  for (uint8_t i = 0; i < kMaxPolyphony; ++i) {
    lru_[i] = kMaxPolyphony - i - 1;
  }
}

/* static */
uint8_t VoiceAllocator::NoteOn(uint8_t note) {
  if (size_ == 0) {
    return 0xff;
  }
  
  uint8_t voice = 0xff;
  // First, check if there is a voice currently playing this note. In this case
  // This voice will be responsible for retriggering this note.
  // Hint: if you're more into string instruments than keyboard instruments,
  // you can safely comment those lines.
  for (uint8_t i = 0; i < size_; ++i) {
    if (pool_[i].note == note) {
      voice = i;
      break;
    }
  }
  
  // Then, try to find the least recently touched, currently inactive voice.
  if (voice == 0xff) {
    for (uint8_t i = 0; i < kMaxPolyphony; ++i) {
      if (lru_[i] < size_ && !pool_[lru_[i]].active) {
        voice = lru_[i];
      }
    }
  }
  // If all voices are active, use the least recently played note.
  if (voice == 0xff) {
    for (uint8_t i = 0; i < kMaxPolyphony; ++i) {
      if (lru_[i] < size_) {
        voice = lru_[i];
      }
    }
  }
  pool_[voice].note = note;
  pool_[voice].active = 1;
  Touch(voice);
  return voice;
}

/* static */
uint8_t VoiceAllocator::NoteOff(uint8_t note) {
  uint8_t voice = 0xff;
  for (uint8_t i = 0; i < size_; ++i) {
    if (pool_[i].note == note) {
      voice = i;
    }
  }
  if (voice != 0xff) {
    pool_[voice].active = 0;
    Touch(voice);
  }
  return voice;
}

/* static */
void VoiceAllocator::Touch(uint8_t voice) {
  int8_t source = kMaxPolyphony - 1;
  int8_t destination = kMaxPolyphony - 1;
  while (source >= 0) {
    if (lru_[source] != voice) {
      lru_[destination--] = lru_[source];
    }
    --source;
  }
  lru_[0] = voice;
}

}  // namespace shruthi
