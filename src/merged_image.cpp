#include "merged_image.h"

namespace godot {

MergedImage::MergedImage() {
    set_data(1, 1, false, Image::FORMAT_RGBA8, PackedByteArray({0,0,0,0}));
}
MergedImage::~MergedImage() {}

void MergedImage::set_images(const TypedArray<Image> images) {
    _images = images;
    _update_merged_image();
}

TypedArray<Image> MergedImage::get_images() const {
    return _images;
}

void MergedImage::_update_merged_image() {
    if (_images.is_empty()) {
        this->resize(1, 1);
        this->fill(Color(0,0,0,0));
        return;
    }
    width = 0;
    height = 0;
    for (int i = 0; i < _images.size(); ++i) {
        Ref<Image> img = _images[i];
        if (img.is_valid()) {
            width = MAX(width, img->get_width());
            height = MAX(height, img->get_height());
        }
    }
    if (width == 0 || height == 0) {
        this->resize(1, 1);
        this->fill(Color(0,0,0,0));
        return;
    }
    int total_height = height * _images.size();
    this->resize(width, total_height);
    this->fill(Color(0,0,0,0));
    for (int i = 0; i < _images.size(); ++i) {
        Ref<Image> img = _images[i];
        if (img.is_valid()) {
            this->blit_rect(img, Rect2i(0,0,img->get_width(),img->get_height()), Vector2i(0, i * height));
        }
    }
}

int MergedImage::get_max_height() const {
    return height;
}

int MergedImage::get_max_width() const {
    return width;
}

void MergedImage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_images", "images"), &MergedImage::set_images);
    ClassDB::bind_method(D_METHOD("get_images"), &MergedImage::get_images);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "images", PROPERTY_HINT_RESOURCE_TYPE, "Image", PROPERTY_USAGE_DEFAULT, "Image"), "set_images", "get_images");
    
    ClassDB::bind_method(D_METHOD("get_max_height"), &MergedImage::get_max_height);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_height"), "", "get_max_height");

    ClassDB::bind_method(D_METHOD("get_max_width"), &MergedImage::get_max_width);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_width"), "", "get_max_width");
}

} // namespace godot
