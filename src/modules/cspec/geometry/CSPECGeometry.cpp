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
	if (VMMID == 0){ // Wires in X=0 and 1 represented by VMM0
		if (Channel < 48){ //Channels 32-47 are in X=0
			LocalXCoord = 0;
		}
		else if (Channel < 64){ //Channels 48-63 are in X=1
			LocalXCoord = 1;
		}
		else{
			XTRACE(DATA, WAR, "Invalid VMM ID and Channel combination for calculating X and Z coordinates");
			// return std::pair<uint8_t, uint8_t>(InvalidCoord, InvalidCoord);
    		return 65535;
    	}
	}
	else if (VMMID == 1){ //Wires in X=2,3,4 and 5 are represented by VMM1
		LocalXCoord = floor(Channel/16) + 2;
	}
	//Vessels are 16 wires deep, and channels count up in Z direction, with %16=0 at the front of the vessel
	ZCoord = Channel % 16;

	//When the vessel is rotated the X coordinates are reversed, Z remains the same
	if (Rotated){
		LocalXCoord = 5 - LocalXCoord;
	}
	//X and Z Coordinates are combined into one unique value for clustering purposes
	//Will later be separated back into X and Z coordinates
	uint16_t ComboCoordinate = 16 * (LocalXCoord + XOffset) + ZCoord;
	return ComboCoordinate;
}

uint8_t Cspec::CSPECGeometry::yCoord(uint8_t HybridID, uint8_t VMMID, uint8_t Channel, uint16_t YOffset, bool Rotated, bool Short){
	uint8_t YCoord;
	//Channel mappings for Y coordinates/grids are detailed in ICD document
	if (!Short){ // channel mapping for full length vessel
		if (HybridID == 1){
			if (VMMID == 0){
				// channels 58-63 represent grids 0-5
				YCoord = Channel - 58;
			}
			else if (VMMID == 1){
				// channels 0-63 represent grids 6-69
				YCoord = 6 + Channel;
			}
			else{
				XTRACE(DATA, WAR, "Invalid VMM ID %u for HybridID 1", VMMID);
				return 255;
			}
		}
		else if (HybridID == 2){
			if (VMMID == 0){
				//70 grids represented by hybrid 1
				//channels 0-63 represent grids 70-133
				YCoord = 70 + Channel;
			}
			else if (VMMID == 1){
				//70 grids represented by hybrid 1, and 64 by hybrid 2, vmm 0
				//channels 0-5 represent grids 134-139
				YCoord = 134 + Channel;
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
	}
	else{ // channel mapping for short vessel
		if (HybridID == 1 and VMMID == 0){
			YCoord = Channel;
		}
		else{
			XTRACE(DATA, ERR, "Invalid Hybrid or VMM for rotated CSPEC vessel, HybridID %u, VMM %u", HybridID, VMMID);
		}
	}

	//when rotated grid order is reversed, and YOffset should be used to place vessel correctly
	if (Rotated) {
		YCoord = - YCoord;
	}

	YCoord += YOffset;
	return YCoord;
}

