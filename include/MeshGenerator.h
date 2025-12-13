#ifndef MESHGENERATOR_H
#define MESHGENERATOR_H

#include "Types.h"
#include <vector>
#include <map>
#include <memory>

class MeshGenerator {
public:
    MeshGenerator(double Lx = 1.0, double Ly = 1.0, int Nx = 10, int Ny = 10);
    ~MeshGenerator() = default;

    // Generate structured mesh
    Mesh generate();

    // Setters
    void setDimensions(double Lx, double Ly) { Lx_ = Lx; Ly_ = Ly; }
    void setNodeCount(int Nx, int Ny) { Nx_ = Nx; Ny_ = Ny; }

    // Getters
    double getLx() const { return Lx_; }
    double getLy() const { return Ly_; }
    int getNx() const { return Nx_; }
    int getNy() const { return Ny_; }

private:
    double Lx_, Ly_;  // Dimensions of the domain
    int Nx_, Ny_;     // Number of nodes in each direction
};

#endif // MESHGENERATOR_H