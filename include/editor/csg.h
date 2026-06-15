#ifndef CSG_H
#define CSG_H

#include "editor/brush.h"

// Splits a brush by a plane and returns two more, leaving the original unchanged
void CSG_SplitBrush(brush_t* in, const plane_t* plane, brush_t** front, brush_t** back);

int CSG_SubtractBrush(const brush_t* in, const brush_t* subtract, brush_t** outpieces);


#endif