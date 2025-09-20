#include "object_utility.h"

#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {


bool object_has_property(const Object *obj, const String &property_name) {
    if (!obj) {
        return false;
    }
    for (const Dictionary &property : obj->get_property_list()) {
        if (property["name"] == property_name) {
            return true;
        }
    }
    return false;
}

}