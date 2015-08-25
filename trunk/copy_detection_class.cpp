#include "stdafx.h"
#include "copy_detection_class.h"

copy_detection::copy_detection()
{
    bowide=NULL;
}

copy_detection::~copy_detection()
{
	if(bowide!=NULL)
	{
		delete bowide;
	}
}

void copy_detection::progress_update(string update_message_new,int percent_new)
{
    #ifdef _MFC_VER       
    
		string *update_message_new_at_heap = new string;   
		*update_message_new_at_heap = update_message_new;

		::SendMessage(AfxGetApp()->GetMainWnd()->m_hWnd,UPDATE_NOTIFY,WPARAM(update_message_new_at_heap),LPARAM(percent_new));

		delete update_message_new_at_heap;
    
	#else

		#ifdef WIN32
			cout<<update_message_new<<" "<<"完成进度"<<percent_new<<"%"<<endl;
		#else

		#endif

	#endif

}

void copy_detection::notify_update(string notify)
{
    notify+="\r\n";

    #ifdef _MFC_VER       

        string *update_message_new_at_heap = new string;   
        *update_message_new_at_heap = notify;

        ::SendMessage(AfxGetApp()->GetMainWnd()->m_hWnd,MESSAGE_NOTIFY,WPARAM(update_message_new_at_heap),NULL);

        delete update_message_new_at_heap;
        
    #else

		#ifdef WIN32
			cout<<notify;
		#else
		
		#endif
      
    #endif

}


map<string,double> copy_detection::hough_transform(video_sim_result pic_list)
{
    vector<double> hough_result;
    for(int j=0;j<pic_list.ref_set.size();j++)
    {
        map<int,double> hough_2d;    
        for(int i=0;i<pic_list.query_set.size();i++)
        {
            string query_v = pic_list.query_set[i];
            vector<string> query_ser = utility::split(query_v,'_');
            int query_frame_count = utility::string2int(query_ser[query_ser.size()-1]);

            string ref_v = pic_list.ref_set[j][i];
            vector<string> ref_ser = utility::split(ref_v,'_');
            int ref_frame_count = utility::string2int(ref_ser[ref_ser.size()-1]);

            int frame_interval = abs(query_frame_count-ref_frame_count);

            if(hough_2d.find(frame_interval)!=hough_2d.end())
            {
                hough_2d[frame_interval] += (pic_list.distance[j][i]); //特殊处理
            }
            else
            {
                hough_2d[frame_interval] = (pic_list.distance[j][i]); //特殊处理
            }           
        }

        double max_score = 0;
        for(map<int,double>::iterator i=hough_2d.begin();i!=hough_2d.end();i++)
        {
            if((i->second)>max_score)
            {
                max_score = i->second;
            }
        }

        max_score/=pic_list.query_set.size();  //特殊处理

        hough_result.push_back(max_score);
    }
    
    
    map<string,double> final_result;
    for(int i=0;i<hough_result.size();i++)
    {
        string video_id = utility::split(pic_list.ref_set[i][0],'_')[0];    
        double score = hough_result[i];
        final_result[video_id] = score;
    }    
    return final_result;
}
//在是用Hough Transform后，必须保证查询集和参照集帧间隔相同，否则会偏低（包括是参照集帧数的公约数以及非公约数）


void copy_detection::save_video_ref_txt(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string output_path)
{
	ofstream o_file;
	o_file.open(output_path.c_str());

	for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
	{
		string video_id = i->first;

		vector<string> single_frame_name = id_frame_name[video_id];
		Mat single_mat = id_mat[video_id];

		for(int j=0;j<single_frame_name.size();j++)
		{
			string frame_name = single_frame_name[j];

			o_file<<video_id<<" "<<frame_name;

			for(int k=0;k<single_mat.cols;k++)
			{
				float f_num = single_mat.at<float>(j,k);
				o_file<<" "<<f_num;
			}
			o_file<<endl;
		}
	}
	o_file.close();
}

void copy_detection::load_video_ref_txt(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string input_path)
{
	ifstream i_file;
	i_file.open(input_path.c_str());

	map<string, vector<vector<float> > > id_bow;
	
	string each_line;  
	while(getline(i_file,each_line))
	{
		string video_id;
		string frame_name;

		stringstream ss;
		ss<<each_line;
		ss>>video_id;
		ss>>frame_name;

		vector<float> each_line_bow;

		float entry_each_bow;
		while (ss>>entry_each_bow)
		{
			each_line_bow.push_back(entry_each_bow);
		}

		vector<string> single_frame_name;
		vector<vector<float> > single_frame_bow;

		if(id_frame_name.find(video_id)!=id_frame_name.end())//可以找到
		{
			single_frame_name = id_frame_name[video_id];
			single_frame_bow = id_bow[video_id];
		}

		single_frame_name.push_back(frame_name);
		single_frame_bow.push_back(each_line_bow);

		id_frame_name[video_id] = single_frame_name;
		id_bow[video_id] = single_frame_bow;

	}

	i_file.close();



	for(map<string, vector<vector<float> > >::iterator it=id_bow.begin();it!=id_bow.end();it++)
	{
		//id_mat
		string v_id = it->first;
		vector<vector<float> > v_bow = it->second;

		int mat_rows = v_bow.size();
		int mat_cols = v_bow[0].size();

		Mat v_mat = Mat::zeros(mat_rows, mat_cols, CV_32F);
		
		for(int i=0;i<mat_rows;i++)
		{
			for(int j=0;j<mat_cols;j++)
			{
				v_mat.at<float>(i,j)=v_bow[i][j];
			}
		}

		id_mat[v_id] =  v_mat;
	}

}

void copy_detection::save_video_ref_txt_by_single_with_hamming_embedding(map<string,vector<string> > id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > all_hamming_embedding_result,string output_path_dir)
{
	for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
	{
		string video_id = i->first;		
		string output_path = output_path_dir + path_split + video_id + ".txt";

		ofstream o_file;
		o_file.open(output_path.c_str());

		vector<string> single_frame_name = id_frame_name[video_id];
		vector<vector<vector<vector<bool> > > > hamming_embedding_result_for_one_video = all_hamming_embedding_result[video_id];

		for(int j=0;j<single_frame_name.size();j++)//hamming_embedding_result_for_one_video.size()==image_num
		{
			string frame_name = single_frame_name[j];
			o_file<<frame_name<<endl;

			//o_file<<"ALL_BINARY_SIGNATURE"<<endl;

			vector<vector<vector<bool> > > hamming_embedding_result_for_one_image = hamming_embedding_result_for_one_video[j];

			for(int c1 = 0;c1<hamming_embedding_result_for_one_image.size();c1++)//hamming_embedding_result_for_one_image.size()==cluster_num
			{
				vector<vector<bool> > hamming_signature_for_one_cluster = hamming_embedding_result_for_one_image[c1];

				for(int b1 = 0;b1<hamming_signature_for_one_cluster.size();b1++)
				{
					for(int b2 = 0;b2<hamming_signature_for_one_cluster[0].size();b2++)//hamming_signature_for_one_cluster[0].size()==32
					{
						o_file<<hamming_signature_for_one_cluster[b1][b2]<<" ";
					}
					o_file<<endl;
				}

				o_file<<"EACH_BINARY_SIGNATURE"<<endl;

			}

			o_file<<"END_BINARY_SIGNATURE"<<endl;
		}

		o_file.close();

	}
}

