#pragma once

#include "Config.hpp"

#include <SFML/Graphics.hpp>

#include <algorithm>

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
    size.x = std::min(size.x, DEF_WIDTH);
    size.y = std::min(size.y, DEF_HEIGHT);
    view->setSize(size);

    sf::Vector2f center = view->getCenter();
    const float halfWidth = size.x * 0.5f;
    const float halfHeight = size.y * 0.5f;

    if (size.x >= DEF_WIDTH)
    {
        center.x = DEF_WIDTH * 0.5f;
    }
    else
    {
        center.x = std::clamp(center.x, halfWidth, DEF_WIDTH - halfWidth);
    }

    if (size.y >= DEF_HEIGHT)
    {
        center.y = DEF_HEIGHT * 0.5f;
    }
    else
    {
        center.y = std::clamp(center.y, halfHeight, DEF_HEIGHT - halfHeight);
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
    float decFactor = 1 - ZOOM_STEP;
    float incFactor = 1 + ZOOM_STEP;

    sf::Vector2i mousePixelPos = sf::Mouse::getPosition(*window);
    sf::Vector2f mouseWorldPosBefore = window->mapPixelToCoords(mousePixelPos, *oldView);

    if (delta > 0 && viewSize.x * decFactor >= DEF_WIDTH / 2.5f && viewSize.y * decFactor >= DEF_HEIGHT / 2.5f)
    {
        newView->zoom(decFactor);
        *accumulatedZoom /= incFactor;
    }
    else if (delta < 0 && viewSize.x * incFactor <= DEF_WIDTH && viewSize.y * incFactor <= DEF_HEIGHT)
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
