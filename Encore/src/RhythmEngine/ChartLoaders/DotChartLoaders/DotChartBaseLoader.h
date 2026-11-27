#pragma once

#include "DotChartHandler.h"

namespace Encore::RhythmEngine {

    class DotChartBaseLoader {
        ChartLoadContext ctx;

        bool loaded = false;
    protected:
        DotChartHandler* parent;
        std::shared_ptr<BaseChart> chart;
    public:
        virtual ~DotChartBaseLoader() = default;
        DotChartBaseLoader(DotChartHandler* parent, ChartLoadContext ctx, int laneCount) : parent(parent), ctx(ctx), chart(std::make_shared<BaseChart>(laneCount)) {};

        TimeSeconds TickToSeconds(TimeTicks tick);

        const std::vector<std::string>& GetChartSection() {
            for (auto& info : parent->GetValidParts()) {
                if (info.Part == ctx.part) {
                    return parent->sections[DotChartHandler::GetDiffName(ctx.diff) + info.SectionName];
                }
            }
            return {};
        }
        std::shared_ptr<BaseChart> GetChart();
        operator std::shared_ptr<BaseChart>() {return GetChart();};

        /// Called for every event in the chart. Be sure to call the base function for overdrive phrases and solos
        virtual void ReceiveEvent(const DotChartEvent& event);
        /// Called after every event in the chart has been sent. Use to reconcile any accumulated state.
        virtual void EventsDone() = 0;
    };
}