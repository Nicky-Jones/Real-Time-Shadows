
//
// Model the update of each particle.  Each particle is modelled in world coordinate space
//

// Resources

//cbuffer GameTime : register (b0) {
//
//	float		gameTimeDelta;
//};
//
//
//cbuffer ClothUpdateConstants : register (b1) {
//
//	float4		gravity;
//	float4		wind;
//	uint		reCalcNormals;
//};

cbuffer sceneCBuffer : register(b3) {
	float4						windDir;
	float						Time;
	float						grassHeight;
	float						deltaTime;
	bool				USE_SHADOW_MAP;
	bool				REFLECTION_PASS;
};


// Input / Output structures

// For the update process, the geometry shader (GS) inputs and outputs the same vertex structure
struct ParticleStructure {
	float3				pos			: POSITION;
	float3				pos_old		: POSITION_OLD;
	float3				normal		: NORMAL;
	float4				matDiffuse	: DIFFUSE;
	float4				matSpecular	: SPECULAR;
	float2				texCoord	: TEXCOORD;
	uint				isFixed		: FIXED;
};


//One particle in maps to 1 paricleout
[maxvertexcount(1)]
void main(point ParticleStructure inputParticle[1],
	inout PointStream<ParticleStructure> outputParticleStream) {

	float		mass=1.0;
	float		dt = deltaTime;//0.05;
	if (dt == 0.0f) dt = 0.001f;
	float4		gravity=float4(0.0,-9.80,0.0,1.0);
	float4		wind = float4(0, 0.0, (sin(Time)+1)*10.6, 1.0);;

	ParticleStructure			outputParticle;
	
	outputParticle = inputParticle[0];

	if (inputParticle[0].isFixed == 0) {   //particle not fixed so apply forces

		float3 normal =  float3(0, 0, 1);//inputParticle[0].normal;//
		if (length(inputParticle[0].normal)>1e-7)
			normal = normalize(inputParticle[0].normal);
		//normal = normalize(inputParticle[0].normal);

		// Forces
		float3 force = gravity.xyz;
			force += dot(wind.xyz, normal)*normal;

		float damping = 0.99;
		
		
	
		float3 acceleration = force / mass;
		
		

		//verlet
		//current_velocity = (current_position - old_position) / dt
		float3 current_velocity =damping * (inputParticle[0].pos - inputParticle[0].pos_old) / dt;
		//semi-implicit euler integration        
		float3 new_velocity = current_velocity + acceleration * dt;

		//new_position = current_position + new_velocity * dt
		outputParticle.pos = inputParticle[0].pos +new_velocity * dt;

		//old_position = current_position;
		outputParticle.pos_old = inputParticle[0].pos;

		// Time dependent update (Verlet Integration)
		//outputParticle.pos += damping * dt*(inputParticle[0].pos - inputParticle[0].pos_old)+(force* dt*dt);
		//outputParticle.pos = inputParticle[0].pos;
	}
	outputParticleStream.Append(outputParticle);
}
