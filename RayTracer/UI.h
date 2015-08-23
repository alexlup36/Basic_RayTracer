#ifndef UI_H
#define UI_H

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "TGUI/TGUI.hpp"

#include "Scene.h"
#include "Object.h"

#include "PointLight.h"
#include "DirectionalLight.h"
#include "Sphere.h"
#include "Triangle.h"

class UI
{
public:

	enum LightingModel
	{
		Phong = 0,
		BlinnPhong,

		InvalidLightModel,
	};

	UI(Scene* scene,
		unsigned int* reflectionDepth,
		unsigned int* refractionDepth,
		float* moveSpeed,
		int* squareLength,
		int* sampleCount,
		float* sampleDistance,
		bool* updateRequired,
		bool* Realtime,
		bool* ShadowsEnabled,
		bool* SoftShadowsEnabled,
		bool* SuperSamplingEnabled,
		bool* PlaneTexturingEnabled,
		bool* ReflectionEnabled,
		bool* RefractionEnabled,
		LightingModel* m_peLightingModel);

	void SetupUI();
	void ProcessUI();
	void LoadUIElements();

private:

	// Data type

	enum CheckboxType
	{
		Realtime = 0,
		Shadows,
		SoftShadows,
		SuperSampling,
		PlaneTexturing,
		Reflection,
		Refraction,

		InvalidCheckBoxID,
	};

	enum ObjectToAdd
	{
		DirectionalLightObj,
		PointLightObj,
		SphereObj,
		AreaLightObj,
		BoxObj,

		InvalidObjectType,
	};

	// Members
	std::vector<Object*>			m_ObjecList;

	// Reference to the scene object
	Scene* m_pScene;

	// Reference to modifiable values
	float* m_pfMoveSpeed;
	unsigned int* m_uipMaxReflectionDepth;
	unsigned int* m_uipMaxRefractionDepth;

	int* m_piSquareLength;
	int* m_piSampleCount;
	float* m_pfSampleDistance;

	bool* m_bRealtime;
	bool* m_bUpdateRequired;
	bool* m_bShadows;
	bool* m_bSoftShadows;
	bool* m_bSuperSampling;
	bool* m_bPlaneTexturing;
	bool* m_bReflection;
	bool* m_bRefraction;

	LightingModel* m_peLightingModel;

	// UI window size
	unsigned int m_uiWindowWidth = 500;
	unsigned int m_uiWindowHeight = 800;

	// UI properties
	unsigned int SliderPositionAmplitude = 1000;

	sf::RenderWindow m_UIWindow;
	sf::Color m_BackgroundColor;

	// GUI elements
	tgui::Gui m_GUI;

	tgui::Slider::Ptr sliderXPtr = nullptr;
	tgui::Slider::Ptr sliderYPtr = nullptr;
	tgui::Slider::Ptr sliderZPtr = nullptr;

	tgui::ComboBox::Ptr comboBox = nullptr;

	// Callbacks
	void comboBoxSelectionCallback(const tgui::Callback& callback);
	void updateButtonCallback(const tgui::Callback& callback);
	void checkBoxCallback(const tgui::Callback& callback);
	void addObjectCallback(const tgui::Callback& callback);
	void lightModelRadioCallback(const tgui::Callback& callback);

	// Functions
	void Update();

	// Layout parameters 
	float m_fLeftAlign = 10.0f;
	float m_fTopAlign = 10.0f;
	float m_fHorizontalPad = 10.0f;
	float m_fVerticalPad = 10.0f;
	float m_fRowHeight = 15.0f;
	unsigned int m_uiFontSize = 12;
};

#endif // UI_H