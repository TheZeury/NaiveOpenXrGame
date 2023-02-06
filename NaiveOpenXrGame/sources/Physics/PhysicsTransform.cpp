#include "PhysicsTransform.h"

std::tuple<bool, glm::mat4*> Noxg::PhysicsTransform::updateMatrix()
{
    auto [updated, newGlobalMatrix] = Transform::updateMatrix();
    if (updated)    // No too much difference with Transform::updateMatrix but need to update the pxActor if needed.
    {
        informChangesToActor();
    }
    return std::make_tuple(updated, newGlobalMatrix);
}

void Noxg::PhysicsTransform::setGlobalMatrix(const glm::mat4& mat)
{
    Transform::setGlobalMatrix(mat);
    informChangesToActor(); // No too much difference with Transform::setGlobalMatrix but need to update the pxActor after that.
}

void Noxg::PhysicsTransform::CalculateFrame()
{
    PxMat44 pxMat{ pxActor->getGlobalPose() };
    glm::mat4 physicsTrans = (pxActor == nullptr) ? glm::mat4{ 1.f } : (*((glm::mat4*)(&pxMat)));
    Transform::setGlobalMatrix(physicsTrans);   // Since we don't want to update the pxActor, use Transform::setGlobalMatrix instead.
}

void Noxg::PhysicsTransform::informChangesToActor()
{
    if (pxActor == nullptr) return;
    pxActor->setGlobalPose(PxTransform(*((PxMat44*)(&globalMatrix))));
}
