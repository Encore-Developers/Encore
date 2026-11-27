#pragma once
#include "RhythmEngine/ChartLoaders/DotChartLoaders/DotChartBaseLoader.h"

namespace Encore {
    namespace RhythmEngine {

        class DotChartGuitarLoader : public DotChartBaseLoader {
            NoteEvent accumulatedNote {0, -1, 0, 0, 0, 0};
            NoteEvent previousNote {0, -1, 0, 0, 0, 0};

            bool hasPreviousNote = false;
            bool hasAccumulatedNote = false;
        public:
            DotChartGuitarLoader(DotChartHandler *parent, const ChartLoadContext &ctx)
                : DotChartBaseLoader(parent, ctx, 1) {}

            int NoteTypeToLane(int type);
            bool InHopoThresh(const NoteEvent &note);

            void StartNewNote(const DotChartEvent& event);
            void FinishNote();

            void ReceiveEvent(const DotChartEvent& event) override;
            void EventsDone() override;
        };

    } // RhythmEngine
} // Encore
