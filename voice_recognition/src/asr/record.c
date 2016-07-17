/*

This program reads from the default PCM device
and writes to standard output for 5 seconds of data.

*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define LENGTH    3   //录音时间,秒
#define RATE    16000 //采样频率
#define SIZE    16   //量化位数
#define CHANNELS 1   //声道数目
#define RSIZE    8    //buf的大小，

/********以下是wave格式文件的文件头格式说明******/
/*------------------------------------------------
|             RIFF WAVE Chunk                  |
|             ID = 'RIFF'                     |
|             RiffType = 'WAVE'                |
------------------------------------------------
|             Format Chunk                     |
|             ID = 'fmt '                      |
------------------------------------------------
|             Fact Chunk(optional)             |
|             ID = 'fact'                      |
------------------------------------------------
|             Data Chunk                       |
|             ID = 'data'                      |
------------------------------------------------*/
/**********以上是wave文件格式头格式说明***********/
/*wave 文件一共有四个Chunk组成，其中第三个Chunk可以省略，每个Chunk有标示（ID）,
大小（size,就是本Chunk的内容部分长度）,内容三部分组成*/
struct fhead
{
    /****RIFF WAVE CHUNK*/
    unsigned char a[4];//四个字节存放'R','I','F','F'
    long int b;        //整个文件的长度-8;每个Chunk的size字段，都是表示除了本Chunk的ID和SIZE字段外的长度;
    unsigned char c[4];//四个字节存放'W','A','V','E'
    /****RIFF WAVE CHUNK*/
    /****Format CHUNK*/
    unsigned char d[4];//四个字节存放'f','m','t',''
    long int e;       //16后没有附加消息，18后有附加消息；一般为16，其他格式转来的话为18
    short int f;       //编码方式，一般为0x0001;
    short int g;       //声道数目，1单声道，2双声道;
    long int h;        //采样频率;
    long int i;        //每秒所需字节数;
    short int j;       //每个采样需要多少字节，若声道是双，则两个一起考虑;
    short int k;       //即量化位数
    /****Format CHUNK*/
    /***Data Chunk**/
    unsigned char p[4];//四个字节存放'd','a','t','a'
    long int q;        //语音数据部分长度，不包括文件头的任何部分
}wavehead;//定义WAVE文件的文件头结构体


int startRecord(char* pcm_driver)
{
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    int fd_f;
    int status;



    /*以下wave 文件头赋值*/
    wavehead.a[0]='R';
    wavehead.a[1]='I';
    wavehead.a[2]='F';
    wavehead.a[3]='F';
    wavehead.b=LENGTH*RATE*CHANNELS*SIZE/8-8;
    wavehead.c[0]='W';
    wavehead.c[1]='A';
    wavehead.c[2]='V';
    wavehead.c[3]='E';
    wavehead.d[0]='f';
    wavehead.d[1]='m';
    wavehead.d[2]='t';
    wavehead.d[3]=' ';
    wavehead.e=16;
    wavehead.f=1;
    wavehead.g=CHANNELS;
    wavehead.h=RATE;
    wavehead.i=RATE*CHANNELS*SIZE/8;
    wavehead.j=CHANNELS*SIZE/8;
    wavehead.k=SIZE;
    wavehead.p[0]='d';
    wavehead.p[1]='a';
    wavehead.p[2]='t';
    wavehead.p[3]='a';
    wavehead.q=LENGTH*RATE*CHANNELS*SIZE/8;
    /*以上wave 文件头赋值*/


    /* Open PCM device for recording (capture). */
    rc = snd_pcm_open(&handle, pcm_driver,
                        SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0) {
        fprintf(stderr,
                "unable to open pcm device: %s\n",
                snd_strerror(rc));
        exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);

    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params,
                        SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params,
                                SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, CHANNELS);

    /* 16000 bits/second sampling rate (CD quality) */
    val = RATE;
    snd_pcm_hw_params_set_rate_near(handle, params,
                                    &val, &dir);

    /* Set period size to 32 frames. */
    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle,
                                params, &frames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        fprintf(stderr,
                "unable to set hw parameters: %s\n",
                snd_strerror(rc));
        exit(1);
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params,
                                        &frames, &dir);
    size = frames * 2; /* 2 bytes/sample, 2 channels */
    buffer = (char *) malloc(size);

    /* We want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params,
                                            &val, &dir);
    loops = 3000000 / val;




    if(( fd_f = open("wav/test2.wav", O_CREAT|O_RDWR,0777))==-1)//创建一个wave格式语音文件
    {
        perror("cannot creat the sound file");
    }
    if((status = write(fd_f, &wavehead, sizeof(wavehead)))==-1)//写入wave文件的文件头
    {
        perror("write to sound'head wrong!!");
    }

    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            fprintf(stderr, "overrun occurred\n");
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            fprintf(stderr,
                    "error from read: %s\n",
                    snd_strerror(rc));
        } else if (rc != (int)frames) {
            fprintf(stderr, "short read, read %d frames\n", rc);
        }
    //  rc = write(1, buffer, size);
        //seek(fd_f,0L,SEEK_END);
        if(write(fd_f, buffer, size)==-1)
        {
            perror("write to sound wrong!!");
        }
        //if (rc != size)
            //fprintf(stderr,
              //      ">", rc);
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);
    close(fd_f);

    return 0;
}
