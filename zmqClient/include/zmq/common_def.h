//
// Created by stan on 2021/10/27.
//

#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_
//train
#define  MSG_TASK_TYPE      "task_type"
#define  MSG_ModelYml_Name      "modelyml_name"
#define  MSG_Model_Name      "model_name"
#define  MSG_DataSets_Path      "datasets_path"
#define  MSG_Epoch_Num      "epoch_num"
#define  MSG_batch_size      "batch_size"

#define  MSG_CMD            "cmd"

#define  MSG_PART_TYPE      "part_type"
#define  MSG_CAMERA_TASK_TYPE      "camera_task_type"
#define  MSG_POS            "pos"
#define  MSG_MSG            "msg"
#define  MSG_RESULT         "result"
#define  MSG_ERROR         "error"
#define  MSG_CAMERA_CLASS   "camera_class"
#define  MSG_CAMERA_ID      "camera_id"
#define  MSG_CELL_NAME      "cell_name"

#define  MSG_IMAGE_NAME      "image_name"
#define  MSG_DEAPTH_NAME      "deapth_name"
#define  MSG_IMAGE_BUFF      "image_buff"
#define  MSG_DEAPTH_BUFF       "deapth_buff"
#define  MSG_CLOUD_BUFF       "cloud_buff"

#define CELL_VIZ          "cell_viz"
#define CELL_MLA_AGRITHM       "cell_mla_agrithm"
#define CELL_MLA_ENGINE     "cell_mla_engine"
#define CELL_CAMERA       "cell_camera"
#define CELL_UI           "cell_ui"
#define CELL_SOURCE           "cell_source"

#define CELL_MLA_CAM1     "cell_mla_cam1"
#define CELL_MLA_CAM2     "cell_mla_cam2"
#define CELL_MLA_CAM3     "cell_mla_cam3"
#define CELL_MLA_CAM4     "cell_mla_cam4"
#define CELL_MLA_CAM5     "cell_mla_cam5"
#define CELL_MLA_CAM6     "cell_mla_cam6"


enum ECmd
{
    E_CMD_UNKNOWN = -1,
    E_CMD_TAKE_PHOTO = 0,              
    E_CMD_TAKE_RESULT = 1,           
};

enum ETaskType
{
    E_TASK_TYPE_UNKNOWN = -1,
    E_TASK_TYPE_CORRECT  = 0,         
    E_TASK_TYPE_CAPTURE  = 1,    
};

enum ECameraTasks
{
    E_CMAERA_TASK_TYPE_UNKNOWN = -1,
    E_CMAERA_TASK_TYPE_PHOTO,
    E_CMAERA_TASK_TYPE_PHOTO_DEAPTH,
    E_CMAERA_TASK_TYPE_PHOTO_CLOUD  ,
    E_CMAERA_TASK_TYPE_PATH,
};
enum EAlgrithmCmd
{

};


#endif //ALL_MSG_CMD_H
