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


__kernel void neighbours(__global float4* pos, __global int* neighbour,__global int* counter, float smoothingLength, __global float* mass,  __global float* isAlive)
{
	unsigned int i = get_global_id(0);
	if (isAlive[i] > 0.5)
	{

	float4 p = pos[i];
	
	counter[i] = 0;
	//save neighbours of THIS particle in an array 
	//array size is 50(n) times bigger than pos[]
	for (int index = 0; index < get_global_size(0); index++)
	{
		if (distance(p.xyz, pos[index].xyz) <= smoothingLength && isAlive[index] == 1) // < smoothingLength
		{
			neighbour[i*50+counter[i]] = index;
			counter[i]++;
			//only saves values with distance < smoothing Lenght --> [0,smoothingLength]
		}
		//stop when 50(n) neighbours of i are found
		if (counter[i] >= 49)
			break;
	}
	}
}

__kernel void densityCalc(__global float4* pos, __global int* neighbour, __global int* counter, __global float* density, __global float* pressure, 
						  __global float* mass, float smoothingLength, float poly6, float rho0, __global float* isAlive)
{
	unsigned int i = get_global_id(0);

	if (isAlive[i] > 0.5)
	{

	float4 p = pos[i];
	float rho = 0;
	float pressure_new = 0;
	float k = 0.00125; //Gaskonstante

	for(int index = 0; index < counter[i]; index++)
	{
		int j = neighbour[i * 50 + index];
		rho += pVarPoly(smoothingLength, distance(p.xyz ,pos[j].xyz));
	}
	rho *= poly6 * mass[i];

	density[i] = rho;
	pressure_new = k * (rho - rho0); //p = k * (rho-rho0)(k = stoffspezifische Konstante (Wasser 999kg/m³)) 

	pressure[i] = pressure_new;

	}
}

__kernel void SPH(__global float4* pos,__global float4* vel,  __global int* neighbour,__global int* counter, __global float* density, __global float* pressure, 
				  __global float* mass, __global float4* forceIntern, float smoothingLength, float spiky, float visConst, __global float* isAlive)
{
    unsigned int i = get_global_id(0);

	if (isAlive[i] > 0.5)
	{
	float4 p = pos[i];
	float viscosityConst = 0.0000005; //je größer desto mehr zusammenhalt
	
	float4 f_pressure = 0.0f;
	float4 f_viscosity = 0.0f;

	//force calculation
	for(int index = 0; index < counter[i]; index++)
	{
		int j = neighbour[i * 50 + index];
	//fpressure calculation
	f_pressure += (pressure[i] + pressure[j])/density[j] * pVarSpiky(smoothingLength, p, pos[j]);
	
	//fviscosity calculation
	f_viscosity +=  (vel[j] - vel[i])/density[j] * pVarVisc(smoothingLength,p, pos[j]);
	}

	f_pressure *= -1.0f  * 1/2 * mass[i] * spiky ;
	
	f_viscosity *=  viscosityConst * visConst * mass[i];
	
	forceIntern[i] = f_pressure +  f_viscosity;
	forceIntern[i] /= density[i];
	}
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float4* pos_gen, __global float4* vel_gen, __global float* life_gen, __global float* life, __global float* density,
						  __global float* mass, __global float4* forceIntern, float rho0, float dt, __global float* isAlive, __global int* aliveHelper, float buoyancy, float lifeDeduction)
{
    unsigned int i = get_global_id(0);
	
	if(aliveHelper[i] == 1)
		isAlive[i] = 1.0;

	if (isAlive[i] > 0.5)
	{

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;

	life[i] -= lifeDeduction*dt;
	if(life[i] <= 0)
    {
        p_old = pos_gen[i];
        v_old = vel_gen[i];
        life[i] = 1;    
    }	

	//float gravityForce = -9.81f * mass[i];
	float gravityForce = buoyancy * (density[i] - rho0) * -9.81f * mass[i];

	//apply intern forces and extern forces
	v_new.x = v_old.x + (forceIntern[i].x/mass[i]) * dt;
	v_new.y = v_old.y + ((forceIntern[i].y + gravityForce)/mass[i]) * dt;
	v_new.z = v_old.z + (forceIntern[i].z/mass[i]) * dt;

	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt ;

//	float bDamp = -1.2;
	float bDamp = -0.9;
	
	//boundarys
	if(p_old.y < -0.5)
	{
		v_new.y *= bDamp;
		p_new.y = -0.5f;
	}	
	
	if(p_old.y > 2)
	{
		v_new.y *= bDamp ;
		p_new.y = 2;
	}

	if(p_old.x > 1){
		v_new.x *= bDamp;
		p_new.x = 1;
	}

	if(p_old.x < -1.0){
		v_new.x *= bDamp;
		p_new.x = -1.0f;
	}

	if(p_old.z > 1){
		v_new.z *= bDamp;
		p_new.z = 1;
	}

	if(p_old.z < -1){
		v_new.z *= bDamp;
		p_new.z = -1.f;
	}

	//global damping
	v_new.xyz *= 0.99999f;

    //update the arrays with newly computed values
    pos[i].xyz = p_new.xyz;
    vel[i].xyz = v_new.xyz;

	}
}



