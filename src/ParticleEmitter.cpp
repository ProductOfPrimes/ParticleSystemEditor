// Authored By: Tom Tsiliopoulos - 100616336
// Authored By: Joss Moo-Young - 100586602
// Modified By: Shawn Matthews

#include <map> // for std::map
#include <iostream> // for std::cout
#include <glm\gtc\random.hpp> // for glm::linearRand
#include <GLM\gtc\type_ptr.hpp>

#include <GLM\gtc\matrix_transform.hpp> // for glm::transform
#include <TTK\GraphicsUtils.h> // for drawing utilities
#include <TTK\OBJMesh.h>

#include "AnimationMath.h"
#include "ParticleEmitter.h"
#include <GLM/gtx/norm.hpp>
#include <glm/glm.hpp>
#include <GLM/gtx/projection.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <limits>

#define RANDOM glm::linearRand(0.0f, 1.0f)
#define PI 3.14159f

// Particle Class Methods 
/****************************************************************************************/

/*
 * @description update method for the particle class
 * @method update
 * @return {void}
 */
void Particle::update(float dt)
{
	acceleration += force / mass;
	velocity += (acceleration * dt);

	force *= 0.f;
	acceleration *= 0.f;

	transform.update();
}

/*
 * @description enables seeking behaviour for each particle
 * @method seek
 * @return {glm::vec3}
 */
glm::vec3 Particle::seek(const glm::vec3 & target, const float & strength)
{
	glm::vec3 targetVector = target - transform.getPosition();
	float length = glm::length(targetVector);

#ifdef _DEBUG
	if (length > 0.0f)
	{
#endif
		return (targetVector / length) * strength;
#ifdef _DEBUG
	}
	else
	{
		return glm::vec3(0.0f);
	}
#endif

}

/*
 * @description enables steering behaviour for each particle
 * @method steer
 * @return {glm::vec3}
 */
glm::vec3 Particle::steer(const glm::vec3 & target, const float & strength, const float& powerCap)
{
	glm::vec3 targetVector = target - transform.getPosition();
	float length = glm::length(targetVector);

	if (length > 0.0f)
	{
		glm::vec3 ret = (targetVector * (strength / length)) - velocity;
		length = glm::length(ret);
		if (length > powerCap)
		{
			return ret * powerCap / length;
		}
	}


	return glm::vec3(0.0f);
}

/*
 * @description enables arrival behaviour for each particle
 * @method arrive
 * @return {glm::vec3}
 */
glm::vec3 Particle::arrive(const glm::vec3 & target, const float& strength, const float & radius, const float& powerCap)
{
	glm::vec3 targetVector = (target - transform.getPosition());

	//faster than length() < radius
	float length2 = glm::length2(targetVector);
	float rad2 = radius * radius;

	glm::vec3 ret;

	// equivalent to
	//	float speedScale = (length2 < rad2)? (sqrt(length2) / radius) : 1.0f;
	//  ret = (glm::normalize(targetVector) * speedScale * strength) - velocity;
	if (length2 < rad2)
	{
		ret = (targetVector / radius * strength) - velocity;
	}
	else
	{
		ret = ((targetVector / sqrt(length2))* strength) - velocity;
	}

	float length = glm::length(ret);
	if (length > powerCap)
	{
		return ret * powerCap / length;
	}

	return ret;
}

/*
* @description enables attract behaviour for each particle
* @method attract
* @return {glm::vec3}
*/
glm::vec3 Particle::attract(const glm::vec3 & target, const float & strength, const float & radius, const float & minRange)
{
	glm::vec3 targetVector = (target - transform.getPosition());
	float length2 = glm::length2(targetVector);
	float rad2 = radius * radius;

	glm::vec3 ret;
	if (length2 > (minRange * minRange))
	{
		if (length2 < rad2)
		{
			ret = (targetVector / sqrt(length2) * strength) - velocity;
		}
	}
	else
	{
		ret = glm::vec3(0.f);
	}

	return ret;
}

