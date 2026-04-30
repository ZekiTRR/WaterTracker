# Как добавить иконку приложения в Qt/CMake

В этом проекте используется один файл иконки:

```text
resources/Iconka.ico
```

Нужно положить `.ico` файл в папку `resources` именно с таким именем.

## 1. Иконка внутри Qt-приложения

Qt-ресурсы описаны в файле:

```text
resources/resources.qrc
```

В нём указано:

```xml
<RCC>
    <qresource prefix="/icons">
        <file>Iconka.ico</file>
    </qresource>
</RCC>
```

После добавления этого файла в `add_executable(...)` в `CMakeLists.txt`, Qt встраивает `Iconka.ico` внутрь приложения. В C++ иконку можно загрузить по resource path:

```cpp
QIcon appIcon(":/icons/Iconka.ico");
```

Потом эту иконку можно назначить всему приложению:

```cpp
a.setWindowIcon(appIcon);
mainWindow.setWindowIcon(appIcon);
```

И отдельно иконке в трее:

```cpp
trayIcon->setIcon(QIcon(":/icons/Iconka.ico"));
```

## 2. Иконка для Windows .exe файла

Для Windows нужен resource script файл:

```text
resources/app_icon.rc
```

Он содержит:

```rc
IDI_ICON1 ICON "Iconka.ico"
```

Именно это заставляет Windows Explorer показывать твою иконку у скомпилированного `.exe` файла вместо стандартной иконки программы.

## 3. Настройка CMake

Оба ресурсных файла подключены в `add_executable(...)`:

```cmake
resources/resources.qrc
resources/app_icon.rc
```

`resources/resources.qrc` обрабатывается Qt, потому что в проекте включён `CMAKE_AUTORCC`.

`resources/app_icon.rc` обрабатывается Windows resource compiler при сборке под Windows.

## Важно

Сборка ожидает, что этот файл существует:

```text
resources/Iconka.ico
```

Если файла нет, компиляция может упасть, потому что и Qt-ресурсы, и Windows resource script ссылаются на него.
