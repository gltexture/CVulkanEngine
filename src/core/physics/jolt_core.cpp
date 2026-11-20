//
// Created by ded on 20.11.2026.
//

#include "jolt_core.h"
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/RegisterTypes.h>

namespace cvulkan::physics::core {
    CVulkanJoltSystem::CVulkanJoltSystem() {
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
    }

    CVulkanJoltSystem::~CVulkanJoltSystem() {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    JPH::uint CVulkanBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const {
        return layers::broad::NUM_LAYERS;
    }

    JPH::BroadPhaseLayer CVulkanBroadPhaseLayerInterface::GetBroadPhaseLayer(const JPH::ObjectLayer layer) const {
        switch (layer) {
            case layers::object::NON_MOVING:
                return layers::broad::NON_MOVING;
            case layers::object::MOVING:
                return layers::broad::MOVING;
            default:
                return layers::broad::NON_MOVING;
        }
    }

    bool CVulkanObjectVsBroadPhaseFilter::ShouldCollide(const JPH::ObjectLayer objectLayer, const JPH::BroadPhaseLayer broadPhaseLayer) const {
        if (objectLayer == layers::object::NON_MOVING) {
            return broadPhaseLayer == layers::broad::MOVING;
        }
        return true;
    }

    bool CVulkanObjectLayerPairFilter::ShouldCollide(const JPH::ObjectLayer layer1, const JPH::ObjectLayer layer2) const {
        if (layer1 == layers::object::NON_MOVING) {
            return layer2 == layers::object::MOVING;
        }
        return true;
    }

    CVulkanPhysicsWorld::CVulkanPhysicsWorld()
        : _tempAllocator(10 * 1024 * 1024), _jobSystem(
              JPH::cMaxPhysicsJobs,
              JPH::cMaxPhysicsBarriers,
              static_cast<int>(std::max(1u, std::thread::hardware_concurrency() - 2))
          )
    {
        this->_physicsSystem.Init(65536, 0, 65536, 10240, this->_broadPhaseLayerInterface, this->_objectVsBroadPhaseFilter, this->_objectLayerPairFilter);
        this->_physicsSystem.SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
    }

    void CVulkanPhysicsWorld::update(const float deltaTime) {
        this->_physicsSystem.Update(deltaTime, 1, &this->_tempAllocator, &this->_jobSystem);
    }
}
