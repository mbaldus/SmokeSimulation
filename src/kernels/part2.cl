__kernel void part2(__global float4* pos,  __global float4* vel, __global float4* pos_gen, __global float4* vel_gen, float dt, int reverse)
{
    //get our index in the array
    unsigned int i = get_global_id(0);
    //copy position and velocity for this iteration to a local variable
    //note: if we were doing many more calculations we would want to have opencl
    //copy to a local memory array to speed up memory access
    float4 p = pos[i];
    float4 v = vel[i];

	float friction = 0.85;
	float normal = 1;

    //we've stored the life in the fourth component of our velocity array
    float life = vel[i].w;
    //decrease the life by the time step (this value could be adjusted to lengthen or shorten particle life
    life -= 0.1*dt;
    //if the life is 0 or less we reset the particle's values back to the original values and set life to 1
    if(life <= 0)
    {
        p = pos_gen[i];
        v = vel_gen[i];
        life = 1.0;    
    }	
	/*
	v.y += 9.8*dt;
	if(p.y > 1)
	{
		v.y *= -1;
		v.xyz *= friction;
		p.xyz += v.xyz*dt;
	}else
		p.xyz += v.xyz*dt;*/
	float3 reflection = v.xyz;
	v.y += 9.8*dt;
	if( ((-0.5 < p.x && p.x < 0.5) && (-0.5 < p.y && p.y < 0.5) && (-0.5 < p.z && p.z < 0.5)))
	{
		reflection = 2 * p.xyz * ((p.x * v.x) + (p.y * v.y) + (p.z * v.z)) - v.xyz; 
		v.xyz = -1 * reflection;	
		p.xyz += v.xyz *dt * friction;
	//v.xyz *= -1 * friction;
	}else
	{
	p.xyz += v.xyz*dt;
	}
	//store the updated life in the velocity array
    v.w = life;

    //update the arrays with our newly computed values
    pos[i] = p;
    vel[i] = v;
}

