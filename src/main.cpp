// Authored By: Tom Tsiliopoulos - 100616336
// Authored By: Joss Moo-Young - 100586602
// Modified By: Shawn Matthews

#define GLEW_STATIC
#include "glew/glew.h"
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "FreeImage.lib")
#pragma comment(lib, "nfd_d.lib")

#include "GameObject.h"
#include "AnimationMath.h"
#include "ParticleEmitter.h"
#include "NodeGrapher.h"
#include "Path.h"

// Core Libraries (std::)
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>
#include <direct.h>
#include <map> // for std::map

// 3rd Party Libraries
#include <GLUT/freeglut.h>
#include <TTK/GraphicsUtils.h>
#include <TTK/Camera.h>
#include <TTK/OBJMesh.h>
#include <nfd/src/include/nfd.h>
#include "imgui/imgui.h"
#include <glm\gtc\random.hpp> // for glm::linearRand

// Defines and Core variables
#define FRAMES_PER_SECOND 60
const int FRAME_DELAY = 1000 / FRAMES_PER_SECOND; // Milliseconds per frame

// Window size
int windowWidth = 1920;
int windowHeight = 1080;

// Angle conversions
const float degToRad = 3.14159f / 180.0f;
const float radToDeg = 180.0f / 3.14159f;

float deltaTime = 0.0f; // amount of time since last update (set every frame in timer callback)

// Mouse position in pixels
glm::vec3 mousePosition; // x, y, 0
glm::vec3 mousePositionFlipped; // x, windowHeight - y, 0
int mouseButton;

GameObject* parentObject = nullptr;
//ParticleEmitter* activeEmitter;
ParticleSystem* activeSystem;

int currentEmitter = 0;

TTK::Camera camera;
algomath::NodeGrapher grapher;

// These values are controlled by imgui
int numParticleScale = 500;

static bool drawTransforms = false;

static bool loadEmitter = false;
static bool saveEmitter = false;
static bool openParticleSystem = false;
static bool openParticleSystemText = false;
static bool saveParticleSystem = false;
static bool saveParticleSystemText = false;
static bool exitEditor = false;
static bool addNewEmitter = false;
static bool addPathNode = false;
static bool removeLastEmitter = false;
static bool resetParticleSystem = false;
static bool displayAbout = false;

glm::vec3 rotation;

std::map<std::string, std::shared_ptr<TTK::OBJMesh>> meshes;
std::string meshPath = "../Assets/Models/";
std::shared_ptr<TTK::OBJMesh> torsoMesh = std::make_shared<TTK::OBJMesh>();
std::shared_ptr<TTK::OBJMesh> legsMesh = std::make_shared<TTK::OBJMesh>();

glm::mat4 worldMatrix;
glm::mat4 scaleMatrix;

#define RANDOM glm::linearRand(0.0f, 1.0f)


void InitializeSystem()
{
	activeSystem->clearSystem();
	ParticleEmitter* emitter = new ParticleEmitter;
	emitter->setNumParticles(100);
	activeSystem->addEmitter(emitter);
	currentEmitter = 0;
}

void addEmitter()
{
	ParticleEmitter* emitter = new ParticleEmitter;
	emitter->setNumParticles(100);
	activeSystem->addEmitter(emitter);
	currentEmitter = activeSystem->numEmitters() - 1;
}

void setUIStyle() {
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 0.8f;
	style.FrameRounding = 3.0f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}

std::string GetProjectDirectory()
{
	char* cwd = _getcwd(0, 0); // **** microsoft specific ****
	std::string working_directory(cwd);
	std::free(cwd);
	return working_directory;
}

/*
 * @description This method loads an emitter system 
 * @method OpenEmitterSystemFile
 * @return {void}
 */
void OpenEmitterSystemFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_OpenDialog("dat", defaultPath, &outPath);

	
	if (result == NFD_OKAY)
	{
		
		std::ifstream emitterSystemFile(outPath, std::ios::in, std::ios::binary);

		boost::archive::text_iarchive ia(emitterSystemFile);

		activeSystem->clearSystem();
		ia >> activeSystem->m_emitters;

		for each (auto emitter in activeSystem->m_emitters)
		{
			emitter->hacksToPaths();
			emitter->setNumParticles(emitter->getNumParticles());
		}

		emitterSystemFile.close();
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

/*
* @description this method saves an emitter system
* @method SaveEmitterSystemFile
* @return {void}
*/
void SaveEmitterSystemFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_SaveDialog("dat", defaultPath, &outPath);

	if (result == NFD_OKAY)
	{

		std::ofstream emitterSystemFile(outPath, std::ios::out, std::ios::binary);

		boost::archive::text_oarchive oa(emitterSystemFile);

		for each (auto emitter in activeSystem->m_emitters)
		{
			emitter->pathsToHacks();
		}

		oa << activeSystem->m_emitters;

		emitterSystemFile.close();
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

/*
* @description This method loads a single emitter
* @method OpenEmitterFile
* @return {void}
*/
void OpenEmitterFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_OpenDialog("dat", defaultPath, &outPath);


	if (result == NFD_OKAY)
	{
		std::ifstream emitterFile(outPath, std::ios::in, std::ios::binary);

		boost::archive::text_iarchive ia(emitterFile);

		ParticleEmitter* emitter = new ParticleEmitter;
		ia >> emitter;

		emitter->hacksToPaths();
		emitter->setNumParticles(emitter->getNumParticles()); //this line might look really, really dumb but its absolutely necessary.

		activeSystem->addEmitter(emitter);
		currentEmitter = activeSystem->numEmitters() - 1;

		emitterFile.close();
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}


/*
* @description this method saves a single emitter
* @method SaveEmitterFile
* @return {void}
*/
void SaveEmitterFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_SaveDialog("dat", defaultPath, &outPath);

	if (result == NFD_OKAY)
	{
		std::ofstream emitterFile(outPath, std::ios::out, std::ios::binary);

		boost::archive::text_oarchive oa(emitterFile);

		activeSystem->getEmitter(currentEmitter)->pathsToHacks();
		oa << activeSystem->getEmitter(currentEmitter);

		emitterFile.close();
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

/*
* @description this method saves an emitter system to a text file
* @method SaveEmitterSystemTextFile
* @return {void}
*/
void SaveEmitterSystemBinaryFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_SaveDialog("pest", defaultPath, &outPath);

	if (result == NFD_OKAY)
	{
		std::ofstream textFile(outPath, std::ios::out, std::ios::binary);
		if (textFile.is_open()) {
			activeSystem->getEmitter(currentEmitter)->myState.path = grapher.getPath();

			int numEmitters = activeSystem->m_emitters.size();
			textFile.write((char*)&numEmitters, sizeof(int));
			for (int ix = 0; ix < activeSystem->m_emitters.size(); ix++) {
				ParticleEmitter* emitter = activeSystem->m_emitters[ix];
				textFile.write(reinterpret_cast<char*>(&emitter->myConfig), sizeof(ParticleEmitter::Config));
				textFile.write(reinterpret_cast<char*>(&emitter->myState.transform), sizeof(Transform));
				emitter->myState.path.Write(textFile);
				emitter->myState.sizeGraph.Write(textFile);
				emitter->myState.speedGraph.Write(textFile);
				emitter->myState.colourGraph.Write(textFile);
			}

			textFile.close();
		}
		
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

/*
* @description This method loads an emitter system from a text file
* @method OpenEmitterSystemTextFile
* @return {void}
*/
void OpenEmitterSystemBinaryFile() {
	const nfdchar_t *defaultPath = NULL;
	nfdchar_t *outPath = NULL;

	std::string currentDirectory = GetProjectDirectory();
	defaultPath = currentDirectory.c_str();

	nfdresult_t result = NFD_OpenDialog("pest", defaultPath, &outPath);


	if (result == NFD_OKAY)
	{

		std::ifstream textFile(outPath, std::ios::in, std::ios::binary);
		if (textFile.is_open()) {
			activeSystem->clearSystem();

			int numEmitters = 0;
			textFile.read((char*)&numEmitters, sizeof(int));
			for (int ix = 0; ix < numEmitters; ix++) {
				addEmitter();
				ParticleEmitter* emitter = activeSystem->m_emitters[ix];
				textFile.read(reinterpret_cast<char*>(&emitter->myConfig), sizeof(ParticleEmitter::Config));
				textFile.read(reinterpret_cast<char*>(&emitter->myState.transform), sizeof(Transform));
				emitter->setNumParticles(emitter->myConfig.numberOfParticles);

				emitter->myState.path.Read(textFile);
				emitter->myState.sizeGraph.Read(textFile);
				emitter->myState.speedGraph.Read(textFile);
				emitter->myState.colourGraph.Read(textFile);
			}

			textFile.close();
		}
		
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
}

void showUI() {
	// You must call this prior to using any imgui functions
	TTK::Graphics::StartUI(windowWidth, windowHeight);
	bool show = true;

	std::string windowString = "Emitter " + std::to_string(currentEmitter);

	if (!ImGui::Begin(&windowString[0], &show, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
	}
	else
	{
		// set window to top left corner
		ImGui::SetWindowPos(ImVec2(0, 20), true);

		// set window colour theme
		setUIStyle();

		// setup main menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("Open Particle System", NULL, &openParticleSystem);
				ImGui::MenuItem("Save Particle System", NULL, &saveParticleSystem);
				ImGui::Separator();
				ImGui::MenuItem("Open Particle System - Text File", NULL, &openParticleSystemText);
				ImGui::MenuItem("Save Particle System - Text File", NULL, &saveParticleSystemText);
				ImGui::Separator();
				ImGui::MenuItem("Load Particle Emitter", NULL, &loadEmitter);
				ImGui::MenuItem("Save Particle Emitter", NULL, &saveEmitter);
				ImGui::Separator();
				ImGui::MenuItem("Exit", NULL, &exitEditor);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Add Emitter", NULL, &addNewEmitter);
				ImGui::MenuItem("Remove Last Emitter", NULL, &removeLastEmitter);
				ImGui::Separator();
				ImGui::MenuItem("Reset", NULL, &resetParticleSystem);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				ImGui::Separator();
				ImGui::MenuItem("About", NULL, &displayAbout);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (openParticleSystem) {
			OpenEmitterSystemFile();
			
			openParticleSystem = false;
		}

		if (saveParticleSystem) {
			SaveEmitterSystemFile();

			saveParticleSystem = false;
		}

		if (openParticleSystemText) {
			OpenEmitterSystemBinaryFile();

			openParticleSystemText = false;
		}

		if (saveParticleSystemText) {
			SaveEmitterSystemBinaryFile();

			saveParticleSystemText = false;
		}


		if (loadEmitter) {
			OpenEmitterFile();

			loadEmitter = false;
		}

		if (saveEmitter) {
			SaveEmitterFile();

			saveEmitter = false;
		}

		if (exitEditor) {
			glutExit();
		}

		if (addNewEmitter) {

			addEmitter();
			addNewEmitter = false;
		}

		if (removeLastEmitter)
		{
			auto size = activeSystem->numEmitters();
			if (size >= 1)
			{
				activeSystem->removeEmitter();

				if (currentEmitter >= activeSystem->numEmitters())
				{
					currentEmitter--;
					if(currentEmitter < 0)
					{
						currentEmitter = 0;
					}
				}
				
			}
			removeLastEmitter = false;
		}



		if (resetParticleSystem) {

			//objects.clear();
			activeSystem->clearSystem();

			currentEmitter = 0;
			InitializeSystem();
			resetParticleSystem = false;
		}

		if (displayAbout) {
			ImGui::Begin("About Particle System Editor", &displayAbout, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Separator();
			ImGui::Text("Authors:");
			ImGui::Text("Tom Tsiliopoulos - 100616336");
			ImGui::Text("Joss Moo-Young   - 100586602");
			ImGui::End();
		}

		if (activeSystem->numEmitters() > 0) {

			// Draw a simple label, this is the same as a "printf" call

			for (int i = 0; i < activeSystem->numEmitters(); i++)
			{
				std::string emitterString = "Emitter " + std::to_string(i);
				if (i > 0)
				{
					ImGui::SameLine();
				}
				if (ImGui::Button(&emitterString[0]))
				{
					currentEmitter = i;
				}
			}
			ParticleEmitter* emitter = activeSystem->getEmitter(currentEmitter);


			// Button, when button is clicked the code in the block is executed
			if (ImGui::Button("Toggle playback"))
			{
				emitter->myConfig.playing = !emitter->myConfig.playing;
			}

			ImGui::Checkbox("Draw transforms", &drawTransforms);

			ImGui::SameLine();

			ImGui::Checkbox("Relative transformations", &emitter->myConfig.parentTransforms);

			ImGui::Separator();

			//****************************************************************************
			if (ImGui::CollapsingHeader("Emitter Transform Options"))
			{
				glm::vec3 position = emitter->myState.transform.getPosition();
				glm::vec3 rotation = emitter->myState.transform.getRotation();

				if (ImGui::DragFloat3("Position", &position.x))
				{
					emitter->myState.transform.setPosition(position);
				}
				if (ImGui::DragFloat3("Rotation", &rotation.x))
				{
					emitter->myState.transform.setRotation(rotation);
				}

				ImGui::DragFloat3("Rotational velocity", &(emitter->myConfig.rotationalVelocity[0]));
			}

			//****************************************************************************
			if (ImGui::CollapsingHeader("Emission Options"))
			{
				numParticleScale = emitter->getNumParticles();
				if (ImGui::DragInt("Max particles", &numParticleScale))
				{
					emitter->setNumParticles(numParticleScale);
				}

				ImGui::DragFloat("Duration", &emitter->myConfig.duration);
				if (ImGui::Checkbox("Loop", &emitter->myConfig.loop))
				{
					emitter->killParticles();
				}
				if (ImGui::DragFloat("Loop delay", &emitter->myConfig.loopDelay))
				{
					emitter->killParticles();
				}
				ImGui::Separator();

				ImGui::DragFloat("Emission rate", &emitter->myConfig.emissionRate);
				if (ImGui::DragFloat2("life range", &(emitter->myConfig.lifeRange[0]), 0.1f, 100.f))
				{
					emitter->killParticles();
				}

				if (ImGui::TreeNode("Emitter Shape Options"))
				{
					if (ImGui::Button("Cube emitter"))
					{
						emitter->myConfig.emissionShape = EMISSION_SHAPE::CUBOID;
					}
					ImGui::Separator();
					ImGui::DragFloat3("Cuboid Emission Dimensions", &(emitter->myConfig.boxSize[0]));
					if (ImGui::Button("Sphere emitter"))
					{
						emitter->myConfig.emissionShape = EMISSION_SHAPE::SPHERE;
					}
					ImGui::Separator();
					ImGui::DragFloat("Sphere Emission radius", &emitter->myConfig.sphereRadius);
					if (ImGui::Button("Frustum emitter"))
					{
						emitter->myConfig.emissionShape = EMISSION_SHAPE::FRUSTUM;
					}
					ImGui::Separator();
					if (ImGui::DragFloat3("Frustum dimensions (radius1, radius2, height)", &emitter->myConfig.frustumRadiusSpawn)) {
						if (emitter->myConfig.frustumRadiusSpawn < 0) {
							emitter->myConfig.frustumRadiusSpawn = 0;
						}
					}
					ImGui::Separator();

					ImGui::DragFloat3("Emission Offset", &(emitter->myConfig.emitterOffset[0]));

					ImGui::TreePop();
				}

			}

			//****************************************************************************
			if (ImGui::CollapsingHeader("Size Options")) {

				ImGui::Checkbox("Size over lifetime", &emitter->myConfig.sizeOverLifetime);
				if (ImGui::Button("Open size graph"))
				{
					grapher.openGraphFile1f(emitter->myState.sizeGraph);
				}
				ImGui::DragFloat2("starting size range", &(emitter->myConfig.sizeRangeBegin[0]));
				ImGui::DragFloat2("ending size range", &(emitter->myConfig.sizeRangeEnd[0]));
			}

			//*******************************************************************
			if (ImGui::CollapsingHeader("Colour Options")) {
				// Color control
				// Tip: You can click and drag the numbers in the UI to change them
				ImGui::Checkbox("Colour over lifetime", &emitter->myConfig.colourOverLifetime);
				if (ImGui::Button("Open colour graph"))
				{
					grapher.openGraphFile1f(emitter->myState.colourGraph);
				}
				ImGui::ColorEdit4("Start Color", &emitter->myConfig.colourBegin0[0]);
				ImGui::ColorEdit4("Start Color Variance", &emitter->myConfig.colourBegin1[0]);
				ImGui::ColorEdit4("End Color", &emitter->myConfig.colourEnd0[0]);
				ImGui::ColorEdit4("End Color Variance", &emitter->myConfig.colourEnd1[0]);
			}

			if (ImGui::CollapsingHeader("Speed Options"))
			{
				ImGui::DragFloat2("Initial Speed Range", &(emitter->myConfig.initialSpeedRange.x));
				ImGui::Separator();

				ImGui::Checkbox("Speed limit over lifetime", &emitter->myConfig.limitSpeedOverLifetime);
				if (ImGui::Button("Open speed graph"))
				{
					grapher.openGraphFile1f(emitter->myState.speedGraph);
				}
				ImGui::DragFloat2("Initial Speed Limit Range", &(emitter->myConfig.initialSpeedLimitRange.x));
				ImGui::DragFloat2("Final Speed Limit Range", &(emitter->myConfig.finalSpeedLimitRange.x));

			}

			//************************************************************************
			if (ImGui::CollapsingHeader("Seeking / Steering Behaviours")) {
				///////////////////// steering behaviours
				ImGui::Checkbox("Seeking behaviours", &emitter->myConfig.seekingBehaviours);

				ImGui::DragFloat3("seekPoint", &(emitter->myConfig.seekPoint.x));
				ImGui::DragFloat("seekForce", &emitter->myConfig.seekForce);
				ImGui::Separator();
				ImGui::DragFloat3("attractPoint", &(emitter->myConfig.attractPoint.x));
				ImGui::DragFloat("attractForce", &emitter->myConfig.attractForce);
				ImGui::DragFloat("attractRange", &emitter->myConfig.attractRange);
				ImGui::Separator();
				ImGui::DragFloat3("gravitatePoint", &(emitter->myConfig.gravitatePoint.x));
				ImGui::DragFloat("gravitatePower", &emitter->myConfig.gravitatePower);
				ImGui::DragFloat("gravitateMaxForce", &emitter->myConfig.gravitateMaxForce);
				ImGui::Separator();

				ImGui::Checkbox("Steering behaviours", &emitter->myConfig.steeringBehaviours);

				ImGui::DragFloat3("steerPoint", &(emitter->myConfig.steerPoint.x));
				ImGui::DragFloat("steerForce", &emitter->myConfig.steerForce);
				ImGui::DragFloat("steerMaxForce", &emitter->myConfig.steerMaxForce);
				ImGui::Separator();
				ImGui::DragFloat3("arrivePoint", &(emitter->myConfig.arrivePoint.x));
				ImGui::DragFloat("arriveForce", &emitter->myConfig.arriveForce);
				ImGui::DragFloat("arriveRange", &emitter->myConfig.arriveRange);
				ImGui::DragFloat("arriveMaxForce", &emitter->myConfig.arriveMaxForce);
				/////////////////////
				ImGui::Separator();
				ImGui::Checkbox("Uniform effects", &emitter->myConfig.globalEffects);
				ImGui::DragFloat3("Force", &(emitter->myConfig.globalForceVector.x));
				ImGui::DragFloat3("Acceleration", &(emitter->myConfig.globalAccelerationVector.x));
				///////////////////////
			}

			//************************************************************************
			if (ImGui::CollapsingHeader("3D Spline Options")) {
				
				auto newNodePosition = activeSystem->getEmitter(currentEmitter)->myState.transform.getPosition();

				if (ImGui::Button("add linear node"))
				{
					grapher.addLinear(newNodePosition);
					grapher.m_nodes.back()->name = "linear";
					grapher.updateTable();
				}
				ImGui::SameLine();
				if (ImGui::Button("add catmull-Rom node"))
				{
					grapher.addCatmullRom(newNodePosition);
					grapher.m_nodes.back()->name = "catmull";
					grapher.updateTable();
				}
				
				if (grapher.m_nodes.size() > 0)
				{
					
					if(ImGui::TreeNode("Path Nodes"))
					{
						for (int i = 0; i < grapher.m_nodes.size(); i++)
						{
							std::string nodeString = "Node " + std::to_string(i) + " {" + grapher.m_nodes[i]->name + "}";
							if (ImGui::DragFloat3(&nodeString[0], &grapher.m_nodes[i]->value.x))
							{
								grapher.updateTable();
							}
						}

						if(ImGui::Button("Remove Last Node"))
						{
							grapher.pop_back();
							if(grapher.m_nodes.size() > 1)
							{
								grapher.updateTable();
							}
						}

						ImGui::TreePop();
					}
				}

				if (emitter->myState.path.m_data.size() >= 1) {
					//***********************************************************************
					if (ImGui::TreeNode("Path Options")) {
						///////////////////// path following
						ImGui::Checkbox("Follow Path", &emitter->myConfig.followPath);

						ImGui::SameLine();
						ImGui::Checkbox("Hard positional follow", &emitter->myConfig.directFollowMode);
						if (ImGui::DragFloat("LookAhead", &emitter->myConfig.lookAhead)) {
							if (emitter->myConfig.lookAhead < 0) {
								emitter->myConfig.lookAhead = 0;
							}
						}
						if (ImGui::DragFloat("speed", &emitter->myConfig.pathPower))
						{
							if (emitter->myConfig.pathPower < 0)
							{
								emitter->myConfig.pathPower = 0;
							}
						}

						if (ImGui::DragFloat("pathRadius", &emitter->myConfig.pathRadius)) {
							if (emitter->myConfig.pathRadius < 0) {
								emitter->myConfig.pathRadius = 0;
							}
						}
						/////////////////////
						ImGui::TreePop();
					}
				}
								
				ImGui::Separator();
				if (ImGui::Button("open path"))
				{
					grapher.openGraphFile3f();
				}

				if (grapher.m_nodes.size() > 0)
				{
					ImGui::SameLine();
					if (ImGui::Button("save path"))
					{
						grapher.saveGraphFile3f();
					}
				}

				if (grapher.m_table.m_data.size() >= 1) {
					ImGui::SameLine();
					if (ImGui::Button("assign path"))
					{
						emitter->myState.path = grapher.getPath();
					}

					ImGui::SameLine();
					if (ImGui::Button("clear path")) {
						grapher.clearNodes();
						grapher.m_table.m_data.clear();
					}
				}
			}
		}

		//static bool drawWindow = true;
		//ImGui::ShowTestWindow(&drawWindow);

		// You must call this once you are done doing UI stuff
		// This is what actually draws the ui on screen
		ImGui::End();
	}

	TTK::Graphics::EndUI();
}

// This is where we draw stuff
void Update(void)
{
	// Set up scene
	TTK::Graphics::SetBackgroundColour(0.5f, 0.5f, 0.5f);
	TTK::Graphics::ClearScreen();
	TTK::Graphics::SetCameraMode3D(0, 0, windowWidth, windowHeight);
	//TTK::Graphics::SetCameraMode2D(int(windowWidth * 0.5), int(windowHeight * 0.5), int(windowWidth * 0.5), int(windowHeight * 0.5));
	camera.update();
	TTK::Graphics::DrawGrid(5.0f);

	// show mech
	//torsoMesh->setAllColours(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	//legsMesh->setAllColours(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

	legsMesh->draw(worldMatrix);
	torsoMesh->draw(worldMatrix);
	

	parentObject->update();
	activeSystem->update();

	grapher.draw();

	showUI();
	
	ParticleEmitter* emitter;

	if(activeSystem->numEmitters() > 0)
	{
		 emitter = activeSystem->getEmitter(currentEmitter);
	}
	if (drawTransforms)
	{
		for (auto e : activeSystem->m_emitters)
		{
			drawTransform(e->myState.transform); // draw all emitter frames
		}
		drawMat4(parentObject->transformable->getTransform(), 6.0f); // draw parent gameobject

		// draw selected emitter
		TTK::Graphics::DrawSphere(emitter->myState.transform.getTransform(), 0.4f, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f));
		drawTransform(emitter->myState.transform, 2.2f);
	}

	// Swap buffers
	// This is how we tell the program to put the things we just drew on the screen
	glutSwapBuffers();
}

/* function void KeyboardCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is pressed
*/
void KeyboardCallbackFunction(unsigned char key, int x, int y)
{
	static bool moveGameObject = false;
	ParticleEmitter* emitter = activeSystem->getEmitter(currentEmitter);

	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = true;
	io.AddInputCharacter((int)key); // this is what makes keyboard input work in imgui
	switch (key)
	{
	case 27: // the escape key
		glutExit();
		break;
	case 'O':
	case 'o':
		moveGameObject = !moveGameObject;
		break;
	}

	if (!moveGameObject)
	{
		switch (key)
		{
		case 'w':
		case 'W':
			emitter->myState.transform.moveY(10.0f);

			break;
		case 'a':
		case 'A':
			emitter->myState.transform.moveX(-10.0f);
			break;
		case 's':
		case 'S':
			emitter->myState.transform.moveY(-10.0f);
			
			break;
		case 'd':
		case 'D':
			emitter->myState.transform.moveX(10.0f);
			break;
		case 'e':
		case 'E':
			emitter->myState.transform.moveZ(-10.0f);
			break;
		case 'q':
		case 'Q':
			emitter->myState.transform.moveZ(10.0f);
			break;
		}
	}
	else
	{
		switch (key)
		{
		case 'w':
		case 'W':
			parentObject->transformable->moveZ(-1.0f);
			break;
		case 'a':
		case 'A':
			parentObject->transformable->moveX(-1.0f);
			break;
		case 's':
		case 'S':
			parentObject->transformable->moveZ(1.0f);
			break;
		case 'd':
		case 'D':
			parentObject->transformable->moveX(1.0f);
			break;
		case 'e':
		case 'E':
			parentObject->transformable->moveY(1.0f);
			break;
		case 'q':
		case 'Q':
			parentObject->transformable->moveY(-1.0f);
			break;
		}
	}
}

/* function void KeyboardUpCallbackFunction(unsigned char, int,int)
* Description:
*   - this handles keyboard input when a button is lifted
*/
void KeyboardUpCallbackFunction(unsigned char key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.KeysDown[key] = false;

	switch (key)
	{
	default:
		break;
	}
}

/* function TimerCallbackFunction(int value)
* Description:
*  - this is called many times per second
*  - this enables you to animate things
*  - no drawing, just changing the state
*  - changes the frame number and calls for a redisplay
*  - FRAME_DELAY is the number of milliseconds to wait before calling the timer again
*/
void TimerCallbackFunction(int value)
{
	// Calculate the amount of time since the last frame
	static int elapsedTimeAtLastTick = 0;
	int totalElapsedTime = glutGet(GLUT_ELAPSED_TIME);

	deltaTime = (float)totalElapsedTime - elapsedTimeAtLastTick;
	deltaTime /= 1000.0f;
	elapsedTimeAtLastTick = totalElapsedTime;

	// Re-trigger the display event
	glutPostRedisplay();

	/* this call gives it a proper frame delay to hit our target FPS */
	glutTimerFunc(FRAME_DELAY, TimerCallbackFunction, 0);
}

/* function WindowReshapeCallbackFunction()
* Description:
*  - this is called whenever the window is resized
*  - and sets up the projection matrix properly
*/
void WindowReshapeCallbackFunction(int w, int h)
{
	/* Update our Window Properties */
	windowWidth = w;
	windowHeight = h;
}

// This is called when a mouse button is clicked
void MouseClickCallbackFunction(int button, int state, int x, int y)
{
	mouseButton = button;

	ImGui::GetIO().MouseDown[0] = !state;

	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;

	if (mouseButton == 3) {
		camera.moveForward();
	}

	if (mouseButton == 4) {
		camera.moveBackward();
	}
}

void SpecialInputCallbackFunction(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		camera.moveUp();
		break;
	case GLUT_KEY_DOWN:
		camera.moveDown();
		break;
	case GLUT_KEY_LEFT:
		camera.moveRight();
		break;
	case GLUT_KEY_RIGHT:
		camera.moveLeft();
		break;
	}
}

// Called when the mouse is clicked and moves
void MouseMotionCallbackFunction(int x, int y)
{
	ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);

	if (mousePosition.length() > 0) {
		if (mouseButton == 1) {
			camera.processMouseMotion(x, y, mousePosition.x, mousePosition.y, deltaTime);
		}
	}


	mousePosition.x = x;
	mousePosition.y = y;

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;
}

// Called when the mouse is moved inside the window
void MousePassiveMotionCallbackFunction(int x, int y)
{
	ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);

	mousePositionFlipped.x = x;
	mousePositionFlipped.y = windowHeight - y;

	//emitter.emitterPosition = mousePositionFlipped;
	//emitter.emitterPosition = glm::vec3(0.0f, 0.0f, 0.0f);
}

/* function main()
* Description:
*  - this is the main function
*  - does initialization and then calls glutMainLoop() to start the event handler
*/

//[STAThreadAttribute]
int main(int argc, char **argv)
{
	/* initialize the window and OpenGL properly */

	// Request an OpenGL 4.4 compatibility
	// A compatibility context is needed to use the provided rendering utilities
	// which are written in OpenGL 1.1
	glutInitContextVersion(4, 4);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Particle System Editor");

	//Init GLEW
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "TTK::InitializeTTK Error: GLEW failed to init" << std::endl;
	}
	printf("OpenGL version: %s, GLSL version: %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	/* set up our function callbacks */
	glutDisplayFunc(Update);
	glutKeyboardFunc(KeyboardCallbackFunction);
	glutKeyboardUpFunc(KeyboardUpCallbackFunction);
	glutReshapeFunc(WindowReshapeCallbackFunction);
	glutMouseFunc(MouseClickCallbackFunction);
	glutMotionFunc(MouseMotionCallbackFunction);
	glutPassiveMotionFunc(MousePassiveMotionCallbackFunction);
	glutTimerFunc(1, TimerCallbackFunction, 0);
	glutSpecialFunc(SpecialInputCallbackFunction);

	// Init GL
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	Transformable* trans = new Transformable();
	activeSystem = new ParticleSystem();
	parentObject = new GameObject({ (Component*)trans, (Component*)activeSystem });

	torsoMesh->loadMesh(meshPath + "torso.obj");
	legsMesh->loadMesh(meshPath + "legs.obj");
	meshes["torso"] = torsoMesh;
	meshes["legs"] = legsMesh;

	worldMatrix = glm::translate(glm::vec3(300.0f, 300.0f, 0.0f));

	scaleMatrix = glm::scale(glm::vec3(5.5f, 5.5f, 5.5f));

	worldMatrix *= scaleMatrix;




	for (size_t i = 0; i < meshes["torso"]->vertices.size(); i++)
	{
		
		meshes["torso"]->colours.push_back(glm::vec4(RANDOM, 0.1f, 0.1f, 1.0f));
	}

	for (size_t i = 0; i < meshes["legs"]->vertices.size(); i++)
	{
		meshes["legs"]->colours.push_back(glm::vec4(0.1f, 0.1f, RANDOM, 1.0f));
	}

	

	/* Start Game Loop */
	deltaTime = (float)glutGet(GLUT_ELAPSED_TIME);
	deltaTime /= 1000.0f;
	InitializeSystem();

	// Init IMGUI
	TTK::Graphics::InitImGUI();

	glutMainLoop();

	return 0;
	delete parentObject;
}