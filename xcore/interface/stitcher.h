/*
 * stitcher.h - stitcher interface
 *
 *  Copyright (c) 2017 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Wind Yuan <feng.yuan@intel.com>
 * Author: Yinhang Liu <yinhangx.liu@intel.com>
 */

#ifndef XCAM_INTERFACE_STITCHER_H
#define XCAM_INTERFACE_STITCHER_H

#include <xcam_std.h>
#include <interface/data_types.h>
#include <vector>
#include <video_buffer.h>

#define XCAM_STITCH_FISHEYE_MAX_NUM    6
#define XCAM_STITCH_MAX_CAMERAS XCAM_STITCH_FISHEYE_MAX_NUM
#define XCAM_STITCH_MIN_SEAM_WIDTH 56

#define INVALID_INDEX (uint32_t)(-1)

namespace XCam {

enum StitchResMode {
    StitchRes1080P2Cams,
    StitchRes1080P4Cams,
    StitchRes4K2Cams,
    StitchRes8K3Cams,
    StitchRes8K6Cams
};

enum StitchScopicMode {
    ScopicMono,
    ScopicStereoLeft,
    ScopicStereoRight
};

struct StitchInfo {
    uint32_t merge_width[XCAM_STITCH_FISHEYE_MAX_NUM];

    ImageCropInfo crop[XCAM_STITCH_FISHEYE_MAX_NUM];
    FisheyeInfo fisheye_info[XCAM_STITCH_FISHEYE_MAX_NUM];

    StitchInfo () {
        xcam_mem_clear (merge_width);
    }
};

struct ImageMergeInfo {
    Rect left;
    Rect right;
};

class Stitcher;

struct CalibrationInfo {
    ExtrinsicParameter extrinsic;
    IntrinsicParameter intrinsic;
};

struct CameraInfo {
    CalibrationInfo   calibration;
    float             round_angle_start;
    float             angle_range;;
};

class VKDevice;

class Stitcher
{
public:
    struct RoundViewSlice {
        float          hori_angle_start;
        float          hori_angle_range;
        uint32_t       width;
        uint32_t       height;

        RoundViewSlice ()
            : hori_angle_start (0.0f), hori_angle_range (0.0f)
            , width (0), height (0)
        {}
    };

    struct CenterMark {
        uint32_t slice_center_x;
        uint32_t out_center_x;
        CenterMark ()
            : slice_center_x (0)
            , out_center_x (0)
        {}
    };

    struct ScaleFactor {
        float left_scale;
        float right_scale;

        ScaleFactor ()
            : left_scale (1.0f)
            , right_scale (1.0f)
        {}
    };

    struct ImageOverlapInfo {
        Rect left;
        Rect right;
        Rect out_area;
    };

    struct CopyArea {
        uint32_t in_idx;
        Rect     in_area;
        Rect     out_area;

        CopyArea ()
            : in_idx (INVALID_INDEX)
        {}
    };
    typedef std::vector<CopyArea>  CopyAreaArray;

public:
    explicit Stitcher (uint32_t align_x, uint32_t align_y = 1);
    virtual ~Stitcher ();
    static SmartPtr<Stitcher> create_ocl_stitcher ();
    static SmartPtr<Stitcher> create_soft_stitcher ();
    static SmartPtr<Stitcher> create_gl_stitcher ();
    static SmartPtr<Stitcher> create_vk_stitcher (const SmartPtr<VKDevice> dev);

    bool set_bowl_config (const BowlDataConfig &config);
    const BowlDataConfig &get_bowl_config () {
        return _bowl_config;
    }
    bool set_camera_num (uint32_t num);
    uint32_t get_camera_num () const {
        return _camera_num;
    }
    bool set_camera_info (uint32_t index, const CameraInfo &info);
    bool get_camera_info (uint32_t index, CameraInfo &info) const;

    bool set_crop_info (uint32_t index, const ImageCropInfo &info);
    bool get_crop_info (uint32_t index, ImageCropInfo &info) const;
    bool is_crop_info_set () const {
        return _is_crop_set;
    }
    //bool set_overlap_info (uint32_t index, const ImageOverlapInfo &info);
    bool is_overlap_info_set () const {
        return _is_overlap_set;
    }

    //bool set_stitch_info (const StitchInfo &stitch_info);
    void set_output_size (uint32_t width, uint32_t height) {
        _output_width = width; //XCAM_ALIGN_UP (width, XCAM_BLENDER_ALIGNED_WIDTH);
        _output_height = height;
    }
    void get_output_size (uint32_t &width, uint32_t &height) const {
        width = _output_width;
        height = _output_height;
    }

    void set_res_mode (StitchResMode mode) {
        _res_mode = mode;
    }
    StitchResMode get_res_mode () {
        return _res_mode;
    }

    void set_dewarp_mode (FisheyeDewarpMode mode) {
        _dewarp_mode = mode;
    }
    FisheyeDewarpMode get_dewarp_mode () {
        return _dewarp_mode;
    }

    void set_scopic_mode (StitchScopicMode mode) {
        _scopic_mode = mode;
    }
    StitchScopicMode get_scopic_mode () {
        return _scopic_mode;
    }

    void set_scale_mode (GeoMapScaleMode scale_mode) {
        _scale_mode = scale_mode;
    }
    GeoMapScaleMode get_scale_mode () {
        return _scale_mode;
    }

