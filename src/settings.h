#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 1000;
    float farPlane = 1;
    bool invert_filter = false;
    bool sharpen_filter = false;
    bool adaptive_detail = false;
    bool use_texture = false;
    bool grayscale_filter = false;
    bool blur_filter = false;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
