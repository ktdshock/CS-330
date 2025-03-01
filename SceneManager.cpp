///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
	m_objectMaterials.clear();
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/***********************************************************
 *  DefineObjectMaterials()
 *  Sets up material properties for objects in the scene.
 ***********************************************************/

void SceneManager::DefineObjectMaterials() {

	// Lamp Body (Metallic Grey)
	OBJECT_MATERIAL lampBody;
	lampBody.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);  // Darker grey
	lampBody.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);   // Medium grey
	lampBody.specularColor = glm::vec3(0.8f, 0.8f, 0.8f);  // Reflective metallic
	lampBody.ambientStrength = 0.2f;  // Reduce ambient influence
	lampBody.shininess = 64.0f; // High shine for a realistic metallic effect
	lampBody.tag = "lampBody";
	m_objectMaterials.push_back(lampBody);

	// Knob & Upper Base (Brass/Gold)
	OBJECT_MATERIAL lampKnob;
	lampKnob.ambientColor = glm::vec3(0.5f, 0.3f, 0.1f);  // Warm brass
	lampKnob.diffuseColor = glm::vec3(0.7f, 0.5f, 0.2f);  // Lighter brass
	lampKnob.specularColor = glm::vec3(0.9f, 0.8f, 0.6f); // Slightly glossy brass
	lampKnob.ambientStrength = 0.2f;  // Reduce ambient influence
	lampKnob.shininess = 32.0f; // Slightly lower shine than metallic grey
	lampKnob.tag = "lampKnob";
	m_objectMaterials.push_back(lampKnob);

	// Cup (Dark Green, High Gloss)
	OBJECT_MATERIAL cupMaterial;
	cupMaterial.ambientColor = glm::vec3(0.05f, 0.2f, 0.05f);  // Deep dark green ambient
	cupMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);  // Rich green diffuse
	cupMaterial.specularColor = glm::vec3(0.8f, 1.0f, 0.8f); // High reflection for gloss
	cupMaterial.ambientStrength = 0.2f;
	cupMaterial.shininess = 128.0f;  // Very high for glossy effect
	cupMaterial.tag = "cup";
	m_objectMaterials.push_back(cupMaterial);

	// Pencil (Yellow)
	OBJECT_MATERIAL pencilMaterial;
	pencilMaterial.ambientColor = glm::vec3(1.0f, 1.0f, 0.0f);
	pencilMaterial.diffuseColor = glm::vec3(0.9f, 0.8f, 0.1f);
	pencilMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.1f);
	pencilMaterial.ambientStrength = 0.3f;
	pencilMaterial.shininess = 16.0f;
	pencilMaterial.tag = "pencil";
	m_objectMaterials.push_back(pencilMaterial);

	// Monitor Screen (Black Matte)
	OBJECT_MATERIAL monitorMaterial;
	monitorMaterial.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
	monitorMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
	monitorMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	monitorMaterial.ambientStrength = 0.1f;
	monitorMaterial.shininess = 10.0f;
	monitorMaterial.tag = "monitor";
	m_objectMaterials.push_back(monitorMaterial);

	// Monitor Stand (Metallic Grey)
	OBJECT_MATERIAL monitorStandMaterial;
	monitorStandMaterial.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	monitorStandMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	monitorStandMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.7f);
	monitorStandMaterial.ambientStrength = 0.2f;
	monitorStandMaterial.shininess = 40.0f;
	monitorStandMaterial.tag = "monitorStand";
	m_objectMaterials.push_back(monitorStandMaterial);

	// Book (Red Cover)
	OBJECT_MATERIAL bookMaterial;
	bookMaterial.ambientColor = glm::vec3(0.0f, 0.0f, 0.6f); // Deep red
	bookMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 0.8f); // Brighter red
	bookMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f); // Slight shine
	bookMaterial.ambientStrength = 0.2f;
	bookMaterial.shininess = 20.0f;
	bookMaterial.tag = "bluebook";
	m_objectMaterials.push_back(bookMaterial);
}

