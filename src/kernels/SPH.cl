float weightFunction(float pDistance, float h)
{
	//poly6
	if (0<= pDistance && pDistance <= h)
		return 315/(64*M_PI*pow(h,9)) * pow((h*h - pDistance*pDistance),3);
	else return 0;
}


__kernel void SPH(__global float4* pos,  __global float4* vel, __global float* density, __global float4* pressure, __global float* viscosity, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float4 v = vel[i];
	float dens = 0.0f;
	float radius = 1.0f;

	for (int count = 0 ; count < get_global_size(0); count++)
	{
		//get_global_id(0);
		//barrier(CLK_GLOBAL_MEM_FENCE)
		dens += 1 * weightFunction(distance(p.xyz,pos[count].xyz),radius);
	}

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




