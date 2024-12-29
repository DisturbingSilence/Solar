#include <Solar.h>
#include <vector>
#include <numeric>
#include <numbers>
#include <algorithm>
#include <iostream>
#include <BASIS/buffer.h>
#include <BASIS/texture.h>
#include <BASIS/manager.h>
namespace bs = BASIS;
// Функція переведення полярних координат в декартові
static glm::vec3 polarToCartesian(float distance, float degrees) 
{	
    float radians = glm::radians(degrees);
    return glm::vec3(distance * std::cos(radians), 0, distance * std::sin(radians));
}
// Функція створення буферу команд візуалізації,який складається з трьох команд: відображення планет,орбіт та кілець Сатурна
// info - інформація про космічні тіла
// planet - вказівник на модель планети
// orbit - вказівник на модель орбіти
bs::Buffer genIndirectDrawBuffer(const std::vector<CelestialBody>& info,const bs::GLTFModel* planet,const bs::GLTFModel* orbit)
{
	// структура команд візуалізації
	struct IndirectCommand 
	{
	    std::uint32_t	idxCount{};
	    std::uint32_t	instanceCount{};
	    std::uint32_t	firstIndex{};
	    std::int32_t	firstVertex{};
	    std::uint32_t   firstInstance{};
	};
	std::array<IndirectCommand,3> drawBufferData = 
	{
		// команда візуалізації планет
		IndirectCommand
		{
			.idxCount = static_cast<std::uint32_t>(planet->idxBuffer->size() / sizeof(std::uint32_t)),
			.instanceCount = static_cast<std::uint32_t>(info.size()),
		},
		// команда візуалізації орбіт
		IndirectCommand
		{
			.idxCount = static_cast<std::uint32_t>(orbit->idxBuffer->size() / sizeof(std::uint32_t)),
			.instanceCount = static_cast<std::uint32_t>(info.size()), 
		},
		// команда візуалізації кілець Сатурна
		IndirectCommand
		{
			.idxCount = 6u,
			.instanceCount = 1
		}
	};
	return bs::Buffer(drawBufferData,0);
}
// Функція створення моделі кілець Сатурна(просто квадрат)
bs::GLTFModel genRingsMesh()
{
	// вектор із вершинами квадрату
	std::vector<bs::Vertex> vertices = 
	{
		bs::Vertex{
			.pos = {-0.5f,0, -0.5f},
		},
		bs::Vertex{
			.pos = {0.5f,0, -0.5f},
		},
		bs::Vertex{
			.pos = {0.5f, 0, 0.5f},
		},
		bs::Vertex{
			.pos = {-0.5f,0, 0.5f},
		},
	};
	// послідовність їх відображення(два трикутники)
	std::vector<std::uint32_t> indices = {0,1,2,0,3,2};
	//створення моделі з наданих даних
    bs::GLTFModel outModel;
    outModel.materials = {{}};
	outModel.idxBuffer = bs::Buffer(std::span<std::uint32_t>(indices),0);
	outModel.vertexBuffer = bs::Buffer(std::span<bs::Vertex>(vertices),0);
	return outModel;
}
// Функція створення буферу даних космічних тіл та їх орбіт
// data - інформація про космічні тіла
// model - вказівник на модель планети
bs::Buffer genCelestialUniformBuffer(std::vector<CelestialBody>& data,const bs::GLTFModel* model)
{
	// вектор необхідних для OpenGL структур
	std::vector<CelestialUniform> uniforms;
	uniforms.reserve(data.size());
	for(std::uint32_t i{};i < data.size();++i)
	{
		// якщо у космічного тіла є батьківських елемент(Сонце або планета) - встановлюємо вказівник на нього
		auto parentIter = std::find_if(data.begin(),data.end(),[&](const CelestialBody& body)
		{
			return body.name == data[i].parentName;
		});
		if(parentIter != data.end())
		{
			data[i].parent = &data[std::distance(data.begin(),parentIter)];
		}
		// трошки підправляємо дані для більш зручного перегляду
		data[i].orbit.speed /= 100.f;
		data[i].orbit.distance *= 5.f;
		// знаходимо правильний матеріал у тривимірній моделі планети
		auto variantIter = std::find(model->materialVariants.begin(),model->materialVariants.end(),data[i].name);
		std::uint32_t mapping = variantIter != model->materialVariants.end() ? std::distance(model->materialVariants.begin(),variantIter) : 0;
		// та створюємо нову структуру у нашому векторі з вибраним матеріалом
		uniforms.push_back( 
		CelestialUniform
		{
			.material = (static_cast<std::uint32_t>(model->nodes[0].mesh.primitives[1].mappings[mapping].value()+1))
		});
	}
	using enum bs::BufferFlags;
	// повертаємо буфер даних з створеними структурами
	return bs::Buffer(std::span<CelestialUniform>(uniforms),PERSISTENT | COHERENT | WRITE);
}
bs::GLTFModel genOrbitMesh(float radius, std::size_t numSegments)
{	
	std::vector<glm::vec3> vertices;
	std::vector<std::uint32_t> indices;
	vertices.reserve(numSegments);
	indices.resize(numSegments);
	// кут одного сегменту
    float angle = 360.0f / numSegments;
    // створюємо вершини на основі радіусу та кількості сегментів
    for (std::size_t i{}; i < numSegments; i++)
    {
        vertices.push_back(polarToCartesian(radius,angle*i));
    }
    // заповнюємо вектор індексів числами від 0 до numSegments
    std::iota(indices.begin(),indices.end(),0);
    //будуємо модель на основі створених даних
    bs::GLTFModel outModel;
    outModel.materials = {{}};
    outModel.idxBuffer = bs::Buffer(std::span<std::uint32_t>(indices),0);
    outModel.vertexBuffer = bs::Buffer(std::span<glm::vec3>(vertices),0);
    return outModel;
}
// Функція оновлення орбітальної позиції, аргументи задаються користувачем через інтерфейс
// orbitMul - множник орбітальної швидкості
// rotationMul - множник швидкості обертання навколо власної осі
glm::mat4 CelestialBody::updateOrbitalPosition(float orbitMul,float rotationMul)
{
	// оновлюємо поточний орбітальний кут
	orbit.currentAngle += orbit.speed * orbitMul;
	// якщо він перевищує 360 градусів - обнуляємо
	if(orbit.currentAngle > 360) orbit.currentAngle = 0;
	// те ж саме з обертанням навколо власної осі
	// так як планети обертаються в різних напрямках - перевіряємо межі 360 та -360
	rotationAngle += rotationSpeed * rotationMul;
	if(rotationSpeed > 0 && rotationAngle > 360) rotationAngle = 0;
	if(rotationSpeed < 0 && rotationAngle < -360) rotationAngle = 0;
	// обчислюємо нову позицію в декартових координатах на основі полярних координат(нашого орбітального кута)
	glm::vec3 newPosition = polarToCartesian(orbit.distance,orbit.currentAngle);

	// оновлюємо матрицю розміщення поточного космічного тіла
	// якщо є батьківське космічне тіло(Сонце або планета) - переміщуємо відносно нього
	matrix = glm::translate(parent ? parent->matrix : glm::mat4(1),newPosition);
	//Окремо обчислюємо матрицю оберту та розмірів щоб не вплинути на дочірні матриці
	glm::mat4 R = glm::rotate(glm::mat4(1.f),glm::radians(rotationAngle),glm::vec3(0,1,0));
	glm::mat4 S = glm::scale(glm::mat4(1.f),glm::vec3(radius));
	return matrix * R * S; // Множимо та повертаємо нову матрицю для поточного тіла
}