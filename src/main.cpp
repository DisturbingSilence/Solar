#include <Solar.h>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <imgui.h>
#include <BASIS/app.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/compatibility.hpp>
namespace bs = BASIS;
// Параметри для користувацького інтерфейсу
static constexpr std::array<const char*,6> parameters = 
{
	"Маса(кг)","Об'єм(км^3)","Діаметр(км)","Період обертання(г)","Орбітальна швидкість(км/с)","Середня температура поверхні(C)"
};
// список космічних об'єктів
std::vector<CelestialBody> celestialData = 
{
	//	Stars
	CelestialBody{"Сонце",15.0,0.2,{0,0},{},"1.989×10^30,1.41×10^18,1391000,609.12,0,~5500"},

	//	Planets
	CelestialBody{"Марс",		1.2,0.97,	{50,2.41},"Сонце","6.39×10^23,1.63×10^11 км^3,6779,24.6,24.1,-63"},
	CelestialBody{"Венера",		1.5,-0.0041,{20,3.50},"Сонце","4.87×10^24,9.28×10^11,12104,-5832.5,35.0,464"},
	CelestialBody{"Земля",		2.0,1,		{26,2.98},"Сонце","5.97×10^24,1.08×10^12,12742,24,29.8,15"},
	CelestialBody{"Сатурн",		3.0,2.27,	{65,0.97},"Сонце","5.68×10^26,8.27×10^14,120536,10.7,9.7,-139"},
	CelestialBody{"Уран",		2.5,-1.39,	{80,0.68},"Сонце","8.68×10^25,6.83×10^13,50724,-17.2,6.8,-224"},
	CelestialBody{"Юпітер",		3.5,2.44,	{35,1.31},"Сонце","1.90×10^27,1.43×10^15,139820,9.9,13.1,-108"},
	CelestialBody{"Меркурій",	1.0,0.017,	{13,4.74},"Сонце","3.30×10^23,6.08×10^10,4880,1407.6,47.4,167"},
	CelestialBody{"Нептун",		2.3,1.49,	{87,0.543},"Сонце","1.02×10^26,6.25×10^13,49244,16.1,5.4,-201"},

	//Dwarfplanets
	CelestialBody{"Еріда",		1.2,-0.926,	{133,0.35},"Сонце","1.66×10^22,6.71×10^9,2326,25.9,3.5,-243"},
	CelestialBody{"Плутон",		0.9,-0.156,	{95,0.47},"Сонце","1.31×10^22,7.15×10^9,2377,-153.3,4.7,-229"},
	CelestialBody{"Церера",		0.5,2.63,	{42.5,0.92},"Сонце","9.39×10^20,4.21×10&7,939,9.1,17.9,-105"},
	CelestialBody{"Хаумеа",		0.67,6.25,	{113,0.84},"Сонце","4.01×10^21,1.99×10^8,1632,3.9,4.5,-241"},
	CelestialBody{"Макемаке",	0.55,-3.125,{118,0.9},"Сонце","3.10×10^21,1.54×10^8,1430,22.5,4.4,-243"},

	//	Moons
	CelestialBody{"Місяць",		0.4,0.037,	{2.3,5.4},"Земля","7.35×10^22 ,2.19×10^10,3474,655.7,1.0,-53"},		
	CelestialBody{"Фобос",		0.24,3.13,	{1.8,2.3},"Марс","1.06×10^16 ,5.72×10^2,22.4,7.7,2.1,-40"},		
	CelestialBody{"Деймос",		0.24,0.79,	{2.4,3.6},"Марс","1.48×10¹^15 ,1.51×10^2,12.4,30.3,1.35,-40"},		
	CelestialBody{"Європа",		0.2,0.28,	{4,4.4},"Юпітер","4.80×10^22 ,1.60×10^10,3122,85.2,13.7,-160"},	
	CelestialBody{"Каллісто",	0.28,0.06,	{5.3,2.3},"Юпітер","1.08×10^23 ,5.90×10^10,4821,400.8,8.2,-139"},	
	CelestialBody{"Ганімед",	0.15,0.14,	{4.7,5},"Юпітер","1.48×10^23 ,7.60×10^10,5268,171.7,10.9,-150"},		
};
bs::Pipeline populatePipeline(std::uint64_t pipelineHash);
// структура для контейнера OpenGL з інформацією про об'єкт користувача в симуляції та текстуру кілець Сатурна
struct alignas(16) GlobalUniforms
{
	std::uint64_t padding;
	std::uint64_t saturnRingsTexture{};
	glm::mat4 viewProj{1.f};
} g_uniforms;
// Основна програма
struct SolarSystem final : public bs::App
{
	SolarSystem(const bs::AppCreateInfo& info);
	void render(double delta) override;
	void gui(double delta) override;
	// Система завантаження активів
	bs::Manager		manager;
	// Спеціальний клас для керування візуалізацією
	bs::Renderer	rndr;
	// Користувацькі налаштування
	float zoom{90};
	float orbitMod{1};
	float rotationMod{1};
	bool paused{false};
	bool showOrbits{false};
	std::size_t selectedPlanet{0};
	// Тривимірні моделі
	const bs::GLTFModel* orbit{};
	const bs::GLTFModel* rings{};
	const bs::GLTFModel* planet{};
	// Шрифт
	ImFont* mainFont{};
	// Вказівники на буфери для запису даних
	OrbitUniform* orbitBufferPtr{};
	CelestialUniform* celestialBufferPtr{};
	// Графічні конвеєри
	std::optional<bs::Pipeline> orbitPipe;
	std::optional<bs::Pipeline> ringsPipe;
	std::optional<bs::Pipeline> planetPipe;
	// Буфери 
	std::optional<bs::Buffer> orbitBuffer;
	std::optional<bs::Buffer> celestialBuffer;
	std::optional<bs::Buffer> indirectDrawBuffer;
	std::optional<bs::TypedBuffer<GlobalUniforms>> globalUniformBuffer;
};