void SceneManager::SetupSceneLights()
{
	// Enable lighting in the shaders
	m_pShaderManager->setBoolValue("bUseLighting", true);

	// Increase overall ambient light slightly to make scene feel more natural
	glm::vec3 globalAmbientLight = glm::vec3(0.15f, 0.15f, 0.15f);
	m_pShaderManager->setVec3Value("globalAmbient", globalAmbientLight);

	// Left Desk Light - Positioned left, angled slightly outward
	glm::vec3 lightPosition1 = glm::vec3(10.0f, 12.0f, -10.0f);
	glm::vec3 lightDirection1 = glm::normalize(glm::vec3(0.3f, -1.0f, 0.2f));

	glm::vec3 ambientLight1 = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 diffuseLight1 = glm::vec3(0.85f, 0.85f, 0.85f);
	glm::vec3 specularLight1 = glm::vec3(0.5f, 0.5f, 0.5f); // Lowered specular to reduce glare

	m_pShaderManager->setVec3Value("lightSources[0].position", lightPosition1);
	m_pShaderManager->setVec3Value("lightSources[0].direction", lightDirection1);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", ambientLight1);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", diffuseLight1);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", specularLight1);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 40.0f); // Softer spread
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 30.0f);

	// Right Desk Light - Positioned right, angled slightly outward
	glm::vec3 lightPosition2 = glm::vec3(20.0f, 12.0f, -10.0f);
	glm::vec3 lightDirection2 = glm::normalize(glm::vec3(-0.3f, -1.0f, 0.2f));

	glm::vec3 ambientLight2 = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::vec3 diffuseLight2 = glm::vec3(0.85f, 0.85f, 0.85f);
	glm::vec3 specularLight2 = glm::vec3(0.5f, 0.5f, 0.5f); // Lowered specular to reduce glare

	m_pShaderManager->setVec3Value("lightSources[1].position", lightPosition2);
	m_pShaderManager->setVec3Value("lightSources[1].direction", lightDirection2);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", ambientLight2);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", diffuseLight2);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", specularLight2);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 40.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 30.0f);

	// Optional: Soft Overhead Light (acts as indirect room light)
	glm::vec3 overheadLightPos = glm::vec3(15.0f, 18.0f, -15.0f);
	glm::vec3 overheadLightColor = glm::vec3(0.4f, 0.4f, 0.4f);

	m_pShaderManager->setVec3Value("lightSources[2].position", overheadLightPos);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", overheadLightColor);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 50.0f); // Very soft room fill
}

void SceneManager::LoadSceneTextures() {

	CreateGLTexture("../../Utilities/textures/knife_handle.jpg", "woodTexture");

	CreateGLTexture("../../Utilities/textures/book.jpg", "backDrop");

	CreateGLTexture("../../Utilities/textures/monitorscreen.jpg", "monScreen");

	CreateGLTexture("../../Utilities/textures/pckeyboard.jpg", "pcKey");

	CreateGLTexture("../../Utilities/textures/stainless_end.jpg", "penCup");

	CreateGLTexture("../../Utilities/textures/circular-brushed-gold-texture.jpg", "lampGold");

	CreateGLTexture("../../Utilities/textures/donut_tex.jpg", "donutTex");

}

