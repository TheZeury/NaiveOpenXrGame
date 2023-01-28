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
