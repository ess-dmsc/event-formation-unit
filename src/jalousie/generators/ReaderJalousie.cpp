/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <jalousie/Readout.h>
#include <jalousie/generators/ReaderJalousie.h>
// GCOVR_EXCL_START


int ReaderJalousie::getNextReadout(char * buffer) {
    Jalousie::Readout readout;

  	while (!feof(FilePtr))
  	{
  		// readbuffer
  		uint64_t data = 0;

  		// read and interpret data
  		fread((char*)&data, sizeof(uint64_t),1,FilePtr);
      Entries++;

  		switch (data & ((0x3ULL) << 62))
  		{
  		// NEUTRON
  		case kNeutronDataType:
  			{
          readout.board = BoardId;
  				readout.anode   = uint8_t(data & 0x3F);
  				readout.cathode = uint8_t((data & 0x1FC0) >> 6);
  				readout.time = (data & (0x1FFFFFFFFFFFE000ULL)) >> 13;
  				readout.sub_id = uint8_t(data & ((1ULL)<<58));
  				//printf("NEUTRON DATA : time: 0x%016llX  cathode: 0x%0X  anode: 0x%0X subId; %d\n",
          //    readout.time,readout.cathode,readout.anode,readout.sub_id);
          memcpy(buffer, &readout, sizeof(readout));
          return 1;
  			}
  			break;
  		// METADATA
  		case kMetaDataType:
  			switch (data & ((0xFULL) << 58))
  			{
  			case kMetaDataIndex1:
  				switch (data & (uint64_t(0xF) << 54))
  				{
  				case kMetaDataSubIndex0:
  					{
  						uint64_t dataTime = 0;
  						dataTime = (data&(uint64_t(0xFFFFFFFFFFFF)));
  						printf("CHOPPER TIMESTAMP : time: 0x%" PRIu64 "\n",dataTime);
  					}
  					break;
  				case kMetaDataSubIndex1:
  					{
  						uint32_t lastBoard = (data & 0xffffff);
  						printf("BOARD-ID: id: %u\n",lastBoard);
              BoardId = lastBoard;
  					}
  					break;
  				default:
  					break;
  				}
  				break;
  			default:
  					break;
  				}
  			break;
  		default:
  			break;
  			}
  		}
      return 0;
}

ReaderJalousie::ReaderJalousie(std::string filename) {

  FilePtr = fopen(filename.c_str(), "rb");
  if (FilePtr == nullptr) {
    printf("cant open file %s\n", filename.c_str());
  }
}

ReaderJalousie::~ReaderJalousie() { fclose(FilePtr); }

int ReaderJalousie::read(char *buffer, size_t bufferlen) {
  if (FilePtr == nullptr) {
    return -1;
  }

  if (not HeaderRead) {
    int header[8];
    fread(header,8*sizeof(int),1,FilePtr);
    HeaderRead = true;
  }

  size_t Size = 0;
  int Readouts = 0;
  auto bp = buffer;
  size_t remaining = bufferlen;
  int res;
  while (remaining >= sizeof(Jalousie::Readout)) {
     if ((res = getNextReadout(bp)) == 0) {
       return -1;
     }
     Readouts++;
     bp += sizeof(Jalousie::Readout);
     remaining -= sizeof(Jalousie::Readout);
     Size += sizeof(Jalousie::Readout);
  }
  printf("Returning %d readouts, total size: %zu\n", Readouts, Size);
  return Size;
}


// GCOVR_EXCL_STOP
