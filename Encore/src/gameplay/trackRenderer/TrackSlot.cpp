
#include "TrackSlot.h"

#include "rlgl.h"

namespace Encore {
    void DrawSegment(Segment segment, Color color, float frontAlpha, float backAlpha) {
        float fm = (segment.front.l.x + segment.front.r.x) / 2;
        float rm = (segment.back.l.x + segment.back.r.x) / 2;

        Color bright = ColorBrightness(color, 0.2);
        // left side
        rlColor4ub(0, 0, 0, 0);
        rlVertex3f(segment.front.l.x, 0.03, segment.front.l.z);
        rlVertex3f(segment.back.l.x, 0.03, segment.back.l.z);
        rlColor4f(
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f * backAlpha
        );
        rlVertex3f(rm, 0.03, segment.back.l.z);
        rlColor4f(
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f * frontAlpha
        );
        rlVertex3f(fm, 0.03, segment.front.l.z);

        rlVertex3f(fm, 0.03, segment.front.r.z);
        rlColor4f(
            color.r / 255.0f,
            color.g / 255.0f,
            color.b / 255.0f,
            color.a / 255.0f * backAlpha
        );
        rlVertex3f(rm, 0.03, segment.back.r.z);
        rlColor4ub(0, 0, 0, 0);
        rlVertex3f(segment.back.r.x, 0.03, segment.back.r.z);
        rlVertex3f(segment.front.r.x, 0.03, segment.front.r.z);
    }
}