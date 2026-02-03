// Copyright (C) 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Backward compatibility header - includes all CBM geometry classes
///
/// \deprecated This header is provided for backward compatibility.
/// Please include the specific geometry headers directly:
/// - Geometry2D.h for EVENT_2D monitors
/// - Geometry0D.h for EVENT_0D and IBM monitors
///
//===----------------------------------------------------------------------===//

#pragma once

#include <modules/cbm/geometry/Geometry2D.h>
#include <modules/cbm/geometry/Geometry0D.h>

namespace cbm {

/// \brief Type alias for backward compatibility
/// \deprecated Use Geometry2D directly
using CbmGeometry = Geometry2D;

} // namespace cbm
