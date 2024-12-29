#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
// попередній опис певних структур сторонньої бібліотеки BASIS, для уникнення включення заголовків
namespace BASIS
{
	struct Manager;
	struct GLTFModel;
	struct Buffer;
};
struct Orbit
{
	// середня відстань до Сонця(для планет) або до планети(для супутників)
	float distance{};
	// орбітальна швидкість
    float speed{};
    // кут орбітальної позиції в полярній системі координат
    float currentAngle{};
};
struct CelestialBody 
{
	// назва космічного тіла
    std::string name{};
    // радіус
    float radius{};
    // швидкість обертання навколо власної осі
    float rotationSpeed{};
    // набір даних про орбіту
    Orbit orbit{};
    // назва об'єкту навколо якого відбувається обертання
    std::string parentName{};
    // короткий опис фізичних характеристик
    std::string description{};

    // вказівник на батьківський об'єкт, встановлюється відповідно до `parentName`
    CelestialBody* parent{};
    // матриця трансформацій, необхідна для визначення позиції об'єкта у просторі
    // на її основі супутники/планети будуть здійснювати обертання
    glm::mat4 matrix{1.f};
    // поточний кут обертання планети навколо власної осі
    float rotationAngle{};
    // Функція оновлення орбітальної позиції, аргументи задаються користувачем через інтерфейс
    // orbitMul - множник орбітальної швидкості
    // rotationMul - множник швидкості обертання навколо власної осі
    // оновлює  `matrix` та повертає матрицю, яка містить інформацію про позицію, розмір та оберт
    glm::mat4 updateOrbitalPosition(float orbitMul,float rotationMul);
};
// структури для OpenGL буферів для зберігання інформації про орбіти та космічні тіла відповідно
struct alignas(16) OrbitUniform
{
	glm::mat4 model{1.f};
	glm::vec3 color{1.f};
};
struct alignas(16) CelestialUniform
{
	glm::mat4 model{1.f};
	std::uint32_t material{};
};
// Функція створення моделі орбіти
// radius - радіус орбіти
// numSegments - кількість сегментів
BASIS::GLTFModel genOrbitMesh(float radius, std::size_t numSegments);
// Функція створення моделі кілець Сатурна(просто квадрат)
BASIS::GLTFModel genRingsMesh();
// Функція створення буферу команд візуалізації,який складається з трьох команд: відображення планет,орбіт та кілець Сатурна
BASIS::Buffer genIndirectDrawBuffer(const std::vector<CelestialBody>& info,const BASIS::GLTFModel* planet,const BASIS::GLTFModel* orbit);
// Функція створення буферу даних космічних тіл та їх орбіт
BASIS::Buffer genCelestialUniformBuffer(std::vector<CelestialBody>& info,const BASIS::GLTFModel* model);