void copy_detection::load_video_ref_txt_by_single_with_hamming_embedding(map<string,vector<string> > &id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > &all_hamming_embedding_result,string input_path_dir)
{
	vector<string> input_path_vector = utility::EnumFiles(input_path_dir);
	

	for(int file_i=0;file_i<input_path_vector.size();file_i++)
	{
		vector<vector<vector<vector<bool> > > > hamming_embedding_result_for_one_video;
		string input_path = input_path_dir + path_split + input_path_vector[file_i];
		string video_id = utility::split(input_path_vector[file_i],'.')[0];

		ifstream i_file;
		i_file.open(input_path.c_str());

		

		vector<string> single_frame_name;
		vector<vector<float> > single_frame_bow;

		string each_line;  
		while(getline(i_file,each_line))
		{

			string frame_name;
			stringstream ss;
			ss<<each_line;
			ss>>frame_name;

			//-------

			vector<vector<vector<bool> > > hamming_embedding_result_for_one_image;
			vector<vector<bool> > vector_bool_array;

			
			while(getline(i_file,each_line))
			{
				if(each_line=="EACH_BINARY_SIGNATURE")
				{
					hamming_embedding_result_for_one_image.push_back(vector_bool_array);
					vector_bool_array.clear();
					continue;
				}

				if(each_line=="END_BINARY_SIGNATURE")
				{
					break;
				}

				stringstream ss_1;
				//ss.str("");
				//ss.clear();//for vs
				//ss.str().clear();//for gnu

				ss_1<<each_line;

				vector<bool> vector_bool;
				bool bool_value;

				while (ss_1>>bool_value)
				{
					vector_bool.push_back(bool_value);
				}

				vector_bool_array.push_back(vector_bool);
			}

			single_frame_name.push_back(frame_name);
			hamming_embedding_result_for_one_video.push_back(hamming_embedding_result_for_one_image);		

		}

		i_file.close();

		id_frame_name[video_id] = single_frame_name;
		all_hamming_embedding_result[video_id] = hamming_embedding_result_for_one_video;
	}

}

void copy_detection::save_video_ref_txt_by_single(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string output_path_dir)
{
	for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
	{
		string video_id = i->first;		
		string output_path = output_path_dir + path_split + video_id + ".txt";

		ofstream o_file;
		o_file.open(output_path.c_str());

		vector<string> single_frame_name = id_frame_name[video_id];
		Mat single_mat = id_mat[video_id];

		for(int j=0;j<single_frame_name.size();j++)
		{
			string frame_name = single_frame_name[j];
			o_file<<frame_name<<endl;

			for(int k=0;k<single_mat.cols;k++)
			{
				float f_num = single_mat.at<float>(j,k);
				o_file<<f_num<<" ";
			}
			o_file<<endl;
		}
		
		o_file.close();

	}

}

void copy_detection::load_video_ref_txt_by_single(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string input_path_dir)
{
	vector<string> input_path_vector = utility::EnumFiles(input_path_dir);


	map<string, vector<vector<float> > > id_bow;
	for(int file_i=0;file_i<input_path_vector.size();file_i++)
	{
		string input_path = input_path_dir + path_split + input_path_vector[file_i];
		string video_id = utility::split(input_path_vector[file_i],'.')[0];

		ifstream i_file;
		i_file.open(input_path.c_str());


		vector<string> single_frame_name;
		vector<vector<float> > single_frame_bow;

		string each_line;  
		while(getline(i_file,each_line))
		{
			string frame_name;

			stringstream ss;
			ss<<each_line;
			ss>>frame_name;

			getline(i_file,each_line);
			stringstream ss_1;
			//ss.clear();
			ss_1<<each_line;

			vector<float> each_line_bow;
			float entry_each_bow;

			while (ss_1>>entry_each_bow)
			{
				each_line_bow.push_back(entry_each_bow);
			}

			single_frame_name.push_back(frame_name);
			single_frame_bow.push_back(each_line_bow);

		}

		id_frame_name[video_id] = single_frame_name;
		id_bow[video_id] = single_frame_bow;

		i_file.close();

	}

	for(map<string, vector<vector<float> > >::iterator it=id_bow.begin();it!=id_bow.end();it++)
	{
		//id_mat
		string v_id = it->first;
		vector<vector<float> > v_bow = it->second;

		int mat_rows = v_bow.size();
		int mat_cols = v_bow[0].size();

		Mat v_mat = Mat::zeros(mat_rows, mat_cols, CV_32F);

		for(int i=0;i<mat_rows;i++)
		{
			for(int j=0;j<mat_cols;j++)
			{
				v_mat.at<float>(i,j)=v_bow[i][j];
			}
		}

		id_mat[v_id] =  v_mat;
	}

}

Mat copy_detection::load_mat_txt(string input_path)
{
	ifstream i_file;
	i_file.open(input_path.c_str());

	vector<vector<float> >  vk_bow;

	string each_line;  
	while(getline(i_file,each_line))
	{

		stringstream ss;
		ss<<each_line;

		vector<float> each_line_bow;

		float entry_each_bow;
		while (ss>>entry_each_bow)
		{
			each_line_bow.push_back(entry_each_bow);
		}

		vk_bow.push_back(each_line_bow);

	}

	i_file.close();


	int mat_rows = vk_bow.size();
	int mat_cols = vk_bow[0].size();

	Mat v_mat = Mat::zeros(mat_rows, mat_cols, CV_32F);

	for(int i=0;i<mat_rows;i++)
	{
		for(int j=0;j<mat_cols;j++)
		{
			v_mat.at<float>(i,j)=vk_bow[i][j];
		}
	}

	return v_mat;

}

void copy_detection::save_mat_txt(Mat vk_bow,string output_path)
{
	ofstream o_file;
	o_file.open(output_path.c_str());


	for(int i=0;i<vk_bow.rows;i++)
	{
		for(int j=0;j<vk_bow.cols;j++)
		{
			o_file<<vk_bow.at<float>(i,j)<<" ";
		}
		o_file<<endl;
	}

	o_file.close();

}

void copy_detection::save_image_keypoint_and_des_txt_by_single(vector<KeyPoint> image_keypoint,Mat image_descriptor,string keypoint_and_des_path)
{

	ofstream o_file;

	o_file.open(keypoint_and_des_path.c_str());


	for(int i=0;i<image_keypoint.size();i++)
	{
		KeyPoint single_keypoint = image_keypoint[i];
		Point2f pt = single_keypoint.pt; 

		float pt_x = pt.x;
		float pt_y = pt.y;
		float size = single_keypoint.size; 
		float angle = single_keypoint.angle; 
		float response = single_keypoint.response;
		int octave = single_keypoint.octave; 
		int class_id = single_keypoint.class_id;
		
		o_file<<pt_x<<" "<<pt_y<<" "<<size<<" "<<angle<<" "<<response<<" "<<octave<<" "<<class_id<<endl;
		
	}

	o_file<<"DUDULU"<<endl;
	o_file<<image_descriptor.rows<<" "<<image_descriptor.cols<<endl;

	for(int j=0;j<image_descriptor.rows;j++)
	{
		for(int k=0;k<image_descriptor.cols;k++)
		{
			o_file<<image_descriptor.at<float>(j,k)<<" ";
		}
		o_file<<endl;
	}

	o_file.close();
	
}

Mat copy_detection::load_image_keypoint_and_des_txt_by_single(vector<KeyPoint> &image_keypoint,string keypoint_and_des_path)
{
	ifstream i_file;
	i_file.open(keypoint_and_des_path.c_str());

	string each_line;  
	while(getline(i_file,each_line))
	{
		if(each_line=="DUDULU")
		{
			break;
		}

		float pt_x;
		float pt_y;
		float size; 
		float angle; 
		float response;
		int octave; 
		int class_id;

		stringstream ss;
		ss<<each_line;
		ss>>pt_x>>pt_y>>size>>angle>>response>>octave>>class_id;

		KeyPoint single_keypoint(pt_x,pt_y,size,angle,response,octave,class_id);
		image_keypoint.push_back(single_keypoint);

	}



	int rows;
	int cols;
	stringstream ss;

	getline(i_file,each_line);

	ss<<each_line;
	ss>>rows>>cols;
	

	Mat image_des = Mat::zeros(rows, cols, CV_32F);

	for(int i=0;i<rows;i++)
	{
		for(int j=0;j<cols;j++)
		{
			float des_value;
			i_file>>des_value;
			image_des.at<float>(i,j)=des_value;
		}
	}

	return image_des;
}