/*
* @description enables gravitational behaviour for each particle
* @method gravitate
* @return {glm::vec3}
*/
glm::vec3 Particle::gravitate(const glm::vec3 & target, const float & strength, const float& powerCap, const float & minRange)
{
	glm::vec3 targetVector = (target - transform.getPosition());
	float length2 = glm::length2(targetVector);

	glm::vec3 ret;
	if (length2 > (minRange * minRange))
	{
		float power = strength / (length2);
		power = algomath::min(power, powerCap);

		targetVector /= sqrt(length2);
		ret = targetVector * power;
	}
	else
	{
		ret = glm::vec3(0.f);
	}

	return ret;
}



/* 
 * @description draws each particle to the viewport
 * @method draw
 * @return {void}
 */
void Particle::draw()
{
	//TTK::Graphics::DrawSphere(worldMatrix, 0.5f, colour);
	
	if(mesh == nullptr)
	{
		TTK::Graphics::DrawSphere(worldMatrix, 0.5f, colour);
	}
	else {
		mesh->setAllColours(colour);
		mesh->draw(worldMatrix);
	}
	

}

// ParticleEmitter Methods
/**************************************************************************************/

void ParticleEmitter::loadMeshes()
{
	// Load meshes
	std::string meshPath = "../Assets/Models/";

	std::shared_ptr<TTK::OBJMesh> teapotMesh = std::make_shared<TTK::OBJMesh>();
	std::shared_ptr<TTK::OBJMesh> sphereMesh = std::make_shared<TTK::OBJMesh>();
	std::shared_ptr<TTK::OBJMesh> torusMesh = std::make_shared<TTK::OBJMesh>();
	std::shared_ptr<TTK::OBJMesh> hexMesh = std::make_shared<TTK::OBJMesh>();

	teapotMesh->loadMesh(meshPath + "teapot.obj");
	sphereMesh->loadMesh(meshPath + "sphere.obj");
	torusMesh->loadMesh(meshPath + "torus.obj");
	hexMesh->loadMesh(meshPath + "hexagon.obj");

	// assign to meshes map
	myState.meshes["teapot"] = teapotMesh;
	myState.meshes["sphere"] = sphereMesh;
	myState.meshes["torus"] = torusMesh;
	myState.meshes["hexagon"] = hexMesh;


	for (size_t i = 0; i < myState.meshes["hexagon"]->vertices.size(); i++)
	{
		myState.meshes["hexagon"]->colours.push_back(glm::vec4(1.0f));
	}

	Particle* particle = particles;
	for (unsigned int i = 0; i < myConfig.numberOfParticles; ++i, ++particle)
	{
		particle->mesh = myState.meshes["hexagon"];
	}

}

/*
* @description emits particles in cuboid shape
* @method emitFromCuboid
* @return {void}
*/
void ParticleEmitter::emitFromCuboid(Particle* p)
{
	glm::vec3 pos;
	pos.x = glm::linearRand(myConfig.boxSize.x / -2.0f, myConfig.boxSize.x / 2.0f);
	pos.y = glm::linearRand(myConfig.boxSize.y / -2.0f, myConfig.boxSize.y / 2.0f);
	pos.z = glm::linearRand(myConfig.boxSize.z / -2.0f, myConfig.boxSize.z / 2.0f);

	p->transform.setPosition(pos);

	p->velocity = glm::vec3(0.0f, 0.0f, 1.0f);
}

/*
* @description emits particles in spherical shape
* @method emitFromSphere
* @return {void}
*/
void ParticleEmitter::emitFromSphere(Particle * p)
{
	float longitude = acos((2.0f * RANDOM) - 1.0f);
	float latitude = RANDOM * 2.0f * PI;

	glm::vec2 polar = glm::vec2(latitude, longitude);

	glm::vec3 direction = glm::euclidean(polar);

	p->transform.setPosition(direction * glm::linearRand(0.0f, myConfig.sphereRadius));

#ifdef _DEBUG
	if (isnan(p->transform.getPosition().x))
	{
		printf("NaN spawn!\n");
	}
#endif //debug

	p->velocity = direction;
}

