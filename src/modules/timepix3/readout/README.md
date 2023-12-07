
## Data processing logic
Timepix data processing handles 3 kind of data type:
* TDC: Camera clock timesamp produced for EVR pulse
* EVR: ESS timestamps from EVR, produced for each EVR pulse
* PIXEL: Activated pixel coordinates and timestamp

To syncronize data according to the EVR system we synconize EVR and TDC times by pairing them according to their arrival. Then later each pixel must be associated to a certain TDC for the clock calculation and for the groupping withthe EVR time header. The charts below introduce the pairing mechanism:

### TDC and EVR pairing mechanism
```mermaid
flowchart TD

Parsing{What type of packet?}
IsPixelNextTDC{Check if \npixel timestamp > nextTDCTimeStamp}
IsTDCCurrent{Check if \nTDC < current TDC + frequency/2}
IsNextTDCExists{Check if\n nextTDCTimeStamp != nullptr}
CheckCurrentBuffer{Check if not\n currentBuffer.empty}
CheckEVRCounter{Check if\n EVRCounter == currentEVRCounter + 1}
CheckTDCCounter{Check if\n TDCCounter == currentTDCCounter + 1}

UpdateCurrentTDCTimeStamp[Update timestamp calculated for current TDC]
UpdateNextTDCTimeStamp[Update timestamp calculated for next TDC]
PublishCurrentPixels[Publish to Kafka all Pixels\n in current buffer]
CopyNextBufferToCurrent[Copy next buffer into current buffer]
UpdateTimings[Copy next TDC and EVR timestamps to current]
CalculateNext[Calculate next TDC and EVR timesamps]
SetSerialiserReferenceTime[Set current EVR time as reference time]
StorePixelDataNext[Store pixel into next buffer]
StorePixelDataCurrent[Store pixel data into current buffer]
UpdateEVRCounter[Update currentCounter with EVRCounter]
EmptyBuffers[Empty current and next buffers\n Drop all pixels]
IncreaseMissEVR[Increase MissEVRCounter]
IncreaseMissTDC[Increase MissTDCCounter]

NextPacket(Next Data packet)
NewPacket(New Data packet)

NewPacket --> Parsing

Parsing -->|PIXEL| IsNextTDCExists
IsNextTDCExists -->|NO| NextPacket
IsNextTDCExists -->|YES| IsPixelNextTDC
IsPixelNextTDC -->|YES| StorePixelDataNext --> NextPacket
IsPixelNextTDC -->|NO| StorePixelDataCurrent --> NextPacket

Parsing -->|EVR| CheckEVRCounter 
CheckEVRCounter -->|NO| IncreaseMissEVR
IncreaseMissEVR--> EmptyBuffers --> SetSerialiserReferenceTime
CheckEVRCounter -->|YES| CheckCurrentBuffer
CheckCurrentBuffer -->|NO| SetSerialiserReferenceTime
CheckCurrentBuffer -->|YES| PublishCurrentPixels --> CopyNextBufferToCurrent
CopyNextBufferToCurrent --> UpdateTimings --> CalculateNext
CalculateNext --> SetSerialiserReferenceTime
SetSerialiserReferenceTime --> UpdateEVRCounter --> NextPacket

Parsing -->|TDC| CheckTDCCounter
CheckTDCCounter -->|NO| IncreaseMissTDC --> IsTDCCurrent
CheckTDCCounter -->|YES| IsTDCCurrent
IsTDCCurrent -->|YES| UpdateCurrentTDCTimeStamp --> NextPacket
IsTDCCurrent -->|NO| UpdateNextTDCTimeStamp --> NextPacket
```