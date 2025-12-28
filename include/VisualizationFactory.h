#ifndef VISUALIZATIONFACTORY_H
#define VISUALIZATIONFACTORY_H

#include "IVisualizer.h"
#include <memory>

enum class VisualizationType {
    Direct3D
};

/**
 * @brief Forward declaration for DirectXVisualizer to avoid circular includes
 */
class DirectXVisualizer;

/**
 * @brief Factory class for creating different types of visualizers.
 */
class VisualizationFactory {
public:
    /**
     * @brief Creates a visualizer of the specified type.
     * @param type The type of visualizer to create.
     * @return A unique pointer to the created visualizer.
     */
    static std::unique_ptr<IVisualizer> createVisualizer(VisualizationType type);
};

#endif // VISUALIZATIONFACTORY_H