namespace Literals

module Issue110 =
    let hexA = +0xA1B2C3D4
    let hexB = -0xCC100000

    // regression checks
    let hexC = 0xCC100000
    let binA = +0b0000_1010
    let binB = -0b1010_0000
    let binC = 0b1010_0000
    let octA = +0o1237777700
    let octB = -0o1237777700
    let octC = 0o1237777700
    let i8a = +0001y
    let i8b = -0001y
    let u8 = 0001uy
    let f32a = +0.001e-003
    let f32b = -0.001E+003
    let f32c = 0.001e-003
    let f128a = +0.001m
    let f128b = -0.001m
    let f128c = 0.001m

    // invalid literals
    let hexD = 0xa0bcde0o
    let hexE = +0xa0bcd0o
    let hexF = -0xa0bcd0o
    let binD = 0b1010_1110xf000
    let binE = +0b1010_1110xf000
    let binF = -0b1010_1110xf000
    let binG = 0b1010_1110o
    let binH = +0b1010_1110o
    let binI = -0b1010_1110o
    let octD = 0o3330xaBcDeF
    let octE = +0o3330xaBcDe
    let octF = -0o3330xaBcDe
    let octG = 0o3330b
    let octH = 0o3330b
    let octI = 0o3330b
