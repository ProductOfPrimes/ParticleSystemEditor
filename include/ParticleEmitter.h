// Tom Tsiliopoulos - 100616336
// Joss Moo-Young - 100586602
// modified by: Shawn Matthews

#pragma once

#include "NodeGrapher.h"
#include "GameObject.h"
#include <TTK\OBJMesh.h>
#include <map> // for std::map

#define PRETTY_MUCH_ZERO 0.0000000001f

class ParticleEmitter;
class ParticleSystem;

enum EMISSION_SHAPE
{
	SPHERE = 0,
	CUBOID,
	FRUSTUM,
	NUM_EMISSION_SHAPES
};

struct Particle
{
	glm::mat4 worldMatrix;
	Transform transform;

	friend class ParticleEmitter;

	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 force;
	float mass;
	glm::vec4 colour = glm::vec4(1.0f);

	// Visual Properties
	float sizeBegin;
	float sizeEnd;
	glm::vec4 colourBegin;
	glm::vec4 colourEnd;
	float speedLimitBegin;
	float speedLimitEnd;

	float life; // lifetime remaining in seconds
	float lifespan;

	

	float distanceTravelledAlongPath = 0.f;

	void update(float dt); // update position based off of forces/acceleration, and then resets them to 0
						   // you can add acceleration or force or both
						   // velocity will not reset each frame
	
	// Asset databases
	std::shared_ptr<TTK::OBJMesh> mesh; //= std::make_shared<TTK::OBJMesh>();

	void draw();

	//return vectors to be added.
	//optimization: create versions of these that dont have to compute vectors to target or ranges
	glm::vec3 seek(const glm::vec3& target, const float& strength);
	glm::vec3 attract(const glm::vec3& target, const float& strength, const float& radius, const float & minRange = PRETTY_MUCH_ZERO); // move toward/away from target, but only within radius
	glm::vec3 gravitate(const glm::vec3& target, const float& strength, const float& powerCap, const float & minRange = PRETTY_MUCH_ZERO); //power = scale / distance^2. capped power, min range within which produces a zero output.

	

	//steering behaviours that take velocity into account
	glm::vec3 steer(const glm::vec3& target, const float& strength, const float& powerCap); //like seek, but takes velocity into account
	glm::vec3 arrive(const glm::vec3& target, const float& strength, const float& radius, const float& powerCap); //move toward target, slow down at the target
};

class ParticleEmitter
{
	friend class boost::serialization::access;
	friend class ParticleSystem;
private:
	glm::mat4 worldMatrix;
	Particle* particles;
	float emissionTime = 0.0f; // keeps track of time passed to know how many particles need to be created
	float timeRemaining;

	void emitFromCuboid(Particle* p);
	void emitFromSphere(Particle* p);
	void emitFromFrustum(Particle* p);

	//these hacks are just for file I/O
	std::vector<std::vector<algomath::NodeGraphTableEntry<glm::vec3>>> pathHack;
	std::vector<std::vector<algomath::NodeGraphTableEntry<float>>> sizeHack;
	std::vector<std::vector<algomath::NodeGraphTableEntry<float>>> speedHack;
	std::vector<std::vector<algomath::NodeGraphTableEntry<float>>> colourHack;

	void hackToPath3D(algomath::Path<glm::vec3>& path, const std::vector<std::vector<algomath::NodeGraphTableEntry<glm::vec3>>> & hack);
	void pathToHack3D(std::vector<std::vector<algomath::NodeGraphTableEntry<glm::vec3>>> & hack, algomath::Path<glm::vec3>& path);
	
	void hackToPath(algomath::Path<float>& path, const std::vector<std::vector<algomath::NodeGraphTableEntry<float>>> & hack);
	void pathToHack(std::vector<std::vector<algomath::NodeGraphTableEntry<float>>> & hack, const algomath::Path<float>& path);

public:

	void loadMeshes();

	void hacksToPaths();
	void pathsToHacks();

	ParticleEmitter();
	~ParticleEmitter();

	void initialize(unsigned int numParticles);

	void killParticles();
	void freeMemory();

	void update(float dt);
	void updateParticle(Particle* particle, const float& dt);
	void draw();

	inline void spawnParticle(Particle* p);

	void applyPathSteering(const float& dt, Particle* particle);
	void applyDirectPathFollow(const float& dt, Particle* particle);


	unsigned int getNumParticles() { return myConfig.numberOfParticles; }
	glm::vec3 getParticlePosition(unsigned int idx);

	void setNumParticles(unsigned int numParticles);

	void setLifeRange(float min, float max);

	void setSizeRangeBegin(float min, float max);
	void setSizeRangeEnd(float min, float max);

	struct ActiveState {
		Transform transform;

		algomath::Path<glm::vec3> path;
		//motion graphs
		algomath::Path<float> sizeGraph;
		algomath::Path<float> speedGraph;
		algomath::Path<float> colourGraph;

		//std::map<std::string, std::shared_ptr<TTK::MeshBase>> meshes;
		std::map<std::string, std::shared_ptr<TTK::OBJMesh>> meshes;
	} myState;

	struct Config {
		unsigned int numberOfParticles;

		////emitter properties///////////////////////////////////////////////////////////////////////
		glm::vec3 rotationalVelocity;
		glm::vec3 emitterOffset;
		int emissionShape = SPHERE;
		float emissionRate = 10.0f;

		//shapes
		glm::vec3 boxSize; // size of the emitter box

		float sphereRadius = 2.0f;

		float frustumRadiusSpawn = 1.0f; //first circle, the part where vectors originate
		float frustumRadiusTarget = 2.0f; //second circle, the part where vectors head toward
		float frustumHeight = 3.0f;

		///// Playback properties
		bool playing = true;
		bool loop = true;