void copy_detection::load_video_ref(map<string,vector<string> > &id_frame_name,map<string,Mat> &id_mat,string path)
{
    FileStorage fs(path.c_str(), FileStorage::READ); 

    vector<string> id_list;
    fs["id_list"]>>id_list;

    for(int i=0;i<id_list.size();i++)
    {
        string video_id = id_list[i];
        fs["id_frame_mat" + video_id] >> id_frame_name[video_id];
        fs["id_mat" + video_id] >> id_mat[video_id];
    }

    fs.release();
}

void copy_detection::save_video_ref(map<string,vector<string> > id_frame_name,map<string,Mat> id_mat,string path)
{
    FileStorage fs(path.c_str(), FileStorage::WRITE);  
    //fs<<"id_frame_name"<<id_frame_name<<"id_mat"<<id_mat;  


    vector<string> id_list;
    for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
    {
        string video_id = i->first;
        id_list.push_back(video_id);
    }

    fs<<"id_list"<<id_list;

    for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
    {
        string video_id = i->first;
        fs<<"id_frame_mat" + video_id<<id_frame_name[video_id];
        fs<<"id_mat" + video_id<<id_mat[video_id];
    }


    fs.release();  
}

void copy_detection::save_ref(Mat mat_ref,vector<string> ref_filename,string path)
{
    FileStorage fs(path.c_str(), FileStorage::WRITE);  
    fs<<"ref_filename"<<ref_filename<<"mat_ref"<<mat_ref;  
    fs.release();  
}

void copy_detection::load_ref(Mat &mat_ref,vector<string> &ref_filename,string path)
{
    FileStorage fs(path.c_str(), FileStorage::READ);  
    fs["mat_ref"] >> mat_ref;  
    fs["ref_filename"] >> ref_filename;      
    fs.release();
}

void copy_detection::save_mat(Mat mat_save,string path)
{
    FileStorage fs(path.c_str(), FileStorage::WRITE);  
    fs<<"vocabulary"<<mat_save;  
    fs.release();  
}

Mat copy_detection::load_mat(string path)
{
    FileStorage fs(path.c_str(), FileStorage::READ);  
    Mat mat_vocabulary;  
    fs["vocabulary"] >> mat_vocabulary;  
    fs.release();
    return mat_vocabulary;
}

void copy_detection::description_detect(Ptr<IplImage> image,vector<KeyPoint>& keypoints,OutputArray des,int descript_type)
{
    Mat image_mat(image);

	if(descript_type==SIFT_DESCRIPTION)
	{
		SIFT image_descript;
		image_descript(image_mat,noArray(),keypoints,des,false);
	}
	else if(descript_type==SURF_DESCRIPTION)
	{
		SURF image_descript;
		image_descript(image_mat,noArray(),keypoints,des,false);
	}


}

void copy_detection::keypoint_detect(Ptr<IplImage> image,vector<KeyPoint>& keypoints,int descript_type)
{
    Mat image_mat(image);

	if(descript_type==SIFT_DESCRIPTION)
	{
		SIFT image_descript;
		image_descript(image_mat,noArray(),keypoints,noArray(),false);
	}
	else if(descript_type==SURF_DESCRIPTION)
	{
		SURF image_descript;
		image_descript(image_mat,noArray(),keypoints,noArray(),false);
	}
};

void copy_detection::bow_trainning_set_vk_set_and_set_descriptor(Mat vk,int descript_type)
{   
	string des_type = "";

	if (descript_type==SIFT_DESCRIPTION)
	{
		des_type = "SIFT";
	} 
	else if(descript_type==SURF_DESCRIPTION)
	{
		des_type = "SURF";
	}

    Ptr<DescriptorExtractor> extractor = DescriptorExtractor::create(des_type);
    //Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce");
	//Ptr<DescriptorMatcher> matcher =  Ptr<DescriptorMatcher>(new FlannBasedMatcher(new flann::AutotunedIndexParams()));//new flann::AutotunedIndexParams()
    bowide = new BOWImgDescriptorExtractor(extractor, matcher);
    bowide->setVocabulary(vk);
}

void copy_detection::bow_trainning(Ptr<IplImage> image,vector<KeyPoint> keypoints,Mat &his/*,vector<vector<int>>* pointIdxsOfClusters, Mat* descriptors*/,string file_name)
{
	//clock_t start,finish;

	//start = clock();
	
    Mat image_mat(image);
    bowide->compute(image_mat,keypoints,his/*,pointIdxsOfClusters,descriptors*/);//实际上这里索引一直进行重复建立，可以重写BOWImgDescriptorExtractor.compute方法使比对时只建立一次,且并不难,参见opencv源码bagofwords.cpp和DescriptorMatcher::knnMatch中的train
    
	//finish = clock();

	//cout<<(double)(finish-start)/CLOCKS_PER_SEC<<" s"<<endl;

    string bow_trainning_string = "量化"+file_name+"文件BOW完毕";
    copy_detection::notify_update(bow_trainning_string);
}

Mat copy_detection::bow_trainning_revise(Mat descriptors,Mat vocabulary,vector<vector<int> > &pointIdxsOfClusters)
{
	Ptr<DescriptorMatcher> dmatcher = DescriptorMatcher::create("BruteForce");
	dmatcher->add( vector<Mat>(1, vocabulary) );
	int clusterCount = vocabulary.rows;



	vector<DMatch> matches;
	dmatcher->match( descriptors, matches );
	// Compute image descriptor

	pointIdxsOfClusters.resize(clusterCount);
	
	Mat imgDescriptor = Mat( 1, clusterCount, CV_32FC1, Scalar::all(0.0) );
	float *dptr = (float*)imgDescriptor.data;
	for( size_t i = 0; i < matches.size(); i++ )
	{
		int queryIdx = matches[i].queryIdx;
		int trainIdx = matches[i].trainIdx; // cluster index
		CV_Assert( queryIdx == (int)i );

		dptr[trainIdx] = dptr[trainIdx] + 1.f;
		pointIdxsOfClusters[trainIdx].push_back( queryIdx );
	}

	if(descriptors.rows==0)
	{
		return Mat( 0, clusterCount, CV_32FC1, Scalar::all(0.0) );
	}
	else
	{
		// Normalize image descriptor.
		imgDescriptor /= descriptors.rows;

		return imgDescriptor;
	}





}

Mat copy_detection::bow_cluster(vector<Mat> all_mat,int cluster_count)
{
    BOWKMeansTrainer vk_cluster(cluster_count);

    for(int i=0;i<all_mat.size();i++)
    {
        if(all_mat[i].rows>0)
        {
            vk_cluster.add(all_mat[i]);
        }        
    }

    return vk_cluster.cluster();
}

void copy_detection::go_split(string v_path,string d_path,int frame_step,int split_mode)
{

    if (d_path[d_path.length()-1]==path_split[0])
    {
        d_path = d_path.substr(0,d_path.length()-1);
    }

    if(-1==split_frames(v_path,d_path,frame_step,split_mode))
    {
        log_record(v_path +" split frames failed\n");
    }
    else
    {
        string v_path_completed = "视频分帧完毕 "+v_path;
        copy_detection::notify_update(v_path_completed);
    }
}

