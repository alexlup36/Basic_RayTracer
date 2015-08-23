#include "UI.h"

// ----------------------------------------------------------------------------

UI::UI(Scene* scene,
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
	LightingModel* LightModelCalculation)
	: m_pScene(scene),
	m_uipMaxReflectionDepth(reflectionDepth),
	m_uipMaxRefractionDepth(refractionDepth),
	m_pfMoveSpeed(moveSpeed),
	m_piSquareLength(squareLength),
	m_piSampleCount(sampleCount),
	m_pfSampleDistance(sampleDistance),
	m_bUpdateRequired(updateRequired), 
	m_bRealtime(Realtime),
	m_bShadows(ShadowsEnabled), 
	m_bSoftShadows(SoftShadowsEnabled), 
	m_bSuperSampling(SuperSamplingEnabled),
	m_bPlaneTexturing(PlaneTexturingEnabled),
	m_bReflection(ReflectionEnabled),
	m_bRefraction(RefractionEnabled),
	m_peLightingModel(LightModelCalculation)
{
	m_BackgroundColor = sf::Color(0, 0, 0, 255);

	// Get the list of objects in the scene
	m_ObjecList = m_pScene->ObjectList();
}

// ----------------------------------------------------------------------------

void UI::SetupUI()
{
	// Create the window for the UI
	m_UIWindow.create(sf::VideoMode(m_uiWindowWidth, m_uiWindowHeight), "RayTracer", sf::Style::Titlebar);

	// Associate the GUI with the current window
	m_GUI.setWindow(m_UIWindow);

	// Load font
	bool bSuccess = m_GUI.setGlobalFont("Lib\\TGUI\\fonts\\DejaVuSans.ttf");

	if (!bSuccess)
	{
		std::cout << "Error loading TGUI font." << std::endl;
	}

	LoadUIElements();
}

// ----------------------------------------------------------------------------

void UI::ProcessUI()
{
	// Create and setup the window
	SetupUI();

	while (m_UIWindow.isOpen())
	{
		sf::Event event;
		while (m_UIWindow.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				m_UIWindow.close();

			// Send the events to the GUI object
			m_GUI.handleEvent(event);
		}

		Update();

		m_UIWindow.clear(m_BackgroundColor);

		// Draw all created widgets
		m_GUI.draw();

		m_UIWindow.display();		
	}
}

// ----------------------------------------------------------------------------

