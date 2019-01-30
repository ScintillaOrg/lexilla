# Tests for Nim
let s = "foobar"

# Feature #1260
{.ident.}
stdin.readLine.split.map(parseInt).max.`$`.echo(" is the maximum!")

# Feature #1261
# IsFuncName("proc") so style ticks as SCE_NIM_FUNCNAME:
proc `$` (x: myDataType): string = ...
# Style ticks as SCE_NIM_BACKTICKS:
if `==`( `+`(3,4),7): echo "True"
