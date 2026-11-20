//
// Created by ded on 20.11.2026.
//

#pragma once
#include "core/utility/system.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace cvulkan::physics::core {
    namespace layers {
        namespace object {
            static constexpr JPH::ObjectLayer NON_MOVING = 0;
            static constexpr JPH::ObjectLayer MOVING = 1;
            static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
        }
        namespace broad {
            static constexpr JPH::BroadPhaseLayer NON_MOVING{0};
            static constexpr JPH::BroadPhaseLayer MOVING{1};
            static constexpr JPH::uint NUM_LAYERS = 2;
        }
    }

    class CVulkanJoltSystem {
    public:
        CVulkanJoltSystem();
        ~CVulkanJoltSystem();

        CLASS_NO_COPY_NO_MOVE(CVulkanJoltSystem);
    };

    class CVulkanBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface {
    public:
        CVulkanBroadPhaseLayerInterface() = default;
        ~CVulkanBroadPhaseLayerInterface() override = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanBroadPhaseLayerInterface)

        JPH::uint GetNumBroadPhaseLayers() const override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;
    };

    class CVulkanObjectVsBroadPhaseFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        CVulkanObjectVsBroadPhaseFilter() = default;
        ~CVulkanObjectVsBroadPhaseFilter() override = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanObjectVsBroadPhaseFilter)

        bool ShouldCollide(JPH::ObjectLayer objectLayer,JPH::BroadPhaseLayer broadPhaseLayer) const override;
    };

    class CVulkanObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter {
    public:
        CVulkanObjectLayerPairFilter() = default;
        ~CVulkanObjectLayerPairFilter() override = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanObjectLayerPairFilter)

        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const override;
    };

    class CVulkanPhysicsWorld {
    public:
        CVulkanPhysicsWorld();
        ~CVulkanPhysicsWorld() = default;

        CLASS_NO_COPY_NO_MOVE(CVulkanPhysicsWorld)

        void update(float deltaTime);

    private:
        CVulkanBroadPhaseLayerInterface _broadPhaseLayerInterface;
        CVulkanObjectVsBroadPhaseFilter _objectVsBroadPhaseFilter;
        CVulkanObjectLayerPairFilter _objectLayerPairFilter;
        JPH::TempAllocatorImpl _tempAllocator;
        JPH::JobSystemThreadPool _jobSystem;
        JPH::PhysicsSystem _physicsSystem;
    };
}
