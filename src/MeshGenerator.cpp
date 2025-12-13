#include "MeshGenerator.h"
#include <cmath>
#include <stdexcept>

MeshGenerator::MeshGenerator(double Lx, double Ly, int Nx, int Ny) 
    : Lx_(Lx), Ly_(Ly), Nx_(Nx), Ny_(Ny) {
    if (Lx <= 0 || Ly <= 0 || Nx < 2 || Ny < 2) {
        throw std::invalid_argument("Invalid mesh parameters: Lx, Ly must be positive, Nx, Ny must be >= 2");
    }
}

Mesh MeshGenerator::generate() {
    Mesh mesh;
    
    // Create nodes
    mesh.nodes.reserve(Nx_ * Ny_);
    
    double dx = Lx_ / (Nx_ - 1);
    double dy = Ly_ / (Ny_ - 1);
    
    for (int i = 0; i < Ny_; ++i) {
        for (int j = 0; j < Nx_; ++j) {
            double x = j * dx;
            double y = i * dy;
            mesh.nodes.emplace_back(x, y);
        }
    }
    
    // Create elements (triangles)
    mesh.elements.reserve(2 * (Nx_ - 1) * (Ny_ - 1));
    
    for (int i = 0; i < Ny_ - 1; ++i) {
        for (int j = 0; j < Nx_ - 1; ++j) {
            int n1 = i * Nx_ + j;
            int n2 = i * Nx_ + (j + 1);
            int n3 = (i + 1) * Nx_ + j;
            int n4 = (i + 1) * Nx_ + (j + 1);
            
            // First triangle
            Element elem1 = {n1, n2, n3};
            mesh.elements.push_back(elem1);
            
            // Second triangle
            Element elem2 = {n2, n4, n3};
            mesh.elements.push_back(elem2);
        }
    }
    
    // Define boundaries
    std::vector<int> west, east, south, north;
    
    // West boundary (x = 0)
    for (int i = 0; i < Ny_; ++i) {
        west.push_back(i * Nx_);
    }
    
    // East boundary (x = Lx)
    for (int i = 0; i < Ny_; ++i) {
        east.push_back(i * Nx_ + Nx_ - 1);
    }
    
    // South boundary (y = 0)
    for (int j = 0; j < Nx_; ++j) {
        south.push_back(j);
    }
    
    // North boundary (y = Ly)
    for (int j = 0; j < Nx_; ++j) {
        north.push_back((Ny_ - 1) * Nx_ + j);
    }
    
    mesh.boundaries["west"] = west;
    mesh.boundaries["east"] = east;
    mesh.boundaries["south"] = south;
    mesh.boundaries["north"] = north;
    
    return mesh;
}