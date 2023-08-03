# Reduction

Reduction covers clustering and coincidence matching for systems where multiple readouts make-up a single event.

## No reduction necessary

Many modules don't rely on the reduction library because a single readout is a single event. These systems include:
- LoKI
- Bifrost
- Miracles
- Magic
- Dream


## Separate X and Y readouts

Systems where readouts for the X and Y planes arrive separate and need to be matched, we use an object of type AbstractClusterer, forming clusters in time for each plane, followed by an object of type AbstractMatcher to combine clusters into 2D events. These systems include:
- NMX
- Freia
- Estia
- Amor

## Joint X and Y readouts

Systems where readouts contain both X and Y information - but don't represent single neutron events and still need reduction - we use an object of type Abstract2DClusterer, forming clusters in time, and then splitting those into clusters in space. These systems include:
- Timepix3