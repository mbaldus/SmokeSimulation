float function_example(float a, float b)
{
    return (float)((a * a) + (b * b)) / 1000;
}

__kernel void part1(__global float* a, __global float* b, __global float* c)
{
    unsigned int i = get_global_id(0);

    c[i] = function_example(a[i], b[i]);
}
