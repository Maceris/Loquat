// pbrt is Copyright(c) 1998-2020 Matt Pharr, Wenzel Jakob, and Greg Humphreys.
// The pbrt source code is licensed under the Apache License, Version 2.0.
// SPDX: Apache-2.0

// This file has been modified from the original, original notice is above.

#pragma once

#include <format>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "main/loquat.h"

#include "debug/logger.h"
#include "pbr/cameras.h"
#include "pbr/cpu/primitive.h"
#include "pbr/parser.h"
#include "pbr/math/transform.h"
#include "pbr/struct/parameter_dictionary.h"
#include "pbr/struct/containers.h"
#include "pbr/util/error.h"
#include "pbr/util/memory.h"
#include "pbr/util/parallel.h"
#include "pbr/util/string.h"

namespace loquat
{

class Integrator;

struct SceneEntity {
    SceneEntity() = default;
    SceneEntity(const std::string &name, ParameterDictionary parameters,
        FileLoc loc)
        : name(interned_strings.lookup(name))
        , parameters(parameters)
        , loc(loc)
    {}

    std::string to_string() const
    {
        return std::format("[ SceneEntity name: {} parameters: {} loc: {} ]",
            name, parameters, loc.to_string());
    }

    InternedString name;
    FileLoc loc;
    ParameterDictionary parameters;
    static InternCache<std::string> interned_strings;
};

struct TransformedSceneEntity : public SceneEntity
{
    TransformedSceneEntity() = default;
    TransformedSceneEntity(const std::string &name,
        ParameterDictionary parameters, FileLoc loc,
        const AnimatedTransform &render_from_object)
        : SceneEntity(name, parameters, loc)
        , render_from_object(render_from_object)
    {}

    std::string to_string() const
    {
        return std::format("[ TransformedSeneEntity name: {} parameters: {} loc: {} "
                            "render_from_object: {} ]",
                            name.to_string(), parameters.to_string(),
                            loc.to_string(), render_from_object.to_string());
    }

    AnimatedTransform render_from_object;
};

struct CameraSceneEntity : public SceneEntity
{
    CameraSceneEntity() = default;
    CameraSceneEntity(const std::string &name, ParameterDictionary parameters,
                      FileLoc loc, const CameraTransform &camera_transform,
                      const std::string &medium)
        : SceneEntity(name, parameters, loc)
        , camera_transform(camera_transform)
        , medium(medium)
    {}

    std::string to_string() const
    {
        return std::format("[ CameraSeneEntity name: {} parameters: {} loc: {} "
                            "camera_transform: {} medium: {} ]",
                            name.to_string(), parameters.to_string(), 
                            loc.to_string(), camera_transform.to_string(), 
                            medium);
    }

    CameraTransform camera_transform;
    std::string medium;
};

struct ShapeSceneEntity : public SceneEntity
{
    ShapeSceneEntity() = default;
    ShapeSceneEntity(const std::string &name, ParameterDictionary parameters, FileLoc loc,
                     const Transform *render_from_object, const Transform *object_from_render,
                     bool reverse_orientation, int material_index,
                     const std::string &material_name, int light_index,
                     const std::string &inside_medium,
                     const std::string &outside_medium)
        : SceneEntity(name, parameters, loc)
        , render_from_object(render_from_object)
        , object_from_render(object_from_render)
        , reverse_orientation(reverse_orientation)
        , material_index(material_index)
        , material_name(material_name)
        , light_index(light_index)
        , inside_medium(inside_medium)
        , outside_medium(outside_medium) {}

    std::string to_string() const
    {
        return std::format(
            "[ ShapeSeneEntity name: {} parameters: {} loc: {} "
            "render_from_object: {} object_from_render: {} reverse_orientation: {} "
            "material_index: {} material_name: {} light_index: {} "
            "inside_medium: {} outside_medium: {}]",
            name.to_string(), parameters.to_string(), loc.to_string(), 
            render_from_object ? render_from_object->to_string() : std::string("nullptr"),
            object_from_render ? object_from_render->to_string() : std::string("nullptr"),
            reverse_orientation, material_index, material_name, light_index,
            inside_medium, outside_medium);
    }

