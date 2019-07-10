//
// Created by Jonas Nilsson on 2019-01-28.
//

#include "WaveformData.h"
#include <iostream>

WaveformData::WaveformData(hdf5::node::Group const &Group)
    : CueIndex(Group, "cue_index"),
      CueTimestampZero(Group, "cue_timestamp_zero"),
      Waveform(Group, "raw_value") {
  EndWaveformPos = CueIndex[1];
}

std::uint64_t WaveformData::getTimestamp() {
  return CueTimestampZero[EventCounter];
}

nonstd::span<std::uint16_t> WaveformData::getWaveform() {
  return Waveform.getRange(StartWaveformPos, EndWaveformPos);
}

void WaveformData::nextWaveform() {
  if (outOfData()) {
    return;
  }
  ++EventCounter;
  StartWaveformPos = EndWaveformPos;
  EndWaveformPos = CueIndex[EventCounter + 1];
}

bool WaveformData::outOfData() const {
  return CueIndex.size() <= static_cast<size_t>(EventCounter + 1);
}
