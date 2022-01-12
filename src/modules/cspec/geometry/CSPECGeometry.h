// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
/// Based on CSPEC ICD documen thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#include <common/debug/Trace.h>
#include <Geometry.h>


namespace CSPEC {

class CSPECGeometry : public Geometry{
public:
	bool isWire(uint8_t LocalHybridID){
		if (LocalHybridID == 0){
			return true;
		}
		else{
			return false;
		}
	}

	bool isGrid(uint8_t LocalHybridID){
		return !isWire(LocalHybridID);
	}
}

} // namespace CSPEC