#pragma once
#include "BaseComponent.h"
#include <vector>
#include <chrono>
#include <imgui.h>

struct DataPoint
{
    float stepSize;
    float duration; // microseconds
};

namespace dae
{
    class BenchmarkComponent final : public BaseComponent
    {
    public:
        BenchmarkComponent(GameObject& owner) : BaseComponent(owner) {}
        void Update() override;
        void Render() override;

    private:
        // Exercise 1 + 2
        std::vector<DataPoint> RunIntBufferTest();
        std::vector<DataPoint> RunGameObject3DTest();
        std::vector<DataPoint> RunGameObject3DAltTest();

        void PlotResult(const char* label, std::vector<DataPoint>& data);

        std::vector<DataPoint> m_IntResults;
        std::vector<DataPoint> m_GameObject3DResults;
        std::vector<DataPoint> m_GameObject3DAltResults;

        bool m_HasRun{ false };
    };
}