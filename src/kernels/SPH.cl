float weightFunction(float pDistance, float h, float poly6const)
{
	//poly6
	if (0<= pDistance && pDistance <= h)
		return poly6const * pow((h*h - pDistance*pDistance),3);
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
		if (distance(p.xyz, pos[index].xyz) <= smoothingLength) // < smoothingLength
		{
			neighbour[i*50+counter] = index;
			counter++;

			//debugging commands
			//mass[i] = -1;
		}
		//stop when 50(n) neighbours of i are found
		if (counter >= 49)
			break;
	}
	//printf("neighbour[%d] = %i:\n", 77, neighbour[77]);
}

__kernel void densityCalc(__global float4* pos, __global int* neighbour, __global float* density, __global float* pressure, __global float* mass, float smoothingLength, float poly6)
{
	unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float rho = 0;
	float pressure_new = 0;
	
	for(int index = 0; index < 50; index++)
	{
		//rho += mass[i*50+index] * weightFunction(distance(p.xyz, neighbour[i*50+index].xyz), smoothingLength); 
		rho += mass[neighbour[i*50+index]] * weightFunction(distance(p.xyz, pos[neighbour[i*50+index]].xyz), smoothingLength, poly6); 
	//	rho += mass[neighbour[i*50+index]] * weightFunction(p.xyz - pos[neighbour[i*50+index]].xyz, smoothingLength, poly6); 
	}

	density[i] = rho;

	pressure_new = rho * 1; //p = rho * k (k = stoffspezifische Konstante (Wasser 999kg/m³)) 
	//pressure_new = 1 * (pow((rho/1),7) - 1); //p  = k * (pow((rho[i]/rho0),7) - 1); 
	//if (density[i] > 0)
	//{
	//printf("density[%d] %f:, pos[%d] %f,%f,%f \n", i, density[i],i,pos[i].x,pos[i].y,pos[i].z);
	//}
	pressure[i] = pressure_new;
	//printf("pressure[%d] = %f:\n", i, pressure[i]);
}

__kernel void SPH(__global float4* pos,__global float4* vel,  __global int* neighbour, __global float* density, __global float* pressure, __global float* viscosity, __global float* mass, __global float4* forceIntern, float smoothingLength, float poly6)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float viscosityConst = 1.0f; //mü

	float4 f_pressure = 0.0f;
	float4 f_viscosity = 0.0f;

	//printf("velodistance[%d] = %f \n", i, distance(vel[neighbour[i*50]], vel[i]));
	//force calculation
	for(int index = 0; index < 50; index++)
	{
	//fpressure calculation
	f_pressure.x += mass[neighbour[i*50+index]] * ((pressure[i] + pressure[neighbour[i*50+index]])/2*density[neighbour[i*50+index]]) 
				 * weightFunction(p.x - pos[neighbour[i*50+index]].x, smoothingLength, poly6);
	f_pressure.y += mass[neighbour[i*50+index]] * ((pressure[i] + pressure[neighbour[i*50+index]])/2*density[neighbour[i*50+index]])
				 * weightFunction(p.y - pos[neighbour[i*50+index]].y, smoothingLength, poly6);
	f_pressure.z += mass[neighbour[i*50+index]] * ((pressure[i] + pressure[neighbour[i*50+index]])/2*density[neighbour[i*50+index]])
				 * weightFunction(p.z - pos[neighbour[i*50+index]].z, smoothingLength, poly6);

	f_viscosity.x +=  mass[neighbour[i*50+index]] * (distance(vel[neighbour[i*50+index]], vel[i])/density[neighbour[i*50+index]])
				* weightFunction(p.x - pos[neighbour[i*50+index]].x, smoothingLength, poly6)
				* weightFunction(p.x - pos[neighbour[i*50+index]].x, smoothingLength, poly6); 
	f_viscosity.y +=  mass[neighbour[i*50+index]] * (distance(vel[neighbour[i*50+index]], vel[i])/density[neighbour[i*50+index]])
				* weightFunction(p.y - pos[neighbour[i*50+index]].y, smoothingLength, poly6)
				* weightFunction(p.y - pos[neighbour[i*50+index]].y, smoothingLength, poly6); 
	f_viscosity.z +=  mass[neighbour[i*50+index]] * (distance(vel[neighbour[i*50+index]], vel[i])/density[neighbour[i*50+index]])
				* weightFunction(p.z - pos[neighbour[i*50+index]].z, smoothingLength, poly6)
				* weightFunction(p.z - pos[neighbour[i*50+index]].z, smoothingLength, poly6); 
	}

	f_pressure.xyz *= -1.0f;
	//printf("fpressure[%d] : x=%f,y=%f,z=%f \n", 30, f_pressure.x, f_pressure.y, f_pressure.z) ;
	f_viscosity.xyz *= viscosityConst;
	//printf("fviscosity[%d] : x=%f,y=%f,z=%f \n", i, f_viscosity.x, f_viscosity.y, f_viscosity.z) ;
	//forceIntern[i].xyz = f_pressure.xyz + f_viscosity.xyz;

//	printf("forceIntern[%d] : x=%f,y=%f,z=%f \n", i, forceIntern[i].x*0.003, forceIntern[i].y*0.003, forceIntern[i].z*0.003) ;
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float* density, __global float4* forceIntern, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;


	float gravity = -9.81f * density[i] *dt;

	//apply intern forces and extern forces
	v_new.x = v_old.x + forceIntern[i].x * dt;
	v_new.y = v_old.y + forceIntern[i].y + gravity * dt;
	v_new.z = v_old.z + forceIntern[i].z * dt;

	if(p_old.y < -0.35){
		v_new.y *= -1;
	}
	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt ;
	
    //update the arrays with newly computed values
    pos[i].xyz = p_new.xyz;
    vel[i].xyz = v_new.xyz;
}



