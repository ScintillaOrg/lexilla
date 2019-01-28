# Tests for Nim
let s = "foobar"

# Feature #1260
{.ident.}
stdin.readLine.split.map(parseInt).max.`$`.echo(" is the maximum!")
