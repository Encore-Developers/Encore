#include "label.h"


using namespace UI;
Label::Label() {
    buffer = hb_buffer_create();
    hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
    hb_buffer_set_language(buffer, hb_language_from_string("en", -1));
}
void Label::SetFont(hb_face_t *face) {
    if (hbfont != nullptr) {
        hb_font_destroy(hbfont);
    }
    hbfont = hb_font_create(face);
}
void Label::BuildMesh(const RenderState &state) {
    unsigned int glyphCount;
    auto glyphInfo = hb_buffer_get_glyph_infos(buffer, &glyphCount);
    auto glyphPos = hb_buffer_get_glyph_positions(buffer, &glyphCount);

    glm::vec<2, hb_position_t> cursor = {0, 0};
}
void Label::Draw(const RenderState &state) {}
void Label::SetText(const std::string &newText) {
    text = newText;
    hb_buffer_clear_contents(buffer);
    hb_buffer_add_utf8(buffer, newText.c_str(), newText.length(), 0, -1);

    hb_shape(hbfont, buffer, NULL, 0);
    dirty = true;
}
Label::~Label() {
    hb_buffer_destroy(buffer);
    if (hbfont != nullptr) {
        hb_font_destroy(hbfont);
    }
}