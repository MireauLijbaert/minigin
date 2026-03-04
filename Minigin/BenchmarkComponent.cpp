#include "BenchmarkComponent.h"
#include <algorithm>

struct Transform
{
    float matrix[16] = 
    { 1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      0,0,0,1 };
};

struct GameObject3D
{
    Transform transform;
    int ID{ 1 };
};

struct GameObject3DAlt
{
    std::vector<Transform> transforms;
    std::vector<int> IDs;

    GameObject3DAlt(size_t count) : transforms(count), IDs(count, 1) {}
};

static float PlotGetter(void* data, int idx)
{
    auto* vec = static_cast<std::vector<DataPoint>*>(data);
    return (*vec)[idx].duration;
}

void dae::BenchmarkComponent::Update()
{
    if (!m_HasRun)
    {
        m_IntResults = RunIntBufferTest();
        m_GameObject3DResults = RunGameObject3DTest();
        m_GameObject3DAltResults = RunGameObject3DAltTest();
        m_HasRun = true;
    }
}

void dae::BenchmarkComponent::Render()
{
    if (!m_HasRun) return; // ensure data exists

    ImGui::Begin("Benchmarks");
    PlotResult("Int Buffer", m_IntResults);
    PlotResult("GameObject3D", m_GameObject3DResults);
    PlotResult("GameObject3DAlt", m_GameObject3DAltResults);
    ImGui::End();
}

std::vector<DataPoint> dae::BenchmarkComponent::RunIntBufferTest()
{
    std::vector<DataPoint> results;
    const size_t bufferSize = 100;
    std::vector<int> buffer(bufferSize, 1);

    for (int step = 1; step <= 1024; step *= 2)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < bufferSize; i += step)
            buffer[i] *= 2;
        auto end = std::chrono::high_resolution_clock::now();

        float duration = std::chrono::duration<float, std::micro>(end - start).count();
        results.push_back({ float(step), duration });
    }

    return results;
}

std::vector<DataPoint> dae::BenchmarkComponent::RunGameObject3DTest()
{
    std::vector<DataPoint> results;
    const size_t count = 100;
    std::vector<GameObject3D> objects(count);

    for (int step = 1; step <= 1024; step *= 2)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < count; i += step)
            objects[i].ID *= 2;
        auto end = std::chrono::high_resolution_clock::now();

        float duration = std::chrono::duration<float, std::micro>(end - start).count();
        results.push_back({ float(step), duration });
    }

    return results;
}

std::vector<DataPoint> dae::BenchmarkComponent::RunGameObject3DAltTest()
{
    std::vector<DataPoint> results;
    const size_t count = 100;
    GameObject3DAlt objects(count);

    for (int step = 1; step <= 1024; step *= 2)
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < count; i += step)
            objects.IDs[i] *= 2;
        auto end = std::chrono::high_resolution_clock::now();

        float duration = std::chrono::duration<float, std::micro>(end - start).count();
        results.push_back({ float(step), duration });
    }

    return results;
}

void dae::BenchmarkComponent::PlotResult(const char* label, std::vector<DataPoint>& data)
{
    if (data.empty()) return;

    float maxVal = std::max_element(
        data.begin(),
        data.end(),
        [](const DataPoint& a, const DataPoint& b) { return a.duration < b.duration; }
    )->duration;

    ImGui::Text("%s", label);
    ImGui::PlotLines(label, PlotGetter, &data, static_cast<int>(data.size()), 0, "us", 0.0f, maxVal, ImVec2(0, 150));

    for (auto& dp : data)
        ImGui::Text("Step %.0f -> %.2f us", dp.stepSize, dp.duration);

    ImGui::Separator();
}