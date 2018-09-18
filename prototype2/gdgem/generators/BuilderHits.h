/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for creating NMX hits from h5 data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/nmx/HitFile.h>

class BuilderHits : public AbstractBuilder {
public:
  BuilderHits(std::string dump_dir, bool dump_csv, bool dump_h5);

  /// \todo Martin document
  ResultStats process_buffer(char *buf, size_t size) override;

private:
  static constexpr size_t psize{sizeof(Hit)};

  std::shared_ptr<HitFile> hit_file_;
  std::vector<Hit> converted_data;
};
