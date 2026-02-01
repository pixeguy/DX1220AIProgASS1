#include "SceneBase.h"
#include "GL\glew.h"

#include "shader.hpp"
#include "MeshBuilder.h"
#include "Application.h"
#include "Utility.h"
#include "LoadTGA.h"
#include <sstream>

static const int fontWidth[] = { 0,26,26,26,26,26,26,26,26,26,26,26,26,0,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,26,12,17,21,26,26,37,35,11,16,16,26,26,13,16,13,20,26,26,26,26,26,26,26,26,26,26,14,14,26,26,26,24,46,30,28,28,32,25,24,33,32,13,17,27,22,44,34,34,27,35,28,24,25,33,30,46,27,25,24,16,20,16,26,26,15,25,27,22,27,26,16,24,27,12,12,24,12,42,27,27,27,27,18,20,17,27,23,37,23,24,21,16,24,16,26,26,26,26,13,16,22,36,26,26,21,54,24,18,45,26,24,26,26,13,13,22,22,26,26,47,23,37,20,18,44,26,21,25,12,17,26,26,26,26,26,26,20,43,21,27,26,16,26,20,18,26,17,17,15,29,30,13,16,13,22,27,33,35,35,24,30,30,30,30,30,30,40,28,25,25,25,25,13,13,13,13,32,34,34,34,34,34,34,26,35,33,33,33,33,25,27,27,25,25,25,25,25,25,40,22,26,26,26,26,12,12,12,12,27,27,27,27,27,27,27,26,28,27,27,27,27,24,27,24 };

SceneBase::SceneBase()
{
}

SceneBase::~SceneBase()
{
}