int copy_detection::split_frames(string video_name,string dest_path,int frame_step,int split_mode)
{
	vector<string> video_ser = utility::split(video_name,'.');
	string video_type = video_ser[video_ser.size()-1];

	string video_id = utility::get_single_name(video_name);

	string filter_some_video[] = {"5221","8203","3377","3378"};
	int filter_size = sizeof(filter_some_video)/sizeof(filter_some_video[0]);

	for(int i=0;i<filter_size;i++)
	{
		if(video_type=="rm" || video_id==filter_some_video[i])
		{
			return -1;
		}
	}



    CvCapture *pCapture = cvCreateFileCapture(video_name.c_str());

    if(pCapture==NULL)
    {
        return -1;
    }

    int total_frames = cvGetCaptureProperty(pCapture,CV_CAP_PROP_FRAME_COUNT);
    int fps = cvGetCaptureProperty(pCapture,CV_CAP_PROP_FPS);
	string string_fps = utility::int2string(fps);


	int increasement = 0;

	if(split_mode==USE_FRAME)
	{
		increasement = frame_step;
	}
	else if(split_mode==USE_SECOND)
	{
		increasement = frame_step * fps;
	}
	else
	{
		return -1;
	}	 

    for(int i=0,k=0;i<total_frames;k++,i+=increasement)
    {
        string split_frame_id = utility::int2string(k);
		string total_frame_id ="";

		if(split_mode==USE_FRAME)
		{
			total_frame_id = utility::int2string(i);
		}
		else if(split_mode==USE_SECOND)
		{
			total_frame_id = utility::int2string(i/fps);
		}
		else
		{
			return -1;
		}

         


        string new_path = dest_path + path_split + video_id + "_"+string_fps+"_" + split_frame_id + "_" + total_frame_id + ".jpg";
		copy_detection::notify_update(new_path);

		fstream _file;
		_file.open(new_path.c_str(),ios::in);
		if(_file)
		{
			continue;
		}

		
		cvSetCaptureProperty(pCapture,CV_CAP_PROP_POS_FRAMES,i);
		cvGrabFrame(pCapture);
		IplImage *pImage = cvRetrieveFrame(pCapture);


		if(pImage==NULL)
		{
			break;
		}

        cvSaveImage(new_path.c_str(), pImage);
		//cvReleaseImage(&pImage);//no need
    }

    cvReleaseCapture(&pCapture);

    return 0;


}

void copy_detection::log_record(string message)
{
    ofstream input_file("log.txt",ios::app | ios::out);
    input_file << message;
    input_file.close();
}

void copy_detection::cluster(string ref_frame_pic,string vk_path,int cluster_count,int des_type,double file_chosen_percentage)
{
	int increasement = 100/file_chosen_percentage;

	vector<Mat> all_cluster_mat;
	vector<string> file_list = utility::EnumFiles(ref_frame_pic);

	notify_update("开始载入图像文件准备聚类");
	for(int i=0;i<file_list.size();i+=increasement)
	{
		string file_name = ref_frame_pic + path_split + file_list[i];
		Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());

		vector<KeyPoint> cluster_keypoint;
		Mat des;
		description_detect(cluster_image,cluster_keypoint,des,des_type);
		all_cluster_mat.push_back(des);
	}

	notify_update("图像文件载入结束,开始聚类");
	Mat vk_save = bow_cluster(all_cluster_mat,cluster_count);

	//show_mat(vk_save);

	save_mat_txt(vk_save,vk_path.c_str());
}

void copy_detection::show_mat(Mat mat_show)
{
	for(int i=0;i<mat_show.rows;i++)
	{
		for(int j=0;j<mat_show.cols;j++)
		{
			cout<<mat_show.at<float>(i,j)<<" ";
		}
		cout<<endl;
	}
}

double copy_detection::hamming_embedding_cal(vector<vector<vector<bool> > > hamming_embedding_result_for_a_image,vector<vector<vector<bool> > > hamming_embedding_result_for_b_image,int hamming_embedding_threshold)
{
	int a_image_point_num = 0;
	int b_image_point_num = 0;

	double final_value = 0;

	assert(hamming_embedding_result_for_a_image.size()==hamming_embedding_result_for_b_image.size());//==cluster_count
	//int total_dim = hamming_embedding_result_for_a_image[0].size();

	for(int i=0;i<hamming_embedding_result_for_a_image.size();i++)
	{
		vector<vector<bool> > cluster_id_hamming_embedding_a = hamming_embedding_result_for_a_image[i];
		vector<vector<bool> > cluster_id_hamming_embedding_b = hamming_embedding_result_for_b_image[i];

		//a_image_point_num+=cluster_id_hamming_embedding_a.size();
		//b_image_point_num+=cluster_id_hamming_embedding_b.size();
		
		//不同的实现方式

		a_image_point_num+=(cluster_id_hamming_embedding_a.size()*cluster_id_hamming_embedding_a.size());
		b_image_point_num+=(cluster_id_hamming_embedding_b.size()*cluster_id_hamming_embedding_b.size());

		for(int j=0;j<cluster_id_hamming_embedding_a.size();j++)
		{
			for(int k=0;k<cluster_id_hamming_embedding_b.size();k++)
			{			
				if(hamming_distance_cal(cluster_id_hamming_embedding_a[j],cluster_id_hamming_embedding_b[k])<=hamming_embedding_threshold)
				{
					final_value++;
				}
			}
		}
	}

	assert(a_image_point_num*b_image_point_num!=0);//这种情况已经在check_whether_exist_point_in_hamming_embedding进行了处理

	float sqrt_a = std::sqrt(float(a_image_point_num));
	float sqrt_b = std::sqrt(float(b_image_point_num));
	final_value/=(sqrt_a*sqrt_b);

	return final_value;

}

int copy_detection::hamming_distance_cal(vector<bool> bool_a,vector<bool> bool_b)
{
	assert(bool_a.size()==bool_b.size());

	int hamming_distance = 0;

	for(int i=0;i<bool_a.size();i++)
	{
		if(bool_a[i]!=bool_b[i])
		{
			hamming_distance++;
		}
	}

	return hamming_distance;

}//以后可以把vector<bool>换成int


vector<vector<vector<bool> > > copy_detection::hamming_embedding_generate(string input_median_value_path,string project_matrix_path,Mat vk,Mat all_descriptors)
{
	Mat all_median_values = load_mat_txt(input_median_value_path);
	Mat project_matrix = load_mat_txt(project_matrix_path);

	vector<vector<int> > pointIdxsOfClusters;
	bow_trainning_revise(all_descriptors,vk,pointIdxsOfClusters);

	assert(all_descriptors.cols==project_matrix.rows);

	Mat all_descriptors_after_project = all_descriptors * project_matrix;

	vector<vector<vector<bool> > > hamming_embedding_result_for_one_image;

	for(int i=0;i<pointIdxsOfClusters.size();i++)
	{
		vector<vector<bool> > hamming_embedding_result_for_one_cluster;

		vector<int> entry_idx_points = pointIdxsOfClusters[i];

		for(int j=0;j<entry_idx_points.size();j++)
		{
			vector<bool> hamming_signature;

			int indice_point = entry_idx_points[j];
			for (int k=0;k<all_descriptors_after_project.cols;k++)
			{
				 if(all_descriptors_after_project.at<float>(indice_point,k) > all_median_values.at<float>(i,k))
				 {
					 hamming_signature.push_back(1);
				 }
				 else
				 {
					 hamming_signature.push_back(0);
				 }
			}

			hamming_embedding_result_for_one_cluster.push_back(hamming_signature);

		}

		hamming_embedding_result_for_one_image.push_back(hamming_embedding_result_for_one_cluster);

	}

	return hamming_embedding_result_for_one_image;

}

bool copy_detection::check_whether_exist_point_in_hamming_embedding(vector<vector<vector<bool> > > hamming_embedding_result_for_image)
{
	int total_point = 0;

	for(int i=0;i<hamming_embedding_result_for_image.size();i++)
	{
		vector<vector<bool> > cluster_id_hamming_embedding = hamming_embedding_result_for_image[i];
		total_point+=cluster_id_hamming_embedding.size();
	}

	if (total_point==0)
	{
		return false;
	}
	else
	{
		return true;
	}

}



