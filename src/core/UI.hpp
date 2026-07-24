#pragma once

#include "Config.hpp"

#include <SFML/Graphics.hpp>

// View/window helpers (pan, zoom, resize, letterboxing) shared by App. Mirrors the
// equivalent helpers in Newton's Notepad and Schrodinger's Sketchbook.
// TODO(Phase 1): implement bodies (see NewtonsNotepad/src/core/UI.hpp and
// SchrodingersSketchbook/src/core/App.cpp for reference implementations).
void handleResize(sf::Window *window, sf::View *newView, const sf::View *oldView, const sf::Event::Resized *resized);
void handleZoom(sf::RenderWindow *window, sf::View *newView, const sf::View *oldView, float delta, float *accumulatedZoom);
void handlePanMouse(sf::RenderWindow *window, sf::View *newView, const sf::View *oldView, const sf::Event::MouseMoved *mouseMoved, sf::Vector2f lastMousePos, float accumulatedZoom);
sf::FloatRect calculateLetterboxViewport(sf::Vector2u windowSize, sf::Vector2f viewSize);
