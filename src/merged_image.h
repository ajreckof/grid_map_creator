#ifndef MERGED_IMAGE_H
#define MERGED_IMAGE_H

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot {

class MergedImage : public Image {
    GDCLASS(MergedImage, Image);

public:
    MergedImage();
    ~MergedImage();

    void set_images(const TypedArray<Image> images);
    void add_image(const Ref<Image> image);
    TypedArray<Image> get_images() const;

    int get_max_height() const;
    int get_max_width() const;

protected:
    static void _bind_methods();

private:
    int width;
    int height;
    TypedArray<Image> _images;
    void _update_merged_image();
};



} // namespace godot

#endif // MERGED_IMAGE_H
