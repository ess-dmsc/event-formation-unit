// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CSPEC geometry class
/// Based on CSPEC ICD documen thttps://project.esss.dk/owncloud/index.php/f/14482406

/// Mapping from digital identifiers to x-, z- and y- coordinates
//===----------------------------------------------------------------------===//

#include <cspec/geometry/CSPECGeometry.h>
#include <utility>
#include <cmath>


uint16_t Cspec::CSPECGeometry::xAndzCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t XOffset, bool Rotated){
	if (isGrid(HybridID)){
		XTRACE(DATA, WAR, "Invalid Hybrid ID for calculating X and Z coordinates");
    	// return std::pair<uint8_t, uint8_t>(Cspec::CSPECGeometry::InvalidCoord, Cspec::CSPECGeometry::InvalidCoord);
    	return 65535;
  	}
	uint8_t LocalXCoord = 0;
	uint8_t ZCoord = 0;

	//Channel mappings for each VMM are detailed in ICD document
	if (VMMID == 0){
		if (Channel < 48){
			LocalXCoord = 0;
		}
		else if (Channel < 64){
			LocalXCoord = 1;
		}
		else{
			XTRACE(DATA, WAR, "Invalid VMM ID and Channel combination for calculating X and Z coordinates");
			// return std::pair<uint8_t, uint8_t>(InvalidCoord, InvalidCoord);
    		return 65535;
    	}
	}
	else if (VMMID == 1){
		LocalXCoord = floor(Channel/16) + 2;
	}
	ZCoord = Channel % 16;

	if (Rotated){
		LocalXCoord = 5 - LocalXCoord;
	}
	//X and Z Coordinates
	uint16_t ComboCoordinate = 16 * (LocalXCoord + XOffset) + ZCoord;
	return ComboCoordinate;
}

uint8_t Cspec::CSPECGeometry::yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t YOffset, bool Rotated){
	uint8_t YCoord;
	//Channel mappings for Y coordinates/grids are detailed in ICD document
	if (HybridID == 1){
		if (VMMID == 0){
			YCoord = 139 - (Channel - 58);
		}
		else if (VMMID == 1){
			YCoord = 133 - Channel;
		}
		else{
			XTRACE(DATA, WAR, "Invalid VMM ID %u for HybridID 1", VMMID);
			return 255;
		}
	}
	else if (HybridID == 2){
		if (VMMID == 0){
			YCoord = 69 - Channel;
		}
		else if (VMMID == 1){
			YCoord = 5 - Channel;
		}
		else{
			XTRACE(DATA, WAR, "Invalid VMM ID %u for HybridID 2", VMMID);
			return 255;
		}
	}
	else{
		XTRACE(DATA, WAR, "Invalid HybridID %u", HybridID);
		return 255;
	}

	if (Rotated){
		YCoord = - YCoord;
	}
	YCoord += YOffset;
	return YCoord;
}

