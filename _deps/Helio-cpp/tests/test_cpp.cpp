/// Verifies that the C++ RAII header compiles correctly.
#include <helio/helio.hpp>
#include <cassert>
#include <cstdio>

int main() {
    // Handle
    helio::Handle h;
    assert(h.is_null());

    helio::Handle h2 = helio::Handle::null();
    assert(h == h2);

    // Vec3/mat4 helpers
    HelioVec3 v = helio::vec3(1, 2, 3);
    assert(v.x == 1 && v.y == 2 && v.z == 3);

    HelioMat4 m = helio::mat4_identity();
    assert(m.data[0][0] == 1.0f);
    assert(m.data[1][1] == 1.0f);
    assert(m.data[2][2] == 1.0f);
    assert(m.data[3][3] == 1.0f);

    // Camera
    HelioCameraDesc cam = helio::camera_perspective_look_at(
        0, 0, 0,    // position
        0, 0, -1,   // target
        1.5708f, 16.0f / 9.0f, 0.1f, 1000.0f);
    (void)cam;

    // MeshUpload move semantics
    helio::MeshUpload m1;
    helio::MeshUpload m2;
    m2 = std::move(m1); // NOLINT

    // Scene (constructor needs a device — can't test without one)
    // Editor, Picker — constructors don't need external resources
    helio::Editor editor;
    assert(!editor.is_dragging());
    assert(editor.gizmo_mode() == 0); // Translate
    assert(editor.hovered_axis() == -1); // None

    printf("Helio-cpp C++ RAII header compiles OK.\n");
    return 0;
}