void copy_detection::hamming_embedding_trainning(string des_folder_path,string output_median_value_path,Mat vk,string project_matrix_path,double hamming_trainning_des_chosen_percent)
{
	vector<string> des_list = utility::EnumFiles(des_folder_path);
	int increasement = 100/hamming_trainning_des_chosen_percent;

	Mat he_trainning_mat;
	for(int i=0;i<des_list.size();i+=increasement)
	{
		vector<KeyPoint> cluster_keypoint;

		string des_keypoint_path = des_folder_path + path_split + des_list[i];
		Mat des = load_image_keypoint_and_des_txt_by_single(cluster_keypoint,des_keypoint_path);
		he_trainning_mat.push_back(des);
	}

	vector<vector<int> > pointIdxsOfClusters; 
	Mat cluster_his_single = bow_trainning_revise(he_trainning_mat,vk,pointIdxsOfClusters);

	Mat project_matrix = load_mat_txt(project_matrix_path);
	Mat after_he_trainning_mat = he_trainning_mat*project_matrix;

	//show_mat(project_matrix);
	//show_mat(he_trainning_mat);
	//show_mat(project_matrix);

	vector<vector<float> > all_median_values;

	for(int i=0;i<pointIdxsOfClusters.size();i++)
	{
		vector<float> median_values(after_he_trainning_mat.cols,0);

		vector<int> entry_idx_points = pointIdxsOfClusters[i];
		for(int j=0;j<entry_idx_points.size();j++)
		{
			int indice_point = entry_idx_points[j];
			for (int k=0;k<after_he_trainning_mat.cols;k++)
			{
				median_values[k] += after_he_trainning_mat.at<float>(indice_point,k); 
			}
		}

		for(int k=0;k<after_he_trainning_mat.cols;k++)
		{
			median_values[k]/=entry_idx_points.size();
		}

		all_median_values.push_back(median_values);
	}


	Mat v_mat = Mat::zeros(all_median_values.size(), all_median_values[0].size(), CV_32F);

	for(int i=0;i<all_median_values.size();i++)
	{
		for(int j=0;j<all_median_values[0].size();j++)
		{
			v_mat.at<float>(i,j) = all_median_values[i][j];
		}
	}

	save_mat_txt(v_mat,output_median_value_path);
	
}

