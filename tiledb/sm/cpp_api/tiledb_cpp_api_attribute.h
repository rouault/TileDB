/**
 * @file   tiledb_cpp_api_attribute.h
 *
 * @author Ravi Gaddipati
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2018 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file declares the C++ API for the TileDB Attribute object.
 */

#ifndef TILEDB_CPP_API_ATTRIBUTE_H
#define TILEDB_CPP_API_ATTRIBUTE_H

#include "tiledb.h"
#include "tiledb_cpp_api_compressor.h"
#include "tiledb_cpp_api_context.h"
#include "tiledb_cpp_api_deleter.h"
#include "tiledb_cpp_api_exception.h"
#include "tiledb_cpp_api_object.h"
#include "tiledb_cpp_api_type.h"

#include <array>
#include <functional>
#include <memory>
#include <type_traits>

namespace tiledb {

/**
 * Describes an attribute of an Array cell.
 *
 * @details
 * An attribute specifies a datatype for a particular parameter in each
 * array cell. There are 3 supported attribute types:
 *
 * - Fundamental types, such as char, int, double, uint64_t, etc..
 * - Fixed sized arrays: std::array<T, N> where T is fundamental
 * - Variable length data: std::string, std::vector<T> where T is fundamental
 *
 * @code{.cpp}
 * tiledb::Context ctx;
 * auto a1 = tiledb::Attribute::create<int>(ctx, "a1");
 * auto a2 = tiledb::Attribute::create<std::string>(ctx, "a2");
 * auto a3 = tiledb::Attribute::create<std::array<float, 3>>(ctx, "a3");
 *
 * // Change compression scheme
 * a1.set_compressor({TILEDB_BLOSC, -1});
 * a1.cell_val_num(); // 1
 * a2.cell_val_num(); // Variable sized, TILEDB_VAR_NUM
 * a3.cell_val_num(); // 2
 * a3.cell_size(); // 3
 * a3.type_size(); // sizeof(float)
 *
 * tiledb::ArraySchema schema(ctx);
 * schema.add_attribute(a1);
 * @endcode
 */
class Attribute {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  Attribute(const Context& ctx, tiledb_attribute_t* attr);
  Attribute(const Attribute& attr) = default;
  Attribute(Attribute&& o) = default;
  Attribute& operator=(const Attribute&) = default;
  Attribute& operator=(Attribute&& o) = default;

  /* ********************************* */
  /*                API                */
  /* ********************************* */

  /** Returns the name of the attribute. */
  std::string name() const;

  /** Returns the attribute datatype. */
  tiledb_datatype_t type() const;

  /** Returns the size (in bytes) of one cell on this attribute. */
  uint64_t cell_size() const;

  /** Returns the size (in bytes) of the attribute type. */
  uint64_t type_size() const;

  /** Returns the number of values stored in each cell. */
  unsigned cell_val_num() const;

  /** Returns the attribute compressor. */
  Compressor compressor() const;

  /** Sets the attribute compressor. */
  Attribute& set_compressor(Compressor c);

  /** Returns the C TileDB attribute object pointer. */
  std::shared_ptr<tiledb_attribute_t> ptr() const;

  /** Auxiliary operator for getting the underlying C TileDB object. */
  operator tiledb_attribute_t*() const;

  /** Dump information about the attribute to a FILE. **/
  void dump(FILE* out = stdout) const {
    ctx_.get().handle_error(
        tiledb_attribute_dump(ctx_.get(), attr_.get(), out));
  }

  /* ********************************* */
  /*          STATIC FUNCTIONS         */
  /* ********************************* */

  /**
   * Factory function for creating a new attribute with datatype T.
   *
   * @tparam T Datatype of the attribute. Can either be fundamental,
   *         std::array<T,N>, std::string, or std::vector<T>. Trivially
   *         copyable classes, defined by std::is_trivially_copyable,
   *         are allowed. In this case the underlying attribute type is
   *         char, and the size is sizeof(T)
   * @param ctx The TileDB context.
   * @param name The attribute name.
   * @return A new Attribute object.
   */
  template <typename T>
  static Attribute create(const Context& ctx, const std::string& name) {
    using DataT = typename impl::TypeHandler<T>;
    auto a = create(ctx, name, DataT::tiledb_type);
    a.set_cell_val_num(DataT::tiledb_num);
    return a;
  }

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** The TileDB context. */
  std::reference_wrapper<const Context> ctx_;

  /** An auxiliary deleter. */
  impl::Deleter deleter_;

  /** The pointer to the C TileDB attribute object. */
  std::shared_ptr<tiledb_attribute_t> attr_;

  /* ********************************* */
  /*         PRIVATE FUNCTIONS         */
  /* ********************************* */

  /** Sets the number of attribute values per cell. */
  Attribute& set_cell_val_num(unsigned num);

  /** Creates an attribute with the input name and datatype. */
  static Attribute create(
      const Context& ctx, const std::string& name, tiledb_datatype_t type);
};

/* ********************************* */
/*               MISC                */
/* ********************************* */

/** Gets a string representation of an attribute for an output stream. */
std::ostream& operator<<(std::ostream& os, const Attribute& a);

}  // namespace tiledb

#endif  // TILEDB_CPP_API_ATTRIBUTE_H