    const Transform* render_from_object = nullptr;
    const Transform* object_from_render = nullptr;
    bool reverse_orientation = false;
    int material_index;  // one of these two...  std::variant?
    std::string material_name;
    int light_index = -1;
    std::string inside_medium;
    std::string outside_medium;
};

struct AnimatedShapeSceneEntity : public TransformedSceneEntity
{
    AnimatedShapeSceneEntity() = default;
    AnimatedShapeSceneEntity(const std::string &name, ParameterDictionary parameters,
                             FileLoc loc, const AnimatedTransform &render_from_object,
                             const Transform *identity, bool reverse_orientation,
                             int material_index, const std::string &material_name,
                             int light_index, const std::string &inside_medium,
                             const std::string &outside_medium)
        : TransformedSceneEntity(name, parameters, loc, render_from_object)
        , identity(identity)
        , reverse_orientation(reverse_orientation)
        , material_index(material_index)
        , material_name(material_name)
        , light_index(light_index)
        , inside_medium(inside_medium)
        , outside_medium(outside_medium) {}

    std::string to_string() const
    {
        return std::format(
            "[ AnimatedShapeSeneEntity name: {} parameters: {} loc: {} "
            "render_from_object: {} reverse_orientation: {} material_index: {} "
            "material_name: {} inside_medium: {} outside_medium: {}]",
            name.to_string(), parameters.to_string(), loc.to_string(), 
            render_from_object.to_string(), reverse_orientation, 
            material_index, material_name, inside_medium, outside_medium);
    }

    const Transform *identity = nullptr;
    bool reverse_orientation = false;
    int material_index;  // one of these two...  std::variant?
    std::string material_name;
    int light_index = -1;
    std::string inside_medium;
    std::string outside_medium;
};

struct InstanceDefinitionSceneEntity
{
    InstanceDefinitionSceneEntity() = default;
    InstanceDefinitionSceneEntity(const std::string &name, FileLoc loc)
        : name(SceneEntity::interned_strings.lookup(name))
        , loc(loc)
    {}

    std::string to_string() const {
        return std::format("[ InstanceDefinitionSceneEntity name: {} loc: {} "
                            " shapes: {} animated_shapes: {} ]",
                            name.to_string(), loc.to_string(), 
                            shapes, animated_shapes);
    }

    InternedString name;
    FileLoc loc;
    std::vector<ShapeSceneEntity> shapes;
    std::vector<AnimatedShapeSceneEntity> animated_shapes;
};

using MediumSceneEntity = TransformedSceneEntity;
using TextureSceneEntity = TransformedSceneEntity;

struct LightSceneEntity : public TransformedSceneEntity
{
    LightSceneEntity() = default;
    LightSceneEntity(const std::string &name, ParameterDictionary parameters,
        FileLoc loc, const AnimatedTransform &render_from_light,
        const std::string &medium)
        : TransformedSceneEntity(name, parameters, loc, render_from_light)
        , medium(medium)
    {}

    std::string to_string() const
    {
        return std::format("[ LightSeneEntity name: {} parameters: {} loc: {} "
                            "render_from_object: {} medium: {} ]",
                            name.to_string(), parameters.to_string(), 
                            loc.to_string(), render_from_object.to_string(), 
                            medium);
    }

    std::string medium;
};

struct InstanceSceneEntity
{
    InstanceSceneEntity() = default;
    InstanceSceneEntity(const std::string &n, FileLoc loc,
                        const AnimatedTransform &render_from_instance_anim)
        : name(SceneEntity::interned_strings.lookup(n))
        , loc(loc)
        , render_from_instance_anim(new AnimatedTransform(render_from_instance_anim))
    {
        LOG_ASSERT(this->render_from_instance_anim->is_animated());
    }
    InstanceSceneEntity(const std::string &n, FileLoc loc,
                        const Transform *render_from_instance)
        : name(SceneEntity::interned_strings.lookup(n))
        , loc(loc)
        , render_from_instance(render_from_instance)
    {}

