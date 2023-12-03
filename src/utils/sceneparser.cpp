#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

glm::mat4 calculate_new_ctm(glm::mat4 ctm, std::vector<SceneTransformation*> transformations)
{
    for (SceneTransformation* t : transformations)
    {
        switch (t->type)
        {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm = ctm * glm::translate(t->translate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm = ctm * t->matrix;
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            ctm = ctm * glm::rotate(t->angle, t->rotate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm = ctm * glm::scale(t->scale);
            break;
        default:
            break;
        }
    }
    return ctm;
}

void dfs_scene_graph(RenderData &renderData, SceneNode* node, glm::mat4 ctm)
{
    glm::mat4 newCtm = calculate_new_ctm(ctm, node->transformations);

    if (!node->lights.empty())
    {
        for (SceneLight* l : node->lights)
        {
            glm::vec4 pos = newCtm * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
            renderData.lights.push_back(SceneLightData{l->id, l->type, l->color, l->function, pos, newCtm * l->dir, l->penumbra, l->angle, l->width, l->height});
        }
    }

    std::vector<SceneNode*> child =  node->children;
    int n = child.size();
    if (!node->primitives.empty())
    {
        for (auto p : node->primitives)
            renderData.shapes.push_back(RenderShapeData{*p, newCtm, glm::inverse(newCtm)});
        return;
    }

    for (int i = 0; i < n; i++)
        dfs_scene_graph(renderData, child[i], newCtm);
}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();
    dfs_scene_graph(renderData, root, glm::mat4(1.0f));

    return true;
}
