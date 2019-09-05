# AbstractClusterer

Interface for clustering strategies. Takes hits or vectors of hits and joins them into clusters
according to the specific implementation.

### Assumptions:
* Each insertion of `Hit` may trigger some clustering activity. There may be some buffering
done by the implementation, but it is assumed that some processing might also occur at this point.
The assumption is that the calling pipeline already has some mechanism for queueing up relevant
hits after parsing and digital geometry mapping, and so it can at this point afford to do some
non-trivial processing.
* A function is provided for flushing the data. Again, it is assumed that there may be some
buffering done by the implementation, and the client might occasionally force a flush of this
buffer.
* The `stash_cluster` method is provided to not only release a cluster to the client,
but also increment the stats counter. It is important that any implementation use this
function rather than directly emplace newly minted clusters onto the queue.

### Encapsulation
* Stats variable could be encapsulated and getter provided
* container of clusters is left public. This container is just an "out" bin for the
clusterer, and should never be reexamined by the clusterer, so anything the client does
with it should have no effect on the clusterer. This is hard to guarantee, however.
An option might be to not even have it as a member variable at all, but instead have the caller
provide a reference to an external dumping ground. But this also does not ensure the desired
invariant.

### Performance
The alias provided for ClusterContainer uses `std::list`, which is claimed by some to be
very poorly performant. Very likely it could be replaced by something like deque. However,
deque cannot be sorted, which is later needed for cluster matching. If this is shown to be a
bottleneck, a valid solution might be to copy or move clusters into another type of container
for matching.

# GapClusterer
Currently the only clusterer implementation available. Groups hits based on their separation
in time and in space.

### Assumptions
* Events, whether inserted individually or in bulk, must come in chronological order. The calling
pipeline can ensure this by observing maximum latency guarantees and only feeding `Hits`
into the clusterer for which we can be sure to have all antecedent events accounted for. If this
guarantee is not upheld, then the following behavior cannot be relied on.
* Hits are accumulated into a tentative cluster until such a cluster is finally processed
and released onto the "out" queue.
* Hit insertion can trigger clustering. Since the chronological guarantee is assumed, any hit
that has a sufficient time-gap from the previous hit will cause the tentative cluster to be
processsed. This is equivalent to completing the "clustering in time" step and immediately
proceeding to the "clustering in space" step for that tentative cluster.
* Time gap is calculated in a similar way as it is for a `Cluster` (see class description),
and no negative time gaps are possible owing to the chronological guarantee above.
* *pathological case:* if a sufficiently large time gap is never encountered, spatial clustering
and subsequent release of clusters to the client may never happen. In such a case, the clusterer
may accumulate Hits forever. The class (or possibly even the interface class) may have to
include some "maximum time-span" criterion for contingency flushing. My suggestion is to
also include some additional flag(s) in `Cluster` class that would identify clusters that
have been released due to this alternative condition. In particular, it could indicate
whether the start-time or end-time of a cluster were artificially truncated, not due to an
expected time-gap, but as a cancer-preventing measure. Such truncation has repercussions
for event analysis, such as uTPC, where time of specific Hits has bearing on reduced event
position.
* Clustering by coordinate assumes that we are dealing with a contiguous coordinate space.
This totally does not work for something like wires. The contingency for now is to provide
a very lenient maximum coordinate gap criterion in the case of wires, essentially ignoring
the spatial dimension for the purposes of clustering. I am not sure what solution is best. Wires,
if counted in a sensible way, woulc be represented by a subest 2-dimensional logical geometry
definition, which could be provided to the clusterer. You'd need to sort and cluster in
dimension 1, and then sort and cluster in dimension 2. So this already sounds like it's worthy of yet
another specialized class.

