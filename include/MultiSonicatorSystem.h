/**
 * @file MultiSonicatorSystem.h
 * @brief DEPRECATED shim. Use include/multiplexer/master_state.h instead.
 *
 * This header previously declared a C-style multi-unit control API and redefined
 * master_state_t. To enforce a single source of truth for master state and to
 * align with the new Multiplexer architecture, this file now only includes the
 * canonical master state enum.
 *
 * Migration:
 * - Replace includes of this file with: #include "multiplexer/master_state.h"
 * - Use the C++ Multiplexer class (see include/multiplexer/Multiplexer.h) for
 *   coordinated multi-sonicator control and per-unit operations.
 */

#ifndef MULTI_SONICATOR_H
#define MULTI_SONICATOR_H

#pragma message("Deprecated: include/MultiSonicatorSystem.h is a shim. Use include/multiplexer/master_state.h")

#include "multiplexer/master_state.h"

/* Legacy interface note:
 * The old C functions (multi_sonicator_begin, multi_sonicator_update, etc.)
 * have been retired in favor of the Multiplexer class API. If any legacy
 * references are discovered, refactor them to use the Multiplexer instance
 * created in src/main.cpp.
 */

#endif // MULTI_SONICATOR_H
