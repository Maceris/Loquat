// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

#include "main/loquat.h"

#include "pbr/struct/containers.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/util/error.h"
#include "pbr/util/pstd.h"

namespace loquat
{
    class ParserTarget
    {
    public:
        virtual void scale(Float sx, Float sy, Float sz, FileLoc loc) = 0;

        virtual void shape(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;

        virtual ~ParserTarget();

        virtual void option(const std::string& name, const std::string& value,
            FileLoc loc) = 0;

        virtual void identity(FileLoc loc) = 0;
        virtual void translate(Float dx, Float dy, Float dz, FileLoc loc) = 0;
        virtual void rotate(Float angle, Float ax, Float ay, Float az,
            FileLoc loc) = 0;
        virtual void look_at(Float ex, Float ey, Float ez, Float lx, Float ly,
            Float lz, Float ux, Float uy, Float uz, FileLoc loc) = 0;
        virtual void concat_transform(Float transform[16], FileLoc loc) = 0;
        virtual void transform(Float transform[16], FileLoc loc) = 0;
        virtual void coordinate_system(const std::string&, FileLoc loc) = 0;
        virtual void coord_sys_transform(const std::string&, FileLoc loc) = 0;
        virtual void active_transform_all(FileLoc loc) = 0;
        virtual void active_transform_end_time(FileLoc loc) = 0;
        virtual void active_transform_start_time(FileLoc loc) = 0;
        virtual void transform_times(Float start, Float end, FileLoc loc) = 0;

        virtual void color_space(const std::string& n, FileLoc loc) = 0;
        virtual void pixel_filter(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void flim(const std::string& type,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void accelerator(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void integrator(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void camera(const std::string&, ParsedParameterVector params,
            FileLoc loc) = 0;
        virtual void make_named_medium(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void medium_interface(const std::string& insideName,
            const std::string& outsideName, FileLoc loc) = 0;
        virtual void sampler(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;

        virtual void world_begin(FileLoc loc) = 0;
        virtual void attribute_begin(FileLoc loc) = 0;
        virtual void attribute_end(FileLoc loc) = 0;
        virtual void attribute(const std::string& target,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void texture(const std::string& name, const std::string& type,
            const std::string& texname, ParsedParameterVector params,
            FileLoc loc) = 0;
        virtual void material(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void make_named_material(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void named_material(const std::string& name, FileLoc loc) = 0;
        virtual void light_source(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void area_light_source(const std::string& name,
            ParsedParameterVector params, FileLoc loc) = 0;
        virtual void reverse_orientation(FileLoc loc) = 0;
        virtual void object_begin(const std::string& name, FileLoc loc) = 0;
        virtual void object_end(FileLoc loc) = 0;
        virtual void object_instance(const std::string& name, FileLoc loc) = 0;

        virtual void end_of_files() = 0;

    protected:
        template <typename... Args>
        void error_exit_deferred(const char* fmt, Args &&...args) const
        {
            error_exit = true;
            Error(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void error_exit_deferred(const FileLoc* loc, const char* fmt,
            Args &&...args) const
        {
            error_exit = true;
            Error(loc, fmt, std::forward<Args>(args)...);
        }

        mutable bool error_exit = false;
    };

    void parse_files(ParserTarget* target, pstd::span<const std::string> filenames);
    void parse_string(ParserTarget* target, std::string str);

    struct Token
    {
        Token() = default;
        Token(std::string_view token, FileLoc loc)
            : token(token)
            , loc(loc)
        {}
        std::string to_string() const;
        std::string_view token;
        FileLoc loc;
    };

    class Tokenizer {
    public:
        Tokenizer(std::string str, std::string filename,
            std::function<void(const char*, const FileLoc*)> error_callback);
#if defined(LOQUAT_HAVE_MMAP) || defined(LOQUAT_IS_WINDOWS)
        Tokenizer(void* ptr, size_t len, std::string filename,
            std::function<void(const char*, const FileLoc*)> error_callback);
#endif
        ~Tokenizer();

        static std::unique_ptr<Tokenizer> create_from_file(
            const std::string& filename,
            std::function<void(const char*, const FileLoc*)> error_callback);
        static std::unique_ptr<Tokenizer> create_from_string(
            std::string str,
            std::function<void(const char*, const FileLoc*)> error_callback);

        pstd::optional<Token> next();

        // Just for parse().
        // TODO(ches)? Have a method to set this?
        FileLoc loc;

    private:
        void check_UTF(const void* ptr, int len) const;

        int get_char()
        {
            if (pos == end)
            {
                return EOF;
            }
            int ch = *pos++;
            if (ch == '\n')
            {
                ++loc.line;
                loc.column = 0;
            }
            else
            {
                ++loc.column;
            }
            return ch;
        }

        void unget_char()
        {
            --pos;
            if (*pos == '\n')
            {
                // Don't worry about the column; we'll be going to the start of
                // the next line again shortly...
                --loc.line;
            }
        }

        // This function is called if there is an error during lexing.
        std::function<void(const char*, const FileLoc*)> error_callback;

#if defined(LOQUAT_HAVE_MMAP) || defined(LOQUAT_IS_WINDOWS)
        // Scene files on disk are mapped into memory for lexing.  We need to
        // hold on to the starting pointer and total length so they can be
        // unmapped in the destructor.
        void* unmap_ptr = nullptr;
        size_t unmap_length = 0;
#endif

        // If the input is stdin, then we copy everything until EOF into this
        // string and then start lexing.  This is a little wasteful (versus
        // tokenizing directly from stdin), but makes the implementation
        // simpler.
        std::string contents;

        // Pointers to the current position in the file and one past the end of
        // the file.
        const char* pos, * end;

        // If there are escaped characters in the string, we can't just return
        // a std::string_view into the mapped file. In that case, we handle the
        // escaped characters and return a std::string_view to s_escaped.  (And
        // thence, std::string_views from previous calls to next() must be
        // invalid after a subsequent call, since we may reuse s_escaped.)
        std::string s_escaped;
    };

    // FormattingParserTarget Definition
    class FormattingParserTarget : public ParserTarget
    {
    public:
        FormattingParserTarget(bool toPly, bool upgrade)
            : toPly(toPly)
            , upgrade(upgrade)
        {}
        ~FormattingParserTarget();

        void option(const std::string& name, const std::string& value,
            FileLoc loc);
        void identity(FileLoc loc);
        void translate(Float dx, Float dy, Float dz, FileLoc loc);
        void rotate(Float angle, Float ax, Float ay, Float az, FileLoc loc);
        void scale(Float sx, Float sy, Float sz, FileLoc loc);
        void look_at(Float ex, Float ey, Float ez, Float lx, Float ly,
            Float lz, Float ux, Float uy, Float uz, FileLoc loc);
        void concat_transform(Float transform[16], FileLoc loc);
        void transform(Float transform[16], FileLoc loc);
        void coordinate_system(const std::string&, FileLoc loc);
        void coord_sys_transform(const std::string&, FileLoc loc);
        void active_transform_all(FileLoc loc);
        void active_transform_end_time(FileLoc loc);
        void active_transform_start_time(FileLoc loc);
        void transform_times(Float start, Float end, FileLoc loc);
        void transform_begin(FileLoc loc);
        void transform_end(FileLoc loc);
        void color_space(const std::string& n, FileLoc loc);
        void pixel_filter(const std::string& name,
            ParsedParameterVector params, FileLoc loc);
        void flim(const std::string& type, ParsedParameterVector params,
            FileLoc loc);
        void sampler(const std::string& name, ParsedParameterVector params,
            FileLoc loc);
        void accelerator(const std::string& name, ParsedParameterVector params,
            FileLoc loc);
        void integrator(const std::string& name, ParsedParameterVector params,
            FileLoc loc);
        void camera(const std::string&, ParsedParameterVector params,
            FileLoc loc);
        void make_named_medium(const std::string& name,
            ParsedParameterVector params, FileLoc loc);
        void medium_interface(const std::string& insideName,
            const std::string& outsideName, FileLoc loc);
        void world_begin(FileLoc loc);
        void attribute_begin(FileLoc loc);
        void attribute_end(FileLoc loc);
        void attribute(const std::string& target, ParsedParameterVector params,
            FileLoc loc);
        void texture(const std::string& name, const std::string& type,
            const std::string& texname, ParsedParameterVector params,
            FileLoc loc);
        void material(const std::string& name, ParsedParameterVector params,
            FileLoc loc);
        void make_named_material(const std::string& name,
            ParsedParameterVector params, FileLoc loc);
        void named_material(const std::string& name, FileLoc loc);
        void light_source(const std::string& name,
            ParsedParameterVector params, FileLoc loc);
        void area_light_source(const std::string& name,
            ParsedParameterVector params, FileLoc loc);
        void shape(const std::string& name, ParsedParameterVector params,
            FileLoc loc);
        void reverse_orientation(FileLoc loc);
        void object_begin(const std::string& name, FileLoc loc);
        void object_end(FileLoc loc);
        void object_instance(const std::string& name, FileLoc loc);

        void end_of_files();

        std::string indent(int extra = 0) const
        {
            return std::string(cat_indent_count + 4 * extra, ' ');
        }

    private:
        std::string upgrade_material_index(const std::string& name,
            ParameterDictionary* dict, FileLoc loc) const;
        std::string upgrade_material(std::string* name,
            ParameterDictionary* dict, FileLoc loc) const;

        int cat_indent_count = 0;
        bool toPly;
        bool upgrade;
        std::map<std::string, std::string> defined_textures;
        std::map<std::string, std::string> defined_named_materials;
        std::map<std::string, ParameterDictionary> named_material_definitions;
        std::map<std::string, std::string> defined_object_instances;
    };
}