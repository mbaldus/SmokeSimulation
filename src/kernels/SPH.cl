__kernel void SPH(__global float4* pos,  __global float4* vel, __global float* density, __global float4* pressure, __global float* viscosity, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
    float4 v = vel[i];

	float friction = 1;
	float normal = 1;
	float mass = -0.5;
	float gravity = 9.8;

	v.y -= gravity*dt;
	
	//compute new position with standart velocity
	p.xyz += v.xyz*dt;
	
	//store the updated life in the velocity array

    //update the arrays with our newly computed values
    pos[i] = p;
    vel[i] = v;
}




