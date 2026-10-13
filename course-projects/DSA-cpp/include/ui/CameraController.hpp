#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

namespace ui {

class CameraController {
public:
    CameraController()
        : view_(), zoomLevel_(1.0f), targetZoomLevel_(1.0f), isDragging_(false), lastMousePos_() {}

    void init(float viewportWidth, float viewportHeight) {
        viewportWidth_ = viewportWidth;
        viewportHeight_ = viewportHeight;
        view_.setSize({viewportWidth, viewportHeight});
        view_.setCenter({viewportWidth * 0.5f, viewportHeight * 0.5f});
        zoomLevel_ = 1.0f;
        targetZoomLevel_ = 1.0f;
        isZoomingSmoothly_ = false;
    }

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
        // Zoom via mouse wheel (smooth, responsive)
        if (const auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (wheel->wheel == sf::Mouse::Wheel::Vertical) {
                float zoomFactor = (wheel->delta > 0) ? 0.88f : 1.13f;
                float baseZoom = isZoomingSmoothly_ ? targetZoomLevel_ : zoomLevel_;
                float newTarget = baseZoom * zoomFactor;
                zoomTo(newTarget, 14.0f);
            }
        }

        // Pan via left mouse button
        if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseBtn->button == sf::Mouse::Button::Left) {
                isDragging_ = true;
                isFollowing_ = false;
                lastMousePos_ = mouseBtn->position;
            }
        }

        if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseBtn->button == sf::Mouse::Button::Left || mouseBtn->button == sf::Mouse::Button::Right) {
                isDragging_ = false;
            }
        }

        if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
            if (isDragging_) {
                sf::Vector2i currentPos = mouseMove->position;
                sf::Vector2f worldLast = window.mapPixelToCoords(lastMousePos_, view_);
                sf::Vector2f worldCurr = window.mapPixelToCoords(currentPos, view_);
                sf::Vector2f delta = worldLast - worldCurr;

                view_.move(delta);
                lastMousePos_ = currentPos;
            }
        }
    }

    void centerOn(float worldX, float worldY) {
        targetCenter_ = {worldX, worldY};
        view_.setCenter({worldX, worldY});
        isFollowing_ = false;
    }

    void panBy(float dx, float dy) {
        view_.move({dx, dy});
        targetCenter_ = view_.getCenter();
        isFollowing_ = false;
    }

    void setFollowTarget(sf::Vector2f target, float smoothness = 5.0f) {
        targetCenter_ = target;
        smoothness_ = smoothness;
        isFollowing_ = true;
    }

    void panTo(float worldX, float worldY, float smoothness = 8.5f) {
        targetCenter_ = {worldX, worldY};
        smoothness_ = smoothness;
        isFollowing_ = true;
    }

    void panTo(sf::Vector2f target, float smoothness = 8.5f) {
        panTo(target.x, target.y, smoothness);
    }

    void stopFollowing() noexcept {
        isFollowing_ = false;
    }

    void zoomTo(float targetZoom, float smoothness = 8.5f) {
        if (targetZoom <= 0.05f) return;
        targetZoomLevel_ = std::clamp(targetZoom, 1.0f / 4.50f, 1.0f / 0.20f);
        zoomSmoothness_ = smoothness;
        isZoomingSmoothly_ = true;
    }

    void zoomTowards(sf::Vector2f worldTarget, float zoomFactor, float smoothness = 14.0f) {
        float baseZoom = isZoomingSmoothly_ ? targetZoomLevel_ : zoomLevel_;
        float newTarget = std::clamp(baseZoom * zoomFactor, 1.0f / 4.50f, 1.0f / 0.20f);

        // Center camera smoothly onto the hovered target on minimap (both for Zoom IN and Zoom OUT)
        targetCenter_ = worldTarget;
        smoothness_ = smoothness;
        isFollowing_ = true;

        if (std::abs(newTarget - baseZoom) > 0.0001f) {
            targetZoomLevel_ = newTarget;
            zoomSmoothness_ = smoothness;
            isZoomingSmoothly_ = true;
        }
    }

    void update(float dt) {
        if (isDragging_ && !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            isDragging_ = false;
        }

        // 1. Smooth position panning
        if (isFollowing_ && !isDragging_) {
            sf::Vector2f current = view_.getCenter();
            float dist = std::hypot(targetCenter_.x - current.x, targetCenter_.y - current.y);
            if (dist < 0.25f) {
                view_.setCenter(targetCenter_);
                isFollowing_ = false;
            } else {
                float factor = 1.0f - std::exp(-smoothness_ * dt);
                view_.setCenter({
                    current.x + (targetCenter_.x - current.x) * factor,
                    current.y + (targetCenter_.y - current.y) * factor
                });
            }
        }

        // 2. Smooth zoom interpolation
        if (isZoomingSmoothly_) {
            float diff = std::abs(targetZoomLevel_ - zoomLevel_);
            if (diff < 0.001f) {
                zoomLevel_ = targetZoomLevel_;
                isZoomingSmoothly_ = false;
            } else {
                float factor = 1.0f - std::exp(-zoomSmoothness_ * dt);
                zoomLevel_ = zoomLevel_ + (targetZoomLevel_ - zoomLevel_) * factor;
            }
            if (viewportWidth_ > 0.0f && viewportHeight_ > 0.0f) {
                view_.setSize({viewportWidth_ * zoomLevel_, viewportHeight_ * zoomLevel_});
            }
        }
    }

    void setZoomLevel(float zoom) {
        if (zoom <= 0.05f) return;
        zoomLevel_ = std::clamp(zoom, 1.0f / 4.50f, 1.0f / 0.20f);
        targetZoomLevel_ = zoomLevel_;
        isZoomingSmoothly_ = false;
        if (viewportWidth_ > 0.0f && viewportHeight_ > 0.0f) {
            view_.setSize({viewportWidth_ * zoomLevel_, viewportHeight_ * zoomLevel_});
        }
    }

    void clampCenter(float minX, float minY, float maxX, float maxY) {
        sf::Vector2f center = view_.getCenter();
        center.x = std::clamp(center.x, minX, maxX);
        center.y = std::clamp(center.y, minY, maxY);
        view_.setCenter(center);
        targetCenter_.x = std::clamp(targetCenter_.x, minX, maxX);
        targetCenter_.y = std::clamp(targetCenter_.y, minY, maxY);
    }

    void setViewport(const sf::FloatRect& rect) {
        view_.setViewport(rect);
    }

    void updateViewportSize(float viewportWidth, float viewportHeight) {
        viewportWidth_ = viewportWidth;
        viewportHeight_ = viewportHeight;
        view_.setSize({viewportWidth_ * zoomLevel_, viewportHeight_ * zoomLevel_});
    }

    void fitMaze(float mazeWidthWorld, float mazeHeightWorld, float availableWidth, float availableHeight, float padding = 1.15f, bool smooth = true) {
        if (availableWidth <= 10.0f || availableHeight <= 10.0f) return;

        float zoomX = (mazeWidthWorld * padding) / availableWidth;
        float zoomY = (mazeHeightWorld * padding) / availableHeight;
        float targetZoom = std::clamp(std::max(zoomX, zoomY), 1.0f / 4.50f, 1.0f / 0.20f);

        if (smooth) {
            zoomTo(targetZoom, 8.0f);
            panTo(mazeWidthWorld * 0.5f, mazeHeightWorld * 0.5f, 8.0f);
        } else {
            setZoomLevel(targetZoom);
            centerOn(mazeWidthWorld * 0.5f, mazeHeightWorld * 0.5f);
        }
    }

    [[nodiscard]] sf::View& getView() noexcept { return view_; }
    [[nodiscard]] const sf::View& getView() const noexcept { return view_; }
    [[nodiscard]] float getZoomLevel() const noexcept { return zoomLevel_; }
    [[nodiscard]] float getTargetZoomLevel() const noexcept { return targetZoomLevel_; }
    [[nodiscard]] bool isZoomingSmoothly() const noexcept { return isZoomingSmoothly_; }
    [[nodiscard]] bool isDragging() const noexcept { return isDragging_; }
    [[nodiscard]] bool isFollowing() const noexcept { return isFollowing_; }

private:
    sf::View view_;
    float zoomLevel_{1.0f};
    float targetZoomLevel_{1.0f};
    float zoomSmoothness_{8.5f};
    bool isZoomingSmoothly_{false};
    float viewportWidth_{1600.0f};
    float viewportHeight_{900.0f};

    bool isDragging_{false};
    sf::Vector2i lastMousePos_{};
    sf::Vector2f targetCenter_{0.0f, 0.0f};
    float smoothness_{5.0f};
    bool isFollowing_{false};
};

} // namespace ui