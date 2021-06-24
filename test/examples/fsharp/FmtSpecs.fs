module FormatSpecifiersTest

let x = List.fold (*) 24.5 [ 1.; 2.; 3. ]

// expect "147.00"
printfn "Speed: %.2f m/s" x

// expect " 147%"
printfn """%% increase:% .0F%% over last year""" x

// expect "1.5E+002"
// NB: units should look like text even without a space
printfn @"Time: %-0.1Esecs" x

// expect "\"         +147\""
printfn @"""Temp: %+12.3g K""" x

// These don't work
printfn ``%.2f`` x
printfn $"%.2f" x
