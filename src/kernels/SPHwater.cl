float pVarPoly(float h, float r)
{
	if (0<= r && r <= h)
		return pow(h*h - r*r,3);
	else return 0;
}

float4 pVarSpiky(float h, float4 p, float4 pn)
{
	float4 r = p - pn;
	float r_length = length(r);
	if (0< r_length && r_length <= h)
		return  r/r_length * pow(h - r_length,2);
	else return 0;
}

float pVarVisc(float h, float4 p, float4 pn)
{
	float r_length = length(p-pn);
	if (0<= r_length && r_length <= h)
		return h - r_length;
	else return 0;
}


__kernel void neighbours(__global float4* pos, __global int* neighbour,__global int* counter, float smoothingLength, __global float* mass)
{
	unsigned int i = get_global_id(0);
	float4 p = pos[i];
	
	counter[i] = 0;
	//save neighbours of THIS particle in an array 
	//array size is 1000(n) times bigger than pos[]
	for (int index = 0; index < get_global_size(0); index++)
	{
		if (distance(p.xyz, pos[index].xyz) <= smoothingLength) // < smoothingLength
		{
			neighbour[i*50+counter[i]] = index;
			counter[i]++;
			//only saves values with distance < smoothing Lenght --> [0,smoothingLength]
		}
		//stop when 50(n) neighbours of i are found
		if (counter[i] >= 49)
			break;
	}
	//printf("neighbour[%d] = %i:\n", 77, neighbour[77]);
}

__kernel void densityCalc(__global float4* pos, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure, 
						  __global float* mass, float smoothingLength, float poly6, float rho0)
{
	unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float rho = 0;
	float pressure_new = 0;
	float k = 0.75; //Gaskonstante 

	for(int index = 0; index < counter[i]; index++)
	{
		int j = neighbour[i * 50 + index];
		rho += pVarPoly(smoothingLength, distance(p.xyz ,pos[j].xyz));
	}
	rho *= poly6 * mass[i];
    //printf("distance p-pos[] -> %f:\n", distance(p, pos[10]));

	density[i] = rho;
	/*if(density[i] == 0)
		printf("RHO von [%d] == 0 \n" , i);*/
	//printf("density[%d] = %f \n", i , density[i]);
	pressure_new = k * (rho - rho0); //p = k * (rho-rho0)(k = stoffspezifische Konstante (Wasser 999kg/m³)) 

	pressure[i] = pressure_new;
	//printf("pressure[%d] = %f:\n", i, pressure[i]);
	
}

__kernel void SPH(__global float4* pos,__global float4* vel,  __global int* neighbour,__global int* counter, __global float* density, __global float* pressure, 
				  __global float* viscosity, __global float* mass, __global float4* forceIntern, float smoothingLength, float spikyConst, float visConst)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float mue = 20; //je größer desto mehr zusammenhalt
	
	float4 f_pressure = 0.0f;
	float4 f_viscosity = 0.0f;

	//force calculation
	for(int index = 0; index < counter[i]; index++)
	{
		int j = neighbour[i * 50 + index];
	//fpressure calculation
	f_pressure += (pressure[i] + pressure[j])/density[j] * pVarSpiky(smoothingLength, p, pos[j]);
	f_viscosity +=  (vel[j] - vel[i])/density[j] * pVarVisc(smoothingLength,p, pos[j]);
	}

	//printf("distance(p, pos[10]) = %f \n",  pow(smoothingLength - distance(p, pos[10]),2));
	//printf("test[i]= %f \n", mass[10] * ((pressure[i] + pressure[10])/density[10]) * (p - pos[10])/distance(p, pos[10]) * pow(smoothingLength - distance(p, pos[10]),2));
	//printf("wSpikyDerivative [%d] -> %f:\n",i, wSpiky(distance(p.xyz, pos[neighbour[i*1000+1]].xyz), smoothingLength, spikyConst));
	//printf("wViscosityDerivative [%d] -> %f:\n",i, wVisc(distance(p.xyz, pos[neighbour[i*1000+1]].xyz), smoothingLength, visConst));

	f_pressure *= -1.0f  * 1/2 * mass[i] * spikyConst ;
	//printf("(p - pos[10])/distance(p, pos[10]) = %f \n", (p - pos[10])/distance(p, pos[10])); //0.0003
	//printf("(pressure[%d] + pressure[10])/density[10]) = %f \n", i, ((pressure[i] + pressure[10])/density[10]));
	//printf("fpressure[%d] : x=%f,y=%f,z=%f \n", i, f_pressure.x, f_pressure.y, f_pressure.z) ;

	f_viscosity *=  mue * visConst * mass[i];
	
	//printf("fviscosity[%d] : x=%f,y=%f,z=%f \n", i, f_viscosity.x, f_viscosity.y, f_viscosity.z) ;
	forceIntern[i] = f_pressure ;//+  f_viscosity;
	forceIntern[i] += f_viscosity;
	forceIntern[i] /= density[i];
	
	
//	printf("forceIntern[%d] : x=%f,y=%f,z=%f \n", i, forceIntern[i].x, forceIntern[i].y, forceIntern[i].z) ;
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float* density,__global float* mass, __global float4* forceIntern, float rho0, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;

	float f_gravity = -9.81f * mass[i];
	
	//printf("f_buoyancy = %f \n", f_buoyancy);

	//apply intern forces and extern forces
	v_new.x = v_old.x + (forceIntern[i].x/mass[i]) * dt;
	v_new.y = v_old.y + ((forceIntern[i].y + f_gravity)/mass[i]) * dt;
	v_new.z = v_old.z + (forceIntern[i].z/mass[i]) * dt;

	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt ;

	float bDamp = -0.05;
	//boundarys
	if(p_old.y < -0.5)
	{
		v_new.y *= bDamp;
		p_new.y = -0.5f;
	}	
	
	if(p_old.y > 0.5)
	{
		v_new.y *= bDamp ;
		p_new.y = 0.5f;
	}

	if(p_old.x > 0.5){
		v_new.x *= bDamp;
		p_new.x = 0.5f;
	}

	if(p_old.x < -0.5){
		v_new.x *= bDamp;
		p_new.x = -0.5f;
	}

	if(p_old.z > 0.5){
		v_new.z *= bDamp;
		p_new.z = 0.5f;
	}

	if(p_old.z < -0.5){
		v_new.z *= bDamp;
		p_new.z = -0.5f;
	}

	//damping
	v_new.xyz *= 0.99999f;

    //update the arrays with newly computed values
    pos[i].xyz = p_new.xyz;
    vel[i].xyz = v_new.xyz;
}