    std::string to_string() const
    {
        return std::format(
            "[ InstanceSeneEntity name: {} loc: {} "
            "render_from_instance_anim: {} render_from_instance: {} ]",
            name.to_string(), loc.to_string(),
            render_from_instance_anim ? render_from_instance_anim->to_string()
                                      : std::string("nullptr"),
            render_from_instance ? render_from_instance->to_string() 
                                 : std::string("nullptr"));
    }

    InternedString name;
    FileLoc loc;
    AnimatedTransform *render_from_instance_anim = nullptr;
    const Transform *render_from_instance = nullptr;
};

constexpr int MAX_TRANSFORMS = 2;

struct TransformSet {
    Transform &operator[](int i)
    {
        LOG_ASSERT(i >= 0);
        LOG_ASSERT(i < MAX_TRANSFORMS);
        return t[i];
    }

    const Transform &operator[](int i) const
    {
        LOG_ASSERT(i >= 0);
        LOG_ASSERT(i < MAX_TRANSFORMS);
        return t[i];
    }

    friend TransformSet inverse(const TransformSet &ts)
    {
        TransformSet tInv;
        for (int i = 0; i < MAX_TRANSFORMS; ++i)
        {
            tInv.t[i] = inverse(ts.t[i]);
        }
        return tInv;
    }

    bool is_animated() const {
        for (int i = 0; i < MAX_TRANSFORMS - 1; ++i)
        {
            if (t[i] != t[i + 1])
            {
                return true;
            }
        }
        return false;
    }

  private:
    Transform t[MAX_TRANSFORMS];
};

class BasicScene {
  public:
    BasicScene();

    void set_options(SceneEntity filter, SceneEntity film, CameraSceneEntity camera,
                    SceneEntity sampler, SceneEntity integrator, SceneEntity accelerator);

    void add_named_material(std::string name, SceneEntity material);
    int add_material(SceneEntity material);
    void add_medium(MediumSceneEntity medium);
    void add_float_texture(std::string name, TextureSceneEntity texture);
    void add__spectrum_texture(std::string name, TextureSceneEntity texture);
    void add_light(LightSceneEntity light);
    int add_area_light(SceneEntity light);
    void add_shapes(pstd::span<ShapeSceneEntity> shape);
    void add_animated_shape(AnimatedShapeSceneEntity shape);
    void add_instance_definition(InstanceDefinitionSceneEntity instance);
    void add_instance_uses(pstd::span<InstanceSceneEntity> in);

    void done();

    Camera get_camera()
    {
        camera_job_mutex.lock();
        while (!camera)
        {
            pstd::optional<Camera> c = camera_job->try_get_result(&camera_job_mutex);
            if (c)
            {
                camera = *c;
            }
        }
        camera_job_mutex.unlock();
        LOG_INFO("Retrieved Camera from future");
        return camera;
    }

    Sampler get_sampler()
    {
        sampler_job_mutex.lock();
        while (!sampler) {
            pstd::optional<Sampler> s = sampler_job->try_get_result(&sampler_job_mutex);
            if (s)
                sampler = *s;
        }
        sampler_job_mutex.unlock();
        LOG_INFO("Retrieved Sampler from future");
        return sampler;
    }

    void create_materials(const NamedTextures &scene_textures,
                         std::map<std::string, Material> *named_materials,
                         std::vector<Material> *materials);

    std::vector<Light> create_lights(
        const NamedTextures &textures,
        std::map<int, pstd::vector<Light> *> *shape_index_to_area_lights);

    std::map<std::string, Medium> create_media();

    Primitive create_aggregate(
        const NamedTextures &textures,
        const std::map<int, pstd::vector<Light> *> &shape_index_to_area_lights,
        const std::map<std::string, Medium> &media,
        const std::map<std::string, Material> &named_materials,
        const std::vector<Material> &materials);

    std::unique_ptr<Integrator> create_integrator(Camera camera, Sampler sampler,
                                                 Primitive accel,
                                                 std::vector<Light> lights) const;

    NamedTextures create_textures();

