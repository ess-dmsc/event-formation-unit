# Hit

The most basic building block of clusters and events.

### Assumptions:
* `time` field likely means nanoseconds, but this may not currently be true for all readout architectures.
* `weight` is usually synonymous with ADC value received from digitizer.
* The (`plane`, `coordinate`) tuple describes digital or logical geometry.

### Geometry considerations:
* Plane can be literally a plane, such as `x` or `y` in the case of area detectors such as Gadolinium gem.
* Plane can also identify something like `grid` vs. `wire`.
* Plane can also encode panel or module of more complex detector, i.e.
    - `plane`=0 -> module 0 grid
    - `plane`=1 -> module 0 wire
    - `plane`=2 -> module 1 grid
    - `plane`=3 -> module 1 wire
    - etc..
* Plane can also identify hits that actually encode pulse or chopper TDC events.
* Scoped constants are defined for `InvalidPlane` (should be =255) and `PulsePlane` (=254)
* In case of wire IDs, the `coordinate` field is not a contiguous coordinate

### Future problems & prospects:
* uint8 may not be enough reslution for `plane` if a detector turns out to be more complex,
i.e. if there are many isolated detector modules for which clustering should happen separately.
If we assume 2 logical planes per event-producing space, then a single EFU pipeline could handle
up to about 127 modules. This may be adequate. However, if at some point we resort to writing out
raw hit data using the `Hit` type (i.e. after resolution of time and geometry from readout-specific
`Readout` type, which is now most commonly written to h5), then there may be ambiguity in what
planes are actually being reported.

# Cluster
A cluster is a collection of one or more `Hits` in one plane. The cluster usually constitutes
one half of an `Event`, which is a pair of clusters matched across physical or logical planes.
The aim of this class is to facilitate clustering, matching and later analysis of cluster.

The cluster is essentially a container of `Hits`, which also keeps track of the following:
* plane identity of the cluster
* time bounds of the cluster
* coordinate bounds of the cluster
* total weight of the cluster (sum of `Hit` weights)
* coordinate and time mass of the cluster

The class also provides convenience functions for:
* merging clusters
* determining if clusters orverlap in time
* calculating the time gap between clusters
* visualizing cluster in "ASCII grayscale" for debug purposes

### Plane identity of cluster:
The `valid` method only returns true if cluster is non-empty and plane is valid.
All hits within the cluster should belong to the same plane. If events are inserted belonging to
differentc planes, plane identity is invalidated. It will remain invalid until cluster is
cleared and new hits (belonging to the same plane) are inserted.

### Time and coordinate bounds
Time and coordinate bounds are useful for clustering and cluster matching. This is why the class
keeps track of them as hits are added.
Time bounds are reported as inclusive of the end points with the assumption that one
timebin has the uncertainty of 1 time unit (most probably nanosecond, but implementation does not
care).

Edge cases:
* no hits in cluster: start and end times are undefined, reported time-span is 0
* one hit in cluster: start and end times will be equal to the one hit's time-stamp; reported time-span is 1
* two hits in cluster: start and end times will be min and max of the timestamps of the two hits; time-span = max - min + 1

Same assumptions hold for coordinate bounds calculation.

### Weights and centers of mass
Centers of mass in time and coordinate space are potentially useful in cluster matching and
event coordinate calculation in the final reduction step.
Since it is easiest to calculate weighted averages by accumulating values, the class automatically
keeps track of these while adding hits to cluster. It may be that this is not always needed,
so for performance's sake it might make sense to (optionally) disable this feature.

### Calculation of time gaps and overlaps
Time overlap between two clusters is calculated with the same assumptions as time-span of a cluster.
If two clusters happen to have touch at one time-point, i.e. end-time of one is the start-time of the
other, then the time-overlap will be reported as 1.
For the same two clusters, time-gap will be reported as 0.
No negative time overlaps or time-gaps are possible. If clusters do not overlap, the overlap is 0.
The `time_gap` method requires more testing. Currently, two empty clusters are aveluated as having
a time gap of 0, which is not strictly true, so these assumptions need to be further refined.

### Encapsulation considerations:
The idea behind this class is that hits should be inserted and the above values kept track of.
Merging clusters does essentially the same. There should never be any need to remove any hits from
a cluster. Ideally, the vector holding the hits should be made private, but currently this is not
practical, as some of the event reduction/analysis algorithms need to re-sort hits for their
particular needs (compare uTPC to multigrid's ADC-prioritized CoM). For those purposes,
subclassing and casting might be an option.

Lack of encapsulation is still a problem because currently you could still manually repopulate
the Hit container, bypassing plane validation and bound calculation. This breaks the assumptions
of what the class provides in terms of clustering and cluster matching facilities, and so
could produce unintended and hard to debug behavior.

Another potential solution could be to have the Analyzer methods have some pre-allocated `Hit`
containers into which it can copy immutable `Cluster` contents and perform its sorting and
othr processing there.

# Event
This class represents a clustered and matched (but unreduced) event, with the particular aim of
facilitating the matching (pairing) of clusters and later reduction or analysis to determine
the detected particicle position.

An `Event` object has a predefined plane identities:
* plane1
* plane2
It also contains the two clusters in each respective plane:
* cluster1
* cluster2

To facilitate matching of clusters, the class provides methods for:
* inserting individual hits
* merging clusters into event
* querying time bounds of event across both planes
* querying time overlap or gap in comparison to other clusters

### Plane identities
An `Event` should have its intended plane identities specified at construction. Any pipeline
responsible for matching clusters should be aware of which coincidence-prone planes it's working
with. There is a default constructor which assumes the planes are 0 and 1, but in the future it might
make sense to require explicit specification of planes.

`Hits` will be added and `Clusters` merged into the `Event` only if they belong to one of the selected
planes. The `both_planes` function reports true only if borth clusters are non-empty and,
with the above pre-conditions enforced, this also implies that those clusters have valid plane
identities and those plane identities match the pre-defined intended plane identities of the `Event`.

### Time bounds
Time bounds, time gap and time overlap are reported with the same assumptions (and shortcomings)
as those for the `Cluster`, except that the bounds of both constituent clusters (if present)
are considered.

### Encapsulation issues:
Constituent clusters are public. Same reasons and same potential problems as with `Cluster`.