map<string,double> copy_detection::video_video_v2(video_sim_input_ess_par ess_par,video_sim_input_run_par run_par)
{   

	//----------------------------------------

	string ref_video = ess_par.ref_video;
	string query_video = ess_par.query_video;
	string ref_frame_pic = ess_par.ref_frame_pic;
	string query_frame_pic = ess_par.query_frame_pic;
	string vk_path = ess_par.vk_path;
	string ref_bow = ess_par.ref_bow;
	int detect_mode = ess_par.detect_mode;

	string ref_frame_pic_des_and_keypoint = ess_par.ref_frame_pic_des_and_keypoint;
	string query_frame_pic_des_and_keypoint = ess_par.query_frame_pic_des_and_keypoint;
	bool exist_ref_des_and_key_point = ess_par.exist_ref_des_and_key_point;

	bool hamming_embedding = ess_par.hamming_embedding;
	string hamming_embedding_median_path = ess_par.hamming_embedding_median_path;
	string hamming_embedding_project_matrix = ess_par.hamming_embedding_project_matrix;

	bool hamming_embedding_need_to_trainning = ess_par.hamming_embedding_need_to_trainning;
	string hamming_embedding_trainning_path = ess_par.hamming_embedding_trainning_path;

	string hamming_embedding_ref_path = ess_par.hamming_embedding_ref_path;

	//----------------------------------------

	int frame_interval = run_par.frame_interval;
	int cluster_count = run_par.cluster_count;
	int descript_type  = run_par.descript_type;
	int split_method = run_par.split_method;
	double cluster_file_chosen_percentage = run_par.cluster_file_chosen_percentage;

	double hamming_trainning_des_chosen_percent = run_par.hamming_trainning_des_chosen_percent;
	int hamming_embedding_threshold = run_par.hamming_embedding_threshold;

	//----------------------------------------

	

	if(utility::is_folder_empty(query_frame_pic)!=1)
	{
		string warning = "警告: 文件夹不存在 或是 " + query_frame_pic + " 文件夹中已经存在若干文件,这些文件会混入最终的分帧文件,可能影响结果.建议找寻空文件夹放置 视频截取帧";
		notify_update(warning);
	}
	if(utility::is_folder_empty(ref_frame_pic)!=1)
	{
		string warning = "警告: 文件夹不存在 或是 " + ref_frame_pic + " 文件夹中已经存在若干文件,这些文件会混入最终的分帧文件,可能影响结果.建议找寻空文件夹放置 视频截取帧";
		notify_update(warning);
	}
	if(utility::is_folder_empty(ref_frame_pic_des_and_keypoint)!=1)
	{
		string warning = "警告: 文件夹不存在 或是 " + query_frame_pic_des_and_keypoint + " 文件夹中已经存在若干文件,这些文件会混入最终的图像特征提取DES文件,可能影响结果.建议找寻空文件夹放置 DES文件";
		notify_update(warning);
	}
	if(utility::is_folder_empty(query_frame_pic_des_and_keypoint)!=1)
	{
		string warning = "警告: 文件夹不存在 或是 " + ref_frame_pic_des_and_keypoint + " 文件夹中已经存在若干文件,这些文件会混入最终的图像特征提取DES文件,可能影响结果.建议找寻空文件夹放置 DES文件";
		notify_update(warning);
	}


	bool is_vk_exist;
	bool is_ref_bow_exist;

	if(detect_mode==NO_VK_NO_REF)
	{
		is_vk_exist = false;
		is_ref_bow_exist = false;
	}
	else if(detect_mode==EXIST_VK_NO_REF)
	{
		is_vk_exist = true;
		is_ref_bow_exist = false;
	}
	else//detect_mode==EXIST_VK_EXIST_REF
	{
		is_vk_exist = true;
		is_ref_bow_exist = true;
	}

	copy_detection cp_de;
	Mat vk;

	map<string,vector<string> > id_frame_name;
	
	//------------------For no_hamming

	map<string,Mat> id_mat;

	//-----------------For hamming

	map<string,vector<vector<vector<vector<bool> > > > > all_hamming_embedding_result;

	

	if(!is_ref_bow_exist)
	{
		if(!exist_ref_des_and_key_point)
		{
			progress_update("参照集分帧进行中……",5);

			vector<string> ref_video_path_list = utility::EnumFiles(ref_video);
			for(int i=0;i<ref_video_path_list.size();i++)
			{
				string video_path = ref_video + path_split + ref_video_path_list[i];
				cp_de.go_split(video_path,ref_frame_pic,frame_interval,split_method);
			}

			progress_update("参照集提取特征进行中……",15);

			vector<string> ref_file_list = utility::EnumFiles(ref_frame_pic);
			for(int i=0;i<ref_file_list.size();i++)
			{
				string file_name = ref_frame_pic+path_split+ref_file_list[i];
				Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());


				vector<KeyPoint> cluster_keypoint;
				Mat des;
				
				cp_de.description_detect(cluster_image,cluster_keypoint,des,descript_type);
				
				string des_keypoint_path = ref_frame_pic_des_and_keypoint + path_split + ref_file_list[i] + ".txt";
				cp_de.save_image_keypoint_and_des_txt_by_single(cluster_keypoint,des,des_keypoint_path);

			}			
		}

		vector<string> ref_file_list_des = utility::EnumFiles(ref_frame_pic_des_and_keypoint);
		vector<string> ref_file_list;
		for(int i=0;i<ref_file_list_des.size();i++)
		{
			string ref_file_list_string;

			vector<string> list_des = utility::split(ref_file_list_des[i],'.');
			for(int j=0;j<list_des.size()-1;j++)
			{
				ref_file_list_string += (list_des[j]+".");
			}
			ref_file_list_string = ref_file_list_string.substr(0,ref_file_list_string.size()-1);

			ref_file_list.push_back(ref_file_list_string);
		}//构造没有txt后缀的ref_file_list

		if(!is_vk_exist)
		{
			progress_update("聚类进行中……",25);

			int increasement = 100/cluster_file_chosen_percentage;

			vector<Mat> all_cluster_mat;

			notify_update("开始载入DES文件");
			for(int i=0;i<ref_file_list_des.size();i+=increasement)
			{
				vector<KeyPoint> cluster_keypoint;

				string des_keypoint_path = ref_frame_pic_des_and_keypoint + path_split + ref_file_list_des[i];
				Mat des = cp_de.load_image_keypoint_and_des_txt_by_single(cluster_keypoint,des_keypoint_path);
				all_cluster_mat.push_back(des);
			}

			notify_update("DES文件载入结束,开始聚类");
			Mat vk_save = cp_de.bow_cluster(all_cluster_mat,cluster_count);

			cp_de.save_mat_txt(vk_save,vk_path.c_str());
		}

		vk = cp_de.load_mat_txt(vk_path.c_str());



		if (hamming_embedding&&hamming_embedding_need_to_trainning)
		{
			progress_update("Hamming_embedding训练进行中……",40);

			cp_de.hamming_embedding_trainning(ref_frame_pic_des_and_keypoint,hamming_embedding_median_path,vk,hamming_embedding_project_matrix,hamming_trainning_des_chosen_percent);
		}


		if (hamming_embedding)
		{
			progress_update("参照集BOW/Hamming_embedding进行中……",50);
		}
		else
		{
			progress_update("参照集BOW进行中……",50);
			
		}

		for(int i=0;i<ref_file_list_des.size();i++)
		{
			vector<KeyPoint> cluster_keypoint;

			string des_keypoint_path = ref_frame_pic_des_and_keypoint + path_split + ref_file_list_des[i];
			Mat des = cp_de.load_image_keypoint_and_des_txt_by_single(cluster_keypoint,des_keypoint_path);

			string video_id = utility::split(ref_file_list[i],'_')[0];	 
			vector<string> single_frame_name;

			if (hamming_embedding)
			{			
				vector<vector<vector<bool> > > hamming_embedding_result_for_image = cp_de.hamming_embedding_generate(hamming_embedding_median_path,hamming_embedding_project_matrix,vk,des);

				if(cp_de.check_whether_exist_point_in_hamming_embedding(hamming_embedding_result_for_image))
				{
					vector<vector<vector<vector<bool> > > > hamming_embedding_result_for_one_video;//太长的vector会warning，虽运行无误，但调试会不便

					if(id_frame_name.find(video_id)!=id_frame_name.end())//找得到
					{
						single_frame_name = id_frame_name[video_id];
						hamming_embedding_result_for_one_video = all_hamming_embedding_result[video_id];
					}

					single_frame_name.push_back(ref_file_list[i]);
					hamming_embedding_result_for_one_video.push_back(hamming_embedding_result_for_image);

					id_frame_name[video_id] = single_frame_name;
					all_hamming_embedding_result[video_id] = hamming_embedding_result_for_one_video;

				}

			}
			else
			{

				vector<vector<int> > pointIdxsOfClusters; 
				Mat cluster_his_single = cp_de.bow_trainning_revise(des,vk,pointIdxsOfClusters);

				if(cluster_his_single.rows>0)
				{			
					Mat single_mat;

					if(id_frame_name.find(video_id)!=id_frame_name.end())//找得到
					{
						single_frame_name = id_frame_name[video_id];
						single_mat = id_mat[video_id];
					}

					single_frame_name.push_back(ref_file_list[i]);
					single_mat.push_back(cluster_his_single);

					id_frame_name[video_id] = single_frame_name;
					id_mat[video_id] = single_mat;
				}
			}

		}

		if (hamming_embedding)
		{
			cp_de.save_video_ref_txt_by_single_with_hamming_embedding(id_frame_name,all_hamming_embedding_result,hamming_embedding_ref_path);
		}
		else
		{
			cp_de.save_video_ref_txt_by_single(id_frame_name,id_mat,ref_bow);
		}
		
	}
	else
	{
		vk = cp_de.load_mat_txt(vk_path.c_str());
	}

	if (hamming_embedding)
	{
		cp_de.load_video_ref_txt_by_single_with_hamming_embedding(id_frame_name,all_hamming_embedding_result,hamming_embedding_ref_path);
	}
	else
	{
		cp_de.load_video_ref_txt_by_single(id_frame_name,id_mat,ref_bow);
	}

	progress_update("查询集分帧进行中……",60);

	cp_de.go_split(query_video,query_frame_pic,frame_interval,split_method);


	if (hamming_embedding)
	{
		progress_update("查询集特征提取和BOW/Hamming_embedding量化进行中……",70);
	}
	else
	{
		progress_update("查询集特征提取和BOW量化进行中……",70);	
	}

	

	Mat query_his;//no hamming
	vector<vector<vector<vector<bool> > > >  query_video_hamming_embedding_result;//hammming
	vector<string> query_bow_file_list;
	
	vector<string> query_file_list = utility::EnumFiles(query_frame_pic);
	
	for(int i=0;i<query_file_list.size();i++)
	{
		string file_name = query_frame_pic + path_split +query_file_list[i];
		Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());   

		vector<KeyPoint> cluster_keypoint;
		Mat des;
		vector<vector<int> > pointIdxsOfClusters; 

		cp_de.description_detect(cluster_image,cluster_keypoint,des,descript_type);
		string des_keypoint_path = query_frame_pic_des_and_keypoint + path_split + query_file_list[i] + ".txt";
		cp_de.save_image_keypoint_and_des_txt_by_single(cluster_keypoint,des,des_keypoint_path);//query 不 load

		if (hamming_embedding)
		{
			vector<vector<vector<bool> > > query_image_hamming_embedding_result = cp_de.hamming_embedding_generate(hamming_embedding_median_path,hamming_embedding_project_matrix,vk,des);
			
			if(cp_de.check_whether_exist_point_in_hamming_embedding(query_image_hamming_embedding_result))
			{
				query_video_hamming_embedding_result.push_back(query_image_hamming_embedding_result);
				query_bow_file_list.push_back(query_file_list[i]);
			}

		} 
		else
		{
			Mat cluster_his_test = cp_de.bow_trainning_revise(des,vk,pointIdxsOfClusters);

			if(cluster_his_test.rows>0)
			{
				query_his.push_back(cluster_his_test);
				query_bow_file_list.push_back(query_file_list[i]);
			}

		}
	}

	progress_update("参照集与查询集比对进行中……",80);

	video_sim_result return_par;

	if(hamming_embedding)
	{
		return_par = cp_de.retrieval_bow_by_cosine_distance_with_hamming_embedding(query_bow_file_list,query_video_hamming_embedding_result,id_frame_name,all_hamming_embedding_result,hamming_embedding_threshold);
	}
	else
	{
		return_par = cp_de.retrieval_bow_by_cosine_distance(query_bow_file_list,query_his,id_frame_name,id_mat);
	}

	progress_update("霍夫变换进行中……",90);

	map<string,double> final_result = cp_de.hough_transform(return_par);

	progress_update("相似视频检测完成",100);

	return final_result;
}

