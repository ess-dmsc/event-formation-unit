
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
PairingTDC{Is pairing true?}
PairingEVR{Is pairing true?}
FindPairPreviousTDC{Is previous TDC and EVR\nTimDiff <= Threshold ?}
FindPairTimeDiff{Is TDC and EVR \nTimeDiff <= Threshold ?}

SetPairingTrue[Set pairing true]
SetPairingFalse[Set pairing false]
StoreEVRData[Store EVR as last EVR]
StoreTDCData1[Store TDC as last TDC]
StoreTDCData2[Store TDC as last TDC]
StoreTDCPair[Store TDC as last TDC pair]
MissingTDCPair[Increment missing\nTDC pair counter]
MissingEVRPair[Increment missing\nEVR pair counter]

NextPacket(Next Data packet)
NewPacket(New Data packet)

NewPacket --> Parsing
Parsing -->|EVR| StoreEVRData --> PairingEVR
Parsing -->|TDC| PairingTDC

PairingTDC -->|YES| StoreTDCData1 --> FindPairTimeDiff
PairingTDC -->|NO| MissingEVRPair --> StoreTDCData2
StoreTDCData2 --> NextPacket


PairingEVR -->|NO| SetPairingTrue --> FindPairPreviousTDC
PairingEVR -->|YES| MissingTDCPair --> SetPairingTrue

FindPairTimeDiff -->|YES| StoreTDCPair --> SetPairingFalse
FindPairTimeDiff -->|No| NextPacket

FindPairPreviousTDC -->|NO| NextPacket
FindPairPreviousTDC -->|YES| StoreTDCPair

SetPairingFalse --> NextPacket
```