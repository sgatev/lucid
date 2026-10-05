#pragma once

#define LUCID_STRINGIFY_IMPL(x) #x

// Returns `x`, expanded, as a string literal.
#define LUCID_STRINGIFY(x) LUCID_STRINGIFY_IMPL(x)

#define LUCID_CONCAT_IMPL(prefix, suffix) prefix##suffix

// Returns `prefix` and `suffix`, expanded, joined into one token.
#define LUCID_CONCAT(prefix, suffix) LUCID_CONCAT_IMPL(prefix, suffix)

// Returns an identifier starting with `prefix` that is unique within a line.
#define LUCID_UNIQUE_VAR(prefix) LUCID_CONCAT(prefix##_, __LINE__)