/*
* @description emits particles in cone-like shape (frustum)
* @method emitFromFrustrum
* @return {void}
*/
void ParticleEmitter::emitFromFrustum(Particle * p)
{
	float normalizedRadiusSpawn = RANDOM;
	float rotSpawn = glm::linearRand(0.0f, 2.0f * PI);

	glm::vec3 posSpawn = glm::vec3(normalizedRadiusSpawn * myConfig.frustumRadiusSpawn * cos(rotSpawn), 0.f, normalizedRadiusSpawn * myConfig.frustumRadiusSpawn * sin(rotSpawn)); // y up
	glm::vec3 posTarget = glm::vec3(normalizedRadiusSpawn * myConfig.frustumRadiusTarget * cos(rotSpawn), myConfig.frustumHeight, normalizedRadiusSpawn * myConfig.frustumRadiusTarget * sin(rotSpawn));

	glm::vec3 delta = posTarget - posSpawn;

	p->transform.setPosition(glm::linearRand(posSpawn, posTarget));

	float deltaLen = glm::length(delta);

	if (deltaLen == 0.0f)
	{
		p->velocity = glm::vec3(0.f, 0.f, 1.f);
	}
	else
	{
		p->velocity = delta / deltaLen;
	}
}

/*
* @description a quick hack to load data using boost for 3D Path
* @method hackToPath3D
* @return {void}
*/
void ParticleEmitter::hackToPath3D(algomath::Path<glm::vec3>& path, const std::vector<std::vector<algomath::NodeGraphTableEntry<glm::vec3>>>& hack)
{
	path.m_data.clear();
	for (size_t i = 0; i < hack.size(); i++)
	{
		std::list<algomath::NodeGraphTableEntry<glm::vec3>> table;
		for each (algomath::NodeGraphTableEntry<glm::vec3> tableEntry in hack[i])
		{
			table.push_back(tableEntry);
		}
		path.m_data.push_back(table);
	}
	path.updateDistances();
}

/*
* @description a quick hack to save data using boost for 3D Path
* @method pathToHack3D
* @return {void}
*/
void ParticleEmitter::pathToHack3D(std::vector<std::vector<algomath::NodeGraphTableEntry<glm::vec3>>>& hack, algomath::Path<glm::vec3>& path)
{
	hack.clear();
	for (int i = 0; i < path.m_data.size(); i++)
	{
		std::vector<algomath::NodeGraphTableEntry<glm::vec3>> table;
		auto tables = path.m_data[i];
		for (auto i = tables.begin(); i != tables.end(); i++)
		{
			table.push_back(*i);
		}
		hack.push_back(table);
	}
}

void ParticleEmitter::hackToPath(algomath::Path<float>& path, const std::vector<std::vector<algomath::NodeGraphTableEntry<float>>>& hack)
{
	path.m_data.clear();
	for (size_t i = 0; i < hack.size(); i++)
	{
		std::list<algomath::NodeGraphTableEntry<float>> table;
		for each (algomath::NodeGraphTableEntry<float> tableEntry in hack[i])
		{
			table.push_back(tableEntry);
		}
		path.m_data.push_back(table);
	}
}

void ParticleEmitter::pathToHack(std::vector<std::vector<algomath::NodeGraphTableEntry<float>>>& hack, const algomath::Path<float>& path)
{
	hack.clear();
	for (int i = 0; i < path.m_data.size(); i++)
	{
		std::vector<algomath::NodeGraphTableEntry<float>> table;
		auto tables = path.m_data[i];
		for (auto i = tables.begin(); i != tables.end(); i++)
		{
			table.push_back(*i);
		}
		hack.push_back(table);
	}
}

void ParticleEmitter::hacksToPaths()
{
	hackToPath3D(myState.path, pathHack);

	hackToPath(myState.speedGraph, speedHack);
	hackToPath(myState.sizeGraph, sizeHack);
	hackToPath(myState.colourGraph, colourHack);
}

void ParticleEmitter::pathsToHacks()
{
	pathToHack3D(pathHack, myState.path);

	pathToHack(speedHack, myState.speedGraph);
	pathToHack(sizeHack, myState.sizeGraph);
	pathToHack(colourHack, myState.colourGraph);
}

/*
 * @description main constructor for ParticleEmitter - initializes paricles, numberOfParticles and playing parameters
 * @constructor
 */
ParticleEmitter::ParticleEmitter()
	: particles(nullptr),
	myConfig(Config()),
	myState(ActiveState())
{
	initialize(100);

	myConfig.numberOfParticles = 0;
	myConfig.playing = true;
}

/*
 * @description default destructor for ParticleEmitter class
 * @destructor
 */
ParticleEmitter::~ParticleEmitter()
{
	freeMemory();
}