map<string,double> copy_detection::video_video(video_sim_input_ess_par ess_par,video_sim_input_run_par run_par)
{   
    string ref_video = ess_par.ref_video;
    string query_video = ess_par.query_video;
    string ref_frame_pic = ess_par.ref_frame_pic;
    string query_frame_pic = ess_par.query_frame_pic;
    string vk_path = ess_par.vk_path;
    string ref_bow = ess_par.ref_bow;
    int detect_mode = ess_par.detect_mode;
    
    int frame_interval = run_par.frame_interval;
    int cluster_count = run_par.cluster_count;
	int descript_type  = run_par.descript_type;
	int split_method = run_par.split_method;
    
    if(utility::is_folder_empty(query_frame_pic)==0)
    {
        string warning = "警告: " + query_frame_pic + " 文件夹中已经存在若干文件,这些文件会混入最终的分帧文件,可能影响结果.建议找寻空文件夹放置 视频截取帧";
        notify_update(warning);
    }
    if(utility::is_folder_empty(ref_frame_pic)==0)
    {
        string warning = "警告: " + ref_frame_pic + " 文件夹中已经存在若干文件,这些文件会混入最终的分帧文件,可能影响结果.建议找寻空文件夹放置 视频截取帧";
        notify_update(warning);
    }
           
    bool is_vk_exist;
    bool is_ref_bow_exist;
    
    if(detect_mode==NO_VK_NO_REF)
    {
        is_vk_exist = false;
        is_ref_bow_exist = false;
    }
    else if(detect_mode==EXIST_VK_NO_REF)
    {
        is_vk_exist = true;
        is_ref_bow_exist = false;
    }
    else//detect_mode==EXIST_VK_EXIST_REF
    {
        is_vk_exist = true;
        is_ref_bow_exist = true;
    }
    
    progress_update("查询集分帧进行中……",7);

    copy_detection cp_de;
    cp_de.go_split(query_video,query_frame_pic,frame_interval,split_method);
    
    progress_update("参照集分帧进行中……",15);
    
    if(!is_ref_bow_exist)
    {
        vector<string> ref_video_path_list = utility::EnumFiles(ref_video);
        for(int i=0;i<ref_video_path_list.size();i++)
        {
            string video_path = ref_video + path_split + ref_video_path_list[i];
            cp_de.go_split(video_path,ref_frame_pic,frame_interval,split_method);
        }

        
        //分帧

        if(!is_vk_exist)
        {
            progress_update("聚类进行中……",20);
			cp_de.cluster(ref_frame_pic,vk_path,cluster_count,descript_type);
        }

    }

    Mat vk = cp_de.load_mat(vk_path);

	//cp_de.show_mat(vk);

	cp_de.bow_trainning_set_vk_set_and_set_descriptor(vk,descript_type);

    //聚类
    
    progress_update("查询集量化进行中……",40);


    Mat query_his;
    vector<string> query_bow_file_list;
    vector<string> query_file_list = utility::EnumFiles(query_frame_pic);
    for(int i=0;i<query_file_list.size();i++)
    {
        string file_name = query_frame_pic + path_split +query_file_list[i];
        Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());   

        Mat cluster_his_test;

        vector<KeyPoint> cluster_keypoint;
        cp_de.keypoint_detect(cluster_image,cluster_keypoint,descript_type);
        cp_de.bow_trainning(cluster_image,cluster_keypoint,cluster_his_test,query_file_list[i]);

        if(cluster_his_test.rows>0)
        {
            query_his.push_back(cluster_his_test);
            query_bow_file_list.push_back(query_file_list[i]);
        }
    }

    //查询集量化
    
    progress_update("参照集量化进行中……",60);



    map<string,vector<string> > id_frame_name;
    map<string,Mat> id_mat;

    if (!is_ref_bow_exist)
    {
		vector<string> ref_file_list = utility::EnumFiles(ref_frame_pic);
        for(int i=0;i<ref_file_list.size();i++)
        {
            string file_name = ref_frame_pic+path_split+ref_file_list[i];
            Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());

            Mat cluster_his_single;

            vector<KeyPoint> cluster_keypoint;
            cp_de.keypoint_detect(cluster_image,cluster_keypoint,descript_type);
            cp_de.bow_trainning(cluster_image,cluster_keypoint,cluster_his_single,ref_file_list[i]);

            if(cluster_his_single.rows>0)
            {
                string video_id = utility::split(ref_file_list[i],'_')[0];	    

                vector<string> single_frame_name;
                Mat single_mat;

                if(id_frame_name.find(video_id)!=id_frame_name.end())
                {
                    single_frame_name = id_frame_name[video_id];
                    single_mat = id_mat[video_id];
                }

                single_frame_name.push_back(ref_file_list[i]);
                single_mat.push_back(cluster_his_single);

                id_frame_name[video_id] = single_frame_name;
                id_mat[video_id] = single_mat;

            }
        }
        cp_de.save_video_ref(id_frame_name,id_mat,ref_bow);   
    }

    cp_de.load_video_ref(id_frame_name,id_mat,ref_bow);
    
    //参照集量化

    progress_update("参照集与查询集比对进行中……",80);

    video_sim_result return_par = cp_de.retrieval_bow_by_cosine_distance(query_bow_file_list,query_his,id_frame_name,id_mat);

    progress_update("霍夫变换进行中……",90);

    map<string,double> final_result = cp_de.hough_transform(return_par);

    progress_update("相似视频检测完成",100);

    return final_result;
}


//retrieval_bow废弃
video_sim_result copy_detection::retrieval_bow(vector<string> query_bow_file_list,Mat query_his,map<string,vector<string> > id_frame_name,map<string,Mat> id_mat)
{
    video_sim_result return_par;
    return_par.query_set = query_bow_file_list;
    for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
    {   
        string video_id = i->first;

        //FLANN_DIST_L2
        BFMatcher matcher(NORM_L2);
        //FlannBasedMatcher matcher;

        vector<DMatch> matches;
        matcher.match(query_his,id_mat[video_id],matches);


        vector<string> single_frame_name;
        vector<double> single_frame_distance;

        for(int j=0;j<matches.size();j++)
        {
            int img_index = matches[j].trainIdx;
            double distance_single = matches[j].distance;

            single_frame_name.push_back(id_frame_name[video_id][img_index]);
			//single_frame_distance.push_back(distance_single);
			single_frame_distance.push_back(1-distance_single);//特殊处理，注释上行
        }
        
        return_par.ref_set.push_back(single_frame_name);
        return_par.distance.push_back(single_frame_distance);
        //比对
    }

    return return_par;
}

video_sim_result copy_detection::retrieval_bow_by_cosine_distance(vector<string> query_bow_file_list,Mat query_his,map<string,vector<string> > id_frame_name,map<string,Mat> id_mat)
{
	video_sim_result return_par;
	return_par.query_set = query_bow_file_list;
	for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
	{   
		string video_id = i->first;


		vector<Cpde_Matcher> matches;
		get_nearest_matcher_by_cosine_distance(query_his,id_mat[video_id],matches);

		vector<string> single_frame_name;
		vector<double> single_frame_distance;

		for(int j=0;j<matches.size();j++)
		{
			int img_index = matches[j].trainIdx;
			double distance_single = matches[j].distance;

			single_frame_name.push_back(id_frame_name[video_id][img_index]);
			single_frame_distance.push_back(distance_single);
		}
		
		return_par.ref_set.push_back(single_frame_name);
		return_par.distance.push_back(single_frame_distance);
		//比对
	}

	return return_par;
}

video_sim_result copy_detection::retrieval_bow_by_cosine_distance_with_hamming_embedding(vector<string> query_bow_file_list,vector<vector<vector<vector<bool> > > > query_video_hamming_embedding_result,map<string,vector<string> > id_frame_name,map<string,vector<vector<vector<vector<bool> > > > > all_hamming_embedding_result,int hamming_embedding_threshold)
{
	video_sim_result return_par;
	return_par.query_set = query_bow_file_list;
	for(map<string,vector<string> >::iterator i=id_frame_name.begin();i!=id_frame_name.end();i++)
	{   
		string video_id = i->first;

		vector<Cpde_Matcher> matches;
		get_nearest_matcher_by_cosine_distance_hamming_embedding(query_video_hamming_embedding_result,all_hamming_embedding_result[video_id],matches,hamming_embedding_threshold);//除了此句话,the same as retrieval_bow_by_cosine_distance

		vector<string> single_frame_name;
		vector<double> single_frame_distance;

		for(int j=0;j<matches.size();j++)
		{
			int img_index = matches[j].trainIdx;
			double distance_single = matches[j].distance;

			single_frame_name.push_back(id_frame_name[video_id][img_index]);
			single_frame_distance.push_back(distance_single);

		}

		return_par.ref_set.push_back(single_frame_name);
		return_par.distance.push_back(single_frame_distance);
		//比对
	}

	return return_par;
}


