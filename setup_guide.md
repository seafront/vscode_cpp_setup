# VS Code C++ Build Environment Setup Guide
## MSVC + Clang (ClangCL) + CMake + GTest/GMock on Windows x64

---

## 환경 정보

| 항목 | 버전 |
|---|---|
| OS | Windows 11 x64 |
| Visual Studio | Community 2022 (v17.11) |
| Clang (VS 번들) | 17.0.3 (`VC/Tools/Llvm/x64/bin/clang-cl.exe`) |
| MSVC | 14.41 |
| CMake | VS 2022 번들 (`Common7/IDE/CommonExtensions/Microsoft/CMake`) |
| GoogleTest / GMock | v1.15.2 (pre-built, ClangCL Debug) |

---

## 1. 필수 설치

### 1-1. Visual Studio Community 2022
[https://visualstudio.microsoft.com/](https://visualstudio.microsoft.com/) 에서 설치.

설치 시 워크로드 선택:
- **C++를 사용한 데스크톱 개발** 체크

개별 구성 요소에서 추가 체크:
- **Windows용 C++ Clang 도구** (`Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset`)

> VS 설치 후 나중에 컴포넌트를 추가하려면: Visual Studio Installer → Visual Studio 2022 → 수정(Modify) → 개별 구성 요소 탭 → "Clang" 검색 후 체크

### 1-2. CMake

별도 설치 불필요. VS 2022에 CMake가 번들로 포함되어 있다.

설치 경로: `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`

> tasks.json에서는 전체 경로를 사용한다. VS Code 터미널은 시스템 PATH를 그대로 상속하지 않을 수 있다.

### 1-3. VS Code 확장

- **C/C++** (`ms-vscode.cpptools`)
- **CMake Tools** (`ms-vscode.cmake-tools`) — 선택 사항

---

## 2. 프로젝트 파일 구성

```
project-root/
├── CMakeLists.txt
├── CMakePresets.json
├── lib/
│   └── gtest/
│       ├── include/
│       │   ├── gtest/
│       │   └── gmock/
│       └── lib/
│           ├── gtest.lib
│           ├── gtest_main.lib
│           ├── gmock.lib
│           └── gmock_main.lib
├── tests/
│   └── test_main.cpp
└── .vscode/
    ├── c_cpp_properties.json
    ├── tasks.json
    └── launch.json
```

### 2-1. `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pre-built googletest v1.15.2 (ClangCL, Debug)
foreach(_lib gtest gtest_main gmock gmock_main)
    add_library(${_lib} STATIC IMPORTED)
    set_target_properties(${_lib} PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/gtest/lib/${_lib}.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lib/gtest/include"
    )
endforeach()

enable_testing()

add_executable(tests tests/test_main.cpp)
target_link_libraries(tests gtest_main gmock)

include(GoogleTest)
gtest_discover_tests(tests)
```

- `lib/gtest/`에 pre-built 라이브러리를 저장하므로 빌드 시 googletest를 매번 컴파일하지 않는다.
- 테스트 소스 파일이 늘어나면 `add_executable(tests ...)` 에 추가한다.

#### lib/gtest/ 구성 방법 (최초 1회)

이미 repo에 포함되어 있으므로 clone 후 별도 작업 불필요.
새로 구성하는 경우:

```powershell
# 1. googletest 소스 다운로드 및 빌드
Invoke-WebRequest -Uri "https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip" -OutFile "gtest.zip"
Expand-Archive "gtest.zip" -DestinationPath "."

# CMakeLists.txt에 add_subdirectory로 임시 추가 후 빌드
# build/lib/Debug/ 에 .lib 생성됨

# 2. 헤더와 .lib 복사
New-Item -ItemType Directory -Force "lib\gtest\include", "lib\gtest\lib"
Copy-Item -Recurse "googletest-1.15.2\googletest\include\gtest" "lib\gtest\include\gtest"
Copy-Item -Recurse "googletest-1.15.2\googlemock\include\gmock" "lib\gtest\include\gmock"
Copy-Item "build\lib\Debug\*.lib" "lib\gtest\lib\"

# 3. 소스 정리
Remove-Item -Recurse "googletest-1.15.2", "gtest.zip"
```

### 2-2. `CMakePresets.json`

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "windows-clangcl-debug",
      "displayName": "Windows ClangCL Debug",
      "generator": "Visual Studio 17 2022",
      "binaryDir": "${sourceDir}/build",
      "architecture": {
        "value": "x64",
        "strategy": "set"
      },
      "toolset": {
        "value": "ClangCL",
        "strategy": "set"
      },
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "debug",
      "configurePreset": "windows-clangcl-debug",
      "configuration": "Debug"
    }
  ]
}
```

- `generator`: VS 버전에 맞게 변경. VS 2019이면 `"Visual Studio 16 2019"`
- `toolset`: `"ClangCL"` → VS 번들 clang-cl 사용 (1-1에서 설치한 컴포넌트)
- 빌드 결과물: `build/Debug/tests.exe`

### 2-3. `.vscode/c_cpp_properties.json`

```json
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE"
            ],
            "compilerPath": "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clang-cl.exe",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "windows-clang-x64"
        }
    ],
    "version": 4
}
```

- `compilerPath`: VS 번들 clang-cl 경로. VS 버전이 다르면 경로의 `2022`를 맞게 수정
- IntelliSense가 MSVC 표준 라이브러리 헤더와 `lib/gtest/include`를 자동으로 인식

### 2-4. `.vscode/tasks.json`

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Configure",
            "type": "shell",
            "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe",
            "args": ["--preset", "windows-clangcl-debug"],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "group": "build",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": []
        },
        {
            "label": "CMake Clean Build",
            "type": "shell",
            "command": "if (Test-Path build) { Remove-Item -Recurse -Force build }; & 'C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --preset windows-clangcl-debug; if ($LASTEXITCODE -eq 0) { & 'C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build --preset debug }",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "group": "build",
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": ["$msCompile"]
        },
        {
            "label": "CMake Build",
            "type": "shell",
            "command": "C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe",
            "args": ["--build", "--preset", "debug"],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "reveal": "always",
                "panel": "shared"
            },
            "problemMatcher": ["$msCompile"],
            "dependsOn": "CMake Configure"
        }
    ]
}
```

- cmake 전체 경로를 사용한다. VS Code 터미널은 시스템 PATH를 그대로 상속하지 않을 수 있다.
- `Ctrl+Shift+B` → CMake Build 실행 (CMake Configure 자동 선행)
- **CMake Clean Build**: `build/` 디렉토리를 삭제 후 configure + build 전체 수행. `lib/gtest/`는 삭제되지 않는다.

### 2-5. `.vscode/launch.json`

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "C/C++: CMake ClangCL Debug",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Debug/tests.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "CMake Build"
        }
    ]
}
```

