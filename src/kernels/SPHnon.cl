float wPoly6(float pDistance, float h, float poly6const)
{
	//poly6
	if (0<= pDistance && pDistance <= h)
		return poly6const * pow((h*h - pDistance*pDistance),3);
	else return 0;
}

float wSpiky(float pDistance, float h, float spikyConst)
{
	//wSpiky derivative
	if (0<= pDistance && pDistance <= h)
		return -3 * spikyConst * pow((h - pDistance),2) ;
	else return 0;
}

float wVisc(float p, float h, float visConst)
{
	//wVisc laplace
	if (0<= p && p <= h)
		return visConst * (h-p);
	else return 0;
}

float pVarPoly(float h, float r)
{
	if (0<= r && r <= h)
		return h*h - r*r;
	else return 0;
}

__kernel void neighbours(__global float4* pos, __global int* neighbour, float smoothingLength, __global float* mass)
{
	unsigned int i = get_global_id(0);
	float4 p = pos[i];
	
	int counter = 0;
	//save neighbours of THIS particle in an array 
	//array size is 1000(n) times bigger than pos[]
	for (int index = 0; index < get_global_size(0); index++)
	{
		if (distance(p.xyz, pos[index].xyz) <= 0.3) // < smoothingLength
		{
			neighbour[i*1000+counter] = index;
			counter++;
			//only saves values with distance < smoothing Lenght --> [0,smoothingLength]
		}
		//stop when 1000(n) neighbours of i are found
		if (counter >= 999)
			break;
	}
	//printf("neighbour[%d] = %i:\n", 77, neighbour[77]);
}

__kernel void densityCalc(__global float4* pos, __global int* neighbour, __global float* density, __global float* pressure, 
						  __global float* mass, float smoothingLength, float poly6)
{
	unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float rho = 0;
	float rho0 = 0; //Ruhedichte
	float pressure_new = 0;
	float k = 1; //Gaskonstante
	
	//for(int index = 0; index < 1000; index++)
	//{
	//	rho += mass[neighbour[i*1000+index]] * wPoly6(distance(p.xyz, pos[neighbour[i*1000+index]].xyz), smoothingLength, poly6); 
	//}

	for(int index = 0; index < get_global_size(0); index++)
	{
		//int j = neighbour[i * 1000 + index];
		rho += mass[index] * pVarPoly(smoothingLength, distance(p.xyz ,pos[index].xyz));
	}
	rho *= poly6;
    //printf("distance p-pos[] -> %f:\n", distance(p.xyz, pos[neighbour[i*1000+1]].xyz));

	density[i] = rho;
	//printf("density[%d] = %f \n", i , density[i]);
	pressure_new = k * (rho - rho0); //p = k * (rho-rho0)(k = stoffspezifische Konstante (Wasser 999kg/m³)) 

	pressure[i] = pressure_new;
	//printf("pressure[%d] = %f:\n", i, pressure[i]);
}

