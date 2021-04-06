// Copyright (C) 2018-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Gem detector base plugin interface definition
///
//===----------------------------------------------------------------------===//
#pragma once

#include <common/Detector.h>
#include <gdgem/nmx/AbstractBuilder.h>
#include <gdgem/NMXStats.h>
#include <gdgem/NMXConfig.h>
#include <common/reduction/clustering/AbstractClusterer.h>
#include <common/reduction/matching/AbstractMatcher.h>
#include <common/monitor/Histogram.h>
#include <common/EV42Serializer.h>
#include <gdgem/nmx/TrackSerializer.h>

const unsigned int MinNMXChannel{0};
const unsigned int MaxNMXChannel{1279};
const unsigned int ZeroNMXOverlapSize{0};

struct NMXSettings {
  std::string ConfigFile;
  std::string CalibrationFile;
  std::string FilePrefix;

  // Parameters to handle detector partition (in x) with overlap region
  unsigned int PMin{MinNMXChannel};
  unsigned int PMax{MaxNMXChannel};
  unsigned int PWidth{ZeroNMXOverlapSize};
};



class GdGemBase : public Detector {
public:
  GdGemBase(BaseSettings const & settings, NMXSettings & LocalSettings);

  /// \brief detector specific threads
  void inputThread();
  void processingThread();

  /// \brief detector specific commands
  int getCalibration(std::vector<std::string> cmdargs, char *output,
                     unsigned int *obytes);
protected:
  struct NMXSettings NMXSettings;
  Gem::NMXConfig NMXOpts;

  std::shared_ptr<Gem::AbstractBuilder> builder_;
  std::shared_ptr<AbstractClusterer> clusterer_x_;
  std::shared_ptr<AbstractClusterer> clusterer_y_;
  std::shared_ptr<AbstractMatcher> matcher_;

  Gem::NMXStats stats_;

  Hists hists_{std::numeric_limits<uint16_t>::max(),
               std::numeric_limits<uint16_t>::max()};

  ReducedEvent neutron_event_;

  uint64_t CurrentPulseTime {0}; /// \todo get PT from data eventually
  uint32_t pixelid_;

  bool sample_next_track_ {false};

  void applyConfiguration();
  void clusterPlane(HitVector& hits, std::shared_ptr<AbstractClusterer> clusterer, bool flush);
  void performClustering(bool flush);
  void processEvents(EV42Serializer&, Gem::TrackSerializer&);
};
