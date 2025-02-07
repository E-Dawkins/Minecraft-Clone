#include "DebugClock.h"

std::vector< std::pair<const char*, t_point>> DebugClock::time_points = {};
bool DebugClock::enabled = false;