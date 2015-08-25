//This class/lib is designed by zhfkt at Tongji university.If you have any question,U can contact zhfkt@hotmail.com

#ifndef __COPY_DETECTION_OPENCV__
#define __COPY_DETECTION_OPENCV__

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cmath>

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/nonfree.hpp"

using namespace std;

#include "copy_detection_structure.h"
#include "utility.h"



using namespace cv;


class copy_detection
{
 
public:

    copy_detection();
    ~copy_detection();

    static map<string,double> video_video(video_sim_input_ess_par ess_par,video_sim_input_run_par run_par);
	static map<string,double> video_video_v2(video_sim_input_ess_par ess_par,video_sim_input_run_par run_par);
    static map<string,double> pic_video(string v_path_folder,string frame_pic_d_path,string img_path,string vk_path,bool is_vk_exist,string ref_bow,bool is_ref_bow_exist,string matcher_string,bool is_matcher_exist);

    static void progress_update(string update_message_new,int percent_new);
    static void notify_update(string notify);

    void go_split(string v_path,string d_path,int frame_step,int split_mode);
	void cluster(string ref_frame_pic,string vk_path,int cluster_count,int des_type,double file_chosen_percentage = 100);
    void bow_trainning(Ptr<IplImage> image,vector<KeyPoint> keypoints,Mat &des,string file_name);
	Mat bow_trainning_revise(Mat descriptors,Mat vocabulary,vector<vector<int> > &pointIdxsOfClusters);
	void bow_trainning_set_vk_set_and_set_descriptor(Mat vk,int descript_type);
    void keypoint_detect(Ptr<IplImage> image,vector<KeyPoint>& keypoints,int descript_type);
    void description_detect(Ptr<IplImage> image,vector<KeyPoint>& keypoints,OutputArray des,int descript_type);
    video_sim_result retrieval_bow(vector<string> query_bow_file_list,Mat query_his,map<string,vector<string> > id_frame_name,map<string,Mat> id_mat);
    video_sim_result retrieval_bow_by_cosine_distance(vector<string> query_bow_file_list,Mat query_his,map<string,vector<string> > id_frame_name,map<string,Mat> id_mat);
	void get_nearest_matcher_by_cosine_distance(const Mat& queryDescriptors, const Mat& trainDescriptors, vector<Cpde_Matcher>& matches);
	double cosine_distance(vector<float> vector_query,vector<float> vector_ref);
	map<string,double> hough_transform(video_sim_result pic_list);


	void hamming_embedding_trainning(string des_folder_path,string output_median_value_path,Mat vk,string project_matrix_path,double hamming_trainning_des_chosen_percent = 100);
	vector<vector<vector<bool> > > hamming_embedding_generate(string input_median_value_path,string project_matrix_path,Mat vk,Mat all_descriptors);
	//just for one image
	//int = cluster_id,int = mat_line_id,vector<bool> = binary_signature_of_mat_line_id
	double hamming_embedding_cal(vector<vector<vector<bool> > > hamming_embedding_result_for_a_image,vector<vector<vector<bool> > > hamming_embedding_result_for_b_image,int hamming_embedding_threshold);
	int hamming_distance_cal(vector<bool> bool_a,vector<bool> bool_b);
	void get_nearest_matcher_by_cosine_distance_hamming_embedding(vector<vector<vector<vector<bool> > > > a_video_hamming_embedding_result, vector<vector<vector<vector<bool> > > > b_video_hamming_embedding_result, vector<Cpde_Matcher>& matches,int hamming_embedding_threshold);
	video_sim_result retrieval_bow_by_cosine_distance_with_hamming_embedding(vector<string> query_bow_file_list,vector<vector<vector<vector<bool> > > > query_image_hamming_embedding_result,map<string,vector<string> > id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > all_hamming_embedding_result,int hamming_embedding_threshold = 32);
	bool check_whether_exist_point_in_hamming_embedding(vector<vector<vector<bool> > > hamming_embedding_result_for_image);

	void save_mat(Mat mat_save,string path);
	Mat load_mat(string path);
	void load_ref(Mat &mat_ref,vector<string> &ref_filename,string path);
	void save_ref(Mat mat_ref,vector<string> ref_filename,string path);
    void load_video_ref(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string path);
    void save_video_ref(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string path);


	void load_video_ref_txt_by_single_with_hamming_embedding(map<string,vector<string> > &id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > &all_hamming_embedding_result,string input_path_dir);
	void save_video_ref_txt_by_single_with_hamming_embedding(map<string,vector<string> > id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > all_hamming_embedding_result,string output_path_dir);
	void save_image_keypoint_and_des_txt_by_single(vector<KeyPoint> image_keypoint,Mat image_descriptor,string keypoint_and_des_path);
	Mat load_image_keypoint_and_des_txt_by_single(vector<KeyPoint> &image_keypoint,string keypoint_and_des_path);
	void save_video_ref_txt_by_single(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string output_path_dir);
	void load_video_ref_txt_by_single(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string input_path_dir);
	void save_video_ref_txt(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string path);
	void load_video_ref_txt(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string path);
	Mat load_mat_txt(string input_path);
	void save_mat_txt(Mat vk_bow,string output_path);

	void show_mat(Mat show_mat);

    
protected:
    
    int split_frames(string,string,int,int);
    void log_record(string);
	Mat bow_cluster(vector<Mat> all_mat,int cluster_count);
       
    BOWImgDescriptorExtractor *bowide;
};



#endif /*__COPY_DETECTION_OPENCV__*/
