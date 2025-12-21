# FemSolver

FemSolver is a C++ program for solving elliptic equations using the Finite Element Method (FEM). The program implements all OOP principles (encapsulation, inheritance, polymorphism), uses templates and containers, handles exceptions, provides a graphical user interface, generates reports, and includes visualization capabilities.

## Compilation Instructions for Windows

### Prerequisites

- CMake 3.20 or higher
- A C++ compiler supporting C++17 (such as MSVC, MinGW-w64, or Clang)
- Git (optional, for cloning the repository)

### Building with CMake

#### Using Visual Studio Developer Command Prompt or Visual Studio Build Tools:

1. Open the Developer Command Prompt for Visual Studio or Visual Studio Developer PowerShell
2. Navigate to your project directory:
   ```cmd
   cd path\to\FemSolver
   ```
3. Create a build directory:
   ```cmd
   mkdir build
   cd build
   ```
4. Configure the project with CMake:
   ```cmd
   cmake .. -G "Visual Studio 17 2022" -A x64
   ```
   Or for older Visual Studio versions:
   ```cmd
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```
5. Build the project:
   ```cmd
   cmake --build . --config Release
   ```
   Or use msbuild directly:
   ```cmd
   msbuild FemSolver.sln /p:Configuration=Release
   ```

#### Using MinGW-w64:

1. Open Command Prompt or PowerShell
2. Navigate to your project directory:
   ```cmd
   cd path\to\FemSolver
   ```
3. Create a build directory:
   ```cmd
   mkdir build
   cd build
   ```
4. Configure the project with CMake:
   ```cmd
   cmake .. -G "MinGW Makefiles"
   ```
5. Build the project:
   ```cmd
   cmake --build . --config Release
   ```
   Or use mingw32-make directly:
   ```cmd
   mingw32-make
   ```

### Alternative: Using CMake GUI

1. Open CMake GUI
2. Set "Where is the source code" to your FemSolver directory
3. Set "Where to build the binaries" to a new directory (e.g., FemSolver/build)
4. Click "Configure" and select your generator (Visual Studio, MinGW, etc.)
5. Click "Generate"
6. Click "Open Project" or navigate to the build directory and build using your IDE

### Running the Program

After successful compilation, you can run the program from the build directory:
```cmd
cd build
.\Release\FemSolver.exe  # For Visual Studio builds
```
or
```cmd
.\FemSolver.exe  # For MinGW builds
```

The program will launch with a graphical user interface by default.

## Features

- Solves elliptic partial differential equations using the finite element method
- Supports variable coefficients for the differential operator
- Implements Dirichlet and Neumann boundary conditions
- Generates triangular meshes for 2D domains
- Provides a graphical user interface for input and visualization
- Exports solution reports to text files
- Includes exception handling for robust operation
- Uses modern C++17 features including templates and STL containers