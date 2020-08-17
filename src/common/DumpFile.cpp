#include <common/DumpFile.h>

static hid_t PrimToH5tNative(H5PrimSubset Prim) {
  static hid_t Map[static_cast<int>(H5PrimSubset::Count)] = {};
  static bool Init = true;
  if (Init) {
    Init = false;

    Map[static_cast<int>(H5PrimSubset::Bool)] = H5T_NATIVE_HBOOL;

    Map[static_cast<int>(H5PrimSubset::Float)] = H5T_NATIVE_FLOAT;

    Map[static_cast<int>(H5PrimSubset::UInt64)] = H5T_NATIVE_ULLONG;
    static_assert(sizeof(uint64_t) == sizeof(unsigned long long),
                  "H5 types: Potential 32/64 bit issue.");

    Map[static_cast<int>(H5PrimSubset::UInt32)] = H5T_NATIVE_UINT;
    static_assert(sizeof(uint32_t) == sizeof(unsigned int),
                  "H5 types: Potential 32/64 bit issue.");

    Map[static_cast<int>(H5PrimSubset::UInt16)] = H5T_NATIVE_USHORT;
    static_assert(sizeof(uint16_t) == sizeof(unsigned short),
                  "Potential 32/64 bit definition issue.");

    Map[static_cast<int>(H5PrimSubset::UInt8)] = H5T_NATIVE_UCHAR;
    static_assert(sizeof(uint8_t) == sizeof(unsigned char),
                  "H5 types: Potential 32/64 bit issue.");

    Map[static_cast<int>(H5PrimSubset::SInt8)] = H5T_NATIVE_SCHAR;
    static_assert(sizeof(int8_t) == sizeof(signed char),
                  "H5 types: Potential 32/64 bit issue.");

    // verify all slots have been written
    for (int i = 0; i < static_cast<int>(H5PrimSubset::Count); i++) {
      if (Map[i] == 0) {
        throw std::runtime_error(
            fmt::format("Missing H5PrimSubset to H5T_NATIVE_* mapping at "
                        "H5PrimSubset enum item {}",
                        i));
      }
    }
  }
  return Map[static_cast<int>(Prim)];
}

PrimDumpFileBase::PrimDumpFileBase(const H5PrimCompoundDef &CompoundDef,
                                   const boost::filesystem::path &file_path,
                                   size_t max_Mb)
    : CompoundDef(CompoundDef),
      ChunkSize(
          9000 /
          CompoundDef.StructSize) // \todo 9000 is MTU? Correct size is <= 8972
                                  // else packet fragmentation will occur.
      ,
      Slab({0}, {ChunkSize}) {

  Compound = hdf5::datatype::Compound::create(CompoundDef.StructSize);

  for (size_t i = 0; i < CompoundDef.MembersCount; i++) {
    const H5PrimDef &Def = CompoundDef.Members[i];
    hid_t H5fNativeType = PrimToH5tNative(Def.PrimType);
    Compound.insert(
        Def.Name, Def.Offset,
        hdf5::datatype::Datatype(hdf5::ObjectHandle(H5Tcopy(H5fNativeType))));
  }

  MaxSize = max_Mb * 1000000 / CompoundDef.StructSize;
  PathBase = file_path;
}

boost::filesystem::path PrimDumpFileBase::get_full_path() const {
  auto Ret = PathBase;
  Ret += ("_" + fmt::format("{:0>5}", SequenceNumber) + ".h5");
  return Ret;
}

void PrimDumpFileBase::openRW() {
  using namespace hdf5;

  File = file::create(get_full_path(), file::AccessFlags::TRUNCATE);

  property::DatasetCreationList dcpl;
  dcpl.layout(property::DatasetLayout::CHUNKED);
  dcpl.chunk({ChunkSize});

  DataSet = File.root().create_dataset(
      CompoundDef.DatasetName, Compound,
      dataspace::Simple({0}, {dataspace::Simple::UNLIMITED}), dcpl);
  DataSet.attributes.create_from("format_version", CompoundDef.FormatVersion);
  DataSet.attributes.create_from("EFU_version", efu_version());
  DataSet.attributes.create_from("EFU_build_string", efu_buildstr());
}

void PrimDumpFileBase::openR() {
  using namespace hdf5;

  auto Path = PathBase;
  Path += ".h5";

  File = file::open(Path, file::AccessFlags::READONLY);
  DataSet = File.root().get_dataset(CompoundDef.DatasetName);
  // if not initial version, expect it to be well formed
  if (CompoundDef.FormatVersion > 0) {
    uint16_t ver{0};
    DataSet.attributes["format_version"].read(ver);
    if (ver != CompoundDef.FormatVersion) {
      throw std::runtime_error("DumpFile version mismatch");
    }
  }
}

size_t PrimDumpFileBase::count() const {
  return hdf5::dataspace::Simple(DataSet.dataspace())
      .current_dimensions()
      .at(0);
}

void PrimDumpFileBase::rotate() {
  SequenceNumber++;
  openRW();
}