    SceneEntity integrator;
    SceneEntity accelerator;
    const RGBColorSpace *film_color_space;
    std::vector<ShapeSceneEntity> shapes;
    std::vector<AnimatedShapeSceneEntity> animated_shapes;
    std::vector<InstanceSceneEntity> instances;
    std::map<InternedString, InstanceDefinitionSceneEntity *> instance_definitions;

  private:
    Medium get_medium(const std::string &name, const FileLoc *loc);

    void start_loading_normal_maps(const ParameterDictionary &parameters);

    AsyncJob<Sampler> *sampler_job = nullptr;
    mutable ThreadLocal<Allocator> thread_allocators;
    Camera camera;
    Film film;
    std::mutex camera_job_mutex;
    AsyncJob<Camera> *camera_job = nullptr;
    std::mutex sampler_job_mutex;
    Sampler sampler;
    std::mutex medium_mutex;
    std::map<std::string, AsyncJob<Medium> *> medium_jobs;
    std::map<std::string, Medium> media_map;
    std::mutex material_mutex;
    std::map<std::string, AsyncJob<Image *> *> normal_map_jobs;
    std::map<std::string, Image *> normal_maps;

    std::vector<std::pair<std::string, SceneEntity>> named_materials;
    std::vector<SceneEntity> materials;

    std::mutex light_mutex;
    std::vector<AsyncJob<Light> *> light_jobs;

    std::mutex area_light_mutex;
    std::vector<SceneEntity> area_lights;

    std::mutex texture_mutex;
    std::vector<std::pair<std::string, TextureSceneEntity>> serial_float_textures;
    std::vector<std::pair<std::string, TextureSceneEntity>> serial_spectrum_textures;
    std::vector<std::pair<std::string, TextureSceneEntity>> async_spectrum_textures;
    std::set<std::string> loading_texture_filenames;
    std::map<std::string, AsyncJob<FloatTexture> *> float_texture_jobs;
    std::map<std::string, AsyncJob<SpectrumTexture> *> spectrum_texture_jobs;
    int missing_texture_count = 0;

