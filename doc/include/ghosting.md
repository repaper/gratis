## Ghosting

Below are three test methods to verify that ghosting within an
acceptable range. Test 1 and Test 2 use measured data to calculate Delta
E which is a single number representing the distance between two colors
in a 3 dimensional color space. Test 1, 2, and 3 are performed at 25ºC.

![Test 1: White to Black Ghosting](images/common/ghosting-1.svg)

![Test 2: Black to White Ghosting](images/common/ghosting-2.svg)

The formula is used to calculate Test1 and Test2. For example of Test 2:

△E*ab = [ (L~B~ – L~B’~)^2^ + (a~B~ – a~B’~)^2^ + (b~B~ – b~B’~)^2^
]^1/2^

![Test 3: PCS (for barcode application)](images/common/ghosting-3.svg)


PCS =( (White Reflection Ratio A’ – Black Reflection Ratio B’) / White
Reflection Ratio B’ ) x 100% \
 @ 630nm (wavelength of bar-code reader)


###  Measurement of Ghosting

-----------------------------------------------
Item                     Min.    Typ.     Max.
----------------------  ------  ------  -------
Test 1 △E*ab              -       -        2

Test 2 △E*ab              -       -        2

Test 3 PCS               0.75     -        -
-----------------------------------------------

Note: Panel is driven by PDI waveform without masking film and optical
measurement by CM-700D with D65 light source and SCE mode.