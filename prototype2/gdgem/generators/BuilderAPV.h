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
#include <gdgem/nmx/Hit.h>

class BuilderAPV : public AbstractBuilder {
public:
  BuilderAPV(std::string dump_dir, bool dump_csv, bool dump_h5);

  /// \todo Martin document
  ResultStats process_buffer(char *buf, size_t size) override;

private:
  size_t psize{sizeof(uint32_t) * 4};
  std::vector<uint32_t> data;

  void make_hit(size_t idx);

  std::shared_ptr<HitFile> hit_file_;
  std::vector<Hit> converted_data;
};
