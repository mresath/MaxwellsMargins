#include "core/Tools.hpp"

#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"

Tools::Tools() : m_activeTool(ToolType::Select)
{
}

ToolType Tools::activeTool() const
{
    return m_activeTool;
}

void Tools::setActiveTool(ToolType tool)
{
    m_activeTool = tool;
}

void Tools::onClick(const Vec2 &worldPos, World &world)
{
    // TODO(Phase 2+): create/select entities in `world` based on m_activeTool
    (void)worldPos;
    (void)world;
}

void Tools::onClick(const Vec2 &worldPos, CircuitGraph &circuit)
{
    // TODO(Phase 5): create/select components in `circuit` based on m_activeTool
    (void)worldPos;
    (void)circuit;
}