void copy_detection::get_nearest_matcher_by_cosine_distance_hamming_embedding(vector<vector<vector<vector<bool> > > > a_video_hamming_embedding_result, vector<vector<vector<vector<bool> > > > b_video_hamming_embedding_result, vector<Cpde_Matcher>& matches,int hamming_embedding_threshold)
{
	for(int i=0;i<a_video_hamming_embedding_result.size();i++)
	{
		double max_hamming_embedding_result_value_between_two_images = 0;
		int img_index = 0;
		for(int j=0;j<b_video_hamming_embedding_result.size();j++)
		{
			double hamming_embedding_result_value_between_two_images = hamming_embedding_cal(a_video_hamming_embedding_result[i],b_video_hamming_embedding_result[j],hamming_embedding_threshold);

			if(hamming_embedding_result_value_between_two_images>max_hamming_embedding_result_value_between_two_images)
			{
				max_hamming_embedding_result_value_between_two_images = hamming_embedding_result_value_between_two_images;
				img_index = j;
			}
		}

		Cpde_Matcher cp_matcher;
		cp_matcher.trainIdx = img_index;
		cp_matcher.distance = max_hamming_embedding_result_value_between_two_images;
		matches.push_back(cp_matcher);
	}
}

void copy_detection::get_nearest_matcher_by_cosine_distance(const Mat& queryDescriptors, const Mat& trainDescriptors, vector<Cpde_Matcher>& matches)
{
	
	for(int i=0;i<queryDescriptors.rows;i++)
	{
		double max_cosine_distance = 0;
		int img_index = 0;
		for(int j=0;j<trainDescriptors.rows;j++)
		{
			vector<float> vector_query;
			vector<float> vector_ref;

			for(int k=0;k<trainDescriptors.cols;k++)
			{
				vector_query.push_back(queryDescriptors.at<float>(i,k));
				vector_ref.push_back(trainDescriptors.at<float>(j,k));
			}

			double cosine = cosine_distance(vector_query,vector_ref);

			if(cosine>max_cosine_distance)
			{
				max_cosine_distance = cosine;
				img_index = j;
			}
		}

		Cpde_Matcher cp_matcher;
		cp_matcher.trainIdx = img_index;
		cp_matcher.distance = max_cosine_distance;
		matches.push_back(cp_matcher);
	}
}

double copy_detection::cosine_distance(vector<float> vector_query,vector<float> vector_ref)
{
	//vector_query.size() == vector_ref.size()
	
	double upper_num = 0;
	double vector_query_mo = 0;
	double vector_ref_mo = 0;
	

	for(int i=0;i<vector_query.size();i++)
	{
		upper_num += (vector_query[i]*vector_ref[i]);
		vector_query_mo += (vector_query[i]*vector_query[i]);
		vector_ref_mo += (vector_ref[i]*vector_ref[i]);
	}

	vector_query_mo = ::sqrt(vector_query_mo);
	vector_ref_mo = ::sqrt(vector_ref_mo);

	double return_cosine = upper_num/(vector_query_mo*vector_ref_mo);

	return return_cosine;
}



map<string,double> copy_detection::pic_video(string v_path_folder,string frame_pic_d_path,string img_path,string vk_path,bool is_vk_exist,string ref_bow,bool is_ref_bow_exist,string matcher_string,bool is_matcher_exist)
{

    if((is_ref_bow_exist==false) && (is_matcher_exist==true))
    {

        cout<<"Invalid method use 'is_ref_bow_exist==false and 'is_matcher_exist==true'"<<endl;
        system("pause");
        exit(0);
    }

    copy_detection cp_de;


    //参照集量化

    //FLANN_DIST_L2
    //BFMatcher matcher(NORM_L2);

    Mat query_his;
    Mat ref_his;
    vector<string> bow_file_list;
    Mat vk;


    if(!is_ref_bow_exist)
    {
        vector<string> video_path_list = utility::EnumFiles(v_path_folder);
        for(int i=0;i<video_path_list.size();i++)
        {
            string video_path = v_path_folder+path_split+video_path_list[i];
            cp_de.go_split(video_path,frame_pic_d_path,100,USE_FRAME);
        }
        //分帧


        if(!is_vk_exist)
        {

			cp_de.cluster(frame_pic_d_path,vk_path,100,SIFT_DESCRIPTION);
            
            //聚类
        }


    }

    vk = cp_de.load_mat(vk_path);
    cp_de.bow_trainning_set_vk_set_and_set_descriptor(vk,SIFT_DESCRIPTION);

    if(!is_ref_bow_exist)
    {
        vector<string> file_list = utility::EnumFiles(frame_pic_d_path);
        for(int i=0;i<file_list.size();i++)
        {
            string file_name = frame_pic_d_path+path_split+file_list[i];
            Ptr<IplImage> cluster_image = cvLoadImage(file_name.c_str());   

            Mat cluster_his_test;

            vector<KeyPoint> cluster_keypoint;
            cp_de.keypoint_detect(cluster_image,cluster_keypoint,SIFT_DESCRIPTION);
            cp_de.bow_trainning(cluster_image,cluster_keypoint,cluster_his_test,file_list[i]);

            if(cluster_his_test.rows>0)
            {
                ref_his.push_back(cluster_his_test);
                bow_file_list.push_back(file_list[i]);
            }
        }

        cp_de.save_ref(ref_his,bow_file_list,ref_bow);
    }

    cp_de.load_ref(ref_his,bow_file_list,ref_bow);

    if(!is_matcher_exist)
    {
        cv::flann::Index treeFlannIndex_train;
        treeFlannIndex_train.build(ref_his, cv::flann::KDTreeIndexParams());
        treeFlannIndex_train.save(matcher_string.c_str());  
    }
    cv::flann::Index treeFlannIndex(ref_his,cv::flann::SavedIndexParams(matcher_string.c_str()));

    Ptr<IplImage> image = cvLoadImage(img_path.c_str(),CV_LOAD_IMAGE_GRAYSCALE);
    vector<KeyPoint> keypoints;
    Mat des;
    cp_de.description_detect(image,keypoints,des,SIFT_DESCRIPTION);
    cp_de.bow_trainning(image,keypoints,query_his,img_path);

    //查询集量化



    Mat out_train_id;
    Mat out_train_distance;

    const int max_result = 100;
    int how_many_result = treeFlannIndex.radiusSearch(query_his,out_train_id,out_train_distance,0.01,max_result);

    vector<string> ref_set;
    vector<double> ref_distance;

    for(int i=0;i<min(how_many_result,max_result);i++)
    {
        int img_index = out_train_id.at<int>(0,i);
        double single_distance = out_train_distance.at<float>(0,i);

        ref_set.push_back(bow_file_list[img_index]);
        ref_distance.push_back(single_distance);
    }




    //比对

    pic_sim_result return_par;
    return_par.query_set = img_path;
    return_par.ref_set = ref_set;
    return_par.ref_distance = ref_distance;
    //事实上return_par不返回在此处,可选择返回return_par,不过这里选择返回map

    map<string,double> final_result;
    
    for(int i=0;i<ref_set.size();i++)
    {
        string video_id = ref_set[i];
        double score = ref_distance[i];
        final_result[video_id] = score;
    }
    
    return final_result;
    
    
}
