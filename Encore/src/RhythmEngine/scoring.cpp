#include "scoring.h"

std::array<Grade, 7> Grades{
    {
        Grade(MAGENTA, "P", { 1.00, 1.00 }),
        Grade(GOLD, "S", { 0.96, 1.00 }),
        Grade(GREEN, "A", { 0.90, 0.96 }),
        Grade(SKYBLUE, "B", { 0.85, 0.90 }),
        Grade(ORANGE, "C", { 0.77, 0.85 }),
        Grade(RED, "D", { 0.60, 0.77 }),
        Grade({ 24, 24, 39, 255 }, "F", { 0.00, 0.60 }),
    }
};

int GetGrade(double acc, Grade** grade = nullptr) {
    int i = 0;
    for (auto &g : Grades) {
        if (acc >= g.range.bottom) {
            if (grade != nullptr) {
                *grade = &g;
            }
            return i;
        }
        i++;
    }
    return 0;
}