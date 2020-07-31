
uint16 htons(uint16 a){
    // \todo on BIG ENDIAN this is just a fallthrough.
    return (a >> 8) | (a << 8);
}

// \todo
// - do computation in uint16 only? (scale up before mul and scale down after mul)
// - align input and output data to target SIMD memory alignment
export void SampleGenIspc(uniform uint16 output[], uniform int count, uniform float PeakBuffer[], 
    uniform float Amplitude, uniform float BkgSlope, uniform float BkgOffset)
{
    foreach (i = 0 ... count){
        output[i] = htons (PeakBuffer[i] * Amplitude + BkgSlope * i + BkgOffset);
    }
}