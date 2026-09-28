#ifndef ENTITYATTRIBUTES_H
#define ENTITYATTRIBUTES_H

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.

#include <meta>
#include <string_view>
#include <type_traits>

#include <YGP/Attribute.h>
#include <YGP/Entity.h>

/**Annotation marking a data member of a YGP::Entity as named attribute.
 *
 * Replaces the former "// %attrib%" comments (and the mgeni.pl generated
 * .meta files): Annotate a member with [[=Attrib{"Name"}]] and call
 * registerAttributes(*this) in the constructor.
 */
struct Attrib {
    const char* name;

    consteval Attrib(std::string_view attrName) : name(std::define_static_string(attrName)) {}
};

//-----------------------------------------------------------------------------
/// Registers every data member of the passed entity annotated with Attrib
/// as YGP::Attribute (in declaration order)
/// \param obj Entity to register the attributes for
//-----------------------------------------------------------------------------
template <typename T>
    requires std::is_base_of_v<YGP::Entity, T>
void registerAttributes(T& obj) {
    constexpr auto ctx = std::meta::access_context::unchecked();
    template for (constexpr auto member : std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx))) {
        constexpr auto annotations = std::define_static_array(std::meta::annotations_of_with_type(member, ^^Attrib));
        if constexpr (!annotations.empty()) {
            constexpr Attrib attrib = std::meta::extract<Attrib>(annotations[0]);
            using Type = typename[:std::meta::type_of(member):];
            obj.addAttribute(*new YGP::Attribute<Type>(attrib.name, obj.[:member:]));
        }
    }
}

#endif
