//
// Created by Jonas Nilsson on 2019-01-28.
//

#pragma once

#include <h5cpp/hdf5.hpp>
#include <h5cpp/property/dataset_access.hpp>
#include <h5cpp/property/property_class.hpp>
#include <span.hpp>
#include <string>
#include <vector>

namespace hdf5 {
namespace datatype {
/// Required for h5cpp to write data provided using ArrayAdapter.
template <typename T> class TypeTrait<nonstd::span<T>> {
public:
  using Type = nonstd::span<T>;
  using TypeClass = typename TypeTrait<T>::TypeClass;
  static TypeClass create(const Type & = Type()) {
    return TypeTrait<T>::create();
  }
};
} // namespace datatype
namespace dataspace {

/// Required for h5cpp to write data provided using ArrayAdapter.
template <typename T> class TypeTrait<nonstd::span<T>> {
public:
  using DataspaceType = Simple;

  static DataspaceType create(const nonstd::span<T> &value) {
    return Simple(
        hdf5::Dimensions{static_cast<unsigned long long>(value.size())},
        hdf5::Dimensions{static_cast<unsigned long long>(value.size())});
  }

  static void *ptr(nonstd::span<T> &data) {
    return reinterpret_cast<void *>(data.data());
  }

  static const void *cptr(const nonstd::span<T> &data) {
    return reinterpret_cast<const void *>(data.data());
  }
};
} // namespace dataspace
} // namespace hdf5

template <typename T> class DatasetReader {
public:
  DatasetReader(hdf5::node::Group const &Group, std::string const &Path)
      : Data(Group.get_dataset(Path)) {
    auto CreationList = Data.creation_list();
    auto ChunkDims = CreationList.chunk();
    Size = Data.dataspace().size();
    ChunkSize = ChunkDims[0] * 10;
    Selector.block(0, ChunkSize);
    DataBuffer.resize(ChunkSize + 10);
  }

  T operator[](size_t Index) {
    if (Index >= UpperElement) {
      Selector.offset(0, UpperElement);
      auto UsedBlockSize = ChunkSize;
      if (UpperElement + ChunkSize >= Size) {
        UsedBlockSize = Size - UpperElement;
        Selector.block(0, Size - UpperElement);
      }
      nonstd::span<T> BufferView(DataBuffer.data(), UsedBlockSize);
      Data.read(BufferView, Selector);
      CurrentLowerPos = UpperElement;
      UpperElement += UsedBlockSize;
    }
    return DataBuffer[Index - CurrentLowerPos];
  }
  nonstd::span<T> getRange(size_t Start, size_t End) {
    if (End >= UpperElement) {
      Selector.offset(0, Start);
      auto UsedBlockSize = ChunkSize;
      if (Start + UsedBlockSize >= Size) {
        UsedBlockSize = Size - Start;
        Selector.block(0, UsedBlockSize);
      }
      nonstd::span<T> BufferView(DataBuffer.data(), UsedBlockSize);
      Data.read(BufferView, Selector);
      CurrentLowerPos = Start;
      UpperElement = Start + UsedBlockSize;
    }
    return nonstd::span<T>(DataBuffer.data() + (Start - CurrentLowerPos),
                           End - Start);
  }
  size_t size() const { return Size; };

private:
  std::vector<T> DataBuffer;
  hdf5::node::Dataset Data;
  hdf5::dataspace::Hyperslab Selector{{0}, {1}, {1}, {1}};
  size_t Size{0};
  size_t CurrentLowerPos{0};
  size_t UpperElement{0};
  size_t ChunkSize{0};
};

class WaveformData {
public:
  WaveformData(hdf5::node::Group const &Group);
  std::uint64_t getTimestamp();
  nonstd::span<uint16_t> getWaveform();
  void nextWaveform();
  bool outOfData() const;

private:
  DatasetReader<std::uint32_t> CueIndex;
  DatasetReader<std::uint64_t> CueTimestampZero;
  DatasetReader<std::uint16_t> Waveform;
  hssize_t EventCounter{0};
  size_t StartWaveformPos{0};
  size_t EndWaveformPos{0};
  hdf5::dataspace::Hyperslab DataSelection{{0}, {1}, {1}, {1}};
};
