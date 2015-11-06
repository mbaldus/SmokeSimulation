__kernel void integration(__global float4* pos,  __global float4* vel, __global float* density, __global float4* pressure, __global float* viscosity, __global float* mass, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float4 v = vel[i];
	
	float gravity = -9.81 * mass;

	//apply gravity
	v.y += gravity;

	//apply intern forces
	//v.xyz += fpressure + fviscosity;

	//compute new position with computed velocity
	p.xyz += v.xyz*dt;
	
    //update the arrays with newly computed values
    pos[i] = p;
    vel[i] = v;
}