/*
 * @description this method initializes the particle emitter properties
 * @method initialize
 * @params {unsigned int} numParticles
 * @return {void}
 */
void ParticleEmitter::initialize(unsigned int numParticles)
{
	freeMemory(); // destroy any existing particles
	

	if (numParticles > 0)
	{
		particles = new Particle[numParticles];
		myConfig.numberOfParticles = numParticles;
	}

	timeRemaining = myConfig.duration;

	myState.sizeGraph = algomath::createDefaultTable<float>();
	myState.speedGraph = algomath::createDefaultTable<float>();
	myState.colourGraph = algomath::createDefaultTable<float>();

	//transforms
	myState.transform.setPosition(glm::vec3(0.f));
	myState.transform.setRotation(glm::vec3(0.f));

	////emitter properties////
	myConfig.rotationalVelocity = glm::vec3(0.f);
	myConfig.emitterOffset = glm::vec3(0.f);
	//myConfig.numberOfParticles = 500;
	myConfig.emissionShape = EMISSION_SHAPE::CUBOID;
	myConfig.emissionRate = 50.0f;

	myConfig.boxSize = glm::vec3(20.0f);

	myConfig.sphereRadius = 10.0f;

	myConfig.frustumRadiusSpawn = 0.0f;
	myConfig.frustumRadiusTarget = 1.0f;
	myConfig.frustumHeight = 2.0f;

	myConfig.playing = true;
	myConfig.loop = true;

	myConfig.loopDelay = 0.f;
	myConfig.duration = -1.f;

	myConfig.parentTransforms = false;

	// path options
	myConfig.followPath = false;
	myConfig.directFollowMode = false;
	myConfig.lookAhead = 0.1f;
	myConfig.pathRadius = 0.1f;
	myConfig.pathPower = 200.0f;

	// seek options
	myConfig.seekingBehaviours = false;
	myConfig.steeringBehaviours = false;
	myConfig.seekPoint = glm::vec3(0.f);
	myConfig.seekForce = 0.0f;

	// steer options
	myConfig.steerPoint = glm::vec3(0.f);
	myConfig.steerForce = 0.0f;
	myConfig.steerMaxForce = 1.0f;

	// gravitate options
	myConfig.gravitatePoint = glm::vec3(0.f);
	myConfig.gravitatePower = 0.f;
	myConfig.gravitateMaxForce = 1.f;

	// attract options
	myConfig.attractPoint = glm::vec3(0.f);
	myConfig.attractForce = 0.f;
	myConfig.attractRange = 10.f;

	// arrive options
	myConfig.arrivePoint = glm::vec3(0.f);
	myConfig.arriveForce = 0.f;
	myConfig.arriveRange = 10.f;
	myConfig.arriveMaxForce = 1.f;

	myConfig.globalEffects = false;
	myConfig.globalForceVector = glm::vec3(0.0f, 10.0f, 0.0f);
	myConfig.globalAccelerationVector = glm::vec3(0.f, 0.0f, -9.8f);

	// Speed Options
	myConfig.initialSpeedRange = glm::vec2(100.f, 500.f);
	myConfig.limitSpeedOverLifetime = false;
	myConfig.initialSpeedLimitRange = glm::vec2(0.f, 0.f);
	myConfig.finalSpeedLimitRange = glm::vec2(0.f, 0.f);

	myConfig.lifeRange = glm::vec2(1.0f, 4.0f);

	// Size Options
	myConfig.sizeOverLifetime = true;
	myConfig.sizeRangeBegin = glm::vec2(10.0f, 20.0f);
	myConfig.sizeRangeEnd = glm::vec2(2.0f, 3.0f);
	myConfig.massRange = glm::vec2(1.0f, 2.0f);

	// Colour Options
	myConfig.colourOverLifetime = true;
	myConfig.colourBegin0 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	myConfig.colourBegin1 = glm::vec4(1.0f, 0.27f, 0.0f, 1.0f);
	myConfig.colourEnd0 = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	myConfig.colourEnd1 = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f);

	loadMeshes();
}

/*
 * @description sets the lifetime of all particles attached to this emitter to -1
 * @method killParticles
 * @return {void}
 */