static void
h5prim_write_contiguous_data(const void *DataBuffer,
                             const hdf5::node::Dataset &dataset,
                             const hdf5::datatype::Datatype &mem_type,
                             const hdf5::dataspace::Dataspace &mem_space,
                             const hdf5::dataspace::Dataspace &file_space) {
  hdf5::property::DatasetTransferList dtpl;
  if (H5Dwrite(static_cast<hid_t>(dataset), static_cast<hid_t>(mem_type),
               static_cast<hid_t>(mem_space), static_cast<hid_t>(file_space),
               static_cast<hid_t>(dtpl), DataBuffer) < 0) {
    std::stringstream ss;
    ss << "Failure to write contiguous data to dataset ["
       << dataset.link().path() << "]!";
    hdf5::error::Singleton::instance().throw_with_stack(ss.str());
  }
}

static void
h5prim_dataset_write(const void *DataBuffer, const size_t DataElmCount,
                     const hdf5::datatype::Compound &memory_type,
                     const hdf5::node::Dataset &dataset,
                     const hdf5::dataspace::Hyperslab &hyperSelection) {

  auto dims = hyperSelection.block();
  auto count = hyperSelection.count();
  for (hdf5::Dimensions::size_type i = 0; i != dims.size(); i++) {
    dims[i] *= count[i];
  }
  hdf5::dataspace::Simple selected_space(dims);

  hdf5::dataspace::Simple memory_space(hdf5::Dimensions{DataElmCount},
                                       hdf5::Dimensions{DataElmCount});
  const hdf5::dataspace::Simple &mem_space =
      hdf5::dataspace::Simple(memory_space);

  hid_t DataspaceId = H5Dget_space(static_cast<hid_t>(dataset));
  hdf5::dataspace::Dataspace file_space{hdf5::ObjectHandle(DataspaceId)};
  file_space.selection(hdf5::dataspace::SelectionOperation::SET,
                       hyperSelection);

  RelAssertMsg(
      selected_space.rank() == 1 && mem_space.rank() == 1 &&
          selected_space.size() == memory_space.size(),
      "test assumption: we always select the entire memspace and rank is "
      "always 1 for the vector");

  if (selected_space.rank() > 1 && mem_space.rank() == 1 &&
      selected_space.size() == memory_space.size())
    h5prim_write_contiguous_data(DataBuffer, dataset, memory_type,
                                 selected_space, file_space);
  else
    h5prim_write_contiguous_data(DataBuffer, dataset, memory_type, memory_space,
                                 file_space);
}

void PrimDumpFileBase::write(const void *DataBuffer, size_t DataElmCount) {
  Slab.offset(0, count());
  Slab.block(0, DataElmCount);
  DataSet.extent({count() + DataElmCount});
  h5prim_dataset_write(DataBuffer, DataElmCount, Compound, DataSet, Slab);
}

static void
h5prim_read_contiguous_data(void *DataBuffer,
                            const hdf5::node::Dataset &dataset,
                            const hdf5::datatype::Datatype &mem_type,
                            const hdf5::dataspace::Dataspace &mem_space,
                            const hdf5::dataspace::Dataspace &file_space) {
  hdf5::property::DatasetTransferList dtpl;
  if (H5Dread(static_cast<hid_t>(dataset), static_cast<hid_t>(mem_type),
              static_cast<hid_t>(mem_space), static_cast<hid_t>(file_space),
              static_cast<hid_t>(dtpl), DataBuffer) < 0) {
    std::stringstream ss;
    ss << "Failure to read contiguous data from dataset ["
       << dataset.link().path() << "]!";
    hdf5::error::Singleton::instance().throw_with_stack(ss.str());
  }
}

static void
h5prim_dataset_read(void *DataBuffer, const size_t DataElmCount,
                    const hdf5::datatype::Compound &memory_type,
                    const hdf5::node::Dataset &dataset,
                    const hdf5::dataspace::Hyperslab &hyperSelection) {
  hdf5::dataspace::Simple memory_space(hdf5::Dimensions{DataElmCount},
                                       hdf5::Dimensions{DataElmCount});

  hid_t DataspaceId = H5Dget_space(static_cast<hid_t>(dataset));
  hdf5::dataspace::Dataspace file_space{hdf5::ObjectHandle(DataspaceId)};
  file_space.selection(hdf5::dataspace::SelectionOperation::SET,
                       hyperSelection);

  auto dims = hyperSelection.block();
  auto count = hyperSelection.count();
  for (hdf5::Dimensions::size_type i = 0; i != dims.size(); i++)
    dims[i] *= count[i];

  hdf5::dataspace::Simple selected_space(dims);
  if (selected_space.size() == memory_space.size())
    h5prim_read_contiguous_data(DataBuffer, dataset, memory_type, selected_space,
                                file_space);
  else
    h5prim_read_contiguous_data(DataBuffer, dataset, memory_type, memory_space,
                                file_space);
}

void PrimDumpFileBase::readAt(void *DataBuffer, const size_t DataElmCount,
                              size_t Index, size_t Count) {
  Slab.offset(0, Index);
  Slab.block(0, Count);
  h5prim_dataset_read(DataBuffer, DataElmCount, Compound, DataSet, Slab);
}