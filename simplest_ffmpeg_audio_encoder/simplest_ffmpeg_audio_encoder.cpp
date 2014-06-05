/* 
 *最简单的基于FFmpeg的音频编码器
 *Simplest FFmpeg Audio Encoder
 *
 *雷霄骅 Lei Xiaohua
 *leixiaohua1020@126.com
 *中国传媒大学/数字电视技术
 *Communication University of China / Digital TV Technology
 *http://blog.csdn.net/leixiaohua1020
 *
 *本程序实现了音频PCM采样数据编码为压缩码流（MP3，WMA，AAC等）。
 *是最简单的FFmpeg音频编码方面的教程。
 *通过学习本例子可以了解FFmpeg的编码流程。
 *This software encode PCM data to AAC bitstream.
 *It's the simplest audio encoding software based on FFmpeg. 
 *Suitable for beginner of FFmpeg 
 */

#include "stdafx.h"

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
};

int _tmain(int argc, _TCHAR* argv[])
{
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* audio_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* frame_buf;
	AVFrame* frame;
	int size;

	FILE *in_file = fopen("tdjm.pcm", "rb");	//音频PCM采样数据 
	int framenum=1000;	//音频帧数
	const char* out_file = "tdjm.aac";					//输出文件路径


	av_register_all();

	//方法1.组合使用几个函数
	pFormatCtx = avformat_alloc_context();
	//猜格式
	fmt = av_guess_format(NULL, out_file, NULL);
	pFormatCtx->oformat = fmt;


	//方法2.更加自动化一些
	//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	//fmt = pFormatCtx->oformat;

	//注意输出路径
	if (avio_open(&pFormatCtx->pb,out_file, AVIO_FLAG_READ_WRITE) < 0)
	{
		printf("输出文件打开失败！\n");
		return -1;
	}

	audio_st = av_new_stream(pFormatCtx, 0);
	if (audio_st==NULL){
		return -1;
	}
	pCodecCtx = audio_st->codec;
	pCodecCtx->codec_id = fmt->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate= 44100;
	pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;  

	//输出格式信息
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		printf("没有找到合适的编码器！\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
	{
		printf("编码器打开失败！\n");
		return -1;
	}
	frame = avcodec_alloc_frame();
	frame->nb_samples= pCodecCtx->frame_size;
	frame->format= pCodecCtx->sample_fmt;
	
	size = av_samples_get_buffer_size(NULL, pCodecCtx->channels,pCodecCtx->frame_size,pCodecCtx->sample_fmt, 1);
	frame_buf = (uint8_t *)av_malloc(size);
	avcodec_fill_audio_frame(frame, pCodecCtx->channels, pCodecCtx->sample_fmt,(const uint8_t*)frame_buf, size, 1);
	
	//写文件头
	avformat_write_header(pFormatCtx,NULL);

	AVPacket pkt;
	av_new_packet(&pkt,size);

	for (int i=0; i<framenum; i++){
		//读入PCM
		if (fread(frame_buf, 1, size, in_file) < 0)
		{
			printf("文件读取错误！\n");
			return -1;
		}else if(feof(in_file)){
			break;
		}
		frame->data[0] = frame_buf;  //采样信号

		frame->pts=i*100;
		int got_frame=0;
		//编码
		int ret = avcodec_encode_audio2(pCodecCtx, &pkt,frame, &got_frame);
		if(ret < 0)
		{
			printf("编码错误！\n");
			return -1;
		}
		if (got_frame==1)
		{
			printf("编码成功第%d帧！\n",i);
			pkt.stream_index = audio_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}
	
	//写文件尾
	av_write_trailer(pFormatCtx);

	//清理
	if (audio_st)
	{
		avcodec_close(audio_st->codec);
		av_free(frame);
		av_free(frame_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;
}


