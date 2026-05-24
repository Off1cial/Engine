#ifndef FLAG_TOOL_H
#define FLAG_TOOL_H

#define SET_FLAG_MASK(mask, flag)   ((mask) |= (flag))
#define CLR_FLAG_MASK(mask, flag)   ((mask) &= ~(flag))
#define HAS_FLAG_MASK(mask, flag)   ((mask) & (flag))
#define TOG_FLAG_MASK(mask, flag)   ((mask) ^= (flag))

#endif