#pragma once
#include <functional>

namespace dae
{
    class Command
    {
    public:
        virtual ~Command() = default;
        virtual void Execute() = 0;
    };

    class LambdaCommand final : public Command
    {
    public:
        explicit LambdaCommand(std::function<void()> fn) : m_fn{ std::move(fn) } {}
        void Execute() override { if (m_fn) m_fn(); }
    private:
        std::function<void()> m_fn;
    };
}
