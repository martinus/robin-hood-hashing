#!/usr/bin/env ruby

def rn(bitness)
    rand(2**bitness) | 1 | (2**(bitness-1))
end
n64 = rn(64)
n32 = rn(32)
printf "0x%16x %s\n", n64, n64.to_s(2)
printf "0x%8x %s\n", n32, n32.to_s(2)
