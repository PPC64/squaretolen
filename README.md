# squaretolen
Implementation of OpenJDK's BigInteger.SquareToLen using POWER8 assembly. Used for RSA cryptography algorithm.

At the moment only general purpose registers are being used. SIMD approach (using VSX) is being analysed.

For that purpose, this implementation yields the same performance as plain C code:
![Compare Results](https://raw.githubusercontent.com/PPC64/squaretolen/master/doc/SquareToLen_perf.png) 