void UI::LoadUIElements()
{
	// ------------------------------------------------------------------------

	// Setup the background 
	tgui::Picture::Ptr background(m_GUI);
	bool bSuccess = background->load("background.png");
	background->setSize((float)m_uiWindowWidth, (float)m_uiWindowHeight);
	background->setPosition(0, 0);

	// ------------------------------------------------------------------------

	// Real-time checkbox
	tgui::Checkbox::Ptr realtimeCheckbox(m_GUI);
	realtimeCheckbox->load("lib//TGUI//widgets//Black.conf");
	realtimeCheckbox->setPosition(m_fLeftAlign, m_fTopAlign);
	realtimeCheckbox->setText("Real-time");
	realtimeCheckbox->setSize(m_fRowHeight, m_fRowHeight);
	realtimeCheckbox->setCallbackId(CheckboxType::Realtime);
	if (*m_bRealtime) { realtimeCheckbox->check(); }
	realtimeCheckbox->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// ------------------------------------------------------------------------

	// Shadows checkbox
	tgui::Checkbox::Ptr shadowsEnable(m_GUI);
	shadowsEnable->load("lib//TGUI//widgets//Black.conf");
	shadowsEnable->setPosition(m_fLeftAlign, m_fTopAlign + m_fVerticalPad + m_fRowHeight);
	shadowsEnable->setText("Shadows");
	shadowsEnable->setSize(m_fRowHeight, m_fRowHeight);
	shadowsEnable->setCallbackId(CheckboxType::Shadows);
	if (*m_bShadows) { shadowsEnable->check(); }
	shadowsEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// ------------------------------------------------------------------------

	// Soft shadows checkbox
	tgui::Checkbox::Ptr softshadowsEnable(m_GUI);
	softshadowsEnable->load("lib//TGUI//widgets//Black.conf");
	softshadowsEnable->setPosition(m_fLeftAlign, m_fTopAlign + 2.0f * (m_fVerticalPad + m_fRowHeight));
	softshadowsEnable->setText("Soft shadows");
	softshadowsEnable->setSize(m_fRowHeight, m_fRowHeight);
	softshadowsEnable->setCallbackId(CheckboxType::SoftShadows);
	if (*m_bSoftShadows) { softshadowsEnable->check(); }
	softshadowsEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// Setup the soft shadow sample count label
	tgui::Label::Ptr softShadowSampleLabel(m_GUI);
	softShadowSampleLabel->setText("SampleCount");
	softShadowSampleLabel->setPosition(140, m_fTopAlign + 2.0f * (m_fVerticalPad + m_fRowHeight));
	softShadowSampleLabel->setSize(100, 25);
	softShadowSampleLabel->setAutoSize(true);
	softShadowSampleLabel->setTextSize(12);

	// Setup the soft shadow sample count edit box
	tgui::EditBox::Ptr softShadowSampleEditBoxX(m_GUI, "SoftShadowSampleEditBoxX");
	softShadowSampleEditBoxX->load("lib//TGUI//widgets//Black.conf");
	softShadowSampleEditBoxX->setSize(40, m_fRowHeight);
	softShadowSampleEditBoxX->setPosition(250, m_fTopAlign + 2.0f * (m_fVerticalPad + m_fRowHeight));

	tgui::EditBox::Ptr softShadowSampleEditBoxZ(m_GUI, "SoftShadowSampleEditBoxZ");
	softShadowSampleEditBoxZ->load("lib//TGUI//widgets//Black.conf");
	softShadowSampleEditBoxZ->setSize(40, m_fRowHeight);
	softShadowSampleEditBoxZ->setPosition(250 + 40 + m_fHorizontalPad, m_fTopAlign + 2.0f * (m_fVerticalPad + m_fRowHeight));

	// ------------------------------------------------------------------------

	// Super sampling checkbox
	tgui::Checkbox::Ptr supersamplingEnable(m_GUI);
	supersamplingEnable->load("lib//TGUI//widgets//Black.conf");
	supersamplingEnable->setPosition(m_fLeftAlign, m_fTopAlign + 3.0f * (m_fVerticalPad + m_fRowHeight));
	supersamplingEnable->setText("Super sampling");
	supersamplingEnable->setSize(m_fRowHeight, m_fRowHeight);
	supersamplingEnable->setCallbackId(CheckboxType::SuperSampling);
	if (*m_bSuperSampling) { supersamplingEnable->check(); }
	supersamplingEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// Setup the super sampling count label
	tgui::Label::Ptr superSamplingCountLabel(m_GUI);
	superSamplingCountLabel->setText("SampleCount");
	superSamplingCountLabel->setPosition(140, 87);
	superSamplingCountLabel->setSize(100, 25);
	superSamplingCountLabel->setAutoSize(true);
	superSamplingCountLabel->setTextSize(12);

	// Setup the super sampling count edit box
	tgui::EditBox::Ptr superSamplingEditBox(m_GUI, "SuperSamplingCountEditBox");
	superSamplingEditBox->load("lib//TGUI//widgets//Black.conf");
	superSamplingEditBox->setSize(40, m_fRowHeight);
	superSamplingEditBox->setPosition(250, 87);

	// ------------------------------------------------------------------------

	// Plane texturing checkbox
	tgui::Checkbox::Ptr planeTexturingEnable(m_GUI);
	planeTexturingEnable->load("lib//TGUI//widgets//Black.conf");
	planeTexturingEnable->setPosition(m_fLeftAlign, m_fTopAlign + 4.0f * (m_fVerticalPad + m_fRowHeight));
	planeTexturingEnable->setText("Plane texturing");
	planeTexturingEnable->setSize(m_fRowHeight, m_fRowHeight);
	planeTexturingEnable->setCallbackId(CheckboxType::PlaneTexturing);
	if (*m_bPlaneTexturing) { planeTexturingEnable->check(); }
	planeTexturingEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// Setup the plane texturing square length label
	tgui::Label::Ptr squareLengthLabel(m_GUI);
	squareLengthLabel->setText("SquareLength");
	squareLengthLabel->setPosition(140, 112);
	squareLengthLabel->setSize(100, 25);
	squareLengthLabel->setAutoSize(true);
	squareLengthLabel->setTextSize(12);

	// Setup the plane texturing square length edit box
	tgui::EditBox::Ptr squareLengthEditBox(m_GUI, "SquareLengthEditBox");
	squareLengthEditBox->load("lib//TGUI//widgets//Black.conf");
	squareLengthEditBox->setSize(40, m_fRowHeight);
	squareLengthEditBox->setPosition(250, 112);

	// ------------------------------------------------------------------------

	// Reflection checkbox
	tgui::Checkbox::Ptr reflectionEnable(m_GUI);
	reflectionEnable->load("lib//TGUI//widgets//Black.conf");
	reflectionEnable->setPosition(m_fLeftAlign, m_fTopAlign + 5.0f * (m_fVerticalPad + m_fRowHeight));
	reflectionEnable->setText("Reflection");
	reflectionEnable->setSize(m_fRowHeight, m_fRowHeight);
	reflectionEnable->setCallbackId(CheckboxType::Reflection);
	if (*m_bReflection) { reflectionEnable->check(); }
	reflectionEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// Setup the reflection depth label
	tgui::Label::Ptr reflectionDepthLabel(m_GUI);
	reflectionDepthLabel->setText("ReflectionDepth");
	reflectionDepthLabel->setPosition(140, 137);
	reflectionDepthLabel->setSize(100, 25);
	reflectionDepthLabel->setAutoSize(true);
	reflectionDepthLabel->setTextSize(12);

	// Setup the plane texturing square length edit box
	tgui::EditBox::Ptr reflectionDepthEditBox(m_GUI, "ReflectionDepthEditBox");
	reflectionDepthEditBox->load("lib//TGUI//widgets//Black.conf");
	reflectionDepthEditBox->setSize(40, m_fRowHeight);
	reflectionDepthEditBox->setPosition(250, 137);

	// ------------------------------------------------------------------------

	// Refraction checkbox
	tgui::Checkbox::Ptr refractionEnable(m_GUI);
	refractionEnable->load("lib//TGUI//widgets//Black.conf");
	refractionEnable->setPosition(m_fLeftAlign, m_fTopAlign + 6.0f * (m_fVerticalPad + m_fRowHeight));
	refractionEnable->setText("Refraction");
	refractionEnable->setSize(m_fRowHeight, m_fRowHeight);
	refractionEnable->setCallbackId(CheckboxType::Refraction);
	if (*m_bRefraction) { refractionEnable->check(); }
	refractionEnable->bindCallbackEx(&UI::checkBoxCallback, this, tgui::Checkbox::Checked | tgui::Checkbox::Unchecked);

	// Setup the refraction depth label
	tgui::Label::Ptr refractionDepthLabel(m_GUI);
	refractionDepthLabel->setText("ReflectionDepth");
	refractionDepthLabel->setPosition(140, 162);
	refractionDepthLabel->setSize(100, 25);
	refractionDepthLabel->setAutoSize(true);
	refractionDepthLabel->setTextSize(12);

	// Setup the plane texturing square length edit box
	tgui::EditBox::Ptr refractionDepthEditBox(m_GUI, "RefractionDepthEditBox");
	refractionDepthEditBox->load("lib//TGUI//widgets//Black.conf");
	refractionDepthEditBox->setSize(40, m_fRowHeight);
	refractionDepthEditBox->setPosition(250, 162);

	// ------------------------------------------------------------------------

	// Add combo box for object type selection
	tgui::ComboBox::Ptr objectTypeSelectionComboBox(m_GUI, "ObjectTypeSelectionComboBox");
	objectTypeSelectionComboBox->load("lib//TGUI//widgets//Black.conf");
	objectTypeSelectionComboBox->setSize(120, m_fRowHeight);
	objectTypeSelectionComboBox->setPosition(m_fLeftAlign, 190);
	objectTypeSelectionComboBox->addItem("Directional Light");
	objectTypeSelectionComboBox->addItem("Point Light");
	objectTypeSelectionComboBox->addItem("Sphere");
	objectTypeSelectionComboBox->addItem("Area Light");
	objectTypeSelectionComboBox->addItem("Box");
	objectTypeSelectionComboBox->setSelectedItem(0);

	// Add object to scene button
	tgui::Button::Ptr addObjectToSceneButton(m_GUI, "AddObjectButton");
	addObjectToSceneButton->load("lib//TGUI//widgets//Black.conf");
	addObjectToSceneButton->setSize(100, m_fRowHeight);
	addObjectToSceneButton->setPosition(120 + m_fLeftAlign + m_fHorizontalPad, 190);
	addObjectToSceneButton->setText("Add Object");
	addObjectToSceneButton->bindCallbackEx(&UI::addObjectCallback, this, tgui::Button::LeftMouseClicked);

	// ------------------------------------------------------------------------
	// Current selected object

	// Object position label
	tgui::Label::Ptr objectPositionLabel(m_GUI);
	objectPositionLabel->setText("Current selected object:");
	objectPositionLabel->setPosition(m_fLeftAlign, 230);
	objectPositionLabel->setAutoSize(true);
	objectPositionLabel->setTextSize(12);
	
	// Object selection combo box
	comboBox = tgui::ComboBox::Ptr(m_GUI, "ObjectList");
	comboBox->load("lib//TGUI//widgets//Black.conf");
	comboBox->setBorders(2, 2, 2, 2);
	comboBox->setSize(270, m_fRowHeight);
	comboBox->setPosition(m_fLeftAlign, 250);
	comboBox->bindCallbackEx(&UI::comboBoxSelectionCallback, this, tgui::ComboBox::ItemSelected);

	// Add objects
	for (unsigned int objectIndex = 0; objectIndex < m_ObjecList.size(); objectIndex++)
	{
		// Get the current object
		Object& currentObject = *m_ObjecList[objectIndex];

		comboBox->addItem(currentObject.GetName());
	}

	comboBox->setSelectedItem(-1);

	// Add sliders for x, y, z coordinates
	sliderXPtr = tgui::Slider::Ptr(m_GUI, "PointLightPosX");
	sliderXPtr->load("lib//TGUI//widgets//Black.conf");
	sliderXPtr->setVerticalScroll(false);
	sliderXPtr->setPosition(2.0f * m_fLeftAlign, 250 + m_fRowHeight + m_fVerticalPad);
	sliderXPtr->setMinimum(0);
	sliderXPtr->setMaximum(SliderPositionAmplitude);

	sliderYPtr = tgui::Slider::Ptr(m_GUI, "PointLightPosY");
	sliderYPtr->load("lib//TGUI//widgets//Black.conf");
	sliderYPtr->setVerticalScroll(false);
	sliderYPtr->setPosition(2.0f * m_fLeftAlign, 250 + m_fRowHeight + 4.0f * m_fVerticalPad);
	sliderYPtr->setMinimum(0);
	sliderYPtr->setMaximum(SliderPositionAmplitude);

	sliderZPtr = tgui::Slider::Ptr(m_GUI, "PointLightPosZ");
	sliderZPtr->load("lib//TGUI//widgets//Black.conf");
	sliderZPtr->setVerticalScroll(false);
	sliderZPtr->setPosition(2.0f * m_fLeftAlign, 250 + m_fRowHeight + 7.0f * m_fVerticalPad);
	sliderZPtr->setMinimum(0);
	sliderZPtr->setMaximum(SliderPositionAmplitude);

	// ------------------------------------------------------------------------
	// Setup the move speed edit box

	// Setup the move speed label
	tgui::Label::Ptr moveSpeedLabel(m_GUI);
	moveSpeedLabel->setTextSize(12);
	moveSpeedLabel->setText("MouseMoveSpeed");
	moveSpeedLabel->setPosition(m_fLeftAlign, 365 + m_fVerticalPad);

	// Move speed edit box
	tgui::EditBox::Ptr moveSpeedEditBox(m_GUI, "MoveSpeed");
	moveSpeedEditBox->load("lib//TGUI//widgets//Black.conf");
	moveSpeedEditBox->setSize(60, m_fRowHeight);
	moveSpeedEditBox->setPosition(m_fLeftAlign + moveSpeedLabel->getSize().x + m_fHorizontalPad, 365 + m_fVerticalPad);

	// ------------------------------------------------------------------------	

	// Setup the update settings button
	tgui::Button::Ptr updateButton(m_GUI);
	updateButton->load("lib//TGUI//widgets//Black.conf");
	updateButton->setSize(100, m_fRowHeight);
	updateButton->setPosition(m_fLeftAlign, 400);
	updateButton->setText("Update");
	updateButton->bindCallbackEx(&UI::updateButtonCallback, this, tgui::Button::LeftMouseClicked);

	// ------------------------------------------------------------------------

	// Setup the radio buttons for the lighting model

	tgui::RadioButton::Ptr phongLightModelRadioButton(m_GUI, "PhongLighting");
	phongLightModelRadioButton->load("lib//TGUI//widgets//Black.conf");
	phongLightModelRadioButton->setPosition(m_fLeftAlign, 400 + m_fRowHeight + m_fVerticalPad);
	phongLightModelRadioButton->setText("Phong lighting");
	phongLightModelRadioButton->setSize(16, m_fRowHeight);
	phongLightModelRadioButton->setCallbackId(LightingModel::Phong);
	phongLightModelRadioButton->bindCallbackEx(&UI::lightModelRadioCallback,
		this,
		tgui::RadioButton::Checked | tgui::RadioButton::Unchecked);
	if (*m_peLightingModel == LightingModel::Phong)
	{
		phongLightModelRadioButton->check();
	}

	tgui::RadioButton::Ptr blinnPhongLightModelRadioButton(m_GUI, "BlinnPhongLighting");
	blinnPhongLightModelRadioButton->load("lib//TGUI//widgets//Black.conf");
	blinnPhongLightModelRadioButton->setPosition(m_fLeftAlign, 400 + 2.0f * (m_fRowHeight + m_fVerticalPad));
	blinnPhongLightModelRadioButton->setText("Blinn-Phong lighting");
	blinnPhongLightModelRadioButton->setSize(16, m_fRowHeight);
	blinnPhongLightModelRadioButton->setCallbackId(LightingModel::BlinnPhong);
	blinnPhongLightModelRadioButton->bindCallbackEx(&UI::lightModelRadioCallback,
		this,
		tgui::RadioButton::Checked | tgui::RadioButton::Unchecked);
	if (*m_peLightingModel == LightingModel::BlinnPhong)
	{
		blinnPhongLightModelRadioButton->check();
	}

	// ------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------

void UI::Update()
{
	// Retrieve the values from the position sliders
	int xPos, yPos, zPos;

	if (sliderXPtr != nullptr)
	{
		xPos = (int)(sliderXPtr->getValue() - SliderPositionAmplitude * 0.5f);
	}

	if (sliderYPtr != nullptr)
	{
		yPos = (int)(sliderYPtr->getValue() - SliderPositionAmplitude * 0.5f);
	}

	if (sliderZPtr != nullptr)
	{
		zPos = (int)(sliderZPtr->getValue() - SliderPositionAmplitude * 0.5f);
	}

	// Get a reference to the selected point light
	int iItemIndex = comboBox->getSelectedItemIndex();
	if (iItemIndex != -1)
	{
		// Update the position of the selected point light
		Object* pSelectedObject = m_ObjecList[iItemIndex];

		if (pSelectedObject != NULL)
		{
			pSelectedObject->SetPosition(glm::vec3(xPos / 50.0f, yPos / 50.0f, zPos / 50.0f));
		}
	}
}

// ----------------------------------------------------------------------------

void UI::checkBoxCallback(const tgui::Callback& callback)
{
	switch (callback.id)
	{
		case CheckboxType::Realtime:
		{
			*m_bRealtime = !(*m_bRealtime);

			break;
		}
		case CheckboxType::Shadows:
		{
			*m_bShadows = !(*m_bShadows);

			break;
		}
		case CheckboxType::SoftShadows:
		{
			*m_bSoftShadows = !(*m_bSoftShadows);

			break;
		}
		case CheckboxType::SuperSampling:
		{
			*m_bSuperSampling = !(*m_bSuperSampling);
			
			if (*m_bSuperSampling == true)
			{
				tgui::EditBox::Ptr superSamplingEditBox = m_GUI.get("SuperSamplingCountEditBox");
				if (superSamplingEditBox != nullptr)
				{
					sf::String value = std::to_string((*m_piSampleCount));
					superSamplingEditBox->setText(value);
				}
			}
			else
			{
				tgui::EditBox::Ptr superSamplingEditBox = m_GUI.get("SuperSamplingCountEditBox");
				if (superSamplingEditBox != nullptr)
				{
					superSamplingEditBox->setText("");
				}
			}
			
			break;
		}
		case CheckboxType::PlaneTexturing:
		{
			*m_bPlaneTexturing = !(*m_bPlaneTexturing);

			if (*m_bPlaneTexturing == true)
			{
				tgui::EditBox::Ptr squareLengthEditBox = m_GUI.get("SquareLengthEditBox");
				if (squareLengthEditBox != nullptr)
				{
					sf::String value = std::to_string((*m_piSquareLength));
					squareLengthEditBox->setText(value);
				}
			}
			else
			{
				tgui::EditBox::Ptr squareLengthEditBox = m_GUI.get("SquareLengthEditBox");
				if (squareLengthEditBox != nullptr)
				{
					squareLengthEditBox->setText("");
				}
			}

			break;
		}
		case CheckboxType::Reflection:
		{
			*m_bReflection = !(*m_bReflection);

			if (*m_bReflection == true)
			{
				tgui::EditBox::Ptr reflectionDepthEditBox = m_GUI.get("ReflectionDepthEditBox");
				if (reflectionDepthEditBox != nullptr)
				{
					sf::String value = std::to_string((*m_uipMaxReflectionDepth));
					reflectionDepthEditBox->setText(value);
				}
			}
			else
			{
				tgui::EditBox::Ptr reflectionDepthEditBox = m_GUI.get("ReflectionDepthEditBox");
				if (reflectionDepthEditBox != nullptr)
				{
					reflectionDepthEditBox->setText("");
				}
			}

			break;
		}
		case CheckboxType::Refraction:
		{
			*m_bRefraction = !(*m_bRefraction);

			if (*m_bRefraction == true)
			{
				tgui::EditBox::Ptr refractionDepthEditBox = m_GUI.get("RefractionDepthEditBox");
				if (refractionDepthEditBox != nullptr)
				{
					sf::String value = std::to_string((*m_uipMaxRefractionDepth));
					refractionDepthEditBox->setText(value);
				}
			}
			else
			{
				tgui::EditBox::Ptr refractionDepthEditBox = m_GUI.get("RefractionDepthEditBox");
				if (refractionDepthEditBox != nullptr)
				{
					refractionDepthEditBox->setText("");
				}
			}

			break;
		}

		default:
			break;
	}
}

// ----------------------------------------------------------------------------

void UI::addObjectCallback(const tgui::Callback& callback)
{
	tgui::ComboBox::Ptr objectTypeSelectionComboBox = m_GUI.get("ObjectTypeSelectionComboBox");

	int iItemIndex = objectTypeSelectionComboBox->getSelectedItemIndex();
	switch (iItemIndex)
	{
		case ObjectToAdd::DirectionalLightObj:
		{
			m_pScene->AddObject(new DirectionalLight());
			break;
		}

		case ObjectToAdd::PointLightObj:
		{
			m_pScene->AddObject(new PointLight());
			break;
		}
		
		case ObjectToAdd::SphereObj:
		{
			m_pScene->AddObject(new Sphere());
			break;
		}

		case ObjectToAdd::AreaLightObj:
		{
			m_pScene->AddObject(new AreaLight());
			break;
		}

		case ObjectToAdd::BoxObj:
		{
			m_pScene->AddObject(new Box());
			break;
		}
		
		default:
			break;
	}

	// Update the combo box with the new items added
	comboBox->removeAllItems();

	// Update the object list
	m_ObjecList = m_pScene->ObjectList();

	for (unsigned int objectIndex = 0; objectIndex < m_ObjecList.size(); objectIndex++)
	{
		// Get the current object
		Object* currentObject = m_ObjecList[objectIndex];

		comboBox->addItem(currentObject->GetName());
	}
}

// ----------------------------------------------------------------------------

void UI::lightModelRadioCallback(const tgui::Callback& callback)
{
	tgui::RadioButton::Ptr phongRadioButton = m_GUI.get("PhongLighting");
	tgui::RadioButton::Ptr blinnPhongRadioButton = m_GUI.get("BlinnPhongLighting");

	switch (callback.id)
	{
	case LightingModel::Phong:
	{
		if (blinnPhongRadioButton != NULL)
		{
			blinnPhongRadioButton->uncheck();
			*m_peLightingModel = LightingModel::Phong;
		}
		break;
	}
		
	case LightingModel::BlinnPhong:
	{
		if (phongRadioButton != NULL)
		{
			phongRadioButton->uncheck();
			*m_peLightingModel = LightingModel::BlinnPhong;
		}
		break;
	}
		
	default:
		break;
	}
}

// ----------------------------------------------------------------------------

void UI::comboBoxSelectionCallback(const tgui::Callback& callback)
{
	// Get a reference to the selected object
	int iItemIndex = comboBox->getSelectedItemIndex();
	if (iItemIndex != -1)
	{
		// Update the position of the selected object
		Object* pSelectedObject = m_ObjecList[iItemIndex];
		if (pSelectedObject != NULL)
		{
			glm::vec3 pos = pSelectedObject->GetPosition();

			std::cout << "Object " << iItemIndex << " selected." << std::endl;
			std::cout << "Position: " << pos.x << " " << pos.y << " " << pos.z << std::endl;

			sliderXPtr->setValue((unsigned int)(pos.x + SliderPositionAmplitude * 0.5f));
			sliderYPtr->setValue((unsigned int)(pos.y + SliderPositionAmplitude * 0.5f));
			sliderZPtr->setValue((unsigned int)(pos.z + SliderPositionAmplitude * 0.5f));
		}
	}
}

// ----------------------------------------------------------------------------

void UI::updateButtonCallback(const tgui::Callback& callback)
{
	// Get the number of reflections
	tgui::EditBox::Ptr reflectionLevelEditBox = m_GUI.get("ReflectionLevel");
	if (reflectionLevelEditBox != nullptr)
	{
		sf::String value = reflectionLevelEditBox->getText();
		if (value != "")
		{
			*m_uipMaxReflectionDepth = std::stoi(value.toAnsiString());
			std::cout << "Reflections level count: " << *m_uipMaxReflectionDepth << std::endl;
		}
	}

	// Get the soft shadow sample count
	tgui::EditBox::Ptr softShadowEditBoxX = m_GUI.get("SoftShadowSampleEditBoxX");
	tgui::EditBox::Ptr softShadowEditBoxZ = m_GUI.get("SoftShadowSampleEditBoxZ");
	if (softShadowEditBoxX != nullptr && softShadowEditBoxZ != nullptr)
	{
		sf::String valueX = softShadowEditBoxX->getText();
		sf::String valueZ = softShadowEditBoxZ->getText();

		if (valueX != "" && valueZ != "")
		{
			int tempValueX = std::stoi(valueX.toAnsiString());
			int tempValueZ = std::stoi(valueZ.toAnsiString());

			if (tempValueX > 0 && tempValueZ > 0)
			{
				// Get the list of area lights
				std::vector<AreaLight*>& areaLightList = m_pScene->AreaLightList();

				for (unsigned int areaLightIndex = 0; areaLightIndex < areaLightList.size(); areaLightIndex++)
				{
					areaLightList[areaLightIndex]->SetSampleCount(tempValueX, tempValueZ);
				}

				std::cout << "Soft shadow sample count : " << tempValueX << " " << tempValueZ << std::endl;
			}
			else
			{
				std::cout << "Soft shadow sample count: Invalid value" << std::endl;
			}
		}
	}

	// Get the number of samples used for super sampling
	tgui::EditBox::Ptr superSamplingEditBox = m_GUI.get("SuperSamplingCountEditBox");
	if (superSamplingEditBox != nullptr)
	{
		sf::String value = superSamplingEditBox->getText();
		if (value != "")
		{
			int tempValue = std::stoi(value.toAnsiString());

			if (tempValue >= 0)
			{
				*m_piSampleCount = tempValue;
				*m_pfSampleDistance = 1.0f / (float)*m_piSampleCount;
				std::cout << "Sample count: " << *m_piSampleCount << std::endl;
			}
			else
			{
				std::cout << "Sample count: Invalid value" << std::endl;
			}
		}
	}

	// Get the square length for the plane texturing
	tgui::EditBox::Ptr squareLengthEditBox = m_GUI.get("SquareLengthEditBox");
	if (squareLengthEditBox != nullptr)
	{
		sf::String value = squareLengthEditBox->getText();
		if (value != "")
		{
			int tempValue = std::stoi(value.toAnsiString());

			if (tempValue > 0)
			{
				*m_piSquareLength = tempValue;
				std::cout << "Square length: " << *m_piSquareLength << std::endl;
			}
			else
			{
				std::cout << "Square length: Invalid value" << std::endl;
			}
		}
	}

	// Get the reflection depth
	tgui::EditBox::Ptr reflectionDepthEditBox = m_GUI.get("ReflectionDepthEditBox");
	if (reflectionDepthEditBox != nullptr)
	{
		sf::String value = reflectionDepthEditBox->getText();
		if (value != "")
		{
			int tempValue = std::stoi(value.toAnsiString());

			if (tempValue >= 0)
			{
				*m_uipMaxReflectionDepth = tempValue;
				std::cout << "Reflection depth: " << *m_uipMaxReflectionDepth << std::endl;
			}
			else
			{
				std::cout << "Reflection depth: Invalid value" << std::endl;
			}
		}
	}

	// Get the refraction depth
	tgui::EditBox::Ptr refractionDepthEditBox = m_GUI.get("RefractionDepthEditBox");
	if (refractionDepthEditBox != nullptr)
	{
		sf::String value = refractionDepthEditBox->getText();
		if (value != "")
		{
			int tempValue = std::stoi(value.toAnsiString());

			if (tempValue >= 0)
			{
				*m_uipMaxRefractionDepth = tempValue;
				std::cout << "Refraction depth: " << *m_uipMaxRefractionDepth << std::endl;
			}
			else
			{
				std::cout << "Refraction depth: Invalid value" << std::endl;
			}
		}
	}
			
	// Get the move speed
	tgui::EditBox::Ptr moveSpeedEditBox = m_GUI.get("MoveSpeed");
	if (moveSpeedEditBox != nullptr)
	{
		sf::String value = moveSpeedEditBox->getText();
		if (value != "")
		{
			float tempValue = std::stof(value.toAnsiString());

			if (tempValue >= 0)
			{
				*m_pfMoveSpeed = tempValue;
				std::cout << "Move speed: " << *m_pfMoveSpeed << std::endl;
			}
			else
			{
				std::cout << "Invalid mouse move speed. " << std::endl;
				moveSpeedEditBox->setText("");
			}
		}
	}

	*m_bUpdateRequired = true;
}


// ----------------------------------------------------------------------------