void ParticleEmitter::killParticles()
{
	Particle* particle = particles;
	for (unsigned int i = 0; i < myConfig.numberOfParticles; ++i, ++particle)
	{
		particle->life = -1.0f;
	}
}

/*
 * @description performs clean up operations for this emitter and all particles
 * @method freeMemory
 * @return {void}
 */
void ParticleEmitter::freeMemory()
{
	if (particles) // if not a null pointer
	{
		delete[] particles;
		particles = nullptr;
		myConfig.numberOfParticles = 0;
	}
}

/*
 * @description this method updates the emitter every frame
 * @method update
 * @params {float} dt - deltaTime
 * @return {void}
 */
void ParticleEmitter::update(float dt)
{
	// update emitter
	glm::vec3 rotation = myConfig.rotationalVelocity * dt;
	myState.transform.rotateXYZ(rotation);
	myState.transform.update();

	// update particles

	if (particles && myConfig.playing) // make sure memory is initialized and system is playing
	{
		emissionTime += dt;
		timeRemaining -= dt;

		if (timeRemaining < 0.0f)
		{
			if (myConfig.duration > 0.0f)
			{
				emissionTime = 0.0f; //will prevent particles from spawning
				if (myConfig.loop)
				{
					if (timeRemaining < -myConfig.loopDelay)
					{
						killParticles();
						timeRemaining = myConfig.duration;
					}
				}
			}
		}

		unsigned int NumParticlesToEmit = emissionTime * myConfig.emissionRate;

		// loop through each particle
		Particle* particle = particles;
		for (unsigned int i = 0; i < myConfig.numberOfParticles; ++i, ++particle)
		{
			if (particle->life <= 0.0f) // if particle has no life remaining
			{
				if (NumParticlesToEmit > 0)
				{
					spawnParticle(particle);
					NumParticlesToEmit--;
					emissionTime -= (1.0f / myConfig.emissionRate); //subtract the time it takes to spawn a particle
					updateParticle(particle, dt);
				}
				else continue; // don't update a dead particle
			}
			else
			{
				updateParticle(particle, dt);
			}
		}
	}
}

/*
 * @description checks which behaviours are active for each particle and 
 * calls the particle update method
 * @method updateParticle
 * @params {Particle *} particle
 * @params {const float&} dt - delta time
 * @return {void}
 */
void ParticleEmitter::updateParticle(Particle * particle, const float& dt)
{
	// Update physics
	if (myConfig.seekingBehaviours)
	{
		particle->force += particle->seek(myConfig.seekPoint, myConfig.seekForce);
		particle->force += particle->gravitate(myConfig.gravitatePoint, myConfig.gravitatePower, myConfig.gravitateMaxForce);
		particle->force += particle->attract(myConfig.attractPoint, myConfig.attractForce, myConfig.attractRange);
	}
	if (myConfig.steeringBehaviours)
	{
		particle->force += particle->steer(myConfig.steerPoint, myConfig.steerForce, myConfig.steerMaxForce);
		particle->force += particle->arrive(myConfig.arrivePoint, myConfig.arriveForce, myConfig.arriveRange, myConfig.arriveMaxForce);
	}

	if (myConfig.globalEffects)
	{
		particle->force += myConfig.globalForceVector;
		particle->acceleration += myConfig.globalAccelerationVector;
	}

	if (myConfig.followPath)
	{
		if (myConfig.directFollowMode)
		{
			applyDirectPathFollow(dt, particle);
		}
		else
		{
			applyPathSteering(dt, particle);
		}
	}


	float normalizedLife = algomath::clamp(1.0f - (particle->life / particle->lifespan), 0.0f, 1.0f);

	if (myConfig.sizeOverLifetime)
	{
		float normalizedSize = myState.sizeGraph.lookupValue(normalizedLife);
		float size = algomath::lerp(particle->sizeBegin, particle->sizeEnd, normalizedSize);
		particle->transform.setScale(size);
	}

	if (myConfig.colourOverLifetime)
	{
		float normalizedColour = myState.colourGraph.lookupValue(normalizedLife);
		glm::vec4 colour = algomath::lerp(particle->colourBegin, particle->colourEnd, normalizedColour);
		particle->colour = colour;
	}

	// Update position
	particle->update(dt);

	if (myConfig.limitSpeedOverLifetime)
	{
		float normalizedSpeed = myState.speedGraph.lookupValue(normalizedLife);
		float speed = algomath::lerp(particle->speedLimitBegin, particle->speedLimitEnd, normalizedSpeed);

		particle->velocity = algomath::limitMagnitude(particle->velocity, speed);
	}

	particle->transform.move(particle->velocity * dt);

#ifdef _DEBUG
	if (isnan(particle->transform.getPosition().x))
	{
		printf("NaN at particle %u ", particle);

		if (isnan(particle->force.x))
		{
			printf("force! ");
		}

		if (isnan(particle->acceleration.x))
		{
			printf("accel! ");
		}

		if (isnan(particle->velocity.x))
		{
			printf("vel! ");
		}

		printf("pos! mass: %f, life: %f, lifespan: %f\n", particle->mass, particle->life, particle->lifespan);
	}
#endif

	if (myConfig.parentTransforms)
	{
		particle->worldMatrix = worldMatrix * particle->transform.getTransform();
//#ifdef _DEBUG
//		if (isnan(transform.getPosition().x))
//		{
//			printf("transform has NaN!\n");
//		}
//#endif
	}
	else
	{
		particle->worldMatrix = particle->transform.getTransform();
	}
	//else
	//{
	//	particle->transform.setParentTransform(nullptr);
	//}

	particle->life -= dt;
}

