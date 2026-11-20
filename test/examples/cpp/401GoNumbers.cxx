// Examples of numeric literals for issue 401.
// New property lexer.cpp.digit.separator.

/* No digit separators */

// Integer literals
1
75
0113
0x4b

// Floating point literals
3.14159
6.02e23
1.6e-19
1.0

/* C++ digit separator ' */
7'500
3.141'59

/* C++ user literal */
1_date_

/* Go, Java, etc. digit separator _ */
7_500
3.141_59
const x = 100_000