		float loopDelay = 0.0f;
		float duration = -1.0f; //negative duration means infinite

		//// Parent toggle
		bool parentTransforms = false;

		// path following stuff
		bool followPath = false;
		bool directFollowMode = false;
		float lookAhead = 10.f; // how far ahead particles will seek
		float pathRadius = 10.f;
		float pathPower = 10.f; //treated like speed in direct mode, like force otherwise

		// steering stuff
		bool seekingBehaviours = false;
		bool steeringBehaviours = false;
		glm::vec3 seekPoint, steerPoint, gravitatePoint, attractPoint, arrivePoint;
		float seekForce = 0.f;
		float steerForce = 0.f, steerMaxForce = 0.f;
		float gravitatePower = 0.f, gravitateMaxForce = 0.f;
		float attractForce = 0.f, attractRange = 0.f;
		float arriveForce = 0.f, arriveRange = 0.f, arriveMaxForce = 0.0f;

		bool globalEffects = false;
		glm::vec3 globalForceVector = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 globalAccelerationVector = glm::vec3(0.0f, 0.0f, -10.0f);

		///// Initial properties for newly spawned particles //////////////////////////////////////////////////////////////////////
		// these properties are single floats, so we can pack the min and max into a vec2, just data!
		bool limitSpeedOverLifetime = false;
		glm::vec2 initialSpeedLimitRange = glm::vec3(0.0f);
		glm::vec2 finalSpeedLimitRange = glm::vec3(0.0f);

		glm::vec2 initialSpeedRange;

		glm::vec2 lifeRange;

		bool sizeOverLifetime = false;
		glm::vec2 sizeRangeBegin;
		glm::vec2 sizeRangeEnd;
		glm::vec2 massRange;

		bool colourOverLifetime = false;
		glm::vec4 colourBegin0;
		glm::vec4 colourBegin1;

		glm::vec4 colourEnd0;
		glm::vec4 colourEnd1;
	} myConfig;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) 
	{
	
		//graphs
		ar & pathHack;
		ar & sizeHack;
		ar & speedHack;
		ar & colourHack;

		//transformation data
		ar & myState.transform;

		////emitter properties///////////////////////////////////////////////////////////////////////
		ar &myConfig.rotationalVelocity;
		ar &myConfig.emitterOffset;
		ar &myConfig.numberOfParticles;
		ar &myConfig.emissionShape;
		ar &myConfig.emissionRate;

		//cuboid
		ar &myConfig.boxSize; // size of the emitter box

		//sphere
		ar &myConfig.sphereRadius;

		//frustum
		ar &myConfig.frustumRadiusSpawn; //first circle, the part where vectors originate
		ar &myConfig.frustumRadiusTarget; //second circle, the part where vectors head toward
		ar &myConfig.frustumHeight;

		///// Playback properties
		ar &myConfig.playing;
		ar &myConfig.loop;

		ar &myConfig.loopDelay;
		ar &myConfig.duration; //negative duration means infinite

		// Parent toggle
		ar &myConfig.parentTransforms;

		// path following stuff
		ar &myConfig.followPath;
		ar &myConfig.directFollowMode;
		ar &myConfig.lookAhead; // how far ahead particles will seek
		ar &myConfig.pathRadius;
		ar &myConfig.pathPower; //treated like speed in direct mode, like force otherwise

		// steering stuff
		ar &myConfig.seekingBehaviours;
		ar &myConfig.steeringBehaviours;

		ar &myConfig.seekPoint;
		ar &myConfig.seekForce;

		ar &myConfig.steerPoint;
		ar &myConfig.steerForce;
		ar &myConfig.steerMaxForce;

		ar &myConfig.gravitatePoint;
		ar &myConfig.gravitatePower;
		ar &myConfig.gravitateMaxForce;

		ar &myConfig.attractPoint;
		ar &myConfig.attractForce;
		ar &myConfig.attractRange;

		ar &myConfig.arrivePoint;
		ar &myConfig.arriveForce;
		ar &myConfig.arriveRange;
		ar &myConfig.arriveMaxForce;

		ar &myConfig.globalEffects;
		ar &myConfig.globalForceVector;
		ar &myConfig.globalAccelerationVector;

		///// Initial properties for newly spawned particles //////////////////////////////////////////////////////////////////////
		ar &myConfig.initialSpeedRange;

		ar &myConfig.limitSpeedOverLifetime;
		ar &myConfig.initialSpeedLimitRange;
		ar &myConfig.finalSpeedLimitRange;

		// these properties are single floats, so we can pack the min and max into a vec2, just data!
		ar &myConfig.lifeRange;

		ar &myConfig.sizeOverLifetime;
		ar &myConfig.sizeRangeBegin;
		ar &myConfig.sizeRangeEnd;
		ar &myConfig.massRange;

		ar &myConfig.colourOverLifetime;
		ar &myConfig.colourBegin0;
		ar &myConfig.colourBegin1;

		ar &myConfig.colourEnd0;
		ar &myConfig.colourEnd1;
	
	}
};

BOOST_CLASS_VERSION(ParticleEmitter, 1)

class ParticleSystem : public Component //encapsulates an entire visual effect
{
public:
	ParticleSystem();
	~ParticleSystem();

	void update();
	void clearSystem();
	
	void removeAt(size_t index);

	size_t numEmitters();
	void addEmitter(ParticleEmitter* emitter);
	ParticleEmitter* getEmitter(size_t index);
	void removeEmitter();
	ParticleEmitter* lastEmitter();
	std::vector<ParticleEmitter*> m_emitters;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		//graphs
		ar & m_emitters;
	}
private:
};

void drawMat4(const glm::mat4& t, const float& scale = 1.0f);
void drawTransform(const Transform& t, const float& scale = 1.0f);