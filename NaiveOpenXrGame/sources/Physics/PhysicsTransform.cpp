#include "PhysicsTransform.h"

glm::vec3 Noxg::PhysicsTransform::getPosition()
{
    if (pxActor == nullptr) return { 0.f, 0.f, 0.f };
    PxVec3 pxPos = pxActor->getGlobalPose().p;
    return *((glm::vec3*)(&pxPos));
}

void Noxg::PhysicsTransform::setPosition(glm::vec3 pos)
{
    
}

glm::quat Noxg::PhysicsTransform::getRotation()
{
    if (pxActor == nullptr) return { 1.f, 0.f, 0.f, 0.f };
    PxQuat pxQuat = pxActor->getGlobalPose().q;
    return *((glm::quat*)(&pxQuat));
}

void Noxg::PhysicsTransform::setRotation(glm::quat rotat)
{
}

glm::vec3 Noxg::PhysicsTransform::getScale()
{
    return scale;
}

void Noxg::PhysicsTransform::setScale(glm::vec3 scal)
{
    scale = scal;
}

glm::mat4 Noxg::PhysicsTransform::getMatrix()
{
    if (pxActor == nullptr) return glm::mat4{ 1.f };
    PxMat44 pxMat{ pxActor->getGlobalPose() };
    glm::mat4 scal = glm::scale(glm::mat4{ 1.f }, scale);
    return (*((glm::mat4*)(&pxMat))) * scal;
}

void Noxg::PhysicsTransform::setMatrix(const glm::mat4& mat)
{
    if (pxActor == nullptr) return;
    pxActor->setGlobalPose(PxTransform(*((PxMat44*)(&mat))));
}
