
## Data processing logic
Timepix data processing handles 3 kind of data type:
* TDC: Camera clock timesamp produced for EVR pulse
* EVR: ESS timestamps from EVR, produced for each EVR pulse
* PIXEL: Activated pixel coordinates and timestamp

### TDC and EVR syncronization
To syncronize data according to the EVR system we synconize EVR and TDC times by pairing them according to their arrival. We check for the arrival difference between the last received EVR or TDC and the currently received packet. If this difference is small enough then we consider these two packets as pairs and we create a **global time** object. The object stores the global time stamp which is expressed in EPOCH nanosecound precesion and it stores also the releated TDC timestamp.

```marmaid
classDiagram

class GlobalTime {
    + const uint64_t globalTimeStamp
    + const uint32_t tdcTimeStamp
    + GlobalTime(globalTimeStamp, tdcTimeStamp)
}
```

## Pixel data processing

For pixel data we calculate their arrival time in EPOCH nanosec global time domain from the previously identified global timestamp.


### Packet processing mechanism
Following chart will introduce the how we process each UDP packets. In case of EVR the UDP packet contains only one segment, therefore we use one processing step. The other cases we process the segments in 8 bytes until we read out all data from the UDP packet.

```mermaid
flowchart TD

UDPPacket[New UDP Packet]
NextUDPPacket[Get Next UDP Packet]
CalculateSize[Calculate UDP packet size]
ProcessPacketContent[Process the data]
GetNext8Byte[Read out next 8 bytes]
CalculateClusters[Execute clustering\n identify neutron events]
PublishtoKafKa[Publish neutron events to Kafka]

Is8BytesLeft{Is not readed packet\n size is >= 8byte}
ISEVRPacket{Is PacketSize == 24 byte}

UDPPacket --> CalculateSize
CalculateSize --> ISEVRPacket
ISEVRPacket -->|YES| ProcessPacketContent
ProcessPacketContent -->|EVR| NextUDPPacket
ISEVRPacket -->|NO| Is8BytesLeft
Is8BytesLeft -->|YES| GetNext8Byte
GetNext8Byte --> ProcessPacketContent
ProcessPacketContent --> Is8BytesLeft
Is8BytesLeft --> |NO| CalculateClusters --> PublishtoKafKa
PublishtoKafKa --> NextUDPPacket
```
The following charts introduce how we process each data segment of the UDP.

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
UpdateEVRCounter[Update EVR Counter\n currentCounter = EVRCounter]
UpdateTDCCounter[Update TDC Counter\n currentCounter = TDCCounter]

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
IsLastTDCPair --> |NO| StoreLastEVR --> NextPacket
IsLastTDCPair --> |YES| UpdateCurrentGlobalTimeStamp --> UpdateEVRCounter 
UpdateEVRCounter --> NextPacket


Parsing -->|TDC| CheckTDCCounter
CheckTDCCounter -->|NO| IncreaseMissTDC --> IsLastEVRPair
CheckTDCCounter -->|YES| IsLastEVRPair
IsLastEVRPair -->|YES| UpdateCurrentGlobalTimeStamp --> UpdateTDCCounter
UpdateTDCCounter-->  NextPacket
IsLastEVRPair -->|NO| StoreLastTDC --> NextPacket
```

### Pixel Time calculation
The following chart shows the steps of the processing of individual pixel. Since Pixel clock resets 4 times during one TDC clock period therefore we have to identify which quarter we processing pixels. If we know the quarter then we can calculate TDC clock in pixel time for that case. We use the following mathematical formula to convert TDC clock into pixel time domain:
```c++
pixelClockBin = 26.8435456 * 1e9;
quarter = int(TDCTimeSamp / pixelClockBin);
TDCinPixelTime = TDCTimeSamp - (pixelClockBin * quarter)
```

```mermaid
flowchart TD

NewPixel[New Pixel Packet]
NextPixel[Next Packet]

CalculatePixelGlobalTimestamp[Calculate Pixel Global TimeStamp]
DelayedPixelCounter[Increase delayed pixel counter]
StorePixel[Store pixel in buffer]

IsPixelBeforeLastTDC{Check if pixel is smaller \nthen the TDC timestamp\n of the last global time\n in pixel time domain}
CheckPixelIsAfterReset{Check pixel timestamp\n is smaller then frequency\n in NS}

NewPixel --> IsPixelBeforeLastTDC
IsPixelBeforeLastTDC -->|YES| CheckPixelIsAfterReset
IsPixelBeforeLastTDC --> |NO| CalculatePixelGlobalTimestamp
CheckPixelIsAfterReset -->|YES| CalculatePixelGlobalTimestamp
CalculatePixelGlobalTimestamp --> StorePixel --> NextPixel
CheckPixelIsAfterReset --> |NO| DelayedPixelCounter -->NextPixel
```