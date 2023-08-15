#pragma once

#include "gmock/gmock.h"

namespace lucid {

// Matches the return code of a command.
MATCHER_P(ReturnsCode, matcher, "") {
  return ExplainMatchResult(matcher, arg.return_code, result_listener);
}

// Matches the string printed on stdout by a command.
MATCHER_P(Prints, matcher, "") {
  return ExplainMatchResult(matcher, arg.out, result_listener);
}

// Matches the string printed on stderr by a command.
MATCHER_P(PrintsError, matcher, "") {
  return ExplainMatchResult(matcher, arg.err, result_listener);
}

}  // namespace lucid