void SceneManager::PrepareScene()
{
	DefineObjectMaterials(); // Define material properties
	SetupSceneLights();      // Configure lighting
	LoadSceneTextures(); // Load the scene texture

	// Load necessary meshes
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadTorusMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene() {
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f, YrotationDegrees = 0.0f, ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Enable texture usage
	m_pShaderManager->setIntValue("bUseTexture", true);
	m_pShaderManager->setIntValue("bUseLighting", true);

	/*** Floor Plane (Wooden Desk) ***/
	scaleXYZ = glm::vec3(20.0f, 1.0f, 6.0f); // Desk surface size
	positionXYZ = glm::vec3(0.0f, -5.0f, 0.0f); // Under the lamp
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("woodTexture");

	// Explicitly bind the texture before drawing (important fix)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("woodTexture"));

	// Draw the plane mesh
	m_basicMeshes->DrawPlaneMesh();

	/*** Book (Red Cover) ***/
	scaleXYZ = glm::vec3(6.0f, 1.0f, 5.0f); // Book size
	positionXYZ = glm::vec3(12.0f, -4.5f, -0.5f); // To the right of the keyboard
	YrotationDegrees = -30.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("backDrop");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("backDrop"));

	m_basicMeshes->DrawBoxMesh();

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	/*** Monitor Screen (Black) ***/
	scaleXYZ = glm::vec3(12.0f, 8.0f, 0.4f);
	positionXYZ = glm::vec3(0.0f, 2.0f, -1.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("monscreen");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("monScreen"));

	m_basicMeshes->DrawBoxMesh();

	/*** Keyboard ***/
	scaleXYZ = glm::vec3(8.0f, 0.5f, 3.0f); // Keyboard size
	positionXYZ = glm::vec3(0.0f, -4.8f, 3.0f); // Position in front of monitor
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("pckey");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("pcKey"));

	m_basicMeshes->DrawBoxMesh();

	/*** Cup ***/
	scaleXYZ = glm::vec3(1.5f, 3.0f, 1.5f);  // Cup size
	positionXYZ = glm::vec3(-16.0f, -5.0f, 4.0f); // Back left of desk
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("pencup");  // Apply cup material (Dark Green)

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("penCup"));

	m_basicMeshes->DrawCylinderMesh();

	/*** Lamp Base (Grey) ***/
	scaleXYZ = glm::vec3(3.0f, 1.0f, 3.0f);
	positionXYZ = glm::vec3(-15.0f, -5.0f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("lampgold");  // Use material, NOT texture

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("lampGold"));

	m_basicMeshes->DrawCylinderMesh();

	/*** Upper Base (Brass/Gold) ***/
	scaleXYZ = glm::vec3(-2.0f, 0.5f, 2.0f);
	positionXYZ = glm::vec3(-15.0f, -4.0, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("pencup");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("penCup"));

	m_basicMeshes->DrawSphereMesh();

	/*** Lamp Pole***/
	scaleXYZ = glm::vec3(0.3f, 7.0f, 0.3f);
	positionXYZ = glm::vec3(-15.0f, -4.0f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("lampgold");

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("lampGold"));

	m_basicMeshes->DrawCylinderMesh();

	/*** Lamp Head ***/
	scaleXYZ = glm::vec3(1.5f, 4.0f, 1.5f);
	positionXYZ = glm::vec3(-14.0f, 2.0f, 0.5f);
	XrotationDegrees = -45.0f;
	YrotationDegrees = 360.0f;
	ZrotationDegrees = 25.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("lampgold");

	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("lampGold"));

	m_basicMeshes->DrawCylinderMesh();

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	/*** A Delicious Donut ***/
	scaleXYZ = glm::vec3(1.0f, 1.0f, 2.0f);
	positionXYZ = glm::vec3(-8.0f, -4.5f, -1.0f);
	XrotationDegrees = 90.0f;
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("donuttex");

	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, FindTextureID("donutTex"));

	m_basicMeshes->DrawTorusMesh();

	// Reset to non-textured mode for other objects
	m_pShaderManager->setIntValue("bUseTexture", false);

	/*** Decorative Top Section (Brass/Gold) ***/
	scaleXYZ = glm::vec3(0.5f, 1.0f, 0.5f);
	positionXYZ = glm::vec3(-15.8f, 5.5f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("lampKnob");
	m_basicMeshes->DrawSphereMesh();

	/*** Lamp Bulb (Glowing White) ***/
	scaleXYZ = glm::vec3(0.8f, 0.8f, 0.8f);
	positionXYZ = glm::vec3(-14.0f, 2.0f, 0.5f);
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 0.9f, 1.0f); // Soft white glow
	m_basicMeshes->DrawSphereMesh();

	float pencilHeight = 3.5f;

	XrotationDegrees = 0.0f;
	YrotationDegrees = 50.0f;
	ZrotationDegrees = 10.0f;

	/*** Pencil 1 (Yellow) ***/
	scaleXYZ = glm::vec3(0.2f, pencilHeight, 0.2f);
	positionXYZ = glm::vec3(-16.0f, -3.5f, 4.0f); // Inside cup, slightly left
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("pencil");
	m_basicMeshes->DrawCylinderMesh();

	XrotationDegrees = 0.0f;
	YrotationDegrees = 80.0f;
	ZrotationDegrees = 10.0f;

	/*** Pencil 2 (Yellow, Slightly Tilted) ***/
	scaleXYZ = glm::vec3(0.2f, pencilHeight, 0.2f);
	positionXYZ = glm::vec3(-16.0f, -3.5f, 4.0f); // Inside cup, slightly right
	XrotationDegrees = -15.0f; // Small tilt
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("pencil");
	m_basicMeshes->DrawCylinderMesh();

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	/*** Pencil 3 (Yellow, Slightly Tilted) ***/
	scaleXYZ = glm::vec3(0.2f, pencilHeight, 0.2f);
	positionXYZ = glm::vec3(16.0f, -4.8f, 4.0f); // Inside cup, slightly right
	XrotationDegrees = -90.0f; // Small tilt
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("pencil");
	m_basicMeshes->DrawCylinderMesh();

	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	/*** Monitor Stand Base ***/
	scaleXYZ = glm::vec3(6.0f, 1.0f, 4.0f);
	positionXYZ = glm::vec3(0.0f, -4.5f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("monitorStand");
	m_basicMeshes->DrawBoxMesh();

	/*** Monitor Stand Adjust ***/
	scaleXYZ = glm::vec3(1.0f, 6.0f, 1.0f);
	positionXYZ = glm::vec3(0.0f, -2.5f, -2.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("monitor");
	m_basicMeshes->DrawBoxMesh();

	/*** Mouse ***/
	scaleXYZ = glm::vec3(1.0f, 0.5f, 1.0f); // Mouse size
	positionXYZ = glm::vec3(6.0f, -4.8f, 3.2f); // To the right of the keyboard
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderMaterial("monitor");
	m_basicMeshes->DrawBoxMesh();
}