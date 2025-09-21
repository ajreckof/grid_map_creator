#include "object_utility.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {


bool object_has_property(const Object *obj, const String &property_name) {
    if (!obj) {
        return false;
    }
    TypedArray<Dictionary> property_list = obj->get_property_list();
    for (int i = 0; i < property_list.size(); ++i) {
        Dictionary property = property_list[i];
        if (property["name"] == property_name) {
            return true;
        }
    }
    return false;
}

}