    void set_fm_mode (FeatureMatchMode fm_mode) {
        _fm_mode = fm_mode;
    }
    FeatureMatchMode get_fm_mode () {
        return _fm_mode;
    }

    void set_fm_status (FeatureMatchStatus fm_status) {
        _fm_status = fm_status;
    }
    FeatureMatchStatus get_fm_status () {
        return _fm_status;
    }

    void set_fm_frames (uint32_t fm_frames) {
        _fm_frames = fm_frames;
    }
    uint32_t get_fm_frames () {
        return _fm_frames;
    }

    void set_fm_frame_count (uint32_t frame_count) {
        _fm_frame_count = frame_count;
    }
    uint32_t get_fm_frame_count () {
        return _fm_frame_count;
    }

    void set_blend_pyr_levels (uint32_t pyr_levels) {
        _blend_pyr_levels = pyr_levels;
    }
    uint32_t get_blend_pyr_levels () {
        return _blend_pyr_levels;
    }

    bool set_viewpoints_range (const float *range);
    bool set_instrinsic_names (const char *instr_names[]);
    bool set_exstrinsic_names (const char *exstr_names[]);

    virtual XCamReturn stitch_buffers (const VideoBufferList &in_bufs, SmartPtr<VideoBuffer> &out_buf) = 0;

protected:
    XCamReturn init_camera_info ();
    XCamReturn estimate_round_slices ();
    virtual XCamReturn estimate_coarse_crops ();
    XCamReturn mark_centers ();
    XCamReturn estimate_overlap ();
    XCamReturn update_copy_areas ();

    const CenterMark &get_center (uint32_t idx) const {
        return _center_marks[idx];
    }
    const RoundViewSlice &get_round_view_slice (uint32_t idx) const {
        return _round_view_slices[idx];
    }
    const ImageOverlapInfo &get_overlap (uint32_t idx) const {
        return _overlap_info[idx];
    }
    const ImageCropInfo &get_crop (uint32_t idx) const {
        return _crop_info[idx];
    }
    const CopyAreaArray &get_copy_area () const {
        return _copy_areas;
    }

private:
    XCAM_DEAD_COPY (Stitcher);

private:
    uint32_t                    _alignment_x, _alignment_y;
    uint32_t                    _output_width, _output_height;
    float                       _out_start_angle;
    uint32_t                    _camera_num;
    CameraInfo                  _camera_info[XCAM_STITCH_MAX_CAMERAS];
    char                       *_instr_names[XCAM_STITCH_MAX_CAMERAS];
    char                       *_exstr_names[XCAM_STITCH_MAX_CAMERAS];

    RoundViewSlice              _round_view_slices[XCAM_STITCH_MAX_CAMERAS];
    bool                        _is_round_view_set;

    float                       _viewpoints_range[XCAM_STITCH_MAX_CAMERAS];

    ImageOverlapInfo            _overlap_info[XCAM_STITCH_MAX_CAMERAS];
    BowlDataConfig              _bowl_config;
    bool                        _is_overlap_set;

    ImageCropInfo               _crop_info[XCAM_STITCH_MAX_CAMERAS];
    bool                        _is_crop_set;

    //auto calculation
    CenterMark                  _center_marks[XCAM_STITCH_MAX_CAMERAS];
    bool                        _is_center_marked;
    CopyAreaArray               _copy_areas;

    StitchResMode               _res_mode;
    FisheyeDewarpMode           _dewarp_mode;

    StitchScopicMode            _scopic_mode;

    GeoMapScaleMode             _scale_mode;
    //update after each feature match
    ScaleFactor                 _scale_factors[XCAM_STITCH_MAX_CAMERAS];

    FeatureMatchMode            _fm_mode;
    FeatureMatchStatus          _fm_status;
    uint32_t                    _fm_frames;
    uint32_t                    _fm_frame_count;

    uint32_t                    _blend_pyr_levels;
};

class BowlModel {
public:
    typedef std::vector<PointFloat3> VertexMap;
    typedef std::vector<PointFloat2> PointMap;
    typedef std::vector<int32_t> IndexVector;

public:
    BowlModel (const BowlDataConfig &config, const uint32_t image_width, const uint32_t image_height);
    bool get_max_topview_area_mm (float &length_mm, float &width_mm);
    bool get_topview_rect_map (
        PointMap &texture_points,
        uint32_t res_width, uint32_t res_height,
        float length_mm = 0.0f, float width_mm = 0.0f);

    bool get_stitch_image_vertex_model (
        VertexMap &vertices, PointMap &texture_points, IndexVector &indeices,
        uint32_t res_width, uint32_t res_height, float vertex_height);

    bool get_bowlview_vertex_model (
        VertexMap &vertices, PointMap &texture_points, IndexVector &indeices,
        uint32_t res_width, uint32_t res_height);

    bool get_topview_vertex_model (
        VertexMap &vertices, PointMap &texture_points, IndexVector &indeices,
        uint32_t res_width, uint32_t res_height);

private:
    BowlDataConfig    _config;
    uint32_t          _bowl_img_width, _bowl_img_height;
    float             _max_topview_width_mm;
    float             _max_topview_length_mm;
};

}

#endif //XCAM_INTERFACE_STITCHER_H
