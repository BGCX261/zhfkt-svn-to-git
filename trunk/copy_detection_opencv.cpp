#include "stdafx.h"
#include "copy_detection_class.h"
#include <map>
#include <vector>
#include <iostream>
#include <ctime>
using namespace std;

void go_process();
void go_process_video();
void go_process_to_test_dataset();

void go_cluster();

int main(int argc,char* argv[])
{
	clock_t start,finish;

	start = clock();

	//go_cluster();
	go_process();

	finish = clock();

	cout<<(double)(finish-start)/CLOCKS_PER_SEC<<" s"<<endl;

	system("pause");

	return 0;
}


void go_process()
{
    //go_process_to_test_dataset();
	go_process_video();
}


void go_process_video()
{

    video_sim_input_ess_par ess_par;
    ess_par.ref_video = "..\\Debug\\video_video\\ref_video";
    ess_par.query_video = "..\\Debug\\video_video\\query_video\\0.flv";
    ess_par.ref_frame_pic = "..\\Debug\\video_video\\ref_frame_pic";
    ess_par.query_frame_pic = "..\\Debug\\video_video\\query_frame_pic";
    ess_par.vk_path = "..\\Debug\\video_video\\vk\\vk_100.txt";
    ess_par.ref_bow = "..\\Debug\\video_video\\ref_bow";
    ess_par.detect_mode = NO_VK_NO_REF;
	ess_par.ref_frame_pic_des_and_keypoint = "..\\Debug\\video_video\\ref_frame_pic_keypoint_des";
	ess_par.query_frame_pic_des_and_keypoint = "..\\Debug\\video_video\\query_frame_pic_keypoint_des";
	ess_par.exist_ref_des_and_key_point = false;

	ess_par.hamming_embedding = true;
	ess_par.hamming_embedding_median_path = "..\\Debug\\video_video\\hamming_embedding\\median_value\\median_value.txt";
	ess_par.hamming_embedding_project_matrix = "..\\Debug\\video_video\\hamming_embedding\\project_file\\project.txt";
	ess_par.hamming_embedding_need_to_trainning = true;
	ess_par.hamming_embedding_trainning_path = "..\\Debug\\video_video\\ref_frame_pic_keypoint_des";
	ess_par.hamming_embedding_ref_path = "..\\Debug\\video_video\\hamming_embedding\\ref_bow_with_hamming_embedding";


    video_sim_input_run_par run_par;
    run_par.cluster_count = 100;
    run_par.frame_interval = 50;
	run_par.descript_type = SIFT_DESCRIPTION;
	run_par.split_method = USE_SECOND;
	run_par.cluster_file_chosen_percentage = 100;
	
	run_par.hamming_embedding_threshold = 32;
	run_par.hamming_trainning_des_chosen_percent = 100;

    map<string,double> final_result = copy_detection::video_video_v2(ess_par,run_par);
    for(map<string,double>::iterator i=final_result.begin();i!=final_result.end();i++)
    {
        string video_id = i->first;
        double final_score = i->second;

        cout<<video_id<<" "<<final_score<<endl;
    }
    
}


void go_process_to_test_dataset()
{

	for(int j=24;j<25;j++)
	{
		video_sim_input_ess_par ess_par;
		ess_par.ref_video = "D:\\Master\\copy_detection\\库\\100G数据库\\按照24Queries编制的视频库\\"+utility::int2string(j)+"\\";
		ess_par.query_video = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\"+utility::int2string(j)+"\\query_video\\0.flv";
		ess_par.ref_frame_pic = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\"+utility::int2string(j)+"\\ref_frame_pic\\";
		ess_par.query_frame_pic = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\"+utility::int2string(j)+"\\query_frame_pic\\";
		ess_par.vk_path = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\vk\\vk_1000_dim_100_random.xml";
		ess_par.ref_bow = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\"+utility::int2string(j)+"\\ref_bow\\ref_bow_1000.xml";
		ess_par.detect_mode = EXIST_VK_NO_REF;

		video_sim_input_run_par run_par;
		run_par.cluster_count = 1000;
		run_par.frame_interval = 60;

		map<string,double> final_result = copy_detection::video_video(ess_par,run_par);
		for(map<string,double>::iterator i=final_result.begin();i!=final_result.end();i++)
		{
			string video_id = i->first;
			double final_score = i->second;

			string result = "D:\\Master\\copy_detection\\实验\\自写代码中间结果\\"+utility::int2string(j)+"\\result.txt";

			ofstream input_file(result,ios::app | ios::out);
			input_file << video_id<<" "<<final_score<<endl;
			input_file.close();

			cout<<video_id<<" "<<final_score<<endl;
		}
		cout<<endl;

	
	}


}

void go_cluster()
{
	copy_detection cp_de;
	cp_de.cluster("K:\\tudou_sources\\2012_task2\\frames\\0\\","F:\\tudou_sources\\2012\\vk_500_dim_10_random.xml",500,SURF_DESCRIPTION,10);
}

