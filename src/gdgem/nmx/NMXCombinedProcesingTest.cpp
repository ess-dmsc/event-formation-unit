/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <test/TestBase.h>

#include <gdgem/generators/BuilderHits.h>
#include <common/reduction/analysis/EventAnalyzer.h>
#include <common/reduction/matching/CenterMatcher.h>
#include <common/reduction/clustering/GapClusterer.h>

using namespace Gem;

class NMXCombinedProcessingTest : public TestBase
{
protected:

    std::shared_ptr<Gem::BuilderHits> builder_;
    std::shared_ptr<EventAnalyzer> analyzer_;
    std::shared_ptr<CenterMatcher> matcher_;
    std::shared_ptr<GapClusterer> clusterer_x_;
    std::shared_ptr<GapClusterer> clusterer_y_;

    void SetUp() override
    {
        builder_ = std::make_shared<Gem::BuilderHits>();

        analyzer_ = std::make_shared<EventAnalyzer>("center-of-mass");

        matcher_ = std::make_shared<CenterMatcher>(8192 * 5, 0, 1);
        matcher_->set_max_delta_time(200);
        matcher_->set_time_algorithm("utpc");

        clusterer_x_ = std::make_shared<GapClusterer>(200, 2);
        clusterer_y_ = std::make_shared<GapClusterer>(200, 2);
    }
    void TearDown() override {}
};

void _cluster_plane(HitVector &hits,
                    std::shared_ptr<AbstractClusterer> clusterer, std::shared_ptr<CenterMatcher>& matcher)
{
    sort_chronologically(hits);
    clusterer->cluster(hits);
    hits.clear();

    if (!clusterer->clusters.empty())
    {
        matcher->insert(clusterer->clusters.front().plane(), clusterer->clusters);
    }
}

TEST_F(NMXCombinedProcessingTest, Dummy) {
    Hit h{0,0,0,0};
    builder_->process_buffer (reinterpret_cast<char*>(&h), sizeof(h));

    bool flush = false;
    if (builder_->hit_buffer_x.size())
    {
        _cluster_plane(builder_->hit_buffer_x, clusterer_x_, matcher_);
    }

    if (builder_->hit_buffer_y.size())
    {
        _cluster_plane(builder_->hit_buffer_y, clusterer_y_, matcher_);
    }

    // \todo but we cannot parallelize this, this is the critical path
    matcher_->match(flush);

    for (auto &event : matcher_->matched_events)
    {
        if (!event.both_planes())
        {
           
            continue;
        }
        ReducedEvent neutron_event_ = analyzer_->analyze(event);
        ASSERT_TRUE(neutron_event_.good);
    }

    builder_->hit_buffer_x.clear();
    builder_->hit_buffer_y.clear();
}