- `type`: `cppvsdbg` — MSVC ABI 기반이므로 MSVC 디버거 사용 (gdb/lldb 아님)
- `preLaunchTask`: F5 누르면 CMake Build 태스크를 먼저 실행 후 디버거 시작

---

## 3. 빌드 및 테스트 실행

| 동작 | 방법 |
|---|---|
| 빌드 | `Ctrl+Shift+B` |
| 빌드 + 디버그 시작 | `F5` |
| Clean Build | `Ctrl+Shift+P` → Tasks: Run Task → **CMake Clean Build** |
| 터미널에서 테스트 실행 | `ctest --test-dir build --build-config Debug --output-on-failure` |

> **Clean Build**: `build/` 디렉토리를 완전히 삭제하고 처음부터 빌드한다. `lib/gtest/`의 pre-built 라이브러리는 삭제되지 않으므로 googletest 재빌드 없이 빠르게 완료된다.

빌드 결과물: `build/Debug/tests.exe`

### 테스트 파일 추가 방법

`tests/` 디렉토리에 파일을 추가하고 `CMakeLists.txt`의 `add_executable`에 등록한다.

```cmake
add_executable(tests
    tests/test_main.cpp
    tests/test_foo.cpp   # 추가
)
```

---

## 4. 트러블슈팅

### `'iostream' file not found`
- **원인**: `C:\Program Files\LLVM\bin\clang++.exe` (standalone LLVM) 사용 중. 이 clang++은 MSVC 표준 라이브러리 헤더가 없다.
- **해결**: VS 번들 `clang-cl.exe`를 사용해야 한다. `c_cpp_properties.json`의 `compilerPath`가 올바르게 설정되어 있는지 확인.

### `cmake 용어가 인식되지 않습니다`
- **원인**: VS Code 터미널의 PATH에 cmake가 없음.
- **해결**: `tasks.json`의 `command`를 `cmake` 대신 `C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe` 전체 경로로 지정.

### `MSB8020: ClangCL에 대한 빌드 도구를 찾을 수 없습니다`
- **원인**: VS에 "Windows용 C++ Clang 도구" 컴포넌트가 미설치.
- **해결**: Visual Studio Installer → 수정 → 개별 구성 요소 → "Windows용 C++ Clang 도구" 설치.

### `C/C++: clang++.exe build active file` 태스크가 실행됨
- **원인**: C/C++ 확장이 자동 생성하는 태스크. `Ctrl+Shift+B`로 실행 시 뜰 수 있음.
- **해결**: F5로 실행하면 `launch.json`의 `preLaunchTask`인 CMake Build만 실행된다. `Ctrl+Shift+B` 시에는 태스크 선택 목록에서 **CMake Build**를 선택.
