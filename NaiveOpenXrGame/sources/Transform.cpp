#include "Transform.h"

glm::vec3 Noxg::Transform::getPosition()
{
    return position;
}

void Noxg::Transform::setPosition(glm::vec3 pos)
{
    position = pos;
    changed = true;
}

glm::quat Noxg::Transform::getRotation()
{
    return rotation;
}

void Noxg::Transform::setRotation(glm::quat rotat)
{
    rotation = rotat;
    changed = true;
}

glm::vec3 Noxg::Transform::getScale()
{
    return scale;
}

void Noxg::Transform::setScale(glm::vec3 scal)
{
    scale = scal;
    changed = true;
}

glm::mat4 Noxg::Transform::getMatrix()
{
    if (changed)
    {
        auto matTranslate = glm::translate(glm::mat4{ 1.f }, position);
        auto matScale = glm::scale(glm::mat4{ 1.f }, scale);
        auto matRotate = glm::mat4_cast(rotation);
        matrix = matTranslate * matScale * matRotate;
        changed = false;
    }
    return matrix;
}

void Noxg::Transform::setMatrix(const glm::mat4& mat)
{
    XrMatrix4x4f_GetTranslation((XrVector3f*)&position, (XrMatrix4x4f*)&mat);
    XrMatrix4x4f_GetRotation((XrQuaternionf*)&rotation, (XrMatrix4x4f*)&mat);
    XrMatrix4x4f_GetScale((XrVector3f*)&scale, (XrMatrix4x4f*)&mat);
    changed = false;
}

glm::vec3 Noxg::XrSpaceTransform::getPosition()
{
    if (spaceLocation.locationFlags & xr::SpaceLocationFlagBits::PositionValid)
    {
        return *((glm::vec3*)(&(spaceLocation.pose.position))) + attach.getRotation() * attach.getPosition();
    }
    return attach.getPosition();
}

glm::quat Noxg::XrSpaceTransform::getRotation()
{
    if (spaceLocation.locationFlags & xr::SpaceLocationFlagBits::OrientationValid)
    {
        return *((glm::quat*)(&(spaceLocation.pose.orientation))) * attach.getRotation();
    }
    return attach.getRotation();
}

glm::mat4 Noxg::XrSpaceTransform::getMatrix()
{
    auto translate = *((glm::vec3*)(&(spaceLocation.pose.position)));
    auto rotation = *((glm::quat*)(&(spaceLocation.pose.orientation)));
    auto matTranslate = glm::translate(glm::mat4{ 1.f }, translate);
    auto matRotate = glm::mat4_cast(rotation);
    return matTranslate * matRotate * attach.getMatrix();
}
