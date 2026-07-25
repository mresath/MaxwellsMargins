#include "graphing/Grapher.hpp"

Grapher::Grapher() = default;

void Grapher::plot(const Logger &logger, const std::vector<std::string> &quantityNames)
{
    // TODO(Phase 6): render selected quantities via matplot++
    (void)logger;
    (void)quantityNames;
}

void Grapher::saveAsImage(const std::string &path) const
{
    // TODO(Phase 6): save current plot to `path`
    (void)path;
}
