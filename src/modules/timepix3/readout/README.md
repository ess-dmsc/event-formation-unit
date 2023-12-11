
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
IsLastEVRPair{Check if\n lastEVRArrival - currentTDCArrival\n < frequency/2}
IsLastTDCPair{Check if\n lastTDCArrival - currentEVRArrival\n < frequency/2}
IsGlobalTimestampExists{Check if\n globalTimeStamp != nullptr}
CheckEVRCounter{Check if\n EVRCounter == currentEVRCounter + 1}
CheckTDCCounter{Check if\n TDCCounter == currentTDCCounter + 1}

UpdateCurrentGlobalTimeStamp[Update current global timestamp]
StoreLastTDC[Store TDC as lastTDC]
StoreLastEVR[Store EVR as lastEVR]
IncredaseMissEVR[Increase MissEVRCounter]
IncreaseMissTDC[Increase MissTDCCounter]
CalculatePixelGlobalTimestamp[Calculate the global timestamp of a pixel]
StorePixelInBuffer[Store pixel in buffer]

NextPacket(Next Data packet)
NewPacket(New Data packet)

NewPacket --> Parsing

Parsing -->|PIXEL| IsGlobalTimestampExists
IsGlobalTimestampExists -->|NO| NextPacket
IsGlobalTimestampExists -->|YES| CalculatePixelGlobalTimestamp
CalculatePixelGlobalTimestamp --> StorePixelInBuffer --> NextPacket

Parsing -->|EVR| CheckEVRCounter 
CheckEVRCounter -->|NO| IncredaseMissEVR
IncredaseMissEVR --> IsLastTDCPair
CheckEVRCounter -->|YES| IsLastTDCPair
IsLastTDCPair --> |NO| StoreLastEVR --> NewPacket
IsLastTDCPair --> |YES| UpdateCurrentGlobalTimeStamp --> NewPacket


Parsing -->|TDC| CheckTDCCounter
CheckTDCCounter -->|NO| IncreaseMissTDC --> IsLastEVRPair
CheckTDCCounter -->|YES| IsLastEVRPair
IsLastEVRPair -->|YES| UpdateCurrentGlobalTimeStamp --> NextPacket
IsLastEVRPair -->|NO| StoreLastTDC --> NextPacket
```