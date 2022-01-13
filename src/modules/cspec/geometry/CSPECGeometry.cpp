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

std::pair<uint8_t, uint8_t> Cspec::CSPECGeometry::xAndzCoord(uint8_t RingID, uint8_t FENID, uint8_t HybridID, uint8_t VMMID, uint8_t Channel){
	if (isGrid(HybridID)){
		XTRACE(DATA, WAR, "Invalid Hybrid ID for calculating X and Z coordinates");
    	// return std::pair<uint8_t, uint8_t>(Cspec::CSPECGeometry::InvalidCoord, Cspec::CSPECGeometry::InvalidCoord);
    	return std::pair<uint8_t, uint8_t>(255, 255);
  	}
	uint8_t LocalXCoord = 0;
	uint8_t ZCoord = 0;

	//Offset from RingID, each Ring is 4 vessels, each vessel is 12 wires wide
	uint8_t XOffset = 4 * 12 * RingID;

	//Add offset from FENID, each FEN is 1 column, each column is 6 wires wide
	XOffset += 6 * FENID;

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
    		return std::pair<uint8_t, uint8_t>(255, 255);
    	}
	}
	else if (VMMID == 1){
		LocalXCoord = floor(Channel/16) + 2;
	}
	ZCoord = Channel % 16;
	std::pair<uint8_t, uint8_t> Coord(LocalXCoord + XOffset, ZCoord);
	return Coord;
}

uint8_t Cspec::CSPECGeometry::yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel){
	//Channel mappings for Y coordinates/grids are detailed in ICD document
	if (HybridID == 1){
		if (VMMID == 0){
			return 139 - (Channel - 58);
		}
		else if (VMMID == 1){
			return 133 - Channel;
		}
	}
	else if (HybridID == 2){
		if (VMMID == 0){
			return 69 - Channel;
		}
		if (VMMID == 1){
			return 5 - Channel;
		}
	}
	XTRACE(DATA, WAR, "Invalid VMM ID and Channel combination for calculating X and Z coordinates");
	//return InvalidCoord;
	return 255;
}