SolarSystem::SolarSystem(const bs::AppCreateInfo& info) :
	App(info)
{
	// Налаштовуємо систему керування активами
	manager.materialUploadCallback = 
	[](const std::vector<bs::Material>& materials)
	{
		struct alignas(16) ShaderMaterial
		{
			std::uint64_t baseColorTexture;
			std::uint32_t flags;
		};
		std::vector<ShaderMaterial> dstMaterials;
		dstMaterials.reserve(materials.size());
		for(const auto& m : materials)
		{
			dstMaterials.emplace_back(
			m.baseColorTexture,
			m.flags);
		}
		return bs::Buffer(std::span<ShaderMaterial>(dstMaterials),0);
	};
	using namespace BASIS::literals;
	// створюємо об'єкти орбіт та кілець Сатурна та вставляємо їх у менеджер активів для зручності
	manager.insertModel("orbit"_hash,genOrbitMesh(1,1024));
	manager.insertModel("rings"_hash,genRingsMesh());

	
	orbit = manager.getModel("orbit"_hash);
	rings = manager.getModel("rings"_hash);
	// Завантажуємо модель планети та кільця Сатурна
	planet = manager.getModel("planet"_hash,"assets\\planet.glb");
	auto* ringsTexture = manager.getTexture("rings"_hash,"assets\\saturn_rings.png");

	// Завантажуємо шрифт з підтримкою української мови
	ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    mainFont = io.Fonts->AddFontFromFileTTF("assets\\arsenal.ttf",16.f,nullptr,io.Fonts->GetGlyphRangesCyrillic());
    if(!mainFont)
    {
    	// якщо не виходить - створюємо помилку та закриваємо програму
    	throw bs::AssetException("Failed to load font 'arsenal.ttf'");
    }
    // Генеруємо графічні конвеєри з спеціальною функції
    orbitPipe  = populatePipeline("orbit"_hash);
	ringsPipe  = populatePipeline("rings"_hash);
	planetPipe = populatePipeline("planet"_hash);

	constexpr auto bufferFlags = bs::BufferFlags::PERSISTENT | bs::BufferFlags::COHERENT | bs::BufferFlags::WRITE;
	//Створюємо буфери
    orbitBuffer = bs::Buffer(sizeof(OrbitUniform) * (celestialData.size()),bufferFlags);
	celestialBuffer	= genCelestialUniformBuffer(celestialData,planet);
	indirectDrawBuffer	= genIndirectDrawBuffer(celestialData,planet,orbit);
	globalUniformBuffer = bs::TypedBuffer<GlobalUniforms>(g_uniforms,bs::BufferFlags::DYNAMIC);
	// Та дістаємо вказівники для запису даних
	orbitBufferPtr = reinterpret_cast<OrbitUniform*>(orbitBuffer->map(bs::AccessFlags::WRITE_ONLY));
	celestialBufferPtr = reinterpret_cast<CelestialUniform*>(celestialBuffer->map(bs::AccessFlags::WRITE_ONLY));
	g_uniforms.saturnRingsTexture = ringsTexture->makeBindless(*manager.getSampler({}));

	//Налаштування керування клавіатури та миші
	glfwSetWindowUserPointer(m_win,reinterpret_cast<void*>(this));
	glfwSetScrollCallback(m_win, [](GLFWwindow* window,double,double yoffset)
	{
		auto* app = static_cast<SolarSystem*>(glfwGetWindowUserPointer(window));
		app->zoom = glm::clamp(app->zoom - yoffset*2,30.,120.);
	});
	glfwSetKeyCallback(m_win, [](GLFWwindow* window, int key,int, int action,int) -> void
	{
		auto* app = static_cast<SolarSystem*>(glfwGetWindowUserPointer(window));

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GL_TRUE);
		if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		{
			app->active = !app->active;
			glfwSetInputMode(window, GLFW_CURSOR, app->active ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		}
		if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) app->m_camera.speed = 100.f;
		if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) app->m_camera.speed = 50.f;
	});
	//Налаштування камери
	m_camera.speed = 50.f;
	m_camera.pos = {0,50,0};
	// Вмикаємо можливість відображати прозорі елементи(необхідно для кілець Сатурна)
	bs::Renderer::enableCapability(bs::Cap::BLEND);
	bs::Renderer::blendFunc(bs::Factor::SRC_ALPHA,bs::Factor::ONE_MINUS_SRC_ALPHA);

	//Встановлюємло колір фону та призначаємо буфер з об'єктом користувача в симуляції на позицію 0
	rndr.clearColor(0.03,0.03,0.03,0);
	rndr.bindUniformBuffer(*globalUniformBuffer,0);
    //оновлюємо розміщення орбіт у просторі та їх колір
	for(std::uint32_t i{1};i < celestialData.size();i++)
	{
		orbitBufferPtr[i].model = glm::scale(glm::mat4(1.f),glm::vec3(celestialData[i].orbit.distance));
		orbitBufferPtr[i].color = glm::vec3(0.11,0.64,0.49);
	}
}
void SolarSystem::render(double)
{	
	// обмежуємо переміщення користувача по симуляції
	m_camera.pos = glm::clamp(m_camera.pos,glm::vec3(-700),glm::vec3(700));
	rndr.beginFrame();
	rndr.clear(bs::MaskFlagBit::COLOR | bs::MaskFlagBit::DEPTH);
	auto projection = glm::perspective( glm::radians(zoom) , float(m_width/m_height) , 0.1f , 1500.0f );
	g_uniforms.viewProj = projection * m_camera.view();
	globalUniformBuffer->update(g_uniforms);
	// відображуємо планети
	rndr.bindPipeline(*planetPipe);
	rndr.bindIndexBuffer(*planet->idxBuffer);
	rndr.bindVertexBuffer(*planet->vertexBuffer,0);
	rndr.bindStorageBuffer(*planet->materialBuffer,0);
	rndr.bindStorageBuffer(*celestialBuffer,1);
	rndr.drawIndexedIndirect(*indirectDrawBuffer,1);
	// якщо користувач ввімкнув орбіти в меню - відображуємо їх
	if(showOrbits)
	{
		rndr.bindPipeline(*orbitPipe);
		rndr.bindIndexBuffer(*orbit->idxBuffer);
		rndr.bindVertexBuffer(*orbit->vertexBuffer,0);
		rndr.bindStorageBuffer(*orbitBuffer,1);
		rndr.drawIndexedIndirect(*indirectDrawBuffer,1,0,20);
	}
	// відображуємо кільця Сатурна
	rndr.bindPipeline(*ringsPipe);
	rndr.bindIndexBuffer(*rings->idxBuffer);
	rndr.bindVertexBuffer(*rings->vertexBuffer,0);
	glm::mat4 ringsMatrix = glm::scale(celestialData[4].matrix,glm::vec3(celestialData[4].radius*20.f));
	bs::Renderer::setUniform(bs::FloatUniform::mat4,0,1,&ringsMatrix[0][0]);
	rndr.drawIndexedIndirect(*indirectDrawBuffer,1,0,40);
	rndr.endFrame();
	// якщо симуляція активна - обчислюємо нові орбітальні позиції планет та орбіти
	if(!paused)
	{
	for(std::uint32_t i{0};i<celestialData.size();++i)
	{
		celestialBufferPtr[i].model = celestialData[i].updateOrbitalPosition(orbitMod,rotationMod);
		if(celestialData[i].parent)
		{
			orbitBufferPtr[i].model = glm::scale(celestialData[i].parent->matrix,glm::vec3(celestialData[i].orbit.distance));
		}
	}
	}

}
void SolarSystem::gui(double)
{
	ImGui::SetNextWindowPos(ImVec2(0,0),ImGuiCond_Once);
	ImGui::SetNextWindowCollapsed(true,ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(m_width/3.3,m_height/1.8),ImGuiCond_Once);

	ImGui::PushFont(mainFont);
	
	ImGui::Begin("##",nullptr,ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::PushItemWidth(-120.f);
	// Кнопка перезапуску симуляції
	if(ImGui::Button("Перезапуск симуляції"))
	{
		rotationMod = 1;
		orbitMod = 1;
		for(auto& body : celestialData)
		{
			body.orbit.currentAngle = 0;
			body.rotationAngle = 0;
		}
	}
	ImGui::Checkbox("Пауза",&paused);
	ImGui::Checkbox("Показати орбіти",&showOrbits);
	ImGui::DragFloat("\nОрбітальна швидкість",&orbitMod,0.1,0,100,"%.3f",ImGuiSliderFlags_NoInput);
	ImGui::DragFloat("\nШвидкість обертання",&rotationMod,0.1,0,50,"%.3f",ImGuiSliderFlags_NoInput);
	ImGui::Separator();
	ImGui::Text("Вибір космічного тіла");
	if (ImGui::BeginCombo("##1", celestialData[selectedPlanet].name.c_str())) 
	{
		for (std::size_t i{}; i < celestialData.size(); ++i) 
		{
			const bool isSelected = selectedPlanet == i;
			if (ImGui::Selectable(celestialData[i].name.c_str(), isSelected)) selectedPlanet = i;
			if (isSelected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// Таблиця фізичних даних обраної планети
	if (ImGui::BeginTable("table1", 2,ImGuiTableFlags_SizingFixedFit |  ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("Параметр");
        ImGui::TableSetupColumn("Значення");
        ImGui::TableHeadersRow();
        const auto& selectedBody = celestialData[selectedPlanet];
      	if(!selectedBody.description.empty())
        {
  			std::size_t dataStart{};
  			std::size_t dataEnd{};
  			for(const auto* param : parameters)
  			{
  				ImGui::TableNextColumn();
  				ImGui::Text(param);
        		ImGui::TableNextColumn();
        		dataEnd = selectedBody.description.find(',',dataStart);
        		std::string value = selectedBody.description.substr(dataStart,dataEnd - dataStart);
        		dataStart = dataEnd +1;
        		ImGui::Text(value.c_str());
  			}
        }
        ImGui::EndTable();
    }
    // Пошук обраного космічного тіла на екрані
	if(ImGui::Button("Пошук обраного космічного тіла"))
	{
		glm::vec3 direction = glm::normalize(glm::vec3(celestialData[selectedPlanet].matrix[3]) - m_camera.pos);
		m_camera.pitch = glm::atan2(direction.y, glm::length(glm::vec2(direction.x, direction.z)));
		m_camera.yaw = glm::atan2(direction.z, direction.x);
	}
	// Якщо орбіти ввімкнені та обрано не Сонце - дати можливість обрати колір орбіти
	ImGui::BeginDisabled(selectedPlanet == 0 || !showOrbits);
	ImGui::Text("Колір орбіти");
	ImGui::SliderFloat("Червоний",&orbitBufferPtr[selectedPlanet].color[0],0.f,1.f,"%.3f");
	ImGui::SliderFloat("Зелений",&orbitBufferPtr[selectedPlanet].color[1],0.f,1.f,"%.3f");
	ImGui::SliderFloat("Синій",&orbitBufferPtr[selectedPlanet].color[2],0.f,1.f,"%.3f");
	ImGui::EndDisabled();
    ImGui::End();
}
// Точка входу в програму
int main()
{	
	// відкриваємо файл помилок
	std::ofstream crashlog("crashlog.txt");
	try
	{
	// Створюємо об'єкт нашого класу з певними параметрами
	SolarSystem solar({
		.name = "Solar System",
		.width = 956,
		.height = 956,
		.flags = bs::AppFlags::DOUBLEBUFFER | bs::AppFlags::UNRESIZABLE});	
	// та запускаємо програму
	solar.run();
	}
	catch(const std::exception& e)
	{
		// при виникненні помилок - записуємо у файл та завершуємо програму
		crashlog << e.what();
	}
}
// Функція створення графічних конвеєрів на основі хеш значення
bs::Pipeline populatePipeline(std::uint64_t pipeHash)
{
// схема буфера вершин(для тривимірних моделей)
bs::VertexInputState vstate = 
{
	bs::VertexBinding
	{
		.location = 0,
		.binding  = 0,
		.offset   = offsetof(bs::Vertex,pos),
		.fmt	  = bs::Format::RGB32F
	},
	/*
	bs::VertexBinding{
		.location = 1,
		.binding  = 0,
		.offset   = offsetof(bs::Vertex,normal),
		.fmt	  = bs::Format::RGB32F
	},*/
	bs::VertexBinding
	{
		.location = 1,
		.binding  = 0,
		.offset   = offsetof(bs::Vertex,uv),
		.fmt 	  = bs::Format::RG32F
	}
};
	// Всі необхідні шейдери
	auto planet_vert = bs::Shader(bs::ShaderType::VERTEX  , bs::App::loadFile("shaders\\planet.vert"),"planet_v");
	auto planet_frag = bs::Shader(bs::ShaderType::FRAGMENT, bs::App::loadFile("shaders\\planet.frag"),"planet_f");
	auto orbit_vert = bs::Shader(bs::ShaderType::VERTEX  , bs::App::loadFile("shaders\\orbit.vert"),"orbit_v");
	auto orbit_frag = bs::Shader(bs::ShaderType::FRAGMENT, bs::App::loadFile("shaders\\orbit.frag"),"orbit_f");
	auto rings_vert = bs::Shader(bs::ShaderType::VERTEX  , bs::App::loadFile("shaders\\rings.vert"),"rings_v");
	auto rings_frag = bs::Shader(bs::ShaderType::FRAGMENT, bs::App::loadFile("shaders\\rings.frag"),"rings_f");

	// Заповнюємо інформацію для графічних конвеєрів
	auto planet_info = bs::PipelineCreateInfo{.vertex = &planet_vert, .fragment = &planet_frag,};
	auto rings_info = bs::PipelineCreateInfo{.vertex = &rings_vert,.fragment = &rings_frag};
	auto orbit_info = bs::PipelineCreateInfo{.vertex = &orbit_vert,.fragment = &orbit_frag};
	planet_info.depthState.depthTestEnable = true;
	planet_info.depthState.depthWriteEnable = true;
	planet_info.vertexInputState = vstate;
	orbit_info.mode = bs::PrimitiveMode::LINES;
	orbit_info.depthState = planet_info.depthState;
	orbit_info.vertexInputState = std::vector(vstate.begin(),vstate.begin()+1);
	rings_info.depthState = planet_info.depthState;
	rings_info.vertexInputState = orbit_info.vertexInputState;
	rings_info.rasterizationState.cullMode = bs::CullMode::NONE;
	using namespace BASIS::literals;
	// повертаємо графічний конвеєр на основі переданого хешу
	switch(pipeHash)
	{
		case "planet"_hash: return bs::Pipeline(planet_info,"planetPipeline");
		case "rings"_hash: return bs::Pipeline(rings_info,"ringsPipeline");
		case "orbit"_hash: return bs::Pipeline(orbit_info,"orbitPipeline");
		default: static_assert("no such pipeline");
	};
}