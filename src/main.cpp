#include <iostream>
#include <optional>
#include <vector>
#include <cmath> // for sqrt

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

using Point2D = sf::Vector2f;

// My DrawLine Function from my Project, adjusted so that window is in the parameters now
void DrawLine(Point2D from, Point2D to, float width, sf::Color c, sf::RenderWindow& window) {
    float deltaX = to.x - from.x;
    float deltaY = to.y - from.y;

    float length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    if (length == 0.0f) return;

    // Perpendicular direction vector (Δy, -Δx), normalized and scaled to half-width
    Point2D end = {
        (deltaY / length) * (width / 2.f),
        (-deltaX / length) * (width / 2.f)
    };

    Point2D p1Plus  = { from.x + end.x, from.y + end.y };
    Point2D p1Minus = { from.x - end.x, from.y - end.y };
    Point2D p2Plus  = { to.x + end.x,   to.y + end.y };
    Point2D p2Minus = { to.x - end.x,   to.y - end.y };

    // Now to draw the line
    sf::ConvexShape line;
    line.setPointCount(4);
    line.setPoint(0, sf::Vector2f(p1Plus.x, p1Plus.y));
    line.setPoint(1, sf::Vector2f(p2Plus.x, p2Plus.y));
    line.setPoint(2, sf::Vector2f(p2Minus.x, p2Minus.y));
    line.setPoint(3, sf::Vector2f(p1Minus.x, p1Minus.y));
    line.setFillColor(c);

    window.draw(line);


}

// Helper function to find the closest control point based on where the mouse clicked
// Returns the index of the closest point
int findClosestControlPoint(const std::vector<sf::Vector2f>& pts, Point2D mouse) {
    int best = 0;
    // Initialize the first point (pts[0]) to be the closest for now
    float bx = pts[0].x - mouse.x;
    float by = pts[0].y - mouse.y;
    float bestDist2 = bx * bx + by * by;

    // Iterate through each pts to find the closest point
    for (int i = 1; i < static_cast<int>(pts.size()); ++i) {
        float dx = pts[i].x - mouse.x;
        float dy = pts[i].y - mouse.y;
        float d2 = dx * dx + dy * dy;
        if (d2 < bestDist2) {
            bestDist2 = d2;
            best = i;
        }
    }
    return best;
}

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) { 
    // B(t) = (1−t)^3xP0 + 3(1−t)^2txP1 + 3(1−t)t^2xP2 + t^3xP3
    float u = 1.0f - t;

    float b0 = u * u * u;
    float b1 = 3.0f * u * u * t;
    float b2 = 3.0f * u * t * t;
    float b3 = t * t * t;

    return Point2D{
        b0 * pts[0].x + b1 * pts[1].x + b2 * pts[2].x + b3 * pts[3].x,
        b0 * pts[0].y + b1 * pts[1].y + b2 * pts[2].y + b3 * pts[3].y
    };
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) { 
    // dx/dt = 3(1-t)^2*(P1.x - P0.x) + 6(1-t)t x (P2.x - P1.x) + 3t^2(P3.x - P2.x)
    float u = 1.0f - t;
    float dxdt = (3.0f * (u * u)) * (pts[1].x - pts[0].x) +
                 (6.0f * u * t) * (pts[2].x - pts[1].x) +
                 (3 * t * t) * (pts[3].x - pts[2].x);
    // dy/dt = 3(1-t)^2 * (P1.y - P0.y) + 6(1-t)t * (P2.y - P1.y) + 3t^2(P3.y - P2.y)
    float dydt = (3.0f * (u * u)) * (pts[1].y - pts[0].y) +
                 (6.0f * u * t) * (pts[2].y - pts[1].y) +
                 (3.0f * t * t) * (pts[3].y - pts[2].y);

    return Point2D{dxdt, dydt}; 
}

// TODO: (Part 1) Store four control points for the curve.
std::vector<sf::Vector2f> pts = {
    {100.f, 100.f},
    {100.f, 500.f},
    {500.f, 100.f},
    {500.f, 500.f}
};
// TODO: (Part 2) Track animation time for the square moving along the curve.
float animationTime = 0.0f;
// TODO: (Part 3) Track the index of the control point being dragged.
int controlPointIndex = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            if (mouse->button == sf::Mouse::Button::Left) {
                Point2D m(mouse->position);
                controlPointIndex = findClosestControlPoint(pts, m);
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            if (mouse->button == sf::Mouse::Button::Left) {
                controlPointIndex = -1;
            }
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
            if (controlPointIndex != -1) {
                pts[controlPointIndex] = Point2D(mouse->position);
            }
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    
    // Draw the curve first
    const int segments = 100;
    Point2D prev = getPoint(pts, 0.0f);
    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        Point2D cur = getPoint(pts, t);
        DrawLine(prev, cur, 2.0f, sf::Color::White, window);
        prev = cur;
    }

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======
    DrawLine(pts[0], pts[1], 1.0f, sf::Color::Red, window);
    DrawLine(pts[2], pts[3], 1.0f, sf::Color::Red, window);

    // Draw the points as circles next
    const float r = 4.0f;
    for (const auto& p : pts) {
        sf::CircleShape dot(r);
        dot.setFillColor(sf::Color::Cyan);
        dot.setOrigin({r, r});
        dot.setPosition(p);
        window.draw(dot);
    }

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    // 100 frames at 30 FPS will be about 3.3 seconds per loop
    animationTime += 0.01f;
    if (animationTime > 1.0f) {
        animationTime = 0.0f;
    }

    Point2D pos = getPoint(pts, animationTime);

    sf::RectangleShape square({16.0f, 16.0f});
    square.setOrigin({8.0f, 8.0f});
    square.setPosition(pos);
    square.setFillColor(sf::Color::Yellow);

    // Adjust for rotations
    Point2D slope = getSlope(pts, animationTime);
    float angle = std::atan2(slope.y, slope.x);
    square.setRotation(sf::radians(angle));

    window.draw(square);

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
