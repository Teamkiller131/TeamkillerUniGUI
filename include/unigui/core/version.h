#pragma once

#define UNIGUI_VERSION_MAJOR 3
#define UNIGUI_VERSION_MINOR 8
#define UNIGUI_VERSION_PATCH 2

#define UNIGUI_VERSION_STRING "3.8.2"

// Single integer encoding the version as major*10000 + minor*100 + patch, so
// downstream code can compare versions numerically:
//   #if UNIGUI_VERSION_NUMBER >= UNIGUI_MAKE_VERSION(3, 5, 0)
#define UNIGUI_MAKE_VERSION(major, minor, patch) ((major) * 10000 + (minor) * 100 + (patch))

#define UNIGUI_VERSION_NUMBER                                                                      \
    UNIGUI_MAKE_VERSION(UNIGUI_VERSION_MAJOR, UNIGUI_VERSION_MINOR, UNIGUI_VERSION_PATCH)

// Convenience predicate for feature-detection guards in downstream code:
//   #if UNIGUI_VERSION_AT_LEAST(3, 5, 0)
#define UNIGUI_VERSION_AT_LEAST(major, minor, patch)                                               \
    (UNIGUI_VERSION_NUMBER >= UNIGUI_MAKE_VERSION(major, minor, patch))
