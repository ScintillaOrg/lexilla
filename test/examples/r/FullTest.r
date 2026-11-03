# coding:utf-8
### hexadecimal floating-point numbers styled 5 SCE_R_NUMBER

# point followed by a digit
0x1.3ae147ae147aep+0 + 0x1.23d70a3d70a3dp+2i # 1.23 + 4.56i

# point not followed by a digit
0x1.ccccccccccccdp-1 + 0x1.ccccccccccccdp-1i # 0.9 + 0.9i
0x1.ccccccccccccdp-1

# upper case letters:
0X1.CCCCCCCCCCCCDP-1 # 0.9
0X1.1CCCCCCCCCCCDP-1

# imaginary numbers
0x.1p1i
0x0.1p1i
0xa.1p1i
0xa.a1p1i



### identifiers styled 9 SCE_R_IDENTIFIER

# identifiers can start with a letter or a dot:
.a._x123
..a
._1

# identifiers must not start with an underscore or a digit:
_1test._x
__a__
2values



### dot-dot-dot styling

# styled 2 SCE_R_KWORD because they are reserved words
..1
..2
..123
...
# these are 9 SCE_R_IDENTIFIER because they are just identifiers
._
..
....
...object
...2.objects
..._objectname_.1



### non-ASCII characters

nem_egyenlő <- 5L != 1L # Hungarian
größer      <- 5L > 1L  # German umlaut and eszett
мање        <- 1L < 5L  # Serbian Cyrillic



### real world examples

deg <- seq(0.0, 2 * pi, length.out = 6L)
rad <- deg * pi / 180.0

variable <- 0x5.ABCp-5
new_var <- if(is.null(variable)) NA_integer_ else variable
new_var == variable %||% NA_integer_

new_fun <- function(...){ list(...) }

sqrt(.Machine$double.eps)

abbreviate(month.name) |> cbind()

data.frame(num = seq_len(26L), char = letters)

object_name@slot_name <- TRUE

library(lattice)
histogram(~ height | voice.part, data = singer, xlab = "Height (inches)")
singer |> densityplot( ~ height | voice.part, data = _, xlab = "Height (inches)", bw = 5.0)

is.environment(.GlobalEnv)

head(boot::acme)

tidyverse::tidyverse_packages()
