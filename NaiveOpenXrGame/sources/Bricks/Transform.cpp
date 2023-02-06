#include "Transform.h"

std::tuple<bool, glm::mat4*> Noxg::Transform::updateMatrix()
{
    auto pa = parent.lock();
    bool updated;

    if (pa == nullptr)
    {
        updated = globalChanged;
        if(updated)
        {
            globalMatrix = getLocalMatrix();
        }
    }
    else
    {
        auto [parentUpdated, parentTransform] = pa->updateMatrix();
        updated = parentUpdated || globalChanged;
        if (updated)
        {
            globalMatrix = *parentTransform * getLocalMatrix();
        }
    }

    if (updated)
    {
        for (auto& _child : children)
        {
            auto child = _child.lock();
            if (child != nullptr)
            {
                child->globalChanged = true;
            }
        }
    }
    globalChanged = false;
    return std::make_tuple(updated, &globalMatrix);
}

glm::mat4 Noxg::Transform::getGlobalMatrix()
{
    updateMatrix();
    return globalMatrix;
}

void Noxg::Transform::setGlobalMatrix(const glm::mat4& mat)
{
    auto pa = parent.lock();
    globalMatrix = mat;
    if (pa == nullptr)
    {
        setLocalMatrix(mat);
    }
    else
    {
        auto [parentUpdated, parentMatrix] = pa->updateMatrix();
        setLocalMatrix(glm::inverse(*parentMatrix) * mat);
    }
    for (auto& _child : children)
    {
        auto child = _child.lock();
        if (child != nullptr)
        {
            child->globalChanged = true;
        }
    }
    globalChanged = false;
}

glm::vec3 Noxg::Transform::getLocalPosition()
{
    return localPosition;
}

void Noxg::Transform::setLocalPosition(glm::vec3 pos)
{
    localPosition = pos;
    localChanged = true;
    globalChanged = true;
}

glm::quat Noxg::Transform::getLocalRotation()
{
    return localRotation;
}

void Noxg::Transform::setLocalRotation(glm::quat rotat)
{
    localRotation = rotat;
    localChanged = true;
    globalChanged = true;
}

glm::vec3 Noxg::Transform::getLocalScale()
{
    return localScale;
}

void Noxg::Transform::setLocalScale(glm::vec3 scal)
{
    localScale = scal;
    localChanged = true;
    globalChanged = true;
}

glm::mat4 Noxg::Transform::getLocalMatrix()
{
    if (localChanged)
    {
        auto matTranslate = glm::translate(glm::mat4{ 1.f }, localPosition);
        auto matScale = glm::scale(glm::mat4{ 1.f }, localScale);
        auto matRotate = glm::mat4_cast(localRotation);
        localMatrix = matTranslate * matScale * matRotate;
        localChanged = false;
    }
    return localMatrix;
}

void Noxg::Transform::setLocalMatrix(const glm::mat4& mat)
{
    XrMatrix4x4f_GetTranslation((XrVector3f*)&localPosition, (XrMatrix4x4f*)&mat);
    XrMatrix4x4f_GetRotation((XrQuaternionf*)&localRotation, (XrMatrix4x4f*)&mat);
    XrMatrix4x4f_GetScale((XrVector3f*)&localScale, (XrMatrix4x4f*)&mat);
    localChanged = false;
    globalChanged = true;
}