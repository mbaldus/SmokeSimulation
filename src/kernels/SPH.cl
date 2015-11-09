float weightFunction(float pDistance, float h)
{
	//poly6
	if (0<= pDistance && pDistance <= h)
		return 315/(64*M_PI*pow(h,9)) * pow((h*h - pDistance*pDistance),3);
	else return 0;
}

__kernel void neighbours(__global float4* pos, __global int* neighbour, float smoothingLength, __global float* mass)
{
	unsigned int i = get_global_id(0);
	float4 p = pos[i];
	
	int counter = 0;
	//save neighbours of THIS particle in an array 
	//array size is 50(n) times bigger than pos[]
	for (int index = 0; index < get_global_size(0); index++)
	{
		if (distance(p.xyz, pos[index].xyz) < 1) // < smoothingLength
		{
			neighbour[i*50+counter] = index;
			counter++;

			//debugging commands
			mass[i] = -1;
		}
		//stop when 50(n) neighbours of i are found
		if (counter >= 49)
			break;
	}
}

__kernel void densityCalc(__global float4* pos, __global int* neighbour, __global float* density, __global float* pressure, __global float* mass, float smoothingLength)
{
	unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float rho = 0;
	float pressure_new = 0;
	
	for(int index = 0; index < 50; index++)
	{
		//rho += mass[i*50+index] * weightFunction(distance(p.xyz, neighbour[i*50+index].xyz), smoothingLength); 
		rho += mass[neighbour[i*50+index]] * weightFunction(distance(p.xyz, pos[neighbour[i*50+index]].xyz), smoothingLength); 

	}

	density[i] = rho;

	pressure_new = rho * 1; //p = rho * k (k = stoffspezifische Konstante (Wasser 999kg/m³)) 
	//pressure_new = 1 * (pow((rho/1),7) - 1); //p  = k * (pow((rho[i]/rho0),7) - 1); 
}

__kernel void SPH(__global float4* pos,  __global float4* vel, __global float* density, __global float4* pressure, __global float* viscosity, __global float* mass, float dt)
{
    unsigned int i = get_global_id(0);

	//float4 p = pos[i];
	//float4 v = vel[i];
	float dens = 0.0f;
	float radius = 1.0f;

	//float friction = 1;
	//float normal = 1;
	//float masse = -0.5;
	//float gravity = 9.8;

	//v.y -= gravity*dt;
	//
	////compute new position with standart velocity
	//p.xyz += v.xyz*dt;
	
	//store the updated life in the velocity array

    //update the arrays with our newly computed values
   /* pos[i] = p;
    vel[i] = v;*/

	mass[i] = -0.2;
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float* density, __global float4* pressure, __global float* viscosity, __global float* mass, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;


	float gravity = -9.81f * mass[i];

	//apply gravity
	v_new.y = v_old.y + gravity * dt;

	//apply intern forces
	//v_new.xyz = v_old.xyz + fpressure + fviscosity;

	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt;
	
    //update the arrays with newly computed values
    pos[i] = p_new;
    vel[i] = v_new;
}