/*
 * @description this method draws the particle emitter and all attached particles
 * @method draw
 * @return {void}
 */
void ParticleEmitter::draw()
{
	Particle* p = particles;
	for (int i = 0; i < myConfig.numberOfParticles; ++i, ++p)
	{
		if (p->life > 0.0f) // if particle is alive, draw it
		{
			//viewfrustum call - only draw particles on screen
			p->draw();
		}
	}
}

/*
 * @description this method spawns each particle according to paramater options assigned
 * @method spawnParticle
 * @params {Particle *} p - a pointer to each particle
 * @return {void}
 */
inline void ParticleEmitter::spawnParticle(Particle * p)
{
	//p->mesh = meshes["sphere"];meshes["sphere"]->vertices.size()
	
	p->colourBegin = algomath::lerp(myConfig.colourBegin0, myConfig.colourBegin1, RANDOM);
	p->colourEnd = algomath::lerp(myConfig.colourEnd0, myConfig.colourEnd1, RANDOM);

	p->lifespan = algomath::lerp(myConfig.lifeRange.x, myConfig.lifeRange.y, RANDOM);
	p->life = p->lifespan;

	float randomTval_A = glm::linearRand(0.0f, 1.0f);
	//couple mass and size relationship
	p->mass = algomath::lerp(myConfig.massRange.x, myConfig.massRange.y, randomTval_A);
	p->sizeBegin = algomath::lerp(myConfig.sizeRangeBegin.x, myConfig.sizeRangeBegin.y, randomTval_A);
	p->sizeEnd = algomath::lerp(myConfig.sizeRangeEnd.x, myConfig.sizeRangeEnd.y, randomTval_A);

	//emission shapes determine initial velocity and position distribution
	switch (myConfig.emissionShape)
	{
	case CUBOID:
	{
		emitFromCuboid(p);
		break;
	}
	case FRUSTUM:
	{
		emitFromFrustum(p);
		break;
	}
	default:
	case SPHERE:
	{
		emitFromSphere(p);
		break;
	}
	}
	//emit functions will set a position and a normalized velocity 

	float startspeed = glm::linearRand(myConfig.initialSpeedRange.x, myConfig.initialSpeedRange.y);
	p->velocity = p->velocity * startspeed;
	p->transform.move(myConfig.emitterOffset);


	p->speedLimitBegin = glm::linearRand(myConfig.initialSpeedLimitRange.x, myConfig.initialSpeedLimitRange.y);
	p->speedLimitEnd = glm::linearRand(myConfig.finalSpeedLimitRange.x, myConfig.finalSpeedLimitRange.y);

	p->distanceTravelledAlongPath = 0.0f;

	if (!myConfig.parentTransforms)
	{
		p->velocity = myState.transform.getRotationMatrix() * glm::vec4(p->velocity, 1.0f);
		p->transform.setPosition(worldMatrix * glm::vec4(p->transform.getPosition(), 1.0f));
	}

	p->transform.setScale(p->sizeBegin);
	p->colour = p->colourBegin;
}

