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

	pressure = pressure_new;
}

__kernel void SPH(__global float4* pos,  __global int* neighbour, __global float* density, __global float* pressure, __global float* viscosity, __global float* mass, __global float* forceIntern, float dt)
{
    unsigned int i = get_global_id(0);

	float viscosityTerm = 1.0f; //mü

	float f_pressure = 0.0f;
	float f_viscosity = 0.0f;

	//force calculation
	for(int index = 0; index < 50; index++)
	{
	//fpressure calculation
	f_pressure += mass[neighbour[i*50+index]] * ((pressure[i] + pressure[neighbour[i*50+index]])/2*density[neighbour[i*50+index]]);//* derivated W
	f_viscosity +=  mass[neighbour[i*50+index]] * ((viscosity[neighbour[i*50+index]] - viscosity[i])/density[neighbour[i*50+index]]); //*second derivated W
	}

	f_pressure *= -1.0f;
	f_viscosity *= viscosityTerm;
//	forceIntern[i] = f_pressure + f_viscosity;
	mass[i] = -0.8;
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float* mass, __global float* forceIntern, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;


	float gravity = -9.81f * mass[i];

	//apply intern forces and extern forces
	v_new.x = v_old.x + forceIntern[i] * dt;
	v_new.y = v_old.y + forceIntern[i] + gravity * dt;
	v_new.z = v_old.z + forceIntern[i] * dt;

	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt;
	
    //update the arrays with newly computed values
    pos[i] = p_new;
    vel[i] = v_new;
}



