#ifndef __ARC_GAME_OBJECT_H__
#define __ARC_GAME_OBJECT_H__

#include "arc_model.hpp"

// std
#include <memory>

namespace arc
{
    struct Transform2DComponent
    {
        glm::vec2 translation{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation;

        glm::mat2 mat2()
        {
            const float sine = glm::sin(rotation);
            const float cosine = glm::cos(rotation);
            glm::mat2 rotMatrix{{cosine, sine}, {-sine, cosine}};

            glm::mat2 scaleMat{{scale.x, 0.f}, {0.f, scale.y}};
            return rotMatrix * scaleMat;
        }
    };

    class ArcGameObject
    {
    public:
        using id_t = unsigned int;

        static ArcGameObject createGameObject()
        {
            static id_t currentId = 0;
            return ArcGameObject{currentId++};
        }

        ArcGameObject(const ArcGameObject &) = delete;
        ArcGameObject &operator=(const ArcGameObject &) = delete;
        ArcGameObject(ArcGameObject &&) = default;
        ArcGameObject &operator=(ArcGameObject &&) = default;

        const id_t getID() const { return id; }

        std::shared_ptr<ArcModel> model{};
        glm::vec3 color{};
        Transform2DComponent transform2D;

    private:
        ArcGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}
#endif // __ARC_GAME_OBJECT_H__