//
// Created by maria on 26/08/2025.
//

#ifndef ENCORE_OVERDRIVE_H
#define ENCORE_OVERDRIVE_H
#include "RhythmEngine/NoteVector.h"

#include <memory>

namespace Encore::RhythmEngine {
    class Overdrive {
    public:
        double Fill = 0.0;
        double ActivationTime = 0.0;
        double ActivationTick = 0.0;
        bool Active = false;
        bool UseOverdriveLift = false;
        void Update(double &CurrentTime);
        bool Activate(const double &CurrentTime);
        bool Add(const double &CurrentTime, std::shared_ptr<BaseChart> & chart);
    };
}
#endif // ENCORE_OVERDRIVE_H