void SceneBase::Init()
{
	// Black background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	
	glEnable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);

	m_programID = LoadShaders( "Shader//comg.vertexshader", "Shader//comg.fragmentshader" );
	
	// Get a handle for our uniform
	m_parameters[U_MVP] = glGetUniformLocation(m_programID, "MVP");
	//m_parameters[U_MODEL] = glGetUniformLocation(m_programID, "M");
	//m_parameters[U_VIEW] = glGetUniformLocation(m_programID, "V");
	m_parameters[U_MODELVIEW] = glGetUniformLocation(m_programID, "MV");
	m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE] = glGetUniformLocation(m_programID, "MV_inverse_transpose");
	m_parameters[U_MATERIAL_AMBIENT] = glGetUniformLocation(m_programID, "material.kAmbient");
	m_parameters[U_MATERIAL_DIFFUSE] = glGetUniformLocation(m_programID, "material.kDiffuse");
	m_parameters[U_MATERIAL_SPECULAR] = glGetUniformLocation(m_programID, "material.kSpecular");
	m_parameters[U_MATERIAL_SHININESS] = glGetUniformLocation(m_programID, "material.kShininess");
	m_parameters[U_LIGHTENABLED] = glGetUniformLocation(m_programID, "lightEnabled");
	m_parameters[U_NUMLIGHTS] = glGetUniformLocation(m_programID, "numLights");
	m_parameters[U_LIGHT0_TYPE] = glGetUniformLocation(m_programID, "lights[0].type");
	m_parameters[U_LIGHT0_POSITION] = glGetUniformLocation(m_programID, "lights[0].position_cameraspace");
	m_parameters[U_LIGHT0_COLOR] = glGetUniformLocation(m_programID, "lights[0].color");
	m_parameters[U_LIGHT0_POWER] = glGetUniformLocation(m_programID, "lights[0].power");
	m_parameters[U_LIGHT0_KC] = glGetUniformLocation(m_programID, "lights[0].kC");
	m_parameters[U_LIGHT0_KL] = glGetUniformLocation(m_programID, "lights[0].kL");
	m_parameters[U_LIGHT0_KQ] = glGetUniformLocation(m_programID, "lights[0].kQ");
	m_parameters[U_LIGHT0_SPOTDIRECTION] = glGetUniformLocation(m_programID, "lights[0].spotDirection");
	m_parameters[U_LIGHT0_COSCUTOFF] = glGetUniformLocation(m_programID, "lights[0].cosCutoff");
	m_parameters[U_LIGHT0_COSINNER] = glGetUniformLocation(m_programID, "lights[0].cosInner");
	m_parameters[U_LIGHT0_EXPONENT] = glGetUniformLocation(m_programID, "lights[0].exponent");
	// Get a handle for our "colorTexture" uniform
	m_parameters[U_COLOR_TEXTURE_ENABLED] = glGetUniformLocation(m_programID, "colorTextureEnabled");
	m_parameters[U_COLOR_TEXTURE] = glGetUniformLocation(m_programID, "colorTexture");
	// Get a handle for our "textColor" uniform
	m_parameters[U_TEXT_ENABLED] = glGetUniformLocation(m_programID, "textEnabled");
	m_parameters[U_TEXT_COLOR] = glGetUniformLocation(m_programID, "textColor");
	
	// Use our shader
	glUseProgram(m_programID);

	lights[0].type = Light::LIGHT_DIRECTIONAL;
	lights[0].position.Set(0, 20, 0);
	lights[0].color.Set(1, 1, 1);
	lights[0].power = 1;
	lights[0].kC = 1.f;
	lights[0].kL = 0.01f;
	lights[0].kQ = 0.001f;
	lights[0].cosCutoff = cos(Math::DegreeToRadian(45));
	lights[0].cosInner = cos(Math::DegreeToRadian(30));
	lights[0].exponent = 3.f;
	lights[0].spotDirection.Set(0.f, 1.f, 0.f);
	
	glUniform1i(m_parameters[U_NUMLIGHTS], 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);

	glUniform1i(m_parameters[U_LIGHT0_TYPE], lights[0].type);
	glUniform3fv(m_parameters[U_LIGHT0_COLOR], 1, &lights[0].color.r);
	glUniform1f(m_parameters[U_LIGHT0_POWER], lights[0].power);
	glUniform1f(m_parameters[U_LIGHT0_KC], lights[0].kC);
	glUniform1f(m_parameters[U_LIGHT0_KL], lights[0].kL);
	glUniform1f(m_parameters[U_LIGHT0_KQ], lights[0].kQ);
	glUniform1f(m_parameters[U_LIGHT0_COSCUTOFF], lights[0].cosCutoff);
	glUniform1f(m_parameters[U_LIGHT0_COSINNER], lights[0].cosInner);
	glUniform1f(m_parameters[U_LIGHT0_EXPONENT], lights[0].exponent);

	camera.Init(Vector3(0, 0, 1), Vector3(0, 0, 0), Vector3(0, 1, 0));

	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		meshList[i] = NULL;
	}
	meshList[GEO_AXES] = MeshBuilder::GenerateAxes("reference", 1000, 1000, 1000);
	meshList[GEO_BALL] = MeshBuilder::GenerateSphere("ball", Color(1, 0, 0), 10, 10, 1.f);
	meshList[GEO_CUBE] = MeshBuilder::GenerateCube("cube", Color(0, 1, 0), 2.f);
	meshList[GEO_MAXCUBE] = MeshBuilder::GenerateCube("cube", Color(1,0,0), 2.f);
	meshList[GEO_ENERGYCUBE] = MeshBuilder::GenerateCube("cube", Color(0, 0, 1), 2.f);
	meshList[GEO_MAXENERGYCUBE] = MeshBuilder::GenerateCube("cube", Color(0.5, 0.5, 0.5), 2.f);
	meshList[GEO_MAXCUBE] = MeshBuilder::GenerateCube("cube", Color(1, 0, 0), 2.f);
	meshList[GEO_LINE] = MeshBuilder::GenerateLine("line", 1.f, Color(1, 0, 0));
	meshList[GEO_TEXT] = MeshBuilder::GenerateText("text", 16, 16);
	meshList[GEO_TEXT]->textureID = LoadTGA("Image//calibri.tga");
	meshList[GEO_BG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1));
	meshList[GEO_BG]->textureID = LoadTGA("Image//grid20.tga");
	//meshList[GEO_TICTACTOE] = MeshBuilder::GenerateQuad("tictactoe", Color(1, 1, 1));
	//meshList[GEO_TICTACTOE]->textureID = LoadTGA("Image//tictactoe.tga");
	//meshList[GEO_CROSS] = MeshBuilder::GenerateQuad("cross", Color(1, 1, 1));
	//meshList[GEO_CROSS]->textureID = LoadTGA("Image//cross.tga");
	meshList[GEO_CIRCLE] = MeshBuilder::GenerateQuad("circle", Color(1, 1, 1));
	meshList[GEO_CIRCLE]->textureID = LoadTGA("Image//circle.tga");
	//meshList[GEO_TOOFULL] = MeshBuilder::GenerateQuad("toofull", Color(1, 1, 1));
	//meshList[GEO_TOOFULL]->textureID = LoadTGA("Image//toofull.tga");
	//meshList[GEO_FULL] = MeshBuilder::GenerateQuad("full", Color(1, 1, 1));
	//meshList[GEO_FULL]->textureID = LoadTGA("Image//full.tga");
	//meshList[GEO_HUNGRY] = MeshBuilder::GenerateQuad("hungry", Color(1, 1, 1));
	//meshList[GEO_HUNGRY]->textureID = LoadTGA("Image//hungry.tga");
	//meshList[GEO_DEAD] = MeshBuilder::GenerateQuad("dead", Color(1, 1, 1));
	//meshList[GEO_DEAD]->textureID = LoadTGA("Image//dead.tga");
	//meshList[GEO_SHARK] = MeshBuilder::GenerateQuad("shark", Color(1, 1, 1));
	//meshList[GEO_SHARK]->textureID = LoadTGA("Image//shark.tga");
	//meshList[GEO_FISHFOOD] = MeshBuilder::GenerateQuad("fishfood", Color(1, 1, 1));
	//meshList[GEO_FISHFOOD]->textureID = LoadTGA("Image//fishfood.tga");
	//meshList[GEO_CRAZY] = MeshBuilder::GenerateQuad("crazy", Color(1, 1, 1));
	//meshList[GEO_CRAZY]->textureID = LoadTGA("Image//crazy.tga");
	//meshList[GEO_HAPPY] = MeshBuilder::GenerateQuad("happy", Color(1, 1, 1));
	//meshList[GEO_HAPPY]->textureID = LoadTGA("Image//happy.tga");
	meshList[GEO_BASEBLUE] = MeshBuilder::GenerateQuad("baseblue", Color(1, 1, 1));
	meshList[GEO_BASEBLUE]->textureID = LoadTGA("Image//BaseBlue.tga");
	meshList[GEO_BASERED] = MeshBuilder::GenerateQuad("basered", Color(1, 1, 1));
	meshList[GEO_BASERED]->textureID = LoadTGA("Image//BaseRed.tga");
	meshList[GEO_SPAWNERBLUE] = MeshBuilder::GenerateQuad("spawnerblue", Color(1, 1, 1));
	meshList[GEO_SPAWNERBLUE]->textureID = LoadTGA("Image//BlueSpawnerReversed.tga");
	meshList[GEO_SPAWNERRED] = MeshBuilder::GenerateQuad("spawnerred", Color(1, 1, 1));
	meshList[GEO_SPAWNERRED]->textureID = LoadTGA("Image//RedSpawnerReversed.tga");
	{
		//meshList[GEO_ATTACKER] = MeshBuilder::GenerateQuad("attacker", Color(1, 1, 1));
		//meshList[GEO_ATTACKER]->textureID = LoadTGA("Image//Attacker1Healthy.tga");
		//meshList[GEO_RANGED] = MeshBuilder::GenerateQuad("ranged", Color(1, 1, 1));
		//meshList[GEO_RANGED]->textureID = LoadTGA("Image//RangedFaceRight.tga");
		//meshList[GEO_RANGEDRIGHT] = MeshBuilder::GenerateQuad("ranged", Color(1, 1, 1));
		//meshList[GEO_RANGEDRIGHT]->textureID = LoadTGA("Image//RangedFaceLeft.tga");
		//meshList[GEO_SUPPORT] = MeshBuilder::GenerateQuad("support", Color(1, 1, 1));
		//meshList[GEO_SUPPORT]->textureID = LoadTGA("Image//hungry.tga");
		//meshList[GEO_MECHANIC] = MeshBuilder::GenerateQuad("support", Color(0, 1, 1));
		//meshList[GEO_MECHANIC]->textureID = LoadTGA("Image//hungry.tga");
		//meshList[GEO_TANK] = MeshBuilder::GenerateQuad("support", Color(0, 1, 1));
		//meshList[GEO_TANK]->textureID = LoadTGA("Image//hungry.tga");
		//meshList[GEO_MORTAR] = MeshBuilder::GenerateQuad("support", Color(0, 1, 1));
		//meshList[GEO_MORTAR]->textureID = LoadTGA("Image//hungry.tga");
		//meshList[GEO_MORBOMB] = MeshBuilder::GenerateQuad("support", Color(0, 1, 1));
		//meshList[GEO_MORBOMB]->textureID = LoadTGA("Image//hungry.tga");
	}
	{
		// Attacker 1
		meshList[GEO_ATTACKER1_FLEEING] = MeshBuilder::GenerateQuad("Attacker1Fleeing", Color(1, 1, 1));
		meshList[GEO_ATTACKER1_FLEEING]->textureID = LoadTGA("Image//Attacker1Fleeing.tga");

		meshList[GEO_ATTACKER1_STAY_STRONG] = MeshBuilder::GenerateQuad("Attacker1StayStrong", Color(1, 1, 1));
		meshList[GEO_ATTACKER1_STAY_STRONG]->textureID = LoadTGA("Image//Attacker1StayStrong.tga");

		meshList[GEO_ATTACKER1_NEAR_DEATH] = MeshBuilder::GenerateQuad("Attacker1NearDeath", Color(1, 1, 1));
		meshList[GEO_ATTACKER1_NEAR_DEATH]->textureID = LoadTGA("Image//Attacker1NearDeath.tga");

		meshList[GEO_ATTACKER1_HEALTHY] = MeshBuilder::GenerateQuad("Attacker1Healthy", Color(1, 1, 1));
		meshList[GEO_ATTACKER1_HEALTHY]->textureID = LoadTGA("Image//Attacker1Healthy.tga");

		meshList[GEO_ATTACKER1_HELPING] = MeshBuilder::GenerateQuad("Attacker1Helping", Color(1, 1, 1));
		meshList[GEO_ATTACKER1_HELPING]->textureID = LoadTGA("Image//Attacker1Helping.tga");

		// Attacker 2
		meshList[GEO_ATTACKER2_HEALTHY] = MeshBuilder::GenerateQuad("Attacker2Healthy", Color(1, 1, 1));
		meshList[GEO_ATTACKER2_HEALTHY]->textureID = LoadTGA("Image//Attacker2Healthy.tga");

		meshList[GEO_ATTACKER2_FLEEING] = MeshBuilder::GenerateQuad("Attacker2Fleeing", Color(1, 1, 1));
		meshList[GEO_ATTACKER2_FLEEING]->textureID = LoadTGA("Image//Attacker2Fleeing.tga");

		meshList[GEO_ATTACKER2_STAY_STRONG] = MeshBuilder::GenerateQuad("Attacker2StayStrong", Color(1, 1, 1));
		meshList[GEO_ATTACKER2_STAY_STRONG]->textureID = LoadTGA("Image//Attacker2StayStrong.tga");

		meshList[GEO_ATTACKER2_HELPING] = MeshBuilder::GenerateQuad("Attacker2Helping", Color(1, 1, 1));
		meshList[GEO_ATTACKER2_HELPING]->textureID = LoadTGA("Image//Attacker2Helping.tga");

		meshList[GEO_ATTACKER2_NEAR_DEATH] = MeshBuilder::GenerateQuad("Attacker2NearDeath", Color(1, 1, 1));
		meshList[GEO_ATTACKER2_NEAR_DEATH]->textureID = LoadTGA("Image//Attacker2NearDeath.tga");


		// Range 1
		meshList[GEO_RANGE1_NEAR_DEATH] = MeshBuilder::GenerateQuad("Range1NearDeath", Color(1, 1, 1));
		meshList[GEO_RANGE1_NEAR_DEATH]->textureID = LoadTGA("Image//Range1NearDeath.tga");

		meshList[GEO_RANGE1_HURT] = MeshBuilder::GenerateQuad("Range1Hurt", Color(1, 1, 1));
		meshList[GEO_RANGE1_HURT]->textureID = LoadTGA("Image//Range1Hurt.tga");

		meshList[GEO_RANGE1_PANIC] = MeshBuilder::GenerateQuad("Range1Panic", Color(1, 1, 1));
		meshList[GEO_RANGE1_PANIC]->textureID = LoadTGA("Image//Range1Panic.tga");

		meshList[GEO_RANGE1_HEALTHY] = MeshBuilder::GenerateQuad("Range1Healthy", Color(1, 1, 1));
		meshList[GEO_RANGE1_HEALTHY]->textureID = LoadTGA("Image//Range1Healthy.tga");

		// Range 2
		meshList[GEO_RANGE2_NEAR_DEATH] = MeshBuilder::GenerateQuad("Range2NearDeath", Color(1, 1, 1));
		meshList[GEO_RANGE2_NEAR_DEATH]->textureID = LoadTGA("Image//Range2NearDeath.tga");

		meshList[GEO_RANGE2_PANIC] = MeshBuilder::GenerateQuad("Range2Panic", Color(1, 1, 1));
		meshList[GEO_RANGE2_PANIC]->textureID = LoadTGA("Image//Range2Panic.tga");

		meshList[GEO_RANGE2_HEALTHY] = MeshBuilder::GenerateQuad("Range2Healthy", Color(1, 1, 1));
		meshList[GEO_RANGE2_HEALTHY]->textureID = LoadTGA("Image//Range2Healthy.tga");

		meshList[GEO_RANGE2_HURT] = MeshBuilder::GenerateQuad("Range2Hurt", Color(1, 1, 1));
		meshList[GEO_RANGE2_HURT]->textureID = LoadTGA("Image//Range2Hurt.tga");


		// Support 1
		meshList[GEO_SUPPORT1_HEALING] = MeshBuilder::GenerateQuad("Support1Healing", Color(1, 1, 1));
		meshList[GEO_SUPPORT1_HEALING]->textureID = LoadTGA("Image//Support1Healing.tga");

		meshList[GEO_SUPPORT1_HIDING] = MeshBuilder::GenerateQuad("Support1Hiding", Color(1, 1, 1));
		meshList[GEO_SUPPORT1_HIDING]->textureID = LoadTGA("Image//Support1Hiding.tga");

		meshList[GEO_SUPPORT1_HEALTHY] = MeshBuilder::GenerateQuad("Support1Healthy", Color(1, 1, 1));
		meshList[GEO_SUPPORT1_HEALTHY]->textureID = LoadTGA("Image//Support1Healthy.tga");

		meshList[GEO_SUPPORT1_HURT] = MeshBuilder::GenerateQuad("Support1Hurt", Color(1, 1, 1));
		meshList[GEO_SUPPORT1_HURT]->textureID = LoadTGA("Image//Support1Hurt.tga");

		meshList[GEO_SUPPORT1_URGENT_HEALING] = MeshBuilder::GenerateQuad("Support1UrgentHealing", Color(1, 1, 1));
		meshList[GEO_SUPPORT1_URGENT_HEALING]->textureID = LoadTGA("Image//Support1UrgentHealing.tga");

		// Support 2
		meshList[GEO_SUPPORT2_HIDING] = MeshBuilder::GenerateQuad("Support2Hiding", Color(1, 1, 1));
		meshList[GEO_SUPPORT2_HIDING]->textureID = LoadTGA("Image//Support2Hiding.tga");

		meshList[GEO_SUPPORT2_URGENT_HEALING] = MeshBuilder::GenerateQuad("Support2UrgentHealing", Color(1, 1, 1));
		meshList[GEO_SUPPORT2_URGENT_HEALING]->textureID = LoadTGA("Image//Support2UrgentHealing.tga");

		meshList[GEO_SUPPORT2_HEALING] = MeshBuilder::GenerateQuad("Support2Healing", Color(1, 1, 1));
		meshList[GEO_SUPPORT2_HEALING]->textureID = LoadTGA("Image//Support2Healing.tga");

		meshList[GEO_SUPPORT2_HURT] = MeshBuilder::GenerateQuad("Support2Hurt", Color(1, 1, 1));
		meshList[GEO_SUPPORT2_HURT]->textureID = LoadTGA("Image//Support2Hurt.tga");

		meshList[GEO_SUPPORT2_HEALTHY] = MeshBuilder::GenerateQuad("Support2Healthy", Color(1, 1, 1));
		meshList[GEO_SUPPORT2_HEALTHY]->textureID = LoadTGA("Image//Support2Healthy.tga");


		// Mechanic 1
		meshList[GEO_MECHANIC1_GOLD] = MeshBuilder::GenerateQuad("Mechanic1Gold", Color(1, 1, 1));
		meshList[GEO_MECHANIC1_GOLD]->textureID = LoadTGA("Image//Mechanic1Gold.tga");

		meshList[GEO_MECHANIC1_HIDING] = MeshBuilder::GenerateQuad("Mechanic1Hiding", Color(1, 1, 1));
		meshList[GEO_MECHANIC1_HIDING]->textureID = LoadTGA("Image//Mechanic1Hiding.tga");

		meshList[GEO_MECHANIC1_HEALTHY] = MeshBuilder::GenerateQuad("Mechanic1Healthy", Color(1, 1, 1));
		meshList[GEO_MECHANIC1_HEALTHY]->textureID = LoadTGA("Image//Mechanic1Healthy.tga");

		meshList[GEO_MECHANIC1_HURT] = MeshBuilder::GenerateQuad("Mechanic1Hurt", Color(1, 1, 1));
		meshList[GEO_MECHANIC1_HURT]->textureID = LoadTGA("Image//Mechanic1Hurt.tga");

		meshList[GEO_MECHANIC1_BUILDING] = MeshBuilder::GenerateQuad("Mechanic1Building", Color(1, 1, 1));
		meshList[GEO_MECHANIC1_BUILDING]->textureID = LoadTGA("Image//Mechanic1Building.tga");

		// Mechanic 2
		meshList[GEO_MECHANIC2_HEALTHY] = MeshBuilder::GenerateQuad("Mechanic2Healthy", Color(1, 1, 1));
		meshList[GEO_MECHANIC2_HEALTHY]->textureID = LoadTGA("Image//Mechanic2Healthy.tga");

		meshList[GEO_MECHANIC2_GOLD] = MeshBuilder::GenerateQuad("Mechanic2Gold", Color(1, 1, 1));
		meshList[GEO_MECHANIC2_GOLD]->textureID = LoadTGA("Image//Mechanic2Gold.tga");

		meshList[GEO_MECHANIC2_BUILDING] = MeshBuilder::GenerateQuad("Mechanic2Building", Color(1, 1, 1));
		meshList[GEO_MECHANIC2_BUILDING]->textureID = LoadTGA("Image//Mechanic2Building.tga");

		meshList[GEO_MECHANIC2_HURT] = MeshBuilder::GenerateQuad("Mechanic2Hurt", Color(1, 1, 1));
		meshList[GEO_MECHANIC2_HURT]->textureID = LoadTGA("Image//Mechanic2Hurt.tga");

		meshList[GEO_MECHANIC2_HIDING] = MeshBuilder::GenerateQuad("Mechanic2Hiding", Color(1, 1, 1));
		meshList[GEO_MECHANIC2_HIDING]->textureID = LoadTGA("Image//Mechanic2Hiding.tga");


		// Tanks
		meshList[GEO_TANK1_PANIC] = MeshBuilder::GenerateQuad("Tank1Panic", Color(1, 1, 1));
		meshList[GEO_TANK1_PANIC]->textureID = LoadTGA("Image//Tank1Panic.tga");

		meshList[GEO_TANK1_HEALTHY] = MeshBuilder::GenerateQuad("Tank1Healthy", Color(1, 1, 1));
		meshList[GEO_TANK1_HEALTHY]->textureID = LoadTGA("Image//Tank1Healthy.tga");

		meshList[GEO_TANK1_BOMB] = MeshBuilder::GenerateQuad("Tank1Bomb", Color(1, 1, 1));
		meshList[GEO_TANK1_BOMB]->textureID = LoadTGA("Image//Tank1Bomb.tga");

		meshList[GEO_TANK2_BOMB] = MeshBuilder::GenerateQuad("Tank2Bomb", Color(1, 1, 1));
		meshList[GEO_TANK2_BOMB]->textureID = LoadTGA("Image//Tank2Bomb.tga");

		meshList[GEO_TANK2_HEALTHY] = MeshBuilder::GenerateQuad("Tank2Healthy", Color(1, 1, 1));
		meshList[GEO_TANK2_HEALTHY]->textureID = LoadTGA("Image//Tank2Healthy.tga");

		meshList[GEO_TANK2_PANIC] = MeshBuilder::GenerateQuad("Tank2Panic", Color(1, 1, 1));
		meshList[GEO_TANK2_PANIC]->textureID = LoadTGA("Image//Tank2Panic.tga");


		// Mortars
		meshList[GEO_MORTAR1_HEALTHY] = MeshBuilder::GenerateQuad("Mortar1Healthy", Color(1, 1, 1));
		meshList[GEO_MORTAR1_HEALTHY]->textureID = LoadTGA("Image//Mortar1Healthy.tga");

		meshList[GEO_MORTAR1_PANIC] = MeshBuilder::GenerateQuad("Mortar1Panic", Color(1, 1, 1));
		meshList[GEO_MORTAR1_PANIC]->textureID = LoadTGA("Image//Mortar1Panic.tga");

		meshList[GEO_MORTAR2_HEALTHY] = MeshBuilder::GenerateQuad("Mortar2Healthy", Color(1, 1, 1));
		meshList[GEO_MORTAR2_HEALTHY]->textureID = LoadTGA("Image//Mortar2Healthy.tga");

		meshList[GEO_MORTAR2_PANIC] = MeshBuilder::GenerateQuad("Mortar2Panic", Color(1, 1, 1));
		meshList[GEO_MORTAR2_PANIC]->textureID = LoadTGA("Image//Mortar2Panic.tga");


		// Golden Orb
		meshList[GEO_GOLDENORB] = MeshBuilder::GenerateQuad("GoldenOrb", Color(1, 1, 1));
		meshList[GEO_GOLDENORB]->textureID = LoadTGA("Image//GoldenOrb.tga");

		//Bullets
		meshList[GEO_BULLET1] = MeshBuilder::GenerateQuad("Bullet1", Color(1, 1, 1));
		meshList[GEO_BULLET1]->textureID = LoadTGA("Image//Bullet1.tga");

		meshList[GEO_BULLET2] = MeshBuilder::GenerateQuad("Bullet2", Color(1, 1, 1));
		meshList[GEO_BULLET2]->textureID = LoadTGA("Image//Bullet2.tga");

		meshList[GEO_ORE] = MeshBuilder::GenerateQuad("Ore", Color(1, 1, 1));
		meshList[GEO_ORE]->textureID = LoadTGA("Image//DiamondOre.tga");

		meshList[GEO_LOG] = MeshBuilder::GenerateQuad("Log", Color(1, 1, 1));
		meshList[GEO_LOG]->textureID = LoadTGA("Image//WoodenLog.tga");

		meshList[GEO_GRASS] = MeshBuilder::GenerateQuad("grass", Color(1, 1, 1));
		meshList[GEO_GRASS]->textureID = LoadTGA("Image//grassTile.tga");

		meshList[GEO_SNOW] = MeshBuilder::GenerateQuad("snow", Color(1, 1, 1));
		meshList[GEO_SNOW]->textureID = LoadTGA("Image//snowTile.tga");

		meshList[GEO_SAND] = MeshBuilder::GenerateQuad("sand", Color(1, 1, 1));
		meshList[GEO_SAND]->textureID = LoadTGA("Image//sandTile.tga");

		meshList[GEO_WATER] = MeshBuilder::GenerateQuad("water", Color(1, 1, 1));
		meshList[GEO_WATER]->textureID = LoadTGA("Image/waterTile.tga");

		meshList[GEO_ICE] = MeshBuilder::GenerateQuad("iceTile", Color(1, 1, 1));
		meshList[GEO_ICE]->textureID = LoadTGA("Image//iceTile.tga");

		meshList[GEO_LAVA] = MeshBuilder::GenerateQuad("lava", Color(1, 1, 1));
		meshList[GEO_LAVA]->textureID = LoadTGA("Image//lavaTile.tga");

		meshList[GEO_MAGMA] = MeshBuilder::GenerateQuad("magma", Color(1, 1, 1));
		meshList[GEO_MAGMA]->textureID = LoadTGA("Image//magmaTile.tga");

		meshList[GEO_WALL] = MeshBuilder::GenerateQuad("wall", Color(1, 1, 1));
		meshList[GEO_WALL]->textureID = LoadTGA("Image//wallTile.tga");

		meshList[GEO_FOG] = MeshBuilder::GenerateQuad("fog", Color(1, 1, 1));
		meshList[GEO_FOG]->textureID = LoadTGA("Image//fogTile.tga");
	}
	meshList[GEO_TEXT]->material.kAmbient.Set(1, 0, 0);

	//week 6 - make chessboard mesh/texture pair
	meshList[GEO_CHESSBOARD] = MeshBuilder::GenerateQuad("chessboard", Color(1, 1, 1));
	meshList[GEO_CHESSBOARD]->textureID = LoadTGA("Image//chessboard.tga");
	meshList[GEO_QUEEN] = MeshBuilder::GenerateQuad("queen", Color(1, 1, 1));
	meshList[GEO_QUEEN]->textureID = LoadTGA("Image//queen.tga");
	meshList[GEO_KNIGHT] = MeshBuilder::GenerateQuad("knight", Color(1, 1, 1));
	meshList[GEO_KNIGHT]->textureID = LoadTGA("Image//knight.tga");

	//week 7
	meshList[GEO_WHITEQUAD] = MeshBuilder::GenerateQuad("whitequad", Color(1, 1, 1));
	meshList[GEO_FLOOR] = MeshBuilder::GenerateQuad("floor", Color(1, 1, 1));
	meshList[GEO_FLOOR]->textureID = LoadTGA("Image//floor.tga");
	meshList[GEO_AGENT] = MeshBuilder::GenerateQuad("agent", Color(1, 1, 1));
	meshList[GEO_AGENT]->textureID = LoadTGA("Image//agent.tga");
	meshList[GEO_WAYPOINT] = MeshBuilder::GenerateQuad("waypoint", Color(1, 1, 1));
	meshList[GEO_WAYPOINT]->textureID = LoadTGA("Image//waypoint.tga");

	//week 13
	meshList[GEO_WHITEHEX] = MeshBuilder::GenerateRing("whitehex", Color(1, 1, 1), 6, 0.5f, 0.f);

	//week 14
	meshList[GEO_REVERSIBOARD4x4] = MeshBuilder::GenerateQuad("reversiboard4x4", Color(1, 1, 1));
	meshList[GEO_REVERSIBOARD4x4]->textureID = LoadTGA("Image//reversi4x4.tga");
	meshList[GEO_REVERSIBOARD] = MeshBuilder::GenerateQuad("reversiboard", Color(1, 1, 1));
	meshList[GEO_REVERSIBOARD]->textureID = LoadTGA("Image//reversi.tga");
	meshList[GEO_REVERSIBLACK] = MeshBuilder::GenerateQuad("reversiblack", Color(1, 1, 1));
	meshList[GEO_REVERSIBLACK]->textureID = LoadTGA("Image//black.tga");
	meshList[GEO_REVERSIWHITE] = MeshBuilder::GenerateQuad("reversiwhite", Color(1, 1, 1));
	meshList[GEO_REVERSIWHITE]->textureID = LoadTGA("Image//white.tga");

	//week 16
	meshList[GEO_FLAPPYBG] = MeshBuilder::GenerateQuad("bg", Color(1, 1, 1));
	meshList[GEO_FLAPPYBG]->textureID = LoadTGA("Image//background-day.tga");
	meshList[GEO_SIDEBAR] = MeshBuilder::GenerateQuad("blackquad", Color(0, 0, 0));
	meshList[GEO_CHARACTER] = MeshBuilder::GenerateQuad("bird", Color(1, 1, 1));
	meshList[GEO_CHARACTER]->textureID = LoadTGA("Image//yellowbird-downflap.tga");
	meshList[GEO_PIPE] = MeshBuilder::GenerateQuad("pipe", Color(1, 1, 1));
	meshList[GEO_PIPE]->textureID = LoadTGA("Image//pipe-green.tga");

	bLightEnabled = false;
}

