# AbstractMatcher

As defined in our clustering primitives, `Clusters` belong to one plane
only. Events are clusters matched across two planes. AbstractMaccher defines the interface for
cluster matching strategies. It also provides some common parts of the implementation for
various matching strategies.

### Latency assumptions
Since hits and clusters for different planes may arrive from different readout units
(expectation for Gadolinium GEM at the very least), it is important to ensure that all relevant
data is considered before the total event is reassembled. This class helps ensure this
by enforcing the maximum latency requirement across the planes. The expectation for
the client pipeline is that the clusters it supplies are chronologically consecutive for each plane.
If something like `GapClusterer` was used to produce the clusters, then this is already taken care of
by meeting the assumptions for that class.

The `ready_to_be_matched` method provides the necessary logic for various matcher implementations.
Recall that each cluster has a plane identity, which tells us that all hits in the cluster belong
to that particular plane. In order to not discard relevant data, clusters may only be matched and
released if we are sure that no more data will arrive that is potentially coincident with some
cluster being considered. And since we are considering multiple planes from which data may arrive
at different times, we must guarantee that each cluster considered for matching satisfies the
maximum latency cirterion for all relevant planes in question, not just the plane that the said
cluster belongs to.

Let us compare the time endpoint of the cluster versus the latest timepoint so far encountered on
that plane. We say that the cluster is safe to process if this difference is greater than the
maximum latency guaranteed by the readout system. We consider the time endpoint rather than the
start point, because the time difference with the end point is likely smaller, so this is
a more conservative criterion. We have now check for the same condition against the other plane,
the one this cluster does not belong to.

Consider a failed comparison for one plane. If the difference between cluster end-time and the
latest time-point observed on a plane is less than the maximum latency, then it means we have
not seen events far enough in time on that particular plane to be sure that there is nothing in
coincidence with our cluster, and therefore we cannot release it.

The way `ready_to_be_matched` approaches this question is by selecting the earliest (least advanced
in time) point from the relevand planes, and then uses this as the comparison point for the
latency check. If pulse times are being considered, then it also includes the latest pulse time
in this selection, i.e. selects the earliest of the three "planes" (in the broad sense) in question.

### Plane identities
Recall that the `Event` class can merge any `Clusters` so long as they belong to one of the relevant
planes. This simplifies matcher implementation, particularly when at this stage we are mostly
interested in merging all clusters that are in some ways time-coincident, for the moment ignoring
the conmplexities of disambiguating multiple time-coincident clusters.

In a similar way, feeding clusters to a Matcher can be simplified if the matcher is already aware
of the relevant planes. Unmatched clusters are kept in a single chronological queue and when
considering each cluster for matching, we only care about the maximum latency condition versus
the latest time point in each plane. When receiving a new cluster (or container of clusters),
the matcher can indentify each cluster's plane and update the appropriate "time horizon".

For this reason, it is reasonable that a `Clusterer` must be initialized with the following
parameters already known:
* maximum latency of readout system
* plane ID for the first relevant plane
* plane ID for the second relevant plane
* plane ID for the (optional) pulse events

Since clustering on a single plane is expected to have happened in one of the `Clusterer`
implementations, it is also very likely that clusters from a single plane will arrive in bulk
and already chronologically sorted.
For this reason a convenience (and performance-advantageous) function is provided which
allows for this assumption. If you indicate which plane a container of events belongs to, only
one time comparison will be needed to establish the time horizon and the entire container
can be spliced into the queue.

Otherwise, clusters can be inserted individually, or in bulk with no assumption about their
planes or origin, allowing the clusterer to evaluate them individually.

When establishing the time horizon for each plane, the `time_start` of the cluster is used,
i.e. the earliest point of the cluster, again to be on the conservative side for the
latency comparison.

### Caveats:
The pathological case of "endless clusters" described in the `Clusterer` documenation could have
unintended consequences for the matcher implementations. The matching pipeline might be blocked
from proceeding until the relevant clusterer releases its data.

### Shortcomings:
The presence of pulse-time complicates things, and the need to keep track of it in the matcher makes
implementations more complex than they would otherwise have to be. It is highly likely
that this class is not the right place to keep track of the pulse time. Particle events and pulse
events have to at some point be queued up on the same track, because of the expectations in the ev42
buffer definition. All particle events have to come after and in relation to the most recent
pulse event. If there is ever an external mechanism that can ensure this (an additional queue),
then it would be very welcome to remove this complexity from the Matcher class.

# OverlapMatcher
The simplest and most strict of the matcher implementations. Requires that clusters overlap in
time for them to be merged into a single Event. There are no threshold-like criteria. Only
the `match` function needs to be implemented.

The first thing that happens in the implementation is that the unmatched clusters are sorted in time.
This is because multiple batches of clusters may have been added from different planes prior to
this matching step, only noting the changing time horizons for each plane. To begin releasing the
clusters, we need to sort them only once prior to performing the actual matching.

The matching loop will continue so long as there are unmatched clusters that satisfy the
`ready_to_be_matched` criterion evaluated by the parent class method.

Any event is considered ready to release, even if it only has a cluster in one dimension. It must
not be empty, and it must not overlap (as defined in the `Event` class) with a subsequent cluster.
Whatever has so far been accumulated into a candidate event will be released onto the "out" queue.

Whether the candidate event has anything or not, the current cluster satisfying the
`ready_to_be_matched` criterion will be merged into the candidate event. By the above logic,
if there was an actual overlap, the candidate will not have ben released. So, true merging
of multiple clusters only happens here in case of time overlap.

At the end of the loop, it may be that there was something accumulated in the candidate event, but
it has not met the criteria for release. In which case it is disassembeled and it's constituent
clusters are put back on the front of the queue of unmatched clusters.

Some additional logic ensures that requests to flush are satisfied and that pulse-type events
are also appropriately released without merging them with multi-dimensional clusters.

# GapMatcher
This implementation merges clusters without requiring a strict time overlap. So long as two
clusters are within a certain small time-gap of each other, they are considered to be
merge-worthy.

A `minimum_time_gap` parameter is provided that defines the threshold for cluster adjacency.

The implementation is almost a carbon copy of the one for `OverlapMatcher`, only that instead
of the time-overlap condition for candidate event relase it is instead the event vs. cluster
time-gap being outside the requested threshold.

#  EndMatcher

This implementation has a similar criterion as `GapMatcher` above, except the time-difference is
not the gap between clusters, but difference in between their end-times. This implementation
is a specific adaptation for the Gadolinium GEM pipeline, where cluster end-points are
of particular interest.

Implementation wise it is a similar story to the above. Some convenience functions have been
provided to encapsule the necessary time-end comparisons.

### Shortcomings
This implementation does not make special arrangements for pulse events. Currently GdGem
does not have or require pulse time in the stream, so it is probably ok. But also in the long term
this should be solved elsewhere (see `AbstractMatcher` above).