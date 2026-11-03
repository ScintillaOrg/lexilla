# https://cran.r-project.org/doc/manuals/r-devel/R-lang.html#Constants
"x\'x" == "x\x27x"
"x\"x" == "x\x22x"
"x\nx" == "x\x0ax"
"x\rx" == "x\x0dx"
"x\tx" == "x\x09x"
"x\bx" == "x\x08x"
"x\ax" == "x\x07x"
"x\fx" == "x\x0cx"
"x\vx" == "x\x0bx"
"x\\x" == "x\x5cx"

# octal: x<U+0001>x
"x\1x"   == "x\x01x"
"x\01x"  == "x\x01x"
"x\001x" == "x\x01x"

# \x: x<U+000f>x
"x\xfx"  == "x\017x"
"x\x0fx" == "x\017x"

# \u
# x<U+000a>x
"x\uax"    == "x\u{a}x"
"x\u0ax"   == "x\u{a}x"
"x\u00ax"  == "x\u{a}x"
"x\u000ax" == "x\u{a}x"

"x\uax"    == "x\u{a}x"      # x<U+000a>x
"x\uaax"   == "x\u{aa}x"     # x<U+00aa>x
"x\uaaax"  == "x\u{aaa}x"    # x<U+0aaa>x
"x\uaaaax" == "x\u{aaaa}x"   # x<U+aaaa>x

# \U
# x<U+000a>x
"x\Uax"        == "x\U{a}x"
"x\U0ax"       == "x\U{a}x"
"x\U00ax"      == "x\U{a}x"
"x\U000ax"     == "x\U{a}x"
"x\U0000ax"    == "x\U{a}x"
"x\U00000ax"   == "x\U{a}x"
"x\U000000ax"  == "x\U{a}x"
"x\U0000000ax" == "x\U{a}x"

"x\Uax"        == "x\U{a}x"          # x<U+000A>x
"x\Uaax"       == "x\U{aa}x"         # x<U+00AA>x
"x\Uaaax"      == "x\U{aaa}x"        # x<U+0AAA>x
"x\Uaaaax"     == "x\U{aaaa}x"       # x<U+AAAA>x
"x\Uaaaaax"    == "x\U{aaaaa}x"      # x<U+AAAAA>x
"x\U0aaaaax"   == "x\U{0aaaaa}x"     # x<U+AAAAA>x
"x\U00aaaaax"  == "x\U{00aaaaa}x"    # x<U+AAAAA>x
"x\U000aaaaax" == "x\U{000aaaaa}x"   # x<U+AAAAA>x
"x\U0010FFFFx" == "x\U{0010FFFF}x"   # x<U+10FFFF>x
