// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include "main/loquat.h"

namespace loquat
{

    class Interaction
    {
        Interaction() = default;
        Interaction(Point3fi point, Normal3f normal, Point2f uv, Vec3f outgoing,
                    Float time)
            : point{ point }
            , normal{ normal }
            , uv{ uv }
            , outgoing{ glm::normalize(outgoing) }
            , time{ time }
        {}
        
        Point3f p() const noexcept
        {
            return Point3f{point};
        }
        
        bool is_surface_interaction() const noexcept
        {
            return n != Normal3f{0, 0, 0};
        }
        
        bool is_medium_interaction() const noexcept
        {
            return !is_surface_interaction();
        }
        
        const SurfaceInteraction& as_surface() const
        {
            LOG_ASSERT(is_surface_interaction());
            return static_cast<const SurfaceInteraction&>(*this);
        }
        
        SurfaceInteraction& as_surface() const
        {
            LOG_ASSERT(is_surface_interaction());
            return static_cast<SurfaceInteraction&>(*this);
        }
        
        Interaction(Point3f point, Vector3f outgoing, Float time, Medium medium)
            : point{ point }
            , time{ time }
            , outgoing{ outgoing }
            , medium{ medium }
        {}
        
        Interaction(Point3f point, Normal3f normal, Float time, Medium medium)
            : pi(point)
            , normal{ normal }
            , time{ time }
            , medium{ medium }
        {}
        
        Interaction(Point3f point, Point2f uv)
            : pi(point)
            , uv{ uv }
        {}
        
        Interaction(const Point3fi& point, Normal3f normal, Float time = 0, Point2f uv = {})
            : point{ point }
            , normal{ normal }
            , uv{ uv }
            , time{ time }
        {}
        
        Interaction(const Point3fi& point, Normal3f normal, Point2f uv)
            : point{ point }
            , normal{ normal }
            , uv{ uv }
        {}
        
        Interaction(Point3f point, Float time, Medium medium)
            : point{ point }
            , time{ time }
            , medium{ medium }
        {}
        
        Interaction(Point3f point, const MediumInterface* medium_interface)
            : point{ point }
            , medium_interface{ medium_interface }
        {}
        
        Interaction(Point3f point, Float time, const MediumInterface* medium_interface)
            : point{ point }
            , time{ time }
            , medium_interface{ medium_interface }
        {}
        
        const MediumInteraction& as_medium() const
        {
            LOG_ASSERT(is_medium_interaction());
            return static_cast<const MediumInteraction&>(*this);
        }
        
        MediumInteraction& as_medium() const
        {
            LOG_ASSERT(is_medium_interaction());
            return static_cast<MediumInteraction&>(*this);
        }
        
        [[nodiscard]]
        std::string to_string() const noexcept;
        
        Point3f offset_ray_origin(Vec3f direction) const noexcept
        {
            return loquat::offset_ray_origin(point, normal, direction);
        }
        
        Point3f offset_ray_origin(Point3f point2) const noexcept
        {
            return offset_ray_origin(point2 - p());
        }
        
        RayDifferential spawn_ray(Vec3f direction) const noexcept
        {
            return RayDifferential{offset_ray_origin(direction), direction, time, get_medium(direction)};
        }
        
        Ray spawn_ray_to(Point3f point2) const noexcept
        {
            Ray r = loquat::spawn_ray_to(point, normal, time, point2);
            ray.medium = get_medium(ray.direction);
            return r;
        }
        
        Ray spawn_ray_to(const Interaction& interaction) const noexcept
        {
            Ray r = loquat::spawn_ray_to(point, normal, time, interaction.point, interaction.normal);
            ray.medium = get_medium(ray.direction);
            return r;
        }
        
        Medium get_medium(Vec3f direction) const noexcept
        {
            if (medium_interface)
            {
                return glm::dot(direction, normal) > 0
                    ? medium_interface->outside
                    : medium_interface->inside;
            }
            return medium;
        }
        
        Medium get_medium() const noexcept
        {
            if (medium_interface)
            {
                LOG_ASSERT(medium_interface->outside == medium_interface->inside);
            }
            return medium_interface ? medium_interface->inside : medium;
        }
        
        Point3fi point;
        Float time = 0;
        Vector3f outgoing;
        Normal3f normal;
        Point2f uv;
        const MediumInterface* medium_interface = nullptr;
        Medium medium = nullptr;
    }
}
