# Что за проект
[English version](ReadMe_eng.md)

Приложение для трекинга питья воды. Позволяет пользователю отслеживать, сколько воды он выпил за день, и устанавливать цели по потреблению воды.

# Технологии
Приложение, написанно на C++ c графической библиотекой Qt

# Установка

## Скачивание готового приложения
- Вы можете скачать уже готовое приложение в Releases

## Сборка из исходного кода
В проекте используется система сборки CMake.
Также обратите внимание на то, что для сборки требуется установленный Qt и CMake.
Путь к Qt указывается в CmakeLists.txt (9 строка - `set(CMAKE_PREFIX_PATH "E:/Qt/6.11.0/mingw_64/lib/cmake"`)

Пример пути к CMake и Ninja из CLion:

```powershell
$cmake = "C:\Tools\CLion\bin\cmake\win\x64\bin\cmake.exe"
$ninja = "C:/Tools/CLion/bin/ninja/win/x64/ninja.exe"
```

В PowerShell строка в кавычках сама по себе считается текстом. Оператор `&` запускает указанный путь как программу.

### Release

Сконфигурировать Release:

```powershell
& $cmake -S . -B cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM="$ninja"
```

Собрать Release:

```powershell
& $cmake --build cmake-build-release --config Release
```

### Debug

Сконфигурировать Debug:

```powershell
& $cmake -S . -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_MAKE_PROGRAM="$ninja"
```

Собрать Debug:

```powershell
& $cmake --build cmake-build-debug --config Debug
```


---
Описание пока неполное, так как проект находится в разработке. В будущем планируется добавить больше информации о функциональности приложения, а также инструкции по использованию.
