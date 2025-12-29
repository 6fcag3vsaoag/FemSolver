#include "VisualizationFactory.h"
#include "rendering/DirectXVisualizer.h"

std::unique_ptr<IVisualizer> VisualizationFactory::createVisualizer(VisualizationType type) {
    switch (type) {
        case VisualizationType::Direct3D:
            return std::make_unique<DirectXVisualizer>();
        default:
            return std::make_unique<DirectXVisualizer>(); // Default to DirectX
    }
}