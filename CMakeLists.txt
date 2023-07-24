# CMakeList.txt: проект CMake для dimension_sensor_reload; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (dimension_sensor_reload dimension_sensor_reload.cpp)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
target_link_libraries(dimension_sensor_reload wiringPi)