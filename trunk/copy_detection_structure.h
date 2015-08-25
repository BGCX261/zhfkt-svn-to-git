#ifndef __COPY_DETECTION_STRUCTURE__
#define __COPY_DETECTION_STRUCTURE__

#include <vector>

#define NO_VK_NO_REF 0
#define EXIST_VK_NO_REF 1
#define EXIST_VK_EXIST_REF 2
//NO_VK_NO_REF and EXIST_VK_NO_REF will generate ref_frame_pic_des_and_keypoint
//all will generate query_frame_pic_des_and_keypoint

#define USE_FRAME 0
#define USE_SECOND 1

#define SIFT_DESCRIPTION 0
#define SURF_DESCRIPTION 1

#define UPDATE_NOTIFY 2001
#define MESSAGE_NOTIFY 2002


//enum detect_mode_ {NO_VK_NO_REF_,EXIST_VK_NO_REF_,EXIST_VK_EXIST_REF_};
//enum split_method_ {USE_FRAME_,USE_SECOND_};
//enum descript_type_ {SIFT_DESCRIPTION_,SURF_DESCRIPTION_};//在下一版本中加入



struct video_sim_input_ess_par
{
    string ref_video;
    string query_video;
    string ref_frame_pic;
    string query_frame_pic;
    string vk_path;
    string ref_bow;
    int detect_mode;

	//----------------v2

	bool exist_ref_des_and_key_point;
	string ref_frame_pic_des_and_keypoint;
	string query_frame_pic_des_and_keypoint;

	bool hamming_embedding;
	string hamming_embedding_median_path;
	string hamming_embedding_project_matrix;

	bool hamming_embedding_need_to_trainning;
	string hamming_embedding_trainning_path;

	string hamming_embedding_ref_path;

	video_sim_input_ess_par()
	{
		exist_ref_des_and_key_point = false;
		ref_frame_pic_des_and_keypoint = "";
		query_frame_pic_des_and_keypoint = "";

		hamming_embedding = false;
		hamming_embedding_median_path = "";
		hamming_embedding_project_matrix = "";

		hamming_embedding_need_to_trainning = false;
		hamming_embedding_trainning_path = "";

		hamming_embedding_ref_path = "";
	}

};

struct video_sim_input_run_par
{
    int frame_interval;
    int cluster_count;
	int descript_type;
	int split_method;

	double cluster_file_chosen_percentage;
	double hamming_trainning_des_chosen_percent;
	int hamming_embedding_threshold;

	video_sim_input_run_par()
	{
		cluster_file_chosen_percentage = 100;
		hamming_trainning_des_chosen_percent = 100;
		hamming_embedding_threshold = 32;
	}
};

struct video_sim_result
{
    vector<string> query_set;
    vector<vector<string> > ref_set;
    vector<vector<double> > distance;
};

struct pic_sim_result
{
    string query_set;
    vector<string> ref_set;
    vector<double> ref_distance;
};

struct Cpde_Matcher
{
	int trainIdx;
	double distance;
};


#endif /*__COPY_DETECTION_STRUCTURE__*/