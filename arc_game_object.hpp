#ifndef __ARC_GAME_OBJECT_H__
#define __ARC_GAME_OBJECT_H__

#include "arc_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace arc
{
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        // Matrix corresponds to translate * Ry * Rx * Rz * scale transformation
        // Rotation convention uses tait-bryan angles with axis order Y(1), X(2), Z(3)
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
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
        TransformComponent transform{};

    private:
        ArcGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}
#endif // __ARC_GAME_OBJECT_H__