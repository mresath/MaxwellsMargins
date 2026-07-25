#pragma once

#include "Config.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <cmath>

// View/window helpers (pan, zoom, resize, letterboxing) shared by App.

inline sf::FloatRect calculateLetterboxViewport(const sf::Vector2u &windowSize, const sf::Vector2f &viewSize)
{
    const float windowAspect = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
    const float viewAspect = viewSize.x / viewSize.y;

    float viewportWidth = 1.0f;
    float viewportHeight = 1.0f;
    float viewportLeft = 0.0f;
    float viewportTop = 0.0f;

    if (windowAspect > viewAspect)
    {
        viewportWidth = viewAspect / windowAspect;
        viewportLeft = (1.0f - viewportWidth) * 0.5f;
    }
    else if (windowAspect < viewAspect)
    {
        viewportHeight = windowAspect / viewAspect;
        viewportTop = (1.0f - viewportHeight) * 0.5f;
    }

    return sf::FloatRect(sf::Vector2f(viewportLeft, viewportTop), sf::Vector2f(viewportWidth, viewportHeight));
}

inline void clampViewToWorld(sf::View *view)
{
    sf::Vector2f size = view->getSize();
    size.x = std::min(size.x, MAX_VIEW_WIDTH);
    size.y = std::min(size.y, MAX_VIEW_HEIGHT);
    view->setSize(size);

    sf::Vector2f center = view->getCenter();
    const float halfWidth = size.x * 0.5f;
    const float halfHeight = size.y * 0.5f;

    if (size.x >= MAX_VIEW_WIDTH)
    {
        center.x = DEF_WIDTH * 0.5f;
    }
    else
    {
        center.x = std::clamp(center.x, DEF_WIDTH * 0.5f - MAX_VIEW_WIDTH * 0.5f + halfWidth, DEF_WIDTH * 0.5f + MAX_VIEW_WIDTH * 0.5f - halfWidth);
    }

    if (size.y >= MAX_VIEW_HEIGHT)
    {
        center.y = DEF_HEIGHT * 0.5f;
    }
    else
    {
        center.y = std::clamp(center.y, DEF_HEIGHT * 0.5f - MAX_VIEW_HEIGHT * 0.5f + halfHeight, DEF_HEIGHT * 0.5f + MAX_VIEW_HEIGHT * 0.5f - halfHeight);
    }

    view->setCenter(center);
}

inline void handleResize(sf::Window *window, sf::View *newView, sf::View *oldView, const sf::Event::Resized *resized)
{
    (void)window;
    (void)oldView;
    (void)resized;

    clampViewToWorld(newView);
    newView->setViewport(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1.0f, 1.0f)));
}

inline void handleZoom(sf::RenderWindow *window, sf::View *newView, sf::View *oldView, float delta, float *accumulatedZoom)
{
    auto viewSize = oldView->getSize();
    // Scaled by the scroll delta's own magnitude, not just its sign - otherwise a fast
    // trackpad flick zooms by the same tiny step as a single slow tick, which reads as choppy.
    const float zoomAmount = std::min(ZOOM_STEP * std::abs(delta), 0.5f);
    float decFactor = 1.0f - zoomAmount;
    float incFactor = 1.0f + zoomAmount;

    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(*window);
    sf::Vector2f mouseWorldPosBefore = window->mapPixelToCoords(mousePixelPos, *oldView);

    if (delta > 0 && viewSize.x * decFactor >= DEF_WIDTH / 2.5f && viewSize.y * decFactor >= DEF_HEIGHT / 2.5f)
    {
        newView->zoom(decFactor);
        *accumulatedZoom /= incFactor;
    }
    else if (delta < 0 && viewSize.x * incFactor <= MAX_VIEW_WIDTH && viewSize.y * incFactor <= MAX_VIEW_HEIGHT)
    {
        newView->zoom(incFactor);
        *accumulatedZoom /= decFactor;
    }

    sf::Vector2f mouseWorldPosAfter = window->mapPixelToCoords(mousePixelPos, *newView);
    sf::Vector2f offset = mouseWorldPosBefore - mouseWorldPosAfter;
    newView->move(offset);

    clampViewToWorld(newView);
}

inline void handlePanMouse(sf::RenderWindow *window, sf::View *newView, sf::View *oldView, const sf::Event::MouseMoved *mouseMoved, sf::Vector2f &lastMousePos, float accumulatedZoom)
{
    (void)window;
    (void)oldView;

    sf::Vector2f currentMousePos = sf::Vector2f(mouseMoved->position.x, mouseMoved->position.y);
    sf::Vector2f deltaPos = lastMousePos - currentMousePos;
    deltaPos *= accumulatedZoom;

    newView->move(deltaPos);
    lastMousePos = currentMousePos;

    clampViewToWorld(newView);
}