    std::mutex shape_mutex;
    std::mutex animated_shape_mutex;
    std::mutex instance_definition_mutex;
    std::mutex instance_use_mutex;
};

class BasicSceneBuilder : public ParserTarget
{
  public:
    BasicSceneBuilder(BasicScene *scene);
    void option(const std::string &name, const std::string &value, FileLoc loc);
    void identity(FileLoc loc);
    void translate(Float dx, Float dy, Float dz, FileLoc loc);
    void rotate(Float angle, Float ax, Float ay, Float az, FileLoc loc);
    void scale(Float sx, Float sy, Float sz, FileLoc loc);
    void look_at(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz, Float ux,
                Float uy, Float uz, FileLoc loc);
    void concat_transform(Float transform[16], FileLoc loc);
    void transform(Float transform[16], FileLoc loc);
    void coordinate_system(const std::string &, FileLoc loc);
    void coord_sys_transform(const std::string &, FileLoc loc);
    void active_transform_all(FileLoc loc);
    void active_transform_end_time(FileLoc loc);
    void active_transform_start_time(FileLoc loc);
    void transform_times(Float start, Float end, FileLoc loc);
    void color_space(const std::string &n, FileLoc loc);
    void pixel_filter(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void film(const std::string &type, ParsedParameterVector params, FileLoc loc);
    void sampler(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void accelerator(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void integrator(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void camera(const std::string &, ParsedParameterVector params, FileLoc loc);
    void make_named_medium(const std::string &name, ParsedParameterVector params,
                         FileLoc loc);
    void medium_interface(const std::string &insideName, const std::string &outsideName,
                         FileLoc loc);
    void world_begin(FileLoc loc);
    void attribute_begin(FileLoc loc);
    void attributeEnd(FileLoc loc);
    void attribute(const std::string &target, ParsedParameterVector params, FileLoc loc);
    void texture(const std::string &name, const std::string &type,
                 const std::string &texname, ParsedParameterVector params, FileLoc loc);
    void material(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void make_named_material(const std::string &name, ParsedParameterVector params,
                           FileLoc loc);
    void named_material(const std::string &name, FileLoc loc);
    void light_source(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void area_light_source(const std::string &name, ParsedParameterVector params,
                         FileLoc loc);
    void shape(const std::string &name, ParsedParameterVector params, FileLoc loc);
    void reverse_orientation(FileLoc loc);
    void object_begin(const std::string &name, FileLoc loc);
    void object_end(FileLoc loc);
    void object_instance(const std::string &name, FileLoc loc);

    void end_of_files();

    BasicSceneBuilder *copy_for_import();
    void merge_imported(BasicSceneBuilder *);

    std::string to_string() const;

  private:
    struct GraphicsState
    {
        GraphicsState();

        template <typename F>
        void for_active_transforms(F func)
        {
            for (int i = 0; i < MAX_TRANSFORMS; ++i)
            {
                if (active_transform_bits & (1 << i))
                {
                    ctm[i] = func(ctm[i]);
                }
            }
        }

        std::string current_inside_medium;
        std::string current_outside_medium;

        int current_material_index = 0;
        std::string current_material_name;

        std::string area_light_name;
        ParameterDictionary area_light_params;
        FileLoc area_light_loc;

        ParsedParameterVector shape_attributes;
        ParsedParameterVector light_attributes;
        ParsedParameterVector material_attributes;
        ParsedParameterVector medium_attributes;
        ParsedParameterVector texture_attributes;
        bool reverse_orientation = false;
        const RGBColorSpace *color_space = RGBColorSpace::sRGB;
        TransformSet ctm;
        uint32_t active_transform_bits = ALL_TRANSFORM_BITS;
        Float transform_start_time = 0;
        Float transform_end_time = 1;
    };

    friend void parse(ParserTarget *scene, std::unique_ptr<Tokenizer> t);

    class Transform render_from_object(int index) const
    {
        return loquat::Transform((render_from_world * graphics_state.ctm[index]).get_matrix());
    }

    AnimatedTransform render_from_object() const
    {
        return {render_from_object(0), graphics_state.transform_start_time,
                render_from_object(1), graphics_state.transform_end_time};
    }

    bool CTM_is_animated() const { return graphics_state.ctm.is_animated(); }

    BasicScene *scene;
    enum class BlockState { OptionsBlock, WorldBlock };
    BlockState current_block = BlockState::OptionsBlock;
    GraphicsState graphics_state;
    static constexpr int START_TRANSFORM_BITS = 1 << 0;
    static constexpr int END_TRANSFORM_BITS = 1 << 1;
    static constexpr int ALL_TRANSFORM_BITS = (1 << MAX_TRANSFORMS) - 1;
    std::map<std::string, TransformSet> named_coordinate_systems;
    class Transform render_from_world;
    InternCache<class Transform> transform_cache;
    std::vector<GraphicsState> pushed_graphics_states;
    std::vector<std::pair<char, FileLoc>> push_stack;  // 'a': attribute, 'o': object
    struct ActiveInstanceDefinition
    {
        ActiveInstanceDefinition(std::string name, FileLoc loc)
            : entity(name, loc)
        {}

        std::mutex mutex;
        std::atomic<int> active_imports{1};
        InstanceDefinitionSceneEntity entity;
        ActiveInstanceDefinition *parent = nullptr;
    };
    ActiveInstanceDefinition *active_instance_definition = nullptr;

    // Buffer these both to avoid mutex contention and so that they are
    // consistently ordered across runs.
    std::vector<ShapeSceneEntity> shapes;
    std::vector<InstanceSceneEntity> instance_uses;

    std::set<std::string> namedMaterialNames;
    std::set<std::string> medium_names;
    std::set<std::string> float_texture_names;
    std::set<std::string> spectrum_texture_names;
    std::set<std::string> instance_names;
    int current_material_index = 0;
    int current_light_index = -1;
    SceneEntity sampler;
    SceneEntity film;
    SceneEntity integrator;
    SceneEntity filter;
    SceneEntity accelerator;
    CameraSceneEntity camera;
};
}