/*
 * @description this method applies steering behaviour to each particle
 * @method applyPathSteering
 * @params {const float&} dt - deltaTime
 * @params {Particle *} particle - a pointer to each particle
 * @return {void}
 */
void ParticleEmitter::applyPathSteering(const float& dt, Particle* particle)
{
	//todo: optimizations e.g. have particles store their interval so i dont have to search it

	particle->distanceTravelledAlongPath = fmod(particle->distanceTravelledAlongPath, myState.path.getLength());//fmod(particle->distanceTravelledAlongPath, path.getLength());
	size_t numIntervals = myState.path.numIntervals();
	// find interval
	unsigned int interval = myState.path.lookupInterval(particle->distanceTravelledAlongPath);
	std::list<algomath::NodeGraphTableEntry<glm::vec3>>::iterator current = myState.path.iterByDist(interval, particle->distanceTravelledAlongPath);
	std::list<algomath::NodeGraphTableEntry<glm::vec3>>::iterator next = std::next(current);

	glm::vec3 proj;
	glm::vec3 pathVec; // the current segment for the path
	glm::vec3 futurePosition = particle->transform.getPosition() + (particle->velocity *  dt);
	glm::vec3 pathTarget;

	if (next != myState.path.m_data[interval].end())
	{
		pathVec = next->val - current->val;
		proj = glm::proj(futurePosition - current->val, pathVec);
	}
	else
	{
		if (interval == numIntervals - 1u)
		{
			pathVec = current->val - std::prev(current)->val;
			float scalarProj = algomath::scalarProj(futurePosition, pathVec);
			proj = pathVec * scalarProj;
			if (isnan(proj.x))
			{
				proj = glm::vec3(0.0f);
			}
		}
		else
		{
			// line to next interval
			pathVec = std::next(myState.path.m_data[interval + 1u].begin())->val - current->val;//next = std::next(path.m_data[interval + 1u].begin());
			proj = glm::proj(futurePosition - current->val, pathVec);
		}
	}


	if (glm::length2(proj + current->val - futurePosition) > (myConfig.pathRadius * myConfig.pathRadius)) // if distance to the path is greater than a threshold
	{
		pathTarget = myState.path.lookupValue(fmod((particle->distanceTravelledAlongPath + myConfig.lookAhead), myState.path.getLength()));
		particle->force += particle->steer(pathTarget, myConfig.pathPower, myConfig.pathPower);
	}

	float distanceAlongInterval = glm::length(proj);
	particle->distanceTravelledAlongPath = current->distanceAlongPath + distanceAlongInterval;
}

/*
* @description this method applies direct path follow behaviour for each particle
* @method applyPathSteering
* @params {const float&} dt - deltaTime
* @params {Particle *} particle - a pointer to each particle
* @return {void}
*/
void ParticleEmitter::applyDirectPathFollow(const float& dt, Particle* particle)
{
	particle->distanceTravelledAlongPath = fmod(particle->distanceTravelledAlongPath, myState.path.getLength());
	float distanceToTravel = myConfig.pathPower * dt;

	glm::vec3 pathTarget = myState.path.lookupValue(particle->distanceTravelledAlongPath + distanceToTravel);
	particle->transform.setPosition(pathTarget);

	particle->distanceTravelledAlongPath += distanceToTravel;
}

/* 
 * @description this method returns the position of each particle
 * @method getParticlePosition
 * @params {unsigned int} idx - index of the particle
 * @return {glm::vec3}
 */
glm::vec3 ParticleEmitter::getParticlePosition(unsigned int idx)
{
	if (idx >= myConfig.numberOfParticles)
	{
		std::cout << "ParticleEmitter::getParticlePosition ERROR: idx " << idx << "out of range!" << std::endl;
		return glm::vec3();
	}
	return particles[idx].transform.getPosition();
}

/*
 * @description this method sets the number of particles in the particle system
 * @method setNumParticles
 * @params {unsigned int} numParticles
 * @return {void}
 */