__kernel void SPH(__global float4* pos,__global float4* vel,  __global int* neighbour, __global float* density, __global float* pressure, 
				  __global float* viscosity, __global float* mass, __global float4* forceIntern, float smoothingLength, float spiky, float visConst)
{
    unsigned int i = get_global_id(0);

	float4 p = pos[i];
	float viscosityConst = 1.0f; //mü

	float4 f_pressure = 0.0f;
	float4 f_viscosity = 1.0f;

	//printf("velodistance[%d] = %f \n", i, distance(vel[neighbour[i*1000]], vel[i]));
	//force calculation


	//for(int index = 0; index < 1000; index++)
	//{
	//	int j = neighbour[i * 1000 + index];
	////fpressure calculation
	//f_pressure.x += mass[j] * ((pressure[i] + pressure[j])/2*density[j]) * wSpiky(p.x - pos[j].x, smoothingLength, spiky);
	//f_pressure.y += mass[j] * ((pressure[i] + pressure[j])/2*density[j]) * wSpiky(p.y - pos[j].y, smoothingLength, spiky);
	//f_pressure.z += mass[j] * ((pressure[i] + pressure[j])/2*density[j]) * wSpiky(p.z - pos[j].z, smoothingLength, spiky);

	//f_viscosity.x +=  mass[j] * ((vel[j].x - vel[i].x)/density[j]) * wVisc(p.x - pos[j].x, smoothingLength, visConst);
	//f_viscosity.y +=  mass[j] * ((vel[j].y - vel[i].y)/density[j]) * wVisc(p.y - pos[j].y, smoothingLength, visConst);
	//f_viscosity.z +=  mass[j] * ((vel[j].z - vel[i].z)/density[j]) * wVisc(p.z - pos[j].z, smoothingLength, visConst);
	//}

	for(int j = 0; j < get_global_size(0); j++)
	{
		//int j = neighbour[i * 1000 + index];
	//fpressure calculation
	f_pressure += mass[j] * ((pressure[i] + pressure[j])/density[j]) * (p - pos[j])/distance(p, pos[j]) * pow(smoothingLength - distance(p, pos[j]),2);

	f_viscosity +=  mass[j] * ((vel[j] - vel[i])/density[j]) * (smoothingLength -  distance(p, pos[j]));

	}


	//printf("wSpikyDerivative [%d] -> %f:\n",i, wSpiky(distance(p.xyz, pos[neighbour[i*1000+1]].xyz), smoothingLength, spiky));
	//printf("wViscosityDerivative [%d] -> %f:\n",i, wVisc(distance(p.xyz, pos[neighbour[i*1000+1]].xyz), smoothingLength, visConst));
	
	//f_pressure *= -1.0f/density[i];
	f_pressure *= -1.0f * spiky * 1/2;
	
//	printf("fpressure[%d] : x=%f, y=%f, z=%f \n", i, f_pressure.x, f_pressure.y, f_pressure.z);
	f_viscosity *= viscosityConst * visConst;
	
// printf("fviscosity[%d] : x=%f,y=%f,z=%f \n", i, f_viscosity.x, f_viscosity.y, f_viscosity.z) ;
	forceIntern[i] = f_pressure;//+ f_viscosity;
	
	//forceIntern[i] = f_viscosity;
  // printf("forceIntern[%d] : x=%f,y=%f,z=%f \n", i, forceIntern[i].x, forceIntern[i].y, forceIntern[i].z) ;
}

__kernel void integration(__global float4* pos,  __global float4* vel, __global float* density,__global float* mass, __global float4* forceIntern, float dt)
{
    unsigned int i = get_global_id(0);

	float4 p_old = pos[i];
	float4 v_old = vel[i];
	float4 p_new = p_old;
	float4 v_new = v_old;


	float gravityForce = -9.81f * mass[i];

	//apply intern forces and extern forces
	v_new.x = v_old.x + (forceIntern[i].x/mass[i]) * dt;
	v_new.y = v_old.y + ((forceIntern[i].y + gravityForce)/mass[i]) * dt;
	v_new.z = v_old.z + (forceIntern[i].z/mass[i]) * dt;


	//compute new position with computed velocity
	p_new.xyz = p_old.xyz + v_new.xyz * dt ;

	//boundarys
	if(p_old.y < -0.5)
	{
		v_new.y *= -0.5f;
		p_new.y = -0.5f;
	}	
	
	if(p_old.y > 0.5)
	{
		v_new.y *= -0.999 ;
		p_new.y = 0.5f;
	}

	if(p_old.x > 0.5){
		v_new.x *= -0.5f;
		p_new.x = 0.5f;
	}

	if(p_old.x < -0.5){
		v_new.x *= -0.5f;
		p_new.x = -0.5f;
	}

	if(p_old.z > 0.5){
		v_new.z *= -0.5f;
		p_new.z = 0.5f;
	}

	if(p_old.z < -0.5){
		v_new.z *= -0.5f;
		p_new.z = -0.5f;
	}

	//damping
	v_new.xyz *= 0.99999f;

    //update the arrays with newly computed values
    pos[i].xyz = p_new.xyz;
    vel[i].xyz = v_new.xyz;
}