void SceneBase::Update(double dt)
{
	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	fps = (float)(1.f / dt);
}

void SceneBase::RenderText(Mesh* mesh, std::string text, Color color)
{
	if(!mesh || mesh->textureID <= 0)
		return;
	
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	float accum = 0;
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(accum, 0, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	
		mesh->Render((unsigned)text[i] * 6, 6);
		accum += (float)fontWidth[(unsigned)text[i]] / 64;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderTextOnScreen(Mesh* mesh, std::string text, Color color, float size, float x, float y)
{
	if(!mesh || mesh->textureID <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	Mtx44 ortho;
	ortho.SetToOrtho(0, 80, 0, 60, -10, 10);
	projectionStack.PushMatrix();
	projectionStack.LoadMatrix(ortho);
	viewStack.PushMatrix();
	viewStack.LoadIdentity();
	modelStack.PushMatrix();
	modelStack.LoadIdentity();
	modelStack.Translate(x, y, 0);
	modelStack.Scale(size, size, size);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 1);
	glUniform3fv(m_parameters[U_TEXT_COLOR], 1, &color.r);
	glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	float accum = 0;
	for(unsigned i = 0; i < text.length(); ++i)
	{
		Mtx44 characterSpacing;
		characterSpacing.SetToTranslation(accum + 0.5f, 0.5f, 0); //1.0f is the spacing of each character, you may change this value
		Mtx44 MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top() * characterSpacing;
		glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);

		mesh->Render((unsigned)text[i] * 6, 6);

		accum += (float)fontWidth[(unsigned)text[i]] / 64;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(m_parameters[U_TEXT_ENABLED], 0);
	modelStack.PopMatrix();
	viewStack.PopMatrix();
	projectionStack.PopMatrix();
	glEnable(GL_DEPTH_TEST);
}

void SceneBase::RenderMesh(Mesh *mesh, bool enableLight)
{
	Mtx44 MVP, modelView, modelView_inverse_transpose;
	
	MVP = projectionStack.Top() * viewStack.Top() * modelStack.Top();
	glUniformMatrix4fv(m_parameters[U_MVP], 1, GL_FALSE, &MVP.a[0]);
	if(enableLight && bLightEnabled)
	{
		glUniform1i(m_parameters[U_LIGHTENABLED], 1);
		modelView = viewStack.Top() * modelStack.Top();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW], 1, GL_FALSE, &modelView.a[0]);
		modelView_inverse_transpose = modelView.GetInverse().GetTranspose();
		glUniformMatrix4fv(m_parameters[U_MODELVIEW_INVERSE_TRANSPOSE], 1, GL_FALSE, &modelView.a[0]);
		
		//load material
		glUniform3fv(m_parameters[U_MATERIAL_AMBIENT], 1, &mesh->material.kAmbient.r);
		glUniform3fv(m_parameters[U_MATERIAL_DIFFUSE], 1, &mesh->material.kDiffuse.r);
		glUniform3fv(m_parameters[U_MATERIAL_SPECULAR], 1, &mesh->material.kSpecular.r);
		glUniform1f(m_parameters[U_MATERIAL_SHININESS], mesh->material.kShininess);
	}
	else
	{	
		glUniform1i(m_parameters[U_LIGHTENABLED], 0);
	}
	if(mesh->textureID > 0)
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
		glUniform1i(m_parameters[U_COLOR_TEXTURE], 0);
	}
	else
	{
		glUniform1i(m_parameters[U_COLOR_TEXTURE_ENABLED], 0);
	}
	mesh->Render();
	if(mesh->textureID > 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SceneBase::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SceneBase::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
}
