#ifndef DEFAULTS_HPP_B71D6BE9_ED49_4477_8DCA_76670C2F0398
#define DEFAULTS_HPP_B71D6BE9_ED49_4477_8DCA_76670C2F0398

#include <chrono>

/* Select default clock
 *
 * The steady clock is a reasonable choice. It is monotonic, meaning that time
 * will never seem to go backwards (this can happen, e.g., during daylight
 * savings switches, or due to adjustments from something like NTP).
 *
 * std::chrono::high_resolution_clock is a reasonable alternative.
 */
using Clock = std::chrono::steady_clock;

/* Alias: time duration in seconds, as a float.
 *
 * Don't use for long durations (e.g., time since application start or
 * similar), as float accuracy decreases quickly.
 */
using Secondsf = std::chrono::duration<float, std::ratio<1>>;

#endif // DEFAULTS_HPP_B71D6BE9_ED49_4477_8DCA_76670C2F0398
