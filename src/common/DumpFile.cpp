#include <common/DumpFile.h>

/*
template<typename T>
class DumpFile2Impl {
public:
  ~DumpFile2Impl();

  static std::unique_ptr<DumpFile2Impl>
  create(const boost::filesystem::path &FilePath, size_t MaxMB = 0);

  static std::unique_ptr<DumpFile2Impl>
  open(const boost::filesystem::path &FilePath);

  size_t count() const;

  void push(const T& Hit);
  template<typename Container> void push(const Container& Hits);

  void readAt(size_t Index, size_t Count);
  static void read(const boost::filesystem::path &FilePath,
      std::vector<T> &ExternalData);

  /// \todo 9000 is MTU? Correct size is <= 8972 else packet
  /// fragmentation will occur.
  static constexpr size_t ChunkSize{9000 / sizeof(T)};

  boost::filesystem::path get_full_path() const;

  void flush();
  void rotate();

  std::vector<T> Data;

private:
  DumpFile2Impl(const boost::filesystem::path &file_path, size_t max_Mb);

  hdf5::file::File File;
  hdf5::datatype::Datatype DataType;
  hdf5::node::Dataset DataSet;
  hdf5::dataspace::Hyperslab Slab{{0}, {ChunkSize}};

  boost::filesystem::path PathBase{};
  size_t MaxSize{0};
  size_t SequenceNumber{0};

  void openRW();
  void openR();

  void write();
};
*/