
#define H5_USE_NEW_COMPOUND_IMPL 1
#include <common/DumpFile.h>
#undef H5_USE_NEW_COMPOUND_IMPL
#include <common/Assert.h>
#include <common/Version.h>
#include <fmt/format.h>
#include <memory>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

class PrimDumpFileBase_Impl : public PrimDumpFileBase {
public:
  friend class PrimDumpFileBase;

  void openRW();
  void openR();

  size_t count() const;
  bool isFileValidNonReadonly();

  void write(const void *DataBuffer, size_t DataElmCount);

  void readAt(void *DataBuffer, const size_t DataElmCount, size_t Index,
              size_t Count);

  void rotate();

private:
  const H5PrimCompoundDef &CompoundDef;

  size_t SequenceNumber{0};

  hdf5::file::File File;
  hdf5::datatype::Compound Compound;
  hdf5::node::Dataset DataSet;
  hdf5::dataspace::Hyperslab Slab;

  boost::filesystem::path PathBase{};

  PrimDumpFileBase_Impl(const H5PrimCompoundDef &CompoundDef,
                        const boost::filesystem::path &file_path);

  boost::filesystem::path get_full_path() const;
};

//-----------------------------------------------------------------------------

std::unique_ptr<PrimDumpFileBase>
PrimDumpFileBase::create(const H5PrimCompoundDef &CompoundDef,
                         const std::string &file_path) {
  return std::unique_ptr<PrimDumpFileBase>(
      new PrimDumpFileBase_Impl(CompoundDef, file_path));
}

PrimDumpFileBase::PrimDumpFileBase(const H5PrimCompoundDef &CompoundDef)
    : ChunkSize(GetChunkSize(CompoundDef.StructSize)) {}

struct PrimToH5tNativeMap {
  hid_t Map[static_cast<int>(H5Prim::Count)] = {};
  PrimToH5tNativeMap() {
    Map[static_cast<int>(H5Prim::Bool)] = H5T_NATIVE_HBOOL;
    Map[static_cast<int>(H5Prim::Char)] = H5T_NATIVE_CHAR;
    Map[static_cast<int>(H5Prim::UnsignedChar)] = H5T_NATIVE_UCHAR;
    Map[static_cast<int>(H5Prim::SignedChar)] = H5T_NATIVE_SCHAR;
    Map[static_cast<int>(H5Prim::Short)] = H5T_NATIVE_SHORT;
    Map[static_cast<int>(H5Prim::UnsignedShort)] = H5T_NATIVE_USHORT;
    Map[static_cast<int>(H5Prim::Int)] = H5T_NATIVE_INT;
    Map[static_cast<int>(H5Prim::UnsignedInt)] = H5T_NATIVE_UINT;
    Map[static_cast<int>(H5Prim::Long)] = H5T_NATIVE_LONG;
    Map[static_cast<int>(H5Prim::UnsignedLong)] = H5T_NATIVE_ULONG;
    Map[static_cast<int>(H5Prim::LongLong)] = H5T_NATIVE_LLONG;
    Map[static_cast<int>(H5Prim::UnsignedLongLong)] = H5T_NATIVE_ULLONG;
    Map[static_cast<int>(H5Prim::Float)] = H5T_NATIVE_FLOAT;
    Map[static_cast<int>(H5Prim::Double)] = H5T_NATIVE_DOUBLE;

    // verify all slots have been written
    for (int i = 0; i < static_cast<int>(H5Prim::Count); i++) {
      if (Map[i] == 0) {
        throw std::runtime_error(fmt::format(
            "Missing H5Prim to H5T_NATIVE_* mapping at H5Prim enum item {}",
            i));
      }
    }
  }
};

static hid_t PrimToH5tNative(H5Prim Prim) {
  static PrimToH5tNativeMap Map;
  return Map.Map[static_cast<int>(Prim)];
}

hdf5::datatype::Compound
createCompoundFromH5PrimCompoundDef(const H5PrimCompoundDef &CompoundDef) {
  hdf5::datatype::Compound Compound =
      hdf5::datatype::Compound::create(CompoundDef.StructSize);
  for (size_t i = 0; i < CompoundDef.MembersCount; i++) {
    const H5PrimDef &Def = CompoundDef.Members[i];
    hid_t H5fNativeType = PrimToH5tNative(Def.PrimType);
    Compound.insert(
        Def.Name, Def.Offset,
        hdf5::datatype::Datatype(hdf5::ObjectHandle(H5Tcopy(H5fNativeType))));
  }
  return Compound;
}

void Hdf5ErrorSetAutoPrint(bool enable) {
  hdf5::error::Singleton::instance().auto_print(enable);
}

std::string Hdf5ErrorPrintNested(const std::exception &exception, int level) {
  return hdf5::error::print_nested(exception, level);
}

PrimDumpFileBase_Impl::PrimDumpFileBase_Impl(
    const H5PrimCompoundDef &CompoundDef,
    const boost::filesystem::path &file_path)
    : PrimDumpFileBase(CompoundDef), CompoundDef(CompoundDef),
      Slab({0}, {ChunkSize}) {
  Compound = createCompoundFromH5PrimCompoundDef(CompoundDef);
  PathBase = file_path;
}

void PrimDumpFileBase::openRW() {
  static_cast<PrimDumpFileBase_Impl *>(this)->openRW();
}
void PrimDumpFileBase_Impl::openRW() {
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
  static_cast<PrimDumpFileBase_Impl *>(this)->openR();
}
void PrimDumpFileBase_Impl::openR() {
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
  return static_cast<const PrimDumpFileBase_Impl *>(this)->count();
}
size_t PrimDumpFileBase_Impl::count() const {
  return hdf5::dataspace::Simple(DataSet.dataspace())
      .current_dimensions()
      .at(0);
}

bool PrimDumpFileBase::isFileValidNonReadonly() {
  return static_cast<PrimDumpFileBase_Impl *>(this)->isFileValidNonReadonly();
}
bool PrimDumpFileBase_Impl::isFileValidNonReadonly() {
  return File.is_valid() &&
         (File.intent() != hdf5::file::AccessFlags::READONLY);
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
  static_cast<PrimDumpFileBase_Impl *>(this)->write(DataBuffer, DataElmCount);
}
void PrimDumpFileBase_Impl::write(const void *DataBuffer, size_t DataElmCount) {
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
    h5prim_read_contiguous_data(DataBuffer, dataset, memory_type,
                                selected_space, file_space);
  else
    h5prim_read_contiguous_data(DataBuffer, dataset, memory_type, memory_space,
                                file_space);
}

void PrimDumpFileBase::readAt(void *DataBuffer, const size_t DataElmCount,
                              size_t Index, size_t Count) {
  static_cast<PrimDumpFileBase_Impl *>(this)->readAt(DataBuffer, DataElmCount,
                                                     Index, Count);
}
void PrimDumpFileBase_Impl::readAt(void *DataBuffer, const size_t DataElmCount,
                                   size_t Index, size_t Count) {
  Slab.offset(0, Index);
  Slab.block(0, Count);
  h5prim_dataset_read(DataBuffer, DataElmCount, Compound, DataSet, Slab);
}

void PrimDumpFileBase::rotate() {
  static_cast<PrimDumpFileBase_Impl *>(this)->rotate();
}
void PrimDumpFileBase_Impl::rotate() {
  SequenceNumber++;
  openRW();
}

boost::filesystem::path PrimDumpFileBase_Impl::get_full_path() const {
  auto Ret = PathBase;
  Ret += ("_" + fmt::format("{:0>5}", SequenceNumber) + ".h5");
  return Ret;
}
