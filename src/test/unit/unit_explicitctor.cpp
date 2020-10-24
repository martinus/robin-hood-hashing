#include <app/doctest.h>
#include <robin_hood.h>

#include <iostream>
#include <unordered_map>
#include <vector>

#if ROBIN_HOOD(CXX) >= ROBIN_HOOD(CXX14)

struct Texture {
    int width;
    int height;
    void* data;
};

struct PerImage {
    std::unordered_map<void*, Texture*> textureIndex;
};

struct Scene {
    std::vector<PerImage> perImage;
    robin_hood::unordered_map<void*, Texture*> texturesPerKey;
};

struct AppState {
    Scene scene;
};

TEST_CASE("unit_create_AppState (issue #97)") {
    AppState appState{};
}

#endif