void ParticleEmitter::setNumParticles(unsigned int numParticles) 
{
	freeMemory(); // destroy any existing particles

	if (numParticles > 0)
	{
		particles = new Particle[numParticles];
		myConfig.numberOfParticles = numParticles;

		Particle* particle = particles;
		for (unsigned int i = 0; i < myConfig.numberOfParticles; ++i, ++particle)
		{
			particle->mesh = myState.meshes["hexagon"];
		}
	}

	
}

/*
* @description this method sets the liferange of particles in the particle system
* @method setLifeRange
* @params {float} min
* @params {float} max
* @return {void}
*/
void ParticleEmitter::setLifeRange(float min, float max) {
	ParticleEmitter::myConfig.lifeRange = glm::vec2(min, max);
}

/*
* @description this method sets the beginning size range of particles in the particle system
* @method setSizeRangeBegin
* @params {float} min
* @params {float} max
* @return {void}
*/
void ParticleEmitter::setSizeRangeBegin(float min, float max) {
	ParticleEmitter::myConfig.sizeRangeBegin = glm::vec2(min, max);
}

/*
* @description this method sets the ending size range of particles in the particle system
* @method setSizeRangeEnd
* @params {float} min
* @params {float} max
* @return {void}
*/
void ParticleEmitter::setSizeRangeEnd(float min, float max) {
	ParticleEmitter::myConfig.sizeRangeEnd = glm::vec2(min, max);
}


// ParticleSystem Methods
/***************************************************************************************/

/*
 * @description this is the empty / default constructor for the ParticleSystem class
 * @constructor
 */
ParticleSystem::ParticleSystem()
{
}

/*
 * @description this is the desctructor for the ParticleSystem class
 * @destructor
 */
ParticleSystem::~ParticleSystem()
{
	clearSystem();
}

/*
* @description this is the update method for the ParticleSystem class
* @method update
* @return {void}
*/
void ParticleSystem::update()
{
	for (auto emitter : m_emitters)
	{
		emitter->worldMatrix = parent->transformable->getTransform() * emitter->myState.transform.getTransform();
		emitter->update(0.016f); //todo: temp dt solution
		emitter->draw();
	}
}

/*
* @description this method removes all particle emitters from the ParticleSystem class
* @method clearSystem
* @return {void}
*/
void ParticleSystem::clearSystem()
{
	for (auto ptr : m_emitters)
	{
		delete ptr;
	}
	m_emitters.clear();
}

/*
* @description this method removes the specified particle emitter from the ParticleSystem class
* @method removeAt
* @params {size_t} index
* @return {void}
*/
void ParticleSystem::removeAt(size_t index)
{
	delete m_emitters[index];
	m_emitters.erase(m_emitters.begin() + index);
}

/*
* @description this method the number of particle emitters contained in the ParticleSystem class
* @method numEmitters
* @return {size_t}
*/
size_t ParticleSystem::numEmitters()
{
	return m_emitters.size();
}

/*
* @description this method adds a new particle emitter to the ParticleSystem
* @method addEmitter
* @params {ParticleEmitter *} emitter
* @return {void}
*/
void ParticleSystem::addEmitter(ParticleEmitter * emitter)
{
	m_emitters.push_back(emitter);
}

/*
* @description this method returns the particle emitter at the specified index
* @method getEmitter
* @params {size_t} index
* @return {ParticleEmitter *}
*/
ParticleEmitter * ParticleSystem::getEmitter(size_t index)
{
	return m_emitters[index];
}

/*
* @description this method removes the last particle emitter
* @method removeEmitter
* @return {void}
*/
void ParticleSystem::removeEmitter()
{
	delete m_emitters.back();
	m_emitters.pop_back();
}

/*
* @description this method returns a reference to the last particle emitter
* @method lastEmitter
* @return {ParticleEmitter *}
*/
ParticleEmitter * ParticleSystem::lastEmitter()
{
	return m_emitters.back();
}


/* UTILITY METHODS */
/****************************************************************************************/
void drawMat4(const glm::mat4 & mat, const float& scale)
{
	TTK::Graphics::DrawVector(mat[3], mat[0] * scale, scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // x
	TTK::Graphics::DrawVector(mat[3], mat[1] * scale, scale, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // y
	TTK::Graphics::DrawVector(mat[3], mat[2] * scale, scale, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)); // z
}

void drawTransform(const Transform & t, const float& scale)
{
	glm::mat4 mat = t.getTransform();
	drawMat4(mat, scale);
}
