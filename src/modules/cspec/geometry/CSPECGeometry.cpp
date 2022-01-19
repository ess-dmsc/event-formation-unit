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


uint16_t Cspec::CSPECGeometry::xAndzCoord(uint8_t FENID, uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t XOffset, bool Rotated){
	if (isGrid(HybridID)){
		XTRACE(DATA, WAR, "Invalid Hybrid ID for calculating X and Z coordinates");
    	// return std::pair<uint8_t, uint8_t>(Cspec::CSPECGeometry::InvalidCoord, Cspec::CSPECGeometry::InvalidCoord);
    	return 65535;
  	}
	
	//Wire equation defined in CSPEC ICD Document
	uint16_t Wire = (HybridID * 2 + VMMID) * 64 + Channel - 32;

	//odd FENs are second column in a vessel, 6 * 16 wires over
	if (FENID % 2){
		Wire = Wire + 6 * 16;
	}

	//when rotated local X coordinate is flipped around centre, ie 11-XCoord
	//Z coord remains the same
	//Then turning value back into wire value/combination of X and Z
	if (Rotated){
		uint8_t LocalXCoord = floor(Wire/16);
		uint8_t LocalZCoord = Wire % 16;
		LocalXCoord = 11 - LocalXCoord;
		Wire = 16 * LocalXCoord + LocalZCoord;
	}

	Wire += 16 * XOffset;
	return Wire;
}

uint8_t Cspec::CSPECGeometry::yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t YOffset, bool Rotated, bool Short){
	uint8_t YCoord;
	//Channel mappings for Y coordinates/grids are detailed in ICD document
	if (!Short){ // channel mapping for full length vessel
		YCoord = (HybridID * 2 + VMMID - 2) * 64 + Channel - 58;
	}
	else{ // channel mapping for short vessel
		if (HybridID == 1 and VMMID == 0 and Channel < 51){
			YCoord = Channel;
		}
		else{
			XTRACE(DATA, ERR, "Invalid Hybrid, VMM, or Channel for rotated CSPEC vessel, HybridID %u, VMM %u, Channe; %u", HybridID, VMMID, Channel);
			return 255;
		}
	}

	//when rotated grid order is reversed, and YOffset should be used to place vessel correctly
	if (Rotated) {
		YCoord = - YCoord;
	}

	YCoord += YOffset;
	return YCoord;
}

