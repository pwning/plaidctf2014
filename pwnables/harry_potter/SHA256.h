#ifndef SHA256_H

#define SHA256_H

// The header file defines a method to compute the SHA256 hash of a block of input data.
//
// This in 'snippet' form and has no external dependencies other than on 'stdint.h' which is available on most compilers.
//
// https://en.wikipedia.org/wiki/SHA-2
//
// The actual implementation of the method is a copy of the code written by Zilong Tan (eric.zltan@gmail.com) and released under MIT license
//

#include <string>
#include <stdint.h>	// Include stdint.h; available on most compilers but, if not, a copy is provided here for Microsoft Visual Studio

void computeSHA256(std::string& input,		// A pointer to the input data to have the SHA256 hash computed for it.
				   uint8_t destHash[32]);	// The output 256 bit (32 byte) hash

#endif
